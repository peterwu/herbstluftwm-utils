#include "systemctl.h"
#include <systemd/sd-bus.h>

namespace wbd::utils {

void systemctl::suspend()
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus* bus = NULL;
    unsigned int r;

    r = sd_bus_open_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_call_method(
        bus, "org.freedesktop.login1", "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager", "Suspend", &error, NULL, "b", true);
    if (r < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", error.message);
        goto finish;
    }

finish:
    sd_bus_error_free(&error);
    sd_bus_unref(bus);
}
} // namespace wbd::utils
