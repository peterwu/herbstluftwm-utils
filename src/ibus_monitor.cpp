#include "ibus_monitor.h"
#include <thread>

namespace wbd::monitor {

static wbd::utils::status_printer printer;
static void print_ibus_status(IBusBus* bus)
{
    IBusEngineDesc* global_engine = ibus_bus_get_global_engine(bus);
    std::string name{ibus_engine_desc_get_name(global_engine)};

    if (name == "libpinyin")
        printer.print("ime", "zh");
    else if (name == "xkb:us::eng")
        printer.print("ime", "en");

    g_object_unref(global_engine);
}

static void global_engine_changed_cb(IBusBus* bus)
{
    print_ibus_status(bus);
}

ibus_monitor::ibus_monitor()
{
    ibus_init();
    bus_ = ibus_bus_new();
}

ibus_monitor::~ibus_monitor()
{
    g_object_unref(bus_);
}

void ibus_monitor::run()
{
    print_ibus_status(bus_);
    g_signal_connect(bus_, "global-engine-changed",
                     G_CALLBACK(global_engine_changed_cb), nullptr);
    ibus_bus_set_watch_ibus_signal(bus_, TRUE);
    std::thread t(ibus_main);
    t.detach();
}

} // namespace wbd::monitor