#include <assert.h>
#include <stdlib.h>
#include "color3.h"

#ifndef MATERIAL_H
#define MATERIAL_H

typedef struct Material {
    Color3 *color;
    float ior;
    float reflectance;
    float roughness;
    float transmission;
    int checker;
} Material;

extern Material *material();

#endif