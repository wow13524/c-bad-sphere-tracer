#include <math.h>
#include <stdlib.h>
#include "camera.h"
#include "hdri.h"
#include "light.h"
#include "render_output.h"
#include "scene.h"

int main(void) {
    Hdri *environment = hdri("clarens_night_02_4k.hdr");
    //return EXIT_SUCCESS;

    SDFInstance *test = sdf_instance(sphere);
    test->instance->position = vector3(0, 0, 5);
    test->instance->size = vector3(5, 5, 5);
    test->material->color = color3(1, 0, 1);
    test->material->ior = 1.1;
    test->material->roughness = 0;
    test->material->transmission = 0;

    SDFInstance *ground_plane = sdf_instance(plane);
    ground_plane->instance->position = vector3(0, -2.501, 0);
    ground_plane->material->color = color3(1, .8, .8);
    ground_plane->material->roughness = .025;
    ground_plane->material->checker = 1;

    SDFInstance *sphere_a = sdf_instance(sphere);
    sphere_a->instance->position = vector3(0, 0, 5);
    sphere_a->instance->size = vector3(5, 5, 5);
    sphere_a->material->color = color3(.8, .8, 1);
    sphere_a->material->ior = 1.125;
    sphere_a->material->roughness = 0;
    sphere_a->material->transmission = 1;

    SDFInstance *sphere_b = sdf_instance(sphere);
    sphere_b->instance->position = vector3(-3.75, 2.5, 12.5);
    sphere_b->instance->size = vector3(10, 10, 10);
    sphere_b->material->color = color3(.5, .8, .8);
    sphere_b->material->ior = 2;
    sphere_b->material->roughness = .05;
    sphere_b->material->transmission = 1;

    SDFInstance *cube_a = sdf_instance(cube);
    cube_a->instance->position = vector3(5, -.5, 8);
    cube_a->instance->size = vector3(4, 4, 4);
    cube_a->material->color = color3(.8, 1, .8);
    cube_a->material->ior = 1.25;
    cube_a->material->roughness = .1;
    cube_a->material->transmission = 1;

    SDFInstance *cube_b = sdf_instance(cube);
    cube_b->instance->position = vector3(-2, 1, 2.5);
    cube_b->instance->size = vector3(.75, .75, .75);
    cube_b->material->color = color3(.8, 0, .8);
    cube_b->material->ior = 1.5;
    cube_b->material->roughness = 0;
    cube_b->material->transmission = 1;

    SDFInstance *wall_left = sdf_instance(cube);
    wall_left->instance->position = vector3(-10, 5, 10);
    wall_left->instance->size = vector3(1, 20, 25);
    wall_left->material->color = color3(.5, .8, .8);
    wall_left->material->roughness = .2;

    SDFInstance *wall_right = sdf_instance(cube);
    wall_right->instance->position = vector3(10, 5, 10);
    wall_right->instance->size = vector3(1, 20, 25);
    wall_right->material->color = color3(.8, .5, .8);
    wall_right->material->roughness = .2;

    SDFInstance *wall_back = sdf_instance(cube);
    wall_back->instance->position = vector3(0, 5, -2.5);
    wall_back->instance->size = vector3(20, 20, 1);
    wall_back->material->color = color3(.8, .8, .5);
    wall_back->material->roughness = .2;

    SDFInstance *wall_front = sdf_instance(cube);
    wall_front->instance->position = vector3(0, 5, 22.5);
    wall_front->instance->size = vector3(20, 20, 1);
    wall_front->material->color = color3(.5, .8, .5);
    wall_front->material->roughness = .2;

    SDFInstance *wall_top = sdf_instance(cube);
    wall_top->instance->position = vector3(0, 15, 10);
    wall_top->instance->size = vector3(20, 1, 25);
    wall_top->material->color = color3(.8, .5, .5);
    wall_top->material->roughness = .2;

    Camera *cam = perspective_camera(70 * M_PI / 180, 16. / 9.);

    Scene *s = scene();
    s->environment = environment;
    //s->add_instance(s, test);
    s->add_instance(s, ground_plane);
    s->add_instance(s, sphere_a);
    s->add_instance(s, sphere_b);
    s->add_instance(s, cube_a);
    s->add_instance(s, cube_b);
    s->add_instance(s, wall_left);
    s->add_instance(s, wall_right);
    s->add_instance(s, wall_back);
    //s->add_instance(s, wall_front);
    s->add_instance(s, wall_top);

    render_to_ppm(s->render(s, cam));

    return EXIT_SUCCESS;
}
