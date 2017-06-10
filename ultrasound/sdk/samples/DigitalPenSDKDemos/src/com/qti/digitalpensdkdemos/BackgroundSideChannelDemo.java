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

import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.Area;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.BackgroundSideChannelCanceledListener;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.OffScreenMode;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.OnSideChannelDataListener;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.SideChannelData;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.SideChannelMapping;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.LinearLayout;

public class BackgroundSideChannelDemo extends Activity {

    private static final String TAG = "BackgroundSideChannelDemo";

    /* This listener's lifecycle is outside onCreate to survive onPause / onResume */
    private final OnSideChannelDataListener listener = new OnSideChannelDataListener() {
        @Override
        public void onDigitalPenData(SideChannelData data) {
            if (mDrawingView != null) {
                mDrawingView.onExternalSideChannelData(data);
            }
        }
    };

    private static final int MAX_HOVER_DISTANCE = 10000;

    /* The digital pen manager object */
    private DigitalPenManager mDigitalPenManager = null;

    /* The view that handles drawing off-screen points on the screen */
    private DrawingView mDrawingView = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_background_side_channel_demo);

        /* Initialize the drawing view and erase button */
        LinearLayout drawingPane = (LinearLayout) findViewById(R.id.offscreen_drawing_pane);
        mDrawingView = new DrawingView(this);
        drawingPane.addView(mDrawingView);

        findViewById(R.id.buttonEraseCanvas).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mDrawingView.erase();
            }
        });

        /* Initialize the Digital Pen Manager */

        mDigitalPenManager = new DigitalPenManager(getApplication());

        /*
         * Enable extended off-screen mode and set the off-screen side-channel
         * mapping to SCALED.
         */
        mDigitalPenManager.getSettings()
                .setOffScreenMode(OffScreenMode.EXTEND)
                .setOffScreenHoverEnabled(MAX_HOVER_DISTANCE)
                .setSideChannelMapping(Area.OFF_SCREEN, SideChannelMapping.SCALED)
                .apply();

        mDigitalPenManager.registerSideChannelEventListener(Area.OFF_SCREEN_BACKGROUND,
                listener);
        mDigitalPenManager.setBackgroundSideChannelCanceledListener(new BackgroundSideChannelCanceledListener() {

            @Override
            public void onBackgroundSideChannelCanceled(Area areaCanceled) {
                Log.d(TAG, "Our listener was canceled!");
            }
        });
    }

}
