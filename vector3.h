#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef VECTOR3_H
#define VECTOR3_H

typedef struct Vector3 {
    float x;
    float y;
    float z;
} Vector3;

extern Vector3 *vector3(float x, float y, float z);

extern Vector3 *X_AXIS;
extern Vector3 *Y_AXIS;
extern Vector3 *Z_AXIS;

__attribute__((always_inline)) inline Vector3 *vec3_cross(Vector3 *a, Vector3 *b, Vector3 *out) {
    out->x = a->y * b->z - b->y * a->z;
    out->y = a->z * b->x - b->z * a->x;
    out->z = a->x * b->y - b->x * a->y;
    return out;
}

__attribute__((always_inline)) inline float vec3_dot(Vector3 *a, Vector3 *b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

__attribute__((always_inline)) inline float vec3_mag2(Vector3 *a) {
    return vec3_dot(a, a);
}

__attribute__((always_inline)) inline float vec3_mag(Vector3 *a) {
    return sqrtf(vec3_mag2(a));
}

__attribute__((always_inline)) inline Vector3 *vec3_cpy(Vector3 *a, Vector3 *out) {
    memcpy(out, a, sizeof(Vector3));
    return out;
}

__attribute__((always_inline)) inline Vector3 *vec3_abs(Vector3 *a, Vector3 *out){
    out->x = fabsf(a->x);
    out->y = fabsf(a->y);
    out->z = fabsf(a->z);
    return out;
}

__attribute__((always_inline)) inline Vector3 *vec3_neg(Vector3 *a, Vector3 *out){
    out->x = -a->x;
    out->y = -a->y;
    out->z = -a->z;
    return out;
}

__attribute__((always_inline)) inline Vector3 *vec3_add(Vector3 *a, Vector3 *b, Vector3 *out) {
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    return out;
}

__attribute__((always_inline)) inline Vector3 *vec3_sub(Vector3 *a, Vector3 *b, Vector3 *out) {
    out->x = a->x - b->x;
    out->y = a->y - b->y;
    out->z = a->z - b->z;
    return out;
}

__attribute__((always_inline)) inline Vector3 *vec3_mul(Vector3 *a, float c, Vector3 *out) {
    out->x = a->x * c;
    out->y = a->y * c;
    out->z = a->z * c;
    return out;
}

__attribute__((always_inline)) inline Vector3 *vec3_fma(Vector3 *a, Vector3 *b, float c, Vector3 *out) {
    out->x = fmaf(b->x, c, a->x);
    out->y = fmaf(b->y, c, a->y);
    out->z = fmaf(b->z, c, a->z);
    return out;
}

__attribute__((always_inline)) inline Vector3 *vec3_div(Vector3 *a, float c, Vector3 *out) {
    out->x = a->x / c;
    out->y = a->y / c;
    out->z = a->z / c;
    return out;
}

__attribute__((always_inline)) inline Vector3 *vec3_unit(Vector3 *a, Vector3 *out) {
    return vec3_div(a, vec3_mag(a), out);
}

__attribute__((always_inline)) inline Vector3 *vec3_proj(Vector3 *a, Vector3 *b, Vector3 *out) {
    return vec3_mul(b, vec3_dot(a, b) / vec3_mag2(b), out);
}

#endif