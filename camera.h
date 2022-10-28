#include "ray.h"
#include "sdf_instance.h"

#ifndef CAMERA_H
#define CAMERA_H

#define EPSILON .0001

typedef struct Camera {
    Instance *instance;
    Ray* (*get_ray)(struct Camera *self, float u, float v, Ray *out);
    float size_x;
    float size_y;
} Camera;

Camera* perspective_camera(float fov_vertical, float aspect_ratio);

#endif