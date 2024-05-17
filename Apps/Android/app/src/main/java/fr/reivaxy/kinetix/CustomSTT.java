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

        intentSpeech.putExtra(RecognizerIntent.EXTRA_PREFER_OFFLINE, true);
        intentSpeech.putExtra(RecognizerIntent.EXTRA_SPEECH_INPUT_POSSIBLY_COMPLETE_SILENCE_LENGTH_MILLIS, 100);
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
        String msg = "";
        boolean restart = true;
        switch(error) {
            case 7:
                msg = "Didn't get it";
                break;
            case 8:
                msg = "Recognizer Busy";
                restart = false;
                break;
            default:
                msg = String.format("onError %d", error);
        }
        handHandler.getVoiceStatusUI().setStatus(msg);
        Log.d(TAG, msg);
        if (restart) {
            startCustomSTT();
        }
    }

    @Override
    public void onResults(Bundle results) {
        String message = results.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION).get(0).toLowerCase();
        Log.d(TAG, String.format("onResults %s", message));
//        handHandler.getVoiceStatusUI().setResult(message);
//        handHandler.gotVocalMessage(message);
        startCustomSTT();
    }

    @Override
    public void onPartialResults(Bundle partialResults) {
        String message = partialResults.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION).get(0).toLowerCase();
        Log.d(TAG, String.format("onPartialResults %s", message));
        handHandler.getVoiceStatusUI().setStatus("onPartialResults");
        handHandler.gotVocalMessage(message);
    }

    @Override
    public void onEvent(int eventType, Bundle params) {
        Log.d(TAG, "onEvent");
        handHandler.getVoiceStatusUI().setStatus("onEvent");
    }
}
