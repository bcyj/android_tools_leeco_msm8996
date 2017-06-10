/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*******************************************************************************
 @file    QcrilMsgTunnelIfaceManager.java
 @brief   Implementation of IQcrilMsgTunnel interface for clients to use
 ******************************************************************************/

package com.qualcomm.qcrilmsgtunnel;

import android.content.Intent;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.util.Log;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.TelephonyProperties;
import com.qualcomm.qcrilhook.IOemHookCallback;

public class QcrilMsgTunnelIfaceManager extends IQcrilMsgTunnel.Stub {

    private static final boolean DBG = true;
    private static final String TAG = "QcrilMsgTunnelIfaceManager";

    private static final String INSTANCE_ID = "INSTANCE_ID";
    private final QcrilMsgTunnelService mService;
    private static QcrilMsgTunnelSocket[] mQcrilMsgTunnelSockets = null;

    // Handler events
    private static final int EVENT_UNSOL_OEM_HOOK_EXT_APP = 0;
    private static final int CMD_INVOKE_OEM_RIL_REQUEST = 1;
    private static final int EVENT_INVOKE_OEM_RIL_REQUEST = 2;
    private static final int CMD_INVOKE_OEM_RIL_REQUEST_ASYNC = 3;
    private static final int EVENT_INVOKE_OEM_RIL_REQUEST_ASYNC_DONE = 4;

    // Retry variables
    private final int RETRY_TIME_MS = 5000;
    private final int RETRY_MAX_ATTEMPTS = 5;
    private int mRetryAttempts = 0;

    private static final int MAX_PHONE_COUNT_SINGLE_SIM = 1;
    private static final int MAX_PHONE_COUNT_DUAL_SIM = 2;
    private static final int MAX_PHONE_COUNT_TRI_SIM = 3;

    QcrilMsgTunnelIfaceManager(QcrilMsgTunnelService service) {
        if (DBG) Log.d(TAG, ":Instantiated");

        this.mService = service;
        int numPhones = getPhoneCount();
        mQcrilMsgTunnelSockets = new QcrilMsgTunnelSocket[numPhones];

        for (int i = 0; i < numPhones; i++) {
            mQcrilMsgTunnelSockets[i] = new QcrilMsgTunnelSocket(i);
            Message msg =  Message.obtain();
            msg.arg1 = i;
            mQcrilMsgTunnelSockets[i].setOnUnsolOemHookExtApp(mMessageHandler,
                    EVENT_UNSOL_OEM_HOOK_EXT_APP, msg);
            Log.d(TAG, "Registered SUB" + i + " for UNSOL OEM HOOK to deliver external apps");
        }
    }

    private int getPhoneCount() {
        String mSimConfig = SystemProperties.get(TelephonyProperties.PROPERTY_MULTI_SIM_CONFIG);
        if (mSimConfig.equals("dsds") || mSimConfig.equals("dsda")) {
            return MAX_PHONE_COUNT_DUAL_SIM;
        } else if (mSimConfig.equals("tsts")) {
            return MAX_PHONE_COUNT_TRI_SIM;
        }
        return MAX_PHONE_COUNT_SINGLE_SIM;
    }

    /**
     * A request object for use with {@link MessageHandler}. Requesters should
     * wait() on the request after sending. The main thread will notify the
     * request when it is complete.
     */
    private static final class MessageRequest {
        /** The argument to use for the request */
        public Object argument;
        /** The sub ID of the request that is run on the main thread */
        public int sub;
        /** The result of the request that is run on the main thread */
        public Object result;

        public MessageRequest(Object argument, int sub) {
            this.argument = argument;
            this.sub = sub;
        }
    }

    /**
     * A request object for use with {@link MessageHandler}. The main thread
     * will notify the request when it is complete.
     */
    private static final class MessageRequestAsync {
        /** The first argument to use for the request */
        public Object arg1;
        /** The second argument to use for the callback */
        public Object arg2;
        /** The sub ID of the request that is run on the main thread */
        public int sub;
        /** The result of the request that is run on the main thread */
        public Object result;

        public MessageRequestAsync(Object arg1, Object arg2, int sub) {
            this.arg1 = arg1;
            this.arg2 = arg2;
            this.sub = sub;
        }
    }

    private Handler mMessageHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            MessageRequest request;
            MessageRequestAsync requestAsync;
            Message onCompleted;
            AsyncResult ar;

            if (DBG) Log.d(TAG, "handleMessage what = " + msg.what);

