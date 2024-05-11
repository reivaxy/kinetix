package fr.reivaxy.kinetix;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.speech.RecognitionListener;
import android.speech.RecognizerIntent;
import android.speech.SpeechRecognizer;
import android.util.Log;
import java.util.Locale;
import fr.reivaxy.kinetix.databinding.FragmentFirstBinding;

public class CustomSTT implements RecognitionListener {
    private final String TAG = CustomSTT.class.getSimpleName();

    private Activity activity;
    private final FirstFragment ff;
    private FragmentFirstBinding ffBinding;
    private Intent intentSpeech;
    private SpeechRecognizer speechRecognizer;

    public CustomSTT(Activity activity, FragmentFirstBinding mainBinding, Locale language, FirstFragment ff) {
        this.activity = activity;
        this.ffBinding = mainBinding;
        this.ff = ff;
        speechRecognizer = SpeechRecognizer.createSpeechRecognizer(activity);
        speechRecognizer.setRecognitionListener(this);
        intentSpeech = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
        intentSpeech.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL,
                RecognizerIntent.LANGUAGE_MODEL_FREE_FORM);
        intentSpeech.putExtra(RecognizerIntent.EXTRA_LANGUAGE, language.getLanguage());
        intentSpeech.putExtra(RecognizerIntent.EXTRA_LANGUAGE_PREFERENCE, language.getLanguage());
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
        ffBinding.textViewSpeechStatus.setText("onReadyForSpeech");
    }

    @Override
    public void onBeginningOfSpeech() {
        Log.d(TAG, "onBeginningOfSpeech");
        ffBinding.textViewSpeechStatus.setText("onBeginningOfSpeech");
    }

    @Override
    public void onRmsChanged(float rmsdB) {
//        Log.d(TAG, "onRmsChanged");
        ffBinding.textViewSpeechStatus.setText("onRmsChanged");
    }

    @Override
    public void onBufferReceived(byte[] buffer) {
        Log.d(TAG, "onBufferReceived");
        ffBinding.textViewSpeechStatus.setText("onBufferReceived");
    }

    @Override
    public void onEndOfSpeech() {
        Log.d(TAG, "onEndOfSpeech");
        ffBinding.textViewSpeechStatus.setText("onEndOfSpeech");
    }

    @Override
    public void onError(int error) {
        String msg = String.format("onError %d", error);
        Log.d(TAG, msg);
        ffBinding.textViewSpeechStatus.setText(msg);
        startCustomSTT();
    }

    @Override
    public void onResults(Bundle results) {
        Log.d(TAG, "onResults ");
        String position = results.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION).get(0).toLowerCase();
        ffBinding.textViewSpeechResult.setText(position);
        ff.sendPosition(position, -1);
        startCustomSTT();
    }

    @Override
    public void onPartialResults(Bundle partialResults) {
        Log.d(TAG, "onPartialResults");
        ffBinding.textViewSpeechStatus.setText("onPartialResults");
    }

    @Override
    public void onEvent(int eventType, Bundle params) {
        Log.d(TAG, "onEvent");
        ffBinding.textViewSpeechStatus.setText("onEvent");
    }
}
