#include "color3.h"
#include "vector3.h"

#ifndef INSTANCE_H
#define INSTANCE_H

typedef struct Instance {
    Vector3 *position;
    Vector3 *size;
} Instance;

Instance* instance();

#endif