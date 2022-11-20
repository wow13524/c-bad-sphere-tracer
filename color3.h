#include <arm_neon.h>

#ifndef COLOR3_H
#define COLOR3_H

int colors;

typedef struct Color3 {
    float32_t r;
    float32_t g;
    float32_t b;
    float32_t _;
} Color3;

Color3* color3(float r, float g, float b);
unsigned int col3_to_int(Color3 *a);
Color3* col3_cpy(Color3 *a, Color3 *out);
Color3* col3_clamp(Color3 *a, Color3 *out);
Color3* col3_exp(Color3 *a, Color3 *out);
Color3* col3_log(Color3 *a, Color3 *out);
Color3* col3_add(Color3 *a, Color3 *b, Color3 *out);
Color3* col3_mul(Color3 *a, Color3 *b, Color3 *out);
Color3* col3_div(Color3 *a, Color3 *b, Color3 *out);
Color3* col3_sadd(Color3 *a, float c, Color3 *out);
Color3* col3_smul(Color3 *a, float c, Color3 *out);
Color3* col3_sdiv(Color3 *a, float c, Color3 *out);
Color3* col3_spow(Color3 *a, float c, Color3 *out);
Color3* col3_lerp(Color3 *a, Color3 *b, float c, Color3 *out);

#endif