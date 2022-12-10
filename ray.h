#include <assert.h>
#include <stdlib.h>
#include "vector3.h"

#ifndef RAY_H
#define RAY_H

typedef struct Ray {
    Vector3* origin;
    Vector3* direction;
} Ray;

Ray* ray(Vector3 *origin, Vector3 *direction);

#endif