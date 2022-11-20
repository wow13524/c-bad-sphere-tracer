#include <arm_neon.h>

#ifndef VECTOR3_H
#define VECTOR3_H

int vectors;

typedef struct Vector3 {
    float32_t x;
    float32_t y;
    float32_t z;
    float32_t _;
} Vector3;

Vector3* vector3(float x, float y, float z);
float vec3_mag2(Vector3 *a);
float vec3_mag(Vector3 *a);
float vec3_dot(Vector3 *a, Vector3 *b);
Vector3* vec3_cpy(Vector3 *a, Vector3 *out);
Vector3* vec3_neg(Vector3 *a, Vector3 *out);
Vector3* vec3_unit(Vector3 *a, Vector3 *out);
Vector3* vec3_add(Vector3 *a, Vector3 *b, Vector3 *out);
Vector3* vec3_sub(Vector3 *a, Vector3 *b, Vector3 *out);
Vector3* vec3_mul(Vector3 *a, float c, Vector3 *out);
Vector3* vec3_div(Vector3 *a, float c, Vector3 *out);

#endif