#include "vector3.h"

Vector3* vector3(float x, float y, float z) {
    Vector3* a = malloc(sizeof(Vector3));
    assert(a);
    a->x = x;
    a->y = y;
    a->z = z;
    return a;
}

Vector3 *X_AXIS = &(Vector3){1, 0, 0};
Vector3 *Y_AXIS = &(Vector3){0, 1, 0};
Vector3 *Z_AXIS = &(Vector3){0, 0, 1};