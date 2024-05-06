package fr.reivaxy.kinetix;

import android.os.Bundle;
import android.os.Handler;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;

public class PreferenceActivity extends AppCompatActivity {
    private final static String TAG = PreferenceActivity.class.getSimpleName();

    @Override
        protected void onCreate(Bundle savedInstanceState) {
            Log.i(TAG, "onCreate");
            super.onCreate(savedInstanceState);
            getSupportFragmentManager()
                    .beginTransaction()
                    .replace(R.id.FirstFragment, new PreferenceFragment())
                    .commit();

        }
    }
