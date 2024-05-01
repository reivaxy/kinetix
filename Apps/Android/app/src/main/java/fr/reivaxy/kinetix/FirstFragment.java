package fr.reivaxy.kinetix;

import android.os.Bundle;
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
                    BluetoothHandler.getInstance().writeCustomCharacteristic("fist".getBytes(StandardCharsets.UTF_8), R.id.button_open);
                }
        );
        binding.buttonOpen.setOnClickListener(v -> {
                    BluetoothHandler.getInstance().writeCustomCharacteristic("five".getBytes(StandardCharsets.UTF_8), R.id.button_open);
                }
        );
        binding.buttonPinch.setOnClickListener(v -> {
                    BluetoothHandler.getInstance().writeCustomCharacteristic("openPinch".getBytes(StandardCharsets.UTF_8), R.id.button_pinch);
                }
        );

        binding.buttonOne.setOnClickListener(v -> {
                    BluetoothHandler.getInstance().writeCustomCharacteristic("one".getBytes(StandardCharsets.UTF_8), R.id.button_one);
                }
        );

        binding.buttonTwo.setOnClickListener(v -> {
                    BluetoothHandler.getInstance().writeCustomCharacteristic("two".getBytes(StandardCharsets.UTF_8), R.id.button_two);
                }
        );

        binding.buttonThree.setOnClickListener(v -> {
                    BluetoothHandler.getInstance().writeCustomCharacteristic("three".getBytes(StandardCharsets.UTF_8), R.id.button_three);
                }
        );

        binding.buttonFour.setOnClickListener(v -> {
                    BluetoothHandler.getInstance().writeCustomCharacteristic("four".getBytes(StandardCharsets.UTF_8), R.id.button_four);
                }
        );

        binding.buttonFive.setOnClickListener(v -> {
                    BluetoothHandler.getInstance().writeCustomCharacteristic("five".getBytes(StandardCharsets.UTF_8), R.id.button_five);
                }
        );

        binding.buttonOk.setOnClickListener(v -> {
                    BluetoothHandler.getInstance().writeCustomCharacteristic("ok".getBytes(StandardCharsets.UTF_8), R.id.button_ok);
                }
        );

        binding.buttonConnect.setOnClickListener(v -> {
                    boolean connected = BluetoothHandler.getInstance().connect(this.getContext(), "74:4D:BD:99:3F:95");
                    if (!connected) {
                        Snackbar.make(view, "Connection initialization failed.", Snackbar.LENGTH_LONG)
                        .setAnchorView(R.id.button_five)
                        .setAction("Action", null).show();
                    } else {
                        // TODO: move this under callback/notif to actual connection successful
                        binding.buttonConnect.setBackgroundTintList(binding.getRoot().getResources().getColorStateList(R.color.green));
                        binding.buttonConnect.setText(binding.getRoot().getResources().getString(R.string.connected));
                        //  binding.buttonConnect.setVisibility(View.INVISIBLE);
//                        binding.build.setText(BluetoothHandler.getInstance().getBuild());

                    }
                }
        );
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        BluetoothHandler.getInstance().close();
        binding = null;
    }

}