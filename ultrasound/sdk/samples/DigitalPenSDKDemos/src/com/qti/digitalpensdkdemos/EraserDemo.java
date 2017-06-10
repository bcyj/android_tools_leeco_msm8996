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
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.LinearLayout;
import android.widget.Switch;

import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.Settings;

public class EraserDemo extends Activity {

    private static final int MAX_HOVER_DISTANCE = 150;

    /* The digital pen manager object */
    private DigitalPenManager mDigitalPenManager = null;

    private DrawingView mDrawingView = null;

    private Switch mEraserBypassSwitch = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_eraser_demo);

        findViewById(R.id.buttonEraseCanvas).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mDrawingView.erase();
            }
        });

        mDigitalPenManager = new DigitalPenManager(getApplication());
        // Enables the digital pen's on-screen hover events up to
        // MAX_HOVER_DISTANCE (in 1mm units)
        mDigitalPenManager.getSettings()
        .setOnScreenHoverEnabled(MAX_HOVER_DISTANCE)
        .apply();

        mEraserBypassSwitch = ((Switch)findViewById(R.id.eraser_switch));

        setupEraserBypassSwitch();

        mEraserBypassSwitch.setChecked(false);

        LinearLayout drawingPane = (LinearLayout)findViewById(R.id.eraser_drawing_pane);
        mDrawingView = new DrawingView(this);
        drawingPane.addView(mDrawingView);
    }

    private void setupEraserBypassSwitch() {
        mEraserBypassSwitch.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                Settings settings = mDigitalPenManager.getSettings();
                if (isChecked) {
                    // Set the eraser bypass feature on. Side button presses
                    // will cause the hover/touch events to be sent with stylus
                    // tool type.
                    settings.setEraserBypass();
                } else {
                    // Disable the eraser bypass feature. If the global setting
                    // for eraser is enabled, hover/touch events from the pen
                    // will be sent with the eraser tool type (depending on the
                    // eraser mode - toggle/hold_to_erase)
                    settings.setEraserBypassDisabled();
                }
                settings.apply();
            }
        });
    }

    private class DrawingView extends View {

        private static final int BACKGROUND_COLOR = Color.GRAY;

        private int mCurrentToolType;

        private int mCurrentColor;

        private final Path mPath;

        private final Paint mPaint;

        private Bitmap mBitmap;

        private Canvas mCanvas;

        private final Paint mBitmapPaint;

        public DrawingView(Context c) {
            super(c);

            mPath = new Path();

            mPaint = new Paint();
            mPaint.setAntiAlias(true);
            mPaint.setDither(true);
            mPaint.setColor(Color.RED);
            mPaint.setStyle(Paint.Style.STROKE);
            mPaint.setStrokeJoin(Paint.Join.ROUND);
            mPaint.setStrokeCap(Paint.Cap.ROUND);
            mPaint.setStrokeWidth(10);

            mBitmapPaint = new Paint(Paint.DITHER_FLAG);

            mCurrentToolType = MotionEvent.TOOL_TYPE_STYLUS;
            mCurrentColor = Color.RED;
        }

        @Override
        protected void onSizeChanged(int w, int h, int oldw, int oldh) {
            super.onSizeChanged(w, h, oldw, oldh);
            mBitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
            mCanvas = new Canvas(mBitmap);
        }

        @Override
        protected void onDraw(Canvas canvas) {
            canvas.drawColor(BACKGROUND_COLOR);

            canvas.drawBitmap(mBitmap, 0, 0, mBitmapPaint);

            canvas.drawPath(mPath, mPaint);
        }

        private void touch_start(float x, float y) {
            mPath.reset();
            mPath.moveTo(x, y);
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            float x = event.getX();
            float y = event.getY();
            int toolType = event.getToolType(0);

            // Tool type not relevant
            if (toolType != MotionEvent.TOOL_TYPE_STYLUS && toolType != MotionEvent.TOOL_TYPE_ERASER){
                return true;
            }

            // Changed from stylus to eraser or vice versa
            if (toolType != mCurrentToolType){
                mCurrentToolType = toolType;
                mCurrentColor = toolType == MotionEvent.TOOL_TYPE_STYLUS ? Color.RED : BACKGROUND_COLOR;

                // Draw path, unless pen was up.
                if (event.getAction() != MotionEvent.ACTION_DOWN){
                    mCanvas.drawPath(mPath, mPaint);
                }
                touch_start(x,y);
                mPaint.setColor(mCurrentColor);
            }
            else{
                switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    touch_start(x,y);
                    break;
                case MotionEvent.ACTION_MOVE:
                    mPath.lineTo(x, y);
                    break;
                case MotionEvent.ACTION_UP:
                    mCanvas.drawPath(mPath, mPaint);
                    break;
                }
            }
            invalidate();
            return true;
        }

        public void erase() {
            mPath.reset();
            mCanvas.drawColor(BACKGROUND_COLOR);
            invalidate();
        }
    }
}
