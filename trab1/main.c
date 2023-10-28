#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#define G_APPLICATION_DEFAULT_FLAGS G_APPLICATION_FLAGS_NONE

void load_image(char *filename, gpointer window) {
  GtkWidget *img;
  img = gtk_image_new_from_file(filename);
  gtk_window_set_child(GTK_WINDOW(window), img);
  gtk_window_set_default_size(GTK_WINDOW(window), 0, 0);
}

void on_open_response(GtkDialog *dialog, int response, gpointer in_window) {
  if (response == GTK_RESPONSE_ACCEPT) {
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
      GFile *file = gtk_file_chooser_get_file (chooser);
      char *filename = g_file_get_path(file);
      printf("Filename: %s\n", filename);
      load_image(filename, in_window);
      free(filename);
      g_object_unref(file);
    }

  gtk_window_destroy (GTK_WINDOW (dialog));
}

void on_load_button(GtkWidget* widget, gpointer in_window) {
  GtkWidget *dialog;
  dialog = gtk_file_chooser_dialog_new ("Open File",
                                      in_window,
                                      GTK_FILE_CHOOSER_ACTION_OPEN,
                                      "_Cancel",
                                      GTK_RESPONSE_CANCEL,
                                      "_Open",
                                      GTK_RESPONSE_ACCEPT,
                                      NULL);
  gtk_window_present(GTK_WINDOW (dialog));
  g_signal_connect(dialog, "response",
                    G_CALLBACK (on_open_response),
                    in_window);
}

static void activate(GtkApplication *app,
                     gpointer user_data) {
  GtkWidget *menu_window, *in_window, *out_window;
  GtkWidget *menu_box;
  GtkWidget *load_button, *save_button;
  GtkWidget *hflip_button, *vflip_button;

  menu_window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(menu_window), "Menu");
  gtk_window_set_default_size(GTK_WINDOW(menu_window), 180, 250);
  gtk_window_set_resizable(GTK_WINDOW(menu_window), FALSE);

  // int menu_x, menu_y;
  // gtk_window_get_position();

  in_window = gtk_application_window_new(app);
  gtk_window_set_transient_for(GTK_WINDOW(in_window), GTK_WINDOW(menu_window));
  gtk_window_set_title(GTK_WINDOW(in_window), "Source");
  // gtk_window_set_default_size(GTK_WINDOW(in_window), 0, 0);
  gtk_window_set_deletable(GTK_WINDOW(in_window), FALSE);
  gtk_window_set_destroy_with_parent(GTK_WINDOW(in_window), TRUE);

  out_window = gtk_application_window_new(app);
  gtk_window_set_transient_for(GTK_WINDOW(out_window), GTK_WINDOW(menu_window));
  gtk_window_set_title(GTK_WINDOW(out_window), "Output");
  // gtk_window_set_default_size(GTK_WINDOW(out_window), 200, 200);
  gtk_window_set_deletable(GTK_WINDOW(out_window), FALSE);
  gtk_window_set_destroy_with_parent(GTK_WINDOW(out_window), TRUE);

  menu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign(menu_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(menu_box, GTK_ALIGN_CENTER);

  gtk_window_set_child(GTK_WINDOW(menu_window), menu_box);

  load_button = gtk_button_new_with_label("Load Image");
  save_button = gtk_button_new_with_label("Save Image");
  hflip_button = gtk_button_new_with_label("H Flip");
  vflip_button = gtk_button_new_with_label("V Flip");

  g_signal_connect(load_button, "clicked", G_CALLBACK(on_load_button), in_window);

  gtk_box_set_spacing(GTK_BOX(menu_box), 8);
  gtk_box_append(GTK_BOX(menu_box), load_button);
  gtk_box_append(GTK_BOX(menu_box), save_button);
  gtk_box_append(GTK_BOX(menu_box), hflip_button);
  gtk_box_append(GTK_BOX(menu_box), vflip_button);

  gtk_widget_show(menu_window);
  gtk_widget_show(in_window);
  gtk_widget_show(out_window);
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("dev.gpl27.fpi.trab1", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
