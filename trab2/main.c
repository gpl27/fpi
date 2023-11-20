#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#define IMGPROC_IMPLEMENTATION
#include "imgproc.h"


typedef struct {
    gboolean grayscale;
    GdkPixbuf *original;
    GdkPixbuf *output;
    unsigned char *hist;
} Image;

typedef struct {
    Image image;
    GtkWidget *src_window;
    GtkWidget *out_window;
    GtkWidget *q_amount;
    GtkWidget *b_amount;
    GtkWidget *c_amount;
    GtkWidget **filter;
} AppData;

void load_image(char *filename, AppData *metadata) {
    // Unref previous data
    if (metadata->image.original != NULL) {
        g_object_unref(metadata->image.original);
    }
    if (metadata->image.output != NULL) {
        g_object_unref(metadata->image.output);
    }

    // Load image data
    metadata->image.original = gdk_pixbuf_new_from_file(filename, NULL);
    metadata->image.output = gdk_pixbuf_copy(metadata->image.original);
    if (gdk_pixbuf_get_n_channels(metadata->image.original) == 1)
        metadata->image.grayscale = TRUE;
    else
        metadata->image.grayscale = FALSE;
}

void save_image(char *filename, AppData *metadata) {
    gdk_pixbuf_save(metadata->image.output, filename, "jpeg", NULL, NULL);
}

void place_image_outwindow(AppData *metadata) {
    GtkWidget *img = gtk_picture_new_for_pixbuf(metadata->image.output);
    int width = gdk_pixbuf_get_width(metadata->image.output);
    int height = gdk_pixbuf_get_height(metadata->image.output);
    gtk_window_set_default_size(GTK_WINDOW(metadata->out_window), width, height);
    gtk_window_set_child(GTK_WINDOW(metadata->out_window), img);
}

void place_image_srcwindow(AppData *metadata) {
    GtkWidget *img = gtk_picture_new_for_pixbuf(metadata->image.original);
    int width = gdk_pixbuf_get_width(metadata->image.original);
    int height = gdk_pixbuf_get_height(metadata->image.original);
    gtk_window_set_default_size(GTK_WINDOW(metadata->src_window), width, height);
    gtk_window_set_child(GTK_WINDOW(metadata->src_window), img);
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
    if (metadata->image.original == NULL) {
        g_print("No image loaded\n");
        return;
    }
    g_object_unref(metadata->image.output);
    metadata->image.output = gdk_pixbuf_copy(metadata->image.original);
    metadata->image.grayscale = FALSE;
    place_image_outwindow(metadata);
}

void save_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->image.output == NULL) {
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
    if (metadata->image.output == NULL) {
        g_print("No image loaded\n");
        return;
    }
    hflip(metadata->image.output);
    place_image_outwindow(metadata);
}

void vflip_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->image.output == NULL) {
        g_print("No image loaded\n");
        return;
    }
    vflip(metadata->image.output);
    place_image_outwindow(metadata);
}

void gray_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->image.output == NULL) {
        g_print("No image loaded\n");
        return;
    }
    rgb_to_l(metadata->image.output);
    place_image_outwindow(metadata);
    metadata->image.grayscale = TRUE;
}

void neg_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->image.output == NULL) {
        g_print("No image loaded\n");
        return;
    }
    negative(metadata->image.output);
    place_image_outwindow(metadata);
}

void q_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->image.output == NULL) {
        g_print("No image loaded\n");
        return;
    }
    if (!metadata->image.grayscale) {
        rgb_to_l(metadata->image.output);
        metadata->image.grayscale = TRUE;
    }
    int q = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(metadata->q_amount));
    l_quantize(metadata->image.output, q);
    place_image_outwindow(metadata);
}

void b_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->image.output == NULL) {
        g_print("No image loaded\n");
        return;
    }
    int b = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(metadata->b_amount));
    brightness(metadata->image.output, b);
    place_image_outwindow(metadata);
}

void c_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->image.output == NULL) {
        g_print("No image loaded\n");
        return;
    }
    double c = gtk_spin_button_get_value(GTK_SPIN_BUTTON(metadata->c_amount));
    contrast(metadata->image.output, c);
    place_image_outwindow(metadata);
}

void rotr_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->image.output == NULL) {
        g_print("No image loaded\n");
        return;
    }
    metadata->image.output = rotate_right90(metadata->image.output);
    place_image_outwindow(metadata);
}

void rotl_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->image.output == NULL) {
        g_print("No image loaded\n");
        return;
    }
    metadata->image.output = rotate_left90(metadata->image.output);
    place_image_outwindow(metadata);
}

void zin_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->image.output == NULL) {
        g_print("No image loaded\n");
        return;
    }
    metadata->image.output = zoom_in(metadata->image.output);
    place_image_outwindow(metadata);
}

void zout_button_click(GtkWidget *widget, AppData *metadata) {
    if (metadata->image.output == NULL) {
        g_print("No image loaded\n");
        return;
    }
    int sx, sy;
    metadata->image.output = zoom_out(metadata->image.output, sx, sy);
    place_image_outwindow(metadata);
}

static void activate(GtkApplication *app, AppData *metadata) {

    GtkWidget *tool_window, *src_window, *out_window;
    GtkWidget *load_button, *reset_button, *save_button,
              *hflip_button, *vflip_button, *gray_button,
              *neg_button, *q_amount, *q_button,
              *b_amount, *b_button, *c_amount,
              *c_button, *rotr_button, *rotl_button,
              *zin_button, *zout_button;
    GtkBuilder *builder = gtk_builder_new_from_file("ui.xml");
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

    gtk_window_set_application(GTK_WINDOW(tool_window), app);

    metadata->src_window = src_window;
    metadata->out_window = out_window;
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

    gtk_widget_show(tool_window);
    gtk_widget_show(src_window);
    gtk_widget_show(out_window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // Init application state
    AppData metadata;
    metadata.image.original = NULL;
    metadata.image.output = NULL;
    metadata.image.hist = malloc(sizeof(unsigned char)*256);

    app = gtk_application_new("dev.gpl27.fpi.trab1", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), &metadata);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    // Destroy application state
    free(metadata.image.hist);

    return status;
}