            switch (msg.what) {
                case CMD_INVOKE_OEM_RIL_REQUEST:
                    request = (MessageRequest) msg.obj;
                    onCompleted = obtainMessage(EVENT_INVOKE_OEM_RIL_REQUEST, request);
                    mQcrilMsgTunnelSockets[request.sub].invokeOemRilRequestRaw(
                            (byte[]) request.argument, onCompleted);
                    break;

                case EVENT_INVOKE_OEM_RIL_REQUEST:
                    ar = (AsyncResult) msg.obj;
                    request = (MessageRequest) ar.userObj;
                    request.result = ar;
                    // Wake up the requesting thread
                    synchronized (request) {
                        request.notifyAll();
                    }
                    break;

                case CMD_INVOKE_OEM_RIL_REQUEST_ASYNC:
                    requestAsync = (MessageRequestAsync) msg.obj;
                    onCompleted = obtainMessage(
                            EVENT_INVOKE_OEM_RIL_REQUEST_ASYNC_DONE, requestAsync);
                    mQcrilMsgTunnelSockets[requestAsync.sub].invokeOemRilRequestRaw(
                            (byte[]) requestAsync.arg1, onCompleted);
                    break;

                case EVENT_INVOKE_OEM_RIL_REQUEST_ASYNC_DONE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        Log.e(TAG, "OEM_RIL_REQUEST_ASYNC_DONE Error received :"
                                + ar.exception);
                    } else {
                        requestAsync = (MessageRequestAsync) ar.userObj;
                        requestAsync.result = ar.result;
                        IOemHookCallback cb = (IOemHookCallback) requestAsync.arg2;
                        Log.e(TAG, "OEM_RIL_REQUEST_ASYNC_DONE requestAsync.sub :"
                                + requestAsync.sub);
                        try {
                            cb.onOemHookResponse((byte[]) (requestAsync.result), requestAsync.sub);
                        } catch (RemoteException e) {
                           e.printStackTrace();
                        }
                    }
                    break;

                case EVENT_UNSOL_OEM_HOOK_EXT_APP:
                    ar = (AsyncResult) msg.obj;
                    Message mesg = (Message)ar.userObj;
                    int instanceId = mesg.arg1;
                    broadcastUnsolOemHookIntent((byte[]) (ar.result), instanceId);
                    break;

                default:
                    Log.w(TAG, "MessageHandler: unexpected message code: " + msg.what);
                    break;
            }
        }
    };

    /**
     * Posts the specified command to be executed on the main thread, waits for
     * the request to complete, and returns the result.
     *
     * @see sendRequestAsync
     */
    private Object sendRequest(int command, Object argument, int sub) {
        if (Looper.myLooper() == mMessageHandler.getLooper()) {
            throw new RuntimeException("This method will deadlock if called from the main thread.");
        }

        MessageRequest request = new MessageRequest(argument, sub);
        Message msg = mMessageHandler.obtainMessage(command, request);
        msg.sendToTarget();

        // Wait for the request to complete
        synchronized (request) {
            while (request.result == null) {
                try {
                    request.wait();
                } catch (InterruptedException e) {
                    // Do nothing, go back and wait until the request is complete
                }
            }
        }
        return request.result;
    }

    /**
     * Asynchronous ("fire and forget") version of sendRequest(): Posts the
     * specified command to be executed on the main thread, and returns
     * immediately.
     *
     * @see sendRequest
     */
    private void sendRequestAsync(int command) {
        mMessageHandler.sendEmptyMessage(command);
    }

    /**
     * Posts the specified command to be executed on the main thread, and
     * returns the result without waiting for the request to complete,
     *
     * @see sendRequestAsync
     */
    private void sendRequestAsync(int command, Object arg1, Object arg2, int sub) {
        MessageRequestAsync request = new MessageRequestAsync(arg1, arg2, sub);
        Message msg = mMessageHandler.obtainMessage(command, request);
        msg.sendToTarget();
    }

    public void broadcastUnsolOemHookIntent(byte[] payload, int instanceId) {
        Intent intent = new Intent(QcrilMsgTunnelService.ACTION_UNSOL_RESPONSE_OEM_HOOK_RAW);
        intent.putExtra("payload", payload);
        intent.putExtra(INSTANCE_ID, instanceId);
        QcrilMsgTunnelService.getContext().sendBroadcast(intent);
    }

    public int sendOemRilRequestRaw(byte[] request, byte[] response, int sub) {
        int returnValue = 0;

        try {
            AsyncResult result = (AsyncResult) sendRequest(CMD_INVOKE_OEM_RIL_REQUEST, request, sub);
            if (result.exception == null) {
                if (result.result != null) {
                    byte[] responseData = (byte[]) (result.result);
                    if (responseData.length > response.length) {
                        Log.w(TAG, "Buffer to copy response too small: Response length is " +
                                responseData.length + "bytes. Buffer Size is " +
                                response.length + "bytes.");
                    }
                    System.arraycopy(responseData, 0, response, 0, responseData.length);
                    returnValue = responseData.length;
                }

            } else {
                if (result.result != null) {
                    byte[] responseData = (byte[]) (result.result);
                    if (responseData.length > response.length) {
                        Log.w(TAG, "Buffer to copy response too small: Response length is " +
                                responseData.length + "bytes. Buffer Size is " +
                                response.length + "bytes.");
                    }
                    System.arraycopy(responseData, 0, response, 0, responseData.length);
                }
                CommandException ex = (CommandException) result.exception;
                returnValue = ex.getCommandError().ordinal();
                if (returnValue > 0)
                    returnValue *= -1;
            }
        } catch (RuntimeException e) {
            Log.w(TAG, "sendOemRilRequestRaw: Runtime Exception");
            returnValue = (CommandException.Error.GENERIC_FAILURE.ordinal());
            if (returnValue > 0)
                returnValue *= -1;
        }

        return returnValue;
    }

    public void sendOemRilRequestRawAsync(byte[] request, IOemHookCallback oemHookCb, int sub) {
        try {
            sendRequestAsync(CMD_INVOKE_OEM_RIL_REQUEST_ASYNC, request, oemHookCb, sub);
        } catch (RuntimeException e) {
            Log.w(TAG, "sendOemRilRequestRawAsync: Runtime Exception", e);
        }
    }
}
