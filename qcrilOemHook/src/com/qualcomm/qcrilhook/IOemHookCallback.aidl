/*******************************************************************************
@file    IOemHookCallback.aidl
@brief   Interface exposed to register & handle response callback

---------------------------------------------------------------------------
Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.qcrilhook;

interface IOemHookCallback {
    void onOemHookResponse(in byte[] response, in int phoneId);
}
