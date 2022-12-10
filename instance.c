#include "instance.h"

Instance* instance() {
    Instance* x = malloc(sizeof(Instance));
    assert(x);
    x->position = vector3(0, 0, 0);
    x->size = vector3(1, 1, 1);
    return x;
}