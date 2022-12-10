#include "vector3.h"

int vectors = 0;

Vector3* vector3(float x, float y, float z) {
    vectors++;
    Vector3* a = malloc(sizeof(Vector3));
    assert(a);
    a->x = x;
    a->y = y;
    a->z = z;
    return a;
}