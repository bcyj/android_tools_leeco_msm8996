/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.timerswitch;

import com.android.timerswitch.utils.Log;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;

public class TimerSwitchInitReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(final Context context, Intent intent) {
        final String action = intent.getAction();
        Log.v("AlarmInitReceiver " + action);

        final PendingResult result = goAsync();

        new Handler().post(new Runnable() {
            @Override
            public void run() {
                Log.d("initReceiver:" + action);
                if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
                    Log.d("TimerSwitchReceive BootComplete");
                    TimerSwitchUtils.disableExpiredTimerSwitch(context);
                    TimerSwitchUtils.setNextTimerSwitch(context);
                } else {
                    TimerSwitchUtils.setNextTimerSwitch(context);
                }
                result.finish();
                Log.v("AlarmInitReceiver finished");
            }
        });
    }
}
