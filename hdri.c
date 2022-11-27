#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "hdri.h"

static inline int rle_next_byte_raw(RleDecoder *self, unsigned int offset) {
    self->bufpos += offset;
    while (self->bufpos >= READ_BUF_SIZE) {
        fread(self->buf, READ_BUF_SIZE, 1, self->stream);
        self->bufpos -= READ_BUF_SIZE;
    }
    return *(self->buf + self->bufpos++);
}

//Implements a generator to decode run length encoding of .hdr files
static inline int rle_next_byte(RleDecoder *self) {
    if (!self->ctr_row--) {
        self->ctr_row = 4 * ((rle_next_byte_raw(self, 2) << 8) + rle_next_byte_raw(self, 0)) - 1;
    }
    if (!self->ctr) {
        int count = rle_next_byte_raw(self, 0);
        if (count > 128) {
            self->mode = 1;
            self->ctr = count - 128;
            self->last = rle_next_byte_raw(self, 0);
        }
        else {
            self->mode = 0;
            self->ctr = count;
        }
    }
    self->ctr--;
    switch (self->mode) {
        case 0:
            return rle_next_byte_raw(self, 0);
        case 1:
        return self->last;
    }
}

RleDecoder* rle_decoder(FILE *stream) {
    RleDecoder *x = malloc(sizeof(RleDecoder));
    assert(x);
    char *buf = malloc(sizeof(char) * READ_BUF_SIZE);
    assert(buf);
    x->stream = stream;
    x->buf = buf;
    x->bufpos = READ_BUF_SIZE;
    x->mode = 0;
    x->ctr = 0;
    x->ctr_row = 0;
    x->last = 0;
    return x;
}

Color3* sample(Hdri *self, Vector3 *direction, Color3 *out) {
    float u = fmod(atan2f(direction->z, direction->x) / M_PI / 2 + .375 + 1, 1);
    float v = fmod(-asinf(direction->y) / M_PI + .5 + 1, 1);

    unsigned int x = u * (self->size_x - 1);
    unsigned int y = v * (self->size_y - 1);
    
    unsigned int color = *(self->data + y * self->size_x + x);
    float e = powf(2, (int)(color & 0xFF) - 136);
    out->r = (color >> 24);
    out->g = ((color >> 16) & 0xFF);
    out->b = ((color >> 8) & 0xFF);
    return col3_smul(out, e, out);
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

    x->data = calloc(sizeof(unsigned int), x->size_x * x->size_y);
    assert(x->data);

    RleDecoder *decoder = rle_decoder(file);

    for (unsigned int i = 0; i < x->size_y; i++) {
        for (int j = 0; j < 4; j++) {
            for (unsigned int k = 0; k < x->size_x; k++) {
                *(x->data + i * x->size_x + k) <<= 8;
                *(x->data + i * x->size_x + k) += rle_next_byte(decoder);
            }
        }
    }

    free(buf);
    fclose(file);

    x->sample = sample;

    return x;
}