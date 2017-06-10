/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/
package com.qti.ultrasound.penpairing.customviews;

import com.qti.ultrasound.penpairing.R;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Typeface;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

@SuppressLint("ViewConstructor")
public class BarChartView extends LinearLayout {
    // List of individual bar views
    private List<BarView> mBars = new ArrayList<BarView>();

    private final float SCALE_TEXT_SIZE = 20f;

    private final float BAR_TITLE_TEXT_SIZE = 25f;

    private final float CHART_TITLE_TEXT_SIZE = 40f;

    private final BarConstants[] mBarsConstants;

    private final long MIN_MAX_PERIOD_MSEC = 1000;

    public BarChartView(Context context, BarConstants[] barsConstants, String title) {
        super(context);

        mBarsConstants = barsConstants;

        for (BarConstants barConstants : mBarsConstants) {
            BarView bv = new BarView(getContext(), barConstants);
            mBars.add(bv);
        }

        setOrientation(VERTICAL);

        LinearLayout ll = new LinearLayout(getContext());
        ll.setOrientation(HORIZONTAL);

        addView(ll, new LayoutParams(android.view.ViewGroup.LayoutParams.MATCH_PARENT,
                android.view.ViewGroup.LayoutParams.WRAP_CONTENT, 1f));

        TextView titleView = new TextView(getContext());
        titleView.setText(title);
        titleView.setTextSize(CHART_TITLE_TEXT_SIZE);
        titleView.setGravity(Gravity.CENTER);
        titleView.setTextColor(Color.BLACK);

        addView(titleView, new LayoutParams(android.view.ViewGroup.LayoutParams.MATCH_PARENT,
                android.view.ViewGroup.LayoutParams.WRAP_CONTENT));

        LayoutParams bar_lp = new LayoutParams(android.view.ViewGroup.LayoutParams.WRAP_CONTENT,
                android.view.ViewGroup.LayoutParams.MATCH_PARENT, 1f);
        for (BarView bv : mBars) {
            ll.addView(bv, bar_lp);
        }

    }

    public void postNewChartData(Float[] data) {
        if (data.length != mBars.size()) {
            Log.e(this.toString(),
                    "Submitted data length not equal to the number of bars in the chart");
            return;
        }

        for (int i = 0; i < data.length; ++i) {
            mBars.get(i).postNewData(data[i]);
        }
    }

    public static class BarConstants {
        // Title of the bar
        public String mTitle;

        // Threshold for "good" values
        public float mThreshold;

        // Minimum value presented by the bar
        public float mScaleMin;

        // Maximum value presented by the bar
        public float mScaleMax;

        public BarConstants(String title) {
            mTitle = title;
            mThreshold = 0;
            mScaleMin = 0;
            mScaleMax = 0;
        }

        public BarConstants(String title, float threshold, float scaleMin, float scaleMax) {
            mTitle = title;
            mThreshold = threshold;
            mScaleMin = scaleMin;
            mScaleMax = scaleMax;
        }
    }

    /**
     * Internal child class that represents a single bar (mic) including all
     * titles related to it.
     */
    private class BarView extends LinearLayout {
        // Minimum value during last second
        private float mDataMin = Float.MAX_VALUE;

        // Maximum value during last second
        private float mDataMax = -Float.MAX_VALUE;

        private float mLastValue = -Float.MAX_VALUE;

        private long lastUpdatedMinMax = System.currentTimeMillis();

        private final BarConstants mBarConstants;

        public BarView(Context context, BarConstants barConstants) {
            super(context);

            mBarConstants = barConstants;

            LayoutInflater layoutInflater = (LayoutInflater)context
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View root = layoutInflater.inflate(R.layout.bar_layout, this);

            TextView barTitle = (TextView)root.findViewById(R.id.bar_title);
            barTitle.setTextColor(Color.BLACK);
            barTitle.setTextSize(BAR_TITLE_TEXT_SIZE);
            barTitle.setTypeface(Typeface.create((String)null, Typeface.BOLD));
            barTitle.setText(barConstants.mTitle);

            TextView threshold = (TextView)root.findViewById(R.id.threshold);
            threshold.setTextColor(Color.BLACK);
            threshold.setTextSize(SCALE_TEXT_SIZE);
            threshold.setText(String.valueOf(barConstants.mThreshold));

            TextView minScale = (TextView)root.findViewById(R.id.min_scale);
            minScale.setTextColor(Color.BLACK);
            minScale.setTextSize(SCALE_TEXT_SIZE);
            minScale.setText(String.valueOf(barConstants.mScaleMin));

            TextView maxScale = (TextView)root.findViewById(R.id.max_scale);
            maxScale.setTextColor(Color.BLACK);
            maxScale.setTextSize(SCALE_TEXT_SIZE);
            maxScale.setText(String.valueOf(barConstants.mScaleMax));
        }

        public void postNewData(float value) {
            if (value < mBarConstants.mScaleMin || value > mBarConstants.mScaleMax) {
                return;
            }

            long curTime = System.currentTimeMillis();
            int containerHeight = findViewById(R.id.value_bar_container).getHeight();

            // This logic updates the minimum and maximum values
            // which have been received over the last "MIN_MAX_PERIOD_MSEC".
            // The min/max values get updated in case a period has passed since the
            // last update, or in case a new value is larger(/smaller) than the current
            // maximum(/minimum).
            if (curTime > lastUpdatedMinMax + MIN_MAX_PERIOD_MSEC) {
                mDataMin = mDataMax = value;
                lastUpdatedMinMax = System.currentTimeMillis();
            } else if (value < mDataMin) {
                mDataMin = value;
                updateMinMaxValueView(R.id.min_in_period, containerHeight, value);
            } else if (value > mDataMax) {
                mDataMax = value;
                updateMinMaxValueView(R.id.max_in_period, containerHeight, value);
            }

            mLastValue = value;
            View bar = findViewById(R.id.value_bar);
            if (value < mBarConstants.mThreshold) {
                bar.setBackgroundColor(Color.RED);
            } else {
                bar.setBackgroundColor(Color.GREEN);
            }

            RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams)findViewById(
                    R.id.value_bar).getLayoutParams();
            params.height = (int)(containerHeight * (mLastValue - mBarConstants.mScaleMin) / (mBarConstants.mScaleMax - mBarConstants.mScaleMin));
            findViewById(R.id.value_bar).setLayoutParams(params);

        }

        private void updateMinMaxValueView(int viewId, int containerHeight, float value) {
            RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams)findViewById(viewId).getLayoutParams();
            params.setMargins(0,
                              0,
                              0,
                              (int)(containerHeight * (value - mBarConstants.mScaleMin) / (mBarConstants.mScaleMax - mBarConstants.mScaleMin)));
            findViewById(viewId).setLayoutParams(params);
        }

        @Override
        protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
            TextView threshold = (TextView)findViewById(R.id.threshold);
            RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams)threshold
                    .getLayoutParams();
            int height = findViewById(R.id.value_bar_container).getHeight();
            params.setMargins(
                    0,
                    0,
                    0,
                    (int)(height * (mBarConstants.mThreshold - mBarConstants.mScaleMin) / (mBarConstants.mScaleMax - mBarConstants.mScaleMin)));

            threshold.setLayoutParams(params);
        }
    }

}
