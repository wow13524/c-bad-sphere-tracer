#include "camera.h"
#include "light.h"
#include "sdf_instance.h"

#ifndef SCENE_H
#define SCENE_H

#define SCENE_INSTANCES_MAX 8
#define SCENE_LIGHTS_MAX 8
#define SCENE_MARCH_ITER_MAX 2048
#define SCENE_OUTPUT_HEIGHT 45
#define SCENE_OUTPUT_WIDTH (int)(80 * (17. / 7.))
#define SCENE_OUTPUT_SAMPLES 1

typedef struct Scene {
    int instance_count;
    int light_count;
    SDFInstance **instances;
    Light **lights;
    void (*add_instance)(struct Scene *self, SDFInstance *instance);
    void (*add_light)(struct Scene *self, Light *light);
    Color3** (*render)(struct Scene *self, Camera *camera);
} Scene;

Scene* scene();

#endif