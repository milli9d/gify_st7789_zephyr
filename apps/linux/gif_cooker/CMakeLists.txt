set(ZEPHYR_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../modules)

file(GLOB AnimatedGIFSrcs
    ${ZEPHYR_MODULES_DIR}/AnimatedGIF/AnimatedGIF/src/*.c*
    ${ZEPHYR_MODULES_DIR}/AnimatedGIF/AnimatedGIF/src/*.inl*
)

find_package(simpleble REQUIRED CONFIG)

file (GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.c"
)

add_executable(gif_cooker ${SRC_FILES})

target_include_directories(gif_cooker
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE ${ZEPHYR_MODULES_DIR}/AnimatedGIF/AnimatedGIF/src
)

target_link_libraries(gif_cooker simpleble::simpleble)