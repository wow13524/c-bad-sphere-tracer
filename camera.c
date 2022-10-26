#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include "camera.h"

Camera* abstract_camera() {
    Camera* camera = malloc(sizeof(Camera));
    assert(camera);
    camera->instance = instance();
    return camera;
}

Ray* get_ray(Camera *self, float u, float v) {
    Vector3 *direction_unnormalized = vector3(
        tan(self->size_x / 2) * 2 * (u - .5),
        -tan(self->size_y / 2) * 2 * (v - .5),
        1
    );
    Vector3 *direction_normalized = vec3_unit(direction_unnormalized);
    free(direction_unnormalized);
    direction_unnormalized = NULL;
    return ray(
        self->instance->position,
        direction_normalized
    );
}

Camera* perspective_camera(float fov_vertical, float aspect_ratio) {
    Camera* camera = abstract_camera();
    camera->get_ray = get_ray;
    camera->size_x = 2 * atan(tan(fov_vertical / 2) * aspect_ratio);
    camera->size_y = fov_vertical;
    return camera;
}