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

float distance_cube(SDFInstance *self, float32x4_t position) {
    float32x4_t dst = vsubq_f32(
        vabsq_f32(vsubq_f32(
            self->instance->_position,
            position
        )),
        vmulq_n_f32(
            self->instance->_size,
            .5
        )
    );
    Vector3 *off = (Vector3 *)(&dst);
    return fmaxf(fmaxf(off->x, off->y), off->z);
}

float distance_plane(SDFInstance *self, float32x4_t position) {
    return vgetq_lane_f32(position, 1) - vgetq_lane_f32(self->instance->_position, 1);
}

float distance_sphere(SDFInstance *self, float32x4_t position) {
    float32x4_t vdst = vsubq_f32(self->instance->_position, position);
    vdst = vmulq_f32(vdst, vdst);
    return sqrtf(
          vgetq_lane_f32(vdst, 0)
        + vgetq_lane_f32(vdst, 1)
        + vgetq_lane_f32(vdst, 2)
    ) - vgetq_lane_f32(self->instance->_size, 0) / 2;
}

distance_function_t cube = distance_cube;
distance_function_t plane = distance_plane;
distance_function_t sphere = distance_sphere;