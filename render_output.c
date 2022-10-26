#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "render_output.h"
#include "scene.h"

void render_to_terminal(Color3 **output) {
    char *output_buffer = malloc(sizeof(char) * (SCENE_OUTPUT_HEIGHT * (SCENE_OUTPUT_WIDTH * TERMINAL_CHARS_PER_PIXEL + 1) + 4 + 1));
    assert(output_buffer);
    char *buf_ptr = output_buffer;
    int last_color = -1;
    for(int i = 0; i < SCENE_OUTPUT_HEIGHT; i++) {
        for (int j = 0; j < SCENE_OUTPUT_WIDTH; j++) {
            Color3 *raw_color = *(output + i * SCENE_OUTPUT_WIDTH + j);
            Color3 *c = col3_clamp(raw_color);
            int c_int = col3_to_int(c);
            if (c_int != last_color) {
                last_color = c_int;
                buf_ptr += sprintf(buf_ptr, "\e[48;2;%d;%d;%dm ", (int)(c->r * 255), (int)(c->g * 255), (int)(c->b * 255));
            }
            else {
                buf_ptr += sprintf(buf_ptr, " ");
            }
            free(raw_color);
            free(c);
            raw_color = NULL;
            c = NULL;
        }
        buf_ptr += sprintf(buf_ptr, "\n");
    }
    sprintf(buf_ptr, "\e[0m");
    printf("%s", output_buffer);
    free(output_buffer);
    output_buffer = NULL;
}