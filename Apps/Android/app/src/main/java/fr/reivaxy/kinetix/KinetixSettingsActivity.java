package fr.reivaxy.kinetix;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
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
            "b_1",
            "b_2",
            "b_3",
            "b_4"
    };

    private static final String[] INT_FIELDS = new String[]{
            "i_1",
            "i_2",
            "i_3",
            "i_4"
    };
    private static final String[] STR_FIELDS = new String[]{
            "s_1",
            "s_2",
            "s_3",
            "s_4"
    };

    private final Map<Integer, String> viewIdToField = new HashMap<>();

    // Prevents BLE writes when we are programmatically applying values received from BLE.
    private boolean suppressWrites = false;


    // Debounce BLE writes from EditText fields: send only after the user pauses typing.
    private static final long TEXT_UPDATE_DEBOUNCE_MS = 350L;
    private final Handler debounceHandler = new Handler(Looper.getMainLooper());
    private final Map<Integer, Runnable> pendingTextUpdates = new HashMap<>();


    private final BroadcastReceiver settingsReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null) return;
            String action = intent.getAction();
            if (BluetoothHandler.ACTION_GATT_CONNECTED.equals(action)) {
                // If the user opens this screen before the device connects, we still want an init read.
                BluetoothHandler.getInstance().readSettingsCharacteristic();
                return;
            }
            if (BluetoothHandler.ACTION_SETTINGS_MESSAGE.equals(action)) {
                String payload = intent.getStringExtra(BluetoothHandler.EXTRA_SETTINGS_MESSAGE);
                if (payload == null) return;
                try {
                    applySettingsPayload(payload);
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
        bindCheckbox(R.id.ks_b_1, BOOL_FIELDS[0]);
        bindCheckbox(R.id.ks_b_2, BOOL_FIELDS[1]);
        bindCheckbox(R.id.ks_b_3, BOOL_FIELDS[2]);
        bindCheckbox(R.id.ks_b_4, BOOL_FIELDS[3]);

        // --- Integers ---
        bindTxtField(R.id.ks_i_1, INT_FIELDS[0], true);
        bindTxtField(R.id.ks_i_2, INT_FIELDS[1], true);
        bindTxtField(R.id.ks_i_3, INT_FIELDS[2], true);
        bindTxtField(R.id.ks_i_4, INT_FIELDS[3], true);

        // --- Strings ---
        bindTxtField(R.id.ks_s_1, STR_FIELDS[0], false);
        bindTxtField(R.id.ks_s_2, STR_FIELDS[1], false);
        bindTxtField(R.id.ks_s_3, STR_FIELDS[2], false);
        bindTxtField(R.id.ks_s_4, STR_FIELDS[3], false);
    }

    @Override
    protected void onStart() {
        super.onStart();
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothHandler.ACTION_SETTINGS_MESSAGE);
        filter.addAction(BluetoothHandler.ACTION_GATT_CONNECTED);
        LocalBroadcastManager.getInstance(this).registerReceiver(settingsReceiver, filter);

        // Trigger a read so the screen initializes (or refreshes) with the device's current config.
        BluetoothHandler.getInstance().readSettingsCharacteristic();

    }

    @Override
    protected void onStop() {
        LocalBroadcastManager.getInstance(this).unregisterReceiver(settingsReceiver);

        // Prevent delayed updates from firing after the activity is no longer visible.
        debounceHandler.removeCallbacksAndMessages(null);
        pendingTextUpdates.clear();

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

    private void bindTxtField(int viewId, String fieldName, boolean isInt) {
        viewIdToField.put(viewId, fieldName);
        EditText et = findViewById(viewId);

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

                final String candidateRaw = s.toString().trim();

                // Debounce: only send once the user pauses typing for a short moment.
                Runnable prev = pendingTextUpdates.get(viewId);
                if (prev != null) {
                    debounceHandler.removeCallbacks(prev);
                }

                Runnable r = new Runnable() {
                    @Override
                    public void run() {
                        String txt = candidateRaw;
                        if (isInt) {
                            if (txt.isEmpty()) {
                                txt = "0";
                            }
                            try {
                                Integer.parseInt(txt);
                            } catch (NumberFormatException e) {
                                Log.w(TAG, "Invalid integer for " + fieldName + ": '" + txt + "'");
                                return;
                            }
                        }

                        if (txt.equals(lastSent)) return;
                        lastSent = txt;
                        sendConfigUpdate(fieldName, txt);
                    }
                };

                pendingTextUpdates.put(viewId, r);
                debounceHandler.postDelayed(r, TEXT_UPDATE_DEBOUNCE_MS);
            }
        });
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
    private void applySettingsPayload(String payload) throws JSONException {
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
                if (fieldName.startsWith("b_")) {
                    CheckBox cb = findViewById(getResources().getIdentifier(fieldId, "id", getPackageName()));
                    boolean b = json.getBoolean(fieldName);
                    if (cb != null) cb.setChecked(b);
                } else {
                    EditText et = findViewById(getResources().getIdentifier(fieldId, "id", getPackageName()));
                    if (et != null) {
                        String value;
                        if (fieldName.startsWith("i_")) {
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
