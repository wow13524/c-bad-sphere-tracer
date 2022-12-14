#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef COLOR3_H
#define COLOR3_H

typedef struct Color3 {
    float r;
    float g;
    float b;
} Color3;

extern Color3 *color3(float r, float g, float b);
extern unsigned int col3_to_int(Color3 *a);
extern Color3 *col3_cpy(Color3 *a, Color3 *out);
extern Color3 *col3_clamp(Color3 *a, Color3 *out);
extern Color3 *col3_exp(Color3 *a, Color3 *out);
extern Color3 *col3_log(Color3 *a, Color3 *out);
extern Color3 *col3_add(Color3 *a, Color3 *b, Color3 *out);
extern Color3 *col3_mul(Color3 *a, Color3 *b, Color3 *out);
extern Color3 *col3_div(Color3 *a, Color3 *b, Color3 *out);
extern Color3 *col3_sadd(Color3 *a, float c, Color3 *out);
extern Color3 *col3_smul(Color3 *a, float c, Color3 *out);
extern Color3 *col3_sfma(Color3 *a, Color3 *b, float c, Color3 *out);
extern Color3 *col3_sdiv(Color3 *a, float c, Color3 *out);
extern Color3 *col3_spow(Color3 *a, float c, Color3 *out);
extern Color3 *col3_lerp(Color3 *a, Color3 *b, float c, Color3 *out);

#endif