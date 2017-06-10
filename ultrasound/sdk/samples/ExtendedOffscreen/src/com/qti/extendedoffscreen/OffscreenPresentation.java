/*===========================================================================
                           OffscreenPresentation.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.extendedoffscreen;

import android.app.Presentation;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnGenericMotionListener;
import android.view.View.OnTouchListener;
import android.widget.ImageView;
import android.widget.TextView;

class OffscreenPresentation extends Presentation {
    private static final int TOUCH_CIRCLE_RADIUS = 2;
    private static final int HOVER_CIRCLE_RADIUS = 10;
    private static final String TAG = "OffscreenPresentation";
    protected int eventCount;
    private TextView textViewTouchCount;
    private ImageView image;

    public OffscreenPresentation(Context context, Display offscreenDisplay) {
        super(context, offscreenDisplay);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "OffscreenPresentation onCreate");
        super.onCreate(savedInstanceState);

        setContentView(R.layout.presentation_extended_offscreen);

        textViewTouchCount = (TextView) findViewById(R.id.textViewPresentationInfo);
        textViewTouchCount.setText("No touch events");
        textViewTouchCount.setBackgroundColor(Color.BLACK);

        image = (ImageView) findViewById(R.id.imageViewPresentationDrawingSurface);
        image.setBackgroundColor(Color.argb(0x40, 0x00, 0x7f, 0x7f));
        image.setImageDrawable(new ColorDrawable(Color.TRANSPARENT));
        image.setOnGenericMotionListener(new OnGenericMotionListener() {

            @Override
            public boolean onGenericMotion(View v, MotionEvent event) {
                Log.d(TAG, "generic motion event: " + event);
                reportEvent(event, HOVER_CIRCLE_RADIUS);
                return true;
            }
        });

        image.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                Log.d(TAG, "onTouch event: " + event);
                reportEvent(event, TOUCH_CIRCLE_RADIUS);
                return true;
            }
        });
    }

    protected void reportEvent(MotionEvent event, int circleRadius) {
        ++eventCount;
        textViewTouchCount.setText("Event count: " + eventCount + ", last event: " + event);
        if (image.getWidth() > 0) {
            Bitmap bitmap = Bitmap.createBitmap(image.getWidth(), image.getHeight(),
                    Bitmap.Config.ARGB_8888);
            Canvas c = new Canvas(bitmap);
            Paint p = new Paint();
            p.setColor(Color.WHITE);
            p.setStrokeWidth(4);
            c.drawCircle(event.getX(), event.getY(), circleRadius, p);
            image.setImageBitmap(bitmap);
        }
    }

}
