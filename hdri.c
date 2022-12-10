#include "hdri.h"

/*
 *Heavily stripped down implementation of a file buffer, only advances when pos
 *is a multiple of READ_BUF_SIZE (due to frequency of calls, keep it simple!)
*/
static inline unsigned char rle_get_buf_byte(FILE *stream, unsigned char *buf, unsigned int *pos) {
    if (*pos >= READ_BUF_SIZE) {
        *pos -= READ_BUF_SIZE;
        fread(buf, READ_BUF_SIZE, 1, stream);
    }
    return *(buf + (*pos)++);
}

static inline void rle_decode(Hdri *self, FILE *stream) {
    unsigned char buf[READ_BUF_SIZE];
    unsigned int rowbuf[READ_BUF_SIZE];
    unsigned int bufpos = READ_BUF_SIZE;
    unsigned int *bufposptr = &bufpos;
    int size_x = self->size_x;
    int size_y = self->size_y;
    register unsigned char mode = 0;
    register unsigned char ctr = 0;
    register unsigned char last = 0;
    for (int i = 0; i < size_y; i++) {
        bufpos += 4;
        for (int _ = 0; _ < 4; _++) {
            for (int j = 0; j < size_x; j++) {
                if (!ctr) {
                    ctr = rle_get_buf_byte(stream, buf, bufposptr);
                    mode = ctr > 128;
                    last = mode ? rle_get_buf_byte(stream, buf, bufposptr) : 0;
                    ctr = mode ? ctr - 128 : ctr;
                }
                ctr--;
                register unsigned int data = *(rowbuf + j);
                data <<= 8;
                data += mode ? last : rle_get_buf_byte(stream, buf, bufposptr);
                *(rowbuf + j) = data;
            }
            memcpy(self->data + i * size_x, rowbuf, size_x * sizeof(unsigned int));
        }
    }
}

Color3* sample(Hdri *self, Vector3 *direction, Color3 *out) {
    float u = fmod(atan2f(direction->z, direction->x) / M_PI / 2 + .375 + 1, 1);
    float v = fmod(-asinf(direction->y) / M_PI + .5 + 1, 1);

    unsigned int x = u * (self->size_x - 1);
    unsigned int y = v * (self->size_y - 1);
    
    unsigned int color = *(self-> data + y * self->size_x + x);
    float e = powf(2, (int)(color & 0xFF) - 136);
    out->r = (color >> 24) & 0xFF;
    out->g = (color >> 16) & 0xFF;
    out->b = (color >>  8) & 0xFF;
    return col3_smul(out, e, out);
}

Hdri* hdri(char *filename) {
    Hdri *x = malloc(sizeof(Hdri));
    assert(x);

    char buf[READ_BUF_SIZE];
    FILE *file = fopen(filename, "rb");

    if (!strcmp(fgets(buf, READ_BUF_SIZE, file),"#?RADIANCE")) {
        fprintf(stderr, "%s is not a valid .hdr file\n", filename);
        fclose(file);
        return NULL;
    }
    if (!strcmp(fgets(buf, READ_BUF_SIZE, file),"FORMAT=32-bit_rle_rgbe")) {
        fprintf(stderr, "%s is not of format 32-bit_rle_rgbe\n", filename);
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
        fclose(file);
        return NULL;
    }
    x->size_x = strtol(endptr + 4, &endptr, 10);
    if (*endptr != '\n') {
        fprintf(stderr, "%s has invalid x dimension\n", filename);
        fclose(file);
        return NULL;
    }

    x->data = malloc(sizeof(unsigned int) * x->size_x * x->size_y);
    assert(x->data);
    rle_decode(x, file);
    fclose(file);

    x->sample = sample;

    return x;
}