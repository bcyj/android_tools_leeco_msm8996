/*
 *Copyright (c) 2014 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.intermediate;

public interface ICallInterfaceListener {

    void imsRegStateChanged(int regstate);

    void imsRegStateChangeReqFailed();

    void onNewRingingConnection(String address, int callType);

    void onPhoneStateChanged(int state);

    void onDisconnect(int status);

    void onAvpUpgradeFailure(String error);

    void onRemoteModifyCallRequest(int callType);

    void onModifyCallDone(boolean success, String result);

}
