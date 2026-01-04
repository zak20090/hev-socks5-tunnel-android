# Build Instructions

This document provides detailed instructions for building the HevSocks5Tunnel Android library.

## Prerequisites

1. **Android Studio** (Arctic Fox or newer)
   - Download from: https://developer.android.com/studio

2. **Android SDK**
   - API Level 21 (Android 5.0) minimum
   - API Level 34 (Android 14) target
   - Install via Android Studio SDK Manager

3. **Android NDK**
   - Version 25.1.8937393 or newer
   - Install via Android Studio SDK Manager

4. **CMake**
   - Version 3.22.1 or newer
   - Install via Android Studio SDK Manager

5. **Java Development Kit (JDK)**
   - JDK 11 or newer
   - Included with Android Studio

## Build from Command Line

### 1. Clone the Repository

```bash
git clone https://github.com/zak20090/hev-socks5-tunnel-android.git
cd hev-socks5-tunnel-android
```

### 2. Build the Library

Build the release AAR:

```bash
./gradlew :library:assembleRelease
```

Output: `library/build/outputs/aar/library-release.aar`

Build the debug AAR:

```bash
./gradlew :library:assembleDebug
```

Output: `library/build/outputs/aar/library-debug.aar`

### 3. Build the Example App

Build and install debug APK:

```bash
./gradlew :app:installDebug
```

Build release APK:

```bash
./gradlew :app:assembleRelease
```

### 4. Build Everything

```bash
./gradlew build
```

### 5. Clean Build

```bash
./gradlew clean
```

## Build from Android Studio

1. Open Android Studio
2. Select "Open an Existing Project"
3. Navigate to the cloned repository
4. Wait for Gradle sync to complete
5. Build menu → Make Project (Ctrl+F9 / Cmd+F9)

### Run Example App

1. Connect an Android device or start an emulator
2. Select "app" configuration from the toolbar
3. Click Run (Shift+F10 / Ctrl+R)

## Build Output Locations

- **Library AAR**: `library/build/outputs/aar/`
- **Example APK**: `app/build/outputs/apk/`
- **Native Libraries**: `library/build/intermediates/cmake/`

## Native Build Process

The native build automatically:
1. Downloads hev-socks5-tunnel from GitHub
2. Downloads all submodules (hev-task-system, lwip, yaml)
3. Compiles C sources for all ABIs
4. Creates JNI wrapper library
5. Packages everything into AAR

### Supported ABIs

- armeabi-v7a (ARM 32-bit)
- arm64-v8a (ARM 64-bit)
- x86 (Intel 32-bit)
- x86_64 (Intel 64-bit)

## Troubleshooting

### NDK Not Found

```
Error: NDK is not installed
```

**Solution**: Install NDK via Android Studio SDK Manager:
- Tools → SDK Manager → SDK Tools → NDK (Side by side)

### CMake Not Found

```
Error: CMake version X.X.X not found
```

**Solution**: Install CMake via Android Studio SDK Manager:
- Tools → SDK Manager → SDK Tools → CMake

### Build Fails with "Failed to fetch hev-socks5-tunnel"

**Solution**: Check internet connection. CMake needs to download from GitHub.

### Gradle Sync Failed

**Solution**: 
1. File → Invalidate Caches / Restart
2. Ensure you have internet connection
3. Check `gradle.properties` settings

## Advanced Build Options

### Build Specific ABI Only

Edit `library/build.gradle`:

```gradle
ndk {
    abiFilters 'arm64-v8a'  // Only build for ARM64
}
```

### Enable Build Logging

```bash
./gradlew :library:assembleRelease --info
```

### Parallel Build

```bash
./gradlew build --parallel --max-workers=4
```

### Clean and Rebuild

```bash
./gradlew clean build --refresh-dependencies
```

## Publishing the Library

### Local Maven Repository

```bash
./gradlew :library:publishToMavenLocal
```

### Generate AAR with Sources

The build process automatically generates:
- `library-release.aar` - Release binary
- `library-debug.aar` - Debug binary with symbols

## CI/CD Integration

### GitHub Actions Example

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Set up JDK 11
        uses: actions/setup-java@v3
        with:
          java-version: '11'
          distribution: 'temurin'
      
      - name: Setup Android SDK
        uses: android-actions/setup-android@v2
      
      - name: Build with Gradle
        run: ./gradlew build
      
      - name: Upload AAR
        uses: actions/upload-artifact@v3
        with:
          name: library-aar
          path: library/build/outputs/aar/*.aar
```

## Performance Optimization

### Release Build Optimization

Release builds include:
- R8 code shrinking and obfuscation
- Native code stripping
- Resource shrinking

### Build Cache

Enable build cache in `gradle.properties`:

```properties
org.gradle.caching=true
```

## Development Tips

1. **Incremental Builds**: Only changed files are recompiled
2. **Build Variants**: Use debug builds during development
3. **ABI Splits**: Consider ABI splits for smaller APKs
4. **NDK Debug**: Use debug builds with debuggable native code

## System Requirements

### Minimum

- RAM: 8 GB
- Disk: 10 GB free space
- Internet: Required for first build (downloads dependencies)

### Recommended

- RAM: 16 GB
- Disk: 20 GB free space
- SSD: Recommended for faster builds

## Build Time Estimates

- **First build**: 10-15 minutes (downloads dependencies)
- **Clean build**: 5-8 minutes
- **Incremental build**: 30 seconds - 2 minutes

Times vary based on hardware and internet speed.

## Support

For build issues, please check:
1. This document
2. README.md
3. GitHub Issues: https://github.com/zak20090/hev-socks5-tunnel-android/issues
