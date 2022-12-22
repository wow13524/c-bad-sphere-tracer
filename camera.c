#include "camera.h"

Camera* abstract_camera() {
    Camera* camera = malloc(sizeof(Camera));
    assert(camera);
    camera->instance = instance();
    return camera;
}

Ray* get_ray(Camera *self, float u, float v, void *rand_state, Ray *out) {
    Vector3 focal_target = {};
    float dof_r = sqrt((float)rand_r(rand_state) / RAND_MAX) * DOF_RADIUS;
    float dof_theta = 2 * M_PI * ((float)rand_r(rand_state) / RAND_MAX);
    float dof_u = dof_r * cosf(dof_theta);
    float dof_v = dof_r * sinf(dof_theta);
    out->direction->x = (self->size_x) * (u - .5);
    out->direction->y = (self->size_y) * (v - .5);
    out->direction->z = 1;
    vec3_cpy(self->instance->position, out->origin);
    vec3_unit(out->direction, out->direction);
    vec3_fma(out->origin, out->direction, DOF_DISTANCE, &focal_target);
    out->origin->x -= dof_u;
    out->origin->y -= dof_v;
    float t = vec3_mag(vec3_sub(&focal_target, out->origin, out->direction));
    vec3_div(out->direction, t, out->direction);
    return out;
}

Camera* perspective_camera(float fov_vertical, float aspect_ratio) {
    Camera* camera = abstract_camera();
    camera->get_ray = get_ray;
    camera->size_x = 2 * tanf(fov_vertical / 2) * aspect_ratio;
    camera->size_y = -2 * tanf(fov_vertical / 2);
    return camera;
}