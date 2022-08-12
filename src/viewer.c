#include <gtk/gtk.h>
#include <magickwand/magickwand.h>
#include <string.h>
#include <time.h>

#include "athenianTransform.h"
#include "fileReader.h"
#include "graphics.h"

static GtkWidget *main_window = NULL;
static gboolean open_flag = false;
static gboolean record_opened = false;

struct settings_struct {
    guint projection_type;
    guint line_type;
    double edge_thickness;
    guint dot_type;
    double vertex_size;
    GdkRGBA background_color;
    GdkRGBA edge_color;
    GdkRGBA vertex_color;
};

typedef struct settings_widgets_struct {
    GtkWidget *projection_type;
    GtkWidget *background_color;
    GtkWidget *line_type;
    GtkWidget *edge_color;
    GtkWidget *edge_thickness;
    GtkWidget *dot_type;
    GtkWidget *vertex_color;
    GtkWidget *vertex_size;
} settings_widgets_t;

typedef struct entries_struct {
    GtkWidget *entry_x;
    GtkWidget *entry_y;
    GtkWidget *entry_z;
    GtkWidget *entry_scale;
} entries_t;

typedef struct file_pack {
    GtkWidget *label;
    GtkWidget *draw_area;
    matrix_t *vert;
    matrix_t *vertNorm;
    matrix_t *vertTex;
    surface_t *surfaces;
    count_v *counts;
    int file_type;
    int gif_count;
} pack_t;

typedef struct struct_pack {
    pack_t *pack;
    entries_t *entries;
    settings_t *settings;
    settings_widgets_t *widget_values;
} str_pack_t;

static void set_text(GtkWidget *entry, const gchar *str) {
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
    gtk_entry_buffer_set_text(buffer, str, strlen(str));
}

static void set_start_value(GtkWidget *entry) { set_text(entry, "0"); }

static void about_btn_clicked(const GtkWidget *button, gpointer user_data) {
    if (button) {
    } else {
    }
    if (open_flag) {
        pack_t *pack = user_data;
        GtkWidget *label = pack->label;
        surface_t *surfaces = pack->surfaces;
        int countVert = pack->counts->surfaceCount;
        char *count = (char *)malloc(sizeof(char) * 10);
        GtkBuilder *builder = gtk_builder_new_from_file("about_builder.ui");
        GObject *window = gtk_builder_get_object(builder, "about_window");
        GObject *close_btn = gtk_builder_get_object(builder, "close_btn");
        GObject *name_label = gtk_builder_get_object(builder, "name_obj_label_value");
        gtk_label_set_label(GTK_LABEL(name_label), gtk_label_get_label(GTK_LABEL(label)));
        snprintf(count, sizeof(char) * 10, "%d", pack->counts->vertCount);
        GObject *vertices_value = gtk_builder_get_object(builder, "vertices_value");
        gtk_label_set_label(GTK_LABEL(vertices_value), count);
        snprintf(count, sizeof(char) * 10, "%d", countEdges(surfaces, countVert));
        GObject *edges_value = gtk_builder_get_object(builder, "edges_value");
        gtk_label_set_label(GTK_LABEL(edges_value), count);
        free(count);
        const gchar *font_size = "window {\n\tfont-size:20px;\n}";
        GtkStyleProvider *provider = GTK_STYLE_PROVIDER(gtk_css_provider_new());
        gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider), font_size, strlen(font_size));
        gtk_style_context_add_provider(gtk_widget_get_style_context(GTK_WIDGET(window)), provider, G_MAXUINT);
        g_object_unref(provider);

        gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(main_window));

        g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(gtk_window_destroy), window);
        gtk_widget_show(GTK_WIDGET(window));
        g_object_unref(builder);
    }
}

char *getFilename(char *fileType) {
    long int cur_time = time(NULL);
    struct tm m_time, *time_info;
    char time_string[9];
    time_info = localtime_r(&cur_time, &m_time);
    strftime(time_string, 9, "%H:%M:%S", time_info);
    char *filename = (char *)calloc(22, sizeof(char));
    strncat(filename, "record_", 8);
    strncat(filename, time_string, 10);
    strncat(filename, fileType, 5);
    return filename;
}

