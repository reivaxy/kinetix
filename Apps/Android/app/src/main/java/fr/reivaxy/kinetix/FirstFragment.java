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
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.fragment.app.Fragment;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import androidx.preference.PreferenceManager;

import com.google.android.material.snackbar.Snackbar;

import org.json.JSONObject;

import java.nio.charset.StandardCharsets;

import fr.reivaxy.kinetix.databinding.FragmentFirstBinding;

public class FirstFragment extends Fragment {

    private FragmentFirstBinding binding;

    private HandHandler handHandler;
    private final static String TAG = FirstFragment.class.getSimpleName();
    private SharedPreferences.OnSharedPreferenceChangeListener preferenceChangeListener;

    private ColorStateList defaultTintList = null;
    private AlertDialog mCurrentPasswordDialog;

    private final BroadcastReceiver connectionReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null) return;
            String action = intent.getAction();
            Log.i(TAG, "onReceive: " + action);
            if (BluetoothHandler.ACTION_GATT_CONNECTED.equals(action)) {
                showConnected(true);
            } else if (BluetoothHandler.ACTION_GATT_DISCONNECTED.equals(action)) {
                showConnected(false);
                if (mCurrentPasswordDialog != null && mCurrentPasswordDialog.isShowing()) {
                    mCurrentPasswordDialog.dismiss();
                }
            } else if (BluetoothHandler.ACTION_PASSWORD_MESSAGE.equals(action)) {
                String response = intent.getStringExtra(BluetoothHandler.EXTRA_PASSWORD_MESSAGE).trim().toLowerCase();
                if ("true".equals(response)) {
                    if (mCurrentPasswordDialog != null && mCurrentPasswordDialog.isShowing()) {
                        mCurrentPasswordDialog.dismiss();
                        Toast.makeText(getContext(), "Authenticated", Toast.LENGTH_SHORT).show();
                    }
                } else if ("false".equals(response)) {
                    // Password required or incorrect
                    if (mCurrentPasswordDialog == null || !mCurrentPasswordDialog.isShowing()) {
                        showPasswordAuthDialog();
                    } else {
                        // Already showing, so it must be an incorrect attempt
                        TextView errorText = mCurrentPasswordDialog.findViewById(R.id.passwordError);
                        if (errorText != null) {
                            errorText.setText(R.string.password_error);
                            errorText.setVisibility(View.VISIBLE);
                        }
                    }
                }
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

        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothHandler.ACTION_GATT_CONNECTED);
        filter.addAction(BluetoothHandler.ACTION_GATT_DISCONNECTED);
        filter.addAction(BluetoothHandler.ACTION_PASSWORD_MESSAGE);
        LocalBroadcastManager.getInstance(getContext()).registerReceiver(connectionReceiver, filter);

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

        // Initialize UI based on connection state, but don't show snackbar at launch
        showConnected(BluetoothHandler.getInstance().isConnected(), false);
    }

    private void showPasswordAuthDialog() {
        if (getContext() == null) return;
        
        AlertDialog.Builder builder = new AlertDialog.Builder(getContext());
        LayoutInflater inflater = requireActivity().getLayoutInflater();
        View dialogView = inflater.inflate(R.layout.dialog_password, null);
        builder.setView(dialogView);

        TextView titleView = new TextView(getContext());
        titleView.setPadding(40, 40, 40, 40);
        titleView.setTextSize(20);
        titleView.setTextColor(getResources().getColor(android.R.color.black));
        titleView.setText(R.string.password_required);
        builder.setCustomTitle(titleView);

        TextView labelCurrentPassword = dialogView.findViewById(R.id.labelCurrentPassword);
        labelCurrentPassword.setText(R.string.enter_password);
        
        EditText txtPassword = dialogView.findViewById(R.id.txtPassword);
        View newPasswordSection = dialogView.findViewById(R.id.newPasswordSection);
        newPasswordSection.setVisibility(View.GONE);

        builder.setPositiveButton(R.string.send, null);
        builder.setNegativeButton(R.string.cancel, (dialog, which) -> dialog.dismiss());

        AlertDialog dialog = builder.create();
        dialog.show();

        dialog.getButton(AlertDialog.BUTTON_POSITIVE).setOnClickListener(v -> {
            String currentPwd = txtPassword.getText().toString().trim();
            if (currentPwd.isEmpty()) {
                return;
            }

            try {
                JSONObject payload = new JSONObject();
                payload.put("pwdCheck", currentPwd);
                BluetoothHandler.getInstance().writePasswordCharacteristic(payload.toString().getBytes(StandardCharsets.UTF_8));
            } catch (Exception e) {
                Log.e(TAG, "Error creating password JSON", e);
            }
        });

        mCurrentPasswordDialog = dialog;
    }

    /**
     * find all Buttons in the layout and set their click listener
     * if they have a tag attribute defined in XML, with tag value as command
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

    // Show relevant message depending on voice control setting
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


    // Send the position command to the hand over BT
    // And flash the button (to show identified voice command)
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
        showConnected(connected, true);
    }

    public void showConnected(boolean connected, boolean showSnackbar) {
        if (binding == null || getActivity() == null) return;
        Button button = binding.buttonConnect;
        if (connected) {
            button.setBackgroundTintList(AppCompatResources.getColorStateList(getContext(), R.color.green));
            button.setText(R.string.connected);
        } else {
            if (showSnackbar) {
                Snackbar.make(getActivity().findViewById(android.R.id.content), R.string.connectionFailed, Snackbar.LENGTH_LONG)
                        .setAnchorView(button)
                        .show();
            }
            button.setBackgroundTintList(AppCompatResources.getColorStateList(getContext(), R.color.red));
            button.setText(R.string.connect);
        }

    }
    
    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
        if (handHandler != null) {
            handHandler.stop();
        }
        LocalBroadcastManager.getInstance(getContext()).unregisterReceiver(connectionReceiver);
    }

}
