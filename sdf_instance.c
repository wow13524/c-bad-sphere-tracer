#include <stdlib.h>
#include <assert.h>
#include "sdf_instance.h"

SDFInstance* sdf_instance(distance_function_t distance_function) {
    SDFInstance *x = malloc(sizeof(SDFInstance));
    assert(x);
    x->instance = instance();
    x->get_distance = distance_function;
    return x;
}

float distance_plane(SDFInstance *self, Vector3 *position) {
    return position->y - self->instance->position->y;
}

float distance_sphere(SDFInstance *self, Vector3 *position) {
    Vector3 *offset = vec3_sub(self->instance->position, position);
    float distance = vec3_mag(offset);
    free(offset);
    offset = NULL;
    return distance - self->instance->size->x / 2;
}

distance_function_t plane = distance_plane;
distance_function_t sphere = distance_sphere;