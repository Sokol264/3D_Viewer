#ifndef SRC_FILEREADER_H_
#define SRC_FILEREADER_H_

#include "s21_matrix.h"
typedef struct surface_struct {
    int point_count;
    int *ver_t;
} surface_t;

typedef struct count_vert {
    int vertNormCount;
    int vertTexCount;
    int vertCount;
    int surfaceCount;
} count_v;

typedef struct count_flags {
    int first;
    int second;
    int third;
} count_t;

int readFile(matrix_t **vert, matrix_t **vertNorm, matrix_t **vertTex, surface_t **surfaces, char *filename,
             count_v *counts);
void freeVerticesMatrix(matrix_t **vert, matrix_t **vertNorm, matrix_t **vertTex, surface_t **surfaces,
                        int vertCount, int vertNormCount, int vertTexCount, int surfaceCount);
int countEdges(surface_t *surfaces, int countSurface);

#endif  // SRC_FILEREADER_H_
