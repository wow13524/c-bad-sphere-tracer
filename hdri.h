#include <stdio.h>
#include "color3.h"
#include "vector3.h"

#ifndef HDRI_H
#define HDRI_H

#define READ_BUF_SIZE 65536

typedef struct RleDecoder {
    FILE *stream;
    char *buf;
    unsigned int bufpos;
    char mode;
    char ctr;
    unsigned int ctr_row;
    char last;

} RleDecoder;

typedef struct Hdri {
    unsigned int size_x;
    unsigned int size_y;
    char *data_r;
    char *data_g;
    char *data_b;
    char *data_e;
    Color3* (*sample)(struct Hdri *self, Vector3 *direction, Color3 *out);
} Hdri;

RleDecoder* rle_decoder(FILE *stream);

Hdri* hdri(char *filename);

#endif