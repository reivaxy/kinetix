package fr.reivaxy.kinetix;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.fragment.app.Fragment;
import androidx.preference.PreferenceManager;

import com.google.android.material.snackbar.Snackbar;

import java.nio.charset.StandardCharsets;

public class ToolsActivity extends AppCompatActivity {
    private final static String TAG = ToolsActivity.class.getSimpleName();
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

    }
    public void calibration(View v) {
        try {
            AlertDialog.Builder builder = new AlertDialog.Builder(v.getContext());
            builder.setMessage(R.string.calibMessage)
                    .setTitle(R.string.calibTitle);
            // Add the buttons.
            builder.setPositiveButton(R.string.confirm, new DialogInterface.OnClickListener() {
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
     public static class ToolsFragment extends Fragment {

    }
}
