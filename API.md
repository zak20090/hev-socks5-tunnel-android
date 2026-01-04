# API Documentation

Complete API reference for HevSocks5Tunnel Android library.

## Package: cc.hev.socks5.tunnel

---

## HevSocks5Tunnel

Main class for managing the SOCKS5 tunnel.

### Constructor

#### `HevSocks5Tunnel()`

Creates a new tunnel instance.

**Throws:**
- `RuntimeException` - If native library failed to load

**Example:**
```java
HevSocks5Tunnel tunnel = new HevSocks5Tunnel();
```

### Static Methods

#### `static boolean isLibraryLoaded()`

Check if native library loaded successfully.

**Returns:** `true` if library is loaded, `false` otherwise

**Example:**
```java
if (!HevSocks5Tunnel.isLibraryLoaded()) {
    Log.e(TAG, "Library not loaded");
}
```

#### `static Throwable getLibraryLoadError()`

Get the error that occurred during library loading, if any.

**Returns:** The exception that occurred, or `null` if library loaded successfully

**Example:**
```java
if (!HevSocks5Tunnel.isLibraryLoaded()) {
    Throwable error = HevSocks5Tunnel.getLibraryLoadError();
    Log.e(TAG, "Load error", error);
}
```

### Instance Methods

#### `void startAsync(String configPath, FileDescriptor tunFd)`

Start tunnel using a configuration file.

**Parameters:**
- `configPath` - Path to YAML configuration file
- `tunFd` - TUN interface file descriptor from VPN service

**Throws:**
- `TunnelException` - If tunnel is already running, config path is invalid, or tunnel fails to start

**Example:**
```java
String configPath = getFilesDir() + "/config.yaml";
FileDescriptor tunFd = vpnInterface.getFileDescriptor();
try {
    tunnel.startAsync(configPath, tunFd);
} catch (TunnelException e) {
    Log.e(TAG, "Failed to start", e);
}
```

#### `void startAsync(TunnelConfig config, FileDescriptor tunFd)`

Start tunnel using a configuration object.

**Parameters:**
- `config` - Tunnel configuration object
- `tunFd` - TUN interface file descriptor from VPN service

**Throws:**
- `TunnelException` - If tunnel is already running, config is invalid, or tunnel fails to start

**Example:**
```java
TunnelConfig config = new TunnelConfig.Builder()
    .setSocks5Address("proxy.example.com")
    .setSocks5Port(1080)
    .build();
    
FileDescriptor tunFd = vpnInterface.getFileDescriptor();
try {
    tunnel.startAsync(config, tunFd);
} catch (TunnelException e) {
    Log.e(TAG, "Failed to start", e);
}
```

#### `void stop()`

Stop the tunnel.

Safe to call even if tunnel is not running.

**Example:**
```java
tunnel.stop();
```

#### `boolean isRunning()`

Check if tunnel is currently running.

**Returns:** `true` if tunnel is running, `false` otherwise

**Example:**
```java
if (tunnel.isRunning()) {
    Log.i(TAG, "Tunnel is active");
}
```

#### `TunnelStats getStats()`

Get current tunnel statistics.

Returns zeros if tunnel is not running.

**Returns:** Current tunnel statistics

**Example:**
```java
TunnelStats stats = tunnel.getStats();
Log.i(TAG, "TX: " + stats.getTxBytes() + " bytes");
Log.i(TAG, "RX: " + stats.getRxBytes() + " bytes");
```

---

## TunnelConfig

Configuration class for tunnel settings. Use the Builder pattern to construct.

### Builder

#### `TunnelConfig.Builder()`

Creates a new configuration builder with default values.

**Default values:**
- SOCKS5 address: `127.0.0.1`
- SOCKS5 port: `1080`
- TUN name: `tun0`
- TUN MTU: `8500`
- TUN IPv4: `10.0.0.2`
- TUN IPv4 Gateway: `10.0.0.1`
- TUN IPv6: `fc00::2`
- TUN IPv6 Gateway: `fc00::1`
- DNS servers: `8.8.8.8`, `8.8.4.4`
- Multi-queue: `4`

### Builder Methods

#### `Builder setSocks5Address(String address)`

Set SOCKS5 server address.

**Parameters:**
- `address` - Server address (hostname or IP)

**Returns:** This builder

**Example:**
```java
builder.setSocks5Address("proxy.example.com");
```

#### `Builder setSocks5Port(int port)`

Set SOCKS5 server port.

**Parameters:**
- `port` - Port number (1-65535)

**Throws:**
- `IllegalArgumentException` - If port is invalid

**Returns:** This builder

