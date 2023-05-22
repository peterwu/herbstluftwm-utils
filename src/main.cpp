#include "ibus_monitor.h"
#include "network_monitor.h"
#include "power_monitor.h"
#include "system_monitor.h"
#include <memory>

int main(int argc, char** argv)
{
    wbd::monitor::system_monitor sm;

    std::unique_ptr<wbd::monitor::monitor> monitors[]{
        std::make_unique<wbd::monitor::ibus_monitor>(),
        std::make_unique<wbd::monitor::network_monitor>(),
        std::make_unique<wbd::monitor::power_monitor>()};

    for (auto& m : monitors) {
        sm.load(std::move(m));
    }

    sm.run();

    for (;;)
        sleep(30);

    return 0;
}