#ifndef HERBSTLUFTWM_UTILS_IBUS_MONITOR_H
#define HERBSTLUFTWM_UTILS_IBUS_MONITOR_H

#include "monitor.h"
#include "status_printer.h"
#include <ibus.h>

namespace wbd::monitor {

class ibus_monitor : public monitor {
public:
    ibus_monitor();
    ~ibus_monitor();

    void run() override;

private:
    IBusBus* bus_;
};

} // namespace wbd::monitor

#endif // HERBSTLUFTWM_UTILS_IBUS_MONITOR_H
