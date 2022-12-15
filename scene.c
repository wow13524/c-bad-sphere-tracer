#include "scene.h"

int pixels = 0;
int marches = 0;

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
    register int instance_count;
    if (!(instance_count = self->instance_count)) {
        *out = NULL;
        return FLT_MAX;
    }
    *out = *self->instances;
    register float closest_distance = (*out)->get_distance(*out, position);
    for (int j = 1; j < instance_count; j++) {
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
    marches++;
    SDFInstance *result = NULL;
    SDFInstance *temp = NULL;
    int direction = is_inside_instance(self, r->origin, &temp) ? -1 : 1;
    register float omega = 1.75; //Over-relaxation factor [1, 2)
    float last_radius = 0;
    float total_distance = 0;
    float next_step = 0;
    for (int i = 0; i < SCENE_MARCH_ITER_MAX; i++) {
        float radius = direction * map(self, vec3_fma(r->origin, r->direction, total_distance, out), &result);
        float abs_radius = fabsf(radius);
        if (omega != 1 && omega * last_radius > last_radius + abs_radius) {
            next_step -= omega * next_step;
            omega = 1;
        }
        else {
            next_step = omega * radius;
        }
        last_radius = abs_radius;
        total_distance += next_step;
         if (total_distance > t_max) {
            result = NULL;
            break;
        }
        if (abs_radius < EPSILON) {
            break;
        }

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
    Vector3 temp_v = (Vector3){};
    float c = vec3_dot(normal, direction);
    float s =  1 - r * r * (1 - c * c);
    if (s < 0) {
        return NULL;
    } 
    else if(c > 0) {    //Correct for when ray is leaving an object
        return vec3_fma(vec3_mul(direction, r, &temp_v), normal, -r * c + sqrtf(s), out);
    }
    else {
        return vec3_fma(vec3_mul(direction, r, &temp_v), normal, -r * c - sqrtf(s), out);
    }
}

float fresnel(Vector3 *direction, Vector3 *normal, float ior_in, float ior_out) {
    float c = 1 - fabsf(vec3_dot(direction, normal));
    float r = (ior_in - ior_out) / (ior_in + ior_out);
    r *= r;
    return r + (1 - r) * c * c * c * c * c;
    
}

float get_distance_axis(SDFInstance *instance, Vector3 *position, Vector3 *axis) {
    Vector3 temp_v = {};
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

//TODO take instance transmission into account
Color3* get_light_color(Scene *self, Vector3 *position, Vector3 *normal, Color3 *out) {
    Vector3 temp_v = (Vector3){};
    Vector3 origin = (Vector3){};
    Vector3 direction = (Vector3){};
    Ray temp_r = (Ray){&origin, &direction};
    perturb_vector3(position, normal, &origin);
    col3_smul(out, 0, out);
    register Light **lights = self->lights;
    register int light_count = self->light_count;
    for (int i = 0; i < light_count; i++) {
        Light *l = *(lights + i);
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

Color3* get_color_iterative(Scene *self, Ray *r, Color3 *out) {
    static float *alpha_stack = NULL;
    static int *back_stack = NULL;
    static int *depth_stack = NULL;
    static float *ior_stack = NULL;
    static Color3 **attenuation_stack = NULL;
    static SDFInstance **instance_stack = NULL;
    static Vector3 **normal_stack = NULL;
    static Vector3 **position_stack = NULL;
    static Ray **ray_stack = NULL;
    static Color3 *temp_c = NULL;
    static SDFInstance *temp_i = NULL;
    static Vector3 *temp_v = NULL;
    //static Color3 *light_color = NULL;
    static Vector3 *perturbed_position = NULL;
    if (!ior_stack) {   //Initialize static temp variables
        int stack_size = pow(SCENE_RECURSION_DEPTH, 3);
        assert((alpha_stack = malloc(sizeof(float) * stack_size)));
        assert((back_stack = malloc(sizeof(int) * stack_size)));
        assert((depth_stack = malloc(sizeof(int) * stack_size)));
        assert((ior_stack = malloc(sizeof(float) * stack_size)));
        assert((attenuation_stack = malloc(sizeof(Color3 *) * stack_size)));
        assert((instance_stack = malloc(sizeof(SDFInstance *) * stack_size)));
        assert((normal_stack = malloc(sizeof(Vector3 *) * stack_size)));
        assert((position_stack = malloc(sizeof(Vector3 *) * stack_size)));
        assert((ray_stack = malloc(sizeof(Ray *) * stack_size)));
        temp_c = color3(0, 0, 0);
        temp_v = vector3(0, 0, 0);
        //light_color = color3(0, 0, 0);
        perturbed_position = vector3(0, 0, 0);
        for (int i = 0; i < stack_size; i++) {
            *(attenuation_stack + i) = color3(0, 0, 0);
            *(normal_stack + i) = vector3(0, 0, 0);
            *(position_stack + i) = vector3(0, 0, 0);
            *(ray_stack + i) = ray(vector3(0, 0, 0), vector3(0, 0, 0));
        }
    }

    *alpha_stack = 1;
    *back_stack = -1;
    *depth_stack = 1;
    *ior_stack = is_inside_instance(self, r->origin, &temp_i) ? temp_i->material->ior : SCENE_ATMOSPHERE_IOR;
    vec3_cpy(r->origin, (*ray_stack)->origin);
    vec3_cpy(r->direction, (*ray_stack)->direction);
    (*attenuation_stack)->r = 1;
    (*attenuation_stack)->g = 1;
    (*attenuation_stack)->b = 1;
    col3_smul(out, 0, out);
    int offset = 0;
    int total = 1;

    do {
        float curr_alpha = *(alpha_stack + offset);
        int curr_back = *(back_stack + offset);
        int curr_depth = *(depth_stack + offset);
        float curr_ior = *(ior_stack + offset);
        Color3 *curr_attenuation = *(attenuation_stack);
        Vector3 *curr_normal = *(normal_stack + offset);
        Vector3 *curr_position = *(position_stack + offset);
        Ray *curr_ray = *(ray_stack + offset);
        SDFInstance *hit_instance =  (*(instance_stack + offset) = ray_march(
            self,
            curr_ray,
            SCENE_MARCH_DIST_MAX,
            curr_position
        ));
        if (hit_instance) {
            get_normal(hit_instance, curr_position, curr_normal);
            if (vec3_dot(curr_ray->direction, curr_normal) > 0) {
                vec3_neg(curr_normal, curr_normal);
            }
            float next_ior = SCENE_ATMOSPHERE_IOR;
            perturb_vector3(curr_position, vec3_neg(curr_normal, temp_v), perturbed_position);
            if (is_inside_instance(self, perturbed_position, &temp_i)) {
                next_ior = temp_i->material->ior;
            }

            float reflectance = hit_instance->material->reflectance;
            float fresnel_add = fresnel(curr_ray->direction, curr_normal, curr_ior, next_ior) * (1 - reflectance);

            float reflectance_alpha = curr_alpha * (reflectance + fresnel_add);
            float transmission_alpha = curr_alpha * (1 - (reflectance + fresnel_add)) * hit_instance->material->transmission;
            float diffuse_alpha = curr_alpha * (1 - reflectance) * (1 - hit_instance->material->transmission);

            if (is_inside_instance(self, curr_ray->origin, &temp_i)) {   //Beer's Law
                col3_mul(curr_attenuation, col3_exp(col3_smul(col3_log((*(instance_stack + curr_back))->material->color, temp_c), vec3_mag(vec3_sub(curr_position, *(position_stack + curr_back), temp_v)), temp_c), temp_c), curr_attenuation);
            }

            if (diffuse_alpha > SCENE_ALPHA_MIN) {
                /*get_light_color(self, curr_position, curr_normal, light_color);
                if (hit_instance->material->checker) {
                    diffuse_alpha *= ((int)floorf(curr_position->x / 2) % 2 + (int)floorf(curr_position->y / 2) % 2 + (int)floorf(curr_position->z / 2) % 2) % 2 ? 1 : .375;
                }*/
                //col3_sfma(out, col3_mul(col3_mul(light_color, hit_instance->material->color, temp_c), curr_attenuation, temp_c), diffuse_alpha, out);
                col3_sfma(out, hit_instance->material->color, diffuse_alpha, out);
            }

            if (curr_depth < SCENE_RECURSION_DEPTH) {
                if (0 && transmission_alpha > SCENE_ALPHA_MIN) {
                    Ray *next_ray = *(ray_stack + total);
                    vec3_cpy(perturbed_position, next_ray->origin);
                    *(alpha_stack + total) = transmission_alpha;
                    *(back_stack + total) = offset;
                    *(depth_stack + total) = curr_depth + 1;
                    *(ior_stack + total) = next_ior;
                    col3_cpy(curr_attenuation, *(attenuation_stack + total));
                    if (refract_vector3(curr_ray->direction, curr_normal, curr_ior / next_ior, next_ray->direction)) {
                        total++;
                    }
                }

                if (reflectance_alpha > SCENE_ALPHA_MIN) {
                    Ray *next_ray = *(ray_stack + total);
                    *(alpha_stack + total) = reflectance_alpha;
                    *(back_stack + total) = offset;
                    *(depth_stack + total) = curr_depth + 1;
                    *(ior_stack + total) = curr_ior;
                    col3_cpy(curr_attenuation, *(attenuation_stack + total));
                    perturb_vector3(curr_position, curr_normal, next_ray->origin);
                    reflect_vector3(curr_ray->direction, curr_normal, next_ray->direction);
                    total++;
                }
            }
        }
        else if (self->environment) {
            //col3_sfma(out, col3_mul(self->environment->sample(self->environment, curr_ray->direction, temp_c), curr_attenuation, temp_c), curr_alpha, out);
            col3_sfma(out, self->environment->sample(self->environment, curr_ray->direction, temp_c), curr_alpha, out);
        }
    } while (++offset < total);

    return out;
}

Color3 *get_color_monte_carlo(Scene *self, Ray *r, void *rand_state, Color3 *out) {
    SDFInstance *temp_i;
    Color3 temp_c = {};
    Vector3 temp_v = {};
    Vector3 position = {};
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
        SDFInstance *hit_instance = ray_march(
            self,
            &cr,
            SCENE_MARCH_DIST_MAX,
            &position
        );
        if (!hit_instance) {
            col3_sfma(out, self->environment->sample(self->environment, &direction, &temp_c), alpha, out);
            break;
        }

        Material *mat = hit_instance->material;

        get_normal(hit_instance, &position, &normal);
        if (vec3_dot(&direction, &normal) > 0) {
            vec3_neg(&normal, &normal);
        }

        float fresnel_add = fresnel(&direction, &normal, ior, mat->ior) * (1 - mat->reflectance);
        float reflectance = mat->reflectance + fresnel_add;
        float transmission = (1 - reflectance) * mat->transmission;
        float diffuse = (1 - mat->reflectance) * (1 - mat->transmission);

        col3_sfma(out, mat->color, diffuse * alpha, out);
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
                    alpha = 0;
                }
            }
        }
        else {
            alpha = 0;
            rr = 0;
        }
    } while ((rr *= .99) > rand2(rand_state));
    return out;
}

