/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.factory.PSensor;

import android.content.Context;
import android.util.AttributeSet;
import android.view.Gravity;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.qualcomm.factory.R;

public class ImageTextView extends LinearLayout {

    ImageView mImageView;
    TextView mTextView;

    public ImageTextView(Context context, AttributeSet attrs) {

        super(context, attrs);
        setOrientation(LinearLayout.HORIZONTAL);
        setGravity(Gravity.CENTER);

        mImageView = new ImageView(context);
        mImageView.setImageResource(R.drawable.off);
        mImageView.setPadding(10, 10, 20, 10);
        addView(mImageView, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);

        mTextView = new TextView(context);
        mTextView.setText(context.getString(R.string.psensor_uncovered));
        addView(mTextView, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
    }

    public ImageTextView(Context context) {

        super(context);
        setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
        setOrientation(LinearLayout.HORIZONTAL);

        mImageView = new ImageView(context);
        mImageView.setImageResource(R.drawable.off);
        mImageView.setPadding(10, 10, 20, 10);
        addView(mImageView, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);

        mTextView = new TextView(context);
        mTextView.setText(context.getString(R.string.psensor_uncovered));
        addView(mTextView, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
    }

    public void setText(CharSequence text) {

        mTextView.setText(text);
    }

    public void setImageResource(int resId) {

        mImageView.setImageResource(resId);
    }

}
