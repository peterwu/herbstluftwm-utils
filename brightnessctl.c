#include "util.h"
#include <stdio.h>
#include <stdlib.h>

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
    const int brightness_perc = brightness * 100 / max_brightness;
    const int buffer_size = 32;
    const char app_name[] = "herbstluftwm-brightnessctl";
    char summary[buffer_size];
    snprintf(summary, buffer_size, "Brightness: %d%%", brightness_perc);

    struct notification_t n = {
            .app_name = app_name,
            .icon = "dialog-information",
            .summary = summary,
            .body = NULL,
            .urgency = LOW,
            .value = brightness_perc,
            .timeout = 1500
    };

    notify_send(&n);
}