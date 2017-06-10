/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (C) 2013 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.gsmtuneaway;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.TelephonyManager.MultiSimVariants;
import android.util.Log;

public class GsmTuneAwayReceiver extends BroadcastReceiver {

    private static final boolean DBG = true;

    private static final String LOG_TAG = "GsmTuneAwayReceiver";
    private static final String PROPERTY_GTA_OPEN_KEY = "persist.radio.multisim.gta";
    private static final String SETTING_PRE_NW_MODE_DEFAULT = "preferred_network_mode_default";

    private static final int EVENT_GET_PREFERRED_NETWORK_TYPE_SELF = 0;
    private static final int EVENT_GET_PREFERRED_NETWORK_TYPE_OTHER = 1;
    private static final int EVENT_SET_PREFERRED_NETWORK_TYPE = 2;

    private Context mContext;
    private boolean isCheckingOtherSub = false;
    private int mSubscription;

    @Override
    public void onReceive(Context context, Intent intent) {
        boolean gtaEnabled = SystemProperties.getBoolean(PROPERTY_GTA_OPEN_KEY, false);

        if (gtaEnabled) {
            mContext = context;
            if (intent.getAction().equals(TelephonyIntents
                    .ACTION_ANY_DATA_CONNECTION_STATE_CHANGED)) {
                if (DBG) log("data connection state changed");

                if ((TelephonyManager.getDefault().getMultiSimConfiguration()
                        != MultiSimVariants.DSDS))
                    return;

                PhoneConstants.DataState state = Enum.valueOf(PhoneConstants.DataState.class,
                        intent.getStringExtra(PhoneConstants.STATE_KEY));
                int subId = PhoneFactory.getDataSubscription();
                mSubscription = SubscriptionManager.getPhoneId(subId);
                if (DBG) log("state = " + state + " subscription = " + mSubscription);

                switch (state) {
                    case CONNECTED:
                        String apnType = intent.getStringExtra(PhoneConstants.DATA_APN_TYPE_KEY);
                        if (DBG) log("apnType is:" + apnType);
                        //Ignore this. This is MMS DDS temporary switching
                        //We want to ignore this so that we will not change NW mode
                        if (PhoneConstants.APN_TYPE_MMS.equals(apnType))
                            return;

                        checkOtherSubState(mSubscription);
                        break;
                    case DISCONNECTED:
                        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
                            if (i != mSubscription) {
                                syncNetworkModeToDB(i);
                            }
                        }
                        break;
                    }
                } else if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) {
                    int phoneCount = TelephonyManager.getDefault().getPhoneCount();
                    Log.e(LOG_TAG, "Received BOOT COMPLETED phoneCount: " + phoneCount);
                    for (int i = 0; i < phoneCount; i++) {
                        setPreNetworkType(getPrefNetworkTypeFromDB(i), i);
                    }
            }
        }
    }

    private void setPreNetworkType(int preNetworkType, int phoneId) {
        android.telephony.TelephonyManager.putIntAtIndex(
                mContext.getContentResolver(),
                SETTING_PRE_NW_MODE_DEFAULT,
                phoneId, preNetworkType);

    }

    private int getPrefNetworkTypeFromDB(int phoneId) {
        int nwMode;
        try {
            nwMode = android.telephony.TelephonyManager.getIntAtIndex(
                    mContext.getContentResolver(),
                    android.provider.Settings.Global.PREFERRED_NETWORK_MODE,
                    phoneId);
        } catch (SettingNotFoundException snfe) {
            log("getPrefNetworkTypeFromDB: Could not find PREFERRED_NETWORK_MODE!!!");
            nwMode = Phone.NT_MODE_TD_SCDMA_GSM_LTE;
        }
        log("getPrefNetworkTypeFromDB network mode: " + nwMode + " for phoneId " + phoneId);
        return nwMode;
    }

    private void syncNetworkModeToDB(int sub) {
        PhoneFactory.getPhone(sub).getPreferredNetworkType(
                mHandler.obtainMessage(EVENT_GET_PREFERRED_NETWORK_TYPE_SELF, sub,
                PhoneConstants.SUB1));
    }

    private void checkOtherSubState(int sub) {
        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
            if (i != sub) {
                if (DBG) log("checkOtherActiveSub:= " + i );
                Phone phone = PhoneFactory.getPhone(i);
                if (phone.getIccCard().getState() != IccCardConstants.State.ABSENT
                        && phone.getIccCard().getState() != IccCardConstants.State.UNKNOWN
                        && phone.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
                    phone.getPreferredNetworkType(
                            mHandler.obtainMessage(EVENT_GET_PREFERRED_NETWORK_TYPE_OTHER, i,
                            PhoneConstants.SUB1));
                }
            }
        }
    }

    private Handler mHandler = new Handler(){
        @Override
        public void handleMessage(Message msg){
            AsyncResult ar;

            switch(msg.what){
                case EVENT_GET_PREFERRED_NETWORK_TYPE_SELF:
                    log("EVENT_GET_PREFERRED_NETWORK_TYPE SELF");
                    ar = (AsyncResult) msg.obj;
                    int toSyncSub = msg.arg1;

                    try{
                        int nwMode = TelephonyManager.getIntAtIndex(mContext.getContentResolver(),
                                SETTING_PRE_NW_MODE_DEFAULT, toSyncSub);
                        log("toSyncSub is " + toSyncSub + " and nvMode is : " + nwMode);

                        if (ar.exception == null) {
                            log("The NW mode of self sub is:" + ((int[])ar.result)[0]);
                        }

                        if ((ar.exception != null) || (nwMode != ((int[])ar.result)[0])) {
                            PhoneFactory.getPhone(toSyncSub).setPreferredNetworkType
                                    (nwMode,obtainMessage(EVENT_SET_PREFERRED_NETWORK_TYPE,
                                    toSyncSub, nwMode));
                            log("send request is sync to db");
                        }
                    } catch (SettingNotFoundException ex) {
                        Log.e(LOG_TAG, "SettingNotFoundException = " + ex);
                    }
                    break;

                case EVENT_GET_PREFERRED_NETWORK_TYPE_OTHER:
                    ar = (AsyncResult) msg.obj;
                    int phoneId = msg.arg1;
                    log("EVENT_GET_PREFERRED_NETWORK_TYPE checkSub is:" + phoneId);
                    if (ar.exception == null) {
                        log("The NW mode of other sub is: " + ((int[])ar.result)[0]);
                        if (((int[])ar.result)[0] != Phone.NT_MODE_GSM_ONLY) {
                            isCheckingOtherSub = true;
                            PhoneFactory.getPhone(phoneId).setPreferredNetworkType
                                    (Phone.NT_MODE_GSM_ONLY,
                                            obtainMessage(EVENT_SET_PREFERRED_NETWORK_TYPE,
                                            phoneId, Phone.NT_MODE_GSM_ONLY));
                            log("send request to set to GSM only");
                        } else {
                            syncNetworkModeToDB(mSubscription);
                        }
                    } else {
                        // Weird state, disable the setting
                        log("get preferred network type, exception=" + ar.exception);
                    }
                    break;

                case EVENT_SET_PREFERRED_NETWORK_TYPE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        log("EVENT_SET_PREFERRED_NETWORK_TYPE failed:ar.exception=" + ar.exception);
                    } else {
                        log("EVENT_SET_PREFERRED_NETWORK_TYPE success");
                        android.telephony.TelephonyManager.putIntAtIndex(
                                mContext.getContentResolver(),
                                android.provider.Settings.Global.PREFERRED_NETWORK_MODE,
                                msg.arg1, msg.arg2);
                        if (isCheckingOtherSub) {
                            syncNetworkModeToDB(mSubscription);
                            isCheckingOtherSub = false;
                        }
                    }
                    break;

                default:
                    break;

                }
            }
    };

    private void log(String string) {
        Log.d(LOG_TAG, string);
    }
}