Color3* tonemap(Color3 *a, Color3 *out) {
    Color3 temp_c1 = (Color3){};
    Color3 temp_c2 = (Color3){};

    out->r = 0.59719 * a->r + 0.35458 * a->g + 0.04823 * a->b;
    out->g = 0.07600 * a->r + 0.90834 * a->g + 0.01566 * a->b;
    out->b = 0.02840 * a->r + 0.13383 * a->g + 0.83777 * a->b;

    col3_sadd(col3_mul(out, col3_sadd(out, 0.0245786, &temp_c1), &temp_c1), -0.000090537, &temp_c1);
    col3_sadd(col3_mul(out, col3_sadd(col3_smul(out, 0.983729, &temp_c2), 0.4329510, &temp_c2), &temp_c2), 0.238081, &temp_c2);
    col3_div(&temp_c1, &temp_c2, &temp_c1);

    out->r =  1.60475 * temp_c1.r - 0.53108 * temp_c1.g - 0.07367 * temp_c1.b;
    out->g = -0.10208 * temp_c1.r + 1.10813 * temp_c1.g - 0.00605 * temp_c1.b;
    out->b = -0.00327 * temp_c1.r - 0.07276 * temp_c1.g + 1.07602 * temp_c1.b;

    col3_clamp(out, out);
    col3_spow(out, 1 / 2.2, out);
    return out;
}

