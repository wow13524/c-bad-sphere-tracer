#include "color3.h"

#ifndef MATERIAL_H
#define MATERIAL_H

typedef struct Material {
    Color3 *color;
    float reflectance;
    float transmission;
} Material;

Material* material();

#endif