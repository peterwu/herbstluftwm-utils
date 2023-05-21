#include "system_monitor.h"
#include <iostream>

namespace wbd::monitor {

void system_monitor::load(std::unique_ptr<monitor> m)
{
    monitors_.push_back(std::move(m));
}

void system_monitor::run()
{
    for (auto& m : monitors_) {
        m->run();
    }
}

} // namespace wbd::monitor