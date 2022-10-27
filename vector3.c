#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "vector3.h"

Vector3* vector3(float x, float y, float z) {
    Vector3* a = malloc(sizeof(Vector3));
    assert(a);
    a->x = x;
    a->y = y;
    a->z = z;
    return a;
}

float vec3_mag2(Vector3 *a) {
    return a->x * a->x + a->y * a->y + a->z * a->z;
}

float vec3_mag(Vector3 *a) {
    return sqrt(vec3_mag2(a));
}

float vec3_dot(Vector3 *a, Vector3 *b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

Vector3* vec3_unit(Vector3 *a, Vector3 *out) {
    return vec3_div(a, vec3_mag(a), out);
}

Vector3* vec3_add(Vector3 *a, Vector3 *b, Vector3 *out) {
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    return out;
}

Vector3* vec3_sub(Vector3 *a, Vector3 *b, Vector3 *out) {
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    return out;
}

Vector3* vec3_mul(Vector3 *a, float c, Vector3 *out) {
    out->x = c * a->x;
    out->y = c * a->y;
    out->z = c * a->z;
    return out;
}

Vector3* vec3_div(Vector3 *a, float c, Vector3 *out) {
    return vec3_mul(a, 1 / c, out);
}