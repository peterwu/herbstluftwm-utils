#include "util.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <systemd/sd-bus.h>
#include <unistd.h>

#define POWER_SUPPLY_PATH "/sys/class/power_supply"

/* PWR ac_online batt_perc */
#define RESULT_FMT "pwr\t%d\t%d"
/* in seconds */
#define PRINT_INTERVAL 10

#define BATTERY_THRESHOLD_LOW 10
#define BATTERY_THRESHOLD_CRITICAL_LOW 5
#define BATTERY_THRESHOLD_SUSPEND 3

/* remember the ids of low and critical low messages */
static uint32_t id_low;
static uint32_t id_critical_low;

static void read_power_supply(char *result) {
    DIR *dir;
    struct stat fstat;
    struct dirent *subdir;
    char pathname[_POSIX_PATH_MAX + 1];
    char filename[_POSIX_PATH_MAX + 1];

    int ac_online = 0;
    int battery_now = 0;
    int battery_max = 0;

    dir = opendir(POWER_SUPPLY_PATH);
    while ((subdir = readdir(dir)) != NULL) {
        snprintf(pathname,
                 _POSIX_PATH_MAX, POWER_SUPPLY_PATH "/%s",
                 subdir->d_name);

        /* Ignore special directories. */
        if ((strcmp(subdir->d_name, ".") == 0) ||
            (strcmp(subdir->d_name, "..") == 0))
            continue;

        /* Print only if it is really directory. */
        if (stat(pathname, &fstat) < 0) continue;

        /* look for type = [Mains, Battery] */
        strncpy(filename, pathname, _POSIX_PATH_MAX);
        strncat(filename, "/type", _POSIX_PATH_MAX);

        char type[16];
        pscanf(filename, "%s", type);

        if (!strcmp(type, "Mains")) {
            strncpy(filename, pathname, _POSIX_PATH_MAX);
            strncat(filename, "/online", _POSIX_PATH_MAX);

            pscanf(filename, "%d", &ac_online);
        } else if (!strcmp(type, "Battery")) {
            int now, max;

            strncpy(filename, pathname, _POSIX_PATH_MAX);
            strncat(filename, "/energy_now", _POSIX_PATH_MAX);
            pscanf(filename, "%ld", &now);
            battery_now += now;

            strncpy(filename, pathname, _POSIX_PATH_MAX);
            strncat(filename, "/energy_full", _POSIX_PATH_MAX);
            pscanf(filename, "%ld", &max);
            battery_max += max;
        }
    }

    battery_now /= 1000;
    battery_max /= 1000;

    sprintf(result, RESULT_FMT, ac_online, battery_now * 100 / battery_max);

    closedir(dir);
}

static void send_notification(const char *result) {
    int ac_online;
    int battery_perc;

    sscanf(result, RESULT_FMT, &ac_online, &battery_perc);

    const int buffer_size = 32;
    const char app_name[] = "herbstluftwm-powermon";
    char summary[buffer_size];
    char icon[256];

    if (ac_online == 1) {
        /* close all PWR notifications if any */
        notify_close(id_critical_low);
        notify_close(id_low);
    } else if (battery_perc <= BATTERY_THRESHOLD_SUSPEND) {
        /* systemctl suspend */
        systemctl_suspend();
    } else {
        snprintf(summary, buffer_size, "Battery: %d%%", battery_perc);

        struct notification_t n;
        n.app_name = app_name;
        n.summary = summary;
        n.body = NULL;
        n.value = battery_perc;
        n.icon = icon;

        if (battery_perc <= BATTERY_THRESHOLD_CRITICAL_LOW) {
            strncpy(icon, "battery-empty-symbolic", 256);

            n.urgency = CRITICAL;
            n.timeout = 0;

            id_critical_low = notify_send(&n);
        } else if (battery_perc <= BATTERY_THRESHOLD_LOW) {
            strncpy(icon, "battery-low-symbolic", 256);

            n.urgency = NORMAL;
            n.timeout = 3000;

            id_low = notify_send(&n);
        }
    }
}

static void print_power_supply() {
    static char old_result[16];
    static char new_result[16];

    read_power_supply(new_result);

    send_notification(new_result);

    if (strncmp(old_result, new_result, 16)) {
        printf("%s\n", new_result);
        strncpy(old_result, new_result, 16);
    }
}

static int method_print(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
    int r;

    r = sd_bus_message_read(m, "");
    if (r < 0) {
        fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-r));
        return r;
    }

    print_power_supply();

    return sd_bus_reply_method_return(m, NULL);
}

static const sd_bus_vtable poweron_vtable[] = {
        SD_BUS_VTABLE_START(0),
        SD_BUS_METHOD("Print", NULL, NULL, method_print, SD_BUS_VTABLE_UNPRIVILEGED),
        SD_BUS_VTABLE_END};

static int run_sd_bus_loop() {
    sd_bus_slot *slot = NULL;
    sd_bus *bus = NULL;
    int r;

    r = sd_bus_default_user(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to user bus: %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_add_object_vtable(bus,
                                 &slot,
                                 "/wbd/utils/powermon", /* object path */
                                 "wbd.utils.powermon",  /* interface name */
                                 poweron_vtable,
                                 NULL);
    if (r < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", strerror(-r));
        goto finish;
    }

    /* Take a well-known service name so that clients can find us */
    r = sd_bus_request_name(bus, "wbd.utils.powermon", 0);
    if (r < 0) {
        fprintf(stderr, "Failed to acquire service name: %s\n", strerror(-r));
        goto finish;
    }

    for (;;) {
        /* Process requests */
        r = sd_bus_process(bus, NULL);
        if (r < 0) {
            fprintf(stderr, "Failed to process bus: %s\n", strerror(-r));
            goto finish;
        }
        if (r > 0) /* we processed a request, try to process another one, right-away */
            continue;

        /* Wait for the next request to process */
        r = sd_bus_wait(bus, (uint64_t) -1);
        if (r < 0) {
            fprintf(stderr, "Failed to wait on bus: %s\n", strerror(-r));
            goto finish;
        }
    }

    finish:
    sd_bus_slot_unref(slot);
    sd_bus_unref(bus);

    return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    if (fork() == 0) {
        for (;;) {
           print_power_supply();
           sleep(PRINT_INTERVAL);
        }
    } else {
        return run_sd_bus_loop();
    }
}