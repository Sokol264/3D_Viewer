#include "graphics.h"

#define X_VIEW 12.0L
#define Y_VIEW 36.0L
#define Z_VIEW 30.0L

matrix_t create_perspective_matrix(long double ro, long double teta, long double fi) {
    matrix_t result = s21_create_matrix(4, 4);
    result.matrix_type = 0;
    result.matrix[0][0] = -sinl(teta);
    result.matrix[1][0] = cosl(teta);
    result.matrix[0][1] = -1 * cosl(fi) * sinl(teta);
    result.matrix[0][2] = -1 * sinl(fi) * cosl(teta);
    result.matrix[1][1] = -1 * cosl(fi) * sinl(teta);
    result.matrix[1][2] = -1 * sinl(fi) * cosl(teta);
    result.matrix[2][1] = sinl(fi);
    result.matrix[2][2] = -1 * cosl(fi);
    result.matrix[3][2] = ro;
    result.matrix[3][3] = 1;
    return result;
}

void connect_points(matrix_t first_point, matrix_t second_point, cairo_t *cr, int perspective) {
    if (perspective) {
        long double ro = sqrtl(powl(X_VIEW, 2) + powl(Y_VIEW, 2) + powl(Z_VIEW, 2));
        long double teta = atanl(sqrtl(powl(X_VIEW, 2) + powl(Y_VIEW, 2)) / Z_VIEW);
        long double fi = atanl(Y_VIEW / X_VIEW);
        matrix_t perspective_matrix = create_perspective_matrix(ro, teta, fi);
        matrix_t trans_fp = s21_transpose(&first_point);
        matrix_t trans_sp = s21_transpose(&second_point);
        matrix_t perspective_first_point = s21_mult_matrix(&trans_fp, &perspective_matrix);
        matrix_t perspective_second_point = s21_mult_matrix(&trans_sp, &perspective_matrix);
        cairo_move_to(cr, ro * (perspective_first_point.matrix[0][0] / perspective_first_point.matrix[0][2]),
                      ro * (perspective_first_point.matrix[0][1] / perspective_first_point.matrix[0][2]));
        cairo_line_to(cr,
                      ro * (perspective_second_point.matrix[0][0] / perspective_second_point.matrix[0][2]),
                      ro * (perspective_second_point.matrix[0][1] / perspective_second_point.matrix[0][2]));
        s21_remove_matrix(&perspective_matrix);
        s21_remove_matrix(&trans_fp);
        s21_remove_matrix(&trans_sp);
        s21_remove_matrix(&perspective_first_point);
        s21_remove_matrix(&perspective_second_point);
    } else {
        cairo_move_to(cr, -first_point.matrix[0][0] + first_point.matrix[1][0] / (2 * sqrt(2)),
                      first_point.matrix[2][0] + first_point.matrix[1][0] / (2 * sqrt(2)));
        cairo_line_to(cr, -second_point.matrix[0][0] + second_point.matrix[1][0] / (2 * sqrt(2)),
                      second_point.matrix[2][0] + second_point.matrix[1][0] / (2 * sqrt(2)));
    }
    cairo_stroke(cr);
}

void draw_arc(matrix_t point, long double ro, int perspective, int size, cairo_t *cr) {
    if (perspective) {
        cairo_arc(cr, ro * (point.matrix[0][0] / point.matrix[0][2]),
                  ro * (point.matrix[0][1] / point.matrix[0][2]), size / X_SCALE, 0.0, 2 * M_PI);
    } else {
        cairo_arc(cr, -point.matrix[0][0] + point.matrix[1][0] / (2 * sqrt(2)),
                  point.matrix[2][0] + point.matrix[1][0] / (2 * sqrt(2)), size / X_SCALE, 0.0, 2 * M_PI);
    }
    cairo_fill(cr);
}

void draw_rec(matrix_t point, long double ro, int perspective, long double size, cairo_t *cr) {
    if (perspective) {
        cairo_rectangle(cr, ro * (point.matrix[0][0] / point.matrix[0][2]) - (size / X_SCALE),
                        ro * (point.matrix[0][1] / point.matrix[0][2]) - (size / Y_SCALE),
                        (size / X_SCALE) * 2, (size / Y_SCALE) * 2);
    } else {
        cairo_rectangle(cr, -point.matrix[0][0] + point.matrix[1][0] / (2 * sqrt(2)) - (size / X_SCALE),
                        point.matrix[2][0] + point.matrix[1][0] / (2 * sqrt(2)) - (size / Y_SCALE),
                        (size / X_SCALE) * 2, (size / Y_SCALE) * 2);
    }
    cairo_fill(cr);
}

void draw_points(matrix_t *points, cairo_t *cr, int count, int point_type, long double point_size,
                 int perspective) {
    matrix_t perspective_matrix = s21_create_matrix(0, 0);
    long double ro = 0.0L;
    if (perspective) {
        ro = sqrtl(powl(X_VIEW, 2) + powl(Y_VIEW, 2) + powl(Z_VIEW, 2));
        long double teta = atanl(sqrtl(powl(X_VIEW, 2) + powl(Y_VIEW, 2)) / Z_VIEW);
        long double fi = atanl(Y_VIEW / X_VIEW);
        perspective_matrix = create_perspective_matrix(ro, teta, fi);
    }
    matrix_t trans_p = s21_create_matrix(0, 0);
    matrix_t perspective_p = s21_create_matrix(0, 0);
    for (int i = 0; i < count; i++) {
        matrix_t draw_point;
        if (perspective) {
            trans_p = s21_transpose(&points[i]);
            perspective_p = s21_mult_matrix(&trans_p, &perspective_matrix);
            draw_point = perspective_p;
        } else {
            draw_point = points[i];
        }
        if (point_type == 1) {
            draw_rec(draw_point, ro, perspective, point_size, cr);
        } else if (point_type == 2) {
            draw_arc(draw_point, ro, perspective, point_size, cr);
        }
        cairo_stroke(cr);
        s21_remove_matrix(&trans_p);
        s21_remove_matrix(&perspective_p);
    }
    s21_remove_matrix(&perspective_matrix);
}

void draw_surface(surface_t surface, matrix_t *points, int perspective, cairo_t *cr) {
    for (int i = 0; i < surface.point_count - 1; i++) {
        connect_points(points[surface.ver_t[i] - 1], points[surface.ver_t[i + 1] - 1], cr, perspective);
    }
    connect_points(points[surface.ver_t[surface.point_count - 1] - 1], points[surface.ver_t[0] - 1], cr,
                   perspective);
}

void draw_model(surface_t *surfaces, matrix_t *points, int surface_count, int point_count, color_t v_color,
                color_t s_color, int perspective, int dot_type, long double dot_size,
                long double rib_thickness, int dashed, cairo_t *cr) {
    cairo_set_line_width(cr, rib_thickness / 100.0);
    cairo_set_source_rgb(cr, s_color.red_color, s_color.green_color, s_color.blue_color);
    static const double dashes[] = {1 / X_SCALE, 3 / X_SCALE};
    if (dashed) {
        static int len = sizeof(dashes) / sizeof(dashes[0]);
        cairo_set_dash(cr, dashes, len, 0);
    }
    for (int i = 0; i < surface_count; i++) {
        draw_surface(surfaces[i], points, perspective, cr);
    }
    cairo_set_dash(cr, dashes, 0, 0);
    cairo_set_line_width(cr, 0.05);
    cairo_set_source_rgb(cr, v_color.red_color, v_color.green_color, v_color.blue_color);
    draw_points(points, cr, point_count, dot_type, dot_size, perspective);
}
