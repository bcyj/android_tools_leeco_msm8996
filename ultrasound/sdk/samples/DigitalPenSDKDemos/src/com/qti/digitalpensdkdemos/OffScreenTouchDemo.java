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

import android.app.Activity;
import android.graphics.Point;
import android.os.Bundle;
import android.view.Display;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.LinearLayout;

import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.OffScreenMode;

public class OffScreenTouchDemo extends Activity {

    private static final int MAX_HOVER_DISTANCE = 10000;

    /* The digital pen manager object */
    private DigitalPenManager mDigitalPenManager = null;

    /* The presentation of the Extended off-screen */
    private OffscreenPresentation mOffscreenPresentation = null;

    private DrawingView mDrawingView = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_offscreen_touch_demo);

        mDigitalPenManager = new DigitalPenManager(getApplication());

        // Enable the off-screen feature in Extended mode. We first create a
        // logical display in the off-screen area, and then pair the
        // off-screen presentation with it.
        // Its layout is defined in offscreen_virtual_layout, and inflated
        // to the OffscreenPresentation object. The off-screen views are fitted
        // with touch and hovering listeners which in turn give feedback to the
        // user through on-screen views.
        mDigitalPenManager.getSettings()
                .setOffScreenMode(OffScreenMode.EXTEND)
                .setOffScreenHoverEnabled(MAX_HOVER_DISTANCE)
                .apply();

        LinearLayout drawingPane = (LinearLayout)findViewById(R.id.offscreen_drawing_pane);

        // Change the layout's height to the screen's proportions
        LayoutParams params = drawingPane.getLayoutParams();
        Display display = getWindowManager().getDefaultDisplay();

        Point point = new Point();
        display.getSize(point);
        params.height = point.y*3/4;

        drawingPane.setLayoutParams(params);

        mDrawingView = new DrawingView(this);
        drawingPane.addView(mDrawingView);

        // set erase button
        findViewById(R.id.buttonEraseCanvas).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mDrawingView.erase();
            }
        });
    }

    @Override
    public void onResume() {
        super.onResume();

        View circle = findViewById(R.id.offscreen_indicator_circle);

        mOffscreenPresentation = new OffscreenPresentation(this,
                mDigitalPenManager.getOffScreenDisplay(),
                mDigitalPenManager.getSettings().getOffScreenHoverMaxDistance(), circle,
                mDrawingView);
        mOffscreenPresentation.show();
    }

    @Override
    public void onPause() {
        mOffscreenPresentation.dismiss();
        super.onPause();
    }

}
