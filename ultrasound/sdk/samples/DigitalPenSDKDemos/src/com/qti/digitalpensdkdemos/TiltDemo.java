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
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;

public class TiltDemo extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(new TiltView(this));
    }

    private class TiltView extends View {
        private final int MAX_SHADOW_LEN = 500;

        private Paint mPaint;

        /* Pen's tip location */
        private float mX;

        private float mY;

        /*
         * The on-screen projection of the direction towards the back of the pen
         * is pointing. [-Pi, +Pi], 0 is up
         */
        private float mOrientation;

        /*
         * Tilt of the pen relative to the perpendicular line to the screen.
         * [0,Pi/2], 0 is when the pen is held perpendicular to the screen.
         */
        private float mTilt;

        public TiltView(Context context) {
            super(context);
            mPaint = new Paint();
            mPaint.setAntiAlias(true);
            mPaint.setDither(true);
            mPaint.setStyle(Paint.Style.STROKE);
            mPaint.setStrokeJoin(Paint.Join.ROUND);
            mPaint.setStrokeCap(Paint.Cap.ROUND);
            mPaint.setStrokeWidth(10);
        }

        @Override
        public void onDraw(Canvas canvas) {
            float endX = (float)(mX - MAX_SHADOW_LEN * Math.sin(mTilt) * Math.sin(mOrientation));
            float endY = (float)(mY + MAX_SHADOW_LEN * Math.sin(mTilt) * Math.cos(mOrientation));

            mPaint.setColor(Color.RED);
            canvas.drawLine(mX, mY, endX, endY, mPaint);

            mPaint.setColor(Color.BLACK);
            canvas.drawCircle(mX, mY, 10, mPaint);

            mPaint.setColor(Color.GREEN);
            canvas.drawLine(mX, mY, endX, mY, mPaint);
            canvas.drawLine(mX, mY, mX, endY, mPaint);
        }

        @Override
        public boolean onHoverEvent(MotionEvent event) {
            return onMotionEvent(event);
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            return onMotionEvent(event);
        }

        private boolean onMotionEvent(MotionEvent event) {
            mX = event.getX();
            mY = event.getY();
            mOrientation = event.getOrientation();
            mTilt = event.getAxisValue(MotionEvent.AXIS_TILT);
            invalidate();
            return true;
        }
    }

}
