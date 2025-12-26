CMAKE_MINIMUM_REQUIRED( VERSION 3.5.0 )

project(ble_ftp)

# Set output directories for binaries and libraries to top-level bin and lib
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)


set(CMAKE_C_STANDARD 26)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -std=c++26")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# enable address sanitizer
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()


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