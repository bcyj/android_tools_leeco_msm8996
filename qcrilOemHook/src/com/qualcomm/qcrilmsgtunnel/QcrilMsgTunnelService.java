/*******************************************************************************
@file    QcrilMsgTunnelService.java
@brief   Communicates with QCRIL for OEM specific reqs/responses

---------------------------------------------------------------------------
Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.qcrilmsgtunnel;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.ServiceManager;
import android.util.Log;

public class QcrilMsgTunnelService extends Service {

    private static final String TAG = "QcrilMsgTunnelService";

    /**
     * <p>Broadcast Action: It indicates the an RIL_UNSOL_OEM_HOOK_RAW message was received
     * <p class="note">.
     * This is to indicate OEM applications that an unsolicited OEM message was received.
     *
     * <p class="note">This is a protected intent that can only be sent
     * by the system.
     */
    public static final String ACTION_UNSOL_RESPONSE_OEM_HOOK_RAW =
            "android.intent.action.ACTION_UNSOL_RESPONSE_OEM_HOOK_RAW";

    static Context mContext;
    private static QcrilMsgTunnelIfaceManager mTunnelIface;

    public static Context getContext() {
        return mContext;
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "Returning mHookIface for QcrilMsgTunnelService binding.");
        return mTunnelIface;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate method");
        mContext = getBaseContext();
        mTunnelIface = new QcrilMsgTunnelIfaceManager(this);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "QcrilMsgTunnelService Destroyed Successfully...");
        super.onDestroy();
    }
}
