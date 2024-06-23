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
    private final Pattern pattern1 = Pattern.compile(".*\\bposition (\\w*)\\b.*");

    // Some positions are two words
    private final Pattern pattern2 = Pattern.compile(".*\\bposition (\\w*\\b \\w*)\\b.*");

    private HandHandler(@NonNull FirstFragment fragment, @NonNull VoiceStatusUI voiceStatusUI) {
        this.fragment = fragment;
        this.voiceStatusUI = voiceStatusUI;
        cstt = new CustomSTT(fragment.getActivity(), Locale.getDefault(), this);
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
        if (connected) {
            fragment.showConnecting();
        }
    }

    public void gotVocalMessage(String message) {
        if (message.startsWith("connect")) {
            connect();
            return;
        }
        Matcher matcher1 = pattern1.matcher(message);
        if (!matcher1.matches()) {
            Log.i(TAG, String.format("No pattern found in: '%s'", message));
            return;
        }
        String position = matcher1.group(1);
        Log.i(TAG, String.format("Position found: '%s'", position));

        String translated = translateMessage(position);
        if (translated == null) {
            Matcher matcher2 = pattern2.matcher(message);
            if (!matcher2.matches()) {
                Log.i(TAG, String.format("No pattern found in: '%s'", message));
                return;
            }
            position = matcher2.group(1);
            translated = translateMessage(position);
        }

        Log.i(TAG, String.format("Translated '%s' to '%s'", message, translated));
        if (translated == null) {
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

    private String translateMessage(String position) {

        voiceStatusUI.setResult(position);

        boolean checkFilter = false;
        switch(position) {
            case "1":
            case "un":
            case "one":
            case "pointé":
            case "pointée":
            case "pointe":
            case "pointer":
            case "montrer":
            case "show":
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
            case "two":
            case "victory":
                position = "two";
                break;

            case "3":
            case "trois":
            case "three":
                position = "three";
                break;

            case "4":
            case "quatre":
            case "for":
            case "four":
                position = "four";
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
            case "five":
            case "cinq":
            case "5":
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

            case "scratch":
            case "gratte":
            case "gratter":
            case "gratté":
                position = "scratch";
                break;

            case "pinch":
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
                checkFilter = true;
                break;

            case "come here":
            case "come":
            case "viens":
            case "viens ici":
            case "venez":
            case "venez ici":
                position = "come";
                break;

            case "rock":
            case "love":
                position = position;  // yeah, I know.
                break;

            default:
                position = null;  // Avoid stopping on non recognized commands
        }
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(fragment.getContext());
        boolean safeFilter = preferences.getBoolean("safeFilter", false);
        if (safeFilter && checkFilter) {
            position = null;
        }
        return position;
    }

    public void stop() {
        if (cstt != null) {
            cstt.stopCustomSTT();
        }
        new Thread(new Runnable() {
            public void run() {
                try {
                    Thread.sleep(200);
                } catch ( InterruptedException e ) {
                }
                voiceStatusUI.clear();
            }
        }).start();
    }
    public void start() {
        if (cstt != null) {
            cstt.startCustomSTT();
        }
    }
}
