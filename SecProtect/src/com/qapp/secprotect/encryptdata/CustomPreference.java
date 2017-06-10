/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.encryptdata;

import android.content.Context;
import android.preference.Preference;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.qapp.secprotect.R;
import com.qapp.secprotect.utils.UtilsLog;

public class CustomPreference extends Preference {

    private Context mContext;

    public CustomPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mContext = context;
    }

    public CustomPreference(Context context, AttributeSet attrs) {
        this(context, attrs, 0);

    }

    public CustomPreference(Context context) {
        super(context, null);
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        UtilsLog.logd("");
    }

    @Override
    protected View onCreateView(ViewGroup parent) {
        UtilsLog.logd("");

        LayoutInflater mLayoutInflater = LayoutInflater.from(mContext);
        View view = mLayoutInflater.inflate(R.layout.custom_preference, null);
        return view;

    }
}