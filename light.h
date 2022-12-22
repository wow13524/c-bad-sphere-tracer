#include <assert.h>
#include <stdlib.h>
#include "color3.h"
#include "instance.h"

#ifndef LIGHT_H
#define LIGHT_H

enum LightVisibility {
    ALWAYS_VISIBLE,
    LINE_OF_SIGHT
};

typedef struct Light {
    Instance *instance;
    Color3 *color;
    enum LightVisibility visibility;
    float (*get_brightness)(struct Light *self, Vector3 *position);
} Light;

typedef float (*brightness_function_t)(Light *self, Vector3 *position);

extern Light *light(brightness_function_t brightness_function, enum LightVisibility visibility);
extern brightness_function_t ambient_light;
extern brightness_function_t point_light;

#endif