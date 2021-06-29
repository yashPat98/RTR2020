package com.RTR.Events;

import android.content.Context;         
import android.graphics.Color;
import androidx.appcompat.widget.AppCompatTextView;

import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

import android.view.Gravity;

public class MyView extends AppCompatTextView
                      implements OnGestureListener, OnDoubleTapListener
{
    private GestureDetector gestureDetector;

    public MyView(Context context)
    {
        super(context);

        setGravity(Gravity.CENTER);
        setBackgroundColor(Color.rgb(0, 0, 0));
        setTextColor(Color.rgb(0, 255, 0));
        setTextSize(64);
        setText("Hello World !!!");

        gestureDetector = new GestureDetector(context, this, null, false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    //overriden methods of OnDoubleTapListener
    @Override
    public boolean onTouchEvent(MotionEvent e)
    {
        //code
        int eventaction = e.getAction();
        if(!gestureDetector.onTouchEvent(e))
        {
            super.onTouchEvent(e);
        }

        return (true);
    }

    @Override 
    public boolean onDoubleTap(MotionEvent e)
    {
        setText("Double Tap");
        return (true);
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e)
    {
        return (true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e)
    {
        setText("Single Tap");
        return (true);
    }

    //overriden methods of OnGestureListener
    @Override
    public boolean onDown(MotionEvent e)
    {
        return (true);
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
    {
        return (true);
    }

    @Override
    public void onLongPress(MotionEvent e)
    {
        setText("Long Press");
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY)
    {
        setText("Scroll");
        System.exit(0);
        return (true);
    }

    @Override
    public void onShowPress(MotionEvent e)
    {
    }

    @Override
    public boolean onSingleTapUp(MotionEvent e)
    {
        return(true);
    }
}