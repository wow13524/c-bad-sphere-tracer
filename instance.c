#include <stdlib.h>
#include <assert.h>
#include "instance.h"

Instance* instance() {
    Instance* x = malloc(sizeof(Instance));
    assert(x);
    x->position = vector3(0, 0, 0);
    x->size = vector3(1, 1, 1);
    x->_position = (float32x4_t){0, 0, 0, 0};
    x->_size = (float32x4_t){0, 0, 0, 0};
    return x;
}

void refresh_instance(Instance* x) {
    x->_position = vld1q_f32((float32_t *)x->position);
    x->_size = vld1q_f32((float32_t *)x->size);
}