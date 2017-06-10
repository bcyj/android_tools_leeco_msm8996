/*******************************************************************************
@file    IQcrilMsgTunnel.aidl
@brief   Interface exposed for other apps to make use of OemQcrilHookService

---------------------------------------------------------------------------
Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.qcrilmsgtunnel;

import com.qualcomm.qcrilhook.IOemHookCallback;

interface IQcrilMsgTunnel {

    /**
     * Sends a OEM request to the RIL and returns the response back to the
     * Caller. The returnValue is negative on failure. 0 or length of response on SUCCESS
     */
    int sendOemRilRequestRaw(in byte[] request, out byte[] response, in int sub);

    /**
     * Sends a OEM request to the RIL asynchronously. An OemHook callback is invoked
     * with the response bytes
     */
    void sendOemRilRequestRawAsync(in byte[] request, in IOemHookCallback oemHookCb, in int sub);

}
