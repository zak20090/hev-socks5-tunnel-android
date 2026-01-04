cmake_minimum_required(VERSION 3.22.1)

project("hev-socks5-tunnel-android")

# Fetch hev-socks5-tunnel from GitHub
include(FetchContent)

FetchContent_Declare(
    hev-socks5-tunnel
    GIT_REPOSITORY https://github.com/heiher/hev-socks5-tunnel.git
    GIT_TAG main
    GIT_SUBMODULES_RECURSE ON
)

FetchContent_MakeAvailable(hev-socks5-tunnel)

# Set source directories
set(HEV_SRC_DIR ${hev-socks5-tunnel_SOURCE_DIR}/src)
set(HEV_THIRD_PART_DIR ${hev-socks5-tunnel_SOURCE_DIR}/third-part)

# Collect all source files
file(GLOB_RECURSE HEV_CORE_SOURCES
    "${HEV_SRC_DIR}/core/src/*.c"
)

file(GLOB_RECURSE HEV_TASK_SOURCES
    "${HEV_THIRD_PART_DIR}/hev-task-system/src/*.c"
)

file(GLOB_RECURSE HEV_LWIP_SOURCES
    "${HEV_THIRD_PART_DIR}/lwip/repo/src/core/*.c"
    "${HEV_THIRD_PART_DIR}/lwip/repo/src/core/*/*.c"
    "${HEV_THIRD_PART_DIR}/lwip/repo/src/netif/*.c"
    "${HEV_THIRD_PART_DIR}/lwip/repo/src/api/*.c"
)

file(GLOB HEV_YAML_SOURCES
    "${HEV_THIRD_PART_DIR}/yaml/src/*.c"
)

file(GLOB HEV_MAIN_SOURCES
    "${HEV_SRC_DIR}/*.c"
)

# Exclude main.c as we're building a library
list(FILTER HEV_MAIN_SOURCES EXCLUDE REGEX ".*/main\\.c$")

# Combine all sources
set(HEV_ALL_SOURCES
    ${HEV_CORE_SOURCES}
    ${HEV_TASK_SOURCES}
    ${HEV_LWIP_SOURCES}
    ${HEV_YAML_SOURCES}
    ${HEV_MAIN_SOURCES}
)

# Build hev-socks5-tunnel as static library
add_library(hev-socks5-tunnel-core STATIC ${HEV_ALL_SOURCES})

# Set include directories - COMPLETE with correct order
target_include_directories(hev-socks5-tunnel-core PUBLIC
    # Main source directories
    ${HEV_SRC_DIR}
    ${HEV_SRC_DIR}/core/include
    
    # hev-task-system (both public and internal headers)
    ${HEV_THIRD_PART_DIR}/hev-task-system/include
    ${HEV_THIRD_PART_DIR}/hev-task-system/src
    
    # lwip - CRITICAL ORDER: repo/src/include BEFORE base lwip dir
    ${HEV_THIRD_PART_DIR}/lwip/repo/src/include
    ${HEV_THIRD_PART_DIR}/lwip
    
    # yaml (both public and internal headers)
    ${HEV_THIRD_PART_DIR}/yaml/include
    ${HEV_THIRD_PART_DIR}/yaml/src
)

# Add ALL required compile definitions for hev-task-system and dependencies
target_compile_definitions(hev-socks5-tunnel-core PRIVATE
    # Base system definitions
    _GNU_SOURCE
    ENABLE_PTHREAD
    
    # Scheduler configuration
    CONFIG_SCHED_CLOCK=CLOCK_MONOTONIC
    
    # Memory allocator configuration (hev-task-system)
    CONFIG_MEMALLOC_SLICE_ALIGN=8              # Memory alignment in bytes
    CONFIG_MEMALLOC_SLICE_MAX_SIZE=32768       # Max slice size: 32KB
    CONFIG_MEMALLOC_SLICE_MAX_COUNT=128        # Max cached slices
)

# Compile options
target_compile_options(hev-socks5-tunnel-core PRIVATE
    -std=gnu11
    -Wno-macro-redefined    # Suppress _GNU_SOURCE redefinition warning
)

# JNI wrapper library
add_library(hev-socks5-tunnel-jni SHARED
    src/main/cpp/hev_socks5_tunnel_jni.cpp
)

target_include_directories(hev-socks5-tunnel-jni PRIVATE
    ${HEV_SRC_DIR}
    ${HEV_SRC_DIR}/core/include
    ${HEV_THIRD_PART_DIR}/hev-task-system/include
)

target_link_libraries(hev-socks5-tunnel-jni
    hev-socks5-tunnel-core
    log
    android
)
