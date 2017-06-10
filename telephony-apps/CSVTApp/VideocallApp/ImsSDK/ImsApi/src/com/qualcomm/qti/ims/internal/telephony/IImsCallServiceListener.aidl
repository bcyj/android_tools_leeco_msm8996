/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.internal.telephony;

interface IImsCallServiceListener {

    /**
     * Get the changed ims registration state
     */
    void imsRegStateChanged(int regstate);

    /**
     * IMS registration state change request failure callback
     */
    void imsRegStateChangeReqFailed();

    void onNewRingingConnection(String address, int callType);

    void onPhoneStateChanged(int state);

    void onDisconnect(int cause);

    void onRemoteModifyCallRequest(int callType);

    void onModifyCallDone(boolean success, String result);

    void onAvpUpgradeFailure(String error);
}