package fr.reivaxy.kinetix;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import androidx.annotation.NonNull;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.preference.PreferenceManager;

import com.google.android.material.snackbar.Snackbar;

import java.nio.charset.StandardCharsets;
import java.util.Locale;

public class HandHandler {
    private final String TAG = HandHandler.class.getSimpleName();

    private static HandHandler instance = null;
    private static FirstFragment fragment = null;
    private static VoiceStatusUI voiceStatusUI = null;
    private CustomSTT cstt;

    private HandHandler(@NonNull FirstFragment fragment, @NonNull VoiceStatusUI voiceStatusUI) {
        this.fragment = fragment;
        this.voiceStatusUI = voiceStatusUI;
        cstt = new CustomSTT(fragment.getActivity(), Locale.getDefault(), this);
        cstt.startCustomSTT();
    }

    public static HandHandler getInstance(@NonNull FirstFragment fragment, @NonNull VoiceStatusUI voiceStatusUI) {
        if (instance == null) {
            instance = new HandHandler(fragment, voiceStatusUI);
        }
        return instance;
    }
    public static HandHandler getInstance() throws Exception {
        if (instance == null) {
            throw new Exception("HandHandler not initialized");
        }
        return instance;
    }

    public VoiceStatusUI getVoiceStatusUI() {
        return voiceStatusUI;
    }

    public void connect() {
        Context context = fragment.getContext();
        SharedPreferences sharedPreferences =
                PreferenceManager.getDefaultSharedPreferences(context);
        String address = sharedPreferences.getString(context.getString(R.string.macAddressKey), "");
        if (address.isEmpty()) {
            fragment.emptyAddress();
            return;
        }
        boolean connected = BluetoothHandler.getInstance().connect(context, address);
        if (!connected) {
            fragment.showConnected(false);
        } else {
            // TODO: move this under callback/notif to actual connection successful
            fragment.showConnected(true);

        }
    }

    public void gotVocalMessage(String message) {
        String translated = translateMessage(message);
        Log.i(TAG, String.format("Translated '%s' to '%s'", message, translated));
        if ("connect".equals(translated)) {
            connect();
            return;
        }
        if(translated.equalsIgnoreCase("calibration")) {
            return;
        }
        setPosition(translated);

    }

    public void setPosition(String position) {
        Log.e(TAG, String.format("setPosition: %s", position));

        if (BluetoothHandler.getInstance().isConnected()) {
            BluetoothHandler.getInstance().writeCustomCharacteristic(position.getBytes(StandardCharsets.UTF_8));
        } else {
            Log.e(TAG, "setPosition: not connected");
            fragment.showConnected(false);
        }
    }

    private String translateMessage(String message) {
        String position = message;
        if (message.startsWith("position ")) {
            position = message.substring("position".length() + 1); // remove with trailing space
            Log.i(TAG, String.format("Position removed: '%s'", position));
        } else {
            if (message.startsWith("connect")) {
                return "connect";
            }
            return "";
        }
        switch(position) {
            case "1":
            case "un":
            case "pointe":
            case "pointer":
            case "montrer":
            case "clavier":
            case "pointing":
            case "point":
            case "keyboard":
                position = "one";
                break;

            case "2":
            case "deux":
            case "de":
            case "to":
            case "too":
                position = "two";
                break;

            case "3":
            case "trois":
                position = "three";
                break;

            case "4":
            case "quatre":
            case "for":
                position = "four";
                break;

            case "5":
            case "cinq":
                position = "five";
                break;

            case "okay":
            case "ok":
                position = "ok";
                break;

            case "open":
            case "opened":
            case "ouvre":
            case "ouvrir":
            case "ouvert":
            case "ouverte":
                position = "five";
                break;

            case "fermer":
            case "ferme":
            case "fermé":
            case "fermée":
            case "close":
            case "clothe":
            case "clothes":
                position = "fist";
                break;

            case "gratte":
            case "gratter":
            case "gratté":
                position = "scratch";
                break;

            case "open pinch":
            case "opened pinch":
            case "pince ouverte":
                position = "openPinch";
                break;

            case "close pinch":
            case "closed pinch":
            case "pince fermée":
            case "pince fermer":
                position = "closePinch";
                break;

            case "finger":
            case "the finger":
            case "doigt":
            case "doigt d'honneur":
                position = "fu";
                break;

            case "come here":
            case "viens":
            case "viens ici":
            case "venez":
            case "venez ici":
                position = "come";
                break;
        }
        return position;
    }

    public void stop() {
        if (cstt != null) {
            cstt.stopCustomSTT();
        }
    }
}
