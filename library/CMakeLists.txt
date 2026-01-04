cmake_minimum_required(VERSION 3.22.1)

project("hev-socks5-tunnel-android")

# Import prebuilt hev-socks5-tunnel-core library
add_library(hev-socks5-tunnel-core SHARED IMPORTED)
set_target_properties(hev-socks5-tunnel-core PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libhev-socks5-tunnel-core.so
)

# JNI wrapper library
add_library(hev-socks5-tunnel-jni SHARED
    src/main/cpp/hev_socks5_tunnel_jni.c
)

target_link_libraries(hev-socks5-tunnel-jni
    hev-socks5-tunnel-core
    log
    android
)
