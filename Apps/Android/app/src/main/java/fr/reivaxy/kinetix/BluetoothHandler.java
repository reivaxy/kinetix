package fr.reivaxy.kinetix;

import static android.bluetooth.BluetoothAdapter.*;

import android.annotation.SuppressLint;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.preference.PreferenceManager;

import java.util.Arrays;
import java.util.UUID;

public class BluetoothHandler  {

    private final static String TAG = BluetoothHandler.class.getSimpleName();
    public final static String ACTION_GATT_CONNECTED =
            "com.example.bluetooth.le.ACTION_GATT_CONNECTED";
    public final static String ACTION_GATT_DISCONNECTED =
            "com.example.bluetooth.le.ACTION_GATT_DISCONNECTED";
    public final static String ACTION_GATT_SERVICES_DISCOVERED =
            "com.example.bluetooth.le.ACTION_GATT_SERVICES_DISCOVERED";
    public final static String ACTION_DATA_AVAILABLE =
            "com.example.bluetooth.le.ACTION_DATA_AVAILABLE";
    public final static String EXTRA_DATA =
            "com.example.bluetooth.le.EXTRA_DATA";
    public static final String SERVICE_UUID = "89d60870-9908-4472-8f8c-e5b3e6573cd1";
    public static final String MOVEMENT_CHARACTERISTIC_UUID = "39dea685-a63e-44b2-8819-9a202581f8fe";
    public static final String CONFIG_CHARACTERISTIC_UUID = "b2a49d41-a2ac-48c3-b6c8-cfd05640654e";
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothGatt mBluetoothGatt;
    private BluetoothManager mBluetoothManager;
    private String mBluetoothDeviceAddress;
    private int mConnectionState = STATE_DISCONNECTED;
    private static BluetoothHandler instance = null;
    private static Context context = null;
    private BluetoothGattService mCustomService;

    // We want a singleton for the whole app.
    private BluetoothHandler() {
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    }

//    @Nullable
//    @Override
//    public IBinder onBind(Intent intent) {
//        Log.i(TAG, "onBind");
//        return null;
//    }

    private void broadcastUpdate(final String action) {
        final Intent intent = new Intent(action);
//        sendBroadcast(intent);
    }

    public static BluetoothHandler getInstance() {
        if (instance == null) {
            instance = new BluetoothHandler();
        }
        return instance;
    }

    // Implements callback methods for GATT events that the app cares about.  For example,
    // connection change and services discovered.
    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            String intentAction;
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.i(TAG, "Connected to GATT server.");
                mConnectionState = STATE_CONNECTED;
                broadcastUpdate(ACTION_GATT_CONNECTED);
                Log.i(TAG, "Attempting to start service discovery:" + mBluetoothGatt.discoverServices());

            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                mConnectionState = STATE_DISCONNECTED;
                Log.i(TAG, "Disconnected from GATT server.");
                broadcastUpdate(ACTION_GATT_DISCONNECTED);
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
//                broadcastUpdate(ACTION_GATT_SERVICES_DISCOVERED);
                /* check if the service is available on the device */
                mCustomService = mBluetoothGatt.getService(UUID.fromString(SERVICE_UUID));
                BluetoothGattCharacteristic configCharacteristic = mCustomService.getCharacteristic(UUID.fromString(CONFIG_CHARACTERISTIC_UUID));
                mBluetoothGatt.readCharacteristic(configCharacteristic);
            } else {
                Log.w(TAG, "onServicesDiscovered received: " + status);
            }
        }
        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic,
                                         int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
//                broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
                if (characteristic.getUuid().toString().equals(CONFIG_CHARACTERISTIC_UUID)) {
                    BluetoothGattCharacteristic configCharacteristic = mCustomService.getCharacteristic(UUID.fromString(CONFIG_CHARACTERISTIC_UUID));

                    String version = configCharacteristic.getStringValue(0);
                    Log.i(TAG, "Got firmware version " + version);

                    SharedPreferences sharedPref = context.getSharedPreferences(
                            context.getString(R.string.preference_file_key), Context.MODE_PRIVATE);
                    SharedPreferences.Editor editor = sharedPref.edit();
                    editor.putString(context.getString(R.string.saved_version_key), version);
                    editor.apply();

                }
            }
        }
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic) {
//            broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
        }
    };

    public void writeCustomCharacteristic(byte[] value) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }

        if (mCustomService == null) {
            Log.w(TAG, "Custom BLE Service not found");
            return;
        }
        /*get the movement write characteristic from the service*/
        BluetoothGattCharacteristic movementWriteCharacteristic = mCustomService.getCharacteristic(UUID.fromString(MOVEMENT_CHARACTERISTIC_UUID));
        Log.i(TAG, String.format("Sending movement '%s'", new String(value)));
        movementWriteCharacteristic.setValue(value);
        if (!mBluetoothGatt.writeCharacteristic(movementWriteCharacteristic)) {
            Log.w(TAG, "Failed to write characteristic");
        }
    }

    /**
     * Connects to the GATT server hosted on the Bluetooth LE device.
     *
     * @param address The device address of the destination device.
     * @return Return true if the connection is initiated successfully. The connection result
     * is reported asynchronously through the
     * {@code BluetoothGattCallback#onConnectionStateChange(android.bluetooth.BluetoothGatt, int, int)}
     * callback.
     */
    public boolean connect(Context context, final String address) {
        this.context = context;
        if (mBluetoothAdapter == null || address == null) {
            Log.w(TAG, "BluetoothAdapter not initialized or unspecified address.");
            return false;
        }
        // Previously connected device.  Try to reconnect.
        if (mBluetoothDeviceAddress != null && address.equals(mBluetoothDeviceAddress)
                && mBluetoothGatt != null) {
            Log.d(TAG, "Trying to use an existing mBluetoothGatt for connection.");
            if (mBluetoothGatt.connect()) {
                mConnectionState = STATE_CONNECTING;
                return true;
            } else {
                return false;
            }
        }
        final BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);
        if (device == null) {
            Log.w(TAG, "Device not found.  Unable to connect.");
            return false;
        }
        // We want to directly connect to the device, so we are setting the autoConnect
        // parameter to false.

        mBluetoothGatt = device.connectGatt(context, false, mGattCallback);
        Log.d(TAG, "Trying to create a new connection.");
        mBluetoothDeviceAddress = address;
        mConnectionState = STATE_CONNECTING;

        return true;

    }

    @SuppressLint("MissingPermission")
    public void close() {
        if (mBluetoothGatt == null) {
            return;
        }
        mBluetoothGatt.disconnect();
        mBluetoothGatt.close();
        mBluetoothGatt = null;
    }

    public boolean isConnected() {
        return mConnectionState == STATE_CONNECTED;
    }
}
