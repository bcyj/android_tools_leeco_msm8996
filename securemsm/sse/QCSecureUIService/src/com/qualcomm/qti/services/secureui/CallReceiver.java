/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.services.secureui;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.telephony.TelephonyManager;
import android.util.Log;

/**
 * Intent receiver for the following:
 * ACTION_PHONE_STATE_CHANGED
 * */
public class CallReceiver extends BroadcastReceiver {
  public static final String TAG = "SecUISvc.CallReceiver";

  @Override
  public void onReceive(Context context, Intent intent) {
    String action = intent.getAction();
    if ((action == null) ||
        (action.equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) == false) {
      Log.w(TAG, "Unexpected intent");
      return;
    }
    String extra = intent.getStringExtra(TelephonyManager.EXTRA_STATE);
    if (extra == null) {
      Log.w(TAG, "No information attached to call intent");
      return;
    }

    if (extra.equals(TelephonyManager.EXTRA_STATE_RINGING)) {
      Log.d(TAG, "Ringing");
      SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_CALL_STATUS, SecureUIService.SUI_MSG_RET_ABORT, SecureUIService.TOUCH_LIB_ADDR);
      SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_CALL_STATUS, SecureUIService.SUI_MSG_RET_ABORT, null);
    } else if (extra.equals(TelephonyManager.EXTRA_STATE_OFFHOOK)) {
      Log.d(TAG, "Off Hook");
      SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_CALL_STATUS, SecureUIService.SUI_MSG_RET_ABORT, SecureUIService.TOUCH_LIB_ADDR);
      SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_CALL_STATUS, SecureUIService.SUI_MSG_RET_ABORT, null);
    } else if (extra.equals(TelephonyManager.EXTRA_STATE_IDLE)) {
      Log.d(TAG, "Phone Idle");
      SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_CALL_STATUS, SecureUIService.SUI_MSG_RET_OK, null);
    } else {
      Log.w(TAG, "Unknown phone state: " + extra);
    }
  }

  public static boolean callActive() {
    TelephonyManager telephonyManager = (TelephonyManager)SecureUIService.context.getSystemService(SecureUIService.context.TELEPHONY_SERVICE);
    return (telephonyManager.getCallState() != TelephonyManager.CALL_STATE_IDLE);
  }
}
