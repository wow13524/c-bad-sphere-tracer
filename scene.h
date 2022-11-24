#include "camera.h"
#include "hdri.h"
#include "light.h"
#include "sdf_instance.h"

#ifndef SCENE_H
#define SCENE_H

#define SCENE_ATMOSPHERE_IOR 1.00029
#define SCENE_ALPHA_MIN .005
#define SCENE_INSTANCES_MAX 8
#define SCENE_LIGHTS_MAX 8
#define SCENE_MARCH_DIST_MAX 2048
#define SCENE_MARCH_ITER_MAX 2048
#define SCENE_OUTPUT_HEIGHT 108
#define SCENE_OUTPUT_WIDTH 192
#define SCENE_OUTPUT_SAMPLES 1
#define SCENE_RECURSION_DEPTH 8

typedef struct Scene {
    int instance_count;
    int light_count;
    Hdri *environment;
    SDFInstance **instances;
    Light **lights;
    void (*add_instance)(struct Scene *self, SDFInstance *instance);
    void (*add_light)(struct Scene *self, Light *light);
    unsigned int* (*render)(struct Scene *self, Camera *camera);
} Scene;

Scene* scene();

#endif