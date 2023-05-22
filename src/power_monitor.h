#ifndef HERBSTLUFTWM_UTILS_POWER_MONITOR_H
#define HERBSTLUFTWM_UTILS_POWER_MONITOR_H

#include "monitor.h"
#include "notification.h"
#include "status_printer.h"
#include <cstdint>
#include <memory>

namespace wbd::monitor {

enum BATTERY_THRESHOLD { LOW = 10, CRITICALLY_LOW = 5, SUSPEND = 3 };
enum URGENCY { /*LOW, */ NORMAL = 1, CRITICAL };

typedef struct {
    int ac_online;
    int battery_perc;
} power_supply_status_t;

class power_monitor : public monitor {
public:
    void run() override;

private:
    void read_power_supply(power_supply_status_t& pss);
    void print_power_supply();
    void send_notification(power_supply_status_t& pss);

    void run_print_loop();
    void run_udev_mon_loop();

    wbd::utils::status_printer printer_;
    int const interval_ = 10;

    std::unique_ptr<wbd::utils::notification> id_low_;
    std::unique_ptr<wbd::utils::notification> id_critical_low_;
};

} // namespace wbd::monitor

#endif // HERBSTLUFTWM_UTILS_POWER_MONITOR_H
