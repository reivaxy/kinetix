package fr.reivaxy.kinetix;

import android.content.Context;
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
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.preference.PreferenceManager;

public class AboutActivity extends AppCompatActivity {
    private final static String TAG = AboutActivity.class.getSimpleName();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.about_activity);
        if (savedInstanceState == null) {
            getSupportFragmentManager()
                    .beginTransaction()
                    .replace(R.id.about, new AboutFragment())
                    .commit();
        }
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }

        PackageManager packageManager = getPackageManager();

        try {
            // Get the package information
            PackageInfo packageInfo = packageManager.getPackageInfo(getPackageName(), 0);

            // Retrieve the version information
            String versionName = packageInfo.versionName;
            int versionCode = packageInfo.versionCode;

            // Use the version information
            TextView versionNameTextView = findViewById(R.id.versionNameTextView);
            versionNameTextView.setText(versionName);

            TextView versionCodeTextView = findViewById(R.id.versionCodeTextView);
            versionCodeTextView.setText(String.valueOf(versionCode));

            SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getBaseContext());

            String address = preferences.getString(getString(R.string.macAddressKey), "Not set yet");
            TextView macAddress = findViewById(R.id.aboutMacAddress);
            macAddress.setText(address);

            SharedPreferences sharedPref = getBaseContext().getSharedPreferences(
                    getString(R.string.preference_file_key), Context.MODE_PRIVATE);

            String firmwareVersion = sharedPref.getString(getString(R.string.saved_version_key), "Not read yet");
            TextView textView = findViewById(R.id.aboutFirmwareVersion);
            textView.setText(firmwareVersion);
        } catch (Exception e) {
            Log.e(TAG, "Failed: ", e);
        }
    }

    public void openLink(View v) {
        Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(String.valueOf(((TextView)v).getText())));
        startActivity(browserIntent);
    }

     public static class AboutFragment extends Fragment {

    }
}
