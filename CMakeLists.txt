cmake_minimum_required(VERSION 3.13)
project(herbstluftwm_utils CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SD REQUIRED libsystemd)
pkg_check_modules(UDEV REQUIRED libudev)
pkg_check_modules(IBUS REQUIRED ibus-1.0)

add_executable(sysmon src/main.cpp)
target_sources(sysmon PRIVATE
        src/status_printer.h

        src/notification.cpp
        src/systemctl.cpp

        src/ibus_monitor.cpp
        src/network_monitor.cpp
        src/power_monitor.cpp
        src/system_monitor.cpp)
target_include_directories(sysmon PRIVATE
        ${IBUS_INCLUDE_DIRS}
        ${SD_INCLUDE_DIRS}
        ${UDEV_INCLUDE_DIRS})
target_link_libraries(sysmon PRIVATE
        ${IBUS_LIBRARIES}
        ${SD_LIBRARIES}
        ${UDEV_LIBRARIES})
target_compile_options(sysmon PRIVATE
        ${IBUS_CFLAGS_OTHER}
        ${SD_CFLAGS_OTHER}
        ${UDEV_CFLAGS_OTHER})

