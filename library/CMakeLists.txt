cmake_minimum_required(VERSION 3.22.1)
project("hev-socks5-tunnel-android")

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)

# Fetch hev-socks5-tunnel from GitHub
FetchContent_Declare(
    hev-socks5-tunnel
    GIT_REPOSITORY https://github.com/heiher/hev-socks5-tunnel.git
    GIT_TAG main
    GIT_SUBMODULES_RECURSE ON
)

FetchContent_MakeAvailable(hev-socks5-tunnel)

# Set source directory for hev-socks5-tunnel
set(HEV_SRC_DIR ${hev-socks5-tunnel_SOURCE_DIR}/src)

# Collect all source files from hev-socks5-tunnel
file(GLOB_RECURSE HEV_SOURCES
    "${HEV_SRC_DIR}/*.c"
)

# Remove main.c as we'll provide our own entry point
list(FILTER HEV_SOURCES EXCLUDE REGEX ".*main\\.c$")

# Create static library for hev-socks5-tunnel
add_library(hev-socks5-tunnel-core STATIC ${HEV_SOURCES})

target_include_directories(hev-socks5-tunnel-core PUBLIC
    ${HEV_SRC_DIR}
    ${hev-socks5-tunnel_SOURCE_DIR}/third-part/hev-task-system/include
    ${hev-socks5-tunnel_SOURCE_DIR}/third-part/lwip/repo/src/include
    ${hev-socks5-tunnel_SOURCE_DIR}/third-part/yaml
)

target_compile_definitions(hev-socks5-tunnel-core PRIVATE
    _GNU_SOURCE
    ENABLE_PTHREAD
)

# Create the JNI shared library
add_library(hev-socks5-tunnel-jni SHARED
    src/main/cpp/hev_socks5_tunnel_jni.cpp
)

target_link_libraries(hev-socks5-tunnel-jni
    hev-socks5-tunnel-core
    log
    android
)

target_include_directories(hev-socks5-tunnel-jni PRIVATE
    ${HEV_SRC_DIR}
)
