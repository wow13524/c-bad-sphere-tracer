#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "hdri.h"

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
        fprintf(stderr, "%s is not of format 32-but_rle_rgbe\n", filename);
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

    x->data = malloc(sizeof(unsigned int) * x->size_x * x->size_y);
    assert(x->data);

    //implement some sort of generator for this that will output the next uncompressed byte
    for (int i = 0; i < x->size_y; i++) {
        fgets(buf, 4, file);    //skip first 4 bytes of each row
        for (int j = 0; j < x->size_x; j++) {
            int c = fgetc(file);
            if (c > 128) {

            }
            else {
                for (int k = 0; k < c; k++) {
                    
                }
            }
        }
    }

    free(buf);
    fclose(file);

    return x;
}