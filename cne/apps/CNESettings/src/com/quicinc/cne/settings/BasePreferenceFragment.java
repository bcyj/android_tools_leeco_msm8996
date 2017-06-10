/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.quicinc.cne.settings;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import com.quicinc.cne.settings.controllers.BaseController;

public abstract class BasePreferenceFragment extends PreferenceFragment {

    protected BaseController mController;

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        Activity activity = getActivity();
        PreferenceScreen preferencesScreen = getPreferenceScreen();
        mController = getController(activity, preferencesScreen);
        super.onActivityCreated(savedInstanceState);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Load the preferences from an XML resource
        addPreferencesFromResource(getPreferencesResId());
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mController != null) {
            mController.onPause();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mController != null) {
            mController.onResume();
        }
    }

    protected abstract int getPreferencesResId();
    protected abstract BaseController getController(Context context,
                                                    PreferenceScreen preferencesScreen);
}