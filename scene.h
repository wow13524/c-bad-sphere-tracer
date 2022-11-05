#include "camera.h"
#include "light.h"
#include "sdf_instance.h"

#ifndef SCENE_H
#define SCENE_H

#define SCENE_AIR_IOR 1.
#define SCENE_ALPHA_MIN .01
#define SCENE_INSTANCES_MAX 8
#define SCENE_LIGHTS_MAX 8
#define SCENE_MARCH_DIST_MAX 512
#define SCENE_MARCH_ITER_MAX 2048
#define SCENE_OUTPUT_HEIGHT (108 + 0 * 45)
#define SCENE_OUTPUT_WIDTH (int)(192 + 0 * 80 * (17. / 7.))
#define SCENE_OUTPUT_SAMPLES 4
#define SCENE_RECURSION_DEPTH 8

typedef struct Scene {
    int instance_count;
    int light_count;
    SDFInstance **instances;
    Light **lights;
    void (*add_instance)(struct Scene *self, SDFInstance *instance);
    void (*add_light)(struct Scene *self, Light *light);
    unsigned int* (*render)(struct Scene *self, Camera *camera);
} Scene;

Scene* scene();

#endif