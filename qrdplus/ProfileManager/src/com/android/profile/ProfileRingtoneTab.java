/*
 * Copyright (c) 2011-2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */
package com.android.profile;

import static com.android.profile.ProfileConst.KEY_CLASS;
import static com.android.profile.ProfileConst.KEY_PACKAGE;
import static com.android.profile.ProfileConst.KEY_SUBSCRIPTION_ID;
import static com.android.profile.ProfileConst.LOG;
import android.app.TabActivity;
import android.content.Intent;
import android.content.res.Resources;
import android.os.Bundle;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.TabHost;
import android.widget.TextView;
import android.content.Context;
public class ProfileRingtoneTab extends TabActivity {

    private static final String TAG = "ProfileRingtoneTab";

    private int[] icons = {
            R.drawable.sim1_tab, R.drawable.sim2_tab
    };
    private String[] tags = {
            "sub1", "sub2"
    };
    private Intent mIntent;

    @Override
    protected void onCreate(Bundle icicle) {

        super.onCreate(icicle);

        mIntent = getIntent();
        setContentView(R.layout.profile_ringtone_tab);

        Resources mResources = getResources();
        TabHost mTabHost = getTabHost();
        TabHost.TabSpec mTabSpec;
        Intent intent;
        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {

            intent = new Intent().setClassName(mIntent.getStringExtra(KEY_PACKAGE),
                    mIntent.getStringExtra(KEY_CLASS)).setAction(mIntent.getAction()).putExtra(
                    KEY_SUBSCRIPTION_ID, SubscriptionManager.getSubId(i)[0]);

            mTabSpec = mTabHost.newTabSpec(tags[i]).setIndicator(getSimName(i),
                    mResources.getDrawable(icons[i])).setContent(intent);

            mTabHost.addTab(mTabSpec);
        }
        mTabHost.setCurrentTab(mIntent.getIntExtra(KEY_SUBSCRIPTION_ID, 0));
    }

    @Override
    protected void onResume() {

        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
            setName(i);
        }
        super.onResume();
    }

    private String getSimName(int subscription) {

        return Settings.System.getString(this.getContentResolver(),
                ProfileSetting.MULTI_SIM_NAME[subscription]);
    }

    private void setName(int subscription) {

        TextView simName = (TextView) getTabHost().getTabWidget().getChildAt(subscription)
                .findViewById(com.android.internal.R.id.title);
        simName.setText(getSimName(subscription));
    }

    @SuppressWarnings("unused")
    private static void loge(Object e) {

        if (!LOG)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        e = "[" + mMethodName + "] " + e;
        Log.e(TAG, e + "");
    }

    @SuppressWarnings("unused")
    private static void logd(Object s) {

        if (!LOG)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }
}
