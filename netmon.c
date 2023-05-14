#include <dirent.h>
#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <systemd/sd-bus.h>
#include <unistd.h>

/* net conneted? eth_or_wifi */
#define RESULT_FMT "net\t%d\t%d"

enum {
    ETHERNET,
    WIRELESS
};

static void print_network_status() {
    int connected = 0;
    int network_type = -1;

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

    udev_enumerate_add_match_subsystem(enumerate, "net");
    udev_enumerate_scan_devices(enumerate);

    /* fillup device list */
    devices = udev_enumerate_get_list_entry(enumerate);
    if (!devices) {
        fprintf(stderr, "Failed to get device list.\n");
        return;
    }

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path, *operstate;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        operstate = udev_device_get_sysattr_value(dev, "operstate");

        if (!strcmp(operstate, "up")) {
            char wireless_subdir[_POSIX_PATH_MAX + 1];
            struct stat fstat;

            connected = 1;

            sprintf(wireless_subdir, "%s/wireless", path);
            if (stat(wireless_subdir, &fstat))
                network_type = ETHERNET;
            else
                network_type = WIRELESS;
        }
    }

    udev_device_unref(dev);
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    printf(RESULT_FMT, connected, network_type);
    fflush(stdout);
}

static void run_udev_mon_loop() {
    struct udev *udev;
    struct udev_device *dev;
    struct udev_monitor *mon;
    int fd;

    /* create udev object */
    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Can't create udev\n");
        return;
    }

    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "net", NULL);
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
                print_network_status();
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
    print_network_status();

    run_udev_mon_loop();

    return 0;
}