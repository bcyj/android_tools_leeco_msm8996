/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.receiver;

import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.RcsApiManager;
import com.suntek.mway.rcs.nativeui.RcsNativeUIApp;
import com.suntek.mway.rcs.nativeui.ui.DialogActivity;
import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;
import com.suntek.mway.rcs.client.aidl.setting.RcsUserProfileInfo;
import com.suntek.mway.rcs.client.api.autoconfig.RcsAccountApi;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.wifi.WifiManager;
import android.telephony.TelephonyManager;
import android.widget.Toast;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class DMSReceiver extends BroadcastReceiver {

    public static final String REQUEST_DMS_CONFIG_VERSION = "-2";
    public static final String ACTION_DMS_OPEN_BUSS = BroadcastConstants.ACTION_DMS_OPEN_BUSS;

    public static final String ACTION_OPEN_PS = BroadcastConstants.ACTION_OPEN_PS;

    public static final String ACTION_DMS_OPEN_BUSS_RESULT = BroadcastConstants.ACTION_DMS_OPEN_BUSS_RESULT;

    public static final String ACTION_CONFIRM_USE_NEW_IMSI = BroadcastConstants.ACTION_CONFIRM_USE_NEW_IMSI;

    public static final String ACTION_DMS_USER_STATUS_CHANGED = BroadcastConstants.ACTION_DMS_USER_STATUS_CHANGED;

    public static final String ACTION_FETCH_CONFIG_FINISH = BroadcastConstants.ACTION_FETCH_CONFIG_FINISH;

    @Override
    public void onReceive(final Context context, Intent intent) {
        if (ACTION_DMS_OPEN_BUSS.equals(intent.getAction())) {
            String title = intent.getStringExtra(BroadcastConstants.DMS_TIPS_TITLE);
            String message = intent.getStringExtra(BroadcastConstants.DMS_TIPS_MESSAGE);
            int accept_btn = intent.getIntExtra(BroadcastConstants.DMS_TIPS_ACCEPT_BTN, -1);
            int rejectBtn = intent.getIntExtra(BroadcastConstants.DMS_TIPS_REJECT_BTN, -1);
            DialogActivity.startOpenAccountDialog(context, title, message, accept_btn, rejectBtn);
        } else if (ACTION_OPEN_PS.equals(intent.getAction())) {
            DialogActivity.startOpenPSDialog(context);
        } else if (ACTION_DMS_OPEN_BUSS_RESULT.equals(intent.getAction())) {
            int resutlCode = intent.getIntExtra(BroadcastConstants.OPER_RESULTCODE, -1);
            String number = intent.getStringExtra(BroadcastConstants.DMS_OPEN_BUSS_MSISDN);
            String resutl = intent.getStringExtra(BroadcastConstants.OPER_RESULTDESC);
            if (resutlCode == BroadcastConstants.DMS_OPEN_BUSS_RESULTCODE_SUCCESS) {
                Toast.makeText(context, context.getString(R.string.auto_open_buss_succeed),
                        Toast.LENGTH_LONG).show();
            } else {
                Toast.makeText(context, context.getString(R.string.auto_open_buss_fail) + resutl,
                        Toast.LENGTH_LONG).show();
            }

        } else if (ACTION_CONFIRM_USE_NEW_IMSI.equals(intent.getAction())) {
        } else if (ACTION_DMS_USER_STATUS_CHANGED.equals(intent.getAction())) {
            int state = intent.getIntExtra(BroadcastConstants.DMS_USER_STATUS, 0);
            String message = intent.getStringExtra(BroadcastConstants.DMS_TIPS_MESSAGE);
            Toast.makeText(context, message, Toast.LENGTH_LONG).show();
            RcsUserProfileInfo info = null;
            try {
                info = RcsApiManager.getAccountApi().getRcsUserProfileInfo();
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
            }
            if(info != null ){
                if(REQUEST_DMS_CONFIG_VERSION.equals(info.getVersion())){
                    DialogActivity.startReGetDmsConfig(context);
                }
            }
        } else if (ACTION_FETCH_CONFIG_FINISH.equals(intent.getAction())){
            if(RcsNativeUIApp.isNeedClosePs()){
                setMobileData(context, false);
                RcsNativeUIApp.setNeedClosePs(false);
            }
            if(RcsNativeUIApp.isNeedOpenWifi()){
                openWifi(context);
                RcsNativeUIApp.setNeedOpenWifi(false);
            }
        }
    }

    private void closeWifi(Context context){
        WifiManager mWifiManager = (WifiManager) context
                .getSystemService(Context.WIFI_SERVICE);
        if (mWifiManager.isWifiEnabled()){
            RcsNativeUIApp.setNeedOpenWifi(true);
            mWifiManager.setWifiEnabled(false);
        }
    }

    private void openWifi(Context context){
        WifiManager mWifiManager = (WifiManager) context
                .getSystemService(Context.WIFI_SERVICE);
        mWifiManager.setWifiEnabled(true);
    }

    private void setMobileData(Context context, boolean pBoolean) {
        try {
            ConnectivityManager mConnectivityManager = (ConnectivityManager) context
                    .getSystemService(Context.CONNECTIVITY_SERVICE);
            Class<? extends ConnectivityManager> ownerClass = mConnectivityManager.getClass();
            @SuppressWarnings("rawtypes")
            Class[] argsClass = new Class[1];
            argsClass[0] = boolean.class;
            Method method = ownerClass.getMethod("setMobileDataEnabled", argsClass);
            method.invoke(mConnectivityManager, pBoolean);
        } catch (Exception e) {
            TelephonyManager telephonyManager = (TelephonyManager) context
                    .getSystemService(Context.TELEPHONY_SERVICE);
            Class<? extends TelephonyManager> ownerClass = telephonyManager.getClass();
            Class[] argsClass = new Class[1];
            argsClass[0] = boolean.class;
            try {
                Method method = ownerClass.getMethod("setDataEnabled", argsClass);
                method.invoke(telephonyManager, pBoolean);
            } catch (NoSuchMethodException e1) {
                e1.printStackTrace();
            } catch (IllegalAccessException e1) {
                e1.printStackTrace();
            } catch (IllegalArgumentException e1) {
                e1.printStackTrace();
            } catch (InvocationTargetException e1) {
                e1.printStackTrace();
            }
            e.printStackTrace();
        }
    }
}
