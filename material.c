#include "material.h"

Material* material() {
    Material *x = malloc(sizeof(Material));
    assert(x);
    x->color = color3(.8, .8, .8);
    x->ior = 1;
    x->reflectance = 0;
    x->transmission = 0;

    x->checker = 0;
    return x;
}