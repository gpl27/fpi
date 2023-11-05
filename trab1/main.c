#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STBI_ONLY_JPEG

#include "imgproc.h"

#define JPEG_QLTY 100

#define unpack_meta(m) m->data, m->width, m->height, m->channels

typedef struct {
    gboolean grayscale;
    int width;
    int height;
    int channels;
    unsigned char *data; // NULL if no image loaded yet
    GdkPixbuf *pixbuf;
    GtkWidget *in_window;
    GtkWidget *out_window;
    GtkSpinButton *q_amount;
} ImageData;

void load_image(char *filename, ImageData *metadata) {
    // Unref previous data
    if (metadata->data != NULL) {
        free(metadata->data);
    }

    // Load image data
    metadata->data = stbi_load(filename, &(metadata->width), &(metadata->height), &(metadata->channels), 0);
    metadata->grayscale = FALSE;
}

void save_image(char *filename, ImageData *metadata) {
    stbi_write_jpg(filename, metadata->width, metadata->height, metadata->channels, metadata->data, JPEG_QLTY);
}

void convert_image_to_Pixbuf(ImageData *metadata) {
    // Unref previous data
    if (metadata->pixbuf != NULL) {
        g_object_unref(metadata->pixbuf);
    }
    GBytes* data = g_bytes_new(metadata->data, metadata->width * metadata->height * metadata->channels);
    metadata->pixbuf = gdk_pixbuf_new_from_bytes(data, GDK_COLORSPACE_RGB, FALSE, 8, metadata->width, metadata->height, metadata->width * metadata->channels);
    g_bytes_unref(data);
}

void place_image_outwindow(ImageData *metadata) {
    convert_image_to_Pixbuf(metadata);
    GtkWidget *img = gtk_picture_new_for_pixbuf(metadata->pixbuf);
    gtk_window_set_default_size(GTK_WINDOW(metadata->out_window), metadata->width, metadata->height);
    gtk_window_set_child(GTK_WINDOW(metadata->out_window), img);
}

void place_image_inwindow(char *filename, ImageData *metadata) {
    GtkWidget *img;
    img = gtk_picture_new_for_filename(filename);
    gtk_window_set_child(GTK_WINDOW(metadata->in_window), img);
    gtk_widget_set_size_request(metadata->in_window, metadata->width, metadata->height);
}

void open_file_dialog_response(GtkDialog *dialog, int response, ImageData *metadata) {
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        GFile *file = gtk_file_chooser_get_file (chooser);
        char *filename = g_file_get_path(file);
        g_print("Filename: %s\n", filename);
        load_image(filename, metadata);
        place_image_inwindow(filename, metadata);
        g_print("x:%d y:%d n:%d\n", metadata->width, metadata->height, metadata->channels);
        free(filename);
        g_object_unref(file);
    }

    gtk_window_destroy (GTK_WINDOW (dialog));
}

void save_file_dialog_response(GtkDialog *dialog, int response, ImageData *metadata) {
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        GFile *file = gtk_file_chooser_get_file (chooser);
        char *filename = g_file_get_path(file);
        g_print("New Filename: %s\n", filename);
        save_image(filename, metadata);
        free(filename);
        g_object_unref(file);
    }

    gtk_window_destroy (GTK_WINDOW (dialog));
}

