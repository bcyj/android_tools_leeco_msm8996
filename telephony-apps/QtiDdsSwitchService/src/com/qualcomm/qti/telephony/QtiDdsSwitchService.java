/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.telephony;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.Messenger;
import android.os.SystemProperties;
import android.os.Looper;
import android.util.Log;

import com.android.internal.telephony.dataconnection.DctController;
import com.android.internal.util.AsyncChannel;
import com.qualcomm.qcrilhook.IQcRilHook;
import com.qualcomm.qcrilhook.QcRilHook;

/**
 * System service to process DDS switch requests
 */
public class QtiDdsSwitchService extends Service {
    public static final String TAG = "QtiDdsSwitchService";

    private static final int EVENT_REGISTER = 0;
    private static final int EVENT_REQUEST_DDS_SWITCH = 1;

    Handler mHandler;
    Context mContext;
    QcRilHook  mQcRilHook;
    DctController mDctController;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate");
        mContext = this;
        mQcRilHook = new QcRilHook(mContext, null);
        HandlerThread handlerThread = new HandlerThread("QcDdsSwitchHandler");
        handlerThread.start();

        mHandler = new DdsSwitchHandler(handlerThread.getLooper());
    }

    @Override
    public int onStartCommand(Intent intet, int flags, int startId) {
        Log.d(TAG, "onStart");

        mHandler.sendMessage(mHandler.obtainMessage(EVENT_REGISTER));
        //recover incase of phone process crash. START_STICKY.
        return Service.START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    void registerWithDctController(Handler handler) {
        mDctController = DctController.getInstance();
        Log.d(TAG, "got the dctController = " + mDctController);
        if (null != mDctController) {
            mDctController.registerDdsSwitchPropService(new Messenger(handler));
        }
    }

    void invokeOemHookDdsSwitch(final int phoneCount, final int phoneId) {
        Log.d(TAG, "invokeOemHookDdsSwitch");
        for(int i = 0; i < phoneCount; i++) {
            Log.d(TAG, "qcRilSendDDSInfo ril= " + i + ", DDS=" + phoneId);
            mQcRilHook.qcRilSendDDSInfo(phoneId, i);
        }
    }

    class DdsSwitchHandler extends Handler {

        public DdsSwitchHandler(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message msg) {
            Log.d(TAG, "HandleMessage msg = " + msg);

            switch (msg.what) {
                case EVENT_REGISTER: {
                    Log.d(TAG, "EVENT_REGISTER");
                    registerWithDctController(this);
                    break;
                }

                case EVENT_REQUEST_DDS_SWITCH: {
                    int phoneId = msg.arg1;
                    int phoneCount = msg.arg2;
                    int result = 1;
                    Log.d(TAG, "EVENT_REQUEST_DDS_SWITCH for phoneId = " + phoneId);
                    AsyncChannel replyAc = new AsyncChannel();

                    invokeOemHookDdsSwitch(phoneCount, phoneId);
                    replyAc.replyToMessage(msg, EVENT_REQUEST_DDS_SWITCH, result);
                    Log.d(TAG, "EVENT_REQUEST_DDS_SWITCH Done.");

                    break;
                }

            }

        }
    }

}
