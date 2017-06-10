/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */
package com.borqs.videocall;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class IPServerReceiver extends BroadcastReceiver {

	// static final String ACTION = "android.intent.action.BOOT_COMPLETED";
	static final String INTENT_STARTUP = "com.borqs.videocall.action.StartupVTIpConnect";
	static final String INTENT_SHUTDOWN ="com.borqs.videocall.action.ShutdownVTIpConnect";

	static final String TAG = "VT/VIIpConnectReceiver";
	public synchronized void onReceive(Context context, Intent intent) {
		if (MyLog.DEBUG) MyLog.v(TAG, intent.getAction());
		if( intent.getAction().equals(INTENT_SHUTDOWN)){
			IPServerService.stopService();
		}
		else {  //BOOT_COMPLETED or INTENT_STARTUP
			IPServerService.startService( context);
        }
	}
}
