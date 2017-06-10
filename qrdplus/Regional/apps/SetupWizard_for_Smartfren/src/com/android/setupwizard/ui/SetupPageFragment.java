/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.setupwizard.ui;

import com.android.setupwizard.data.Page;
import com.android.setupwizard.data.SetupDataCallbacks;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

public abstract class SetupPageFragment extends Fragment {

    protected SetupDataCallbacks mCallbacks;
    protected String mKey;
    protected Page mPage;
    protected View mRootView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Bundle args = getArguments();
        mKey = args.getString(Page.KEY_PAGE_ARGUMENT);
        if (mKey == null) {
            throw new IllegalArgumentException("No KEY_PAGE_ARGUMENT given");
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        mRootView = inflater.inflate(getLayoutResource(), container, false);
        TextView titleView = (TextView) mRootView.findViewById(android.R.id.title);
        titleView.setText(getTitleResource());
        return mRootView;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mPage = mCallbacks.getPage(mKey);
        setUpPage();
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        if (!(activity instanceof SetupDataCallbacks)) {
            throw new ClassCastException("Activity implement SetupDataCallbacks");
        }
        mCallbacks = (SetupDataCallbacks) activity;
    }

    @Override
    public void onDetach() {
        super.onDetach();
        mCallbacks = null;
    }

    protected abstract void setUpPage();
    protected abstract int getLayoutResource();
    protected abstract int getTitleResource();
}
