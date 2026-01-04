cmake_minimum_required(VERSION 3.22.1)

project("hev-socks5-tunnel-android")

include(ExternalProject)

# Prepare compiler flags for Android
string(REPLACE ";" " " CMAKE_C_FLAGS_STR "${CMAKE_C_FLAGS}")
string(REPLACE ";" " " CMAKE_CXX_FLAGS_STR "${CMAKE_CXX_FLAGS}")

# Build hev-socks5-tunnel using their Makefile
ExternalProject_Add(
    hev-socks5-tunnel-external
    GIT_REPOSITORY https://github.com/heiher/hev-socks5-tunnel.git
    GIT_TAG main
    GIT_SUBMODULES_RECURSE ON
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make static
        CC=${CMAKE_C_COMPILER}
        AR=${CMAKE_AR}
        STRIP=${CMAKE_STRIP}
        "CFLAGS=${CMAKE_C_FLAGS_STR} -fPIC"
        CROSS_PREFIX=${ANDROID_TOOLCHAIN_PREFIX}
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(hev-socks5-tunnel-external SOURCE_DIR)
ExternalProject_Get_Property(hev-socks5-tunnel-external BINARY_DIR)

# Import the built static library
add_library(hev-socks5-tunnel-core STATIC IMPORTED)
set_target_properties(hev-socks5-tunnel-core PROPERTIES
    IMPORTED_LOCATION ${BINARY_DIR}/bin/libhev-socks5-tunnel.a
)
add_dependencies(hev-socks5-tunnel-core hev-socks5-tunnel-external)

# JNI wrapper library
add_library(hev-socks5-tunnel-jni SHARED
    src/main/cpp/hev_socks5_tunnel_jni.cpp
)

target_include_directories(hev-socks5-tunnel-jni PRIVATE
    ${SOURCE_DIR}/src
    ${SOURCE_DIR}/src/core/include
    ${SOURCE_DIR}/third-part/hev-task-system/include
)

target_link_libraries(hev-socks5-tunnel-jni
    hev-socks5-tunnel-core
    log
    android
)
