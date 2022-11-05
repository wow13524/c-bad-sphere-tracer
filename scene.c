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
    *out = *self->instances;
    float closest_distance = (*out)->get_distance(*out, position);
    SDFInstance *instance = NULL;
    for (int j = 1; j < self->instance_count; j++) {
        instance = *(self->instances + j);
        float distance = instance->get_distance(instance, position);
        if (distance < closest_distance) {
            closest_distance = distance;
            *out = instance;
        }
    }
    return fabs(closest_distance);
}

SDFInstance* ray_march(Scene *self, Ray *r, float t_max, Vector3 *out) {
    marches++;
    static Vector3 temp_v = (Vector3){};
    Vector3 *position = vec3_cpy(r->origin, out);
    SDFInstance *result = NULL;
    float total_distance = 0;
    for (int i = 0; i < SCENE_MARCH_ITER_MAX; i++) {
        float closest_distance = map(self, position, &result);
        total_distance += closest_distance;
        vec3_add(position, vec3_mul(r->direction, closest_distance, &temp_v), position);
         if (total_distance > t_max) {
            result = NULL;
            break;
        }
        else if (closest_distance < EPSILON) {
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
    float c = vec3_dot(vec3_neg(normal, &temp_v), direction);
    float s =  1 - r * r * (1 - c * c);
    return s < 0 ? NULL : vec3_add(vec3_mul(direction, r, &temp_v), vec3_mul(normal, r * c - sqrt(s), out), out);
}

float get_distance_axis(SDFInstance *instance, Vector3 *position, Vector3 *axis) {
    static Vector3 offset = (Vector3){};
    float distance = instance->get_distance(instance, vec3_add(position, axis, &offset));
    return distance - instance->get_distance(instance, vec3_sub(position, axis, &offset));
}

Vector3* get_normal(SDFInstance *instance, Vector3 *position, Vector3 *out) {
    static Vector3 dx = (Vector3){EPSILON, 0, 0};
    static Vector3 dy = (Vector3){0, EPSILON, 0};
    static Vector3 dz = (Vector3){0, 0, EPSILON};
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

//TODO turn this from a recursive approach to an iterative approach
//Use pointer as a FILO stack for hit position, instance, etc.
//Hopefully can take advantage of cached data for some sort of speedup?
Color3* get_color(Scene *self, Ray *r, int recursion_depth, float *ior_stack, float alpha, Color3 *out) {
    static Color3 temp_c = (Color3){};
    Vector3 *position = vector3(0, 0, 0);
    SDFInstance *hit_instance = ray_march(
        self,
        r,
        SCENE_MARCH_DIST_MAX,
        position
    );
    if (hit_instance) {
        float reflectance_multiplier = hit_instance->material->reflectance;
        float transmission_multiplier = (1 - reflectance_multiplier) * hit_instance->material->transmission;
        float diffuse_multiplier = (1 - reflectance_multiplier) * (1 - hit_instance->material->transmission);
        int recursion_depth_reached = recursion_depth == SCENE_RECURSION_DEPTH;
        int has_transmission = transmission_multiplier > EPSILON;
        int has_diffuse = diffuse_multiplier > EPSILON;
        int has_reflectance = reflectance_multiplier > EPSILON;
        Color3 *instance_color = hit_instance->material->color;
        Vector3 *normal = get_normal(hit_instance, position, vector3(0, 0, 0));
        Ray *second_ray = has_transmission || has_reflectance ? ray(vector3(0, 0, 0), vector3(0, 0, 0)) : NULL;
        if (vec3_dot(r->direction, normal) > 0) {   //If the ray exited an instance
            vec3_neg(normal, normal);
            ior_stack--;
        }
        //Diffuse contribution
        if (has_diffuse) {
            Color3 *light_color = get_light_color(self, position, normal, &temp_c);
            Color3 *diffuse_color = col3_mul(instance_color, light_color, &temp_c);
            if (hit_instance->material->checker) {
                col3_smul(diffuse_color, ((int)floor(position->x / 2) % 2 + (int)floor(position->y / 2) % 2 + (int)floor(position->z / 2) % 2) % 2 ? 1 : .375, diffuse_color);
            }
            col3_add(out, col3_smul(diffuse_color, alpha * diffuse_multiplier, &temp_c), out);
        }
        if (!recursion_depth_reached) {
            //Transmission contribution
            if (has_transmission) {
                perturb_vector3(position, vec3_neg(normal, second_ray->origin), second_ray->origin);
                if (refract_vector3(r->direction, normal, *ior_stack / hit_instance->material->ior, second_ray->direction)) {
                    *++ior_stack = hit_instance->material->ior;
                    get_color(self, second_ray, recursion_depth + 1, ior_stack, alpha * transmission_multiplier, out);
                }
            }
            //Reflectance contribution
            if (has_reflectance) {
                perturb_vector3(position, normal, second_ray->origin);
                reflect_vector3(r->direction, normal, second_ray->direction);
                get_color(self, second_ray, recursion_depth + 1, ior_stack, alpha * reflectance_multiplier, out);
            }
        }
        free(normal);
        if (second_ray) {
            free(second_ray->origin);
            free(second_ray->direction);
            free(second_ray);
        }
    }
    free(position);
    return out;
}

Color3* get_color_iterative(Scene *self, Ray *r, Color3 *out) {
    static int *depth_stack = NULL;
    static float *ior_stack = NULL;
    static Vector3 **position_stack = NULL;
    static Ray **ray_stack = NULL;
    if (!ior_stack) {   //Initialize static temp variables
        int stack_size = pow(SCENE_RECURSION_DEPTH, 2);
        assert((depth_stack = malloc(sizeof(int) * stack_size)));
        assert((ior_stack = malloc(sizeof(float) * stack_size)));
        assert((position_stack = malloc(sizeof(Vector3 *) * stack_size)));
        assert((ray_stack = malloc(sizeof(Ray *) * stack_size)));
        for (int i = 0; i < stack_size; i++) {
            *(position_stack + i) = vector3(0, 0, 0);
            *(ray_stack + i) = ray(vector3(0, 0, 0), vector3(0, 0, 0));
        }
    }
    *depth_stack = 0;
    *ior_stack = SCENE_AIR_IOR; //TODO correct if ray origin is inside instance
    vec3_cpy(r->origin, (*ray_stack)->origin);
    vec3_cpy(r->direction, (*ray_stack)->direction);
    col3_smul(out, 0, out);
    int offset = 0;
    while (offset >= 0) {
        SDFInstance *hit_instance = ray_march(
            self,
            *(ray_stack + offset),
            SCENE_MARCH_DIST_MAX,
            *(position_stack + offset)
        );
        if (hit_instance) {
            col3_add(out, hit_instance->material->color, out);
        }
        offset--;
    }
    return out;
}

Color3* tonemap(Color3 *c, Color3 *out) {
    float l = 0.299 * c->r + 0.587 * c->g + 0.114 * c->b;
    float c_r = c->r / (1 + c->r);
    float c_g = c->g / (1 + c->g);
    float c_b = c->b / (1 + c->b);
    col3_sdiv(c, 1 + l, out);
    out->r = out->r * (1 - c_r) + c_r * c_r;
    out->g = out->g * (1 - c_g) + c_g * c_g;
    out->b = out->b * (1 - c_b) + c_b * c_b;
    return out;
}

unsigned int* render(Scene *self, Camera *camera) {
    unsigned int *output = malloc(sizeof(unsigned int *) * SCENE_OUTPUT_HEIGHT * SCENE_OUTPUT_WIDTH);
    assert(output);
    //float *ior_stack = malloc(sizeof(float *) * SCENE_RECURSION_DEPTH);
    //assert(ior_stack);
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
                    //*ior_stack = SCENE_AIR_IOR;
                    //col3_add(temp_cout, get_color(self, temp_r, 0, ior_stack, alpha, col3_smul(temp_c, 0, temp_c)), temp_cout);
                    col3_add(temp_cout, col3_smul(get_color_iterative(self, temp_r, temp_c), alpha, temp_c), temp_cout);
                }
            }
            *(output + SCENE_OUTPUT_WIDTH * i + j) = col3_to_int(col3_clamp(tonemap(temp_cout, temp_cout), temp_cout));
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
    Scene* x = malloc(sizeof(Scene));
    assert(x);
    SDFInstance** instances = malloc(sizeof(SDFInstance *) * SCENE_INSTANCES_MAX);
    assert(instances);
    Light** lights = malloc(sizeof(Light *) * SCENE_LIGHTS_MAX);
    assert(lights);
    x->instance_count = 0;
    x->light_count = 0;
    x->instances = instances;
    x->lights = lights;
    x->add_instance = add_instance;
    x->add_light = add_light;
    x->render = render;
    return x;
}