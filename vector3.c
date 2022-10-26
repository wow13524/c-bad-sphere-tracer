#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "vector3.h"

Vector3 *VECTOR3_ZERO = &(Vector3){};

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

Vector3* vec3_neg(Vector3 *a) {
    return vector3(
        -a->x,
        -a->y,
        -a->z
    );
}

Vector3* vec3_unit(Vector3 *a) {
    return vec3_div(a, vec3_mag(a));
}

Vector3* vec3_add(Vector3 *a, Vector3 *b) {
    return vector3(
        a->x + b->x,
        a->y + b->y,
        a->z + b->z
    );
}

Vector3* vec3_sub(Vector3 *a, Vector3 *b) {
    return vector3(
        a->x - b->x,
        a->y - b->y,
        a->z - b->z
    );
}

Vector3* vec3_mul(Vector3 *a, float c) {
    return vector3(
        c * a->x,
        c * a->y,
        c * a->z
    );
}

Vector3* vec3_div(Vector3 *a, float c) {
    return vec3_mul(a, 1 / c);
}