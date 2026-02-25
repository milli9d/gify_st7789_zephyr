find_package(simpleble REQUIRED CONFIG)

file (GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.c"
)

add_executable(ble_ftp ${SRC_FILES})

target_include_directories(ble_ftp
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(ble_ftp simpleble::simpleble)