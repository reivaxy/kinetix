package fr.reivaxy.kinetix;

import static android.bluetooth.BluetoothAdapter.*;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import java.nio.charset.StandardCharsets;
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

    // Broadcasted when the CONFIG characteristic is read.
    public static final String ACTION_SETTINGS_MESSAGE =
            "fr.reivaxy.kinetix.ACTION_SETTINGS_MESSAGE";
    public static final String EXTRA_SETTINGS_MESSAGE = "fr.reivaxy.kinetix.EXTRA_SETTINGS_MESSAGE";

    // Broadcasted when the SYSTEM characteristic is read.
    public static final String ACTION_ABOUT_MESSAGE =
            "fr.reivaxy.kinetix.ACTION_ABOUT_MESSAGE";
    public static final String EXTRA_ABOUT_MESSAGE = "fr.reivaxy.kinetix.EXTRA_ABOUT_MESSAGE";

    // Broadcasted when the PASSWORD characteristic is read.
    public static final String ACTION_PASSWORD_MESSAGE =
            "fr.reivaxy.kinetix.ACTION_PASSWORD_MESSAGE";
    public static final String EXTRA_PASSWORD_MESSAGE = "fr.reivaxy.kinetix.EXTRA_PASSWORD_MESSAGE";

    public static final String SERVICE_UUID = "89d60870-9908-4472-8f8c-e5b3e6573cd1";
    public static final String MOVEMENT_CHARACTERISTIC_UUID = "39dea685-a63e-44b2-8819-9a202581f8fe";
    public static final String ABOUT_CHARACTERISTIC_UUID = "b2a49d41-a2ac-48c3-b6c8-cfd05640654e";
    public static final String SETTINGS_CHARACTERISTIC_UUID = "68b788da-819b-4feb-b478-8d237ef29f5f";
    public static final String OTA_CHARACTERISTIC_UUID = "3168e56f-6ea1-420d-98f8-08a3b34afc9b";
    public static final String PASSWORD_CHARACTERISTIC_UUID = "7c4a2e1f-5b9a-4d8e-9c3b-2f8a1e5c6d7a";
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothGatt mBluetoothGatt;
    private String mBluetoothDeviceAddress;
    private int mConnectionState = STATE_DISCONNECTED;
    private static BluetoothHandler instance = null;
    private static Context context = null;
    private BluetoothGattService mCustomService;
    private BluetoothGattCharacteristic mOtaCharacteristic;
    private BluetoothGattCharacteristic settingsCharacteristic;
    private BluetoothGattCharacteristic movementWriteCharacteristic;
    private BluetoothGattCharacteristic aboutCharacteristic;
    private BluetoothGattCharacteristic passwordCharacteristic;

    private boolean mDeviceHasPassword = false;

    // Cache last about payload so activities opened after the initial read can still initialize.
    @Nullable
    private String mLastAboutPayload = null;

    private void broadcastOtaMessage(final String msg) {
        final Intent intent = new Intent(ACTION_OTA_MESSAGE);
        intent.putExtra(EXTRA_OTA_MESSAGE, msg);
        LocalBroadcastManager.getInstance(context).sendBroadcast(intent);
    }

    private void broadcastAboutMessage(final String msg) {
        final Intent intent = new Intent(ACTION_ABOUT_MESSAGE);
        intent.putExtra(EXTRA_ABOUT_MESSAGE, msg);
        LocalBroadcastManager.getInstance(context).sendBroadcast(intent);
    }

    private void broadcastSettingsMessage(final String msg) {
        final Intent intent = new Intent(ACTION_SETTINGS_MESSAGE);
        intent.putExtra(EXTRA_SETTINGS_MESSAGE, msg);
        LocalBroadcastManager.getInstance(context).sendBroadcast(intent);
    }

    private void broadcastPasswordMessage(final String msg) {
        final Intent intent = new Intent(ACTION_PASSWORD_MESSAGE);
        intent.putExtra(EXTRA_PASSWORD_MESSAGE, msg);
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
                mDeviceHasPassword = false; // Reset for new connection
                broadcastUpdate(ACTION_GATT_CONNECTED);
                // Request a larger MTU
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
                    Log.w(TAG, "OTA characteristic not found, probably old firmware");
                }

                movementWriteCharacteristic =
                        mCustomService.getCharacteristic(UUID.fromString(MOVEMENT_CHARACTERISTIC_UUID));
                if (movementWriteCharacteristic == null) {
                    Log.w(TAG, "no 'movement' characteristic");
                }

                aboutCharacteristic =
                        mCustomService.getCharacteristic(UUID.fromString(ABOUT_CHARACTERISTIC_UUID));
                if (aboutCharacteristic == null) {
                    Log.w(TAG, "no 'about' characteristic");
                }

                settingsCharacteristic =
                        mCustomService.getCharacteristic(UUID.fromString(SETTINGS_CHARACTERISTIC_UUID));
                if (settingsCharacteristic == null) {
                    Log.w(TAG, "no 'settings' characteristic");
                }

                passwordCharacteristic =
                        mCustomService.getCharacteristic(UUID.fromString(PASSWORD_CHARACTERISTIC_UUID));
                if (passwordCharacteristic == null) {
                    Log.w(TAG, "no 'password' characteristic");
                }
            } else {
                Log.w(TAG, "onServicesDiscovered received: " + status);
            }
        }

        @SuppressLint("MissingPermission")
        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            Log.i(TAG, "onDescriptorWrite status=" + status + " uuid=" + descriptor.getUuid());

            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.w(TAG, "Descriptor write failed: " + status);
                return;
            }

            // Now it's safe to start the next GATT operation
            if (mCustomService != null) {
                if (aboutCharacteristic != null) {
                    boolean started = mBluetoothGatt.readCharacteristic(aboutCharacteristic);
                    Log.i(TAG, "readCharacteristic(about) started=" + started);
                } else if (passwordCharacteristic != null) {
                    boolean started = mBluetoothGatt.readCharacteristic(passwordCharacteristic);
                    Log.i(TAG, "readCharacteristic(password) started=" + started);
                }
            }
        }

        @SuppressLint("MissingPermission")
        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic,
                                         int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                UUID charUuid = characteristic.getUuid();
                if (charUuid.equals(UUID.fromString(ABOUT_CHARACTERISTIC_UUID))) {
                    byte[] raw = characteristic.getValue();
                    String msg = raw == null ? null : new String(raw, StandardCharsets.UTF_8);
                    Log.i(TAG, "Got config payload: " + msg);
                    SharedPreferences sharedPref = context.getSharedPreferences(
                            context.getString(R.string.preference_file_key), Context.MODE_PRIVATE);
                    SharedPreferences.Editor editor = sharedPref.edit();
                    editor.putString(context.getString(R.string.saved_version_key), msg);
                    editor.apply();

                    if (msg != null) {
                        mLastAboutPayload = msg;
                        broadcastAboutMessage(msg);
                    }

                    // Chain to read password status
                    if (passwordCharacteristic != null) {
                        mBluetoothGatt.readCharacteristic(passwordCharacteristic);
                    }
                } else if (charUuid.equals(UUID.fromString(PASSWORD_CHARACTERISTIC_UUID))) {
                    byte[] raw = characteristic.getValue();
                    String msg = raw == null ? null : new String(raw, StandardCharsets.UTF_8);
                    Log.i(TAG, "Got password response: " + msg);
                    if (msg != null) {
                        String response = msg.trim().toLowerCase();
                        if ("false".equals(response)) {
                            mDeviceHasPassword = true;
                        }
                        broadcastPasswordMessage(msg);
                    }
                } else if (charUuid.equals(UUID.fromString(SETTINGS_CHARACTERISTIC_UUID))) {
                    byte[] raw = characteristic.getValue();
                    String msg = raw == null ? null : new String(raw, StandardCharsets.UTF_8);
                    if (msg != null) {
                        broadcastSettingsMessage(msg);
                    }
                }
            }
        }
        
        @SuppressLint("MissingPermission")
        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            UUID charUuid = characteristic.getUuid();
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (charUuid.equals(UUID.fromString(PASSWORD_CHARACTERISTIC_UUID))) {
                    Log.i(TAG, "Password written successfully, reading back status");
                    mBluetoothGatt.readCharacteristic(characteristic);
                } else if (charUuid.equals(UUID.fromString(ABOUT_CHARACTERISTIC_UUID))) {
                    Log.i(TAG, "About (device name) written successfully, reading back");
                    mBluetoothGatt.readCharacteristic(characteristic);
                }
            } else {
                Log.w(TAG, "Characteristic write failed: " + status + " for " + charUuid);
                if (charUuid.equals(UUID.fromString(PASSWORD_CHARACTERISTIC_UUID))) {
                    broadcastPasswordMessage("false"); // Signal failure to re-enable UI
                }
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic) {
            if (characteristic.getUuid().equals(UUID.fromString(OTA_CHARACTERISTIC_UUID))) {
                String msg = characteristic.getStringValue(0);
                if (msg != null) {
                    Log.i(TAG, "OTA notify: " + msg);
                    broadcastOtaMessage(msg);
                }
            }
        }
    };

    @Nullable
    public String getLastAboutPayload() {
        return mLastAboutPayload;
    }

    public boolean deviceHasPassword() {
        return mDeviceHasPassword;
    }

    public void setDeviceHasPassword(boolean hasPassword) {
        this.mDeviceHasPassword = hasPassword;
    }

    @SuppressLint("MissingPermission")
    private void enableNotifications(BluetoothGattCharacteristic characteristic) {
        if (mBluetoothGatt == null) return;

        boolean ok = mBluetoothGatt.setCharacteristicNotification(characteristic, true);
        if (!ok) {
            Log.w(TAG, "setCharacteristicNotification failed");
            return;
        }

        UUID cccdUuid = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
        BluetoothGattDescriptor cccd = characteristic.getDescriptor(cccdUuid);
        if (cccd == null) {
            Log.w(TAG, "CCCD descriptor not found");
            return;
        }

        cccd.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        mBluetoothGatt.writeDescriptor(cccd);
    }

    @SuppressLint("MissingPermission")
    public boolean writeCustomCharacteristic(byte[] value) {
        if (mBluetoothGatt == null || movementWriteCharacteristic == null) return false;
        movementWriteCharacteristic.setValue(value);
        return mBluetoothGatt.writeCharacteristic(movementWriteCharacteristic);
    }

    @SuppressLint("MissingPermission")
    public boolean writeConfigCharacteristic(byte[] value) {
        if (mBluetoothGatt == null || aboutCharacteristic == null) return false;
        aboutCharacteristic.setValue(value);
        return mBluetoothGatt.writeCharacteristic(aboutCharacteristic);
    }

    @SuppressLint("MissingPermission")
    public boolean readSettingsCharacteristic() {
        if (mBluetoothGatt == null || settingsCharacteristic == null) return false;
        return mBluetoothGatt.readCharacteristic(settingsCharacteristic);
    }

    @SuppressLint("MissingPermission")
    public boolean readAboutCharacteristic() {
        if (mBluetoothGatt == null || aboutCharacteristic == null) return false;
        return mBluetoothGatt.readCharacteristic(aboutCharacteristic);
    }

    @SuppressLint("MissingPermission")
    public void writeOtaCharacteristic(byte[] value) {
        if (mBluetoothGatt == null || mOtaCharacteristic == null) return;
        mOtaCharacteristic.setValue(value);
        mBluetoothGatt.writeCharacteristic(mOtaCharacteristic);
    }

    @SuppressLint("MissingPermission")
    public boolean readPasswordCharacteristic() {
        if (mBluetoothGatt == null || passwordCharacteristic == null) return false;
        return mBluetoothGatt.readCharacteristic(passwordCharacteristic);
    }

    @SuppressLint("MissingPermission")
    public boolean writePasswordCharacteristic(byte[] value) {
        if (mBluetoothGatt == null || passwordCharacteristic == null) {
            Log.w(TAG, "writePasswordCharacteristic: GATT or characteristic null");
            return false;
        }
        Log.i(TAG, "writePasswordCharacteristic: initiating write");
        passwordCharacteristic.setValue(value);
        passwordCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
        boolean success = mBluetoothGatt.writeCharacteristic(passwordCharacteristic);
        if (!success) {
            Log.w(TAG, "writePasswordCharacteristic: mBluetoothGatt.writeCharacteristic returned false");
        }
        return success;
    }

    public boolean connect(Context context, final String address) {
        this.context = context;
        close();
        if (mBluetoothAdapter == null || address == null) return false;
        final BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);
        if (device == null) return false;
        mBluetoothGatt = device.connectGatt(context, false, mGattCallback);
        mBluetoothDeviceAddress = address;
        mConnectionState = STATE_CONNECTING;
        return true;
    }

    @SuppressLint("MissingPermission")
    public void close() {
        if (mBluetoothGatt == null) return;
        mBluetoothGatt.disconnect();
        mBluetoothGatt.close();
        mBluetoothGatt = null;
    }

    public boolean isConnected() {
        return mConnectionState == STATE_CONNECTED;
    }
}
