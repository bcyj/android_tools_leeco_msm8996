/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.gesture;

import android.content.Context;
import android.gesture.Gesture;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.RectF;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.View;

public class GestureStrokeView extends View {

    private Gesture mGesture;
    private final Paint mPaint;
    private static final long ANIM_INTERVAL_TIME = 250;

    public static final float ANIM_CYCLE_SLOW = 0.05f;
    public static final float ANIM_CYCLE_FAST = 0.1f;

    public static final int STYLE_DEFAULT = 0;
    public static final int STYLE_FULL_SCREEN = 1;

    private float mAnimCycle = 0;
    private float mStrokeWidth = 5.0f;
    private int mStrokeColor = Color.GREEN;
    private float mCycleSpeed = ANIM_CYCLE_SLOW;
    private boolean mRepeat = true;
    private int mStyle = STYLE_DEFAULT;
    private boolean mEnable = true;

    public static interface GestureListener {

        void onGestureFinish(Gesture gesture);

    }

    private GestureListener mGestureListener;

    private static final int EVENT_ANIM = 1;

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_ANIM:
                    removeMessages(EVENT_ANIM);
                    if (playFrame()) {
                        sendMessageDelayed(obtainMessage(EVENT_ANIM), ANIM_INTERVAL_TIME);
                    }
                    break;
            }
        }

    };

    public GestureStrokeView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mPaint = new Paint();
        mPaint.setAntiAlias(true);
        mPaint.setDither(true);
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setStrokeJoin(Paint.Join.ROUND);
        mPaint.setStrokeCap(Paint.Cap.ROUND);
        mPaint.setStrokeWidth(mStrokeWidth);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (mGesture == null) {
            return;
        }
        mPaint.setColor(mEnable ? mStrokeColor : Color.DKGRAY);
        Path path = Utils.getGesturePath(mGesture,
                (int) (Utils.getGestureLength(mGesture) * (mEnable ? mAnimCycle : 1)));

        if (mStyle == STYLE_DEFAULT) {

            final RectF bounds = new RectF();
            mGesture.toPath().computeBounds(bounds, true);
            final float sx = (getWidth() - mStrokeWidth) / bounds.width();
            final float sy = (getHeight() - mStrokeWidth) / bounds.height();
            final float scale = sx > sy ? sy : sx;
            mPaint.setStrokeWidth(mStrokeWidth / scale);
            path.offset(-bounds.left + (getWidth() - bounds.width() * scale) / 2, -bounds.top
                    + (getHeight() - bounds.height() * scale) / 2);
            canvas.translate(mStrokeWidth / 2, mStrokeWidth / 2);
            canvas.scale(scale, scale);
        }

        canvas.drawPath(path, mPaint);
    }

    public void setGesture(Gesture gesture) {
        mGesture = gesture;
        startAnimation();
    }

    @Override
    public void setEnabled(boolean enable) {
        if (enable == mEnable)
            return;
        mEnable = enable;
        startAnimation();
    }

    public void setStyle(int style) {
        mStyle = style;
        startAnimation();
    }

    public void setStrokeWith(float strokeWidth) {
        mStrokeWidth = strokeWidth;
        startAnimation();
    }

    public void setStrokeColor(int strokeColor) {
        mStrokeColor = strokeColor;
        startAnimation();
    }

    public void setCycleSpeed(float cycleSpeed) {
        mCycleSpeed = cycleSpeed;
    }

    public void setRepeat(boolean repeat) {
        mRepeat = repeat;
    }

    public void setGestureListener(GestureListener gestureListener) {
        mGestureListener = gestureListener;
    }

    private boolean playFrame() {
        invalidate();
        if (mGesture != null && mEnable) {
            if (mAnimCycle >= 1) {
                mAnimCycle = 0;
                if (!mRepeat) {
                    if (mGestureListener != null) {
                        mGestureListener.onGestureFinish(mGesture);
                    }
                    return false;
                }
            } else {
                mAnimCycle += mCycleSpeed;
            }
            return true;
        }
        return false;
    }

    private void startAnimation() {
        mHandler.removeMessages(EVENT_ANIM);
        mHandler.sendMessage(mHandler.obtainMessage(EVENT_ANIM));
    }
}
