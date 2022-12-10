#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "color3.h"

#ifndef RENDER_OUTPUT_H
#define RENDER_OUTPUT_H

#define TERMINAL_CHARS_PER_PIXEL 20

void render_to_terminal(unsigned int *output);
void render_to_ppm(unsigned int *output);

#endif