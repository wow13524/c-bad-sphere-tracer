#include <stdlib.h>
#include <assert.h>
#include "instance.h"

Instance* instance() {
    Instance* x = malloc(sizeof(Instance));
    assert(x);
    x->position = VECTOR3_ZERO;
    x->size = vector3(1, 1, 1);
    x->color = color3(.8, .8, .8);
    return x;
}