void create_file_for_gif(cairo_surface_t *surface, pack_t *pack) {
    char *filename = (char *)malloc(sizeof(char) * 20);
    snprintf(filename, sizeof(char) * 20, "objects/record%d.png", pack->gif_count);
    pack->gif_count++;
    cairo_surface_write_to_png(surface, filename);
    free(filename);
}

void create_gif(pack_t *pack) {
    MagickWand *wand;
    wand = NewMagickWand();
    for (int i = 0; i < pack->gif_count; i++) {
        char *name = (char *)malloc(sizeof(char) * 20);
        snprintf(name, sizeof(char) * 20, "objects/record%d.png", i);
        MagickReadImage(wand, name);
        remove(name);
        free(name);
    }
    char *filename = getFilename(".gif");
    MagickWriteImages(wand, filename, MagickTrue);
    DestroyMagickWand(wand);
    free(filename);
}

void draw_background_color(cairo_t *cr, color_t color, int width, int height) {
    cairo_set_line_width(cr, width);
    cairo_set_source_rgb(cr, color.red_color, color.green_color, color.blue_color);
    cairo_move_to(cr, width / 2, 0);
    cairo_line_to(cr, width / 2, height);
    cairo_stroke(cr);
}

static double get_color_value(const gchar *color_value_str, const guint color_type) {
    double result = 0;
    char *start;
    if (color_type == 'r')
        start = strchr(color_value_str, '(');
    else if (color_type == 'g')
        start = strchr(color_value_str, ',');
    else
        start = strrchr(color_value_str, ',');
    sscanf(start + 1, "%lf", &result);
    return result / 255.0;
}

void apply_settings(str_pack_t *str_pack, cairo_t *cr, int width, int height) {
    pack_t *pack = str_pack->pack;
    settings_t *settings = str_pack->settings;
    color_t background_color;
    char *color_string = gdk_rgba_to_string(&((*settings).background_color));
    background_color.red_color = get_color_value(color_string, 'r');
    background_color.green_color = get_color_value(color_string, 'g');
    background_color.blue_color = get_color_value(color_string, 'b');
    free(color_string);
    draw_background_color(cr, background_color, width, height);
    cairo_set_line_width(cr, 0.05);
    cairo_translate(cr, width / 2, height / 2);
    cairo_scale(cr, X_SCALE, -Y_SCALE);
    color_t v_color;
    color_string = gdk_rgba_to_string(&((*settings).vertex_color));
    v_color.red_color = get_color_value(color_string, 'r');
    v_color.green_color = get_color_value(color_string, 'g');
    v_color.blue_color = get_color_value(color_string, 'b');
    free(color_string);
    color_t s_color;
    color_string = gdk_rgba_to_string(&((*settings).edge_color));
    s_color.red_color = get_color_value(color_string, 'r');
    s_color.green_color = get_color_value(color_string, 'g');
    s_color.blue_color = get_color_value(color_string, 'b');
    free(color_string);
    int perspective = (int)str_pack->settings->projection_type;
    int dot_type = (int)str_pack->settings->dot_type;
    long double dot_size = str_pack->settings->vertex_size;
    long double edge_thickness = str_pack->settings->edge_thickness;
    int dashed = (int)str_pack->settings->line_type;

    draw_model(pack->surfaces, pack->vert, pack->counts->surfaceCount, pack->counts->vertCount, v_color,
               s_color, perspective, dot_type, dot_size, edge_thickness, dashed, cr);
}

static void draw_function(const GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data) {
    if (area) {
    } else {
    }
    cairo_surface_t *surface = cairo_get_target(cr);
    str_pack_t *str_pack = data;
    pack_t *pack = str_pack->pack;
    apply_settings(str_pack, cr, width, height);
    if (pack->file_type == 1) {
        char *filename = getFilename(".jpeg");
        cairo_surface_write_to_png(surface, filename);
        free(filename);
        pack->file_type = 0;
    } else if (pack->file_type == 2) {
        char *filename = getFilename(".bmp");
        cairo_surface_write_to_png(surface, filename);
        free(filename);
        pack->file_type = 0;
    } else if (pack->file_type == 3) {
        create_file_for_gif(surface, pack);
    } else if (pack->file_type == 4) {
        create_gif(pack);
        pack->file_type = 0;
        pack->gif_count = 0;
    }
}

