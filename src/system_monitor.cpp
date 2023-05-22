#include "status_printer.h"
#include "system_monitor.h"
#include <iostream>

namespace wbd::monitor {

static int method_emit(sd_bus_message* m, void* userdata,
                       sd_bus_error* ret_error)
{
    int r;
    char const* s;
    int i1, i2;

    r = sd_bus_message_read(m, "sii", &s, &i1, &i2);
    if (r < 0) {
        fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-r));
        return r;
    }

    utils::status_printer printer;
    printer.print(s, i1, i2);

    return sd_bus_reply_method_return(m, NULL);
}

void system_monitor::load(std::unique_ptr<monitor> m)
{
    monitors_.push_back(std::move(m));
}

void system_monitor::run()
{
    for (auto& m : monitors_) {
        m->run();
    }

    run_sd_bus_loop();
}

void system_monitor::run_sd_bus_loop()
{
    sd_bus_slot* slot = NULL;
    sd_bus* bus = NULL;
    int r;

    sd_bus_vtable const sysmon_vtable[] = {
        SD_BUS_VTABLE_START(0),
        SD_BUS_METHOD("Emit", "sii", NULL, method_emit,
                      SD_BUS_VTABLE_UNPRIVILEGED),
        SD_BUS_VTABLE_END};

    r = sd_bus_default_user(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to user bus: %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_add_object_vtable(bus, &slot,
                                 "/wbd/utils/sysmon", /* object path */
                                 "wbd.utils.sysmon",  /* interface name */
                                 sysmon_vtable, NULL);
    if (r < 0) {
        fprintf(stderr, "Failed to issue method call: %s\n", strerror(-r));
        goto finish;
    }

    /* Take a well-known service name so that clients can find us */
    r = sd_bus_request_name(bus, "wbd.utils.sysmon", 0);
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
        if (r > 0) /* we processed a request, try to process another one,
                      right-away */
            continue;

        /* Wait for the next request to process */
        r = sd_bus_wait(bus, (uint64_t)-1);
        if (r < 0) {
            fprintf(stderr, "Failed to wait on bus: %s\n", strerror(-r));
            goto finish;
        }
    }

finish:
    sd_bus_slot_unref(slot);
    sd_bus_unref(bus);
}

} // namespace wbd::monitor