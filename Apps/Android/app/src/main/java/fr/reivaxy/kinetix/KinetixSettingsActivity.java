package fr.reivaxy.kinetix;

import android.os.Bundle;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.widget.CheckBox;
import android.widget.EditText;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * KinetiX device settings screen.
 *
 * Displays 5 boolean values (checkboxes) and 5 integer values.
 * Each time a value is changed, we push "<field>=<value>" to the config characteristic.
 */
public class KinetixSettingsActivity extends AppCompatActivity {

    private static final String TAG = KinetixSettingsActivity.class.getSimpleName();

    // Field names sent over BLE. Keep them stable: firmware side should parse these.
    private static final String[] BOOL_FIELDS = new String[]{
            "bool_1",
            "bool_2",
            "bool_3"
    };

    private static final String[] INT_FIELDS = new String[]{
            "int_1",
            "int_2",
            "int_3"
    };
    private static final String[] STR_FIELDS = new String[]{
            "str_1",
            "str_2",
            "str_3"
    };

    private final Map<Integer, String> viewIdToField = new HashMap<>();

    // Prevents BLE writes when we are programmatically applying values received from BLE.
    private boolean suppressWrites = false;

    // Accept "key=value" pairs separated by new lines, ';', ','... etc.
    private static final Pattern KV_PATTERN = Pattern.compile(
            "([A-Za-z0-9_]+)\\s*=\\s*([^;\\n\\r,\\s]+)"
    );

    private final BroadcastReceiver configReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null) return;
            String action = intent.getAction();
            if (BluetoothHandler.ACTION_GATT_CONNECTED.equals(action)) {
                // If the user opens this screen before the device connects, we still want an init read.
                BluetoothHandler.getInstance().readConfigCharacteristic();
                return;
            }
            if (BluetoothHandler.ACTION_CONFIG_MESSAGE.equals(action)) {
                String payload = intent.getStringExtra(BluetoothHandler.EXTRA_CONFIG_MESSAGE);
                if (payload == null) return;
                try {
                    applyConfigPayload(payload);
                } catch(Exception e) {
                    Log.i(TAG, "Failed parsing config payload", e);
                }
            }
        }
    };

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_kinetix_settings);

        setTitle(R.string.title_activity_kinetix_settings);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        }

        // --- Checkboxes ---
        bindCheckbox(R.id.ks_bool_1, BOOL_FIELDS[0]);
        bindCheckbox(R.id.ks_bool_2, BOOL_FIELDS[1]);
        bindCheckbox(R.id.ks_bool_3, BOOL_FIELDS[2]);

        // --- Integers ---
        bindIntField(R.id.ks_int_1, INT_FIELDS[0]);
        bindIntField(R.id.ks_int_2, INT_FIELDS[1]);
        bindIntField(R.id.ks_int_3, INT_FIELDS[2]);

        // --- Strings ---
        bindIntField(R.id.ks_str_1, STR_FIELDS[0]);
        bindIntField(R.id.ks_str_2, STR_FIELDS[1]);
        bindIntField(R.id.ks_str_3, STR_FIELDS[2]);
    }

    @Override
    protected void onStart() {
        super.onStart();
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothHandler.ACTION_CONFIG_MESSAGE);
        filter.addAction(BluetoothHandler.ACTION_GATT_CONNECTED);
        LocalBroadcastManager.getInstance(this).registerReceiver(configReceiver, filter);

        // If we already read the config earlier (e.g. during connect), apply it immediately.
        // should we do that ? TODO
