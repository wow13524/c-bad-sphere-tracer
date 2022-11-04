#include "color3.h"

#ifndef MATERIAL_H
#define MATERIAL_H

typedef struct Material {
    Color3 *color;
    float ior;
    float reflectance;
    float transmission;
    int checker;
} Material;

Material* material();

#endif