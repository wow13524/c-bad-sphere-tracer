#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include "instance.h"
#include "ray.h"

#ifndef CAMERA_H
#define CAMERA_H

#define DOF_DISTANCE 10
#define DOF_RADIUS 0.25
#define EPSILON .0001

typedef struct Camera {
    Instance *instance;
    Ray* (*get_ray)(struct Camera *self, float u, float v, void *rand_state, Ray *out);
    float size_x;
    float size_y;
} Camera;

Camera* perspective_camera(float fov_vertical, float aspect_ratio);

#endif