//        String cached = BluetoothHandler.getInstance().getLastConfigPayload();
//        if (cached != null) {
//            applyConfigPayload(cached);
//        }

        // Trigger a read so the screen initializes (or refreshes) with the device's current config.
        BluetoothHandler.getInstance().readConfigCharacteristic();
        // Read firmware version
        BluetoothHandler.getInstance().readSystemCharacteristic();
    }

    @Override
    protected void onStop() {
        LocalBroadcastManager.getInstance(this).unregisterReceiver(configReceiver);
        super.onStop();
    }

    @Override
    public boolean onSupportNavigateUp() {
        finish();
        return true;
    }

    private void bindCheckbox(int viewId, String fieldName) {
        viewIdToField.put(viewId, fieldName);
        CheckBox cb = findViewById(viewId);
        cb.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (suppressWrites) return;
            sendConfigUpdate(fieldName, isChecked ? "1" : "0");
        });
    }

    private void bindIntField(int viewId, String fieldName) {
        viewIdToField.put(viewId, fieldName);
        EditText et = findViewById(viewId);

        // Send update when user leaves the field.
        et.setOnFocusChangeListener((v, hasFocus) -> {
            if (!hasFocus) {
                sendIntIfValid((EditText) v, fieldName);
            }
        });

        // Also send update on explicit "Done"/enter.
        et.setOnEditorActionListener((v, actionId, event) -> {
            sendIntIfValid((EditText) v, fieldName);
            return false;
        });

        // Track changes so we don't spam identical writes when focus toggles.
        et.addTextChangedListener(new TextWatcher() {
            private String lastSent = null;

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) { }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) { }

            @Override
            public void afterTextChanged(Editable s) {
                // Only send live changes when field is focused (user actively editing).
                if (!et.hasFocus()) return;
                if (suppressWrites) return;

                String txt = s.toString().trim();
                if (txt.isEmpty()) return;
                try {
                    Integer.parseInt(txt);
                } catch (NumberFormatException e) {
                    return;
                }

                if (txt.equals(lastSent)) return;
                lastSent = txt;
                sendConfigUpdate(fieldName, txt);
            }
        });
    }

    private void sendIntIfValid(EditText et, String fieldName) {
        if (suppressWrites) return;
        String txt = et.getText().toString().trim();
        if (txt.isEmpty()) return;
        try {
            Integer.parseInt(txt);
        } catch (NumberFormatException e) {
            Log.w(TAG, "Invalid integer for " + fieldName + ": '" + txt + "'");
            return;
        }
        sendConfigUpdate(fieldName, txt);
    }

    private void sendConfigUpdate(String fieldName, String value) {
        BluetoothHandler ble = BluetoothHandler.getInstance();
        if (!ble.isConnected()) {
            Log.w(TAG, "Not connected; ignoring config update " + fieldName + "=" + value);
            return;
        }
        ble.writeConfigCharacteristic((fieldName + "=" + value).getBytes(StandardCharsets.UTF_8));
    }

    /**
     * Parses a CONFIG characteristic payload and populates UI fields.
     * Expected format Json. Each field must be the id of a ui element, from which we find the type
     */
    private void applyConfigPayload(String payload) throws JSONException {
        suppressWrites = true;
        try {
            // the payload is a json string we need to deserialize
            JSONObject json = new JSONObject(payload);
            JSONArray fields = json.names();
            if (fields == null) {
                return;
            }

            // Browse the fields
            for (int i = 0; i < fields.length(); i++) {
                String fieldName = fields.getString(i);
                Log.i(TAG, "applyConfigPayload: " + fieldName);
                String fieldId = "ks_" + fieldName;
                if (fieldName.startsWith("bool_")) {
                    CheckBox cb = findViewById(getResources().getIdentifier(fieldId, "id", getPackageName()));
                    boolean b = json.getBoolean(fieldName);
                    if (cb != null) cb.setChecked(b);
                } else {
                    EditText et = findViewById(getResources().getIdentifier(fieldId, "id", getPackageName()));
                    if (et != null) {
                        String value;
                        if (fieldName.startsWith("int_")) {
                            value = String.valueOf(json.getInt(fieldName));
                        } else {
                            value = json.getString(fieldName);
                        }
                        et.setText(value);
                    }
                }
            }
        } finally {
            suppressWrites = false;
        }

    }
}
