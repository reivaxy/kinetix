package fr.reivaxy.kinetix;

import static android.bluetooth.BluetoothAdapter.*;

import android.annotation.SuppressLint;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import androidx.preference.PreferenceManager;

import java.util.Arrays;
import java.util.UUID;

public class BluetoothHandler  {

    private final static String TAG = BluetoothHandler.class.getSimpleName();
    public final static String ACTION_GATT_CONNECTED =
            "fr.reivaxy.kinetix.ACTION_GATT_CONNECTED";
    public final static String ACTION_GATT_DISCONNECTED =
            "fr.reivaxy.kinetix.ACTION_GATT_DISCONNECTED";

    // Broadcasted when we receive OTA-related notifications from the ESP32.
    public static final String ACTION_OTA_MESSAGE =
            "fr.reivaxy.kinetix.ACTION_OTA_MESSAGE";
    public static final String EXTRA_OTA_MESSAGE = "fr.reivaxy.kinetix.EXTRA_OTA_MESSAGE";
    public static final String SERVICE_UUID = "89d60870-9908-4472-8f8c-e5b3e6573cd1";
    public static final String MOVEMENT_CHARACTERISTIC_UUID = "39dea685-a63e-44b2-8819-9a202581f8fe";
    public static final String CONFIG_CHARACTERISTIC_UUID = "b2a49d41-a2ac-48c3-b6c8-cfd05640654e";
    // OTA characteristic UUID (must match firmware)
    public static final String OTA_CHARACTERISTIC_UUID = "3168e56f-6ea1-420d-98f8-08a3b34afc9b";
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothGatt mBluetoothGatt;
    private String mBluetoothDeviceAddress;
    private int mConnectionState = STATE_DISCONNECTED;
    private static BluetoothHandler instance = null;
    private static Context context = null;
    private BluetoothGattService mCustomService;
    private BluetoothGattCharacteristic mOtaCharacteristic;

    private void broadcastOtaMessage(final String msg) {
        final Intent intent = new Intent(ACTION_OTA_MESSAGE);
        intent.putExtra(EXTRA_OTA_MESSAGE, msg);
        LocalBroadcastManager.getInstance(context).sendBroadcast(intent);
    }

    // We want a singleton for the whole app.
    private BluetoothHandler() {
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    }

    private void broadcastUpdate(final String action) {
        final Intent intent = new Intent(action);
        LocalBroadcastManager.getInstance(context).sendBroadcast(intent);
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
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.i(TAG, "Connected to GATT server.");
                mConnectionState = STATE_CONNECTED;
                broadcastUpdate(ACTION_GATT_CONNECTED);
                // Do NOT call discoverServices() yet; wait for onMtuChanged()
//                Log.i(TAG, "Attempting to start service discovery:" + mBluetoothGatt.discoverServices());
                // Request a larger MTU (247 is widely supported; 517 is max but less reliable)
                boolean started = mBluetoothGatt.requestMtu(247);

            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                mConnectionState = STATE_DISCONNECTED;
                Log.i(TAG, "Disconnected from GATT server.");
                broadcastUpdate(ACTION_GATT_DISCONNECTED);
            }
        }

        @Override
        public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
            Log.i(TAG, "onMtuChanged mtu=" + mtu + " status=" + status);

            // Now discover services
            @SuppressLint("MissingPermission") boolean started = gatt.discoverServices();
            Log.i(TAG, "discoverServices started=" + started);
        }


        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                mCustomService = mBluetoothGatt.getService(UUID.fromString(SERVICE_UUID));
                if (mCustomService == null) {
                    Log.w(TAG, "Custom service not found");
                    return;
                }

                // Cache characteristics
                mOtaCharacteristic = mCustomService.getCharacteristic(UUID.fromString(OTA_CHARACTERISTIC_UUID));
                if (mOtaCharacteristic != null) {
                    enableNotifications(mOtaCharacteristic); // <-- do this FIRST
                } else {
                    Log.w(TAG, "OTA characteristic not found");
                }

                // Defer reading config until CCCD write completes in onDescriptorWrite()
            } else {
                Log.w(TAG, "onServicesDiscovered received: " + status);
            }
        }

        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            Log.i(TAG, "onDescriptorWrite status=" + status + " uuid=" + descriptor.getUuid());

            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.w(TAG, "Descriptor write failed: " + status);
                return;
            }

            // Now it's safe to start the next GATT operation
            if (mCustomService != null) {
                BluetoothGattCharacteristic configCharacteristic =
                        mCustomService.getCharacteristic(UUID.fromString(CONFIG_CHARACTERISTIC_UUID));
                if (configCharacteristic != null) {
                    boolean started = mBluetoothGatt.readCharacteristic(configCharacteristic);
                    Log.i(TAG, "readCharacteristic(config) started=" + started);
                }
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
            if (characteristic.getUuid().toString().equals(OTA_CHARACTERISTIC_UUID)) {
                String msg = characteristic.getStringValue(0);
                if (msg != null) {
                    Log.i(TAG, "OTA notify: " + msg);
                    broadcastOtaMessage(msg);
                }
            }
        }
    };

    @SuppressLint("MissingPermission")
    private void enableNotifications(BluetoothGattCharacteristic characteristic) {
        if (mBluetoothGatt == null) return;

        boolean ok = mBluetoothGatt.setCharacteristicNotification(characteristic, true);
        if (!ok) {
            Log.w(TAG, "setCharacteristicNotification failed");
            return;
        }

        // Write CCCD (0x2902)
        UUID cccdUuid = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

        BluetoothGattDescriptor cccd = null;
        for (BluetoothGattDescriptor d : characteristic.getDescriptors()) {
            Log.i(TAG, "uid: " + d.getUuid());
            if (cccdUuid.equals(d.getUuid())) {
                cccd = d;
                break;
            }
        }

        if (cccd == null) {
            Log.w(TAG, "CCCD descriptor not found. descriptors=" + characteristic.getDescriptors().size());
            return;
        }

        cccd.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        boolean started = mBluetoothGatt.writeDescriptor(cccd);
        Log.i(TAG, "writeDescriptor(CCCD) started=" + started);
        if (!started) {
            Log.w(TAG, "writeDescriptor(CCCD) failed");
        }
    }

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
     * Sends a command to the OTA characteristic (e.g. "OTA_START", "OTA_STOP").
     */
    @SuppressLint("MissingPermission")
    public void writeOtaCharacteristic(byte[] value) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        if (mCustomService == null) {
            Log.w(TAG, "Custom BLE Service not found");
            return;
        }
        if (mOtaCharacteristic == null) {
            mOtaCharacteristic = mCustomService.getCharacteristic(UUID.fromString(OTA_CHARACTERISTIC_UUID));
        }
        if (mOtaCharacteristic == null) {
            Log.w(TAG, "OTA characteristic not found");
            return;
        }

        Log.i(TAG, String.format("Sending OTA cmd '%s'", new String(value)));
        mOtaCharacteristic.setValue(value);
        if (!mBluetoothGatt.writeCharacteristic(mOtaCharacteristic)) {
            Log.w(TAG, "Failed to write OTA characteristic");
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
        close();
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
