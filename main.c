#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "camera.h"
#include "light.h"
#include "render_output.h"
#include "scene.h"

int main(void) {
    Light *ambient = light(ambient_light, ALWAYS_VISIBLE);
    ambient->instance->size = vector3(.375, 0, 0);

    Light *point_light_a = light(point_light, LINE_OF_SIGHT);
    point_light_a->instance->position = vector3(-2, 4, 0);
    point_light_a->instance->size = vector3(24, 0, 0);
    point_light_a->color = color3(1, .5, 1);

    Light *point_light_b = light(point_light, LINE_OF_SIGHT);
    point_light_b->instance->position = vector3(4, 1, 3);
    point_light_b->instance->size = vector3(12, 0, 0);
    point_light_b->color = color3(.5, 1, 1);

    SDFInstance *ground_plane = sdf_instance(plane);
    ground_plane->instance->position = vector3(0, -2.501, 0);
    ground_plane->material->color = color3(1, .8, .8);
    ground_plane->material->reflectance = .25;

    SDFInstance *sphere_a = sdf_instance(sphere);
    sphere_a->instance->position = vector3(0, 0, 5);
    sphere_a->instance->size = vector3(5, 0, 0);
    sphere_a->material->color = color3(.8, .8, 1);
    sphere_a->material->reflectance = .5;
    sphere_a->material->transmission = .5;

    SDFInstance *sphere_b = sdf_instance(sphere);
    sphere_b->instance->position = vector3(5, -.5, 8);
    sphere_b->instance->size = vector3(4, 0, 0);
    sphere_b->material->color = color3(.8, 1, .8);
    sphere_b->material->reflectance = .5;

    SDFInstance *sphere_c = sdf_instance(sphere);
    sphere_c->instance->position = vector3(-2, 1, 2.5);
    sphere_c->instance->size = vector3(1, 0, 0);
    sphere_c->material->color = color3(.8, 0, .8);
    sphere_c->material->reflectance = .5;

    Camera *cam = perspective_camera(70 * M_PI / 180, 16. / 9.);

    Scene *s = scene();
    s->add_instance(s, ground_plane);
    s->add_instance(s, sphere_a);
    s->add_instance(s, sphere_b);
    s->add_instance(s, sphere_c);
    s->add_light(s, ambient);
    s->add_light(s, point_light_a);
    s->add_light(s, point_light_b);

    render_to_ppm(s->render(s, cam));

    return EXIT_SUCCESS;
}