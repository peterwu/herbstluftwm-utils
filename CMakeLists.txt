cmake_minimum_required(VERSION 3.20)
project(hlwm-utils C)

set(CMAKE_C_STANDARD 11)

add_executable(hlwm-show-time show-time.c)
add_executable(hlwm-show-tray show-tray.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

add_definitions(${GTK3_CFLAGS_OTHER})

target_link_libraries(hlwm-show-time ${GTK3_LIBRARIES})
target_link_libraries(hlwm-show-tray ${GTK3_LIBRARIES})
