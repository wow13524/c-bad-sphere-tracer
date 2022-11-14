#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "hdri.h"

//Implements a generator to decode run length encoding of .hdr files
int rle_next_byte(RleDecoder *self) {
    if (!self->ctr_row) {
        fseek(self->stream, 2, SEEK_CUR);
        self->ctr_row = 4 * ((fgetc(self->stream) << 8) + fgetc(self->stream));
    }
    self->ctr_row--;
    if (!self->ctr) {
        int count = fgetc(self->stream);
        if (count > 128) {
            self->mode = 1;
            self->ctr = count - 128;
            self->last = fgetc(self->stream);
        }
        else {
            self->mode = 0;
            self->ctr = count;
        }
    }
    self->ctr--;
    if (self->mode) {
        return self->last;
    }
    else {
        return fgetc(self->stream);
    }
}

RleDecoder* rle_decoder(FILE *stream) {
    RleDecoder *x = malloc(sizeof(RleDecoder));
    assert(x);
    x->stream = stream;
    x->mode = 0;
    x->ctr = 0;
    x->ctr_row = 0;
    x->last = 0;
    return x;
}

Color3* sample(Hdri *self, Vector3 *direction, Color3 *out) {
    float u = fmod(atan2(direction->z, direction->x) / M_PI + .25 + 1, 1);
    float v = fmod(-asin(direction->y) / M_PI + .5 + 1, 1);

    unsigned int x = u * (self->size_x - 1);
    unsigned int y = v * (self->size_y - 1);
    
    return col3_cpy(*(self->data + y * self->size_x + x), out);
}

Hdri* hdri(char *filename) {
    Hdri *x = malloc(sizeof(Hdri));
    assert(x);
    char *buf = malloc(sizeof(char) * READ_BUF_SIZE);
    assert(buf);
    FILE *file = fopen(filename, "rb");
    if (!strcmp(fgets(buf, READ_BUF_SIZE, file),"#?RADIANCE")) {
        fprintf(stderr, "%s is not a valid .hdr file\n", filename);
        free(buf);
        fclose(file);
        return NULL;
    }
    if (!strcmp(fgets(buf, READ_BUF_SIZE, file),"FORMAT=32-bit_rle_rgbe")) {
        fprintf(stderr, "%s is not of format 32-bit_rle_rgbe\n", filename);
        free(buf);
        fclose(file);
        return NULL;
    }
    fgetc(file);    //blank line
    fgets(buf, READ_BUF_SIZE, file);    //load dimension info
    char *endptr;

    //get size of hdr
    x->size_y = strtol(buf + 3, &endptr, 10);
    if (*endptr != ' ') {
        fprintf(stderr, "%s has invalid y dimension\n", filename);
        free(buf);
        fclose(file);
        return NULL;
    }
    x->size_x = strtol(endptr + 4, &endptr, 10);
    if (*endptr != '\n') {
        fprintf(stderr, "%s has invalid x dimension\n", filename);
        free(buf);
        fclose(file);
        return NULL;
    }

    x->data = malloc(sizeof(Color3 *) * x->size_x * x->size_y);
    assert(x->data);
    for (unsigned int i = 0; i < x->size_y * x->size_x; i++) {
        *(x->data + i) = color3(0, 0, 0);
    }

    RleDecoder *decoder = rle_decoder(file);

    for (unsigned int i = 0; i < x->size_y; i++) {
        for (int j = 0; j < 4; j++) {
            for (unsigned int k = 0; k < x->size_x; k++) {
                int c = rle_next_byte(decoder);
                Color3 *color = *(x->data + i * x->size_x + k);
                if (j == 0) {
                    color->r = c;
                }
                else if (j == 1) {
                    color->g = c;
                }
                else if (j == 2) {
                    color->b = c;
                }
                else if (j == 3) {
                    col3_smul(color, pow(2, c - 136), color);
                }
            }
        }
    }

    free(buf);
    fclose(file);

    x->sample = sample;

    return x;
}