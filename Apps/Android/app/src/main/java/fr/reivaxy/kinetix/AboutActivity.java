package fr.reivaxy.kinetix;

import android.content.Context;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

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

        // Copy-to-clipboard button (bottom-right)
        View copyFab = findViewById(R.id.copyAllButton);
        if (copyFab != null) {
            copyFab.setOnClickListener(v -> copyAllAboutInfoToClipboard());
        }
    }

    private void copyAllAboutInfoToClipboard() {
        View root = findViewById(R.id.aboutContentRoot);
        if (root == null) {
            // Fallback to the whole activity root if the expected container id is not present
            root = getWindow().getDecorView().getRootView();
        }

        String text = buildCopyText(root).trim();
        if (text.isEmpty()) {
            Toast.makeText(this, "Nothing to copy", Toast.LENGTH_SHORT).show();
            return;
        }

        ClipboardManager clipboard = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
        if (clipboard == null) {
            Toast.makeText(this, "Clipboard unavailable", Toast.LENGTH_SHORT).show();
            return;
        }

        clipboard.setPrimaryClip(ClipData.newPlainText("About", text));
    }

    /**
     * Builds a readable, email-friendly text:
     * - horizontal rows become one line ("Label: value")
     * - headings (larger text size) are separated by blank lines
     */
    private String buildCopyText(View view) {
        StringBuilder sb = new StringBuilder();
        float headingPx = 18f * getResources().getDisplayMetrics().scaledDensity;
        appendViewText(view, sb, headingPx);
        return sb.toString();
    }

    private void appendViewText(View v, StringBuilder sb, float headingPx) {
        if (v == null) return;

        if (v instanceof LinearLayout) {
            LinearLayout ll = (LinearLayout) v;
            if (ll.getOrientation() == LinearLayout.HORIZONTAL) {
                // Join direct TextView children into a single line.
                StringBuilder line = new StringBuilder();
                for (int i = 0; i < ll.getChildCount(); i++) {
                    View child = ll.getChildAt(i);
                    if (child instanceof TextView) {
                        CharSequence t = ((TextView) child).getText();
                        if (t != null) {
                            String s = t.toString().trim();
                            if (!s.isEmpty()) {
                                if (line.length() > 0 && !line.toString().endsWith(" ")) line.append(' ');
                                line.append(s);
                            }
                        }
                    }
                }
                String out = line.toString().trim();
                if (!out.isEmpty()) {
                    sb.append(out).append('\n');
                }
                return; // Do not recurse; we already consumed the row.
            }
        }

        if (v instanceof ViewGroup) {
            ViewGroup vg = (ViewGroup) v;
            for (int i = 0; i < vg.getChildCount(); i++) {
                appendViewText(vg.getChildAt(i), sb, headingPx);
            }
            return;
        }

        if (v instanceof TextView) {
            TextView tv = (TextView) v;
            CharSequence t = tv.getText();
            if (t == null) return;

            String s = t.toString().trim();
            if (s.isEmpty()) return;

            // Separate sections for headings (e.g., 20sp in this layout).
            if (tv.getTextSize() >= headingPx && sb.length() > 0) {
                // Ensure exactly one blank line before headings.
                if (!sb.toString().endsWith("\n\n")) {
                    if (sb.toString().endsWith("\n")) sb.append('\n');
                    else sb.append("\n\n");
                }
            }

            sb.append(s).append('\n');
        }
    }

    public void openLink(View v) {
        Intent browserIntent = new Intent(Intent.ACTION_VIEW,
                Uri.parse(String.valueOf(((TextView) v).getText())));
        startActivity(browserIntent);
    }

    public static class AboutFragment extends Fragment {
        // (unused here; can be removed if you no longer need it)
    }
}
