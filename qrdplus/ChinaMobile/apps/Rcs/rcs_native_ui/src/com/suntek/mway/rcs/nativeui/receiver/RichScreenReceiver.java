/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.receiver;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import com.suntek.mway.rcs.nativeui.service.RichScreenService;

public class RichScreenReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d("onreceive RichScreenReceiver:",intent.getAction());
        intent.setClass(context, RichScreenService.class);
        context.startService(intent);
    }
}

