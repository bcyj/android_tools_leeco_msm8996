/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.dm;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import com.android.internal.telephony.PhoneConstants;
import com.android.dm.vdmc.MyTreeIoHandler;
import com.android.dm.vdmc.Vdmc;

public class DmJniInterface {
    private static final String TAG = "DmJniInterface";
    private static Context mContext;
    private static int pppstatus = PhoneConstants.APN_TYPE_NOT_AVAILABLE;
    private static DmJniInterface mInstance = null;

    // public static DmJniInterface getInstance()
    // {
    // if (null == mInstance)
    // {
    // mInstance = new DmJniInterface();
    // //Log.d("DM ==> DmService: ",
    // "getInstance: new DmService() , mInstance = " + mInstance);
    // }
    // //Log.d("DM ==> DmService: ", "getInstance: mInstance = " + mInstance);
    // return mInstance;
    // }

    DmJniInterface(Context context) {
        mContext = context;
        // mInstance = this;
    }

    // display alert dialog
    public void displayDialog(int id, byte[] data, int timeout)
    {
        String message = null;
        if (data != null) {
            message = new String(data);
        }
        Log.d(TAG, "displayDialog id: " + id + " message: " + message);
        Intent intent = new Intent(mContext, DmAlertDialog.class);
        int intentFlags = Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP;

        intent.setFlags(intentFlags);
        intent.putExtra("dialogId", id);
        intent.putExtra("message", message);
        intent.putExtra("timeout", timeout);

        mContext.startActivity(intent);
    }

    private static DmNetwork mNetwork;

    public int startPppConnect()
    {
        int status = -1;
        String type = DmService.getInstance().getAPN();

        Log.d(TAG, "startPppConnect: apn = " + type);

        mNetwork = DmNetwork.getInstance();
        if (mNetwork != null) {
            status = mNetwork.beginConnectivity(type);
            Log.d(TAG, "startPppConnect: mNetwork is not null,begin connect");
        } else {
            mNetwork = new DmNetwork(mContext);
            Log.d(TAG, "startPppConnect: new DmNetwork(mContext)");
            if (mNetwork != null) {
                Log.d(TAG, "startPppConnect: new DmNetwork(mContext) success!");
                mNetwork.init();
                Log.d(TAG, "startPppConnect: init");
                status = mNetwork.beginConnectivity(type);
            } else {
                Log.d(TAG, "startPppConnect: new DmNetwork(mContext) fail!");
                status = -1;
            }
        }
        Log.d(TAG, "startPppConnect: status = " + status);
        setNetworkState(status);
        // pppstatus = status;
        return pppstatus;
    }

    public static void setNetworkState(int state) {
        pppstatus = state;
    }

    public void stopPppConnect()
    {
        Log.d(TAG, "stopPppConnect: mNetwork = " + mNetwork);
        if (mNetwork != null)
        {
            if (getPppConnectStatus() == PhoneConstants.APN_REQUEST_STARTED
                    || getPppConnectStatus() == PhoneConstants.APN_ALREADY_ACTIVE)
            {
                mNetwork.endConnectivity();
            }
            mNetwork.end();

            mNetwork = null;
        }
        setNetworkState(PhoneConstants.APN_TYPE_NOT_AVAILABLE);
        Vdmc.getInstance().stopVDM();
        // pppstatus=PhoneConstants.APN_TYPE_NOT_AVAILABLE;
    }

    // MMIDM_GetPdpActiveStatus
    // static final int APN_ALREADY_ACTIVE = 0;
    // static final int APN_REQUEST_STARTED = 1;
    // static final int APN_TYPE_NOT_AVAILABLE = 2;
    // static final int APN_REQUEST_FAILED = 3;
    public int getPppConnectStatus()
    {
        Log.d(TAG, "PdpActiveStatus = " + pppstatus);
        return pppstatus;
    }

    public byte[] read_CBFunc(int handlerType, int offset)
    {
        byte[] data = new byte[1024];
        MyTreeIoHandler.getInstance().read(handlerType, offset, data);
        return data;

    }

    public void write_CBFunc(int handlerType, int offset, byte[] data, int totalSize)
    {

        MyTreeIoHandler.getInstance().write(handlerType, offset, data, totalSize);
    }

    public void write_null_CBFunc(int handlerType)
    {

        MyTreeIoHandler.getInstance().writenull(handlerType);
    }

    public byte[] getServerNonce()
    {
        byte[] data = new byte[1024];
        DmService.getInstance().getServerNonce(mContext, data);
        return data;
    }

    public void setServerNonce(byte[] data)
    {
        DmService.getInstance().setServerNonce(mContext, data);
    }

    public byte[] getClientNonce()
    {
        byte[] data = new byte[1024];
        DmService.getInstance().getClientNonce(mContext, data);
        return data;
    }

    public void setClientNonce(byte[] data)
    {
        DmService.getInstance().setClientNonce(mContext, data);
    }

}
