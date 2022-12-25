#include "scene.h"

static inline float rand2(void *state) {
    return (float)rand_r(state) / RAND_MAX;
}

void add_instance(Scene *self, SDFInstance *instance) {
    assert(self->instance_count < SCENE_INSTANCES_MAX);
    *(self->instances + self->instance_count++) = instance;
}

void add_light(Scene *self, Light *light) {
    assert(self->light_count < SCENE_LIGHTS_MAX);
    *(self->lights + self->light_count++) = light;
}

float map(Scene *self, Vector3 *position, SDFInstance **out) {
    *out = *self->instances;
    float closest_distance = (*out)->get_distance(*out, position);
    for (int j = 1; j < self->instance_count; j++) {
        SDFInstance *instance = *(self->instances + j);
        float distance = instance->get_distance(instance, position);
        if (distance < closest_distance) {
            closest_distance = distance;
            *out = instance;
            if (closest_distance < EPSILON) {
                break;
            }
        }
    }
    return closest_distance;
}

int is_inside_instance(Scene *self, Vector3 *position, SDFInstance **out) {
    return map(self, position, out) < 0;
}

//TODO leverage explicit surface formulas
//Leave ray marching for implicit surfaces (mandelbulb, etc.)
//Over-relaxation sphere tracing: https://erleuchtet.org/~cupe/permanent/enhanced_sphere_tracing.pdf
SDFInstance* ray_march(Scene *self, Ray *r, float t_max, Vector3 *out) {
    SDFInstance *result = NULL;
    int direction = is_inside_instance(self, r->origin, &result) ? -1 : 1;
    register int backtracked = 0;
    register float omega = 1.99; //Over-relaxation factor [1, 2)
    float last_radius = 0;
    float total_distance = 0;
    float next_step = 0;
    vec3_cpy(r->origin, out);
    for (int i = 0; i < SCENE_MARCH_ITER_MAX; i++) {
        float radius = direction * map(self, out, &result);
        float abs_radius = fabsf(radius);
        if (abs_radius < EPSILON) {
            break;
        }
        else if (backtracked) {
            next_step = radius;
        }
        else if (omega * last_radius > last_radius + abs_radius) {
            next_step -= omega * next_step;
            backtracked = 1;
        }
        else {
            last_radius = abs_radius;
            next_step = omega * radius;
        }
        total_distance += next_step;
        if (total_distance > t_max) {
            result = NULL;
            break;
        }
        vec3_fma(r->origin, r->direction, total_distance, out);
    }
    return result;
}

Vector3* perturb_vector3(Vector3 *position, Vector3* offset, Vector3* out) {
    return vec3_fma(position, offset, 2 * EPSILON, out);
}

Vector3* reflect_vector3(Vector3 *direction, Vector3 *normal, Vector3 *out) {
    return vec3_fma(direction, normal, -2 * vec3_dot(direction, normal), out);
}

Vector3* refract_vector3(Vector3 *direction, Vector3 *normal, float r, Vector3 *out) {
    float c = vec3_dot(direction, normal);
    float s =  1 - r * r * (1 - c * c);
    if (s < 0) {
        return NULL;
    } 
    return vec3_fma(vec3_mul(direction, r, out), normal, -r * c + (c > 0 ? sqrtf(s) : -sqrtf(s)), out);
}

float fresnel(Vector3 *direction, Vector3 *normal, float ior_in, float ior_out) {
    float c = 1 - fabsf(vec3_dot(direction, normal));
    float r = (ior_in - ior_out) / (ior_in + ior_out);
    r *= r;
    return r + (1 - r) * c * c * c * c * c;
    
}

static inline float get_distance_axis(SDFInstance *instance, Vector3 *position, Vector3 *axis) {
    Vector3 temp_v;
    return instance->get_distance(instance, vec3_add(position, axis, &temp_v))
         - instance->get_distance(instance, vec3_sub(position, axis, &temp_v));
}

Vector3 *get_normal(SDFInstance *instance, Vector3 *position, Vector3 *out) {
    static Vector3 dx = {EPSILON, 0, 0};
    static Vector3 dy = {0, EPSILON, 0};
    static Vector3 dz = {0, 0, EPSILON};
    out->x = get_distance_axis(instance, position, &dx);
    out->y = get_distance_axis(instance, position, &dy);
    out->z = get_distance_axis(instance, position, &dz);
    return vec3_unit(out, out);
}

