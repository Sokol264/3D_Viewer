#ifndef SRC_GRAPHICS_H_
#define SRC_GRAPHICS_H_

#include <gtk/gtk.h>

#include "fileReader.h"
#include "s21_matrix.h"

#define X_SCALE 50.0
#define Y_SCALE 50.0

typedef struct color_struct {
    long double red_color;
    long double green_color;
    long double blue_color;
} color_t;

typedef struct settings_struct settings_t;

void draw_model(surface_t *surfaces, matrix_t *points, int surface_count, int point_count, color_t v_color,
                color_t s_color, int perspective, int dot_type, long double dot_size,
                long double rib_thickness, int dashed, cairo_t *cr);
#endif  // SRC_GRAPHICS_H_
