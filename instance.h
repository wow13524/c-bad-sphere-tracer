#include <arm_neon.h>
#include "color3.h"
#include "vector3.h"

#ifndef INSTANCE_H
#define INSTANCE_H

typedef struct Instance {
    Vector3 *position;
    Vector3 *size;

    float32x4_t _position;
    float32x4_t _size;
} Instance;

Instance* instance();
void refresh_instance(Instance *x);

#endif