static void open_file_response(GtkNativeDialog *dialog, int response, gpointer user_data) {
    if (response == GTK_RESPONSE_ACCEPT) {
        str_pack_t *str_pack = user_data;
        pack_t *pack = str_pack->pack;

        GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
        char *path = g_file_get_path(file);
        char *name = g_file_get_basename(file);
        gtk_label_set_text(GTK_LABEL(pack->label), name);
        freeVerticesMatrix(&(pack->vert), &(pack->vertNorm), &(pack->vertTex), &(pack->surfaces),
                           pack->counts->vertCount, pack->counts->vertNormCount, pack->counts->vertTexCount,
                           pack->counts->surfaceCount);
        pack->counts->vertCount = 0;
        pack->counts->vertNormCount = 0;
        pack->counts->vertTexCount = 0;
        pack->counts->surfaceCount = 0;
        readFile(&(pack->vert), &(pack->vertNorm), &(pack->vertTex), &(pack->surfaces), path, pack->counts);

        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(pack->draw_area),
                                       (GtkDrawingAreaDrawFunc)draw_function, str_pack, NULL);

        g_object_unref(file);
        g_free(path);
        g_free(name);
        open_flag = true;
    }
    gtk_native_dialog_destroy(dialog);
}

static void open_btn_clicked(GtkWidget *button, gpointer user_data) {
    GtkFileChooserNative *dialog;
    GtkFileFilter *filter;

    dialog = gtk_file_chooser_native_new("Select an obj file", GTK_WINDOW(gtk_widget_get_root(button)),
                                         GTK_FILE_CHOOSER_ACTION_OPEN, NULL, NULL);
    filter = gtk_file_filter_new();
    gtk_file_filter_add_suffix(filter, "obj");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    g_object_unref(filter);
    gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(dialog), TRUE);
    g_signal_connect(dialog, "response", G_CALLBACK(open_file_response), user_data);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog));
}

const gchar *get_entry_text(GtkWidget *entry) {
    return gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry)));
}

guint is_number(guint ch) { return ch >= '0' && ch <= '9'; }

static guint check_input_str(const gchar *str) {
    guint error = 0;
    guint len = strlen(str);
    guint dot_err = 0;
    for (guint i = 0; i < len; i++) {
        if (i == 0) {
            if (str[i] != '-' && !is_number(str[i])) error = 1;
        } else if (!is_number(str[i]) && str[i] != '.') {
            error = 1;
        }
    }
    int dot_count = 0;
    for (guint i = 0; i < len; i++) {
        if (str[i] == '.') {
            if (!is_number(str[i - 1])) dot_err = 1;
            dot_count++;
        }
    }
    if (dot_count > 1) dot_err = 1;
    return error + dot_err;
}

static void close_window(GtkWidget *window, gpointer user_data) {
    if (window == main_window) {
        str_pack_t *str_pack = user_data;
        pack_t *pack = str_pack->pack;
        g_free(str_pack->entries);
        g_free(str_pack->pack->counts);
        if (open_flag) {
            freeVerticesMatrix(&(pack->vert), &(pack->vertNorm), &(pack->vertTex), &(pack->surfaces),
                               pack->counts->vertCount, pack->counts->vertNormCount,
                               pack->counts->vertTexCount, pack->counts->surfaceCount);
        }
        g_free(str_pack->pack);
        g_free(str_pack->settings);
        g_free(str_pack);
    }
    gtk_window_destroy(GTK_WINDOW(window));
}

static void show_errors(entries_t *entries, guint *errors) {
    const gchar *error = "Error";
    if (errors[0]) set_text(entries->entry_x, error);
    if (errors[1]) set_text(entries->entry_y, error);
    if (errors[2]) set_text(entries->entry_z, error);
}

