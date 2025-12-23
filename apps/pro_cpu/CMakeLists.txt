# cmake boilerplate
cmake_minimum_required(VERSION 3.5.0)

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)

# add modules
list(APPEND EXTRA_ZEPHYR_MODULES
    ${PROJECT_ROOT_DIR}/modules/AnimatedGIF
)

# import overlays for CPUs
set(DTC_OVERLAY_FILE ${PROJECT_ROOT_DIR}/overlays/waveshare_1_47_tft_esp32c6.overlay)

# add extra configuration
set(EXTRA_CONF_FILE ${CMAKE_CURRENT_LIST_DIR}/prj.conf)

# add zephyr package
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# declare project
project(GifyESP32Zephyr)

# add sources
file(GLOB_RECURSE srcs ${CMAKE_CURRENT_LIST_DIR}/src/*.c*)

target_sources(app
    PRIVATE ${srcs}
)

target_include_directories(app
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/
)