package com.example.demo;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.net.VpnService;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import cc.hev.socks5.tunnel.TunnelStats;

public class MainActivity extends AppCompatActivity {
    private static final int VPN_REQUEST_CODE = 100;
    
    private Button startButton;
    private Button stopButton;
    private TextView statusText;
    private TextView statsText;
    
    private ExampleVpnService vpnService;
    private boolean serviceBound = false;
    
    private Handler handler = new Handler(Looper.getMainLooper());
    private Runnable statsUpdater = new Runnable() {
        @Override
        public void run() {
            updateStats();
            handler.postDelayed(this, 1000); // Update every second
        }
    };
    
    private ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            ExampleVpnService.LocalBinder binder = (ExampleVpnService.LocalBinder) service;
            vpnService = binder.getService();
            serviceBound = true;
            updateUI();
        }
        
        @Override
        public void onServiceDisconnected(ComponentName name) {
            vpnService = null;
            serviceBound = false;
        }
    };
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        startButton = findViewById(R.id.startButton);
        stopButton = findViewById(R.id.stopButton);
        statusText = findViewById(R.id.statusText);
        statsText = findViewById(R.id.statsText);
        
        startButton.setOnClickListener(v -> startVpn());
        stopButton.setOnClickListener(v -> stopVpn());
        
        // Bind to service
        Intent intent = new Intent(this, ExampleVpnService.class);
        bindService(intent, serviceConnection, BIND_AUTO_CREATE);
        
        // Start stats updater
        handler.post(statsUpdater);
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        handler.removeCallbacks(statsUpdater);
        if (serviceBound) {
            unbindService(serviceConnection);
            serviceBound = false;
        }
    }
    
    private void startVpn() {
        // Request VPN permission
        Intent intent = VpnService.prepare(this);
        if (intent != null) {
            startActivityForResult(intent, VPN_REQUEST_CODE);
        } else {
            onActivityResult(VPN_REQUEST_CODE, Activity.RESULT_OK, null);
        }
    }
    
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        
        if (requestCode == VPN_REQUEST_CODE) {
            if (resultCode == Activity.RESULT_OK) {
                Intent intent = new Intent(this, ExampleVpnService.class);
                intent.setAction(ExampleVpnService.ACTION_START);
                startService(intent);
                updateUI();
            } else {
                Toast.makeText(this, "VPN permission denied", Toast.LENGTH_SHORT).show();
            }
        }
    }
    
    private void stopVpn() {
        Intent intent = new Intent(this, ExampleVpnService.class);
        intent.setAction(ExampleVpnService.ACTION_STOP);
        startService(intent);
        updateUI();
    }
    
    private void updateUI() {
        boolean running = vpnService != null && vpnService.isRunning();
        
        startButton.setEnabled(!running);
        stopButton.setEnabled(running);
        
        String status = running ? getString(R.string.status_running) : getString(R.string.status_stopped);
        statusText.setText(getString(R.string.vpn_status, status));
    }
    
    private void updateStats() {
        if (vpnService != null && vpnService.isRunning()) {
            TunnelStats stats = vpnService.getStats();
            if (stats != null) {
                statsText.setText(stats.toString());
            }
        } else {
            statsText.setText("No statistics available");
        }
        updateUI();
    }
}
