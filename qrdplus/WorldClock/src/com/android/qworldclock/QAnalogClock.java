/*
 * Copyright (c) 2011, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 */
package com.android.qworldclock;

import java.util.Calendar;
import java.util.TimeZone;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.drawable.BitmapDrawable;

import android.util.AttributeSet;
// import android.util.Log;
import android.view.View;

public class QAnalogClock extends View implements TickListener {
    private static final String TAG = "QAnalogClock";

    private TimeZone mTimeZone;

    private Bitmap mDialog;
    private BitmapDrawable mDialogDrawable;
    private int mDialogWidth;
    private int mDialogHeight;
    private int mCenterX;
    private int mCenterY;

    private Bitmap mHour;
    private BitmapDrawable mHourDrawable;
    private int mHourWidth;  // Only the upper half picture is useful
    private int mHourHeight;

    private Bitmap mMinute;
    private BitmapDrawable mMinuteDrawable;
    private int mMinuteWidth;  // Only the upper half picture is useful
    private int mMinuteHeight;

    private Bitmap mSecond;
    private BitmapDrawable mSecondDrawable;
    private int mSecondWidth;  // Only the upper half picture is useful
    private int mSecondHeight;

    Paint mPaint;

    /**
     * Adjust the size of the view by given parameter
     * @param width new width of the view
     * @param height new height of the view
     * @return
     */
    public boolean setAvailableSize(int width, int height) {
        mHourWidth = mHourWidth * width / mDialogWidth;
        mHourHeight = mHourHeight * height / mDialogHeight;
        mMinuteWidth = mMinuteWidth * width / mDialogWidth;
        mMinuteHeight = mMinuteHeight * height / mDialogHeight;
        mSecondWidth = mSecondWidth * width / mDialogWidth;
        mSecondHeight = mSecondHeight * height / mDialogHeight;
        mCenterX = mCenterX * width / mDialogWidth;
        mCenterY = mCenterY * height / mDialogHeight;
        mDialogWidth = width;
        mDialogHeight = height;
        return true;
    }

    public QAnalogClock(Context context) {
        this(context, null, TimeZone.getDefault().getID());
    }

    public QAnalogClock(Context context, AttributeSet attr) {
        this(context, attr, TimeZone.getDefault().getID());
    }

    public QAnalogClock(Context context, String timezone) {
        this(context, null, timezone);
    }

    public QAnalogClock(Context context, AttributeSet attr, String timezone) {
        super(context, attr);
        mTimeZone = TimeZone.getTimeZone(timezone);

        // Initialize the resource
        mDialog = BitmapFactory.decodeResource(getResources(), R.drawable.clock_dial_current);
        mDialogDrawable = new BitmapDrawable(mDialog);
        mDialogWidth = mDialog.getWidth();
        mDialogHeight = mDialog.getHeight();

        mHour = BitmapFactory.decodeResource(getResources(), R.drawable.clock_current_hour);
        mHourDrawable = new BitmapDrawable(mHour);
        mHourWidth = mHourDrawable.getIntrinsicWidth() >> 1;
        mHourHeight = mHourDrawable.getIntrinsicHeight() >> 1;

        mMinute = BitmapFactory.decodeResource(getResources(), R.drawable.clock_current_minute);
        mMinuteDrawable = new BitmapDrawable(mMinute);
        mMinuteWidth = mMinuteDrawable.getIntrinsicWidth() >> 1;
        mMinuteHeight = mMinuteDrawable.getIntrinsicHeight() >> 1;

        mSecond = BitmapFactory.decodeResource(getResources(), R.drawable.clock_current_second);
        mSecondDrawable = new BitmapDrawable(mSecond);
        mSecondWidth = mSecondDrawable.getIntrinsicWidth() >> 1;
        mSecondHeight = mSecondDrawable.getIntrinsicHeight() >> 1;

        mPaint = new Paint();

        mCenterX = mDialogWidth >> 1;
        mCenterY = mDialogHeight >> 1;

        // The current size is a little too big
        setAvailableSize(mDialogWidth >> 1, mDialogHeight >> 1);
    }

    @Override
    protected void onAttachedToWindow() {
        // Log.d(TAG, "onAttachedToWindow");
        register();
        updateTime();
    }

    @Override
    protected void onDetachedFromWindow() {
        // Log.d(TAG, "onDetachedFromWindow");
        unregister();
    }

    public void setTimeZone(String timezone) {
        mTimeZone = TimeZone.getTimeZone(timezone);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        setMeasuredDimension(mDialogWidth, mDialogHeight);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        Calendar calendar = Calendar.getInstance(mTimeZone);
        int hour = calendar.get(Calendar.HOUR);
        int minute = calendar.get(Calendar.MINUTE);
        int second = calendar.get(Calendar.SECOND);

        float hourRotate =  hour * 30.0f + minute / 2.0f;   // neglect the effect of second
        float minuteRotate = minute * 6.0f + second / 10.0f;
        float secondRotate = second * 6.0f;
        mDialogDrawable.setBounds(mCenterX - (mDialogWidth >> 1), mCenterY - (mDialogHeight >> 1),
                mCenterX + (mDialogWidth >> 1), mCenterY + (mDialogHeight >> 1));
        mDialogDrawable.draw(canvas);

        canvas.save();
        canvas.rotate(hourRotate, mCenterX, mCenterY);
        mHourDrawable.setBounds(mCenterX - mHourWidth, mCenterY - mHourHeight,
                mCenterX + mHourWidth, mCenterY + mHourHeight);
        mHourDrawable.draw(canvas);
        canvas.restore();

        canvas.save();
        canvas.rotate(minuteRotate, mCenterX, mCenterY);
        mMinuteDrawable.setBounds(mCenterX - mMinuteWidth, mCenterY - mMinuteHeight,
                mCenterX + mMinuteWidth, mCenterY + mMinuteHeight);
        mMinuteDrawable.draw(canvas);
        canvas.restore();

        canvas.save();
        canvas.rotate(secondRotate, mCenterX, mCenterY);
        mSecondDrawable.setBounds(mCenterX - mSecondWidth, mCenterY - mSecondHeight,
                mCenterX + mSecondWidth, mCenterY + mSecondHeight);
        mSecondDrawable.draw(canvas);
        canvas.restore();
    }

    public boolean register() {
        return TickBroadCastor.getInstance().addListener(this);
    }

    public boolean unregister() {
        return TickBroadCastor.getInstance().removeListener(this);
    }

    public void updateTime() {
        invalidate();
    }
}
