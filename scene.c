#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
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

SDFInstance* map(Scene *self, Vector3 *position) {
    float closest_distance = FLT_MAX;
    SDFInstance *closest_instance = *self->instances;
    for (int j = 0; j < self->instance_count; j++) {
        SDFInstance *instance = *(self->instances + j);
        float distance = instance->get_distance(instance, position);
        if (distance < closest_distance) {
            closest_distance = distance;
            closest_instance = instance;
        }
    }
    return closest_instance;
}

SDFInstance* ray_march(Scene *self, Ray *r, float t_min, float t_max, Vector3 *out) {
    marches++;
    Vector3 *temp = vec3_mul(r->direction, t_min, vector3(0, 0, 0));
    Vector3 *position = vec3_add(r->origin, temp, out);
    SDFInstance *result = NULL;
    for (int i = 0; i < SCENE_MARCH_ITER_MAX; i++) {
        result = map(self, position);
        float closest_distance = result->get_distance(result, position);
        vec3_mul(r->direction, closest_distance, temp);
        vec3_add(position, temp, position);
         if (closest_distance > t_max) {
            result = NULL;
            break;
        }
        else if (closest_distance < EPSILON) {
            break;
        }
    }
    free(temp);
    return result;
}

Color3* get_light(Scene *self, Vector3 *position, Vector3 *normal, Color3 *out) {
    Color3 *temp_c = color3(0, 0, 0);
    Vector3 *temp_v1 = vec3_mul(normal, 2 * EPSILON, vector3(0, 0, 0));
    Vector3 *temp_v2 = vector3(0, 0, 0);
    position = vec3_add(position, temp_v1, vector3(0, 0, 0));
    Ray *visibility_ray = ray(position, temp_v1);
    col3_smul(out, 0, out);
    for (int i = 0; i < self->light_count; i++) {
        Light *l = *(self->lights + i);
        Vector3 *offset = vec3_sub(l->instance->position, position, temp_v1);
        if (l->visibility == ALWAYS_VISIBLE || !ray_march(self, visibility_ray, 0, vec3_mag(offset), temp_v2)) {
            float local_brightness = l->get_brightness(l, position);
            if (l->visibility == LINE_OF_SIGHT) {
                local_brightness *= vec3_dot(normal, vec3_unit(offset, temp_v1));
            }
            col3_add(out, col3_smul(l->instance->color, local_brightness, temp_c), out);
        }
    }
    free(temp_c);
    free(temp_v1);
    free(temp_v2);
    free(position);
    free(visibility_ray);
    return out;
}

float get_distance_axis(SDFInstance *instance, Vector3 *position, Vector3 *axis) {
    Vector3 *offset = vec3_add(position, axis, vector3(0, 0, 0));
    float distance = instance->get_distance(instance, offset);
    vec3_sub(position, axis, offset);
    distance -= instance->get_distance(instance, offset);
    free(offset);
    return distance;
}

Vector3* get_normal(SDFInstance *instance, Vector3 *position, Vector3 *out) {
    Vector3 *dd = vector3(EPSILON, 0, 0);
    out->x = get_distance_axis(instance, position, dd);
    dd->x = 0;
    dd->y = EPSILON;
    out->y = get_distance_axis(instance, position, dd);
    dd->y = 0;
    dd->z = EPSILON;
    out->z = get_distance_axis(instance, position, dd);
    vec3_unit(out, out);
    free(dd);
    return out;
}

unsigned int* render(Scene *self, Camera *camera) {
    unsigned int *output = malloc(sizeof(unsigned int *) * SCENE_OUTPUT_HEIGHT * SCENE_OUTPUT_WIDTH);
    assert(output);
    Color3 *temp_c = color3(0, 0, 0);
    Color3 *temp_cout = color3(0, 0, 0);
    Vector3 *temp_v1 = vector3(0, 0, 0);
    Vector3 *temp_v2 = vector3(0, 0, 0);
    Vector3 *temp_v3 = vector3(0, 0, 0);
    Ray *temp_r = ray(temp_v1, temp_v2);
    SDFInstance *hit_instance = NULL;
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
                    hit_instance = ray_march(
                        self,
                        temp_r,
                        0,
                        FLT_MAX,
                        temp_v3
                    );
                    if (hit_instance) {
                        Vector3 *normal = get_normal(hit_instance, temp_v3, temp_v1);
                        Color3 *light_color = get_light(self, temp_v3, normal, temp_c);
                        Color3 *instance_color = hit_instance->instance->color;
                        instance_color = col3_mul(instance_color, light_color, temp_c);
                        col3_add(temp_cout, instance_color, temp_cout);
                    }
                }
            }
            col3_sdiv(temp_cout, SCENE_OUTPUT_SAMPLES * SCENE_OUTPUT_SAMPLES, temp_cout);
            *(output + SCENE_OUTPUT_WIDTH * i + j) = col3_to_int(col3_clamp(temp_cout, temp_cout));
        }
    }
    printf("%d pixels resulted in %d marches\n", pixels, marches);
    free(temp_c);
    free(temp_cout);
    free(temp_r);
    free(temp_v1);
    free(temp_v2);
    free(temp_v3);
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