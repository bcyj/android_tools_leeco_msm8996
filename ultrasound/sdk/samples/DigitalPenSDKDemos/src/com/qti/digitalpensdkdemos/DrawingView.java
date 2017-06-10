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

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.view.MotionEvent;
import android.view.View;

import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.SideChannelData;

/*
 * This utility class is used to show off-screen drawing action in an on-screen
 * view.
 *
 * It expects to either receive MotionEvents from the extended display
 * (onExternalTouchEvent) or side-channel data (onExternalSideChannelData)
 */
class DrawingView extends View {

    private static final int BACKGROUND_COLOR = Color.GRAY;

    private final Path mPath;

    private final Paint mPaint;

    private Bitmap mBitmap;

    private Canvas mCanvas;

    private final Paint mBitmapPaint;

    private boolean mLastSideChannelDataIsDown;

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
        mPaint.setStrokeWidth(5);

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

    private void handleAction(float x, float y, int action) {
        switch (action) {
            case MotionEvent.ACTION_DOWN:
                touch_start(x, y);
                break;
            case MotionEvent.ACTION_MOVE:
                touch_move(x, y);
                break;
            case MotionEvent.ACTION_UP:
                touch_up(x, y);
                break;
            default:
                return;
        }
    }

    public void onExternalTouchEvent(MotionEvent event) {
        float x = event.getX();
        float y = event.getY();

        int action = event.getAction();
        handleAction(x, y, action);
        invalidate(); // expect this to be called from the UI thread
    }

    public void onExternalSideChannelData(SideChannelData data) {
        float x = scaledToPhysicalPosition(data.xPos, getWidth());
        float y = scaledToPhysicalPosition(data.yPos, getHeight());

        int action = translateIsDownStateToAction(data);
        handleAction(x, y, action);
        postInvalidate(); // expect this to be called from a non-UI thread

        mLastSideChannelDataIsDown = data.isDown;
    }

    private int translateIsDownStateToAction(SideChannelData data) {
        // code word: bit 1 is last "isDown", bit 0 is current "isDown"
        final int code = (data.isDown ? 1 : 0) +
                (mLastSideChannelDataIsDown ? 2 : 0);
        final int[] TRANSITION_TABLE = {
                MotionEvent.ACTION_HOVER_MOVE, // up -> up
                MotionEvent.ACTION_DOWN, // up -> down
                MotionEvent.ACTION_UP, // down -> up
                MotionEvent.ACTION_MOVE, // down -> down
        };
        return TRANSITION_TABLE[code];
    }

    private float scaledToPhysicalPosition(int pos, int maxScreenDimension) {
        float ratio = ((float) pos) / SideChannelData.MAX_LOGICAL_UNIT;
        return ratio * maxScreenDimension;
    }

    public void erase() {
        mCanvas.drawColor(BACKGROUND_COLOR);
        invalidate();
    }
}
