/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.services.secureui;
import com.android.internal.telephony.cat.AppInterface;

import android.app.KeyguardManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.PowerManager;


/**
 * Intent receiver for the following:
 * ACTION_SCREEN_OFF
 * ACTION_SCREEN_ON
 * ACTION_USER_PRESENT --> lock screen passed
 * CAT_IDLE_SCREEN_ACTION --> back to home screen
 * */
public class ScreenReceiver extends BroadcastReceiver {

  /*
   * @see android.content.BroadcastReceiver#onReceive(android.content.Context, android.content.Intent)
   */
  @Override
  public void onReceive(Context context, Intent intent) {
    String intentAction = intent.getAction();
    if (intentAction == null)
      return;
    KeyguardManager keyguard = (KeyguardManager) context.getSystemService(Context.KEYGUARD_SERVICE);
    if (intentAction.equals(Intent.ACTION_SCREEN_OFF)) {
      SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_SCREEN_STATUS, SecureUIService.SUI_MSG_RET_ABORT, SecureUIService.TOUCH_LIB_ADDR);
      SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_SCREEN_STATUS, SecureUIService.SUI_MSG_RET_ABORT, null);
    } else if (intentAction.equals(Intent.ACTION_SCREEN_ON)) {
      if (!keyguard.isKeyguardLocked()) {
        SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_SCREEN_STATUS, SecureUIService.SUI_MSG_RET_OK, null);
      }
    } else if (intentAction.equals(Intent.ACTION_USER_PRESENT)) {
      SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_SCREEN_STATUS, SecureUIService.SUI_MSG_RET_OK, null);
    }
    else if (intentAction.equals(AppInterface.CAT_IDLE_SCREEN_ACTION)) {
      if (intent.getBooleanExtra("SCREEN_IDLE",true)) {
        // back to home screen
        SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_ABORT, SecureUIService.SUI_MSG_RET_NOP, SecureUIService.TOUCH_LIB_ADDR);
        SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_ABORT, SecureUIService.SUI_MSG_RET_NOP, null);
      }
    }
  }

  public static boolean screenOn() {
    PowerManager powerManager = (PowerManager) SecureUIService.context.getSystemService(Context.POWER_SERVICE);
    KeyguardManager keyguard = (KeyguardManager) SecureUIService.context.getSystemService(Context.KEYGUARD_SERVICE);
    if (powerManager.isScreenOn() && !keyguard.isKeyguardLocked()) {
      return true;
    }
    return false;
  }

}
