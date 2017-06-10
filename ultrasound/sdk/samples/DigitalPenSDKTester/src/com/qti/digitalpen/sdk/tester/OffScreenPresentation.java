/*===========================================================================
                           OffscreenPresentation.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.digitalpen.sdk.tester;

import com.qti.digitalpen.sdk.tester.DigitalPenSdkTesterActivity.HandleMotionEvent;

import android.app.Presentation;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnHoverListener;
import android.view.View.OnTouchListener;

class OffScreenPresentation extends Presentation {

    private static final String TAG = "OffscreenPresentation";

    protected int eventCount;

    private HandleMotionEvent motionEventHandler;

    public OffScreenPresentation(Context context, Display offscreenDisplay,
            HandleMotionEvent motionEventHandler) {
        super(context, offscreenDisplay);
        this.motionEventHandler = motionEventHandler;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "OffscreenPresentation onCreate");
        super.onCreate(savedInstanceState);

        setContentView(R.layout.offscreen_presentation);

        View topView = findViewById(R.id.offscreen_view);
        topView.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                motionEventHandler.setOnMotionDataEvents("OffScreenPresentation touch event: ",
                        event);
                return true;
            }
        });
        topView.setOnHoverListener(new OnHoverListener() {

            @Override
            public boolean onHover(View v, MotionEvent event) {
                motionEventHandler.setOnMotionDataEvents("OffScreenPresentation hover event: ",
                        event);
                return true;
            }
        });
    }
}