static void draw_after(str_pack_t *str_pack) {
    if (strlen(gtk_label_get_text(GTK_LABEL(str_pack->pack->label))))
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(str_pack->pack->draw_area),
                                       (GtkDrawingAreaDrawFunc)draw_function, str_pack, NULL);
}

static void save_jpeg(const GtkWidget *button, gpointer user_data) {
    if (button) {
    } else {
    }
    str_pack_t *str_pack = user_data;
    str_pack->pack->file_type = 1;
    draw_after(str_pack);
}

static void save_bmp(const GtkWidget *button, gpointer user_data) {
    if (button) {
    } else {
    }
    str_pack_t *str_pack = user_data;
    str_pack->pack->file_type = 2;
    draw_after(str_pack);
}

static const gchar *get_btn_name(GtkWidget *button) { return gtk_button_get_label(GTK_BUTTON(button)); }

static void save_gif(GtkWidget *button, gpointer user_data) {
    str_pack_t *str_pack = user_data;
    const gchar *btn_name = get_btn_name(button);
    if (g_str_equal(btn_name, "Save gif")) record_opened = true;
    if (str_pack->pack->file_type != 3) {
        str_pack->pack->file_type = 3;
        str_pack->pack->gif_count = 0;
    } else {
        str_pack->pack->file_type = 4;
    }
    draw_after(str_pack);
}

static void record_btn_clicked(const GtkWidget *button, gpointer user_data) {
    if (button) {
    } else {
    }
    if (open_flag) {
        GtkBuilder *builder = gtk_builder_new_from_file("record_builder.ui");
        GObject *window = gtk_builder_get_object(builder, "record_window");
        gtk_widget_show(GTK_WIDGET(window));

        GObject *save_jpeg_btn = gtk_builder_get_object(builder, "save_jpeg_btn");
        GObject *save_bmp_btn = gtk_builder_get_object(builder, "save_bmp_btn");
        GObject *save_gif_btn = gtk_builder_get_object(builder, "save_gif_btn");
        if (record_opened) {
            gtk_button_set_label(GTK_BUTTON(save_gif_btn), "Stop");
            record_opened = false;
        }
        g_signal_connect(save_jpeg_btn, "clicked", G_CALLBACK(save_jpeg), user_data);
        g_signal_connect(save_bmp_btn, "clicked", G_CALLBACK(save_bmp), user_data);
        g_signal_connect(save_gif_btn, "clicked", G_CALLBACK(save_gif), user_data);
        g_signal_connect_swapped(save_jpeg_btn, "clicked", G_CALLBACK(close_window), window);
        g_signal_connect_swapped(save_bmp_btn, "clicked", G_CALLBACK(close_window), window);
        g_signal_connect_swapped(save_gif_btn, "clicked", G_CALLBACK(close_window), window);

        g_object_unref(builder);
    }
}

static guint check_entries(const gchar *x_str, const gchar *y_str, const gchar *z_str, guint *errors) {
    guint x_err = check_input_str(x_str);
    guint y_err = check_input_str(y_str);
    guint z_err = check_input_str(z_str);
    errors[0] = x_err;
    errors[1] = y_err;
    errors[2] = z_err;
    return x_err + y_err + z_err;
}

static void move_obj(const gchar *x_str, const gchar *y_str, const gchar *z_str, str_pack_t *str_pack) {
    pack_t *pack = str_pack->pack;
    double x, y, z;
    sscanf(x_str, "%lf", &x);
    sscanf(y_str, "%lf", &y);
    sscanf(z_str, "%lf", &z);
    move_model(pack->vert, pack->counts->vertCount, x, y, z);
    draw_after(str_pack);
}

static void rotate_obj(const gchar *x_str, const gchar *y_str, const gchar *z_str, str_pack_t *str_pack) {
    double x, y, z;
    sscanf(x_str, "%lf", &x);
    sscanf(y_str, "%lf", &y);
    sscanf(z_str, "%lf", &z);
    pack_t *pack = str_pack->pack;
    rotate_model(pack->vert, pack->counts->vertCount, x, y, z);
    draw_after(str_pack);
}

