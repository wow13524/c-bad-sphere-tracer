#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include "scene.h"

int pixels = 0;
int marches = 0;

void add_instance(Scene *self, SDFInstance *instance) {
    assert(self->instance_count < SCENE_INSTANCES_MAX);
    *(self->instances + self->instance_count++) = instance;
}

void add_light(Scene *self, Light *light) {
    assert(self->light_count < SCENE_LIGHTS_MAX);
    *(self->lights + self->light_count++) = light;
}

float map(Scene *self, Vector3 *position, SDFInstance **out) {
    if (!self->instance_count) {
        *out = NULL;
        return FLT_MAX;
    }
    *out = *self->instances;
    float closest_distance = (*out)->get_distance(*out, position);
    for (int j = 1; j < self->instance_count; j++) {
        SDFInstance *instance = *(self->instances + j);
        float distance = instance->get_distance(instance, position);
        if (distance < closest_distance) {
            closest_distance = distance;
            *out = instance;
            if (distance < EPSILON) {
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
    static Vector3 temp_v = (Vector3){};
    SDFInstance *result = NULL;
    SDFInstance *temp = NULL;
    int direction = is_inside_instance(self, r->origin, &temp) ? -1 : 1;
    float omega = 1.75; //Over-relaxation factor [1, 2)
    float last_radius = 0;
    float total_distance = 0;
    float next_step = 0;
    for (int i = 0; i < SCENE_MARCH_ITER_MAX; i++) {
        float radius = direction * map(self, vec3_add(r->origin, vec3_mul(r->direction, total_distance, &temp_v), out), &result);
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
        else if (abs_radius < EPSILON) {
            break;
        }

    }
    return result;
}

Vector3* perturb_vector3(Vector3 *position, Vector3* offset, Vector3* out) {
    return vec3_add(position, vec3_mul(offset, 2 * EPSILON, out), out);
}

Vector3* reflect_vector3(Vector3 *direction, Vector3 *normal, Vector3 *out) {
    return vec3_sub(direction, vec3_mul(normal, 2 * vec3_dot(direction, normal), out), out);
}

Vector3* refract_vector3(Vector3 *direction, Vector3 *normal, float r, Vector3 *out) {
    static Vector3 temp_v = (Vector3){};
    float c = vec3_dot(normal, direction);
    float s =  1 - r * r * (1 - c * c);
    if (s < 0) {
        return NULL;
    } 
    else if(c > 0) {    //Correct for when ray is leaving an object
        return vec3_sub(vec3_mul(direction, r, &temp_v), vec3_mul(normal, r * c - sqrtf(s), out), out);
    }
    else {
        return vec3_add(vec3_mul(direction, r, &temp_v), vec3_mul(normal, r * -c - sqrtf(s), out), out);
    }
}

float fresnel(Vector3 *direction, Vector3 *normal, float ior_in, float ior_out) {
    float c = 1 - fabsf(vec3_dot(direction, normal));
    float r = (ior_in - ior_out) / (ior_in + ior_out);
    r *= r;
    return r + (1 - r) * c * c * c * c * c;
    
}

float get_distance_axis(SDFInstance *instance, Vector3 *position, Vector3 *axis) {
    static Vector3 offset = (Vector3){};
    return instance->get_distance(instance, vec3_add(position, axis, &offset))
         - instance->get_distance(instance, vec3_sub(position, axis, &offset));
}

Vector3* get_normal(SDFInstance *instance, Vector3 *position, Vector3 *out) {
    static Vector3 dx = (Vector3){EPSILON, 0, 0, 0};
    static Vector3 dy = (Vector3){0, EPSILON, 0, 0};
    static Vector3 dz = (Vector3){0, 0, EPSILON, 0};
    out->x = get_distance_axis(instance, position, &dx);
    out->y = get_distance_axis(instance, position, &dy);
    out->z = get_distance_axis(instance, position, &dz);
    return vec3_unit(out, out);
}

//TODO take instance transmission into account
Color3* get_light_color(Scene *self, Vector3 *position, Vector3 *normal, Color3 *out) {
    static Color3 temp_c = (Color3){};
    static Vector3 temp_v1 = (Vector3){};
    static Vector3 temp_v2 = (Vector3){};
    static Vector3 temp_v3 = (Vector3){};
    static Ray temp_r = (Ray){&temp_v3, &temp_v1};
    perturb_vector3(position, normal, &temp_v3);
    col3_smul(out, 0, out);
    for (int i = 0; i < self->light_count; i++) {
        Light *l = *(self->lights + i);
        Vector3 *offset = vec3_sub(l->instance->position, position, &temp_v1);
        float offset_mag = 0;
        if (l->visibility == ALWAYS_VISIBLE || ((offset_mag = vec3_mag(offset)) && vec3_unit(offset, offset) && !ray_march(self, &temp_r, offset_mag, &temp_v2))) {
            float local_brightness = l->get_brightness(l, position);
            if (l->visibility == LINE_OF_SIGHT) {
                local_brightness *= vec3_dot(normal, vec3_unit(offset, &temp_v1));
            }
            col3_add(out, col3_smul(l->color, local_brightness, &temp_c), out);
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
    static Color3 *light_color = NULL;
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
        light_color = color3(0, 0, 0);
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
                get_light_color(self, curr_position, curr_normal, light_color);
                if (hit_instance->material->checker) {
                    diffuse_alpha *= ((int)floorf(curr_position->x / 2) % 2 + (int)floorf(curr_position->y / 2) % 2 + (int)floorf(curr_position->z / 2) % 2) % 2 ? 1 : .375;
                }
                col3_add(out, col3_smul(col3_mul(col3_mul(light_color, hit_instance->material->color, temp_c), curr_attenuation, temp_c), diffuse_alpha, temp_c), out);
            }

            if (curr_depth < SCENE_RECURSION_DEPTH) {
                if (transmission_alpha > SCENE_ALPHA_MIN) {
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
            col3_add(out, col3_smul(col3_mul(self->environment->sample(self->environment, curr_ray->direction, temp_c), curr_attenuation, temp_c), curr_alpha, temp_c), out);
        }
    } while (++offset < total);

    return out;
}

Color3* tonemap(Color3 *a, Color3 *out) {
    static Color3 temp_c1 = (Color3){};
    static Color3 temp_c2 = (Color3){};

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

unsigned int* render(Scene *self, Camera *camera) {
    unsigned int *output = malloc(sizeof(unsigned int *) * SCENE_OUTPUT_HEIGHT * SCENE_OUTPUT_WIDTH);
    assert(output);
    Color3 *temp_c = color3(0, 0, 0);
    Color3 *temp_cout = color3(0, 0, 0);
    Vector3 *temp_v1 = vector3(0, 0, 0);
    Vector3 *temp_v2 = vector3(0, 0, 0);
    Ray *temp_r = ray(temp_v1, temp_v2);
    float alpha = 1. / (SCENE_OUTPUT_SAMPLES * SCENE_OUTPUT_SAMPLES);
    for (int i = 0; i < SCENE_OUTPUT_HEIGHT; i++) {
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
                    get_color_iterative(self, temp_r, temp_c);
                    col3_smul(temp_c, alpha, temp_c);
                    col3_add(temp_cout, temp_c, temp_cout);
                }
            }
            *(output + SCENE_OUTPUT_WIDTH * i + j) = col3_to_int(tonemap(temp_cout, temp_cout));
        }
    }
    fprintf(stderr, "%d pixels resulted in %d marches with a total of %d Vector3s and %d Color3s created\n", pixels, marches, vectors, colors);
    free(temp_c);
    free(temp_cout);
    free(temp_r);
    free(temp_v1);
    free(temp_v2);
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