#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-bus.h>

/*
 * ensure to add user to the video group
 * sudo usermod -a -G video `whoami`
 *
 * add the following udev rules
 * /etc/udev/rules/90-backlight.rules
 *
 * SUBSYSTEM=="backlight", ACTION=="add",
 * RUN+="/bin/chgrp video /sys/class/backlight/intel_backlight/brightness",
 * RUN+="/bin/chmod g+w   /sys/class/backlight/intel_backlight/brightness"
 */


/*
 * https://www.kernel.org/doc/html/latest/gpu/backlight.html
 */

#define BACKLIGHT_PATH "/sys/class/backlight/intel_backlight"
#define BRIGHTNESS_PATH BACKLIGHT_PATH "/brightness"
#define ACTUAL_BRIGHTNESS_PATH BACKLIGHT_PATH "/actual_brightness"
#define MAX_BRIGHTNESS_PATH BACKLIGHT_PATH "/max_brightness"

int get_max_brightness() {
    int brightness;

    if (pscanf(MAX_BRIGHTNESS_PATH, "%d", &brightness) != 1)
        return -1;

    return brightness;
}

int get_brightness() {
    int brightness;

    if (pscanf(ACTUAL_BRIGHTNESS_PATH, "%d", &brightness) != 1)
        return -1;

    return brightness;
}

int set_brightness(int brightness) {
    if (pprintf(BRIGHTNESS_PATH, "%d", brightness) < 0)
        return -1;

    return 0;
}

int main(int argc, char *argv[]) {
    int brightness;
    int delta;
    const int max_brightness = get_max_brightness();

    if (argc == 1) delta = 0;
    else
        /* delta is in percentage */
        delta = atoi(argv[1]) * max_brightness / 100;

    if (delta == 0) return 0;

    brightness = get_brightness() + delta;
    brightness = MAX(0, MIN(brightness, max_brightness)); /* [0, max_brightness] */
    set_brightness(brightness);

    /* send notification */
    const int brightness_in_percentage = brightness * 100 / max_brightness;
    const int buffer_size = 32;
    const char app_name[] = "herbstluftwm-brightnessctl";
    char summary[buffer_size];

    snprintf(summary, buffer_size, "Brightness: %d%%", brightness_in_percentage);

    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus *bus = NULL;
    int r;

    enum { LOW,
           NORMAL,
           CRITICAL };

    r = sd_bus_open_user(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_call_method(bus,
                           "org.freedesktop.Notifications",
                           "/org/freedesktop/Notifications",
                           "org.freedesktop.Notifications",
                           "Notify",
                           &error,
                           NULL,
                           "susssasa{sv}i",
                           app_name,
                           0,
                           "dialog-information",
                           summary,
                           NULL,
                           0,
                           3,
                           "urgency", "y", LOW,
                           "value", "i", brightness_in_percentage,
                           "x-dunst-stack-tag", "s", app_name,
                           1500);
    if (r < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", error.message);
        goto finish;
    }

finish:
    sd_bus_error_free(&error);
    sd_bus_unref(bus);

    return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}