/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.gsmtuneaway;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.MSimConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.MSimTelephonyManager;
import android.telephony.TelephonyManager;
import android.telephony.MSimTelephonyManager.MultiSimVariants;
import android.util.Log;

import com.codeaurora.telephony.msim.MSimPhoneFactory;

public class GsmTuneAwayReceiver extends BroadcastReceiver {

    private static final String LOG_TAG = "GsmTuneAwayReceiver";
    private static final boolean DBG = true;
    private static final String PROPERTY_GTA_OPEN_KEY = "persist.env.phone.gsmtuneaway";

    private static final int EVENT_GET_PREFERRED_NETWORK_TYPE_SELF = 0;
    private static final int EVENT_GET_PREFERRED_NETWORK_TYPE_OTHER = 1;
    private static final int EVENT_SET_PREFERRED_NETWORK_TYPE = 2;

    private int mPreviousType = Phone.NT_MODE_WCDMA_PREF;
    private int mSubscription;

    private Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {

        if (intent.getAction().equals(TelephonyIntents.
                ACTION_ANY_DATA_CONNECTION_STATE_CHANGED)) {
            if (DBG) log("data connection state changed");

            if ((MSimTelephonyManager.getDefault().getMultiSimConfiguration()
                    == MultiSimVariants.DSDS)) {
                mContext = context;
                mSubscription = MSimPhoneFactory.getDataSubscription();
            } else {
                return;
            }

            PhoneConstants.DataState state = Enum.valueOf(PhoneConstants.DataState.class,
                    intent.getStringExtra(PhoneConstants.STATE_KEY));
            if (DBG) log("state = " + state);
            switch (state) {
                case CONNECTED:
                        if (DBG) log("mSubscription = " + mSubscription);

                        boolean gtaEnabled = SystemProperties.getBoolean(PROPERTY_GTA_OPEN_KEY,
                                true);
                        if (gtaEnabled) {
                            if ((MSimConstants.SUB1 == mSubscription) &&
                                    checkPhoneState(MSimConstants.SUB2)) {
                                getNetworkType(MSimConstants.SUB2);
                            } else if ((MSimConstants.SUB2 == mSubscription) &&
                                    checkPhoneState(MSimConstants.SUB1)) {
                                getNetworkType(MSimConstants.SUB1);
                            }

                            MSimPhoneFactory.getPhone(mSubscription).getPreferredNetworkType(
                                    mHandler.obtainMessage(EVENT_GET_PREFERRED_NETWORK_TYPE_SELF));
                        }
                        break;
                }
        }
    }

    private boolean checkPhoneState(int sub) {
        return MSimPhoneFactory.getPhone(sub).getIccCard().getState()
                != IccCardConstants.State.ABSENT
                && MSimPhoneFactory.getPhone(sub).getIccCard().getState()
                != IccCardConstants.State.UNKNOWN
                && MSimPhoneFactory.getPhone(sub).getPhoneType()
                == PhoneConstants.PHONE_TYPE_GSM;
    }

    private void getNetworkType(int sub) {
        MSimPhoneFactory.getPhone(sub).getPreferredNetworkType(
                mHandler.obtainMessage(EVENT_GET_PREFERRED_NETWORK_TYPE_OTHER,sub,0));
    }

    private Handler mHandler = new Handler(){
        @Override
        public void handleMessage(Message msg){
            AsyncResult ar;

            switch(msg.what){
                case EVENT_GET_PREFERRED_NETWORK_TYPE_SELF:
                    log("EVENT_GET_PREFERRED_NETWORK_TYPE SELF");
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        mPreviousType = ((int[])ar.result)[0];
                        log("current dds network preferred mode: type= " + mPreviousType);
                    }

                    try{
                        int nwMode = MSimTelephonyManager.getIntAtIndex(
                                mContext.getContentResolver(),
                                android.provider.Settings.Global.PREFERRED_NETWORK_MODE,
                                mSubscription);
                        log("mSubscription " + mSubscription + " nvMode is : " + nwMode);

                        if (nwMode != mPreviousType) {
                            MSimPhoneFactory.getPhone(mSubscription).setPreferredNetworkType
                                    (nwMode,obtainMessage(EVENT_SET_PREFERRED_NETWORK_TYPE));
                            log("send request is sync to db");
                        }
                    } catch (SettingNotFoundException ex) {
                        Log.e(LOG_TAG, "SettingNotFoundException = " + ex);
                    }

                    break;

                case EVENT_GET_PREFERRED_NETWORK_TYPE_OTHER:
                    ar = (AsyncResult) msg.obj;
                    int sub = msg.arg1;
                    log("EVENT_GET_PREFERRED_NETWORK_TYPE checkSub is:" + sub);
                    if (ar.exception == null) {
                        mPreviousType = ((int[])ar.result)[0];
                        log("current other sub network preferred mode: type= " + mPreviousType);
                        if (mPreviousType != Phone.NT_MODE_GSM_ONLY) {
                            MSimPhoneFactory.getPhone(sub).setPreferredNetworkType
                                (Phone.NT_MODE_GSM_ONLY,obtainMessage
                                    (EVENT_SET_PREFERRED_NETWORK_TYPE));
                            log("send request to set to GSM only");
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
