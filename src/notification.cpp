#include "notification.h"
#include <systemd/sd-bus.h>

namespace wbd::utils {

void notification::send()
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message* m = NULL;
    sd_bus* bus = NULL;
    unsigned int r;

    r = sd_bus_default_user(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to user bus: %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_call_method(
        bus, "org.freedesktop.Notifications", "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications", "Notify", &error, &m, "susssasa{sv}i",
        app_name.c_str(), 0, icon.c_str(), summary.c_str(), body.c_str(),
        0, /* no actions */
        3, /* 3 k-v pairs */
        "urgency", "y", urgency, "value", "i", value, "x-dunst-stack-tag", "s",
        app_name.c_str(), timeout);
    if (r < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", error.message);
        goto finish;
    }

    /* Parse the response message */
    r = sd_bus_message_read(m, "u", &id_);
    if (r < 0) {
        fprintf(stderr, "Failed to parse response message: %s\n", strerror(-r));
        goto finish;
    }
finish:
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_unref(bus);
}

void notification::close()
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus* bus = NULL;
    unsigned int r;

    r = sd_bus_open_user(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_call_method(bus, "org.freedesktop.Notifications",
                           "/org/freedesktop/Notifications",
                           "org.freedesktop.Notifications", "CloseNotification",
                           &error, NULL, "u", id_);
    if (r < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", error.message);
        goto finish;
    }

finish:
    sd_bus_error_free(&error);
    sd_bus_unref(bus);
}

} // namespace wbd::utils