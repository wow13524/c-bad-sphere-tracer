#include <stdio.h>
#include "color3.h"
#include "vector3.h"

#ifndef HDRI_H
#define HDRI_H

#define READ_BUF_SIZE 129

typedef struct RleDecoder {
    FILE *stream;
    unsigned int mode:1;
    unsigned int ctr:8;
    unsigned int ctr_row;
    unsigned int last:8;

} RleDecoder;

typedef struct Hdri {
    unsigned int size_x;
    unsigned int size_y;
    Color3 **data;
    Color3* (*sample)(struct Hdri *self, Vector3 *direction, Color3 *out);
} Hdri;

RleDecoder* rle_decoder(FILE *stream);

Hdri* hdri(char *filename);

#endif