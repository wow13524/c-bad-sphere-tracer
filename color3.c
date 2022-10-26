#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "color3.h"

Color3 *COLOR3_ZERO = &(Color3){};

Color3* color3(float r, float g, float b) {
    Color3* a = malloc(sizeof(Color3));
    assert(a);
    a->r = r;
    a->g = g;
    a->b = b;
    return a;
}

int col3_to_int(Color3 *a) {
    return ((int)(255 * a->r) << 16) + ((int)(255 * a->g) << 8) + (int)(255 * a->b);
}

Color3* col3_clamp(Color3 *a) {
    return color3(
        fmax(0, fmin(a->r, 1)),
        fmax(0, fmin(a->g, 1)),
        fmax(0, fmin(a->b, 1))
    );
}

Color3* col3_add(Color3 *a, Color3 *b) {
    return color3(
        a->r + b->r,
        a->g + b->g,
        a->b + b->b
    );
}

Color3* col3_mul(Color3 *a, Color3 *b) {
    return color3(
        a->r * b->r,
        a->g * b->g,
        a->b * b->b
    );
}

Color3* col3_smul(Color3 *a, float c) {
    return color3(
        c * a->r,
        c * a->g,
        c * a->b
    );
}

Color3* col3_sdiv(Color3 *a, float c) {
    return color3(
        a->r / c,
        a->g / c,
        a->b / c
    );
}