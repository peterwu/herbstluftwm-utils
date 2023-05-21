#ifndef HERBSTLUFTWM_UTILS_NETWORK_MONITOR_H
#define HERBSTLUFTWM_UTILS_NETWORK_MONITOR_H

#include "status_printer.h"
#include "monitor.h"

namespace wbd::monitor {

enum class NETWORK_TYPE { UNKNOWN, ETHERNET, WIRELESS };

typedef struct {
    bool connected;
    NETWORK_TYPE type;
} network_status_t;

class network_monitor : public monitor {
public:
    void run() override;

private:
    void print_network_status();
    void run_network_loop();

    utils::status_printer printer_;
};

} // namespace wbd::monitor

#endif
