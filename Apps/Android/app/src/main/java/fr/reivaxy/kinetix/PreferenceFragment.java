package fr.reivaxy.kinetix;

import android.os.Bundle;
import android.util.Log;

import androidx.preference.PreferenceFragmentCompat;

public class PreferenceFragment extends PreferenceFragmentCompat {
    private final static String TAG = PreferenceFragment.class.getSimpleName();
    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        Log.i(TAG, "onCreatePreferences");
        setPreferencesFromResource(R.xml.preferences, rootKey);
    }
}
