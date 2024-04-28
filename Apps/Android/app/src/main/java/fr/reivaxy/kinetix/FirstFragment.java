package fr.reivaxy.kinetix;

import static android.bluetooth.BluetoothAdapter.*;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.fragment.app.Fragment;
import androidx.navigation.fragment.NavHostFragment;

import com.google.android.material.snackbar.Snackbar;

import java.nio.charset.StandardCharsets;
import java.util.UUID;

import fr.reivaxy.kinetix.databinding.FragmentFirstBinding;

public class FirstFragment extends Fragment {
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
    private FragmentFirstBinding binding;
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothGatt mBluetoothGatt;
    private final static String TAG = FirstFragment.class.getSimpleName();

    private BluetoothManager mBluetoothManager;
    private String mBluetoothDeviceAddress;
    private int mConnectionState = STATE_DISCONNECTED;

    // Implements callback methods for GATT events that the app cares about.  For example,
    // connection change and services discovered.
    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            String intentAction;
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                intentAction = ACTION_GATT_CONNECTED;
                mConnectionState = STATE_CONNECTED;
//                broadcastUpdate(intentAction);
                Log.i(TAG, "Connected to GATT server.");
                // Attempts to discover services after successful connection.
                Log.i(TAG, "Attempting to start service discovery:" +
                        mBluetoothGatt.discoverServices());
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                intentAction = ACTION_GATT_DISCONNECTED;
                mConnectionState = STATE_DISCONNECTED;
                Log.i(TAG, "Disconnected from GATT server.");
//                broadcastUpdate(intentAction);
            }
        }
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
//                broadcastUpdate(ACTION_GATT_SERVICES_DISCOVERED);
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
            }
        }
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic) {
//            broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
        }
    };

    @Override
    public View onCreateView(
            @NonNull LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState
    ) {

        binding = FragmentFirstBinding.inflate(inflater, container, false);
        return binding.getRoot();

    }

    public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();


        binding.buttonFist.setOnClickListener(v -> {
                    writeCustomCharacteristic("fist".getBytes(StandardCharsets.UTF_8), R.id.button_open);
                }
        );
        binding.buttonOpen.setOnClickListener(v -> {
                    writeCustomCharacteristic("five".getBytes(StandardCharsets.UTF_8), R.id.button_open);
                }
        );
        binding.buttonPinch.setOnClickListener(v -> {
                    writeCustomCharacteristic("openPinch".getBytes(StandardCharsets.UTF_8), R.id.button_pinch);
                }
        );

        binding.buttonOne.setOnClickListener(v -> {
                    writeCustomCharacteristic("one".getBytes(StandardCharsets.UTF_8), R.id.button_one);
                }
        );

        binding.buttonTwo.setOnClickListener(v -> {
                    writeCustomCharacteristic("two".getBytes(StandardCharsets.UTF_8), R.id.button_two);
                }
        );

        binding.buttonThree.setOnClickListener(v -> {
                    writeCustomCharacteristic("three".getBytes(StandardCharsets.UTF_8), R.id.button_three);
                }
        );

        binding.buttonFour.setOnClickListener(v -> {
                    writeCustomCharacteristic("four".getBytes(StandardCharsets.UTF_8), R.id.button_four);
                }
        );

        binding.buttonFive.setOnClickListener(v -> {
                    writeCustomCharacteristic("five".getBytes(StandardCharsets.UTF_8), R.id.button_five);
                }
        );

        binding.buttonOk.setOnClickListener(v -> {
                    writeCustomCharacteristic("ok".getBytes(StandardCharsets.UTF_8), R.id.button_ok);
                }
        );

        binding.buttonFu.setOnClickListener(v -> {
                    writeCustomCharacteristic("fu".getBytes(StandardCharsets.UTF_8), R.id.button_fu);
                }
        );
        binding.buttonConnect.setOnClickListener(v -> {
                    boolean connected = connect("74:4D:BD:99:3F:95");
                    if (!connected) {
                        Snackbar.make(view, "Connection failed.", Snackbar.LENGTH_LONG)
                        .setAnchorView(R.id.button_five)
                        .setAction("Action", null).show();
                    }
                }
        );
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }

    public void writeCustomCharacteristic(byte[] value, int id) {
        if (ActivityCompat.checkSelfPermission(this.getContext(), android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            // TODO: Consider calling
            //    ActivityCompat#requestPermissions
            // here to request the missing permissions, and then overriding
            //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
            //                                          int[] grantResults)
            // to handle the case where the user grants the permission. See the documentation
            // for ActivityCompat#requestPermissions for more details.

            return;
        }
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        /*check if the service is available on the device*/
        BluetoothGattService mCustomService = mBluetoothGatt.getService(UUID.fromString("89d60870-9908-4472-8f8c-e5b3e6573cd1"));
        if (mCustomService == null) {
            Log.w(TAG, "Custom BLE Service not found");
            return;
        }
        /*get the read characteristic from the service*/
        BluetoothGattCharacteristic mWriteCharacteristic = mCustomService.getCharacteristic(UUID.fromString("39dea685-a63e-44b2-8819-9a202581f8fe"));
        mWriteCharacteristic.setValue(value);
        if (!mBluetoothGatt.writeCharacteristic(mWriteCharacteristic)) {
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
    public boolean connect(final String address) {
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
        if (ActivityCompat.checkSelfPermission(this.getContext(), android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            Log.w(TAG, "Bluetooth connection permission not granted");
            Snackbar.make(this.getView(), "Bluetooth connection permission not granted", Snackbar.LENGTH_LONG)
                    .setAnchorView(this.getId())
                    .setAction("Action", null).show();
            requestPermissions(
                    new String[]{android.Manifest.permission.BLUETOOTH_CONNECT},
                    42);
            return false;
        }
        mBluetoothGatt = device.connectGatt(this.getContext(), false, mGattCallback);
        Log.d(TAG, "Trying to create a new connection.");
        mBluetoothDeviceAddress = address;
        mConnectionState = STATE_CONNECTING;
        return true;

    }
}