#include <dirent.h>
#include <libudev.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

/* net conneted? eth_or_wifi */
#define RESULT_FMT "net\t%d\t%d"

enum {
    ETHERNET,
    WIRELESS
};

static void print_network_status() {
    static char old_result[16];
    static char new_result[16];

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

    sprintf(new_result, RESULT_FMT, connected, network_type);
    if (strncmp(old_result, new_result, 16)) {
        printf("%s\n", new_result);
        strncpy(old_result, new_result, 16);
    }
}

static void run_network_loop() {
    struct sockaddr_nl addr;
    int nls, len;
    char buffer[4096];
    struct nlmsghdr *nlh;

    if ((nls = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1) {
        printf("socket failure\n");
        return;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;

    if (bind(nls, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        printf("bind failure\n");
        return;
    }

    nlh = (struct nlmsghdr *) buffer;
    while ((len = recv(nls, nlh, 4096, 0)) > 0) {
        for (; (NLMSG_OK(nlh, len)) && (nlh->nlmsg_type != NLMSG_DONE);
             nlh = NLMSG_NEXT(nlh, len)) {
            print_network_status();
        }
    }
}

int main(int argc, char *argv[]) {
    print_network_status();
    run_network_loop();

    return 0;
}