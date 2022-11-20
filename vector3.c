#include <stdlib.h>
#include <assert.h>
#include <math.h>
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

float vec3_mag2(Vector3 *a) {
    return vec3_dot(a, a);
}

float vec3_mag(Vector3 *a) {
    return sqrtf(vec3_mag2(a));
}

float vec3_dot(Vector3 *a, Vector3 *b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

Vector3* vec3_cpy(Vector3 *a, Vector3 *out) {
    vst1q_f32(
        (float32_t *)out,
        vld1q_f32((float32_t *)a)
    );
    return out;
}

Vector3 *vec3_neg(Vector3 *a, Vector3 *out){
    vst1q_f32(
        (float32_t *)out,
        vnegq_f32(
            vld1q_f32((float32_t *)a)
        )
    );
    return out;
}

Vector3* vec3_unit(Vector3 *a, Vector3 *out) {
    return vec3_div(a, vec3_mag(a), out);
}

Vector3* vec3_add(Vector3 *a, Vector3 *b, Vector3 *out) {
    vst1q_f32(
        (float32_t *)out,
        vaddq_f32(
            vld1q_f32((float32_t *)a),
            vld1q_f32((float32_t *)b)
        )
    );
    return out;
}

Vector3* vec3_sub(Vector3 *a, Vector3 *b, Vector3 *out) {
    vst1q_f32(
        (float32_t *)out,
        vsubq_f32(
            vld1q_f32((float32_t *)a),
            vld1q_f32((float32_t *)b)
        )
    );
    return out;
}

Vector3* vec3_mul(Vector3 *a, float c, Vector3 *out) {
    vst1q_f32(
        (float32_t *)out,
        vmulq_n_f32(
            vld1q_f32((float32_t *)a),
            c
        )
    );
    return out;
}

Vector3* vec3_div(Vector3 *a, float c, Vector3 *out) {
    return vec3_mul(a, 1 / c, out);
}