Color3* get_light_color(Scene *self, Vector3 *position, Vector3 *normal, Color3 *out) {
    Vector3 temp_v, origin, direction;
    Ray temp_r = {&origin, &direction};
    perturb_vector3(position, normal, &origin);
    col3_smul(out, 0, out);
    register int light_count = self->light_count;
    for (int i = 0; i < light_count; i++) {
        Light *l = *(self->lights + i);
        Vector3 *offset = vec3_sub(l->instance->position, position, &direction);
        int is_visible = l->visibility == ALWAYS_VISIBLE;
        if (!is_visible) {
            float offset_mag = vec3_mag(offset);
            vec3_unit(offset, offset);
            is_visible = ray_march(self, &temp_r, offset_mag, &temp_v) == NULL;
        }
        if (is_visible) {
            float local_brightness = l->get_brightness(l, position);
            if (l->visibility == LINE_OF_SIGHT) {
                local_brightness *= vec3_dot(normal, offset);
            }
            col3_sfma(out, l->color, local_brightness, out);
        }
    }
    return out;
}

Color3 *get_color_monte_carlo(Scene *self, Ray *r, void *rand_state, Color3 *out) {
    SDFInstance *temp_i;
    Color3 temp_c = {};
    Vector3 temp_v = {};
    Color3 attenuation = {1, 1, 1};
    Color3 light_color = {};
    Vector3 position = {};
    Vector3 position_last = {};
    Vector3 position_pn = {};
    Vector3 normal = {};
    Vector3 normal_neg = {};
    Vector3 origin = {};
    Vector3 direction = {};
    Ray cr = {vec3_cpy(r->origin, &origin), vec3_cpy(r->direction, &direction)};
    float alpha = 1;
    float rr = 1;
    float ior = SCENE_ATMOSPHERE_IOR;
    col3_smul(out, 0, out);
    do {
        vec3_cpy(&position, &position_last);
        SDFInstance *hit_instance = ray_march(
            self,
            &cr,
            SCENE_MARCH_DIST_MAX,
            &position
        );
        if (!hit_instance) {
            if (self->environment) {
                col3_sfma(out, col3_mul(self->environment->sample(self->environment, &direction, &temp_c), &attenuation, &temp_c), alpha, out);
            }
            break;
        }

        Material *mat = hit_instance->material;

        get_normal(hit_instance, &position, &normal);
        if (vec3_dot(&direction, &normal) > 0) {
            vec3_neg(&normal, &normal);
            col3_mul(&attenuation, col3_exp(col3_smul(col3_log(mat->color, &temp_c), vec3_mag(vec3_sub(&position, &position_last, &temp_v)), &temp_c), &temp_c), &attenuation);
        }

        float fresnel_add = fresnel(&direction, &normal, ior, mat->ior) * (1 - mat->reflectance);
        float reflectance = mat->reflectance + fresnel_add;
        float transmission = (1 - reflectance) * mat->transmission;
        float diffuse = (1 - reflectance) * (1 - mat->transmission);

        if (diffuse < 1) {
            alpha *= 1 - diffuse;
            if (rand2(rand_state) * (reflectance + transmission) < reflectance) {
                perturb_vector3(&position, &normal, &origin);
                reflect_vector3(&direction, &normal, &temp_v);
                vec3_cpy(&temp_v, &direction);
            }
            else {
                float next_ior = SCENE_ATMOSPHERE_IOR;
                vec3_neg(&normal, &normal_neg);
                perturb_vector3(&position, &normal_neg, &position_pn);
                if (is_inside_instance(self, &position_pn, &temp_i)) {
                    next_ior = temp_i->material->ior;
                }
                if (refract_vector3(&direction, &normal, ior / next_ior, &temp_v)) {
                    vec3_cpy(&position_pn, &origin);
                    vec3_cpy(&temp_v, &direction);
                    ior = next_ior;
                }
                else {
                    perturb_vector3(&position, &normal, &origin);
                    reflect_vector3(&direction, &normal, &temp_v);
                    vec3_cpy(&temp_v, &direction);
                }
            }
        }
        else {
            rr = 0;
        }
        if (diffuse > SCENE_ALPHA_MIN) {
            if (hit_instance->material->checker) {
                diffuse *= ((int)floorf(position.x / 2) % 2 + (int)floorf(position.y / 2) % 2 + (int)floorf(position.z / 2) % 2) % 2 ? 1 : .375;
            }
            get_light_color(self, &position, &normal, &light_color);
            col3_sfma(out, col3_mul(&light_color, col3_mul(mat->color, &attenuation, &temp_c), &temp_c), diffuse * alpha, out);
        }
    } while ((rr *= .99) > rand2(rand_state));
    return out;
}

