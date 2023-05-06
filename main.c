#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-bus.h>

int main() {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    const char *path;
    int r;

    /* Connect to the system bus */
    r = sd_bus_open_user(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        goto finish;
    }

    /* Issue the method call and store the respons message in m */
    r = sd_bus_call_method(bus,
                           "org.freedesktop.Notifications",         /* service to contact */
                           "/org/freedesktop/Notifications",        /* object path */
                           "org.freedesktop.Notifications", /* interface name */
                           "Notify",                        /* method name */
                           &error,                             /* object to return error in */
                           &m,                                 /* return message on success */
                           "susssasa{sv}i",                               /* input signature */
                           "herbstluftwm-brightnessctl",                     /* first argument */
                           0,
                           "dialog-information",
                           "summary",
                           "body",
                           0,
                           2, "value", "i", 50, "x-dunst-stack-tag", "s", "herbstluftwm-brightnessctl",
                           1500);                         /* second argument */
    if (r < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", error.message);
        goto finish;
    }

finish:
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_unref(bus);

    return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
