cmake_minimum_required(VERSION 3.25)
project(herbstluftwm_utils C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

find_package(PkgConfig REQUIRED)
pkg_check_modules(SD REQUIRED libsystemd)
pkg_check_modules(UDEV REQUIRED libudev)
pkg_check_modules(IBUS REQUIRED ibus-1.0)

add_executable(main main.c)
target_include_directories(main PRIVATE
        ${SD_INCLUDE_DIRS}
        ${UDEV_INCLUDE_DIRS})
target_link_libraries(main PRIVATE
        ${SD_LIBRARIES}
        ${UDEV_LIBRARIES})
target_compile_options(main PRIVATE
        ${SD_CFLAGS_OTHER}
        ${UDEV_CFLAGS_OTHER})

# brightnessctl
add_executable(brightnessctl brightnessctl.c util.c)
target_include_directories(brightnessctl PRIVATE ${SD_INCLUDE_DIRS})
target_link_libraries(brightnessctl PRIVATE ${SD_LIBRARIES})
target_compile_options(brightnessctl PRIVATE ${SD_CFLAGS_OTHER})

# volumectl
add_executable(volumectl volumectl.c util.c)
target_include_directories(volumectl PRIVATE
        ${SD_INCLUDE_DIRS})
target_link_libraries(volumectl PRIVATE
        ${SD_LIBRARIES})
target_compile_options(volumectl PRIVATE
        ${SD_CFLAGS_OTHER})

# sysmon
add_executable(sysmon sysmon.c)
#target_include_directories(sysmon PRIVATE
#        ${IBUS_INCLUDE_DIRS}
#        ${SD_INCLUDE_DIRS}
#        ${WP_INCLUDE_DIRS})
#target_link_libraries(sysmon PRIVATE
#        ${IBUS_LIBRARIES}
#        ${SD_LIBRARIES}
#        ${WP_LIBRARIES})
#target_compile_options(sysmon PRIVATE
#        ${IBUS_CFLAGS_OTHER}
#        ${SD_CFLAGS_OTHER}
#        ${WP_CFLAGS_OTHER})

# will be incorporated into sysmom later
add_executable(powermon powermon.c util.c)
target_include_directories(powermon PRIVATE
        ${SD_INCLUDE_DIRS}
        ${UDEV_INCLUDE_DIRS})
target_link_libraries(powermon PRIVATE
        ${SD_LIBRARIES}
        ${UDEV_LIBRARIES})
target_compile_options(powermon PRIVATE
        ${SD_CFLAGS_OTHER}
        ${UDEV_CFLAGS_OTHER})

# will be incorporated into sysmom later
add_executable(netmon netmon.c)
target_include_directories(netmon PRIVATE ${UDEV_INCLUDE_DIRS})
target_link_libraries(netmon PRIVATE ${UDEV_LIBRARIES})
target_compile_options(netmon PRIVATE ${UDEV_CFLAGS_OTHER})

add_executable(ibusmon ibusmon.c)
target_include_directories(ibusmon PRIVATE ${IBUS_INCLUDE_DIRS})
target_link_libraries(ibusmon PRIVATE ${IBUS_LIBRARIES})
target_compile_options(ibusmon PRIVATE ${IBUS_CFLAGS_OTHER})