static void move_rotate_btn_clicked(GtkWidget *button, gpointer user_data) {
    str_pack_t *str_pack = user_data;
    entries_t *entries = str_pack->entries;
    guint *errors = calloc(3, sizeof(guint));
    const gchar *x_str = get_entry_text(entries->entry_x);
    const gchar *y_str = get_entry_text(entries->entry_y);
    const gchar *z_str = get_entry_text(entries->entry_z);
    guint has_err = check_entries(x_str, y_str, z_str, errors);
    if ((!has_err) && (strlen(x_str) || strlen(y_str) || strlen(z_str))) {
        const gchar *btn_name = gtk_label_get_text(GTK_LABEL(gtk_button_get_child(GTK_BUTTON(button))));
        if (btn_name[0] == 'M')
            move_obj(x_str, y_str, z_str, str_pack);
        else
            rotate_obj(x_str, y_str, z_str, str_pack);
    } else {
        show_errors(entries, errors);
    }
    free(errors);
}

static void scale_obj(const gchar *scale_value_str, str_pack_t *str_pack) {
    double x;
    sscanf(scale_value_str, "%lf", &x);
    pack_t *pack = str_pack->pack;
    resize_model(pack->vert, pack->counts->vertCount, x, x, x);
    draw_after(str_pack);
}

static void scale_btn_clicked(const GtkWidget *button, gpointer user_data) {
    if (button) {
    } else {
    }
    str_pack_t *str_pack = user_data;
    entries_t *entries = str_pack->entries;
    const gchar *scale_value_str = get_entry_text(GTK_WIDGET(entries->entry_scale));
    const guint has_err = check_input_str(scale_value_str);
    if ((!has_err && !strchr(scale_value_str, '-')) && strlen(scale_value_str))
        scale_obj(scale_value_str, str_pack);
    else
        set_text(GTK_WIDGET(entries->entry_scale), "Error");
}

static void clear_btn_clicked(const GtkWidget *button, GtkWidget *entry) {
    if (button) {
    } else {
    }
    set_start_value(entry);
}

static void write_to_file(settings_t *settings) {
    FILE *file = fopen("settings", "wb");
    fwrite(settings, sizeof(settings_t), 1, file);
    fclose(file);
}

static void settings_window_response(GtkDialog *dialog, int response, gpointer user_data) {
    if (response == GTK_RESPONSE_OK) {
        str_pack_t *str_pack = user_data;
        GdkRGBA background_color, edge_color, vertex_color;
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(str_pack->widget_values->background_color),
                                   &background_color);
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(str_pack->widget_values->edge_color), &edge_color);
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(str_pack->widget_values->vertex_color), &vertex_color);
        str_pack->settings->projection_type =
            gtk_drop_down_get_selected(GTK_DROP_DOWN(str_pack->widget_values->projection_type));
        str_pack->settings->line_type =
            gtk_drop_down_get_selected(GTK_DROP_DOWN(str_pack->widget_values->line_type));
        str_pack->settings->dot_type =
            gtk_drop_down_get_selected(GTK_DROP_DOWN(str_pack->widget_values->dot_type));
        str_pack->settings->edge_thickness =
            gtk_spin_button_get_value(GTK_SPIN_BUTTON(str_pack->widget_values->edge_thickness));
        str_pack->settings->vertex_size =
            gtk_spin_button_get_value(GTK_SPIN_BUTTON(str_pack->widget_values->vertex_size));
        str_pack->settings->background_color = background_color;
        str_pack->settings->edge_color = edge_color;
        str_pack->settings->vertex_color = vertex_color;
        write_to_file(str_pack->settings);
        draw_after(str_pack);
    }
    gtk_window_destroy(GTK_WINDOW(dialog));
}

