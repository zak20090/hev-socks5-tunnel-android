package com.example.demo;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.net.VpnService;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import androidx.core.app.NotificationCompat;

import cc.hev.socks5.tunnel.HevSocks5Tunnel;
import cc.hev.socks5.tunnel.TunnelConfig;
import cc.hev.socks5.tunnel.TunnelException;
import cc.hev.socks5.tunnel.TunnelStats;

import java.io.FileDescriptor;
import java.lang.reflect.Field;

public class ExampleVpnService extends VpnService {
    private static final String TAG = "ExampleVpnService";
    private static final String CHANNEL_ID = "vpn_service_channel";
    private static final int NOTIFICATION_ID = 1;
    
    public static final String ACTION_START = "com.example.demo.START_VPN";
    public static final String ACTION_STOP = "com.example.demo.STOP_VPN";
    
    private final IBinder binder = new LocalBinder();
    private HevSocks5Tunnel tunnel;
    private ParcelFileDescriptor tunInterface;
    private volatile boolean running = false;
    
    public class LocalBinder extends Binder {
        ExampleVpnService getService() {
            return ExampleVpnService.this;
        }
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "Service created");
        createNotificationChannel();
    }
    
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent == null) {
            return START_NOT_STICKY;
        }
        
        String action = intent.getAction();
        if (ACTION_START.equals(action)) {
            startTunnel();
        } else if (ACTION_STOP.equals(action)) {
            stopTunnel();
        }
        
        return START_STICKY;
    }
    
    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
        stopTunnel();
        Log.i(TAG, "Service destroyed");
    }
    
    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID,
                getString(R.string.notification_channel_name),
                NotificationManager.IMPORTANCE_LOW
            );
            channel.setDescription(getString(R.string.notification_channel_desc));
            
            NotificationManager manager = getSystemService(NotificationManager.class);
            if (manager != null) {
                manager.createNotificationChannel(channel);
            }
        }
    }
    
    private Notification createNotification() {
        Intent notificationIntent = new Intent(this, MainActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(
            this,
            0,
            notificationIntent,
            PendingIntent.FLAG_IMMUTABLE
        );
        
        return new NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle(getString(R.string.notification_title))
            .setContentText(getString(R.string.notification_text))
            .setSmallIcon(android.R.drawable.ic_dialog_info)
            .setContentIntent(pendingIntent)
            .setOngoing(true)
            .build();
    }
    
    public void startTunnel() {
        if (running) {
            Log.w(TAG, "Tunnel already running");
            return;
        }
        
        try {
            // Create VPN interface
            Builder builder = new Builder();
            builder.setSession("HevSocks5Tunnel")
                .addAddress("10.0.0.2", 24)
                .addRoute("0.0.0.0", 0)
                .addDnsServer("8.8.8.8")
                .setMtu(8500);
            
            tunInterface = builder.establish();
            if (tunInterface == null) {
                Log.e(TAG, "Failed to establish VPN interface");
                return;
            }
            
            // Start foreground service
            startForeground(NOTIFICATION_ID, createNotification());
            
            // Get FileDescriptor from ParcelFileDescriptor
            FileDescriptor tunFd = tunInterface.getFileDescriptor();
            
            // Create tunnel configuration
            // NOTE: You should configure this with your actual SOCKS5 server
            TunnelConfig config = new TunnelConfig.Builder()
                .setSocks5Address("127.0.0.1")  // Replace with your SOCKS5 server
                .setSocks5Port(1080)             // Replace with your SOCKS5 port
                .setTunMtu(8500)
                .build();
            
            // Create and start tunnel
            tunnel = new HevSocks5Tunnel();
            tunnel.startAsync(config, tunFd);
            
            running = true;
            Log.i(TAG, "Tunnel started successfully");
            
        } catch (TunnelException e) {
            Log.e(TAG, "Failed to start tunnel", e);
            cleanup();
        } catch (Exception e) {
            Log.e(TAG, "Unexpected error starting tunnel", e);
            cleanup();
        }
    }
    
    public void stopTunnel() {
        if (!running) {
            Log.w(TAG, "Tunnel not running");
            return;
        }
        
        Log.i(TAG, "Stopping tunnel");
        running = false;
        
        if (tunnel != null) {
            try {
                tunnel.stop();
            } catch (Exception e) {
                Log.e(TAG, "Error stopping tunnel", e);
            }
            tunnel = null;
        }
        
        cleanup();
        Log.i(TAG, "Tunnel stopped");
    }
    
    private void cleanup() {
        if (tunInterface != null) {
            try {
                tunInterface.close();
            } catch (Exception e) {
                Log.e(TAG, "Error closing TUN interface", e);
            }
            tunInterface = null;
        }
        
        stopForeground(true);
        stopSelf();
    }
    
    public boolean isRunning() {
        return running && tunnel != null && tunnel.isRunning();
    }
    
    public TunnelStats getStats() {
        if (tunnel != null && running) {
            return tunnel.getStats();
        }
        return new TunnelStats(0, 0, 0, 0);
    }
}
