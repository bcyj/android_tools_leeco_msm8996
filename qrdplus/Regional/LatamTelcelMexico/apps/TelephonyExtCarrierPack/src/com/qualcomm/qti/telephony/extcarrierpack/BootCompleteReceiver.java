/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.telephony.extcarrierpack;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
/**
 * This Boot complete receiver is used to start the application process
 * and once the process is started, CarrierApplication will be created
 * and carrier application will register for perso state changes.
 */
public class BootCompleteReceiver extends BroadcastReceiver{

    @Override
    public void onReceive(Context arg0, Intent arg1) {
        Log.d("BootCompleteReceiver", "onreceive");
    }

}
