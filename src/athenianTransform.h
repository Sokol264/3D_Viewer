#ifndef SRC_ATHENIANTRANSFORM_H_
#define SRC_ATHENIANTRANSFORM_H_

#include "s21_matrix.h"

matrix_t create_move_matrix(long double x_move, long double y_move, long double z_move);
matrix_t create_resize_matrix(long double x_resize, long double y_resize, long double z_resize);
matrix_t create_x_rotate_matrix(long double x_rotate);
matrix_t create_y_rotate_matrix(long double y_rotate);
matrix_t create_z_rotate_matrix(long double z_rotate);
void move_model(matrix_t *points, int point_count, long double x_move, long double y_move,
                long double z_move);
void resize_model(matrix_t *points, int point_count, long double x_resize, long double y_resize,
                  long double z_resize);
void rotate_model(matrix_t *points, int point_count, long double x_rotate, long double y_rotate,
                  long double z_rotate);

#endif  // SRC_ATHENIANTRANSFORM_H_
