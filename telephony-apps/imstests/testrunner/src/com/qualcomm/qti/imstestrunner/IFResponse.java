/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
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

package com.qualcomm.qti.imstestrunner;

import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Handler;
import android.util.Log;
class IFResponse {
    static final String LOG_TAG = "IFResponse";

    // ***** Class Variables
    static int sNextSerial = 0;
    static Object sSerialMonitor = new Object();
    private static Object sPoolSync = new Object();
    private static IFResponse sPool = null;
    private static int sPoolSize = 0;
    private static final int MAX_POOL_SIZE = 4;

    // ***** Instance Variables
    int mSerial;
    int mRequest;
    Message mResult;
    // FIXME delete parcel
    // Parcel mp;
    IFResponse mNext;
    byte[] mData;

    /**
     * Retrieves a new IFResponse instance from the pool.
     *
     * @param request ImsQmiIF.MsgId.REQUEST_*
     * @param result sent when operation completes
     * @return a IFResponse instance from the pool.
     */
    static IFResponse obtain(int request, int mSerial) {
        IFResponse rr = null;

        synchronized (sPoolSync) {
            if (sPool != null) {
                rr = sPool;
                sPool = rr.mNext;
                rr.mNext = null;
                sPoolSize--;
            }
        }

        if (rr == null) {
            rr = new IFResponse();
        }

        synchronized (sSerialMonitor) {
            rr.mSerial = mSerial;
        }
        rr.mRequest = request;
        rr.mResult = null;

        return rr;
    }

    /**
     * Returns a IFResponse instance to the pool. Note: This should only be
     * called once per use.
     */
    void release() {
        synchronized (sPoolSync) {
            if (sPoolSize < MAX_POOL_SIZE) {
                this.mNext = sPool;
                sPool = this;
                sPoolSize++;
                mResult = null;
            }
        }
    }

    private IFResponse() {
    }

    void resetSerial() {
        synchronized (sSerialMonitor) {
            sNextSerial = 0;
        }
    }

    String serialString() {
        // Cheesy way to do %04d
        StringBuilder sb = new StringBuilder(8);
        String sn;

        sn = Integer.toString(mSerial);

        // sb.append("J[");
        sb.append('[');
        for (int i = 0, s = sn.length(); i < 4 - s; i++) {
            sb.append('0');
        }

        sb.append(sn);
        sb.append(']');
        return sb.toString();
    }

    void onError(int error, Object ret) {
        RuntimeException ex;
        String errorMsg;

        if (error == ImsQmiIF.E_SUCCESS) {
            ex = null;
        } else {
            errorMsg = ImsSocketAgent.errorIdToString(error);
            ex = new RuntimeException(errorMsg);
        }

        if (ImsSocketAgent.IF_LOGD)
            Log.d(LOG_TAG, serialString() + "< "
                    + ImsSocketAgent.msgIdToString(mRequest)
                    + " error: " + error);
    }

}