package fr.reivaxy.kinetix;

import android.widget.TextView;

public class VoiceStatusUI {

    TextView language;
    TextView status;
    TextView result;

    // pass null values if no display wanted
    public VoiceStatusUI(TextView language, TextView status, TextView result) {
        this.language = language;
        this.status = status;
        this.result = result;
    }

    public void setLanguage(String value) {
        if (language != null) {
            language.setText(value);
        }
    }

    public void setStatus(String value) {
        if (language != null) {
            status.setText(value);
        }
    }

    public void setResult(String value) {
        if (language != null) {
            result.setText(value);
        }
    }



}
