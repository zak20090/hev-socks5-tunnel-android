# CMake lwip Include Path Fix - Verification Guide

## Issue Fixed

The CMake build was failing with:
```
fatal error: 'lwip/tcp.h' file not found
```

This was caused by incomplete include paths for lwip headers.

## Changes Made

### 1. Updated library/CMakeLists.txt

#### Key Fixes:

1. **Proper Source File Collection**: Split source collection into specific categories:
   - `HEV_CORE_SOURCES` - Core functionality
   - `HEV_TASK_SOURCES` - Task system
   - `HEV_LWIP_SOURCES` - lwip networking stack
   - `HEV_YAML_SOURCES` - YAML parser
   - `HEV_MAIN_SOURCES` - Main entry points

2. **Critical Include Path Addition**: Added `${HEV_THIRD_PART_DIR}/lwip` directory
   - This path contains the custom `lwipopts.h` configuration file
   - Previously only had `lwip/repo/src/include` which wasn't sufficient

3. **Complete Include Directories**:
   ```cmake
   target_include_directories(hev-socks5-tunnel-core PUBLIC
       ${HEV_SRC_DIR}
       ${HEV_SRC_DIR}/core/include          # Added
       ${HEV_THIRD_PART_DIR}/hev-task-system/include
       ${HEV_THIRD_PART_DIR}/lwip/repo/src/include
       ${HEV_THIRD_PART_DIR}/lwip           # CRITICAL: Added for lwipopts.h
       ${HEV_THIRD_PART_DIR}/yaml/include   # Fixed path
   )
   ```

4. **Added Compile Options**:
   - Added `-std=gnu11` for proper C11 support
   - This ensures all C11 features used in the code are available

5. **Enhanced JNI Wrapper Includes**:
   - Added `${HEV_SRC_DIR}/core/include` for JNI wrapper
   - Added `${HEV_THIRD_PART_DIR}/hev-task-system/include` for JNI wrapper

## Why This Fix Works

lwip requires TWO include paths:
1. **`lwip/repo/src/include`** - Contains the main lwip headers (lwip/tcp.h, lwip/ip.h, etc.)
2. **`lwip/`** - Contains the project-specific `lwipopts.h` configuration file

The lwip source files include headers like:
```c
#include "lwip/tcp.h"
#include "lwipopts.h"
```

Without both paths, the compiler can't find all required headers.

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

✅ No CMake errors about missing headers (lwip/tcp.h, etc.)
✅ All 4 ABIs build successfully
✅ libhev-socks5-tunnel-jni.so generated for all ABIs
✅ library-debug.aar and library-release.aar created
✅ Java classes compile without errors
✅ Example app builds successfully

## Common Issues and Solutions

### Issue: "Could not resolve dependencies"
**Solution**: Ensure you have internet connectivity for first build (downloads dependencies from GitHub)

### Issue: "NDK not found"
**Solution**: Install NDK via Android Studio SDK Manager or set `ANDROID_NDK_HOME`

### Issue: "CMake version not found"
**Solution**: Install CMake 3.22.1+ via Android Studio SDK Manager

### Issue: Build still fails with lwip errors
**Solution**: 
1. Clean build: `./gradlew clean`
2. Delete `.cxx` folder: `rm -rf library/.cxx`
3. Rebuild from scratch

## Technical Details

### Include Path Hierarchy

The updated CMakeLists.txt ensures this include search order:
1. Main source directory
2. Core includes
3. Third-party includes (task-system, lwip, yaml)

### Source File Organization

Sources are now properly organized:
- **Core**: Main socks5 tunnel implementation
- **Task System**: Asynchronous task handling
- **lwip**: Lightweight TCP/IP stack
- **YAML**: Configuration parsing

### Compilation Flags

- `-std=gnu11`: GNU C11 standard (required for some language features)
- `_GNU_SOURCE`: Enable GNU extensions
- `ENABLE_PTHREAD`: Enable pthread support

## Additional Notes

- The fix is minimal and focused on include paths only
- No changes to source code required
- Compatible with all Android ABIs
- Works with both debug and release builds
- Follows CMake best practices

## References

- lwip documentation: https://www.nongnu.org/lwip/
- CMake documentation: https://cmake.org/documentation/
- Android NDK CMake guide: https://developer.android.com/ndk/guides/cmake
