#include "fileReader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "athenianTransform.h"
#include "s21_matrix.h"

void nextNumber(count_t *count) {
    if (count->first) {
        count->first = 0;
        count->second = 1;
    } else if (count->second) {
        count->second = 0;
        count->third = 1;
    }
}

count_t setFlags(int i1, int i2, int i3) {
    count_t c = {i1, i2, i3};
    return c;
}

void readNumber(char **input, int *number) {
    int step;
    sscanf(*input, "%d%n", number, &step);
    *input += step;
}

void chooseWritenNumber(count_t c, char **input, int *first, int *second, int *third, int *result,
                        surface_t *surface, int *index) {
    if (c.first) {
        readNumber(input, first);
        if (surface != NULL) surface->ver_t[*index] = *first;
        (*result)++;
        (*index)++;
    } else if (c.second) {
        readNumber(input, second);
    } else {
        readNumber(input, third);
    }
}

int countOrWritePoints(char *input, surface_t *surface) {
    int result = 0, first, second, third, space_flag = 0, number = 0, pointIndex = 0;
    count_t c = setFlags(1, 0, 0);
    while (*input != '\0') {
        if (*input >= '0' && *input <= '9') {
            if (space_flag && number) {
                c = setFlags(1, 0, 0);
                space_flag = 0;
                number = 0;
            }
            chooseWritenNumber(c, &input, &first, &second, &third, &result, surface, &pointIndex);
            number = 1;
        } else if (*input == '/') {
            number = 0;
            space_flag = 0;
            nextNumber(&c);
            input++;
        } else if (*input == ' ') {
            space_flag = 1;
            input++;
        } else {
            input++;
        }
    }
    return result;
}

void countingVertices(FILE *obj, int *vert, int *vertN, int *vertT, int *surf) {
    char *line = NULL;
    while (!feof(obj)) {
        size_t len;
        getline(&line, &len, obj);
        if (!strncmp(line, "vn ", 3))
            (*vertN)++;
        else if (!strncmp(line, "vt ", 3))
            (*vertT)++;
        else if (!strncmp(line, "v ", 2))
            (*vert)++;
        else if (!strncmp(line, "f ", 2))
            (*surf)++;
    }
    free(line);
}

int countEdges(surface_t *surfaces, int countSurface) {
    int result = 0;
    for (int i = 0; i < countSurface; i++) {
        result += surfaces->point_count;
    }
    return result;
}

void initVerticesMatrix(matrix_t **vert, matrix_t **vertNorm, matrix_t **vertTex, surface_t **surfaces,
                        int vertCount, int vertNormCount, int vertTexCount, int surfaceCount) {
    *surfaces = (surface_t *)malloc(sizeof(surface_t) * surfaceCount);
    *vert = (matrix_t *)malloc(sizeof(matrix_t) * vertCount);
    for (int i = 0; i < vertCount; i++) (*vert)[i] = s21_create_matrix(4, 1);
    *vertNorm = (matrix_t *)malloc(sizeof(matrix_t) * vertNormCount);
    for (int i = 0; i < vertNormCount; i++) (*vertNorm)[i] = s21_create_matrix(4, 1);
    *vertTex = (matrix_t *)malloc(sizeof(matrix_t) * vertTexCount);
    for (int i = 0; i < vertTexCount; i++) (*vertTex)[i] = s21_create_matrix(1, 2);
}

void freeVerticesMatrix(matrix_t **vert, matrix_t **vertNorm, matrix_t **vertTex, surface_t **surfaces,
                        int vertCount, int vertNormCount, int vertTexCount, int surfaceCount) {
    for (int i = 0; i < vertCount; i++) s21_remove_matrix(&(*vert)[i]);
    free(*vert);
    for (int i = 0; i < vertNormCount; i++) s21_remove_matrix(&(*vertNorm)[i]);
    free(*vertNorm);
    for (int i = 0; i < vertTexCount; i++) s21_remove_matrix(&(*vertTex)[i]);
    free(*vertTex);
    for (int i = 0; i < surfaceCount; i++) free((*surfaces)[i].ver_t);
    free(*surfaces);
}

void readVertices(FILE *obj, matrix_t **vert, matrix_t **vertNorm, matrix_t **vertTex, surface_t **surfaces) {
    char *line = NULL;
    int indexV = 0, indexVN = 0, indexVT = 0, indexS = 0;
    while (!feof(obj)) {
        size_t len;
        getline(&line, &len, obj);
        if (!strncmp(line, "vn ", 3)) {
            sscanf(line, "vn %Lf %Lf %Lf", &(*vertNorm)[indexVN].matrix[0][0],
                   &(*vertNorm)[indexVN].matrix[1][0], &(*vertNorm)[indexVN].matrix[2][0]);
            (*vertNorm)[indexVN].matrix[3][0] = 1.0L;
            indexVN++;
        } else if (!strncmp(line, "vt ", 3)) {
            sscanf(line, "vt %Lf %Lf", &(*vertTex)[indexVT].matrix[0][0], &(*vertTex)[indexVT].matrix[0][1]);
            indexVT++;
        } else if (!strncmp(line, "v ", 2)) {
            sscanf(line, "v %Lf %Lf %Lf", &(*vert)[indexV].matrix[0][0], &(*vert)[indexV].matrix[1][0],
                   &(*vert)[indexV].matrix[2][0]);
            (*vert)[indexV].matrix[3][0] = 1.0L;
            indexV++;
        } else if (!strncmp(line, "f ", 2)) {
            int countPointInSurfaces = countOrWritePoints(line, NULL);
            (*surfaces)[indexS].point_count = countPointInSurfaces;
            (*surfaces)[indexS].ver_t = (int *)malloc(sizeof(int) * countPointInSurfaces);
            countOrWritePoints(line, &(*surfaces)[indexS]);
            indexS++;
        }
    }
    free(line);
}

int readFile(matrix_t **vert, matrix_t **vertNorm, matrix_t **vertTex, surface_t **surfaces, char *filename,
             count_v *counts) {
    FILE *obj = fopen(filename, "r");
    if (obj == NULL) return -1;

    countingVertices(obj, &((*counts).vertCount), &((*counts).vertNormCount), &((*counts).vertTexCount),
                     &((*counts).surfaceCount));
    fseek(obj, 0, SEEK_SET);
    initVerticesMatrix(vert, vertNorm, vertTex, surfaces, counts->vertCount, counts->vertNormCount,
                       counts->vertTexCount, counts->surfaceCount);

    readVertices(obj, vert, vertNorm, vertTex, surfaces);
    return 0;
}
