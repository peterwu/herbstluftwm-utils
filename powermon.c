#include "util.h"
#include <libudev.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-bus.h>
#include <unistd.h>

/* pwr ac_online batt_perc */
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
    int ac_online = 0;
    int battery_now = 0;
    int battery_max = 0;

    struct udev *udev;
    struct udev_device *dev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;

    /* create udev object */
    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Cannot create udev context.\n");
        return;
    }

    /* create enumerate object */
    enumerate = udev_enumerate_new(udev);
    if (!enumerate) {
        fprintf(stderr, "Cannot create enumerate context.\n");
        return;
    }

    udev_enumerate_add_match_subsystem(enumerate, "power_supply");
    udev_enumerate_scan_devices(enumerate);

    /* fillup device list */
    devices = udev_enumerate_get_list_entry(enumerate);
    if (!devices) {
        fprintf(stderr, "Failed to get device list.\n");
        return;
    }

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path, *type;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        type = udev_device_get_sysattr_value(dev, "type");

        if (!strcmp(type, "Mains")) {
            ac_online = atoi(udev_device_get_sysattr_value(dev, "online"));
        } else if (!strcmp(type, "Battery")) {
            int now, max;

            now = atoi(udev_device_get_sysattr_value(dev, "energy_now"));
            battery_now += now;

            max = atoi(udev_device_get_sysattr_value(dev, "energy_full"));
            battery_max += max;
        }
    }

    udev_device_unref(dev);
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    battery_now /= 1000;
    battery_max /= 1000;

    sprintf(result, RESULT_FMT, ac_online, battery_now * 100 / battery_max);
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


static void *run_print_loop(void *) {
    for (;;) {
        print_power_supply();
        sleep(PRINT_INTERVAL);
    }

    return NULL;
}

static void *run_udev_mon_loop(void *) {
    struct udev *udev;
    struct udev_device *dev;
    struct udev_monitor *mon;
    int fd;

    /* create udev object */
    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Can't create udev\n");
        return NULL;
    }

    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "power_supply", NULL);
    udev_monitor_enable_receiving(mon);
    fd = udev_monitor_get_fd(mon);

    for (;;) {
        fd_set fds;
        struct timeval tv;
        int ret;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd + 1, &fds, NULL, NULL, &tv);
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            dev = udev_monitor_receive_device(mon);
            if (dev) {
                print_power_supply();
                udev_device_unref(dev);
            }
        }
        /* 500 milliseconds */
        usleep(500 * 1000);
    }

    /* free udev */
    udev_unref(udev);
}

int main(int argc, char *argv[]) {
    pthread_t t1, t2;
    int err;

    err = pthread_create(&t1, NULL, run_print_loop, NULL);
    if (err != 0) {
        printf("\ncan't create thread :[%s]", strerror(err));
        return EXIT_FAILURE;
    }

    err = pthread_create(&t2, NULL, run_udev_mon_loop, NULL);
    if (err != 0) {
        printf("\ncan't create thread :[%s]", strerror(err));
        return EXIT_FAILURE;
    }

    return run_sd_bus_loop();
}