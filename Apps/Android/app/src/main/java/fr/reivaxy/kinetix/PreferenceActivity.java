package fr.reivaxy.kinetix;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

public class PreferenceActivity extends AppCompatActivity {
        @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            getSupportFragmentManager()
                    .beginTransaction()
                    .replace(R.id.FirstFragment, new PreferenceFragment())
                    .commit();
        }
    }
