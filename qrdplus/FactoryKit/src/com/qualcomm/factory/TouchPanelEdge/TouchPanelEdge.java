/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.TouchPanelEdge;

import java.util.ArrayList;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import com.qualcomm.factory.Utilities;

public class TouchPanelEdge extends Activity {

    String TAG = "TouchPanelEdge";
    ArrayList<EdgePoint> mArrayList;
    String resultString = "Failed";
    int mHightPix = 0, mWidthPix = 0, mRadius = 20, mStep = 0;
    float w = 0, h = 0;
    Context mContext;
    // If points is too more, it will be hard to touch edge points.
    private final int X_MAX_POINTS = 16;

    public class EdgePoint {

        int x;
        int y;
        boolean isChecked = false;

        public EdgePoint(int x, int y, boolean isCheck) {

            this.x = x;
            this.y = y;
            this.isChecked = isCheck;
        }

    }
    
    int getStep(int hightPix, int widthPix) {
        
        int MIN_STEP = widthPix / X_MAX_POINTS;
        int step = MIN_STEP;
        for (int i = MIN_STEP; i < widthPix / 5; i++) {
            if (hightPix % i == 0 && widthPix % i == 0)
                return i;
        }
        
        return step;
    }

    public ArrayList<EdgePoint> getTestPoint() {

        ArrayList<EdgePoint> list = new ArrayList<EdgePoint>();

        for (int x = mRadius; x < mWidthPix + mRadius; x += mStep) {
            for (int y = mRadius; y < mHightPix + mRadius; y += mStep) {
                if (x > mRadius && x < mWidthPix - mRadius && y > mRadius && y < mHightPix - mRadius)
                    continue;
                list.add(new EdgePoint(x, y, false));
            }
        }

        return list;

    }

    @Override
    public void finish() {

        Utilities.writeCurMessage(this, TAG, resultString);
        super.finish();
    }

    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        init(this);

        // full screen
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        // get panel size
        DisplayMetrics mDisplayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(mDisplayMetrics);
        mHightPix = mDisplayMetrics.heightPixels;
        mWidthPix = mDisplayMetrics.widthPixels;
        // It must be common divisor of width and hight
        mStep = getStep(mWidthPix, mHightPix);
        mRadius = mStep / 2;
        logd(mHightPix + "x" + mWidthPix + " Step:" + mStep);
        setContentView(new Panel(this));
    }

    private void init(Context context) {

        mContext = context;
        resultString = "Failed";
    }

    class Panel extends View {

        public static final int TOUCH_TRACE_NUM = 30;
        public static final int PRESSURE = 500;
        private TouchData[] mTouchData = new TouchData[TOUCH_TRACE_NUM];
        private int traceCounter = 0;
        private Paint mPaint = new Paint();

        public class TouchData {

            public float x;
            public float y;
            public float r;
        }

        public Panel(Context context) {

            super(context);
            mArrayList = getTestPoint();
            mPaint.setARGB(100, 100, 100, 100);
            for (int i = 0; i < TOUCH_TRACE_NUM; i++) {
                mTouchData[i] = new TouchData();
            }

        }

        private int getNext(int c) {

            int temp = c + 1;
            return temp < TOUCH_TRACE_NUM ? temp : 0;
        }

        public void onDraw(Canvas canvas) {

            super.onDraw(canvas);
            mPaint.setColor(Color.LTGRAY);
            mPaint.setTextSize(20);
            canvas.drawText("W: " + w, mWidthPix / 2 - 20, mHightPix / 2 - 10, mPaint);
            canvas.drawText("H: " + h, mWidthPix / 2 - 20, mHightPix / 2 + 10, mPaint);

            mPaint.setColor(Color.RED);
            mPaint.setStrokeWidth(mRadius);
            for (int i = 0; i < mArrayList.size(); i++) {
                EdgePoint point = mArrayList.get(i);
                mPaint.setColor(Color.RED);
                canvas.drawCircle(point.x, point.y, mPaint.getStrokeWidth(), mPaint);

            }

            for (int i = 0; i < TOUCH_TRACE_NUM; i++) {
                TouchData td = mTouchData[i];
                mPaint.setColor(Color.BLUE);
                if (td.r > 0) {
                    canvas.drawCircle(td.x, td.y, 2, mPaint);
                }

            }
            invalidate();
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {

            final int eventAction = event.getAction();

            w = event.getRawX();
            h = event.getRawY();
            if ((eventAction == MotionEvent.ACTION_MOVE) || (eventAction == MotionEvent.ACTION_UP)) {
                for (int i = 0; i < mArrayList.size(); i++) {
                    EdgePoint point = mArrayList.get(i);
                    if (!point.isChecked && ((w >= (point.x - mRadius)) && (w <= (point.x + mRadius)))
                            && ((h >= (point.y - mRadius)) && (h <= (point.y + mRadius)))) {
                        mArrayList.remove(i);
                        break;
                    }

                }

                if (mArrayList.isEmpty()) {
                    ((Activity) mContext).setResult(RESULT_OK);
                    resultString = Utilities.RESULT_PASS;
                    finish();
                }

                TouchData tData = mTouchData[traceCounter];
                tData.x = event.getX();
                tData.y = event.getY();
                tData.r = event.getPressure() * PRESSURE;
                traceCounter = getNext(traceCounter);
                invalidate();

            }
            return true;
        }

    }

    void logd(Object d) {

        Log.d(TAG, "" + d);
    }

    void loge(Object e) {

        Log.e(TAG, "" + e);
    }
}