void load_button_click(GtkWidget* widget, ImageData *metadata) {
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new ("Open File",
                                          GTK_WINDOW(metadata->in_window),
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

void save_button_click(GtkWidget *widget, ImageData *metadata) {
    if (metadata->data == NULL) {
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

void hflip_button_click(GtkWidget *widget, ImageData *metadata) {
    if (metadata->data == NULL) {
        g_print("No image loaded\n");
        return;
    }
    hflip(unpack_meta(metadata));
    place_image_outwindow(metadata);
}

void vflip_button_click(GtkWidget *widget, ImageData *metadata) {
    if (metadata->data == NULL) {
        g_print("No image loaded\n");
        return;
    }
    vflip(unpack_meta(metadata));
    place_image_outwindow(metadata);
}

void gray_button_click(GtkWidget *widget, ImageData *metadata) {
    if (metadata->data == NULL) {
        g_print("No image loaded\n");
        return;
    }
    rgb_to_l(unpack_meta(metadata));
    place_image_outwindow(metadata);
    metadata->grayscale = TRUE;
}

void q_button_click(GtkWidget *widget, ImageData *metadata) {
    if (metadata->data == NULL) {
        g_print("No image loaded\n");
        return;
    }
    if (!metadata->grayscale) {
        g_print("Image must be grayscale\n");
        return;
    }
    int q = gtk_spin_button_get_value_as_int(metadata->q_amount);
    g_print("q: %d\n", q);
    l_quantize(unpack_meta(metadata), q);
    place_image_outwindow(metadata);
}

static void activate(GtkApplication *app, ImageData *metadata) {
    GtkWidget *menu_window, *in_window, *out_window;
    GtkWidget *menu_box;
    GtkWidget *load_button, *save_button;
    GtkWidget *hflip_button, *vflip_button;
    GtkWidget *gray_button;
    GtkAdjustment *q_adjustment;
    GtkWidget *q_amount, *q_button;

    // Create windows
    menu_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(menu_window), "Menu");
    gtk_window_set_default_size(GTK_WINDOW(menu_window), 180, 350);
    gtk_window_set_resizable(GTK_WINDOW(menu_window), FALSE);

    in_window = gtk_application_window_new(app);
    gtk_window_set_transient_for(GTK_WINDOW(in_window), GTK_WINDOW(menu_window));
    gtk_window_set_title(GTK_WINDOW(in_window), "Source");
    gtk_window_set_deletable(GTK_WINDOW(in_window), FALSE);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(in_window), TRUE);

    out_window = gtk_application_window_new(app);
    gtk_window_set_transient_for(GTK_WINDOW(out_window), GTK_WINDOW(menu_window));
    gtk_window_set_title(GTK_WINDOW(out_window), "Output");
    gtk_window_set_deletable(GTK_WINDOW(out_window), FALSE);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(out_window), TRUE);

    // Create buttons
    menu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(menu_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(menu_box, GTK_ALIGN_CENTER);
    gtk_window_set_child(GTK_WINDOW(menu_window), menu_box);

    load_button = gtk_button_new_with_label("Load Image");
    save_button = gtk_button_new_with_label("Save Image");
    hflip_button = gtk_button_new_with_label("H Flip");
    vflip_button = gtk_button_new_with_label("V Flip");
    gray_button = gtk_button_new_with_label("Grayscale");
    q_adjustment = gtk_adjustment_new (128, 1, 256, 1, 0, 0);
    q_amount = gtk_spin_button_new (q_adjustment, 1, 0);
    q_button = gtk_button_new_with_label("Quantize");
    gtk_box_set_spacing(GTK_BOX(menu_box), 8);
    gtk_box_append(GTK_BOX(menu_box), load_button);
    gtk_box_append(GTK_BOX(menu_box), save_button);
    gtk_box_append(GTK_BOX(menu_box), hflip_button);
    gtk_box_append(GTK_BOX(menu_box), vflip_button);
    gtk_box_append(GTK_BOX(menu_box), gray_button);
    gtk_box_append(GTK_BOX(menu_box), q_amount);
    gtk_box_append(GTK_BOX(menu_box), q_button);

    // Connect callbacks
    metadata->in_window = in_window;
    metadata->out_window = out_window;
    metadata->q_amount = GTK_SPIN_BUTTON(q_amount);
    g_signal_connect(load_button, "clicked", G_CALLBACK(load_button_click), metadata);
    g_signal_connect(save_button, "clicked", G_CALLBACK(save_button_click), metadata);
    g_signal_connect(hflip_button, "clicked", G_CALLBACK(hflip_button_click), metadata);
    g_signal_connect(vflip_button, "clicked", G_CALLBACK(vflip_button_click), metadata);
    g_signal_connect(gray_button, "clicked", G_CALLBACK(gray_button_click), metadata);
    g_signal_connect(q_button, "clicked", G_CALLBACK(q_button_click), metadata);

    gtk_widget_show(menu_window);
    gtk_widget_show(in_window);
    gtk_widget_show(out_window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // Init application state
    ImageData metadata;
    metadata.data = NULL;
    metadata.pixbuf = NULL;

    app = gtk_application_new("dev.gpl27.fpi.trab1", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), &metadata);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