Color3* tonemap(Color3 *a, Color3 *out) {
    Color3 temp_c1, temp_c2;

    out->r = 0.59719 * a->r + 0.35458 * a->g + 0.04823 * a->b;
    out->g = 0.07600 * a->r + 0.90834 * a->g + 0.01566 * a->b;
    out->b = 0.02840 * a->r + 0.13383 * a->g + 0.83777 * a->b;

    col3_sadd(col3_mul(out, col3_sadd(out, 0.0245786, &temp_c1), &temp_c1), -0.000090537, &temp_c1);
    col3_sadd(col3_mul(out, col3_sadd(col3_smul(out, 0.983729, &temp_c2), 0.4329510, &temp_c2), &temp_c2), 0.238081, &temp_c2);
    col3_div(&temp_c1, &temp_c2, &temp_c1);

    out->r =  1.60475 * temp_c1.r - 0.53108 * temp_c1.g - 0.07367 * temp_c1.b;
    out->g = -0.10208 * temp_c1.r + 1.10813 * temp_c1.g - 0.00605 * temp_c1.b;
    out->b = -0.00327 * temp_c1.r - 0.07276 * temp_c1.g + 1.07602 * temp_c1.b;

    col3_spow(col3_clamp(out, out), 1 / 2.2, out);
    return out;
}

static inline void render_thread(SceneRenderArgs *args) {
    Scene *self = args->self;
    Camera *camera = args->camera;
    unsigned int thread_i = args->thread_i;
    unsigned int *output = args->output;
    int *line_cnt = args->line_cnt;
    unsigned int rand_state = thread_i;

    Color3 temp_c, temp_cout;
    Vector3 temp_v1, temp_v2;
    Ray temp_r = {&temp_v1, &temp_v2};
    float alpha = 1. / (SCENE_OUTPUT_SAMPLES * SCENE_OUTPUT_SAMPLES);

    int i;
    while ((i = (*line_cnt)++) < SCENE_OUTPUT_HEIGHT) {
        for (int j = 0; j < SCENE_OUTPUT_WIDTH; j++) {
            temp_cout = (Color3){0, 0, 0};
            for (int k = 0; k < SCENE_OUTPUT_SAMPLES; k++) {
                for (int l = 0; l < SCENE_OUTPUT_SAMPLES; l++) {
                    camera->get_ray(
                        camera,
                        (j + (l + .5) / SCENE_OUTPUT_SAMPLES) / (SCENE_OUTPUT_WIDTH - 1),
                        (i + (k + .5) / SCENE_OUTPUT_SAMPLES) / (SCENE_OUTPUT_HEIGHT - 1),
                        &rand_state,
                        &temp_r
                    );
                    get_color_monte_carlo(self, &temp_r, &rand_state, &temp_c);
                    col3_sfma(&temp_cout, &temp_c, alpha, &temp_cout);
                }
            }
            *(output + SCENE_OUTPUT_WIDTH * i + j) = col3_to_int(tonemap(&temp_cout, &temp_cout));
        }
    }
}

unsigned int* render(Scene *self, Camera *camera) {
    pthread_t threads[SCENE_RENDER_THREADS];
    SceneRenderArgs thread_args[SCENE_RENDER_THREADS];
    unsigned int *output = malloc(sizeof(unsigned int *) * SCENE_OUTPUT_HEIGHT * SCENE_OUTPUT_WIDTH);
    assert(output);
    int line_cnt = 0;

    if (!self->instance_count) {
        return output;
    }
    
    for (int i = 0; i < SCENE_RENDER_THREADS; i++) {
        *(thread_args + i) = (SceneRenderArgs){self, camera, i, output, &line_cnt};
        pthread_create(threads + i, NULL, (void *)render_thread, thread_args + i);
    }
    for (int i = 0; i < SCENE_RENDER_THREADS; i++) {
        pthread_join(*(threads + i), NULL);
    }
    return output;
}

Scene* scene() {
    Scene *x = malloc(sizeof(Scene));
    assert(x);
    SDFInstance **instances = malloc(sizeof(SDFInstance *) * SCENE_INSTANCES_MAX);
    assert(instances);
    Light **lights = malloc(sizeof(Light *) * SCENE_LIGHTS_MAX);
    assert(lights);
    x->instance_count = 0;
    x->light_count = 0;
    x->environment = NULL;
    x->instances = instances;
    x->lights = lights;
    x->add_instance = add_instance;
    x->add_light = add_light;
    x->render = render;
    return x;
}