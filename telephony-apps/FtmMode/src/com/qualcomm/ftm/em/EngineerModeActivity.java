/******************************************************************************
 * @file    EngineerModeActivity.java
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 *******************************************************************************/
package com.qualcomm.ftm.em;

import android.app.Activity;
import android.app.TabActivity;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TabHost;
import android.widget.TabHost.TabSpec;
import com.android.internal.telephony.TelephonyIntents;

import com.qualcomm.ftm.R;
import com.qualcomm.qcrilhook.IQcRilHook;
import com.qualcomm.qcrilhook.QcRilHook;
import com.qualcomm.qcrilhook.QcRilHookCallback;

class Rat {
    static final int INVALID = -1;
    static final int CDMA = 0;
    static final int GSM = 1;
    static final int WCDMA = 2;
}

public class EngineerModeActivity extends TabActivity implements
        TabHost.OnTabChangeListener, OnCheckedChangeListener {

    private static final String EXTRA_TAB_INDEX = "tab_index";
    private static final String[] TAB_LABEL = {"SUB 1", "SUB 2", "SUB 3"};
    // Tab label to be displayed for single sim configuration
    private static final String SINGLE_SIM_TAB_LABEL = "FTM data";
    private static final String ACTION_ENABLE_ENGINEER_MODE =
            "android.intent.action.ENABLE_ENGINEER_MODE";
    private static final String TAG = "EmgineerModeActivity";
    private static boolean sIsOverlayServiceActive = false;

    private boolean mIsQcRilHookReady = false;

    private TabHost mTabHost;
    private Context mContext;
    private RadioGroup mRadioGroup;
    private RadioButton mRadioButtonOn;
    private RadioButton mRadioButtonOff;
    private SQLiteDatabase mSqliteDatabase;
    private QcRilHook mQcRilHook;
    private static int mNumPhones = TelephonyManager.getDefault().getPhoneCount();

    public static int ratType[] = new int[mNumPhones];

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.em_tab);
        Log.d(TAG, "in onCreate " + mNumPhones);
        DatabaseHelper mDbHelper = new DatabaseHelper(EngineerModeActivity.this, "em.db");
        mSqliteDatabase = mDbHelper.getReadableDatabase();

        mContext = getApplicationContext();
        mQcRilHook = new QcRilHook(mContext, mQcrilHookCb);
        mTabHost = getTabHost();
        mTabHost.setOnTabChangedListener(this);
        mTabHost.getTabWidget().setStripEnabled(false);
        mRadioGroup = (RadioGroup) findViewById(R.id.radiogroup);
        setupEmList();
        initButton();
    }

    private void initButton() {
        mRadioButtonOn = (RadioButton)findViewById(R.id.radio_on);
        mRadioButtonOff = (RadioButton)findViewById(R.id.radio_off);
        mRadioButtonOn.setOnCheckedChangeListener(this);
        mRadioButtonOff.setOnCheckedChangeListener(this);
     }

     @Override
     public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
         if (isChecked && mTabHost != null) {
             Intent intent = new Intent(Intent.ACTION_MAIN);
             Bundle bundle = new Bundle();

             if (buttonView == mRadioButtonOn) {
                 bundle.putBoolean("startOverlay", true);
                 sIsOverlayServiceActive = true;
             } else if (buttonView == mRadioButtonOff) {
                 bundle.putBoolean("startOverlay", false);
                 sIsOverlayServiceActive = false;
             }

             intent.putExtras(bundle);
             intent.setClass(mContext, EmInfoService.class);
             mContext.startService(intent);

         }
     }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
        for (int i = 0; i < mNumPhones; i++) {
            setFtmMode(1, i);
        }
    }

    private QcRilHookCallback mQcrilHookCb = new QcRilHookCallback() {
        public void onQcRilHookReady() {
            mIsQcRilHookReady = true;
            Log.d(TAG, "In Qcril hook cb");
            for (int i = 0; i < mNumPhones; i++) {
                setFtmMode(1, i);
            }
        }
        public void onQcRilHookDisconnected() {
            mIsQcRilHookReady = false;
        }
    };

    private int mapRatType(int rat) {
        Log.d(TAG, "mapRatType: rat = " + rat);
        switch  (rat) {
            case TelephonyManager.NETWORK_TYPE_UMTS:
            case TelephonyManager.NETWORK_TYPE_HSPA:
            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPAP:
            case TelephonyManager.NETWORK_TYPE_TD_SCDMA:
                return Rat.WCDMA;

            case TelephonyManager.NETWORK_TYPE_CDMA:
            case TelephonyManager.NETWORK_TYPE_EVDO_0:
            case TelephonyManager.NETWORK_TYPE_EVDO_A:
            case TelephonyManager.NETWORK_TYPE_EVDO_B:
            case TelephonyManager.NETWORK_TYPE_1xRTT:
                return Rat.CDMA;

            case TelephonyManager.NETWORK_TYPE_GPRS:
            case TelephonyManager.NETWORK_TYPE_GSM:
                return Rat.GSM;

            default:
                return Rat.INVALID;
        }
    }

    public void setFtmMode(int enable, int phoneId) {
        boolean result = false;
        int rat;
        if (enable == 1) {
            int[] subId = SubscriptionManager.getSubId(phoneId);
            if (subId[0] < 0) {
                Log.e(TAG, "invalid subId: " + subId[0]);
                return;
            }
            // Query nw type only when enabling FTM, info is already
            // available in ratType when disabling FTM
            rat = TelephonyManager.getDefault().getVoiceNetworkType(subId[0]);
            ratType[phoneId] = mapRatType(rat);
        }
        if (ratType[phoneId] == Rat.INVALID) {
            Log.e(TAG, "Invalid rat " + ratType[phoneId] + " phoneId = " + phoneId);
            return;
        }
        Log.d(TAG, "Sending RIL OEM HOOK ENGINEER MODE request phoneId:" + phoneId
                + "ratType:" + ratType[phoneId] + " enable:" + enable);
        if (!mIsQcRilHookReady) {
            Log.e(TAG, "qcril oem hook is not ready yet");
            return;
        }
        result = mQcRilHook.qcRilSetFieldTestMode(phoneId, (byte)ratType[phoneId], enable);
        if (result == false) {
            Log.e(TAG, "qcRilSetFieldTestMode failed ");
        }
    }

    @Override
    public void onPause() {
        if (!sIsOverlayServiceActive) {
            mSqliteDatabase.delete("eminfo", null, null);
            for (int i = 0; i < mNumPhones; i++) {
                setFtmMode(0, i);
            }
        }
        super.onPause();
    }

    public Intent setTab(Intent intent, int tabIndex) {
        if (tabIndex == -1) {
            intent.removeExtra(EXTRA_TAB_INDEX);
        } else {
            intent.putExtra(EXTRA_TAB_INDEX, tabIndex);
        }
        return intent;
    }

    private void setupEmList() {
        for (int i = 0; i < mNumPhones; i++) {
            Intent intent = new Intent();
            intent.setClass(this, EmSIMActivity.class)
                    .setAction(intent.getAction()).putExtra("sub_id", i);
            setTab(intent, i);
            mTabHost.addTab(mTabHost.newTabSpec(TAB_LABEL[i])
                    .setIndicator((mNumPhones == 1) ? SINGLE_SIM_TAB_LABEL : TAB_LABEL[i],
                     null).setContent(intent));
        }
    }

    @Override
    public void onTabChanged(String tabId) {
        Activity activity = getLocalActivityManager().getActivity(tabId);
        if (activity != null) {
            activity.onWindowFocusChanged(true);
        }
    }
}
