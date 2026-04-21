package fr.reivaxy.kinetix;

import android.Manifest;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.core.app.ActivityCompat;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.navigation.ui.AppBarConfiguration;
import androidx.navigation.ui.NavigationUI;
import fr.reivaxy.kinetix.databinding.ActivityMainBinding;
import android.view.Menu;
import android.view.MenuItem;

import org.json.JSONObject;

import java.nio.charset.StandardCharsets;


public class MainActivity extends AppCompatActivity {

    private AppBarConfiguration appBarConfiguration;
    private ActivityMainBinding binding;
    private final static String TAG = MainActivity.class.getSimpleName();
    private AlertDialog mCurrentPasswordDialog;
    private String mDeviceName = null;


    @RequiresApi(api = Build.VERSION_CODES.UPSIDE_DOWN_CAKE)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        setSupportActionBar(binding.toolbar);

        NavController navController = Navigation.findNavController(this, R.id.nav_host_fragment_content_main);
        appBarConfiguration = new AppBarConfiguration.Builder(navController.getGraph()).build();
        NavigationUI.setupActionBarWithNavController(this, navController, appBarConfiguration);

        // Listen for destination changes to re-apply the device name to the title
        navController.addOnDestinationChangedListener((controller, destination, arguments) -> {
            updateTitle();
        });

        if (ActivityCompat.checkSelfPermission(binding.getRoot().getContext(), android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            Log.w(TAG, "Bluetooth connection permission not granted");
            ActivityCompat.requestPermissions(this,
                    new String[]{android.Manifest.permission.BLUETOOTH_CONNECT},
                    42);
        }
        if (ActivityCompat.checkSelfPermission(binding.getRoot().getContext(), android.Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
            Log.w(TAG, "Bluetooth scan permission not granted");
            ActivityCompat.requestPermissions(this,
                    new String[]{android.Manifest.permission.BLUETOOTH_SCAN},
                    42);
        }
        if (ActivityCompat.checkSelfPermission(binding.getRoot().getContext(), Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            Log.w(TAG, "Record audio permission not granted");
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.RECORD_AUDIO},
                    42);
        }
        if (ActivityCompat.checkSelfPermission(binding.getRoot().getContext(), android.Manifest.permission.FOREGROUND_SERVICE_MICROPHONE) != PackageManager.PERMISSION_GRANTED) {
            Log.w(TAG, "Foreground service microphone permission not granted");
            ActivityCompat.requestPermissions(this,
                    new String[]{android.Manifest.permission.FOREGROUND_SERVICE_MICROPHONE},
                    42);
        }

    }

    @Override
    protected void onStart() {
        super.onStart();
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothHandler.ACTION_PASSWORD_MESSAGE);
        filter.addAction(BluetoothHandler.ACTION_ABOUT_MESSAGE);
        filter.addAction(BluetoothHandler.ACTION_GATT_DISCONNECTED);
        LocalBroadcastManager.getInstance(this).registerReceiver(mReceiver, filter);

