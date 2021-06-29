package com.RTR.Events;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.content.pm.ActivityInfo;
import android.view.View;

public class MainActivity extends AppCompatActivity
{
    private MyView myView;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_IMMERSIVE               |
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE           |
            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION  |
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN       |
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION         |
            View.SYSTEM_UI_FLAG_FULLSCREEN
        );
        
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        myView = new MyView(this);
        setContentView(myView);
    }
}