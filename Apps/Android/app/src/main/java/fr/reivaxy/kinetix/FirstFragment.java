package fr.reivaxy.kinetix;

import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.res.ColorStateList;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.fragment.app.Fragment;
import androidx.preference.PreferenceManager;

import com.google.android.material.snackbar.Snackbar;
import java.nio.charset.StandardCharsets;
import fr.reivaxy.kinetix.databinding.FragmentFirstBinding;

public class FirstFragment extends Fragment {

    private FragmentFirstBinding binding;

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

        binding.buttonFist.setOnClickListener(v -> {
                    sendPosition(view, "fist", R.id.button_fist);
                }
        );
        binding.buttonOpen.setOnClickListener(v -> {
                    sendPosition(view, "five", R.id.button_open);
                }
        );
        binding.buttonPinch.setOnClickListener(v -> {
                    sendPosition(view, "openPinch", R.id.button_pinch);
                }
        );

        binding.buttonOne.setOnClickListener(v -> {
                    sendPosition(view, "one", R.id.button_one);
                }
        );

        binding.buttonTwo.setOnClickListener(v -> {
                    sendPosition(view, "two", R.id.button_two);
                }
        );

        binding.buttonThree.setOnClickListener(v -> {
                    sendPosition(view, "three", R.id.button_three);
                }
        );

        binding.buttonFour.setOnClickListener(v -> {
                    sendPosition(view, "four", R.id.button_four);
                }
        );

        binding.buttonFive.setOnClickListener(v -> {
                    sendPosition(view, "five", R.id.button_five);
                }
        );

        binding.buttonOk.setOnClickListener(v -> {
                    sendPosition(view, "ok", R.id.button_ok);
                }
        );



        binding.buttonConnect.setOnClickListener(v -> {
                    SharedPreferences sharedPreferences =
                            PreferenceManager.getDefaultSharedPreferences(getContext());
                    String address = sharedPreferences.getString(getString(R.string.macAddressKey), "");
                    if (address.isEmpty()) {
                        final Snackbar snackBar = Snackbar.make(view, R.string.noMacAddress, Snackbar.LENGTH_LONG)
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
                    connectTo(view, address, binding.buttonConnect);
                }
        );
    }

    private void connectTo(View view, String address, Button button ) {
        boolean connected = BluetoothHandler.getInstance().connect(this.getContext(), address);
        if (!connected) {
            Snackbar.make(view, "Connection initialization failed.", Snackbar.LENGTH_LONG)
                    .setAnchorView(button.getId())
                    .setAction("Action", null).show();
            showConnected(false);
        } else {
            // TODO: move this under callback/notif to actual connection successful
            showConnected(true);

        }
    }

    private void sendPosition(View view, String position, int buttonId) {
        if (BluetoothHandler.getInstance().isConnected()) {
            BluetoothHandler.getInstance().writeCustomCharacteristic(position.getBytes(StandardCharsets.UTF_8));
        } else {
            Log.e(TAG, "sendPosition: not connected");
//            showConnected(binding.buttonConnectProto, false);
            showConnected(false);
            Snackbar.make(view, binding.getRoot().getResources().getString(R.string.notConnected), Snackbar.LENGTH_LONG)
                .setAnchorView(buttonId)
                .setAction("Action", null).show();
        }
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
    }

}