#include <stdlib.h>
#include <assert.h>
#include <math.h>
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
    out->r = a->r;
    out->g = a->g;
    out->b = a->b;
    return out;
}

Color3* col3_clamp(Color3 *a, Color3 *out) {
    out->r = fmax(0, fmin(a->r, 1));
    out->g = fmax(0, fmin(a->g, 1));
    out->b = fmax(0, fmin(a->b, 1));
    return out;
}

Color3* col3_exp(Color3 *a, Color3 *out) {
    out->r = exp(a->r);
    out->g = exp(a->g);
    out->b = exp(a->b);
    return out;
}

Color3* col3_log(Color3 *a, Color3 *out) {
    out->r = log(a->r);
    out->g = log(a->g);
    out->b = log(a->b);
    return out;
}

Color3* col3_add(Color3 *a, Color3 *b, Color3 *out) {
    out->r = a->r + b->r;
    out->g = a->g + b->g;
    out->b = a->b + b->b;
    return out;
}

Color3* col3_mul(Color3 *a, Color3 *b, Color3 *out) {
    out->r = a->r * b->r;
    out->g = a->g * b->g;
    out->b = a->b * b->b;
    return out;
}

Color3* col3_smul(Color3 *a, float c, Color3 *out) {
    out->r = c * a->r;
    out->g = c * a->g;
    out->b = c * a->b;
    return out;
}

Color3* col3_sdiv(Color3 *a, float c, Color3 *out) {
    out->r = a->r / c;
    out->g = a->g / c;
    out->b = a->b / c;
    return out;
}