package fr.reivaxy.kinetix;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
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
                    sendPosition("fist", R.id.button_open);
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

        binding.buttonConnect.setOnClickListener(v -> {
                    boolean connected = BluetoothHandler.getInstance().connect(this.getContext(), "74:4D:BD:99:3F:95");
                    if (!connected) {
                        Snackbar.make(view, "Connection initialization failed.", Snackbar.LENGTH_LONG)
                        .setAnchorView(R.id.button_five)
                        .setAction("Action", null).show();
                        showConnected(false);
                    } else {
                        // TODO: move this under callback/notif to actual connection successful
                        showConnected(true);
                    }
                }
        );
    }

    private void sendPosition(String position, int buttonId) {
        if (BluetoothHandler.getInstance().isConnected()) {
            BluetoothHandler.getInstance().writeCustomCharacteristic(position.getBytes(StandardCharsets.UTF_8));
        } else {
            Log.e(TAG, "sendPosition: not connected");
            showConnected(false);
            Snackbar.make(this.getView(), binding.getRoot().getResources().getString(R.string.notConnected), Snackbar.LENGTH_LONG)
                .setAnchorView(buttonId)
                .setAction("Action", null).show();
        }
    }
    private void showConnected(boolean connected) {
        if (connected) {
            binding.buttonConnect.setBackgroundTintList(binding.getRoot().getResources().getColorStateList(R.color.green));
            binding.buttonConnect.setText(binding.getRoot().getResources().getString(R.string.connected));
        } else {
            binding.buttonConnect.setBackgroundTintList(binding.getRoot().getResources().getColorStateList(R.color.red));
            binding.buttonConnect.setText(binding.getRoot().getResources().getString(R.string.connect));
        }

    }
    
    @Override
    public void onDestroyView() {
        super.onDestroyView();
        BluetoothHandler.getInstance().close();
        binding = null;
    }

}