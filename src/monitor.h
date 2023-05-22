#ifndef HERBSTLUFTWM_UTILS_MONITOR_H
#define HERBSTLUFTWM_UTILS_MONITOR_H

namespace wbd::monitor {

class monitor {
public:
    virtual void run() = 0;
};
} // namespace wbd::monitor

#endif // HERBSTLUFTWM_UTILS_MONITOR_H
