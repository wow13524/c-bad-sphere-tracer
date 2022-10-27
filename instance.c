#include <stdlib.h>
#include <assert.h>
#include "instance.h"

Instance* instance() {
    Instance* x = malloc(sizeof(Instance));
    assert(x);
    x->position = vector3(0, 0, 0);
    x->size = vector3(1, 1, 1);
    x->color = color3(.8, .8, .8);
    return x;
}