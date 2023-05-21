#include "power_monitor.h"
#include "notification.h"
#include "systemctl.h"

#include <csignal>
#include <libudev.h>
#include <thread>

namespace wbd::monitor {

void power_monitor::send_notification(power_supply_status_t& pss)
{
    std::string app_name{"herbstluftwm-powermon"};
    std::string summary, icon;

    if (pss.ac_online == 1) {
        /* close all PWR notifications if any */
        if (id_critical_low_)
            id_critical_low_->close();
        if (id_low_)
            id_low_->close();
    }
    else if (pss.battery_perc <= BATTERY_THRESHOLD::SUSPEND) {
        /* systemctl suspend */
        utils::systemctl::suspend();
    }
    else {
        summary = "Battery: " + std::to_string(pss.battery_perc) + "%";

        auto n = std::make_unique<utils::notification>();
        n->app_name = app_name;
        n->summary = summary;
        n->value = pss.battery_perc;
        n->icon = icon;

        if (pss.battery_perc <= BATTERY_THRESHOLD::CRITICALLY_LOW) {
            n->icon = "battery-empty-symbolic";
            n->urgency = URGENCY::CRITICAL;
            n->timeout = 0;

            n->send();
            id_critical_low_ = std::move(n);
        }
        else if (pss.battery_perc <= BATTERY_THRESHOLD::LOW) {
            n->icon = "battery-low-symbolic";
            n->urgency = URGENCY::NORMAL;
            n->timeout = 3000;

            n->send();
            id_low_ = std::move(n);
        }
    }
}

void power_monitor::read_power_supply(power_supply_status_t& pss)
{
    int ac_online = 0;
    int battery_now = 0;
    int battery_max = 0;

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

    udev_enumerate_add_match_subsystem(enumerate, "power_supply");
    udev_enumerate_scan_devices(enumerate);

    /* fillup device list */
    devices = udev_enumerate_get_list_entry(enumerate);
    if (!devices) {
        std::cerr << "Failed to get device list.\n";
        return;
    }

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        std::string path, type;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path.c_str());

        type = udev_device_get_sysattr_value(dev, "type");

        if (type == "Mains") {
            ac_online = atoi(udev_device_get_sysattr_value(dev, "online"));
        }
        else if (type == "Battery") {
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

    pss.ac_online = ac_online;
    pss.battery_perc = battery_now * 100 / battery_max;
}

void power_monitor::print_power_supply()
{
    power_supply_status_t pss;

    read_power_supply(pss);
    send_notification(pss);

    printer_.print("pwr", pss.ac_online, pss.battery_perc);
}

void power_monitor::run_print_loop()
{
    for (;;) {
        print_power_supply();
        sleep(interval_);
    }
}

void power_monitor::run_udev_mon_loop()
{
    struct udev* udev;
    struct udev_device* dev;
    struct udev_monitor* mon;
    int fd;

    /* create udev object */
    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Can't create udev\n");
        return;
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

void power_monitor::run()
{
    std::thread t1(&power_monitor::run_print_loop, this);
    t1.detach();

    std::thread t2(&power_monitor::run_udev_mon_loop, this);
    t2.detach();
}
} // namespace wbd::monitor