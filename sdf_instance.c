#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
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
    float32x4_t dst = vsubq_f32(
        vabsq_f32(vsubq_f32(
            vld1q_f32((float32_t *)self->instance->position),
            vld1q_f32((float32_t *)position)
        )),
        vmulq_n_f32(
            vld1q_f32((float32_t *)self->instance->size),
            .5
        )
    );
    Vector3 *off = (Vector3 *)(&dst);
    return fmaxf(fmaxf(off->x, off->y), off->z);
}

float distance_plane(SDFInstance *self, Vector3 *position) {
    return position->y - self->instance->position->y;
}

float distance_sphere(SDFInstance *self, Vector3 *position) {
    static Vector3 temp = (Vector3){};
    return vec3_mag(vec3_sub(self->instance->position, position, &temp)) - self->instance->size->x / 2;
}

distance_function_t cube = distance_cube;
distance_function_t plane = distance_plane;
distance_function_t sphere = distance_sphere;