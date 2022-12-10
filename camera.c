#include "camera.h"

Camera* abstract_camera() {
    Camera* camera = malloc(sizeof(Camera));
    assert(camera);
    camera->instance = instance();
    return camera;
}

Ray* get_ray(Camera *self, float u, float v, Ray *out) {
    out->direction->x = self->size_x * (u - .5);
    out->direction->y = self->size_y * (v - .5);
    out->direction->z = 1;
    vec3_cpy(self->instance->position, out->origin);
    vec3_unit(out->direction, out->direction);
    return out;
}

Camera* perspective_camera(float fov_vertical, float aspect_ratio) {
    Camera* camera = abstract_camera();
    camera->get_ray = get_ray;
    camera->size_x = 2 * tanf(fov_vertical / 2) * aspect_ratio;
    camera->size_y = -2 * tanf(fov_vertical / 2);
    return camera;
}

//TODO add lens camera