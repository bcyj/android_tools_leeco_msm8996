/*******************************************************************************
@file    QcRilHookCallback.java
@brief   A simple interface to implement the callback to the caller
         when the oemhook library gets onServiceConnected notification
         QcrilMsgTunnel service binding.
---------------------------------------------------------------------------
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.qcrilhook;

public interface QcRilHookCallback {
    public void onQcRilHookReady();

    public void onQcRilHookDisconnected();
}
