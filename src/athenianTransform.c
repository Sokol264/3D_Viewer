#include "athenianTransform.h"

matrix_t create_move_matrix(long double x_move, long double y_move, long double z_move) {
    matrix_t result = s21_create_matrix(4, 4);
    result.matrix_type = 0;
    for (int i = 0; i < 4; i++) {
        result.matrix[i][i] = 1;
    }
    result.matrix[0][3] = x_move;
    result.matrix[1][3] = y_move;
    result.matrix[2][3] = z_move;
    return result;
}

matrix_t create_resize_matrix(long double x_resize, long double y_resize, long double z_resize) {
    matrix_t result = s21_create_matrix(4, 4);
    result.matrix_type = 0;
    result.matrix[0][0] = x_resize;
    result.matrix[1][1] = y_resize;
    result.matrix[2][2] = z_resize;
    result.matrix[3][3] = 1;
    return result;
}

matrix_t create_x_rotate_matrix(long double x_rotate) {
    matrix_t result = s21_create_matrix(4, 4);
    x_rotate = (x_rotate * M_PI) / 180;
    result.matrix_type = 0;
    result.matrix[0][0] = 1;
    result.matrix[1][1] = cosl(x_rotate);
    result.matrix[1][2] = sinl(x_rotate);
    result.matrix[2][2] = cosl(x_rotate);
    result.matrix[2][1] = -1 * sinl(x_rotate);
    result.matrix[3][3] = 1;
    return result;
}

matrix_t create_y_rotate_matrix(long double y_rotate) {
    matrix_t result = s21_create_matrix(4, 4);
    y_rotate = (y_rotate * M_PI) / 180;
    result.matrix_type = 0;
    result.matrix[0][0] = cosl(y_rotate);
    result.matrix[1][1] = 1;
    result.matrix[0][2] = -1 * sin(y_rotate);
    result.matrix[2][2] = cosl(y_rotate);
    result.matrix[2][0] = sin(y_rotate);
    result.matrix[3][3] = 1;
    return result;
}

matrix_t create_z_rotate_matrix(long double z_rotate) {
    matrix_t result = s21_create_matrix(4, 4);
    z_rotate = (z_rotate * M_PI) / 180;
    result.matrix_type = 0;
    result.matrix[0][0] = cosl(z_rotate);
    result.matrix[1][1] = cosl(z_rotate);
    result.matrix[0][1] = sinl(z_rotate);
    result.matrix[2][2] = 1;
    result.matrix[1][0] = -1 * sinl(z_rotate);
    result.matrix[3][3] = 1;
    return result;
}

void move_model(matrix_t *points, int point_count, long double x_move, long double y_move,
                long double z_move) {
    matrix_t move_matrix = create_move_matrix(x_move, y_move, z_move);
    for (int i = 0; i < point_count; i++) {
        matrix_t moved_point = s21_mult_matrix(&move_matrix, &(points[i]));
        s21_remove_matrix(&(points[i]));
        points[i] = moved_point;
    }
    s21_remove_matrix(&move_matrix);
}

void resize_model(matrix_t *points, int point_count, long double x_resize, long double y_resize,
                  long double z_resize) {
    matrix_t resize_matrix = create_resize_matrix(x_resize, y_resize, z_resize);
    for (int i = 0; i < point_count; i++) {
        matrix_t moved_point = s21_mult_matrix(&resize_matrix, &(points[i]));
        s21_remove_matrix(&(points[i]));
        points[i] = moved_point;
    }
    s21_remove_matrix(&resize_matrix);
}

void rotate_model(matrix_t *points, int point_count, long double x_rotate, long double y_rotate,
                  long double z_rotate) {
    matrix_t x_rotate_matrix = create_x_rotate_matrix(x_rotate);
    matrix_t y_rotate_matrix = create_y_rotate_matrix(y_rotate);
    matrix_t z_rotate_matrix = create_z_rotate_matrix(z_rotate);
    matrix_t temp_transform = s21_mult_matrix(&x_rotate_matrix, &y_rotate_matrix);
    matrix_t result_transform = s21_mult_matrix(&temp_transform, &z_rotate_matrix);
    for (int i = 0; i < point_count; i++) {
        matrix_t moved_point = s21_mult_matrix(&result_transform, &(points[i]));
        s21_remove_matrix(&(points[i]));
        points[i] = moved_point;
        // moved_point = s21_mult_matrix(&y_rotate_matrix, &(points[i]));
        // s21_remove_matrix(&(points[i]));
        // points[i] = moved_point;
        // moved_point = s21_mult_matrix(&z_rotate_matrix, &(points[i]));
        // s21_remove_matrix(&(points[i]));
        // points[i] = moved_point;
    }
    s21_remove_matrix(&x_rotate_matrix);
    s21_remove_matrix(&y_rotate_matrix);
    s21_remove_matrix(&z_rotate_matrix);
    s21_remove_matrix(&temp_transform);
    s21_remove_matrix(&result_transform);
}
