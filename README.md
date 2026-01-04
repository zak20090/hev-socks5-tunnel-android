# HevSocks5Tunnel Android

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Android](https://img.shields.io/badge/Android-21%2B-green.svg)](https://developer.android.com)

A complete Android library that provides a JNI wrapper for [hev-socks5-tunnel](https://github.com/heiher/hev-socks5-tunnel), enabling SOCKS5 proxy tunneling through Android VPN service.

## Features

- ✅ Complete JNI wrapper for hev-socks5-tunnel
- ✅ Easy-to-use Java API
- ✅ Thread-safe implementation
- ✅ Support for both config file and programmatic configuration
- ✅ Real-time tunnel statistics
- ✅ VPN service integration
- ✅ Support for all major ABIs (armeabi-v7a, arm64-v8a, x86, x86_64)
- ✅ Automatic download and build of native dependencies

## Requirements

- Android SDK 21 (Android 5.0) or higher
- NDK 25.1.8937393 or higher
- CMake 3.22.1 or higher
- Gradle 8.0 or higher

## Installation

### Add to your project

Add the library module to your `settings.gradle`:

```gradle
include ':library'
project(':library').projectDir = new File('path/to/library')
```

Add dependency in your app's `build.gradle`:

```gradle
dependencies {
    implementation project(':library')
}
```

### Build from source

Clone the repository:

```bash
git clone https://github.com/zak20090/hev-socks5-tunnel-android.git
cd hev-socks5-tunnel-android
```

Build the library:

```bash
./gradlew :library:build
```

## Usage

### Basic Usage

```java
import cc.hev.socks5.tunnel.HevSocks5Tunnel;
import cc.hev.socks5.tunnel.TunnelConfig;
import cc.hev.socks5.tunnel.TunnelException;
import cc.hev.socks5.tunnel.TunnelStats;

// Create configuration
TunnelConfig config = new TunnelConfig.Builder()
    .setSocks5Address("your.socks5.server")
    .setSocks5Port(1080)
    .setSocks5Username("username")  // Optional
    .setSocks5Password("password")  // Optional
    .setTunMtu(8500)
    .build();

// Create tunnel instance
HevSocks5Tunnel tunnel = new HevSocks5Tunnel();

// Start tunnel (requires VPN FileDescriptor)
try {
    tunnel.startAsync(config, tunFd);
} catch (TunnelException e) {
    // Handle error
}

// Get statistics
TunnelStats stats = tunnel.getStats();
System.out.println("TX: " + stats.getTxBytes() + " bytes");
System.out.println("RX: " + stats.getRxBytes() + " bytes");

// Stop tunnel
tunnel.stop();
```

### Using with VPN Service

See the complete example in `app/src/main/java/com/example/demo/ExampleVpnService.java`:

```java
public class MyVpnService extends VpnService {
    private HevSocks5Tunnel tunnel;
    private ParcelFileDescriptor tunInterface;
    
    public void startTunnel() {
        // Create VPN interface
        Builder builder = new Builder();
        builder.setSession("MyVPN")
            .addAddress("10.0.0.2", 24)
            .addRoute("0.0.0.0", 0)
            .addDnsServer("8.8.8.8")
            .setMtu(8500);
        
        tunInterface = builder.establish();
        FileDescriptor tunFd = tunInterface.getFileDescriptor();
        
        // Create configuration
        TunnelConfig config = new TunnelConfig.Builder()
            .setSocks5Address("your.server.com")
            .setSocks5Port(1080)
            .build();
        
        // Start tunnel
        tunnel = new HevSocks5Tunnel();
        try {
            tunnel.startAsync(config, tunFd);
        } catch (TunnelException e) {
            Log.e(TAG, "Failed to start tunnel", e);
        }
    }
    
    public void stopTunnel() {
        if (tunnel != null) {
            tunnel.stop();
        }
        if (tunInterface != null) {
            try {
                tunInterface.close();
            } catch (IOException e) {
                // Handle error
            }
        }
    }
}
```

### Configuration Options

```java
TunnelConfig config = new TunnelConfig.Builder()
    // SOCKS5 server settings (required)
    .setSocks5Address("proxy.example.com")
    .setSocks5Port(1080)
    
    // SOCKS5 authentication (optional)
    .setSocks5Username("username")
    .setSocks5Password("password")
    
    // TUN interface settings
    .setTunName("tun0")
    .setTunMtu(8500)
    .setTunIPv4Address("10.0.0.2")
    .setTunIPv4Gateway("10.0.0.1")
    .setTunIPv6Address("fc00::2")
    .setTunIPv6Gateway("fc00::1")
    
    // DNS settings
    .addDnsServer("8.8.8.8")
    .addDnsServer("8.8.4.4")
    
    // Performance tuning
    .setMultiQueue(4)
    
    .build();
```

### Using Config File

You can also start the tunnel with a YAML config file:

```java
String configPath = "/path/to/config.yaml";
tunnel.startAsync(configPath, tunFd);
```

Example config file format:

```yaml
tunnel:
  name: tun0
  mtu: 8500
  multi-queue: 4
  ipv4:
    address: 10.0.0.2
    gateway: 10.0.0.1

socks5:
  address: your.socks5.server
  port: 1080
  username: user
  password: pass

misc:
  dns:
    - 8.8.8.8
    - 8.8.4.4
```

## API Reference

### HevSocks5Tunnel

Main class for tunnel management.

#### Methods

- `static boolean isLibraryLoaded()` - Check if native library loaded successfully
- `static Throwable getLibraryLoadError()` - Get library load error if any
- `void startAsync(String configPath, FileDescriptor tunFd)` - Start tunnel with config file
- `void startAsync(TunnelConfig config, FileDescriptor tunFd)` - Start tunnel with config object
- `void stop()` - Stop the tunnel
- `boolean isRunning()` - Check if tunnel is running
- `TunnelStats getStats()` - Get tunnel statistics

### TunnelConfig

Configuration builder for tunnel settings.

#### Builder Methods

- `setSocks5Address(String)` - Set SOCKS5 server address
- `setSocks5Port(int)` - Set SOCKS5 server port
- `setSocks5Username(String)` - Set SOCKS5 username (optional)
- `setSocks5Password(String)` - Set SOCKS5 password (optional)
- `setTunName(String)` - Set TUN interface name
- `setTunMtu(int)` - Set TUN MTU (1280-65535)
- `setTunIPv4Address(String)` - Set TUN IPv4 address
- `setTunIPv4Gateway(String)` - Set TUN IPv4 gateway
- `setTunIPv6Address(String)` - Set TUN IPv6 address
- `setTunIPv6Gateway(String)` - Set TUN IPv6 gateway
- `addDnsServer(String)` - Add DNS server
- `setDnsServers(List<String>)` - Set DNS servers list
- `setMultiQueue(int)` - Set multi-queue value
- `build()` - Build configuration

### TunnelStats

Statistics for data transfer.

#### Methods

- `long getTxBytes()` - Get transmitted bytes
- `long getRxBytes()` - Get received bytes
- `long getTxPackets()` - Get transmitted packets
- `long getRxPackets()` - Get received packets
- `static String formatBytes(long)` - Format bytes to human-readable string
- `String toString()` - Get formatted statistics string

### TunnelException

Exception thrown when tunnel operations fail.

## Example App

The repository includes a complete example app demonstrating VPN service integration:

- `MainActivity.java` - UI with start/stop buttons and statistics display
- `ExampleVpnService.java` - VPN service implementation with foreground notification
- Complete lifecycle management
- Real-time statistics updates

To run the example:

```bash
./gradlew :app:installDebug
```

## Building

### Build the library

```bash
./gradlew :library:assembleRelease
```

Output: `library/build/outputs/aar/library-release.aar`

### Build the example app

```bash
./gradlew :app:assembleDebug
```

### Build all

```bash
./gradlew build
```

## Architecture

```
┌─────────────────────────────────────┐
│        Android Application          │
│     (MainActivity, VpnService)      │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│      Java API Layer                 │
│   (HevSocks5Tunnel, TunnelConfig)   │
└──────────────┬──────────────────────┘
               │ JNI
┌──────────────▼──────────────────────┐
│      JNI Wrapper (C++)              │
│   (hev_socks5_tunnel_jni.cpp)       │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│    hev-socks5-tunnel (C)            │
│   (Automatically fetched by CMake)  │
└─────────────────────────────────────┘
```

## Supported ABIs

- armeabi-v7a (ARM 32-bit)
- arm64-v8a (ARM 64-bit)
- x86 (Intel 32-bit)
- x86_64 (Intel 64-bit)

## ProGuard

The library includes ProGuard rules to keep all necessary classes and methods. No additional configuration needed.

## Thread Safety

The library is thread-safe and can be safely called from multiple threads. All operations are properly synchronized.

## Error Handling

All errors are reported through `TunnelException`. Always wrap tunnel operations in try-catch blocks:

```java
try {
    tunnel.startAsync(config, tunFd);
} catch (TunnelException e) {
    Log.e(TAG, "Tunnel error: " + e.getMessage(), e);
    // Handle error appropriately
}
```

## Logging

The library uses Android's Log system with tag `HevSocks5Tunnel`. Native logs use tag `HevSocks5TunnelJNI`.

To see logs:

```bash
adb logcat -s HevSocks5Tunnel HevSocks5TunnelJNI
```

## Troubleshooting

### Library not loaded

If you get "Native library not loaded" error:

1. Check that all ABIs are built correctly
2. Verify NDK is installed
3. Check CMake configuration

```java
if (!HevSocks5Tunnel.isLibraryLoaded()) {
    Throwable error = HevSocks5Tunnel.getLibraryLoadError();
    Log.e(TAG, "Library load failed", error);
}
```

### VPN permission denied

Make sure to request VPN permission:

```java
Intent intent = VpnService.prepare(context);
if (intent != null) {
    startActivityForResult(intent, VPN_REQUEST_CODE);
}
```

### Tunnel fails to start

1. Verify SOCKS5 server is accessible
2. Check TUN file descriptor is valid
3. Review logs for detailed error messages
4. Ensure proper permissions in AndroidManifest.xml

## Performance Tips

1. Use appropriate MTU size (8500 recommended)
2. Adjust multi-queue value based on CPU cores
3. Consider using release builds for production
4. Monitor statistics to detect issues

## Security Notes

1. Never hardcode credentials in your app
2. Use secure storage for sensitive data
3. Validate all user inputs
4. Keep the library updated
5. Use ProGuard/R8 in release builds

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Credits

This project is a wrapper around [hev-socks5-tunnel](https://github.com/heiher/hev-socks5-tunnel) by heiher.

Special thanks to:
- [heiher](https://github.com/heiher) for the excellent hev-socks5-tunnel library
- The Android open source community

## License

This project is licensed under the MIT License - see the LICENSE file for details.

The underlying hev-socks5-tunnel library is licensed under its own terms. Please refer to its repository for details.

## Support

For issues, questions, or contributions, please use GitHub Issues:
https://github.com/zak20090/hev-socks5-tunnel-android/issues

## Changelog

### Version 1.0.0
- Initial release
- Complete JNI wrapper implementation
- Support for all major ABIs
- Example VPN service application
- Comprehensive documentation
