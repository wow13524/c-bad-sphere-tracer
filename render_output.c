#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "render_output.h"
#include "scene.h"

void render_to_terminal(unsigned int *output) {
    char *output_buffer = malloc(sizeof(char) * (SCENE_OUTPUT_HEIGHT * (SCENE_OUTPUT_WIDTH * TERMINAL_CHARS_PER_PIXEL + 1) + 4 + 1));
    assert(output_buffer);
    char *buf_ptr = output_buffer;
    int last_color = -1;
    for(int i = 0; i < SCENE_OUTPUT_HEIGHT; i++) {
        for (int j = 0; j < SCENE_OUTPUT_WIDTH; j++) {
            int c_int = *(output + i * SCENE_OUTPUT_WIDTH + j);
            if (c_int != last_color) {
                last_color = c_int;
                buf_ptr += sprintf(buf_ptr, "\e[48;2;%d;%d;%dm ", c_int >> 16, (c_int >> 8) & 0xFF, c_int & 0xFF);
            }
            else {
                buf_ptr += sprintf(buf_ptr, " ");
            }
        }
        buf_ptr += sprintf(buf_ptr, "\n");
    }
    sprintf(buf_ptr, "\e[0m");
    printf("%s", output_buffer);
    free(output_buffer);
    output_buffer = NULL;
}

void render_to_ppm(unsigned int *output) {
    printf("P6\n%d %d\n255\n", SCENE_OUTPUT_WIDTH, SCENE_OUTPUT_HEIGHT);
    for (int i = 0; i < SCENE_OUTPUT_WIDTH * SCENE_OUTPUT_HEIGHT; i++) {
        unsigned int c_int = *(output + i);
        printf("%c%c%c", c_int >> 16, (c_int >> 8) & 0xFF, c_int & 0xFF);
    }
}