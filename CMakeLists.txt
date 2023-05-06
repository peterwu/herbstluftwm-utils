cmake_minimum_required(VERSION 3.25)
project(herbstluftwm_utils C)

set(CMAKE_C_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SD REQUIRED libsystemd)

add_executable(main main.c)
target_include_directories(main PUBLIC ${SD_INCLUDE_DIRS})
target_link_libraries(main PUBLIC ${SD_LIBRARIES})
target_compile_options(main PUBLIC ${SD_CFLAGS_OTHER})

add_executable(brightnessctl brightnessctl.c util.c)
target_include_directories(brightnessctl PUBLIC ${SD_INCLUDE_DIRS})
target_link_libraries(brightnessctl PUBLIC ${SD_LIBRARIES})
target_compile_options(brightnessctl PUBLIC ${SD_CFLAGS_OTHER})
