#include <stdlib.h>
#include <assert.h>
#include "ray_march_result.h"

RayMarchResult* ray_march_result() {
    RayMarchResult* x = malloc(sizeof(RayMarchResult));
    assert(x);
    return x;
}