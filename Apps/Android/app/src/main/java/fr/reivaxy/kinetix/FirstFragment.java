package fr.reivaxy.kinetix;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.ColorStateList;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;

import androidx.annotation.NonNull;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.fragment.app.Fragment;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import androidx.preference.PreferenceManager;

import com.google.android.material.snackbar.Snackbar;

import fr.reivaxy.kinetix.databinding.FragmentFirstBinding;

public class FirstFragment extends Fragment {

    private FragmentFirstBinding binding;

    private HandHandler handHandler;
    private final static String TAG = FirstFragment.class.getSimpleName();
    private SharedPreferences.OnSharedPreferenceChangeListener preferenceChangeListener;

    private ColorStateList defaultTintList = null;
    private final BroadcastReceiver connectionReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // Handle the connection failure here
            Log.i(TAG, "onReceive: " + intent.getAction());
            if (intent.getAction().equals(BluetoothHandler.ACTION_GATT_CONNECTED)) {
                showConnected(true);
            } else {
                showConnected(false);
            }
        }
    };

    @Override
    public View onCreateView(
            @NonNull LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState
    ) {

        binding = FragmentFirstBinding.inflate(inflater, container, false);
        return binding.getRoot();
    }

    public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        VoiceStatusUI vsui = new VoiceStatusUI(binding.textViewSpeechLocale,
                binding.textViewSpeechStatus, binding.textViewSpeechResult);
        handHandler = HandHandler.getInstance(this, vsui);

        LocalBroadcastManager.getInstance(getContext()).registerReceiver(connectionReceiver,
                new IntentFilter(BluetoothHandler.ACTION_GATT_CONNECTED));
        LocalBroadcastManager.getInstance(getContext()).registerReceiver(connectionReceiver,
                new IntentFilter(BluetoothHandler.ACTION_GATT_DISCONNECTED));

        defaultTintList = binding.buttonOpenPinch.getBackgroundTintList(); // whichever

        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getContext());

        // Set up the SharedPreferences listener
        preferenceChangeListener = new SharedPreferences.OnSharedPreferenceChangeListener() {
            @Override
            public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
                if (key.equals(getString(R.string.buttonsPositionKey))) {
                    updateButtonPosition(sharedPreferences);
                }
                if (key.equals(getString(R.string.voiceControlKey))) {
                    updateVoiceControl(sharedPreferences);
                }
            }
        };
        preferences.registerOnSharedPreferenceChangeListener(preferenceChangeListener);
        updateButtonPosition(preferences);
        updateVoiceControl(preferences);

        // Set up listeners for all buttons with a tag
        setupButtons(binding.centerLayout);

        // Special case for connect button which doesn't use a position command
        binding.buttonConnect.setOnClickListener(v -> handHandler.connect());

        // Initialize UI based on connection state
        showConnected(BluetoothHandler.getInstance().isConnected());
    }

    /**
     * Recursively find all Buttons in the layout and set their click listener
     * if they have a tag defined in XML.
     */
    private void setupButtons(ViewGroup layout) {
        for (int i = 0; i < layout.getChildCount(); i++) {
            View child = layout.getChildAt(i);
            if (child instanceof Button) {
                Button button = (Button) child;
                Object tag = button.getTag();
                if (tag != null) {
                    button.setOnClickListener(v -> sendPosition(tag.toString(), button));
                }
            } else if (child instanceof ViewGroup) {
                setupButtons((ViewGroup) child);
            }
        }
    }

    private void updateVoiceControl(SharedPreferences preferences) {
        if (binding == null) return;
        boolean voiceControl = preferences.getBoolean(getString(R.string.voiceControlKey), false);
        if (voiceControl) {
            handHandler.start();
            binding.instructions.setVisibility(View.VISIBLE);
            binding.voiceControlDisabled.setVisibility(View.GONE);
        } else {
            handHandler.stop();
            binding.instructions.setVisibility(View.GONE);
            binding.voiceControlDisabled.setVisibility(View.VISIBLE);
        }
    }
    private void updateButtonPosition(SharedPreferences preferences) {
        if (binding == null) return;
        String buttonsPosition = preferences.getString(getString(R.string.buttonsPositionKey), "center");
        LinearLayout.LayoutParams paramsRight = (LinearLayout.LayoutParams) binding.buttonListRight.getLayoutParams();
        LinearLayout.LayoutParams paramsLeft = (LinearLayout.LayoutParams) binding.buttonListLeft.getLayoutParams();
        if (buttonsPosition.equals("right")) {
            // Set the new weight
            paramsRight.weight = 0;
            paramsLeft.weight = 2;
        }
        if (buttonsPosition.equals("left")) {
            paramsRight.weight = 2;
            paramsLeft.weight = 0;
        }
        if (buttonsPosition.equals("center")) {
            paramsRight.weight = 1;
            paramsLeft.weight = 1;
        }
        // Apply the updated LayoutParams back to the view
        binding.buttonListLeft.setLayoutParams(paramsLeft);
        binding.buttonListRight.setLayoutParams(paramsRight);
    }

    public void emptyAddress() {
        if (binding == null || getActivity() == null) return;
        final Snackbar snackBar = Snackbar.make(getActivity().findViewById(android.R.id.content), R.string.noMacAddress, Snackbar.LENGTH_LONG)
                .setAnchorView(binding.buttonConnect);
        snackBar.setAction(R.string.closeView, new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                snackBar.dismiss();
            }

        });
        snackBar.show();
    }


    private void sendPosition(String position, Button button) {
        handHandler.setPosition(position);
        flashButton(button);
    }

    public void flashButton(String command) {
        if (binding == null) return;
        Button button = null;
        switch (command) {
            case "five":
                button = binding.buttonOpen;
                break;
            case "fist":
                button = binding.buttonFist;
                break;
            case "one":
                button = binding.buttonOne;
                break;
            case "two":
                button = binding.buttonTwo;
                break;
            case "three":
                button = binding.buttonThree;
                break;
            case "four":
                button = binding.buttonFour;
                break;
            case "openPinch":
                button = binding.buttonOpenPinch;
                break;
            case "closePinch":
                button = binding.buttonClosePinch;
                break;
            case "ok":
                button = binding.buttonOk;
                break;
            case "rock":
                button = binding.buttonRock;
                break;
            case "love":
                button = binding.buttonLove;
                break;
            case "come":
                button = binding.buttonCome;
                break;
            case "scratch":
                button = binding.buttonScratch;
                break;
            case "demo":
                button = binding.buttonDemo;
                break;
        }
        if (button != null) {
            flashButton(button);
        }
    }
    public void flashButton(Button button) {
        button.setBackgroundTintList(AppCompatResources.getColorStateList(getContext(), R.color.green));
        new Thread(new Runnable() {
            public void run() {
                try {
                    Thread.sleep(100);
                } catch ( InterruptedException e ) {
                    // not bad if interrupted: sleeps a bit faster (can happen?)
                }
                if (binding != null) {
                    button.setBackgroundTintList(defaultTintList);
                }
            }
        }).start();
    }


    public void showConnecting() {
        if (binding == null) return;
        Button button = binding.buttonConnect;
        button.setBackgroundTintList(AppCompatResources.getColorStateList(getContext(), R.color.yellow));
        button.setText(R.string.connecting);
    }

    public void showConnected(boolean connected) {
        if (binding == null || getActivity() == null) return;
        Button button = binding.buttonConnect;
        if (connected) {
            button.setBackgroundTintList(AppCompatResources.getColorStateList(getContext(), R.color.green));
            button.setText(R.string.connected);
        } else {
            Snackbar.make(getActivity().findViewById(android.R.id.content), R.string.connectionFailed, Snackbar.LENGTH_LONG)
                    .setAnchorView(button)
                    .show();
            button.setBackgroundTintList(AppCompatResources.getColorStateList(getContext(), R.color.red));
            button.setText(R.string.connect);
        }

    }
    
    @Override
    public void onDestroyView() {
        super.onDestroyView();
        BluetoothHandler.getInstance().close();
        binding = null;
        if (handHandler != null) {
            handHandler.stop();
        }
        LocalBroadcastManager.getInstance(getContext()).unregisterReceiver(connectionReceiver);


    }

}
