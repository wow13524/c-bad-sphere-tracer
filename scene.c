#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include "ray_march_result.h"
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

RayMarchResult* ray_march(Scene *self, Ray *r, float t_min, float t_max) {
    marches++;
    Vector3 *temp = vec3_mul(r->direction, t_min);
    Vector3 *temp2 = NULL;
    Vector3 *position = vec3_add(r->origin, temp);
    free(temp);
    temp = NULL;
    for (int i = 0; i < SCENE_MARCH_ITER_MAX; i++) {
        SDFInstance *closest_instance = map(self, position);
        float closest_distance = closest_instance->get_distance(closest_instance, position);
        temp = vec3_mul(r->direction, closest_distance);
        temp2 = position;
        position = vec3_add(position, temp);
        free(temp);
        free(temp2);
        temp = NULL;
        temp2 = NULL;
         if (closest_distance > t_max) {
            break;
        }
        else if (closest_distance < EPSILON) {
            RayMarchResult *result = ray_march_result();
            result->instance = closest_instance;
            result->position = position;
            return result;
        }
    }
    free(position);
    position = NULL;
    return NULL;
}

Color3* get_light(Scene *self, Vector3 *position, Vector3 *normal) {
    Vector3 *temp = vec3_mul(normal, 2 * EPSILON);
    position = vec3_add(position, temp);
    Color3 *composite_color = COLOR3_ZERO;
    for (int i = 0; i < self->light_count; i++) {
        Light *l = *(self->lights + i);
        Vector3 *offset = vec3_sub(l->instance->position, position);
        Vector3 *offset_unit = vec3_unit(offset);
        Ray *visibility_ray = ray(position, offset_unit);
        if (l->visibility == ALWAYS_VISIBLE || !ray_march(self, visibility_ray, 0, vec3_mag(offset))) {
            float local_brightness = l->get_brightness(l, position);
            if (l->visibility == LINE_OF_SIGHT) {
                local_brightness *= vec3_dot(normal, offset_unit);
            }
            composite_color = col3_add(composite_color, col3_smul(l->instance->color, local_brightness));
        }
        free(offset);
        free(offset_unit);
        free(visibility_ray);
        offset = NULL;
        offset_unit = NULL;
        visibility_ray = NULL;
    }
    return composite_color;
}

Vector3* get_normal(SDFInstance *instance, Vector3 *position) {
    Vector3 *dx = vector3(EPSILON, 0, 0);
    Vector3 *dy = vector3(0, EPSILON, 0);
    Vector3 *dz = vector3(0, 0, EPSILON);
    Vector3 *x_pls = vec3_add(position, dx);
    Vector3 *x_mns = vec3_sub(position, dx);
    Vector3 *y_pls = vec3_add(position, dy);
    Vector3 *y_mns = vec3_sub(position, dy);
    Vector3 *z_pls = vec3_add(position, dz);
    Vector3 *z_mns = vec3_sub(position, dz);
    Vector3 *unnormalized = vector3(
        instance->get_distance(instance, x_pls) - instance->get_distance(instance, x_mns),
        instance->get_distance(instance, y_pls) - instance->get_distance(instance, y_mns),
        instance->get_distance(instance, z_pls) - instance->get_distance(instance, z_mns)
    );
    Vector3 *normal = vec3_unit(unnormalized);
    free(dx);
    free(dy);
    free(dz);
    free(x_pls);
    free(x_mns);
    free(y_pls);
    free(y_mns);
    free(z_pls);
    free(z_mns);
    free(unnormalized);
    dx = NULL;
    dy = NULL;
    dz = NULL;
    x_pls = NULL;
    x_mns = NULL;
    y_pls = NULL;
    y_mns = NULL;
    z_pls = NULL;
    z_mns = NULL;
    return normal;
}

Color3** render(Scene *self, Camera *camera) {
    Color3 **output = malloc(sizeof(Color3 *) * SCENE_OUTPUT_HEIGHT * SCENE_OUTPUT_WIDTH);
    assert(output);
    for (int i = 0; i < SCENE_OUTPUT_HEIGHT; i++) {
        for (int j = 0; j < SCENE_OUTPUT_WIDTH; j++) {
            pixels++;
            Color3 *total_output = COLOR3_ZERO;
            for (int k = 0; k < SCENE_OUTPUT_SAMPLES; k++) {
                for (int l = 0; l < SCENE_OUTPUT_SAMPLES; l++) {
                    Ray* r = camera->get_ray(
                        camera,
                        (j + (l + .5) / SCENE_OUTPUT_SAMPLES) / (SCENE_OUTPUT_WIDTH - 1),
                        (i + (k + .5) / SCENE_OUTPUT_SAMPLES) / (SCENE_OUTPUT_HEIGHT - 1)
                    );
                    RayMarchResult* result = ray_march(
                        self,
                        r,
                        0,
                        FLT_MAX
                    );
                    if (result) {
                        Vector3 *normal = get_normal(result->instance, result->position);
                        Color3 *light_color = get_light(self, result->position, normal);
                        Color3 *instance_color = result->instance->instance->color;
                        instance_color = col3_mul(instance_color, light_color);
                        total_output = col3_add(total_output, instance_color);
                        free(normal);
                        free(light_color);
                        free(instance_color);
                        free(result->position);
                        free(result);
                        normal = NULL;
                        light_color = NULL;
                        instance_color = NULL;
                        result = NULL;
                    }
                    //free(r->origin);  //Perspective camera passes in a pointer to its own position as the origin
                    free(r->direction);
                    free(r);
                    r = NULL;
                }
            }
            total_output = col3_sdiv(total_output, SCENE_OUTPUT_SAMPLES * SCENE_OUTPUT_SAMPLES);
            *(output + SCENE_OUTPUT_WIDTH * i + j) = total_output;
        }
    }
    printf("%d pixels resulted in %d marches", pixels, marches);
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