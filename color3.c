#include "color3.h"

Color3* color3(float r, float g, float b) {
    Color3* a = malloc(sizeof(Color3));
    assert(a);
    a->r = r;
    a->g = g;
    a->b = b;
    return a;
}

unsigned int col3_to_int(Color3 *a) {
    return    ((int)(255 * a->r) << 16)
            + ((int)(255 * a->g) << 8)
            + (int)(255 * a->b);
}

Color3* col3_cpy(Color3 *a, Color3 *out) {
    memcpy(out, a, sizeof(Color3));
    return out;
}

Color3* col3_clamp(Color3 *a, Color3 *out) {
    out->r = fmaxf(0, fminf(a->r, 1));
    out->g = fmaxf(0, fminf(a->g, 1));
    out->b = fmaxf(0, fminf(a->b, 1));
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

Color3* col3_div(Color3 *a, Color3 *b, Color3 *out) {
    out->r = a->r / b->r;
    out->g = a->g / b->g;
    out->b = a->b / b->b;
    return out;
}

Color3* col3_sadd(Color3 *a, float c, Color3 *out) {
    out->r = a->r + c;
    out->g = a->g + c;
    out->b = a->b + c;
    return out;
}

Color3* col3_smul(Color3 *a, float c, Color3 *out) {
    out->r = a->r * c;
    out->g = a->g * c;
    out->b = a->b * c;
    return out;
}

Color3* col3_sfma(Color3 *a, Color3 *b, float c, Color3 *out) {
    out->r = fmaf(b->r, c, a->r);
    out->g = fmaf(b->g, c, a->g);
    out->b = fmaf(b->b, c, a->b);
    return out;
}

Color3* col3_sdiv(Color3 *a, float c, Color3 *out) {
    out->r = a->r / c;
    out->g = a->g / c;
    out->b = a->b / c;
    return out;
}
Color3* col3_spow(Color3 *a, float c, Color3 *out) {
    out->r = powf(a->r, c);
    out->g = powf(a->g, c);
    out->b = powf(a->b, c);
    return out;
}

Color3* col3_lerp(Color3 *a, Color3 *b, float c, Color3 *out) {
    out->r = a->r + c * (b->r - a->r);
    out->g = a->g + c * (b->g - a->g);
    out->b = a->b + c * (b->b - a->b);
    return out;
}