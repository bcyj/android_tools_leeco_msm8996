/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.timerswitch;

import com.android.timerswitch.provider.TimerSwitch;
import com.android.timerswitch.utils.Log;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import android.telephony.TelephonyManager;
import android.telephony.PhoneStateListener;

public class TimerSwitchReceiver extends BroadcastReceiver {
    private static final String TAG = TimerSwitchReceiver.class.getSimpleName();

    private static Context mContext;
    private static TelephonyManager mTelephonyManager;
    private static PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onCallStateChanged(int state, String ignored) {
            Log.d("onCallStateChanged:" + state);
            if (state == TelephonyManager.CALL_STATE_IDLE && mTelephonyManager != null) {
                fireShutDown(mContext);
            }
        }
    };

    @Override
    public void onReceive(Context context, Intent intent) {
        String receiveAction  = intent.getAction();
        mContext = context;

        if (receiveAction.equals(TimerSwitchConstants.ACTION_POWER_OFF)) {
            mTelephonyManager = (TelephonyManager) context.getSystemService(
                    Context.TELEPHONY_SERVICE);
            int mInitialCallState = mTelephonyManager.getCallState();

            Log.d(TAG + " PowerOFF Action:" + receiveAction + " CallState:" + mInitialCallState);

            TimerSwitch timerSwitch = TimerSwitchUtils.parseTimerSwitchFromIntent(intent);
            if (timerSwitch == null) {
                Log.d(TAG + ":Failed to parse the timerswitch from the intent");
                return;
            }

            long staleInMinutes = (System.currentTimeMillis() - timerSwitch.switchtime)/
                    TimerSwitchConstants.MILLIS_IN_MINUTE;
            if (!timerSwitch.daysOfWeek.isRepeating()) {
                timerSwitch.enabled = false;
                TimerSwitchUtils.updateTimerSwitch(context.getContentResolver(), timerSwitch);
            } else {
                TimerSwitchUtils.enableTimerSwitch(context, timerSwitch);
            }
            if (staleInMinutes > 0) {
                Log.d(TAG + " Expired POWER OFF switch");
                return;
            }

            if (mInitialCallState != TelephonyManager.CALL_STATE_IDLE) {
                mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_CALL_STATE);
            } else {
                fireShutDown(context);
            }

        } else if (receiveAction.equals(TimerSwitchConstants.ACTION_POWER_ON)) {
            Log.d(TAG + " PowerON:" + receiveAction);

            TimerSwitch timerSwitch = TimerSwitchUtils.parseTimerSwitchFromIntent(intent);
            if (timerSwitch == null) {
                Log.d(TAG + ":Failed to parse the timerswitch from the intent");
                return;
            }
            if (!timerSwitch.daysOfWeek.isRepeating()) {
                Log.d("TimerSwitchReceive: is not repeat and expired");
                timerSwitch.enabled = false;
                TimerSwitchUtils.updateTimerSwitch(context.getContentResolver(), timerSwitch);
            } else {
                Log.d("TimerSwitchReceive: is repeat");
                TimerSwitchUtils.enableTimerSwitch(context, timerSwitch);
            }
        }

    }

    public static void fireShutDown(Context context) {
        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);

        Intent closeDialogs = new Intent(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        context.sendBroadcast(closeDialogs);
        final int schduleTimeOff = 500;

        Intent i = new Intent(context, ShutdownActivity.class);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, i,
                PendingIntent.FLAG_ONE_SHOT);
        AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        am.set(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + schduleTimeOff,
                pendingIntent);

    }
}
