package com.rtr.hello;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatTextView;
import android.os.Bundle;
import android.view.Gravity;
import android.graphics.Color;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().getDecorView().setBackgroundColor(Color.rgb(0, 0, 0));

        AppCompatTextView myView = new AppCompatTextView(this);
        
        myView.setText("Hello World !!!");
        myView.setTextSize(32);
        myView.setTextColor(Color.rgb(0, 255, 0));
        myView.setGravity(Gravity.CENTER);
        myView.setBackgroundColor(Color.rgb(0, 0, 0));

        //setContentView(R.layout.activity_main);
        setContentView(myView);
    }
}