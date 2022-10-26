#include <stdlib.h>
#include <assert.h>
#include "ray.h"

Ray* ray(Vector3 *origin, Vector3 *direction) {
    Ray* x = malloc(sizeof(Ray));
    assert(x);
    x->origin = origin;
    x->direction = direction;
    return x;
}