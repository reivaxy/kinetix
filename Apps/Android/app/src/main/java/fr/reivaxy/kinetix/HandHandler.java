package fr.reivaxy.kinetix;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;
import androidx.annotation.NonNull;
import androidx.preference.PreferenceManager;

import java.nio.charset.StandardCharsets;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class HandHandler {
    private final String TAG = HandHandler.class.getSimpleName();

    private static HandHandler instance = null;
    private static FirstFragment fragment = null;
    private static VoiceStatusUI voiceStatusUI = null;
    private CustomSTT cstt;
    private final Pattern pattern = Pattern.compile(".*\\bposition (\\w*)\\b.*");

    private HandHandler(@NonNull FirstFragment fragment, @NonNull VoiceStatusUI voiceStatusUI) {
        this.fragment = fragment;
        this.voiceStatusUI = voiceStatusUI;
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(fragment.getContext());
        boolean voiceControl = preferences.getBoolean(fragment.getString(R.string.voiceControlKey), false);
        if (voiceControl) {
            cstt = new CustomSTT(fragment.getActivity(), Locale.getDefault(), this);
            cstt.startCustomSTT();
        }

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
        connect(null);
    }
    public void connect(String macAddress) {

        Context context = fragment.getContext();
        SharedPreferences sharedPreferences =
                PreferenceManager.getDefaultSharedPreferences(context);
        if (macAddress != null) {
            SharedPreferences.Editor editor = sharedPreferences.edit();
            editor.putString(context.getString(R.string.macAddressKey), macAddress);
            editor.apply();
        } else {
            macAddress = sharedPreferences.getString(context.getString(R.string.macAddressKey), "");
        }

        if (macAddress.isEmpty()) {
            fragment.emptyAddress();
            return;
        }
        boolean connected = BluetoothHandler.getInstance().connect(context, macAddress);
        if (!connected) {
            fragment.showConnected(false);
        } else {
            // TODO: move this under callback/notif to actual connection successful
            fragment.showConnected(true);

        }
    }

    public void gotVocalMessage(String message) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(fragment.getContext());
        boolean voiceControl = preferences.getBoolean(fragment.getString(R.string.voiceControlKey), false);
        if (!voiceControl) {
            return;
        }

        String translated = translateMessage(message);
        Log.i(TAG, String.format("Translated '%s' to '%s'", message, translated));
        if (translated == null) {
            return;
        }
        if ("connect".equals(translated)) {
            connect();
            return;
        }
        if(translated.equalsIgnoreCase("calibration")) {
            return;
        }
        fragment.flashButton(translated);
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
        if (message.startsWith("connect")) {
            return "connect";
        }
        Matcher matcher = pattern.matcher(message);
        if (!matcher.matches()) {
            Log.i(TAG, String.format("No pattern found in: '%s'", message));
            return null;
        }
        String position = matcher.group(1);
        Log.i(TAG, String.format("Position found: '%s'", position));
        switch(position) {
            case "1":
            case "un":
            case "pointé":
            case "pointée":
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
            case "victoire":
            case "de":
            case "to":
            case "too":
            case "victory":
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
            case "closed":
            case "clothe":
            case "fist":
            case "clothes":  // I need to improve my pronunciation...
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

            case "rock":
            case "love":
                position = position;
                break;

            default:
                position = null;
        }
        return position;
    }

    public void stop() {
        if (cstt != null) {
            cstt.stopCustomSTT();
        }
    }
}
