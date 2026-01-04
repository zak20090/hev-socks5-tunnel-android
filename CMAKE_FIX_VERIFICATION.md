# CMake ExternalProject Build - Verification Guide

## Issue Fixed

The CMake build was failing because it attempted to manually compile hev-socks5-tunnel source files, which caused multiple issues:
- **YAML library missing version macros**: `YAML_VERSION_STRING`, `YAML_VERSION_MAJOR`, etc.
- **lwip headers not found**: `lwip/tcp.h`, `lwip/init.h`, `lwip/udp.h`
- **Missing internal headers**: `hev-compiler.h` and other configuration files

## Root Cause

The hev-socks5-tunnel project has its own sophisticated build system (Makefile) that:
- Generates required configuration headers
- Properly configures all submodules (yaml, lwip, hev-task-system)
- Sets up correct include paths automatically
- Builds everything in the correct order with proper dependencies

**We cannot bypass their build system by manually compiling files.**

## Solution Implemented

### Using CMake ExternalProject_Add

The updated `library/CMakeLists.txt` now uses `ExternalProject_Add` to:
1. Download hev-socks5-tunnel from GitHub
2. Initialize all submodules recursively
3. Build it using **their own Makefile** with `make static`
4. Import the resulting static library
5. Link our JNI wrapper with the built library

### Key Changes:

1. **ExternalProject Configuration**:
   ```cmake
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
   ```

2. **Proper Android Cross-Compilation**:
   - Uses Android NDK compiler (`CMAKE_C_COMPILER`)
   - Passes Android-specific flags via `CFLAGS`
   - Uses Android archiver and strip tools
   - Sets cross-compilation prefix for toolchain

3. **Import Built Library**:
   ```cmake
   add_library(hev-socks5-tunnel-core STATIC IMPORTED)
   set_target_properties(hev-socks5-tunnel-core PROPERTIES
       IMPORTED_LOCATION ${BINARY_DIR}/bin/libhev-socks5-tunnel.a
   )
   ```

4. **JNI Wrapper Links to Imported Library**:
   - No manual source compilation
   - Clean dependency chain
   - Proper include paths from source directory

## Why This Fix Works

By using the upstream Makefile:
- All configuration headers are generated automatically
- Submodules (yaml, lwip, hev-task-system) build with their own Makefiles
- Version macros and defines are set correctly
- Include paths are managed by the upstream build system
- Build order is correct (dependencies before main library)

## How to Verify the Fix

### 1. Clean Previous Build Artifacts

```bash
cd /path/to/hev-socks5-tunnel-android
./gradlew clean
rm -rf library/build
rm -rf library/.cxx
```

### 2. Build the Library

```bash
./gradlew :library:assembleDebug
```

### 3. Expected Output

The build should succeed with output similar to:
```
> Task :library:externalNativeBuildDebug
Building hev-socks5-tunnel with upstream Makefile...
> Task :library:buildCMakeDebug[arm64-v8a]
...
> Task :library:buildCMakeDebug[armeabi-v7a]
...
> Task :library:buildCMakeDebug[x86]
...
> Task :library:buildCMakeDebug[x86_64]
...
> Task :library:assembleDebug

BUILD SUCCESSFUL in XXs
```

### 4. Verify Native Libraries

Check that all ABI libraries are built:

```bash
find library/build -name "libhev-socks5-tunnel-jni.so"
```

Expected output (all ABIs):
```
library/build/intermediates/cmake/debug/obj/arm64-v8a/libhev-socks5-tunnel-jni.so
library/build/intermediates/cmake/debug/obj/armeabi-v7a/libhev-socks5-tunnel-jni.so
library/build/intermediates/cmake/debug/obj/x86/libhev-socks5-tunnel-jni.so
library/build/intermediates/cmake/debug/obj/x86_64/libhev-socks5-tunnel-jni.so
```

### 5. Verify AAR Output

```bash
ls -lh library/build/outputs/aar/
```

Should show:
```
library-debug.aar
```

### 6. Build Full Release

```bash
./gradlew :library:assembleRelease
```

### 7. Test with Example App

```bash
./gradlew :app:assembleDebug
```

This should also succeed if the library builds correctly.

## Success Criteria

✅ No CMake errors about missing headers or macros
✅ hev-socks5-tunnel builds successfully with its own Makefile
✅ All 4 ABIs build successfully
✅ libhev-socks5-tunnel-jni.so generated for all ABIs
✅ library-debug.aar and library-release.aar created
✅ Java classes compile without errors
✅ Example app builds successfully

## Common Issues and Solutions

### Issue: "Could not resolve dependencies"
**Solution**: Ensure you have internet connectivity for first build (downloads hev-socks5-tunnel from GitHub)

### Issue: "NDK not found"
**Solution**: Install NDK via Android Studio SDK Manager or set `ANDROID_NDK_HOME`

### Issue: "CMake version not found"
**Solution**: Install CMake 3.22.1+ via Android Studio SDK Manager

### Issue: Build fails with "make: command not found"
**Solution**: ExternalProject requires `make` to be available. On Windows, use WSL or install make via MSYS2/Cygwin.

### Issue: ExternalProject build fails
**Solution**: 
1. Clean build: `./gradlew clean`
2. Delete `.cxx` folder: `rm -rf library/.cxx`
3. Check internet connection
4. Rebuild from scratch

## Technical Details

### Build Process Flow

1. **CMake Configuration Phase**:
   - Sets up ExternalProject with git repository
   - Configures compiler and toolchain variables
   - Prepares build commands

2. **ExternalProject Download Phase**:
   - Clones hev-socks5-tunnel from GitHub
   - Initializes all submodules recursively
   - Downloads yaml, lwip, hev-task-system

3. **ExternalProject Build Phase**:
   - Runs `make static` in source directory
   - Uses Android NDK compiler
   - Builds all dependencies first (via their Makefiles)
   - Produces `bin/libhev-socks5-tunnel.a`

4. **JNI Wrapper Build Phase**:
   - Compiles JNI C++ wrapper
   - Links with imported static library
   - Produces `libhev-socks5-tunnel-jni.so` for each ABI

### Advantages of ExternalProject Approach

1. **No Manual Dependency Management**: Upstream handles all submodules
2. **Automatic Configuration**: All headers and macros generated correctly
3. **Version Control**: Easy to update by changing `GIT_TAG`
4. **Clean Separation**: JNI wrapper separate from core library
5. **Upstream Compatibility**: Works with any version of hev-socks5-tunnel

### Architecture Support

The build supports all standard Android ABIs:
- **armeabi-v7a**: 32-bit ARM (most phones)
- **arm64-v8a**: 64-bit ARM (modern phones)
- **x86**: 32-bit Intel (emulators)
- **x86_64**: 64-bit Intel (emulators, rare devices)

Each ABI builds independently with proper cross-compilation.

## Additional Notes

- The fix uses CMake's built-in ExternalProject module
- No changes to upstream hev-socks5-tunnel required
- Compatible with all Android ABIs
- Works with both debug and release builds
- Follows CMake best practices for external dependencies
- Build time may be longer on first build (downloads from GitHub)

## References

- CMake ExternalProject: https://cmake.org/cmake/help/latest/module/ExternalProject.html
- hev-socks5-tunnel: https://github.com/heiher/hev-socks5-tunnel
- Android NDK CMake guide: https://developer.android.com/ndk/guides/cmake