static void set_settings_on_widgets(settings_t *settings, settings_widgets_t *widgets) {
    FILE *file = fopen("settings", "rb");
    if (file) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(widgets->projection_type), settings->projection_type);
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(widgets->background_color),
                                   &((*settings).background_color));
        gtk_drop_down_set_selected(GTK_DROP_DOWN(widgets->line_type), settings->line_type);
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(widgets->edge_color), &((*settings).edge_color));
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->edge_thickness), settings->edge_thickness);
        gtk_drop_down_set_selected(GTK_DROP_DOWN(widgets->dot_type), settings->dot_type);
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(widgets->vertex_color), &((*settings).vertex_color));
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->vertex_size), settings->vertex_size);
        fclose(file);
    }
}

static void settings_btn_clicked(const GtkWidget *button, gpointer user_data) {
    if (button) {
    } else {
    }
    GtkBuilder *builder = gtk_builder_new_from_file("settings_builder.ui");
    GObject *window = gtk_builder_get_object(builder, "settings_window");
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(main_window));
    str_pack_t *str_pack = user_data;

    GObject *edge_adj = gtk_builder_get_object(builder, "edge_spin_adj");
    GObject *vertex_adj = gtk_builder_get_object(builder, "vertex_spin_adj");
    gtk_adjustment_set_value(GTK_ADJUSTMENT(edge_adj), 1);
    gtk_adjustment_set_value(GTK_ADJUSTMENT(vertex_adj), 1);

    static settings_widgets_t widget_values;
    widget_values.projection_type = GTK_WIDGET(gtk_builder_get_object(builder, "projection_type"));
    widget_values.background_color = GTK_WIDGET(gtk_builder_get_object(builder, "background_color_btn"));
    widget_values.line_type = GTK_WIDGET(gtk_builder_get_object(builder, "line_type"));
    widget_values.edge_color = GTK_WIDGET(gtk_builder_get_object(builder, "edge_color_btn"));
    widget_values.edge_thickness = GTK_WIDGET(gtk_builder_get_object(builder, "edge_thickness_spin"));
    widget_values.dot_type = GTK_WIDGET(gtk_builder_get_object(builder, "dot_type"));
    widget_values.vertex_color = GTK_WIDGET(gtk_builder_get_object(builder, "vertex_color_btn"));
    widget_values.vertex_size = GTK_WIDGET(gtk_builder_get_object(builder, "vertex_size_spin"));
    str_pack->widget_values = &widget_values;
    set_settings_on_widgets(str_pack->settings, str_pack->widget_values);
    g_signal_connect_data(window, "response", G_CALLBACK(settings_window_response), str_pack, NULL, 0);
    g_object_add_weak_pointer(window, (gpointer *)&window);
    gtk_widget_show(GTK_WIDGET(window));
}

static void load_settings(settings_t *settings, const GtkWidget *draw_area) {
    if (draw_area) {
    } else {
    }
    FILE *file = fopen("settings", "rb");
    if (file != NULL) {
        fread(settings, sizeof(settings_t), 1, file);
        fclose(file);
    }
}

static void activate_buttons(GtkBuilder *builder, str_pack_t *str_pack) {
    GObject *open_btn = gtk_builder_get_object(builder, "open_btn");
    GObject *settings_btn = gtk_builder_get_object(builder, "settings_btn");
    GObject *move_btn = gtk_builder_get_object(builder, "move_btn");
    GObject *rotate_btn = gtk_builder_get_object(builder, "rotate_btn");
    GObject *scale_btn = gtk_builder_get_object(builder, "scale_btn");
    GObject *about_btn = gtk_builder_get_object(builder, "info_btn");
    GObject *clear_x_btn = gtk_builder_get_object(builder, "clear_x");
    GObject *clear_y_btn = gtk_builder_get_object(builder, "clear_y");
    GObject *clear_z_btn = gtk_builder_get_object(builder, "clear_z");
    GObject *clear_scale_btn = gtk_builder_get_object(builder, "clear_scale");
    GObject *record_btn = gtk_builder_get_object(builder, "record_btn");

    g_signal_connect(record_btn, "clicked", G_CALLBACK(record_btn_clicked), str_pack);
    g_signal_connect(clear_x_btn, "clicked", G_CALLBACK(clear_btn_clicked),
                     GTK_WIDGET(str_pack->entries->entry_x));
    g_signal_connect(clear_y_btn, "clicked", G_CALLBACK(clear_btn_clicked),
                     GTK_WIDGET(str_pack->entries->entry_y));
    g_signal_connect(clear_z_btn, "clicked", G_CALLBACK(clear_btn_clicked),
                     GTK_WIDGET(str_pack->entries->entry_z));
    g_signal_connect(clear_scale_btn, "clicked", G_CALLBACK(clear_btn_clicked),
                     GTK_WIDGET(str_pack->entries->entry_scale));
    g_signal_connect(settings_btn, "clicked", G_CALLBACK(settings_btn_clicked), str_pack);
    g_signal_connect(open_btn, "clicked", G_CALLBACK(open_btn_clicked), str_pack);
    g_signal_connect(move_btn, "clicked", G_CALLBACK(move_rotate_btn_clicked), str_pack);
    g_signal_connect(rotate_btn, "clicked", G_CALLBACK(move_rotate_btn_clicked), str_pack);
    g_signal_connect(scale_btn, "clicked", G_CALLBACK(scale_btn_clicked), str_pack);
    g_signal_connect(about_btn, "clicked", G_CALLBACK(about_btn_clicked), str_pack->pack);
}

