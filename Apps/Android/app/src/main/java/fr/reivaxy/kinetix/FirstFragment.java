package fr.reivaxy.kinetix;

import static android.view.Gravity.CENTER;
import static android.view.Gravity.LEFT;
import static android.view.Gravity.RIGHT;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import androidx.annotation.NonNull;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.fragment.app.Fragment;
import androidx.preference.PreferenceManager;

import com.google.android.material.snackbar.Snackbar;
import java.nio.charset.StandardCharsets;
import java.util.Locale;

import fr.reivaxy.kinetix.databinding.FragmentFirstBinding;

public class FirstFragment extends Fragment {

    private FragmentFirstBinding binding;
    private CustomSTT cstt;

    private final static String TAG = FirstFragment.class.getSimpleName();

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

        cstt = new CustomSTT(this.getActivity(), binding, Locale.FRENCH, this);
        cstt.startCustomSTT();


        binding.buttonFist.setOnClickListener(v -> {
                    sendPosition("fist", R.id.button_fist);
                }
        );
        binding.buttonOpen.setOnClickListener(v -> {
                    sendPosition("five", R.id.button_open);
                }
        );
        binding.buttonPinch.setOnClickListener(v -> {
                    sendPosition("openPinch", R.id.button_pinch);
                }
        );

        binding.buttonOne.setOnClickListener(v -> {
                    sendPosition("one", R.id.button_one);
                }
        );

        binding.buttonTwo.setOnClickListener(v -> {
                    sendPosition("two", R.id.button_two);
                }
        );

        binding.buttonThree.setOnClickListener(v -> {
                    sendPosition("three", R.id.button_three);
                }
        );

        binding.buttonFour.setOnClickListener(v -> {
                    sendPosition("four", R.id.button_four);
                }
        );

        binding.buttonFive.setOnClickListener(v -> {
                    sendPosition("five", R.id.button_five);
                }
        );

        binding.buttonOk.setOnClickListener(v -> {
                    sendPosition("ok", R.id.button_ok);
                }
        );

        binding.buttonScratch.setOnClickListener(v -> {
                    sendPosition("scratch", R.id.button_scratch);
                }
        );

        binding.buttonConnect.setOnClickListener(v -> {
                    connect();
                }
        );

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

    }


    private void connect() {
        SharedPreferences sharedPreferences =
                PreferenceManager.getDefaultSharedPreferences(getContext());
        String address = sharedPreferences.getString(getString(R.string.macAddressKey), "");
        if (address.isEmpty()) {
            final Snackbar snackBar = Snackbar.make(binding.getRoot().getRootView(), R.string.noMacAddress, Snackbar.LENGTH_LONG)
                    .setAnchorView(binding.buttonConnect.getId());
            snackBar.setAction(R.string.closeView, new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    snackBar.dismiss();
                }

            });
            snackBar.show();
            return;
        }
        boolean connected = BluetoothHandler.getInstance().connect(this.getContext(), address);
        if (!connected) {
            Snackbar.make(binding.getRoot().getRootView(), "Connection initialization failed.", Snackbar.LENGTH_LONG)
                    .setAnchorView(R.id.button_connect)
                    .setAction("Action", null).show();
            showConnected(false);
        } else {
            // TODO: move this under callback/notif to actual connection successful
            showConnected(true);

        }
    }

    public void sendPosition(String position, int buttonId) {
        if (buttonId == -1) {
            position = translatePosition(position);
        }
        if ("connect".equals(position)) {
            connect();
            return;
        }

        if (BluetoothHandler.getInstance().isConnected()) {
            BluetoothHandler.getInstance().writeCustomCharacteristic(position.getBytes(StandardCharsets.UTF_8));
        } else {
            Log.e(TAG, "sendPosition: not connected");
//            showConnected(binding.buttonConnectProto, false);
            showConnected(false);
            if (buttonId == -1) {
                buttonId = R.id.button_scratch;
            }
            Snackbar.make(binding.getRoot().getRootView(), binding.getRoot().getResources().getString(R.string.notConnected), Snackbar.LENGTH_LONG)
                .setAnchorView(buttonId)
                .setAction("Action", null).show();
        }
    }

    private String translatePosition(String position) {
        switch(position) {
            case "open":
            case "ouvre":
            case "ouvrir":
                position = "five";
                break;
            case "fermer":
            case "ferme":
            case "close":
                position = "fist";
                break;
            case "connecter":
            case "connecte":
                position = "connect";
                break;
        }
        return position;
    }

    private void showConnected(boolean connected) {
        Button button = binding.buttonConnect;
        if (connected) {
            button.setBackgroundTintList(AppCompatResources.getColorStateList(getContext(), R.color.green));
            button.setText(binding.getRoot().getResources().getString(R.string.connected));
        } else {
            button.setBackgroundTintList(AppCompatResources.getColorStateList(getContext(), R.color.red));
            button.setText(binding.getRoot().getResources().getString(R.string.connect));
        }

    }
    
    @Override
    public void onDestroyView() {
        super.onDestroyView();
        BluetoothHandler.getInstance().close();
        binding = null;
        if (cstt != null) {
            cstt.stopCustomSTT();
        }
    }

}