**Example:**
```java
builder.setSocks5Port(1080);
```

#### `Builder setSocks5Username(String username)`

Set SOCKS5 username for authentication (optional).

**Parameters:**
- `username` - Username, or `null` for no authentication

**Returns:** This builder

**Example:**
```java
builder.setSocks5Username("myuser");
```

#### `Builder setSocks5Password(String password)`

Set SOCKS5 password for authentication (optional).

**Parameters:**
- `password` - Password, or `null` for no authentication

**Returns:** This builder

**Example:**
```java
builder.setSocks5Password("mypassword");
```

#### `Builder setTunName(String name)`

Set TUN interface name.

**Parameters:**
- `name` - Interface name

**Returns:** This builder

**Example:**
```java
builder.setTunName("tun0");
```

#### `Builder setTunMtu(int mtu)`

Set TUN interface MTU.

**Parameters:**
- `mtu` - MTU value (1280-65535)

**Throws:**
- `IllegalArgumentException` - If MTU is invalid

**Returns:** This builder

**Example:**
```java
builder.setTunMtu(8500);
```

#### `Builder setTunIPv4Address(String address)`

Set TUN IPv4 address.

**Parameters:**
- `address` - IPv4 address, or `null` to disable IPv4

**Returns:** This builder

**Example:**
```java
builder.setTunIPv4Address("10.0.0.2");
```

#### `Builder setTunIPv4Gateway(String gateway)`

Set TUN IPv4 gateway.

**Parameters:**
- `gateway` - IPv4 gateway address

**Returns:** This builder

**Example:**
```java
builder.setTunIPv4Gateway("10.0.0.1");
```

#### `Builder setTunIPv6Address(String address)`

Set TUN IPv6 address.

**Parameters:**
- `address` - IPv6 address, or `null` to disable IPv6

**Returns:** This builder

**Example:**
```java
builder.setTunIPv6Address("fc00::2");
```

#### `Builder setTunIPv6Gateway(String gateway)`

Set TUN IPv6 gateway.

**Parameters:**
- `gateway` - IPv6 gateway address

**Returns:** This builder

**Example:**
```java
builder.setTunIPv6Gateway("fc00::1");
```

#### `Builder addDnsServer(String server)`

Add a DNS server.

**Parameters:**
- `server` - DNS server address

**Returns:** This builder

**Example:**
```java
builder.addDnsServer("8.8.8.8");
builder.addDnsServer("1.1.1.1");
```

#### `Builder setDnsServers(List<String> servers)`

Set DNS servers list, replacing any existing servers.

**Parameters:**
- `servers` - List of DNS server addresses

**Returns:** This builder

**Example:**
```java
List<String> dns = Arrays.asList("8.8.8.8", "8.8.4.4");
builder.setDnsServers(dns);
```

#### `Builder setMultiQueue(int queues)`

Set multi-queue value for performance tuning.

**Parameters:**
- `queues` - Number of queues (must be positive)

**Throws:**
- `IllegalArgumentException` - If value is invalid

**Returns:** This builder

**Example:**
```java
builder.setMultiQueue(4);
```

#### `TunnelConfig build()`

Build the configuration.

**Throws:**
- `IllegalStateException` - If SOCKS5 address is not set

**Returns:** Immutable configuration object

**Example:**
```java
TunnelConfig config = new TunnelConfig.Builder()
    .setSocks5Address("proxy.example.com")
    .setSocks5Port(1080)
    .build();
```

### Instance Methods

#### `String toYaml()`

Convert configuration to YAML format for hev-socks5-tunnel.

**Returns:** YAML configuration string

**Example:**
```java
String yaml = config.toYaml();
System.out.println(yaml);
```

---

## TunnelStats

Statistics for tunnel data transfer.

### Constructor

#### `TunnelStats(long txBytes, long rxBytes, long txPackets, long rxPackets)`

Create statistics object.

**Parameters:**
- `txBytes` - Transmitted bytes
- `rxBytes` - Received bytes
- `txPackets` - Transmitted packets
- `rxPackets` - Received packets

### Methods

#### `long getTxBytes()`

Get transmitted bytes.

**Returns:** Number of bytes transmitted

#### `long getRxBytes()`

Get received bytes.

**Returns:** Number of bytes received

#### `long getTxPackets()`

Get transmitted packets.

**Returns:** Number of packets transmitted

#### `long getRxPackets()`

Get received packets.

**Returns:** Number of packets received

#### `static String formatBytes(long bytes)`

Format bytes to human-readable string.

**Parameters:**
- `bytes` - Number of bytes

**Returns:** Formatted string (e.g., "1.50 MiB")

