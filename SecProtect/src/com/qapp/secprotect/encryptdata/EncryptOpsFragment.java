/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.encryptdata;

import static com.qapp.secprotect.utils.UtilsLog.logd;
import android.app.Activity;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;

import com.qapp.secprotect.Configs;
import com.qapp.secprotect.R;

public class EncryptOpsFragment extends PreferenceFragment {

    OnOperationClickListener mOnOperationClickListener;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        logd("");
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.encryptdata_operation);
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        mOnOperationClickListener = (OnOperationClickListener) activity;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        String key = preference.getKey();
        logd(key);
        if ("encrypt".equals(key))
            mOnOperationClickListener.onOperationClick(Configs.MODE_ENCRYPT);
        else if ("decrypt".equals(key))
            mOnOperationClickListener.onOperationClick(Configs.MODE_DECRYPT);
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    public interface OnOperationClickListener {

        public void onOperationClick(int mode);
    }
}
