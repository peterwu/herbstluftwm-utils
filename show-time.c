#include <gtk/gtk.h>

const int screen_width = 1920;
const int width = 360;
const int height = 360;
const int posy = 30;
const int posx = (screen_width - width) / 2;

const char *clocks[] = {
        "America/Toronto",
        "America/Vancouver",
        "Asia/Shanghai"
};

const char *
get_city_name(const char *p) {
    while (p != NULL && *p != '/') p++;
    return ++p;
}

void
create_window() {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *calendar;

    const struct tm *tm;
    time_t t;
    char *markup;
    char buffer[64];
    const int buffer_size = sizeof(buffer) / sizeof(buffer[0]);

    // main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "show time");
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);

    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_NORMAL);
    gtk_window_move(GTK_WINDOW(window), posx, posy);

    g_signal_connect(G_OBJECT(window), "focus-out-event", G_CALLBACK(gtk_main_quit), NULL);

    //vbox
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // banner
    t = time(NULL);
    tm = localtime(&t);
    strftime(buffer, buffer_size, "%A", tm);

    label = gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(label), buffer);
    gtk_label_set_xalign(GTK_LABEL(label), 0.03);
    gtk_container_add(GTK_CONTAINER(vbox), label);

    label = gtk_label_new(NULL);
    strftime(buffer, buffer_size, "%B %e %Y", tm);
    markup = g_markup_printf_escaped("<b><big>%s</big></b>",
                                     buffer);

    gtk_label_set_markup(GTK_LABEL(label), markup);
    gtk_label_set_xalign(GTK_LABEL(label), 0.03);

    gtk_container_add(GTK_CONTAINER(vbox), label);
    g_free(markup);

    // separator
    gtk_container_add(GTK_CONTAINER(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // calendar
    calendar = gtk_calendar_new();
    gtk_calendar_set_display_options(GTK_CALENDAR(calendar),
                                     GTK_CALENDAR_SHOW_HEADING
                                     | GTK_CALENDAR_SHOW_DAY_NAMES
                                     | GTK_CALENDAR_SHOW_WEEK_NUMBERS);

    gtk_container_add(GTK_CONTAINER(vbox), calendar);

    // separator
    gtk_container_add(GTK_CONTAINER(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // World clocks
    label = gtk_label_new(NULL);
    markup = g_markup_printf_escaped("<b>%s</b>", "World Clocks");
    gtk_label_set_markup(GTK_LABEL(label), markup);
    gtk_label_set_xalign(GTK_LABEL(label), 0.03);

    gtk_container_add(GTK_CONTAINER(vbox), label);
    g_free(markup);

    // separator
    gtk_container_add(GTK_CONTAINER(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // list clocks
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(vbox), grid);

    for (int i = 0; i < 3; i++) {
        setenv("TZ", clocks[i], 1);
        t = time(NULL);
        tm = localtime(&t);

        // city name
        snprintf(buffer, buffer_size, "  %-23s", get_city_name(clocks[i]));
        label = gtk_label_new(NULL);
        gtk_label_set_text(GTK_LABEL(label), buffer);
        gtk_label_set_xalign(GTK_LABEL(label), 0);
        gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);

        // local time
        char tmp[buffer_size];
        strftime(tmp, buffer_size, "%H:%M", tm);
        snprintf(buffer, buffer_size, "%11s", tmp);
        label = gtk_label_new(NULL);
        gtk_label_set_text(GTK_LABEL(label), buffer);
        gtk_label_set_xalign(GTK_LABEL(label), 1);
        gtk_grid_attach(GTK_GRID(grid), label, 1, i, 1, 1);

        // gmt offset
        snprintf(buffer, buffer_size, "%+19d", tm->tm_gmtoff / 3600);
        label = gtk_label_new(NULL);
        label = gtk_label_new(NULL);
        gtk_label_set_text(GTK_LABEL(label), buffer);
        gtk_label_set_xalign(GTK_LABEL(label), 1);
        gtk_grid_attach(GTK_GRID(grid), label, 2, i, 1, 1);
    }

    gtk_widget_show_all(GTK_WIDGET(window));
}

gint
main(gint argc, gchar *argv[]) {
    setenv("GTK_THEME", "Adwaita:dark", 1);

    gtk_init(&argc, &argv);
    gdk_set_program_class("hlwm-show-time");
    create_window();
    gtk_main();

    return 0;
}
