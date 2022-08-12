#include "s21_matrix.h"

int s21_check_zero_matrix(matrix_t A) {
    int result = 1;
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.columns; j++) {
            if (fabsl(A.matrix[i][j]) > EPS) {
                result = 0;
            }
        }
    }
    return result;
}

int s21_check_identity_matrix(matrix_t A) {
    int result = 1;
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.columns; j++) {
            if (i == j) {
                if (fabsl(A.matrix[i][j] - 1.0) > EPS) {
                    result = 0;
                }
            } else {
                if (fabsl(A.matrix[i][j]) > EPS) {
                    result = 0;
                }
            }
        }
    }
    return result;
}

int s21_check_matrix_type(matrix_t A) {
    int result = 0;
    if (A.matrix_type == INCORRECT_MATRIX) {
        result = 1;
    } else if (s21_check_zero_matrix(A)) {
        result = 3;
    } else if (s21_check_identity_matrix(A)) {
        result = 2;
    } else {
    }
    return result;
}

matrix_t s21_create_matrix(int rows, int columns) {
    matrix_t result;
    if (rows > 0 && columns > 0) {
        result.rows = rows;
        result.columns = columns;
        result.matrix = (long double **)malloc(rows * sizeof(long double *));
        for (int i = 0; i < rows; i++) {
            result.matrix[i] = (long double *)malloc(columns * sizeof(long double));
            for (int j = 0; j < columns; j++) {
                result.matrix[i][j] = 0.0;
            }
        }
        result.matrix_type = ZERO_MATRIX;
    } else {
        result.rows = 0;
        result.columns = 0;
        result.matrix = NULL;
        result.matrix_type = INCORRECT_MATRIX;
    }
    return result;
}

void s21_remove_matrix(matrix_t *A) {
    for (int i = 0; i < A->rows; i++) {
        free(A->matrix[i]);
    }
    if (A->rows > 0) free(A->matrix);
    A->rows = 0;
    A->columns = 0;
    A->matrix_type = INCORRECT_MATRIX;
}

int s21_eq_matrix(matrix_t *A, matrix_t *B) {
    int result = SUCCESS;
    if (A->matrix_type == INCORRECT_MATRIX || B->matrix_type == INCORRECT_MATRIX) {
        result = FAILURE;
    } else {
        if (A->rows == B->rows && A->columns == B->columns) {
            for (int i = 0; i < A->rows && result; i++) {
                for (int j = 0; j < A->columns && result; j++) {
                    if (fabsl(A->matrix[i][j] - B->matrix[i][j]) >= EPS) {
                        result = FAILURE;
                    }
                }
            }
        } else {
            result = FAILURE;
        }
    }
    return result;
}

matrix_t s21_mult_matrix(matrix_t *A, matrix_t *B) {
    matrix_t result;
    if (A->matrix_type == INCORRECT_MATRIX || B->matrix_type == INCORRECT_MATRIX) {
        result = s21_create_matrix(0, 0);
    } else {
        if (A->columns == B->rows) {
            result = s21_create_matrix(A->rows, B->columns);
            for (int i = 0; i < A->rows; i++) {
                for (int j = 0; j < B->columns; j++) {
                    for (int k = 0; k < A->columns; k++) {
                        result.matrix[i][j] += A->matrix[i][k] * B->matrix[k][j];
                    }
                }
            }
            result.matrix_type = s21_check_matrix_type(result);
        } else {
            result = s21_create_matrix(0, 0);
        }
    }
    return result;
}

matrix_t s21_transpose(matrix_t *A) {
    matrix_t result;
    if (A->matrix_type == INCORRECT_MATRIX) {
        result = s21_create_matrix(0, 0);
    } else {
        result = s21_create_matrix(A->columns, A->rows);
        for (int i = 0; i < A->rows; i++) {
            for (int j = 0; j < A->columns; j++) {
                result.matrix[j][i] = A->matrix[i][j];
            }
        }
        result.matrix_type = s21_check_matrix_type(result);
    }
    return result;
}
