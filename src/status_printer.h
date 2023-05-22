#ifndef HERBSTLUFTWM_UTILS_STATUS_PRINTER_H
#define HERBSTLUFTWM_UTILS_STATUS_PRINTER_H

#include <iostream>
#include <sstream>
#include <string>

namespace wbd::utils {

class status_printer {
public:
    template <typename T, typename... Ts>
    void print(T first, Ts... rest)
    {
        ss_ << first;
        ss_ << '\t';
        print(rest...);
    }

    void print(){
        if (ss_.str().compare(old_)){
            std::cout << ss_.str() << std::endl;
            old_ = ss_.str();
        }
        ss_.str(std::string());
    };

private:
    std::stringstream ss_;
    std::string old_;
};

} // namespace wbd::utils

#endif // HERBSTLUFTWM_UTILS_STATUS_PRINTER_H
