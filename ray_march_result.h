#include "sdf_instance.h"

#ifndef RAY_MARCH_RESULT_H
#define RAY_MARCH_RESULT_H

typedef struct RayMarchResult {
    SDFInstance *instance;
    Vector3 *position;
} RayMarchResult;

RayMarchResult* ray_march_result();

#endif