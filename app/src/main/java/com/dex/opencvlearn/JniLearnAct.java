package com.dex.opencvlearn;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

public class JniLearnAct extends AppCompatActivity {

    static {
        System.loadLibrary("jniLearn-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_jni_learn);
        TextView jniText = findViewById(R.id.tv_jni);
        jniText.setText(stringJniLearn());
    }

    public native String stringJniLearn();
}
