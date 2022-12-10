#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "color3.h"

int colors = 0;

Color3* color3(float r, float g, float b) {
    colors++;
    Color3* a = malloc(sizeof(Color3));
    assert(a);
    a->r = r;
    a->g = g;
    a->b = b;
    return a;
}

unsigned int col3_to_int(Color3 *a) {
    return ((int)(255 * a->r) << 16) + ((int)(255 * a->g) << 8) + (int)(255 * a->b);
}

Color3* col3_cpy(Color3 *a, Color3 *out) {
    /*vst1q_f32(
        (float32_t *)out,
        *((float32x4_t *)a)
    );*/
    memcpy(out, a, sizeof(Color3));
    return out;
}

Color3* col3_clamp(Color3 *a, Color3 *out) {
    static float32x4_t zero = (float32x4_t){0,0,0,0};
    static float32x4_t one = (float32x4_t){1,1,1,1};
    vst1q_f32(
        (float32_t *)out,
        vmaxq_f32(
            zero,
            vminq_f32(
                *((float32x4_t *)a),
                one
            )
        )
    );
    return out;
}

Color3* col3_exp(Color3 *a, Color3 *out) {
    out->r = expf(a->r);
    out->g = expf(a->g);
    out->b = expf(a->b);
    return out;
}

Color3* col3_log(Color3 *a, Color3 *out) {
    out->r = logf(a->r);
    out->g = logf(a->g);
    out->b = logf(a->b);
    return out;
}

Color3* col3_add(Color3 *a, Color3 *b, Color3 *out) {
    vst1q_f32(
        (float32_t *)out,
        vaddq_f32(
            vld1q_f32((float32_t *)a),
            vld1q_f32((float32_t *)b)
        )
    );
    return out;
}

Color3* col3_mul(Color3 *a, Color3 *b, Color3 *out) {
    vst1q_f32(
        (float32_t *)out,
        vmulq_f32(
            vld1q_f32((float32_t *)a),
            vld1q_f32((float32_t *)b)
        )
    );
    return out;
}

Color3* col3_div(Color3 *a, Color3 *b, Color3 *out) {
    vst1q_f32(
        (float32_t *)out,
        vmulq_f32(
            vld1q_f32((float32_t *)a),
            vrecpeq_f32(vld1q_f32((float32_t *)b))
        )
    );
    return out;
}

Color3* col3_sadd(Color3 *a, float c, Color3 *out) {    //vaddq_n_f32 not defined :(
    out->r = a->r + c;
    out->g = a->g + c;
    out->b = a->b + c;
    return out;
}

Color3* col3_smul(Color3 *a, float c, Color3 *out) {
    vst1q_f32(
        (float32_t *)out,
        vmulq_n_f32(
            vld1q_f32((float32_t *)a),
            c
        )
    );
    return out;
}

Color3* col3_sdiv(Color3 *a, float c, Color3 *out) {
    return col3_smul(a, 1 / c, out);
}
Color3* col3_spow(Color3 *a, float c, Color3 *out) {
    out->r = powf(a->r, c);
    out->g = powf(a->g, c);
    out->b = powf(a->b, c);
    return out;
}

Color3* col3_lerp(Color3 *a, Color3 *b, float c, Color3 *out) {
    float32x4_t fa = vld1q_f32((float32_t *)a);
    float32x4_t fb = vld1q_f32((float32_t *)b);
    vst1q_f32(
        (float32_t *)out,
        vaddq_f32(
            fa,
            vmulq_n_f32(
                vsubq_f32(fb, fa),
                c
            )
        )
    );
    return out;
}