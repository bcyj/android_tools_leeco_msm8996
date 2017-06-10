/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 */
 /*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.qti.digitalpensdkdemos;


import android.app.Presentation;
import android.content.Context;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnHoverListener;
import android.view.View.OnTouchListener;

class OffscreenPresentation extends Presentation {

    private static final String TAG = "OffscreenPresentation";

    private final int mHoverMaxDistance;

    private final View mOffscreenCircleIndicator;

    private final DrawingView mDrawingView;

    public OffscreenPresentation(Context context, Display offscreenDisplay, int hoverMaxDistance,
            View offscreenCircleIndicator, DrawingView drawingView) {
        super(context, offscreenDisplay);
        mHoverMaxDistance = hoverMaxDistance;
        mOffscreenCircleIndicator = offscreenCircleIndicator;
        mDrawingView = drawingView;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "OffscreenPresentation onCreate");
        super.onCreate(savedInstanceState);

        setContentView(R.layout.offscreen_virtual_layout);

        setDrawingAreaListeners();
        setButtonListeners();

    }

    private void setDrawingAreaListeners() {
        View vDrawingArea = findViewById(R.id.offscreen_virtual_drawing_area);

        vDrawingArea.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                // Scaling is done in each axis by multiplying the
                // coordinate by the percent that the on-screen view is larger/smaller
                // than the off-screen drawing view in the corresponding dimension.
                float x_scaled = ((float)mDrawingView.getMeasuredWidth() / (float)v
                        .getMeasuredWidth()) * event.getX();
                float y_scaled = ((float)mDrawingView.getMeasuredHeight() / (float)v
                        .getMeasuredHeight()) * event.getY();

                MotionEvent scaled_event = MotionEvent.obtain(event);
                scaled_event.setLocation(x_scaled, y_scaled);
                mDrawingView.onExternalTouchEvent(scaled_event);
                scaled_event.recycle();
                return true;
            }
        });

    }

    private void setButtonListeners() {
        View vButton = findViewById(R.id.offscreen_virtual_button);

        vButton.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                    case MotionEvent.ACTION_MOVE:
                        ((GradientDrawable)mOffscreenCircleIndicator.getBackground())
                        .setColor(Color.GREEN);
                        break;
                    case MotionEvent.ACTION_UP:
                        ((GradientDrawable)mOffscreenCircleIndicator.getBackground())
                        .setColor(Color.RED);
                        break;
                }
                return true;
            }
        });

        vButton.setOnHoverListener(new OnHoverListener() {
            @Override
            public boolean onHover(View view, MotionEvent event) {
                // The on-screen circle gets more transparent as the pen is held
                // higher above the virtual button.
                // event.getAxisValue(MotionEvent.AXIS_DISTANCE) is the current
                // height of the digital pen.
                mOffscreenCircleIndicator.setAlpha(1f - (event
                        .getAxisValue(MotionEvent.AXIS_DISTANCE) / mHoverMaxDistance));
                return true;
            }
        });
    }
}
