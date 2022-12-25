#include "sdf_instance.h"

SDFInstance* sdf_instance(distance_function_t distance_function) {
    SDFInstance *x = malloc(sizeof(SDFInstance));
    assert(x);
    x->instance = instance();
    x->material = material();
    x->get_distance = distance_function;
    return x;
}

float distance_cube(SDFInstance *self, Vector3 *position) {
    Vector3 temp_v1, temp_v2;
    vec3_sub(self->instance->position, position, &temp_v1);
    vec3_abs(&temp_v1, &temp_v1);
    vec3_div(self->instance->size, 2, &temp_v2);
    vec3_sub(&temp_v1, &temp_v2, &temp_v1);
    return fmaxf(
        temp_v1.x,
        fmaxf(
            temp_v1.y,
            temp_v1.z
        )
    );
}

float distance_plane(SDFInstance *self, Vector3 *position) {
    return position->y - self->instance->position->y;
}

float distance_sphere(SDFInstance *self, Vector3 *position) {
    Vector3 temp_v;
    return vec3_mag(vec3_sub(self->instance->position, position, &temp_v)) - self->instance->size->x / 2;
}

distance_function_t cube = distance_cube;
distance_function_t plane = distance_plane;
distance_function_t sphere = distance_sphere;