static str_pack_t *init_str_pack(GtkWidget *label, GtkWidget *draw_area, entries_t *entries) {
    matrix_t *vert = NULL;
    matrix_t *vertNorm = NULL;
    matrix_t *vertTex = NULL;
    surface_t *surfaces = NULL;
    count_v *counts = g_new(count_v, 1);
    counts->vertNormCount = 0;
    counts->vertTexCount = 0;
    counts->vertCount = 0;
    counts->surfaceCount = 0;

    pack_t *pack = g_new(pack_t, 1);
    pack->label = label;
    pack->draw_area = draw_area;
    pack->vert = vert;
    pack->vertNorm = vertNorm;
    pack->vertTex = vertTex;
    pack->surfaces = surfaces;
    pack->counts = counts;
    pack->file_type = 0;

    str_pack_t *str_pack = g_new(str_pack_t, 1);
    str_pack->entries = entries;
    str_pack->pack = pack;
    return str_pack;
}

static void activate(GtkApplication *app, gpointer user_data) {
    if (user_data) {
    } else {
    }
    GtkBuilder *builder = gtk_builder_new_from_file("main_builder.ui");
    GObject *window = gtk_builder_get_object(builder, "window");
    gtk_window_set_application(GTK_WINDOW(window), app);
    g_object_add_weak_pointer(window, (gpointer *)&window);
    main_window = GTK_WIDGET(window);
    GObject *file_label = gtk_builder_get_object(builder, "file_label");
    GObject *draw_area = gtk_builder_get_object(builder, "draw_area");
    GObject *entry_x = gtk_builder_get_object(builder, "entry_x");
    GObject *entry_y = gtk_builder_get_object(builder, "entry_y");
    GObject *entry_z = gtk_builder_get_object(builder, "entry_z");
    GObject *entry_scale = gtk_builder_get_object(builder, "entry_scale");

    entries_t *entries = g_new(entries_t, 1);
    entries->entry_x = GTK_WIDGET(entry_x);
    entries->entry_y = GTK_WIDGET(entry_y);
    entries->entry_z = GTK_WIDGET(entry_z);
    entries->entry_scale = GTK_WIDGET(entry_scale);
    set_start_value(entries->entry_x);
    set_start_value(entries->entry_y);
    set_start_value(entries->entry_z);
    set_start_value(GTK_WIDGET(entry_scale));

    str_pack_t *str_pack = init_str_pack(GTK_WIDGET(file_label), GTK_WIDGET(draw_area), entries);

    settings_t *settings = g_new(settings_t, 1);

    load_settings(settings, GTK_WIDGET(draw_area));

    str_pack->settings = settings;

    activate_buttons(builder, str_pack);

    g_signal_connect(window, "destroy", G_CALLBACK(close_window), str_pack);
    g_object_unref(builder);

    gtk_widget_show(GTK_WIDGET(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("s21.viewer.app", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
