package com.dex.opencvlearn;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

public class OpenCvActivity extends AppCompatActivity {

    static {
        System.loadLibrary("native-lib");
    }

    private Bitmap mBmp;
    private ImageView mViewOpen;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_open_cv);
        ImageView viewById = findViewById(R.id.iv_open);
        mViewOpen = findViewById(R.id.iv_open_deal);
        Button btnOpen = findViewById(R.id.btn_open);
        mBmp = BitmapFactory.decodeResource(getResources(), R.drawable.android);
//        viewById.setImageBitmap(mBmp);

        int w = mBmp.getWidth();
        int h = mBmp.getHeight();
        int[] pixels = new int[w * h];
        mBmp.getPixels(pixels, 0, w, 0, 0, w, h);
        int[] resultInt = grayProc(pixels, w, h);
        Bitmap resultImg = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        resultImg.setPixels(resultInt, 0, w, 0, 0, w, h);
        mViewOpen.setImageBitmap(resultImg);

        btnOpen.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
            }
        });


    }

    public static native int[] grayProc(int[] pixels, int w, int h);
}
