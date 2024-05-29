package fr.reivaxy.kinetix;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.speech.RecognitionListener;
import android.speech.RecognizerIntent;
import android.speech.SpeechRecognizer;
import android.util.Log;

import java.util.Locale;

public class CustomSTT implements RecognitionListener {
    private final String TAG = CustomSTT.class.getSimpleName();

    private final HandHandler handHandler;
    private final Intent speechIntent;
    private final SpeechRecognizer speechRecognizer;
    private final Locale locale;
    private final Activity activity;
    private boolean refreshErrors = true;
    private boolean dontRestart = false;

    private int error13Count = 0;

    public CustomSTT(Activity activity, Locale locale, HandHandler handHandler) {
        this.handHandler = handHandler;
        this.locale = locale;
        this.activity = activity;
        speechRecognizer = SpeechRecognizer.createSpeechRecognizer(activity);
        speechRecognizer.setRecognitionListener(this);
        speechIntent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
        speechIntent.putExtra(RecognizerIntent.EXTRA_PREFER_OFFLINE, true);

        if (locale != null) {
            String language = locale.toLanguageTag();
            speechIntent.putExtra(RecognizerIntent.EXTRA_LANGUAGE, language);
            speechIntent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_PREFERENCE, language);
            speechIntent.putExtra(RecognizerIntent.EXTRA_ONLY_RETURN_LANGUAGE_PREFERENCE, true);
        }
        speechIntent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL,RecognizerIntent.LANGUAGE_MODEL_FREE_FORM);
        speechIntent.putExtra(RecognizerIntent.EXTRA_SPEECH_INPUT_POSSIBLY_COMPLETE_SILENCE_LENGTH_MILLIS, 100);
    }

    public void startCustomSTT() {
        Log.d(TAG, "startCustomSTT");
        dontRestart = false;
        handHandler.getVoiceStatusUI().setLanguage(String.format("%s (%s)", locale.getDisplayLanguage(), locale.toLanguageTag()));
        if(speechRecognizer != null && speechIntent != null) {
            speechRecognizer.startListening(speechIntent);
        }
    }
    public void restartCustomSTT() {
        Log.d(TAG, "restartCustomSTT");
        if (dontRestart) {
            Log.d(TAG, "CustomSTT not restarted");
        } else {
            startCustomSTT();
        }
    }

    public void stopCustomSTT() {
        Log.d(TAG, "stopCustomSTT");
        dontRestart = true;
        if(speechRecognizer != null) {
            speechRecognizer.stopListening();
        }

    }

    @Override
    public void onReadyForSpeech(Bundle params) {
        Log.d(TAG, "onReadyForSpeech");
        handHandler.getVoiceStatusUI().setStatus("onReadyForSpeech", refreshErrors);
    }

    @Override
    public void onBeginningOfSpeech() {
        Log.d(TAG, "onBeginningOfSpeech");
        handHandler.getVoiceStatusUI().setStatus("onBeginningOfSpeech", refreshErrors);
        handHandler.getVoiceStatusUI().setResult("");
    }

    @Override
    public void onRmsChanged(float rmsdB) {
//        Log.d(TAG, "onRmsChanged");
//        ffBinding.textViewSpeechStatus.setText("onRmsChanged");
    }

    @Override
    public void onBufferReceived(byte[] buffer) {
        Log.d(TAG, "onBufferReceived");
        handHandler.getVoiceStatusUI().setStatus("onBufferReceived", refreshErrors);
    }

    @Override
    public void onEndOfSpeech() {
        Log.d(TAG, "onEndOfSpeech");
        handHandler.getVoiceStatusUI().setStatus("onEndOfSpeech", refreshErrors);
    }

    @Override
    public void onError(int error) {
        String msg = "";
        switch(error) {
            case 5:
                msg = "Client error";
                dontRestart = true;
                break;
            case 7:
                msg = "Didn't get it";
                break;
            case 8:
                msg = "Recognizer Busy";
                dontRestart = true;
                break;
            case 13:
                error13Count++;
                if (error13Count > 4) {
                    stopCustomSTT();
                    msg = String.format(activity.getString(R.string.languagePackageMessage), locale.getDisplayLanguage());
                    dontRestart = true;
                    handHandler.getVoiceStatusUI().setStatus(msg, true);
                    refreshErrors = false;
                } else {
                    msg = String.format("onError %d", error);
                }
                break;
            default:
                msg = String.format("onError %d", error);
        }
        handHandler.getVoiceStatusUI().setStatus(msg, refreshErrors);
        Log.d(TAG, msg);
        restartCustomSTT();
    }

    @Override
    public void onResults(Bundle results) {
        String message = results.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION).get(0).toLowerCase();
        Log.d(TAG, String.format("onResults %s", message));
//        handHandler.getVoiceStatusUI().setResult(message);
//        handHandler.gotVocalMessage(message);
        restartCustomSTT();
    }

    @Override
    public void onPartialResults(Bundle partialResults) {
        String message = partialResults.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION).get(0).toLowerCase();
        Log.d(TAG, String.format("onPartialResults %s", message));
        handHandler.getVoiceStatusUI().setStatus("onPartialResults", refreshErrors);
        handHandler.gotVocalMessage(message);
    }

    @Override
    public void onEvent(int eventType, Bundle params) {
        Log.d(TAG, "onEvent");
        handHandler.getVoiceStatusUI().setStatus("onEvent", refreshErrors);
    }
}
