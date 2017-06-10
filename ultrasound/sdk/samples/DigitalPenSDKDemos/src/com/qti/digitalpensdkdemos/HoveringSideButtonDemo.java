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
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Switch;

import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.Settings;

public class HoveringSideButtonDemo extends Activity {

    private static final int MAX_HOVER_DISTANCE = 150;

    /* The digital pen manager object */
    private DigitalPenManager mDigitalPenManager = null;

    private DrawingView mDrawingView = null;

    private Switch mHoveringSwitch = null;

    private SeekBar mHoveringRange = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_hovering_side_button_demo);

        mDigitalPenManager = new DigitalPenManager(getApplication());

        mHoveringSwitch = ((Switch)findViewById(R.id.hovering_switch));
        mHoveringRange = (SeekBar)findViewById(R.id.hovering_range);

        setupHoveringRangeSeekBar();

        setupHoveringSwitch();

        mHoveringSwitch.setChecked(true);

        LinearLayout drawingPane = (LinearLayout)findViewById(R.id.hovering_drawing_pane);
        mDrawingView = new DrawingView(this);
        drawingPane.addView(mDrawingView);

        // set the erase button
        findViewById(R.id.buttonEraseCanvas).setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mDrawingView.erase();
            }
        });
    }

    private void setupHoveringSwitch() {
        mHoveringSwitch.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                Settings settings = mDigitalPenManager.getSettings();
                if (isChecked) {
                    settings.setOnScreenHoverEnabled(MAX_HOVER_DISTANCE);
                    mHoveringRange.setEnabled(true);
                    mHoveringRange.setProgress(MAX_HOVER_DISTANCE);
                } else {
                    settings.setOnScreenHoverDisabled();
                    mHoveringRange.setEnabled(false);
                }
                settings.apply();
            }
        });
    }

    private void setupHoveringRangeSeekBar() {
        mHoveringRange.setMax(MAX_HOVER_DISTANCE);
        mHoveringRange.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mDigitalPenManager.getSettings()
                        .setOnScreenHoverEnabled(seekBar.getProgress())
                        .apply();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            }
        });
    }

    private class DrawingView extends View {

        private static final int BACKGROUND_COLOR = Color.GRAY;

        private final Path mPath;

        private final Paint mPaint;

        private Bitmap mBitmap;

        private Canvas mCanvas;

        private final Paint mBitmapPaint;

        private float mXCircle;

        private float mYCircle;

        private float mRCircle;

        private final Paint mCirclePaint;

        private final float MAX_CIRCLE_RADIUS = 200;

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

            mCirclePaint = new Paint(mPaint);
            mCirclePaint.setColor(Color.GREEN);

            mBitmapPaint = new Paint(Paint.DITHER_FLAG);
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

            if (mHoveringSwitch.isChecked()) {
                canvas.drawCircle(mXCircle, mYCircle, mRCircle, mCirclePaint);
            }
        }

        private void touch_start(float x, float y) {
            mPath.reset();
            mPath.moveTo(x, y);
        }

        private void touch_move(float x, float y) {
            mPath.lineTo(x, y);
        }

        private void touch_up(float x, float y) {
            mPath.lineTo(x, y);
            // commit the path to our offscreen
            mCanvas.drawPath(mPath, mPaint);
            // kill this so we don't double draw
            mPath.reset();
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            float x = event.getX();
            float y = event.getY();

            mRCircle = 0f;

            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    touch_start(x, y);
                    invalidate();
                    break;
                case MotionEvent.ACTION_MOVE:
                    touch_move(x, y);
                    invalidate();
                    break;
                case MotionEvent.ACTION_UP:
                    touch_up(x, y);
                    invalidate();
                    break;
            }
            return true;
        }

        @Override
        public boolean onHoverEvent(MotionEvent event) {
            mXCircle = event.getX();
            mYCircle = event.getY();

            // The circle drawn around the hovering position is made larger as the pen
            // goes higher above the screen. We multiply the MAX_HOVER_DISTANCE by 100
            // to convert to the MotionEvent.AXIS_DISTANCE units. the command
            // event.getAxisValue(MotionEvent.AXIS_DISTANCE) gets the height of the pen.
            mRCircle = MAX_CIRCLE_RADIUS
                    * (event.getAxisValue(MotionEvent.AXIS_DISTANCE) / (MAX_HOVER_DISTANCE * 100));

            // event.getButtonState() returns a number, where each bit in its binary
            // representation stands for a state in a different button on the pen. The primary
            // button is the tip of the pen, and the secondary is the side-button. The circle
            // around the hover point changes color according to the state of the
            // secondary buttons.
            switch(event.getButtonState()){
                case MotionEvent.BUTTON_SECONDARY:
                    mCirclePaint.setColor(Color.WHITE);
                    break;
                case MotionEvent.BUTTON_TERTIARY:
                    mCirclePaint.setColor(Color.MAGENTA);
                    break;
                default:
                    mCirclePaint.setColor(Color.GREEN);
            }

            invalidate();
            return true;
        }

        public void erase() {
            mCanvas.drawColor(BACKGROUND_COLOR);
            invalidate();
        }
    }
}
