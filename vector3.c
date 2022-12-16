#include "vector3.h"

Vector3* vector3(float x, float y, float z) {
    Vector3* a = malloc(sizeof(Vector3));
    assert(a);
    a->x = x;
    a->y = y;
    a->z = z;
    return a;
}