**Example:**
```java
String formatted = TunnelStats.formatBytes(1572864);
// Returns: "1.50 MiB"
```

#### `String toString()`

Get formatted statistics string.

**Returns:** Human-readable statistics

**Example:**
```java
TunnelStats stats = tunnel.getStats();
System.out.println(stats.toString());
// Output: TunnelStats{tx=1.50 MiB (1234 packets), rx=2.30 MiB (2345 packets)}
```

---

## TunnelException

Exception thrown when tunnel operations fail.

### Constructors

#### `TunnelException(String message)`

Create exception with message.

**Parameters:**
- `message` - Error message

#### `TunnelException(String message, Throwable cause)`

Create exception with message and cause.

**Parameters:**
- `message` - Error message
- `cause` - Underlying cause

#### `TunnelException(Throwable cause)`

Create exception with cause.

**Parameters:**
- `cause` - Underlying cause

---

## Usage Examples

### Complete VPN Service Example

```java
public class MyVpnService extends VpnService {
    private HevSocks5Tunnel tunnel;
    private ParcelFileDescriptor vpnInterface;
    
    public void startVpn() {
        // Check library
        if (!HevSocks5Tunnel.isLibraryLoaded()) {
            Log.e(TAG, "Native library not loaded", 
                  HevSocks5Tunnel.getLibraryLoadError());
            return;
        }
        
        // Create VPN interface
        Builder builder = new Builder();
        builder.setSession("MyVPN")
            .addAddress("10.0.0.2", 24)
            .addRoute("0.0.0.0", 0)
            .addDnsServer("8.8.8.8")
            .setMtu(8500);
        
        vpnInterface = builder.establish();
        if (vpnInterface == null) {
            Log.e(TAG, "Failed to establish VPN");
            return;
        }
        
        // Configure tunnel
        TunnelConfig config = new TunnelConfig.Builder()
            .setSocks5Address("proxy.example.com")
            .setSocks5Port(1080)
            .setSocks5Username("user")
            .setSocks5Password("pass")
            .setTunMtu(8500)
            .build();
        
        // Start tunnel
        tunnel = new HevSocks5Tunnel();
        try {
            tunnel.startAsync(config, vpnInterface.getFileDescriptor());
            Log.i(TAG, "Tunnel started");
        } catch (TunnelException e) {
            Log.e(TAG, "Failed to start tunnel", e);
            cleanup();
        }
    }
    
    public void stopVpn() {
        if (tunnel != null) {
            tunnel.stop();
            tunnel = null;
        }
        cleanup();
    }
    
    private void cleanup() {
        if (vpnInterface != null) {
            try {
                vpnInterface.close();
            } catch (IOException e) {
                Log.e(TAG, "Error closing VPN interface", e);
            }
            vpnInterface = null;
        }
    }
    
    public TunnelStats getStats() {
        if (tunnel != null && tunnel.isRunning()) {
            return tunnel.getStats();
        }
        return new TunnelStats(0, 0, 0, 0);
    }
}
```

### Statistics Display

```java
// Update stats every second
handler.postDelayed(new Runnable() {
    @Override
    public void run() {
        TunnelStats stats = tunnel.getStats();
        
        textView.setText(String.format(
            "TX: %s (%d packets)\nRX: %s (%d packets)",
            TunnelStats.formatBytes(stats.getTxBytes()),
            stats.getTxPackets(),
            TunnelStats.formatBytes(stats.getRxBytes()),
            stats.getRxPackets()
        ));
        
        handler.postDelayed(this, 1000);
    }
}, 1000);
```

### Error Handling

```java
try {
    tunnel.startAsync(config, tunFd);
} catch (TunnelException e) {
    // Log the error
    Log.e(TAG, "Tunnel error: " + e.getMessage(), e);
    
    // Show user-friendly message
    if (e.getMessage().contains("already running")) {
        showToast("Tunnel is already active");
    } else if (e.getMessage().contains("Invalid TUN")) {
        showToast("VPN permission required");
    } else {
        showToast("Failed to start tunnel");
    }
    
    // Clean up
    cleanup();
}
```

---

## Thread Safety

All methods in `HevSocks5Tunnel` are thread-safe and can be called from any thread. The tunnel runs on its own background thread.

## Memory Management

The library properly manages native memory. No manual cleanup is required beyond calling `stop()` when done.

## Logging

- Java logs use tag: `HevSocks5Tunnel`
- Native logs use tag: `HevSocks5TunnelJNI`

View logs:
```bash
adb logcat -s HevSocks5Tunnel HevSocks5TunnelJNI
```
