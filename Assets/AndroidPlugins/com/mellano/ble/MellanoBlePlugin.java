package com.mellano.ble;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;

import com.unity3d.player.UnityPlayer;

import java.nio.charset.StandardCharsets;
import java.util.UUID;

public class MellanoBlePlugin {
    private static MellanoBlePlugin instance;

    private final Activity activity;
    private final BluetoothManager bluetoothManager;
    private final BluetoothAdapter bluetoothAdapter;
    private BluetoothLeScanner scanner;
    private BluetoothGatt gatt;
    private BluetoothGattCharacteristic writeCharacteristic;

    private static final String UNITY_RECEIVER = "DeviceConfigService";

    private static final UUID SERVICE_UUID =
            UUID.fromString("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    private static final UUID WRITE_UUID =
            UUID.fromString("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");

    public MellanoBlePlugin(Activity activity) {
        this.activity = activity;
        this.bluetoothManager = (BluetoothManager) activity.getSystemService(Context.BLUETOOTH_SERVICE);
        this.bluetoothAdapter = bluetoothManager.getAdapter();
    }

    public static MellanoBlePlugin getInstance() {
        if (instance == null) {
            instance = new MellanoBlePlugin(UnityPlayer.currentActivity);
        }
        return instance;
    }

    public boolean isBluetoothReady() {
        return bluetoothAdapter != null && bluetoothAdapter.isEnabled();
    }

    public void startScan() {
        if (bluetoothAdapter == null) return;

        scanner = bluetoothAdapter.getBluetoothLeScanner();
        if (scanner == null) return;

        UnityPlayer.UnitySendMessage(UNITY_RECEIVER, "OnBleStatus", "SCAN_STARTED");
        scanner.startScan(scanCallback);
    }

    public void stopScan() {
        if (scanner != null) {
            scanner.stopScan(scanCallback);
            UnityPlayer.UnitySendMessage(UNITY_RECEIVER, "OnBleStatus", "SCAN_STOPPED");
        }
    }

    public void disconnect() {
        if (gatt != null) {
            gatt.disconnect();
            gatt.close();
            gatt = null;
            writeCharacteristic = null;
            UnityPlayer.UnitySendMessage(UNITY_RECEIVER, "OnBleConnectionChanged", "DISCONNECTED");
        }
    }

    public void connectToAddress(final String address) {
        if (bluetoothAdapter == null) return;

        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
        if (device == null) return;

        UnityPlayer.UnitySendMessage(UNITY_RECEIVER, "OnBleStatus", "CONNECTING:" + address);
        gatt = device.connectGatt(activity, false, gattCallback);
    }

    public void writeCommand(final String command) {
        if (gatt == null || writeCharacteristic == null) {
            UnityPlayer.UnitySendMessage(UNITY_RECEIVER, "OnBleStatus", "WRITE_FAILED:NO_CHARACTERISTIC");
            return;
        }

        byte[] payload = (command + "\n").getBytes(StandardCharsets.UTF_8);
        writeCharacteristic.setValue(payload);
        boolean ok = gatt.writeCharacteristic(writeCharacteristic);

        UnityPlayer.UnitySendMessage(UNITY_RECEIVER, "OnBleStatus",
                ok ? "WRITE_OK:" + command : "WRITE_FAILED:" + command);
    }

    private final ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            if (result == null || result.getDevice() == null) return;

            String name = result.getDevice().getName();
            String address = result.getDevice().getAddress();

            if (name != null && name.contains("Mellano Config")) {
                UnityPlayer.UnitySendMessage(UNITY_RECEIVER, "OnBleDeviceFound", name + "|" + address);
            }
        }
    };

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt g, int status, int newState) {
            if (newState == BluetoothGatt.STATE_CONNECTED) {
                UnityPlayer.UnitySendMessage(UNITY_RECEIVER, "OnBleConnectionChanged", "CONNECTED");
                g.discoverServices();
            } else if (newState == BluetoothGatt.STATE_DISCONNECTED) {
                writeCharacteristic = null;
                UnityPlayer.UnitySendMessage(UNITY_RECEIVER, "OnBleConnectionChanged", "DISCONNECTED");
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt g, int status) {
            BluetoothGattService service = g.getService(SERVICE_UUID);
            if (service == null) {
                UnityPlayer.UnitySendMessage(UNITY_RECEIVER, "OnBleStatus", "SERVICE_NOT_FOUND");
                return;
            }

            writeCharacteristic = service.getCharacteristic(WRITE_UUID);
            if (writeCharacteristic == null) {
                UnityPlayer.UnitySendMessage(UNITY_RECEIVER, "OnBleStatus", "WRITE_CHAR_NOT_FOUND");
                return;
            }

            UnityPlayer.UnitySendMessage(UNITY_RECEIVER, "OnBleStatus", "READY");
        }
    };
}