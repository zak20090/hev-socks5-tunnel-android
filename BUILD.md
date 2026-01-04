# Building hev-socks5-tunnel-android

## Overview

This project integrates [hev-socks5-tunnel](https://github.com/heiher/hev-socks5-tunnel) into an Android library.

## Architecture

The build uses **prebuilt native libraries** for simplicity:
- `libhev-socks5-tunnel-core.so` - Core SOCKS5 tunnel implementation
- `libhev-socks5-tunnel-jni.so` - JNI wrapper (built from source)

**Important Note**: The current prebuilt libraries in this repository are minimal placeholders for demonstration purposes. For production use, you must build the actual libraries from hev-socks5-tunnel source following the instructions below.

## Supported ABIs

- ✅ arm64-v8a (64-bit ARM)
- ✅ armeabi-v7a (32-bit ARM)
- ❌ x86/x86_64 (not included)

## Quick Start

### Prerequisites

- Android Studio Arctic Fox or later
- Android NDK 25.1.8937393
- CMake 3.22.1+
- Gradle 7.0+

### Build Library

```bash
./gradlew :library:assembleDebug
```

Output: `library/build/outputs/aar/library-debug.aar`

### Build Release

```bash
./gradlew :library:assembleRelease
```

## Rebuilding Native Libraries

If you need to rebuild `libhev-socks5-tunnel-core.so`:

### 1. Clone hev-socks5-tunnel

```bash
git clone --recursive https://github.com/heiher/hev-socks5-tunnel.git
cd hev-socks5-tunnel
```

### 2. Patch lwip for Android

Edit `third-part/lwip/src/ports/unix/include/cc.h` around line 71:

```c
// Change this:
typedef __kernel_fd_set fd_set;

// To this:
#ifndef __ANDROID__
typedef __kernel_fd_set fd_set;
#endif
```

### 3. Set Up NDK Environment

```bash
export NDK=/path/to/ndk/25.1.8937393
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
```

### 4. Build for arm64-v8a

```bash
make clean
make ENABLE_ANDROID=1 \
     CC=$TOOLCHAIN/bin/aarch64-linux-android21-clang \
     CFLAGS="-fPIC -D__ANDROID__" \
     LDFLAGS="-shared"

cp bin/hev-socks5-tunnel \
   /path/to/hev-socks5-tunnel-android/library/src/main/jniLibs/arm64-v8a/libhev-socks5-tunnel-core.so
```

### 5. Build for armeabi-v7a

```bash
make clean
make ENABLE_ANDROID=1 \
     CC=$TOOLCHAIN/bin/armv7a-linux-androideabi21-clang \
     CFLAGS="-fPIC -D__ANDROID__" \
     LDFLAGS="-shared"

cp bin/hev-socks5-tunnel \
   /path/to/hev-socks5-tunnel-android/library/src/main/jniLibs/armeabi-v7a/libhev-socks5-tunnel-core.so
```

### 6. Rebuild Android Library

```bash
cd /path/to/hev-socks5-tunnel-android
rm -rf library/.cxx library/build
./gradlew :library:assembleDebug
```

## Adding x86/x86_64 Support

To add x86 support:

1. Build hev-socks5-tunnel for x86/x86_64 (see steps above with appropriate compiler)
2. Copy `.so` files to `library/src/main/jniLibs/x86/` and `library/src/main/jniLibs/x86_64/`
3. Update `library/build.gradle`:
   ```gradle
   ndk {
       abiFilters 'arm64-v8a', 'armeabi-v7a', 'x86', 'x86_64'
   }
   ```
4. Rebuild

## Project Structure

```
hev-socks5-tunnel-android/
├── library/
│   ├── src/main/
│   │   ├── java/           # Java/Kotlin code
│   │   ├── cpp/            # JNI wrapper (hev_socks5_tunnel_jni.c)
│   │   └── jniLibs/        # Prebuilt native libraries
│   │       ├── arm64-v8a/
│   │       │   └── libhev-socks5-tunnel-core.so
│   │       └── armeabi-v7a/
│   │           └── libhev-socks5-tunnel-core.so
│   ├── CMakeLists.txt      # CMake configuration
│   └── build.gradle        # Gradle configuration
└── BUILD.md                # This file
```

## Troubleshooting

### Build Fails with "missing .so"

Ensure prebuilt libraries exist:
```bash
ls -la library/src/main/jniLibs/*/libhev-socks5-tunnel-core.so
```

### CMake Configuration Errors

Clean build cache:
```bash
rm -rf library/.cxx library/build
```

### NDK Not Found

Set NDK path in `local.properties`:
```properties
ndk.dir=/path/to/ndk/25.1.8937393
```

### JNI Errors

Verify JNI wrapper is C (not C++):
```bash
file library/src/main/cpp/hev_socks5_tunnel_jni.c
```

## License

This project uses hev-socks5-tunnel which is licensed under MIT. See upstream repository for details.

## Credits

- [hev-socks5-tunnel](https://github.com/heiher/hev-socks5-tunnel) by @heiher
- Integration and Android adaptation
