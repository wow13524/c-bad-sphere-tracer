#include "light.h"

Light* light(brightness_function_t brightness_function, enum LightVisibility visibility) {
    Light *x = malloc(sizeof(Light));
    assert(x);
    x->instance = instance();
    x->color = color3(1, 1, 1);
    x->visibility = visibility;
    x->get_brightness = brightness_function;
    return x;
}

float brightness_ambient_light(Light *self, Vector3 *position) {
    (void) position;
    return self->instance->size->x;
}

float brightness_point_light(Light *self, Vector3 *position) {
    Vector3 temp = (Vector3){};
    return self->instance->size->x
        / vec3_mag2(vec3_sub(self->instance->position, position, &temp));
}

brightness_function_t ambient_light = brightness_ambient_light;
brightness_function_t point_light = brightness_point_light;