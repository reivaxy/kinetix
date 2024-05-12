package fr.reivaxy.kinetix;

import static android.speech.RecognizerIntent.EXTRA_MASK_OFFENSIVE_WORDS;

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
    private Intent intentSpeech;
    private SpeechRecognizer speechRecognizer;

    public CustomSTT(Activity activity, Locale language, HandHandler handHandler) {
        this.handHandler = handHandler;
        speechRecognizer = SpeechRecognizer.createSpeechRecognizer(activity);
        speechRecognizer.setRecognitionListener(this);
        intentSpeech = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
        intentSpeech.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL,
                RecognizerIntent.LANGUAGE_MODEL_FREE_FORM);
        if (language == null) {
            intentSpeech.putExtra(RecognizerIntent.EXTRA_LANGUAGE, language.getLanguage());
            intentSpeech.putExtra(RecognizerIntent.EXTRA_LANGUAGE_PREFERENCE, language.getLanguage());
        }
        handHandler.getVoiceStatusUI().setLanguage(language.getDisplayLanguage());

//        intentSpeech.putExtra(RecognizerIntent.EXTRA_PREFER_OFFLINE, true);
    }

    public void startCustomSTT() {
        if(speechRecognizer != null && intentSpeech != null) {
            speechRecognizer.startListening(intentSpeech);
        }
    }

    public void stopCustomSTT() {
        Log.d(TAG, "stopCustomSTT");
        if(speechRecognizer != null) {
            speechRecognizer.stopListening();
        }
    }

    @Override
    public void onReadyForSpeech(Bundle params) {
        Log.d(TAG, "onReadyForSpeech");
        handHandler.getVoiceStatusUI().setStatus("onReadyForSpeech");
    }

    @Override
    public void onBeginningOfSpeech() {
        Log.d(TAG, "onBeginningOfSpeech");
        handHandler.getVoiceStatusUI().setStatus("onBeginningOfSpeech");
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
        handHandler.getVoiceStatusUI().setStatus("onBufferReceived");
    }

    @Override
    public void onEndOfSpeech() {
        Log.d(TAG, "onEndOfSpeech");
        handHandler.getVoiceStatusUI().setStatus("onEndOfSpeech");
    }

    @Override
    public void onError(int error) {
        String msg = "Didn't get it";
        if (error == 7) {
            handHandler.getVoiceStatusUI().setStatus(msg);
        } else {
            msg = String.format("onError %d", error);
            handHandler.getVoiceStatusUI().setStatus(msg);
        }
        Log.d(TAG, msg);
        startCustomSTT();
    }

    @Override
    public void onResults(Bundle results) {
        String message = results.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION).get(0).toLowerCase();
        Log.d(TAG, String.format("onResults %s", message));
        handHandler.getVoiceStatusUI().setResult(message);
        handHandler.gotVocalMessage(message);
        startCustomSTT();
    }

    @Override
    public void onPartialResults(Bundle partialResults) {
        Log.d(TAG, "onPartialResults");
        handHandler.getVoiceStatusUI().setStatus("onPartialResults");
    }

    @Override
    public void onEvent(int eventType, Bundle params) {
        Log.d(TAG, "onEvent");
        handHandler.getVoiceStatusUI().setStatus("onEvent");
    }
}
