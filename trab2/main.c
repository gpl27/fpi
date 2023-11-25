/*!
 * TODO:
 * change Image struct to be just the Pixbuf*
 * see where histogram must be calculated and grayscale copies of
 * images must be made
 */
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#define IMGPROC_IMPLEMENTATION
#include "imgproc.h"


typedef struct {
    GtkWidget *src_window;
    GdkPixbuf *original_img;
    GtkWidget *img_to_match_widget;
    GtkWidget *hist_to_match_widget;
    GdkPixbuf *img_to_match;
    GtkWidget *out_window;
    GdkPixbuf *output_img;
    GtkWidget *q_amount;
    GtkWidget *b_amount;
    GtkWidget *c_amount;
    GtkWidget *kernel_grid;
} AppData;

void load_image(char *filename, AppData *metadata) {
    // Unref previous data
    if (metadata->original_img != NULL) {
        g_object_unref(metadata->original_img);
    }
    if (metadata->output_img != NULL) {
        g_object_unref(metadata->output_img);
    }

    // Load image data
    metadata->original_img = gdk_pixbuf_new_from_file(filename, NULL);
    metadata->output_img = gdk_pixbuf_copy(metadata->original_img);
}

void save_image(char *filename, AppData *metadata) {
    gdk_pixbuf_save(metadata->output_img, filename, "jpeg", NULL, NULL);
}

void load_img_to_match(char *filename, AppData *metadata) {
    // Unref previous data
    if (metadata->img_to_match != NULL) {
        g_object_unref(metadata->img_to_match);
    }
    // Load image data
    metadata->img_to_match = gdk_pixbuf_new_from_file(filename, NULL);
    rgb_to_l(metadata->img_to_match);
}

void place_image_outwindow(AppData *metadata) {
    GtkWidget *img = gtk_picture_new_for_pixbuf(metadata->output_img);
    int width = gdk_pixbuf_get_width(metadata->output_img);
    int height = gdk_pixbuf_get_height(metadata->output_img);
    gtk_window_set_default_size(GTK_WINDOW(metadata->out_window), width, height);
    gtk_window_set_child(GTK_WINDOW(metadata->out_window), img);
}

void place_image_srcwindow(AppData *metadata) {
    GtkWidget *img = gtk_picture_new_for_pixbuf(metadata->original_img);
    int width = gdk_pixbuf_get_width(metadata->original_img);
    int height = gdk_pixbuf_get_height(metadata->original_img);
    gtk_window_set_default_size(GTK_WINDOW(metadata->src_window), width, height);
    gtk_window_set_child(GTK_WINDOW(metadata->src_window), img);
}

void place_img_to_match(AppData *metadata) {
    GdkPixbuf *hist = create_histogram_img(metadata->img_to_match, 0);
    gtk_picture_set_pixbuf(GTK_PICTURE(metadata->img_to_match_widget), metadata->img_to_match);
    gtk_picture_set_pixbuf(GTK_PICTURE(metadata->hist_to_match_widget), hist);
    // unref hist?
}

void open_file_dialog_response(GtkDialog *dialog, int response, AppData *metadata) {
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        GFile *file = gtk_file_chooser_get_file (chooser);
        char *filename = g_file_get_path(file);
        load_image(filename, metadata);
        place_image_srcwindow(metadata);
        place_image_outwindow(metadata);
        free(filename);
        g_object_unref(file);
    }

    gtk_window_destroy (GTK_WINDOW (dialog));
}

void save_file_dialog_response(GtkDialog *dialog, int response, AppData *metadata) {
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        GFile *file = gtk_file_chooser_get_file (chooser);
        char *filename = g_file_get_path(file);
        save_image(filename, metadata);
        free(filename);
        g_object_unref(file);
    }

    gtk_window_destroy (GTK_WINDOW (dialog));
}

void hmatchsel_dialog_response(GtkDialog *dialog, int response, AppData *metadata) {
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        GFile *file = gtk_file_chooser_get_file (chooser);
        char *filename = g_file_get_path(file);
        load_img_to_match(filename, metadata);
        place_img_to_match(metadata);
        free(filename);
        g_object_unref(file);
    }

    gtk_window_destroy (GTK_WINDOW (dialog));
}

