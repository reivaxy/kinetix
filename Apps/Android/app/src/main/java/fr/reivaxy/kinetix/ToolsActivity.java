package fr.reivaxy.kinetix;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.Uri;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.OpenableColumns;
import android.text.InputType;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.Manifest;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.Fragment;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import com.google.android.material.snackbar.Snackbar;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import android.widget.LinearLayout;
import android.widget.EditText;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class ToolsActivity extends AppCompatActivity {
    private final static String TAG = ToolsActivity.class.getSimpleName();

    private Button otaButton;
    private ProgressBar otaProgress;
    private TextView otaStatus;

    private final ExecutorService netExecutor = Executors.newSingleThreadExecutor();
    private final Handler mainHandler = new Handler(Looper.getMainLooper());

    private static final int REQ_WIFI_PERMS = 9001;

    private Uri pendingBinUri = null;

    // ---- Wi-Fi info for OTA ----
    private String pendingWifiSsid = null;
    private String pendingWifiPassword = "";

    private String pendingFileName = null;
    private long pendingFileSize = -1;

    private boolean awaitingOtaReady = false;
    private final Handler timeoutHandler = new Handler(Looper.getMainLooper());
    private Runnable otaReadyTimeoutRunnable = null;

    private final ActivityResultLauncher<String[]> pickBinLauncher =
            registerForActivityResult(new ActivityResultContracts.OpenDocument(), uri -> {
                if (uri == null) return;
                onBinPicked(uri);
            });

    private final BroadcastReceiver otaReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (!BluetoothHandler.ACTION_OTA_MESSAGE.equals(intent.getAction())) return;
            String msg = intent.getStringExtra(BluetoothHandler.EXTRA_OTA_MESSAGE);
            if (msg == null) return;
            handleOtaBleMessage(msg);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.tools_activity);
        if (savedInstanceState == null) {
            getSupportFragmentManager()
                    .beginTransaction()
                    .replace(R.id.tools, new ToolsFragment())
                    .commit();
        }
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }

        otaButton = findViewById(R.id.button_ota);
        otaProgress = findViewById(R.id.ota_progress);
        otaStatus = findViewById(R.id.ota_status);

        otaButton.setOnClickListener(v -> {
            if (!BluetoothHandler.getInstance().isConnected()) {
                Snackbar.make(v, getResources().getString(R.string.notConnected), Snackbar.LENGTH_LONG)
                        .setAnchorView(R.id.button_ota)
                        .setAction("Action", null)
                        .show();
                return;
            }
            // Pick the firmware .bin via SAF
            pickBinLauncher.launch(new String[]{"application/octet-stream", "application/*", "*/*"});
        });
    }

    @Override
    protected void onStart() {
        super.onStart();
        LocalBroadcastManager.getInstance(this)
                .registerReceiver(otaReceiver, new IntentFilter(BluetoothHandler.ACTION_OTA_MESSAGE));
    }

    @Override
    protected void onStop() {
        LocalBroadcastManager.getInstance(this).unregisterReceiver(otaReceiver);
        super.onStop();
    }

    public void calibration(View v) {
        try {
            AlertDialog.Builder builder = new AlertDialog.Builder(v.getContext());
            builder.setMessage(R.string.calibMessage)
                    .setTitle(R.string.calibTitle);
            // Add the buttons.
            builder.setPositiveButton(R.string.confirm_calibration, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int id) {
                    // User taps OK button.
                    Log.i(TAG, "Starting calibration");
                    sendPosition(v, "calibration", R.id.button_calib);

                }
            });
            builder.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int id) {
                    // User cancels the dialog.
                    Log.i(TAG, "Cancelling calibration");
                }
            });
            // Create the AlertDialog.
            AlertDialog dialog = builder.create();
            dialog.show();

        } catch (Exception e) {
            Log.e(TAG, "Failed: ", e);
        }
    }

    private void sendPosition(View view, String position, int buttonId) {
        if (BluetoothHandler.getInstance().isConnected()) {
            BluetoothHandler.getInstance().writeCustomCharacteristic(position.getBytes(StandardCharsets.UTF_8));
        } else {
            Log.e(TAG, "sendPosition: not connected");
            Snackbar.make(view, getResources().getString(R.string.notConnected), Snackbar.LENGTH_LONG)
                    .setAnchorView(buttonId)
                    .setAction("Action", null).show();
        }
    }

    private void onBinPicked(Uri uri) {
        pendingBinUri = uri;
        pendingFileName = queryDisplayName(uri);
        pendingFileSize = querySize(uri);

        String displayName = (pendingFileName != null) ? pendingFileName : uri.toString();
        String msg = getString(R.string.ota_confirm_message, displayName);

        
        // Try to pre-fill SSID (may be null if permission/location is missing)
        requestWifiSsidPermissionIfNeeded();
        String currentSsid = getCurrentWifiSsid();
        if (currentSsid != null) pendingWifiSsid = currentSsid;

        // Build a small form for SSID + password
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        int pad = (int) (16 * getResources().getDisplayMetrics().density);
        layout.setPadding(pad, pad, pad, pad);

        TextView fileTv = new TextView(this);
        fileTv.setText(msg);
        layout.addView(fileTv);

        final EditText ssidEt = new EditText(this);
        ssidEt.setHint(getString(R.string.ota_wifi_ssid_hint));
        ssidEt.setInputType(InputType.TYPE_CLASS_TEXT);
        if (pendingWifiSsid != null) ssidEt.setText(pendingWifiSsid);
        layout.addView(ssidEt);

        final EditText passEt = new EditText(this);
        passEt.setHint(getString(R.string.ota_wifi_password_hint));
        passEt.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
        passEt.setText(pendingWifiPassword != null ? pendingWifiPassword : "");
        layout.addView(passEt);

        AlertDialog dialog = new AlertDialog.Builder(this)
                .setTitle(R.string.ota_confirm_title)
                .setView(layout)
                .setPositiveButton(R.string.confirm_ota, null) // 👈 listener added later
                .setNegativeButton(R.string.cancel, null)
                .show();

        dialog.getButton(AlertDialog.BUTTON_POSITIVE).setOnClickListener(v -> {
            pendingWifiSsid = ssidEt.getText().toString().trim();
            pendingWifiPassword = passEt.getText().toString();

            if (pendingWifiSsid.isEmpty()) {
                ssidEt.setError(getString(R.string.error_ssid_required));
                return; // dialog stays open
            }

            if (pendingWifiPassword.isEmpty()) {
                passEt.setError(getString(R.string.error_password_required));
                return; // dialog stays open
            }

            startOtaFlow();
            dialog.dismiss(); // ✅ only dismiss on success
        });

    }

    private void startOtaFlow() {
        if (pendingBinUri == null) return;
        if (!BluetoothHandler.getInstance().isConnected()) {
            showOtaStatus(getString(R.string.notConnected));
            return;
        }

        otaProgress.setVisibility(View.VISIBLE);
        otaProgress.setProgress(0);
        otaStatus.setVisibility(View.VISIBLE);
        showOtaStatus(getString(R.string.ota_starting));

        awaitingOtaReady = true;
        scheduleOtaReadyTimeout();

        // Ask ESP32 to start its OTA HTTP server and reply OTA_READY <url>
        BluetoothHandler.getInstance().writeOtaCharacteristic(buildStartOtaCommand().getBytes(StandardCharsets.UTF_8));
    }

    private String buildStartOtaCommand() {
        // Firmware expects: START_OTA <ssid>;<password>
        String ssid = (pendingWifiSsid != null) ? pendingWifiSsid.trim() : "";
        String pass = (pendingWifiPassword != null) ? pendingWifiPassword : "";

        // Fallback: try to read SSID right now if empty
        if (ssid.isEmpty()) {
            requestWifiSsidPermissionIfNeeded();
            String current = getCurrentWifiSsid();
            if (current != null) ssid = current.trim();
        }

        return "OTA_START " + ssid + ";" + pass;
    }

    private void scheduleOtaReadyTimeout() {
        cancelOtaReadyTimeout();
        otaReadyTimeoutRunnable = () -> {
            if (awaitingOtaReady) {
                awaitingOtaReady = false;
                showOtaStatus(getString(R.string.ota_ready_timeout));
            }
        };
        timeoutHandler.postDelayed(otaReadyTimeoutRunnable, 30000);
    }

    private void cancelOtaReadyTimeout() {
        if (otaReadyTimeoutRunnable != null) {
            timeoutHandler.removeCallbacks(otaReadyTimeoutRunnable);
            otaReadyTimeoutRunnable = null;
        }
    }

    private void handleOtaBleMessage(String msg) {
        // Examples from firmware:
        //   OTA_READY http://192.168.x.y:8080/update?token=...
        //   OTA_PROGRESS 12 245760/2000000
        //   OTA_DONE

        if (msg.startsWith("OTA_READY ")) {
            awaitingOtaReady = false;
            cancelOtaReadyTimeout();
            String url = msg.substring("OTA_READY ".length()).trim();
            showOtaStatus(getString(R.string.ota_uploading));
            startUpload(url);
            return;
        }

        if (msg.startsWith("OTA_PROGRESS ")) {
            // Optional: show BLE-reported percent as status text (upload progress is primary)
            showOtaStatus(msg);
            try {
                String[] parts = msg.split("\\D+");
                int pct = Integer.parseInt(parts[1]);
                mainHandler.post(() -> otaProgress.setProgress(pct));
            } catch(Exception e) {
                // Ignore exception
                Log.w("Couldn't read progression ", e);
            }

            return;
        }

        if (msg.startsWith("OTA_DONE")) {
            showOtaStatus(getString(R.string.ota_done));
            return;
        }

        if (msg.startsWith("OTA_TIMEOUT")) {
            showOtaStatus(msg);
            return;
        }

        // Default: show raw messages
        if (msg.startsWith("[OTA]")) {
            showOtaStatus(msg);
        }
    }

    private void startUpload(String urlString) {
        final Uri uri = pendingBinUri;
        final long size = pendingFileSize;

        if (uri == null) {
            showOtaStatus(getString(R.string.ota_no_file));
            return;
        }

        netExecutor.execute(() -> {
            HttpURLConnection conn = null;
            try {
                URL url = new URL(urlString);
                Uri urlUri = Uri.parse(urlString);
                String token = urlUri.getQueryParameter("token");

                String boundary = "----KinetixBoundary" + System.currentTimeMillis();
                String crlf = "\r\n";

                conn = (HttpURLConnection) url.openConnection();
                conn.setConnectTimeout(15000);
                conn.setReadTimeout(60000);
                conn.setRequestMethod("POST");
                conn.setDoOutput(true);
                conn.setUseCaches(false);

                conn.setRequestProperty("X-OTA-Token", token);
                conn.setRequestProperty("Content-Type", "multipart/form-data; boundary=" + boundary);

                // Multipart header for the file part
                String fileName = (pendingFileName != null) ? pendingFileName : "firmware.bin";
                String partHeader =
                        "--" + boundary + crlf +
                                "Content-Disposition: form-data; name=\"update\"; filename=\"" + fileName + "\"" + crlf +
                                "Content-Type: application/octet-stream" + crlf +
                                crlf;

                String partFooter = crlf + "--" + boundary + "--" + crlf;

                // If you know file size, compute the full content-length
                // so Android doesn't switch to chunked transfer on some devices.
                if (size > 0) {
                    long contentLength =
                            partHeader.getBytes(StandardCharsets.UTF_8).length +
                                    size +
                                    partFooter.getBytes(StandardCharsets.UTF_8).length;
                    conn.setFixedLengthStreamingMode(contentLength);
                } else {
                    // fallback (may become chunked; usually still OK, but fixed-length is better)
                    conn.setChunkedStreamingMode(8192);
                }

                try (InputStream in = new BufferedInputStream(getContentResolver().openInputStream(uri));
                     OutputStream out = new BufferedOutputStream(conn.getOutputStream())) {

                    out.write(partHeader.getBytes(StandardCharsets.UTF_8));

                    byte[] buf = new byte[8192];
                    long sent = 0;
                    int r;
                    while ((r = in.read(buf)) != -1) {
                        out.write(buf, 0, r);
                        sent += r;

                    }

                    out.write(partFooter.getBytes(StandardCharsets.UTF_8));
                    out.flush();
                }

                int code = conn.getResponseCode();
                if (code >= 200 && code < 300) {
                    showOtaStatus(getString(R.string.ota_http_ok, code));
                } else {
                    showOtaStatus(getString(R.string.ota_http_fail, code));
                }

            } catch (Exception e) {
                Log.e(TAG, "Upload failed", e);
                showOtaStatus(getString(R.string.ota_http_exception, e.getMessage()));
            } finally {
                if (conn != null) conn.disconnect();
            }
        });
    }

    private void showOtaStatus(String s) {
        mainHandler.post(() -> {
            otaStatus.setVisibility(View.VISIBLE);
            otaStatus.setText(s);
        });
    }

    private String queryDisplayName(Uri uri) {
        try (Cursor cursor = getContentResolver().query(uri, null, null, null, null)) {
            if (cursor != null && cursor.moveToFirst()) {
                int idx = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                if (idx >= 0) return cursor.getString(idx);
            }
        } catch (Exception ignored) {
        }
        return null;
    }

    private long querySize(Uri uri) {
        try (Cursor cursor = getContentResolver().query(uri, null, null, null, null)) {
            if (cursor != null && cursor.moveToFirst()) {
                int idx = cursor.getColumnIndex(OpenableColumns.SIZE);
                if (idx >= 0) return cursor.getLong(idx);
            }
        } catch (Exception ignored) {
        }
        return -1;
    }

    public static class ToolsFragment extends Fragment {
    }

    private boolean hasWifiSsidPermission() {
        // SSID access requires location permission on many Android versions.
        return ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED;
    }

    private void requestWifiSsidPermissionIfNeeded() {
        if (!hasWifiSsidPermission()) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, REQ_WIFI_PERMS);
        }
    }

    private String sanitizeSsid(String ssid) {
        if (ssid == null) return null;
        ssid = ssid.trim();
        if (ssid.equalsIgnoreCase("<unknown ssid>")) return null;
        // WifiInfo.ssid may come with quotes
        if (ssid.length() >= 2 && ssid.startsWith("\"") && ssid.endsWith("\"")) {
            ssid = ssid.substring(1, ssid.length() - 1);
        }
        return ssid;
    }

    private String getCurrentWifiSsid() {
        try {
            // Preferred approach (works well on newer APIs)
            ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
            if (cm != null) {
                Network active = cm.getActiveNetwork();
                if (active != null) {
                    NetworkCapabilities caps = cm.getNetworkCapabilities(active);
                    if (caps != null && caps.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)) {
                        WifiManager wm = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
                        if (wm != null) {
                            WifiInfo info = wm.getConnectionInfo();
                            if (info != null) {
                                return sanitizeSsid(info.getSSID());
                            }
                        }
                    }
                }
            }

            // Fallback
            WifiManager wm = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
            if (wm != null) {
                WifiInfo info = wm.getConnectionInfo();
                if (info != null) return sanitizeSsid(info.getSSID());
            }
        } catch (Exception e) {
            Log.w(TAG, "Failed to get SSID", e);
        }
        return null;
    }

}
