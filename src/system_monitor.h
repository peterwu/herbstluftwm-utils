#ifndef HERBSTLUFTWM_UTILS_SYSTEM_MONITOR_H
#define HERBSTLUFTWM_UTILS_SYSTEM_MONITOR_H

/* create a system monitor that can do:
 * powermon
 * brightness
 * volume
 * ibus
 *
 * the architecture is that sysmon exposes a list of methods on
 * dbus that allows for invocation from the above components.
 *
 * sysmon's job is just to print_power_supply out the status from these
 * components in the format of {indicator}\t{data1}\t{data2}\t{data...n}
 *
 * sysmon may use pthread to host those components in order to
 * serve their respective clients.
 *
 * brightnessctl -> sysmon
 * volumectl -> sysmon
 * powerctl -> sysmon (when detecting AC plugged-in)
 */

#include "monitor.h"
#include <memory>
#include <systemd/sd-bus.h>
#include <vector>

namespace wbd::monitor {

class system_monitor {
public:
    void load(std::unique_ptr<monitor> m);
    void run();

private:
    void run_sd_bus_loop();

    std::vector<std::unique_ptr<monitor>> monitors_;
};

} // namespace wbd::monitor

#endif
