/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.imstestrunner;

import android.app.Service;
import android.content.Intent;
import android.content.Context;
import android.content.IntentFilter;
import android.os.IBinder;
import android.util.Log;
import android.os.Bundle;
import android.os.SystemProperties;
import android.content.BroadcastReceiver;

public class FakeImsResponseService extends Service {
    ImsSocketAgent socketAgent = null;
    static final String TEST_MODE = "com.qcom.imstestrunner.TEST_MODE";
    static final String MAKE_MT_CALL = "com.qcom.imstestrunner.MAKE_MT_CALL";
    static final String RESPONSE_GEN_READY = "com.qcom.imstestrunner.RESPONSE_GEN_READY";
    static final String MT_CALL_MADE = "com.qcom.imstestrunner.MT_CALL_MADE";
    static final String RESET_SOCKET_REQUEST = "com.qcom.imstestrunner.RESET_SOCKET_REQUEST";
    static final String SOCKET_RESET_COMPLETE = "com.qcom.imstestrunner.SOCKET_RESET_COMPLETE";

    static final String TEST_OPTION = "TEST_OPTION";
    static final String CALL_TYPE = "CALL_TYPE";
//    static final String ANDROID_VERSION = "ANDROID_VERSION";
    static final String LOGTAG = "FakeImsResponseService";

    private final BroadcastReceiver receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (MAKE_MT_CALL.equals(action)) {
                if (socketAgent != null) {
                    int callType = socketAgent.makeMtCall(100, ImsQmiIF.CALL_INCOMING);
                    boolean usingInstr = SystemProperties.getBoolean("persist.radio.testing", false);
                    if(usingInstr) {
                        while(!(socketAgent.isIncomingCallPolled()));
                        socketAgent.setIncomingCallPolled(false);
                        Intent sendIntent = new Intent(MT_CALL_MADE);
                        sendIntent.putExtra(CALL_TYPE, callType);
                        sendBroadcast(sendIntent);
                    }
                }
            } else if (TEST_MODE.equals(action)) {
                int testOption = intent.getIntExtra(TEST_OPTION, -1);
                if (testOption != -1 && socketAgent != null) {
                    socketAgent.setTestOption(testOption);
                    Log.d(LOGTAG, "Set test option to " + testOption);
                    Intent sendIntent = new Intent(RESPONSE_GEN_READY);
                    sendBroadcast(sendIntent);
                }
            } else if(RESET_SOCKET_REQUEST.equals(action)) {
                Log.d(LOGTAG, "Received RESET_SOCKET_REQUEST Intent");
                if(socketAgent != null) {
                    ImsSocketAgent.reset();
                    socketAgent = ImsSocketAgent.getInstance();
                }
                Intent sendIntent = new Intent(SOCKET_RESET_COMPLETE);
                sendBroadcast(sendIntent);
            }
        }
    };

    public void onCreate() {
        super.onCreate();
        startImsServerTask();
        IntentFilter filter = new IntentFilter();
        filter.addAction(TEST_MODE);
        filter.addAction(MAKE_MT_CALL);
        filter.addAction(RESET_SOCKET_REQUEST);
        registerReceiver(receiver, filter);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return Service.START_STICKY;
    }

    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    private void startImsServerTask() {
        if (socketAgent == null) {
            socketAgent = ImsSocketAgent.getInstance();
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(receiver);
    }
}
