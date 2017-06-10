/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

public class NetManager {
    private static final String TAG = "Download.NetManager";
    private static final boolean DEBUG = true;
    private static boolean mOrigData = true;
    private static int mOrigSub = 0;
    private static boolean mConnectSettingChanged = false;
    private static String CHINA_TELECOM_CODE = "46003, 46005";

    public static boolean isCTCardInsert(Context context) {
        TelephonyManager mTelephonyManager = TelephonyManager.getDefault();
        for(int i = 0; i < mTelephonyManager.getPhoneCount(); i++) {
            if( CTCardAvailable(context, i)) {
                return true;
            }
        }
        return false;
    }

    public static boolean haveNetConnective(Context context) {
        boolean dataEnable;
        ConnectivityManager cm = (ConnectivityManager)
                context.getSystemService(Context.CONNECTIVITY_SERVICE);
        TelephonyManager mTelephonyManager = TelephonyManager.getDefault();
        //mobile 3G Data Network
        NetworkInfo.State mobile = cm.getNetworkInfo(ConnectivityManager.TYPE_MOBILE).getState();
        //wifi
        NetworkInfo.State wifi = cm.getNetworkInfo(ConnectivityManager.TYPE_WIFI).getState();

        logd("haveNetConnective, wifi state is " + wifi + ", mobile state is " + mobile);
        //only when have wifi connection or sim1 is CT card and data active to update
        if(wifi == NetworkInfo.State.CONNECTED) {
            return true;
        }

        if(mTelephonyManager.isMultiSimEnabled()) {
            dataEnable = (mobile == NetworkInfo.State.CONNECTED && CTCardAvailable(context, 0)
                    && SubscriptionManager.getDefaultDataSubId() == 0);
        } else {
            dataEnable = (mobile == NetworkInfo.State.CONNECTED);
        }
        return dataEnable;
    }

    public static boolean CTCardAvailable(Context context, int sub) {
        TelephonyManager mTelephonyManager = TelephonyManager.getDefault();
        if(mTelephonyManager.isMultiSimEnabled()) {
            logd("sim" + sub + "'s state is " + mTelephonyManager.getSimState(sub));
            logd("operator is " + mTelephonyManager.getSimOperator(sub));
            return  mTelephonyManager.getSimState(sub) == TelephonyManager.SIM_STATE_READY &&
                    CHINA_TELECOM_CODE.contains(mTelephonyManager.getSimOperator(sub));
        } else {
            TelephonyManager telephonyManager = (TelephonyManager)
                    context.getSystemService(Context.TELEPHONY_SERVICE);
            logd("sim's state is " + telephonyManager.getSimState());
            logd("operator is " + telephonyManager.getSimOperator());
            return telephonyManager.getSimState() == TelephonyManager.SIM_STATE_READY &&
                    CHINA_TELECOM_CODE.contains(telephonyManager.getSimOperator());
        }
    }

    public static void setDataEnableTo(Context context, int subscribe) {
        TelephonyManager mTelephonyManager = TelephonyManager.getDefault();
        ConnectivityManager cm = (ConnectivityManager)
                context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo.State state = cm.getNetworkInfo(ConnectivityManager.TYPE_MOBILE).getState();
        if(state != NetworkInfo.State.CONNECTED) {
            mOrigData = false;
            mTelephonyManager.setDataEnabled(true);
            mConnectSettingChanged = true;
        }
        if(mTelephonyManager.isMultiSimEnabled()) {
            if(SubscriptionManager.getDefaultDataSubId() != subscribe) {
                mOrigSub = SubscriptionManager.getDefaultDataSubId();
                SubscriptionManager.from(context).setDefaultDataSubId(subscribe);
                mConnectSettingChanged = true;
            }
        }
    }

    public static void resetDataConnect(Context context) {
        ConnectivityManager cm = (ConnectivityManager)
                context.getSystemService(Context.CONNECTIVITY_SERVICE);
        TelephonyManager mTelephonyManager = TelephonyManager.getDefault();
        if(mTelephonyManager.isMultiSimEnabled()) {
            SubscriptionManager.from(context).setDefaultDataSubId(mOrigSub);
        }
        mTelephonyManager.setDataEnabled(mOrigData);
        mConnectSettingChanged = false;
    }

    public static boolean isConnectSettingChanged() {
        return mConnectSettingChanged;
    }

    public static String getMEID(Context context) {
        TelephonyManager mTelephonyManager = TelephonyManager.getDefault();
        boolean isMultiSimEnabled = mTelephonyManager.isMultiSimEnabled();
        if (isMultiSimEnabled) {
            return mTelephonyManager.getDeviceId(0);
        } else {
            TelephonyManager telephonyManager =
                    (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
            return telephonyManager.getDeviceId();
        }
    }

    public static String getIMSI(Context context) {
        TelephonyManager mTelephonyManager = TelephonyManager.getDefault();
        boolean isMultiSimEnabled = mTelephonyManager.isMultiSimEnabled();
        if (isMultiSimEnabled) {
            return mTelephonyManager.getSubscriberId(0);
        } else {
            TelephonyManager telephonyManager =
                    (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
            return telephonyManager.getSubscriberId();
        }
    }

    private static void logd(String msg) {
        if(DEBUG) {
            Log.d(TAG, msg);
        }
    }
}
