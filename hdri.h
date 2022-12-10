#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "color3.h"
#include "vector3.h"

#ifndef HDRI_H
#define HDRI_H

#define READ_BUF_SIZE 16384

typedef struct Hdri {
    unsigned int size_x;
    unsigned int size_y;
    unsigned int *data;
    Color3* (*sample)(struct Hdri *self, Vector3 *direction, Color3 *out);
} Hdri;

Hdri* hdri(char *filename);

#endif