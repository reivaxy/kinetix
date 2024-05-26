package fr.reivaxy.kinetix;

import static android.view.Gravity.CENTER;
import static android.view.Gravity.LEFT;
import static android.view.Gravity.RIGHT;

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

        LocalBroadcastManager.getInstance(getContext()).registerReceiver(connectionReceiver,
                new IntentFilter(BluetoothHandler.ACTION_GATT_CONNECTED));
        LocalBroadcastManager.getInstance(getContext()).registerReceiver(connectionReceiver,
                new IntentFilter(BluetoothHandler.ACTION_GATT_DISCONNECTED));

        defaultTintList = binding.buttonOpenPinch.getBackgroundTintList(); // whichever

        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getContext());
        String buttonsPosition = preferences.getString(getString(R.string.buttonsPositionKey), "center");
        if (buttonsPosition.equals("right")) {
            binding.buttonList.setGravity(RIGHT);
        }
        if (buttonsPosition.equals("left")) {
            binding.buttonList.setGravity(LEFT);
        }
        if (buttonsPosition.equals("center")) {
            binding.buttonList.setGravity(CENTER);
        }

        VoiceStatusUI vsui = new VoiceStatusUI(binding.textViewSpeechLocale,
                binding.textViewSpeechStatus, binding.textViewSpeechResult);
        handHandler = HandHandler.getInstance(this, vsui);

        binding.buttonFist.setOnClickListener(v -> {
                    sendPosition("fist", binding.buttonFist);
                }
        );
        binding.buttonOpen.setOnClickListener(v -> {
                    sendPosition("five", binding.buttonOpen);
                }
        );
        binding.buttonOpenPinch.setOnClickListener(v -> {
                    sendPosition("openPinch", binding.buttonOpenPinch);
                }
        );
        binding.buttonClosePinch.setOnClickListener(v -> {
                    sendPosition("closePinch", binding.buttonClosePinch);
                }
        );

        binding.buttonOne.setOnClickListener(v -> {
                    sendPosition("one", binding.buttonOne);
                }
        );

        binding.buttonTwo.setOnClickListener(v -> {
                    sendPosition("two", binding.buttonTwo);
                }
        );

        binding.buttonThree.setOnClickListener(v -> {
                    sendPosition("three", binding.buttonThree);
                }
        );

        binding.buttonFour.setOnClickListener(v -> {
                    sendPosition("four", binding.buttonFour);
                }
        );

//        binding.buttonFive.setOnClickListener(v -> {
//                    sendPosition("five", R.id.button_five);
//                }
//        );

        binding.buttonOk.setOnClickListener(v -> {
                    sendPosition("ok", binding.buttonOk);
                }
        );

        binding.buttonScratch.setOnClickListener(v -> {
                    sendPosition("scratch", binding.buttonScratch);
                }
        );

        binding.buttonCome.setOnClickListener(v -> {
                    sendPosition("come", binding.buttonCome);
                }
        );

        binding.buttonRock.setOnClickListener(v -> {
                    sendPosition("rock", binding.buttonRock);
                }
        );

        binding.buttonLove.setOnClickListener(v -> {
                    sendPosition("love", binding.buttonLove);
                }
        );

        binding.buttonConnect.setOnClickListener(v -> {
                    handHandler.connect();
                }
        );
    }

    public void emptyAddress() {
        final Snackbar snackBar = Snackbar.make(getView(), R.string.noMacAddress, Snackbar.LENGTH_LONG)
                .setAnchorView(binding.buttonConnect.getId());
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
                button.setBackgroundTintList(defaultTintList);
            }
        }).start();
    }


    public void showConnecting() {
        Button button = binding.buttonConnect;
        button.setBackgroundTintList(AppCompatResources.getColorStateList(getContext(), R.color.yellow));
        button.setText(binding.getRoot().getResources().getString(R.string.connecting));
    }

    public void showConnected(boolean connected) {
        Button button = binding.buttonConnect;
        if (connected) {
            button.setBackgroundTintList(AppCompatResources.getColorStateList(getContext(), R.color.green));
            button.setText(binding.getRoot().getResources().getString(R.string.connected));
        } else {
            Snackbar.make(binding.getRoot().getRootView(), binding.getRoot().getResources().getString(R.string.connectionFailed), Snackbar.LENGTH_LONG)
                    .setAnchorView(R.id.button_connect)
                    .setAction("Action", null).show();
            button.setBackgroundTintList(AppCompatResources.getColorStateList(getContext(), R.color.red));
            button.setText(binding.getRoot().getResources().getString(R.string.connect));
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