#include "util.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <systemd/sd-bus.h>
#include <unistd.h>

int pscanf(const char *path, const char *fmt, ...) {
    FILE *fp;
    va_list ap;
    int n;

    if (!(fp = fopen(path, "r"))) {
        return -1;
    }

    va_start(ap, fmt);
    n = vfscanf(fp, fmt, ap);
    va_end(ap);
    fclose(fp);

    return (n == EOF) ? -1 : n;
}

int pprintf(const char *path, const char *fmt, ...) {
    FILE *fp;
    va_list ap;
    int n;

    if (!(fp = fopen(path, "w"))) {
        return -1;
    }

    va_start(ap, fmt);
    n = vfprintf(fp, fmt, ap);
    va_end(ap);
    fclose(fp);

    return (n == EOF) ? -1 : n;
}

uint32_t notify_send(struct notification_t *n) {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;
    sd_bus *bus = NULL;
    uint32_t id;
    unsigned int r;

    r = sd_bus_default_user(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to user bus: %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_call_method(bus,
                           "org.freedesktop.Notifications",
                           "/org/freedesktop/Notifications",
                           "org.freedesktop.Notifications",
                           "Notify",
                           &error,
                           &m,
                           "susssasa{sv}i",
                           n->app_name,
                           0,
                           n->icon,
                           n->summary,
                           n->body,
                           0, /* no actions */
                           3, /* 3 k-v pairs */
                           "urgency", "y", n->urgency,
                           "value", "i", n->value,
                           "x-dunst-stack-tag", "s", n->app_name,
                           n->timeout);
    if (r < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", error.message);
        goto finish;
    }

    /* Parse the response message */
    r = sd_bus_message_read(m, "u", &id);
    if (r < 0) {
        fprintf(stderr, "Failed to parse response message: %s\n", strerror(-r));
        goto finish;
    }
finish:
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_unref(bus);

    return id;
}

void notify_close(unsigned int id) {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus *bus = NULL;
    unsigned int r;

    r = sd_bus_open_user(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_call_method(bus,
                           "org.freedesktop.Notifications",
                           "/org/freedesktop/Notifications",
                           "org.freedesktop.Notifications",
                           "CloseNotification",
                           &error,
                           NULL,
                           "u",
                           id);
    if (r < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", error.message);
        goto finish;
    }

finish:
    sd_bus_error_free(&error);
    sd_bus_unref(bus);
}

void systemctl_suspend(){
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus *bus = NULL;
    unsigned int r;

    r= sd_bus_open_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_call_method(bus,
                           "org.freedesktop.login1",
                           "/org/freedesktop/login1",
                           "org.freedesktop.login1.Manager",
                           "Suspend",
                           &error,
                           NULL,
                           "b",
                           true);
    if (r < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", error.message);
        goto finish;
    }

finish:
    sd_bus_error_free(&error);
    sd_bus_unref(bus);
}
