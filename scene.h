#include <assert.h>
#include <float.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
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
#define SCENE_OUTPUT_SAMPLES 32
#define SCENE_RECURSION_DEPTH 8
#define SCENE_RENDER_THREADS 4

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

typedef struct SceneRenderArgs {
    Scene *self;
    Camera *camera;
    unsigned int thread_i;
    unsigned int *output;
    int *line_cnt;
} SceneRenderArgs;

Scene* scene();

#endif