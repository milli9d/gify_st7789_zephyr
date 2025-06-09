# cmake boilerplate
cmake_minimum_required(VERSION 3.30.2)

# add modules
list(APPEND EXTRA_ZEPHYR_MODULES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/AnimatedGIF
)

# import custom board package
# list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../)

# import overlays for CPUs
set(DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/waveshare_1_69_touch_tft_esp32s3.overlay)

# add drivers

# add modules
set(EXTRA_CONF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/prj.conf)

# add zephyr package
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# declare project
project(PixyToyZephyr)

# add sources
file(GLOB_RECURSE srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*)

target_sources(app
    PRIVATE ${srcs}
)

target_include_directories(app
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/
)