void load_button_click(GtkWidget* widget, AppData *metadata) {
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new ("Open File",
                                          GTK_WINDOW(metadata->src_window),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          "_Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "_Open",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    g_signal_connect(dialog, "response",
                     G_CALLBACK (open_file_dialog_response),
                     metadata);
    gtk_window_present(GTK_WINDOW (dialog));
}

void reset_button_click(GtkWidget* widget, AppData *metadata) {
    if (metadata->original_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    g_object_unref(metadata->output_img);
    metadata->output_img = gdk_pixbuf_copy(metadata->original_img);
    place_image_outwindow(metadata);
}

void save_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new ("Save File",
                                          GTK_WINDOW(metadata->out_window),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          "_Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "_Save",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "Untitled.jpg");
    g_signal_connect(dialog, "response",
                     G_CALLBACK (save_file_dialog_response),
                     metadata);
    gtk_window_present(GTK_WINDOW (dialog));
}

void hflip_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    hflip(metadata->output_img);
    place_image_outwindow(metadata);
}

void vflip_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    vflip(metadata->output_img);
    place_image_outwindow(metadata);
}

void gray_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    rgb_to_l(metadata->output_img);
    place_image_outwindow(metadata);
}

void neg_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    negative(metadata->output_img);
    place_image_outwindow(metadata);
}

void q_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    rgb_to_l(metadata->output_img);
    int q = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(metadata->q_amount));
    l_quantize(metadata->output_img, q);
    place_image_outwindow(metadata);
}

void b_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    int b = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(metadata->b_amount));
    brightness(metadata->output_img, b);
    place_image_outwindow(metadata);
}

void c_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    double c = gtk_spin_button_get_value(GTK_SPIN_BUTTON(metadata->c_amount));
    contrast(metadata->output_img, c);
    place_image_outwindow(metadata);
}

void rotr_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    metadata->output_img = rotate_right90(metadata->output_img);
    place_image_outwindow(metadata);
}

void rotl_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    metadata->output_img = rotate_left90(metadata->output_img);
    place_image_outwindow(metadata);
}

void zin_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    metadata->output_img = zoom_in(metadata->output_img);
    place_image_outwindow(metadata);
}

void zout_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    int sx = 2;
    int sy = 2;
    metadata->output_img = zoom_out(metadata->output_img, sx, sy);
    place_image_outwindow(metadata);
}

void gauss_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    double kernel[3][3] = {0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625};
    convolute(metadata->output_img, kernel);
    place_image_outwindow(metadata);
}

void lapl_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    double kernel[3][3] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
    convolute(metadata->output_img, kernel);
    place_image_outwindow(metadata);
}

void hpgen_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    double kernel[3][3] = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
    convolute(metadata->output_img, kernel);
    place_image_outwindow(metadata);
}

void prex_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    double kernel[3][3] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
    convolute(metadata->output_img, kernel);
    place_image_outwindow(metadata);
}

void prey_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    double kernel[3][3] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
    convolute(metadata->output_img, kernel);
    place_image_outwindow(metadata);
}

void sobx_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    double kernel[3][3] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    convolute(metadata->output_img, kernel);
    place_image_outwindow(metadata);
}

void soby_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    double kernel[3][3] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    convolute(metadata->output_img, kernel);
    place_image_outwindow(metadata);
}

void hshow_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }

    GdkPixbuf *orig_hist = create_histogram_img(metadata->original_img, 0);
    GtkWidget *orig_hist_pic = gtk_picture_new_for_pixbuf(orig_hist);
    GtkWidget *orig_hist_win = gtk_window_new();
    gtk_window_set_child(GTK_WINDOW(orig_hist_win), orig_hist_pic);
    gtk_window_set_title(GTK_WINDOW(orig_hist_win), "Source Histogram");
    gtk_window_set_default_size(GTK_WINDOW(orig_hist_win), 300, 256);

    GdkPixbuf *out_hist = create_histogram_img(metadata->output_img, 0);
    GtkWidget *out_hist_pic = gtk_picture_new_for_pixbuf(out_hist);
    GtkWidget *out_hist_win = gtk_window_new();
    gtk_window_set_child(GTK_WINDOW(out_hist_win), out_hist_pic);
    gtk_window_set_title(GTK_WINDOW(out_hist_win), "Output Histogram");
    gtk_window_set_default_size(GTK_WINDOW(out_hist_win), 300, 256);


    gtk_widget_show(orig_hist_win);
    gtk_widget_show(out_hist_win);
    // unref pixbufs on window destroy (create callback)
}