static inline void render_thread(SceneRenderArgs *args) {
    Scene *self = args->self;
    Camera *camera = args->camera;
    unsigned int thread_i = args->thread_i;
    unsigned int *output = args->output;
    unsigned int rand_state = thread_i;

    Color3 *temp_c = color3(0, 0, 0);
    Color3 *temp_cout = color3(0, 0, 0);
    Vector3 *temp_v1 = vector3(0, 0, 0);
    Vector3 *temp_v2 = vector3(0, 0, 0);
    Ray *temp_r = ray(temp_v1, temp_v2);
    float alpha = 1. / (SCENE_OUTPUT_SAMPLES * SCENE_OUTPUT_SAMPLES);

    int i_start = thread_i * ((float) SCENE_OUTPUT_HEIGHT / SCENE_RENDER_THREADS);
    int i_end = (thread_i + 1) * ((float) SCENE_OUTPUT_HEIGHT / SCENE_RENDER_THREADS);

    for (int i = i_start; i < i_end; i++) {
        for (int j = 0; j < SCENE_OUTPUT_WIDTH; j++) {
            pixels++;
            temp_cout = col3_smul(temp_cout, 0, temp_cout);
            for (int k = 0; k < SCENE_OUTPUT_SAMPLES; k++) {
                for (int l = 0; l < SCENE_OUTPUT_SAMPLES; l++) {
                    camera->get_ray(
                        camera,
                        (j + (l + .5) / SCENE_OUTPUT_SAMPLES) / (SCENE_OUTPUT_WIDTH - 1),
                        (i + (k + .5) / SCENE_OUTPUT_SAMPLES) / (SCENE_OUTPUT_HEIGHT - 1),
                        temp_r
                    );
                    //get_color_iterative(self, temp_r, temp_c);
                    get_color_monte_carlo(self, temp_r, &rand_state, temp_c);
                    col3_sfma(temp_cout, temp_c, alpha, temp_cout);
                }
            }
            *(output + SCENE_OUTPUT_WIDTH * i + j) = col3_to_int(tonemap(temp_cout, temp_cout));
        }
    }
    free(temp_c);
    free(temp_cout);
    free(temp_r);
    free(temp_v1);
    free(temp_v2);
}

unsigned int* render(Scene *self, Camera *camera) {
    pthread_t threads[SCENE_RENDER_THREADS];
    SceneRenderArgs thread_args[SCENE_RENDER_THREADS];
    unsigned int *output = malloc(sizeof(unsigned int *) * SCENE_OUTPUT_HEIGHT * SCENE_OUTPUT_WIDTH);
    assert(output);
    
    for (int i = 0; i < SCENE_RENDER_THREADS; i++) {
        *(thread_args + i) = (SceneRenderArgs){self, camera, i, output};
        pthread_create(threads + i, NULL, (void *)render_thread, thread_args + i);
    }
    for (int i = 0; i < SCENE_RENDER_THREADS; i++) {
        pthread_join(*(threads + i), NULL);
    }

    fprintf(stderr, "%d pixels resulted in %d marches with a total of %d Vector3s and %d Color3s created\n", pixels, marches, vectors, colors);
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