#ifndef HDRI_H
#define HDRI_H

#define READ_BUF_SIZE 129

typedef struct Hdri {
    unsigned int size_x;
    unsigned int size_y;
    unsigned int *data;
} Hdri;

Hdri* hdri(char *filename);

#endif