void hequ_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    histogram_equalization(metadata->output_img);
    place_image_outwindow(metadata);
}

void hmatchsel_button_click(GtkWidget *widget, AppData *metadata) {
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new ("Open File",
                                          GTK_WINDOW(metadata->src_window),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          "_Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "_Open",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    g_signal_connect(dialog, "response",
                     G_CALLBACK (hmatchsel_dialog_response),
                     metadata);
    gtk_window_present(GTK_WINDOW (dialog));

}

void hmatch_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL || metadata->img_to_match == NULL) {
        g_print("No image loaded\n");
        return;
    }
    histogram_matching(metadata->output_img, metadata->img_to_match);
    place_image_outwindow(metadata);

}

void conv_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->output_img == NULL) {
        g_print("No image loaded\n");
        return;
    }
    GtkWidget *entry;
    double kernel[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            entry = gtk_grid_get_child_at(GTK_GRID(metadata->kernel_grid), j, i);
            kernel[i][j] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(entry));
        }
    }
    convolute(metadata->output_img, kernel);
    place_image_outwindow(metadata);

}

static void activate(GtkApplication *app, AppData *metadata) {

    GtkWidget *tool_window, *src_window, *out_window;
    GtkWidget *load_button, *reset_button, *save_button,
              *hflip_button, *vflip_button, *gray_button,
              *neg_button, *q_amount, *q_button,
              *b_amount, *b_button, *c_amount,
              *c_button, *rotr_button, *rotl_button,
              *zin_button, *zout_button, *gauss_button,
              *lapl_button, *hpgen_button, *prex_button,
              *prey_button, *sobx_button, *soby_button,
              *hshow_button, *hequ_button, *hmatchsel_button,
              *hmatch_button, *img_to_match, *hist_to_match,
              *kernel_grid, *conv_button;
    GtkBuilder *builder = gtk_builder_new_from_file("main.ui");
    tool_window = GTK_WIDGET(gtk_builder_get_object(builder, "tools"));
    src_window = GTK_WIDGET(gtk_builder_get_object(builder, "source"));
    out_window = GTK_WIDGET(gtk_builder_get_object(builder, "output"));
    load_button = GTK_WIDGET(gtk_builder_get_object(builder, "load-button"));
    reset_button = GTK_WIDGET(gtk_builder_get_object(builder, "reset-button"));
    save_button = GTK_WIDGET(gtk_builder_get_object(builder, "save-button"));
    hflip_button = GTK_WIDGET(gtk_builder_get_object(builder, "hflip-button"));
    vflip_button = GTK_WIDGET(gtk_builder_get_object(builder, "vflip-button"));
    gray_button = GTK_WIDGET(gtk_builder_get_object(builder, "gray-button"));
    neg_button = GTK_WIDGET(gtk_builder_get_object(builder, "neg-button"));
    q_amount = GTK_WIDGET(gtk_builder_get_object(builder, "q-amount"));
    q_button = GTK_WIDGET(gtk_builder_get_object(builder, "q-button"));
    b_amount = GTK_WIDGET(gtk_builder_get_object(builder, "b-amount"));
    b_button = GTK_WIDGET(gtk_builder_get_object(builder, "b-button"));
    c_amount = GTK_WIDGET(gtk_builder_get_object(builder, "c-amount"));
    c_button = GTK_WIDGET(gtk_builder_get_object(builder, "c-button"));
    rotr_button = GTK_WIDGET(gtk_builder_get_object(builder, "rotr-button"));
    rotl_button = GTK_WIDGET(gtk_builder_get_object(builder, "rotl-button"));
    zin_button = GTK_WIDGET(gtk_builder_get_object(builder, "zin-button"));
    zout_button = GTK_WIDGET(gtk_builder_get_object(builder, "zout-button"));
    gauss_button = GTK_WIDGET(gtk_builder_get_object(builder, "gauss-button"));
    lapl_button = GTK_WIDGET(gtk_builder_get_object(builder, "lapl-button"));
    hpgen_button = GTK_WIDGET(gtk_builder_get_object(builder, "hpgen-button"));
    prex_button = GTK_WIDGET(gtk_builder_get_object(builder, "prex-button"));
    prey_button = GTK_WIDGET(gtk_builder_get_object(builder, "prey-button"));
    sobx_button = GTK_WIDGET(gtk_builder_get_object(builder, "sobx-button"));
    soby_button = GTK_WIDGET(gtk_builder_get_object(builder, "soby-button"));
    hshow_button = GTK_WIDGET(gtk_builder_get_object(builder, "hshow-button"));
    hequ_button = GTK_WIDGET(gtk_builder_get_object(builder, "hequ-button"));
    hmatchsel_button = GTK_WIDGET(gtk_builder_get_object(builder, "hmatchsel-button"));
    hmatch_button = GTK_WIDGET(gtk_builder_get_object(builder, "hmatch-button"));
    img_to_match = GTK_WIDGET(gtk_builder_get_object(builder, "img-to-match"));
    hist_to_match = GTK_WIDGET(gtk_builder_get_object(builder, "hist-to-match"));
    kernel_grid = GTK_WIDGET(gtk_builder_get_object(builder, "kernel-grid"));
    conv_button = GTK_WIDGET(gtk_builder_get_object(builder, "conv-button"));

    gtk_window_set_application(GTK_WINDOW(tool_window), app);

    metadata->src_window = src_window;
    metadata->out_window = out_window;
    metadata->img_to_match_widget = img_to_match;
    metadata->hist_to_match_widget = hist_to_match;
    metadata->kernel_grid = kernel_grid;
    metadata->q_amount = q_amount;
    metadata->b_amount = b_amount;
    metadata->c_amount = c_amount;

    g_signal_connect(load_button, "clicked", G_CALLBACK(load_button_click), metadata);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(reset_button_click), metadata);
    g_signal_connect(save_button, "clicked", G_CALLBACK(save_button_click), metadata);
    g_signal_connect(hflip_button, "clicked", G_CALLBACK(hflip_button_click), metadata);
    g_signal_connect(vflip_button, "clicked", G_CALLBACK(vflip_button_click), metadata);
    g_signal_connect(gray_button, "clicked", G_CALLBACK(gray_button_click), metadata);
    g_signal_connect(neg_button, "clicked", G_CALLBACK(neg_button_click), metadata);
    g_signal_connect(q_button, "clicked", G_CALLBACK(q_button_click), metadata);
    g_signal_connect(b_button, "clicked", G_CALLBACK(b_button_click), metadata);
    g_signal_connect(c_button, "clicked", G_CALLBACK(c_button_click), metadata);
    g_signal_connect(rotr_button, "clicked", G_CALLBACK(rotr_button_click), metadata);
    g_signal_connect(rotl_button, "clicked", G_CALLBACK(rotl_button_click), metadata);
    g_signal_connect(zin_button, "clicked", G_CALLBACK(zin_button_click), metadata);
    g_signal_connect(zout_button, "clicked", G_CALLBACK(zout_button_click), metadata);
    g_signal_connect(gauss_button, "clicked", G_CALLBACK(gauss_button_click), metadata);
    g_signal_connect(lapl_button, "clicked", G_CALLBACK(lapl_button_click), metadata);
    g_signal_connect(hpgen_button, "clicked", G_CALLBACK(hpgen_button_click), metadata);
    g_signal_connect(prex_button, "clicked", G_CALLBACK(prex_button_click), metadata);
    g_signal_connect(prey_button, "clicked", G_CALLBACK(prey_button_click), metadata);
    g_signal_connect(sobx_button, "clicked", G_CALLBACK(sobx_button_click), metadata);
    g_signal_connect(soby_button, "clicked", G_CALLBACK(soby_button_click), metadata);
    g_signal_connect(hshow_button, "clicked", G_CALLBACK(hshow_button_click), metadata);
    g_signal_connect(hequ_button, "clicked", G_CALLBACK(hequ_button_click), metadata);
    g_signal_connect(hmatchsel_button, "clicked", G_CALLBACK(hmatchsel_button_click), metadata);
    g_signal_connect(hmatch_button, "clicked", G_CALLBACK(hmatch_button_click), metadata);
    g_signal_connect(conv_button, "clicked", G_CALLBACK(conv_button_click), metadata);


    gtk_widget_show(tool_window);
    gtk_widget_show(src_window);
    gtk_widget_show(out_window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // Init application state
    AppData metadata;
    metadata.original_img = NULL;
    metadata.output_img = NULL;
    metadata.img_to_match = NULL;

    app = gtk_application_new("dev.gpl27.fpi.trab1", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), &metadata);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    // Destroy application state

    return status;
}
