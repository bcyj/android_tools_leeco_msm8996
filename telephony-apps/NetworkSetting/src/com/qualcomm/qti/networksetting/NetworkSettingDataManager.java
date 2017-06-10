/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */


package com.qualcomm.qti.networksetting;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.os.AsyncResult;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.WindowManager;
import android.widget.Button;
import android.telephony.SubscriptionManager;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

public class NetworkSettingDataManager {
    private static final String LOG_TAG = "NetworkSettingDataManager";
    private static final boolean DBG = true;
    Context mContext;
    private TelephonyManager mTelephonyManager;
    private boolean mNetworkSearchDataDisconnecting = false;
    private boolean mNetworkSearchDataDisabled = false;
    private boolean mAppDisableData = true;
    Message mMsg;

    public NetworkSettingDataManager(Context context) {
        mContext  = context;
        if (DBG) log(" Create NetworkSettingDataManager");
        mTelephonyManager = (TelephonyManager)mContext.getSystemService(Context.TELEPHONY_SERVICE);
        mAppDisableData = SystemProperties.getBoolean("persist.radio.plmn_disable_data", true);
        log("mAppDisableData: " + mAppDisableData);
    }

    /**
     * Receiver for ANY_DATA_CONNECTION_STATE_CHANGED
     */
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (mNetworkSearchDataDisconnecting) {
                if (action.equals(TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED)) {
                    if (mTelephonyManager.getDataState() == TelephonyManager.DATA_DISCONNECTED) {
                        log("network disconnect data done");
                        mNetworkSearchDataDisabled = true;
                        mNetworkSearchDataDisconnecting = false;
                        mMsg.arg1 = 1;
                        mMsg.sendToTarget();

                    }
                }
            }
        }
    };

    public void updateDataState(boolean enable, Message msg) {
        if (!enable) {
            if (mTelephonyManager.getDataState() == TelephonyManager.DATA_CONNECTED) {
                log("Data is in CONNECTED state");
                mMsg = msg;
                ConfirmDialogListener listener = new ConfirmDialogListener(msg);
                AlertDialog d = new AlertDialog.Builder(mContext)
                        .setTitle(android.R.string.dialog_alert_title)
                        .setIcon(android.R.drawable.ic_dialog_alert)
                        .setMessage(R.string.disconnect_data_confirm)
                        .setPositiveButton(android.R.string.ok, listener)
                        .setNegativeButton(android.R.string.no, listener)
                        .setOnCancelListener(listener)
                        .create();

                d.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
                d.show();
            } else {
                msg.arg1 = 1;
                msg.sendToTarget();
            }
        } else {
            if (mAppDisableData &&
                        mNetworkSearchDataDisabled || mNetworkSearchDataDisconnecting) {
                log("Enabling data");
                //enable data service
                mTelephonyManager.setDataEnabled( SubscriptionManager
                        .getDefaultDataSubId(), true);
                mContext.unregisterReceiver(mReceiver);
                mNetworkSearchDataDisabled = false;
                mNetworkSearchDataDisconnecting = false;
            }
        }
    }

    private final class ConfirmDialogListener
            implements DialogInterface.OnClickListener, DialogInterface.OnCancelListener {

        Message msg1;
        ConfirmDialogListener(Message msg) {
            msg1 = msg;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE){
                // If apps not required to disable DATA then send SUCCESS
                // status to client for processing MPLMN search request
                if (!mAppDisableData) {
                    msg1.arg1 = 1;
                    msg1.sendToTarget();
                    return;
                }
                //disable data service
                IntentFilter intentFilter = new IntentFilter();
                intentFilter.addAction(
                        TelephonyIntents.ACTION_ANY_DATA_CONNECTION_STATE_CHANGED);
                mContext.registerReceiver(mReceiver, intentFilter);
                mNetworkSearchDataDisconnecting = true;
                mTelephonyManager.setDataEnabled(SubscriptionManager
                        .getDefaultDataSubId(), false);
            } else if (which == DialogInterface.BUTTON_NEGATIVE){
                log("network search, do nothing");
                msg1.arg1 = 0;
                msg1.sendToTarget();
            }
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            msg1.arg1 = 0;
            msg1.sendToTarget();
        }
    }

    private void log(String msg) {
        Log.d(LOG_TAG, "[NetworkSettingDataManager] " + msg);
    }
}
