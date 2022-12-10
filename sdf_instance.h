#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "instance.h"
#include "material.h"

#ifndef SDF_INSTANCE_H
#define SDF_INSTANCE_H

typedef struct SDFInstance {
    Instance *instance;
    Material *material;
    float (*get_distance)(struct SDFInstance *self, Vector3 *position);
} SDFInstance;

typedef float (*distance_function_t)(SDFInstance *self, Vector3 *position);

SDFInstance* sdf_instance(distance_function_t distance_function);
distance_function_t cube;
distance_function_t plane;
distance_function_t sphere;

#endif