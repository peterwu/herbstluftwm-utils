#include <gtk/gtk.h>

const int screen_width = 1920;
const int width = 360;
const int height = 360;
const int posy = 30;
const int posx = (screen_width - width) / 2;

void create_window() {}

gint main(gint argc, gchar *argv[]) {
  setenv("GTK_THEME", "Adwaita:dark", 1);

  gtk_init(&argc, &argv);
  gdk_set_program_class("hlwm-show-tray");
  create_window();
  gtk_main();

  return 0;
}