#include "network_monitor.h"

#include <cstring>
#include <iostream>
#include <libudev.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <thread>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

namespace wbd::monitor {

void network_monitor::print_network_status()
{
    network_status_t ns{.connected = false, .type = NETWORK_TYPE::UNKNOWN};

    struct udev* udev;
    struct udev_device* dev;
    struct udev_enumerate* enumerate;
    struct udev_list_entry *devices, *dev_list_entry;

    /* create udev object */
    udev = udev_new();
    if (!udev) {
        std::cerr << "Cannot create udev context.\n";
        return;
    }

    /* create enumerate object */
    enumerate = udev_enumerate_new(udev);
    if (!enumerate) {
        std::cerr << "Cannot create enumerate context.\n";
        return;
    }

    udev_enumerate_add_match_subsystem(enumerate, "net");
    udev_enumerate_scan_devices(enumerate);

    /* fillup device list */
    devices = udev_enumerate_get_list_entry(enumerate);
    if (!devices) {
        std::cerr << "Failed to get device list.\n";
        return;
    }

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        std::string path{udev_list_entry_get_name(dev_list_entry)};
        dev = udev_device_new_from_syspath(udev, path.c_str());
        std::string operstate{udev_device_get_sysattr_value(dev, "operstate")};

        if (operstate == "up") {
            std::string wireless_subdir;
            struct stat fstat;

            ns.connected = true;

            wireless_subdir = path + "/wireless";
            if (stat(wireless_subdir.c_str(), &fstat))
                ns.type = NETWORK_TYPE::ETHERNET;
            else
                ns.type = NETWORK_TYPE::WIRELESS;
        }
    }

    udev_device_unref(dev);
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    printer_.print("net", ns.connected, static_cast<int>(ns.type));
}

void network_monitor::run_network_loop()
{
    struct sockaddr_nl addr;
    int nls, len;
    char buffer[4096];
    struct nlmsghdr* nlh;

    if ((nls = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1) {
        std::cerr << "socket failure\n";
        return;
    }

    std::memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;

    if (bind(nls, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "bind failure\n";
        return;
    }

    nlh = (struct nlmsghdr*)buffer;
    while ((len = recv(nls, nlh, 4096, 0)) > 0) {
        for (; (NLMSG_OK(nlh, len)) && (nlh->nlmsg_type != NLMSG_DONE);
             nlh = NLMSG_NEXT(nlh, len)) {
            print_network_status();
        }
    }
}

void network_monitor::run()
{
    print_network_status();
    std::thread t{&network_monitor::run_network_loop, this};
    t.detach();
}

} // namespace wbd::monitor
