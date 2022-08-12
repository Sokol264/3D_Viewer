#include "fileReader.h"

int main() {
    matrix_t *vert = NULL;
    matrix_t *vertNorm = NULL;
    matrix_t *vertTex = NULL;
    surface_t *surfaces = NULL;

    char *filename = "objects/test.obj";
    count_v counts = {0, 0, 0, 0};
    readFile(&vert, &vertNorm, &vertTex, &surfaces, filename, &counts);

    printf("%Lf %Lf\n", vert[0].matrix[0][0], vert[0].matrix[1][0]);

    return 0;
}
