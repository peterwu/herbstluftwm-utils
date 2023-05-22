#ifndef HERBSTLUFTWM_UTILS_NOTIFICATION_H
#define HERBSTLUFTWM_UTILS_NOTIFICATION_H

#include <cstdint>
#include <string>

namespace wbd::utils {

struct notification {
    std::string app_name;
    std::string icon;
    std::string summary;
    std::string body;
    int urgency;
    int value;
    int timeout;

    void send();
    void close();

private:
    std::uint32_t id_;
};

} // namespace wbd::utils
#endif // HERBSTLUFTWM_UTILS_NOTIFICATION_H
