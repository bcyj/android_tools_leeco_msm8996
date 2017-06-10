/*
 * Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qcrilhook;

import android.os.Message;
import android.os.RemoteException;
import android.util.Log;

import com.qualcomm.qcrilhook.IOemHookCallback;

public class OemHookCallback extends IOemHookCallback.Stub {

    Message mAppMessage;

    public OemHookCallback(Message msg) {
        mAppMessage = msg;
    }

    /**
     * Callback method to handle asynchronous OemHook responses
     */
    public void onOemHookResponse(byte[] response, int phoneId) throws RemoteException {
        Log.w ( "OemHookCallback" ,"mPhoneId: " + phoneId);
        QmiOemHook.receive(response, mAppMessage,
                QmiOemHookConstants.ResponseType.IS_ASYNC_RESPONSE, phoneId);
    }

}