        // Try to recover device name if it was already read
        String lastAbout = BluetoothHandler.getInstance().getLastAboutPayload();
        if (lastAbout != null) {
            parseDeviceName(lastAbout);
            updateTitle();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        LocalBroadcastManager.getInstance(this).unregisterReceiver(mReceiver);
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null) return;
            String action = intent.getAction();
            if (BluetoothHandler.ACTION_PASSWORD_MESSAGE.equals(action)) {
                String response = intent.getStringExtra(BluetoothHandler.EXTRA_PASSWORD_MESSAGE);
                if (response == null) return;
                response = response.trim().toLowerCase();

                if ("true".equals(response)) {
                    if (mCurrentPasswordDialog != null && mCurrentPasswordDialog.isShowing()) {
                        mCurrentPasswordDialog.dismiss();
                    }
                    Toast.makeText(MainActivity.this, "Success", Toast.LENGTH_SHORT).show();
                    BluetoothHandler.getInstance().setDeviceHasPassword(false);
                } else {
                    if (mCurrentPasswordDialog != null && mCurrentPasswordDialog.isShowing()) {
                        mCurrentPasswordDialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(true);
                        TextView errorText = mCurrentPasswordDialog.findViewById(R.id.passwordError);
                        if (errorText != null) {
                            errorText.setText(R.string.password_error);
                            errorText.setVisibility(View.VISIBLE);
                        }
                    }
                }
            } else if (BluetoothHandler.ACTION_ABOUT_MESSAGE.equals(action)) {
                String payload = intent.getStringExtra(BluetoothHandler.EXTRA_ABOUT_MESSAGE);
                Log.d(TAG, "ACTION_ABOUT_MESSAGE received: " + payload);
                parseDeviceName(payload);
                updateTitle();
            } else if (BluetoothHandler.ACTION_GATT_DISCONNECTED.equals(action)) {
                mDeviceName = null;
                updateTitle();
            }
        }
    };

    private void parseDeviceName(String payload) {
        try {
            if (payload != null && payload.startsWith("{")) {
                JSONObject json = new JSONObject(payload);
                mDeviceName = json.optString("deviceName", null);
                if (mDeviceName != null && mDeviceName.isEmpty()) mDeviceName = null;
                Log.d(TAG, "Parsed deviceName: " + mDeviceName);
            }
        } catch (Exception e) {
            Log.e(TAG, "Error parsing about payload for title", e);
        }
    }

    private void updateTitle() {
        ActionBar actionBar = getSupportActionBar();
        if (actionBar == null) return;

        NavController navController = Navigation.findNavController(this, R.id.nav_host_fragment_content_main);
        if (navController.getCurrentDestination() == null) return;
        
        CharSequence label = navController.getCurrentDestination().getLabel();
        String baseTitle = label != null ? label.toString() : getString(R.string.app_name);

        if (mDeviceName != null) {
            actionBar.setTitle(baseTitle + " - " + mDeviceName);
        } else {
            actionBar.setTitle(baseTitle);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        if (id == R.id.action_settings) {
            Log.i(TAG, "cliqued Settings");
            Intent i = new Intent(MainActivity.this, SettingsActivity.class);
            startActivity(i);
            return true;
        }

        if (id == R.id.action_settings_kinetix) {
            Log.i(TAG, "clicked Settings KinetiX");
            Intent i = new Intent(MainActivity.this, KinetixSettingsActivity.class);
            startActivity(i);
            return true;
        }

        if (id == R.id.action_about) {
            Log.i(TAG, "cliqued About");
            Intent i = new Intent(MainActivity.this, AboutActivity.class);
            startActivity(i);
            return true;
        }

        if (id == R.id.action_password) {
            Log.i(TAG, "clicked Password");
            showPasswordDialog();
            return true;
        }

        if (id == R.id.action_tools) {
            Log.i(TAG, "cliqued Tools");
            Intent i = new Intent(MainActivity.this, ToolsActivity.class);
            startActivity(i);
            return true;
        }

        if (id == R.id.action_devices) {
            Log.i(TAG, "cliqued Devices");
            Intent i = new Intent(MainActivity.this, DeviceListActivity.class);
            startActivity(i);
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private void showPasswordDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        LayoutInflater inflater = getLayoutInflater();
        View dialogView = inflater.inflate(R.layout.dialog_password, null);
        builder.setView(dialogView);

        TextView titleView = new TextView(this);
        titleView.setPadding(40, 40, 40, 40);
        titleView.setTextSize(20);
        titleView.setTextColor(getResources().getColor(android.R.color.black));
        titleView.setText(R.string.password_settings);
        builder.setCustomTitle(titleView);

        EditText txtPassword = dialogView.findViewById(R.id.txtPassword);
        EditText txtNewPassword = dialogView.findViewById(R.id.txtNewPassword);
        CheckBox chkResetPassword = dialogView.findViewById(R.id.chkResetPassword);
        View newPasswordSection = dialogView.findViewById(R.id.newPasswordSection);
        TextView labelCurrentPassword = dialogView.findViewById(R.id.labelCurrentPassword);

        labelCurrentPassword.setText(R.string.current_password);
        newPasswordSection.setVisibility(View.VISIBLE);

        builder.setPositiveButton(R.string.send, null);
        builder.setNegativeButton(R.string.cancel, (dialog, which) -> dialog.dismiss());

        AlertDialog dialog = builder.create();
        dialog.show();

        dialog.getButton(AlertDialog.BUTTON_POSITIVE).setOnClickListener(v -> {
            String currentPwd = txtPassword.getText().toString().trim();

            if (currentPwd.isEmpty() && BluetoothHandler.getInstance().deviceHasPassword()) {
                txtPassword.setError(getString(R.string.error_password_required));
                return;
            }

            if (!BluetoothHandler.getInstance().isConnected()) {
                Toast.makeText(this, R.string.notConnected, Toast.LENGTH_SHORT).show();
                return;
            }

            try {
                JSONObject payload = new JSONObject();
                payload.put("pwdCheck", currentPwd);
                payload.put("newPwd", txtNewPassword.getText().toString().trim());
                payload.put("resetPwd", chkResetPassword.isChecked());

                Log.d(TAG, "Sending password payload: " + payload.toString());
                boolean started = BluetoothHandler.getInstance().writePasswordCharacteristic(payload.toString().getBytes(StandardCharsets.UTF_8));
                if (!started) {
                    Toast.makeText(this, "BLE write failed to start", Toast.LENGTH_SHORT).show();
                } else {
                    v.setEnabled(false);
                }
            } catch (Exception e) {
                Log.e(TAG, "Error creating password JSON", e);
                Toast.makeText(this, "Error: " + e.getMessage(), Toast.LENGTH_SHORT).show();
            }
        });

        mCurrentPasswordDialog = dialog;
    }

    @Override
    public boolean onSupportNavigateUp() {
        NavController navController = Navigation.findNavController(this, R.id.nav_host_fragment_content_main);
        return NavigationUI.navigateUp(navController, appBarConfiguration)
                || super.onSupportNavigateUp();
    }


}