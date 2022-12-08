#include "instance.h"
#include "material.h"

#ifndef SDF_INSTANCE_H
#define SDF_INSTANCE_H

typedef struct SDFInstance {
    Instance *instance;
    Material *material;
    float (*get_distance)(struct SDFInstance *self, float32x4_t position);
} SDFInstance;

typedef float (*distance_function_t)(SDFInstance *self, float32x4_t position);

SDFInstance* sdf_instance(distance_function_t distance_function);
distance_function_t cube;
distance_function_t plane;
distance_function_t sphere;

#endif