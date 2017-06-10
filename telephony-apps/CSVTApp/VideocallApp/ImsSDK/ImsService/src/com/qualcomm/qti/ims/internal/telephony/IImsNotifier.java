/*
 *Copyright (c) 2014 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.internal.telephony;

public interface IImsNotifier {

    public abstract void notifyImsRegStateChanged(int regstate);

    public abstract void notifyImsRegStateChangeReqFailed();

    public abstract void notifyNewRingingConnection(String address, int callType);

    public abstract void notifyImsPhoneStateChanged(int state);

    public abstract void notifyImsDisconnect(int cause);

    public abstract void notifyImsRemoteModifyCallRequest(int callType);

    public abstract void notifyImsModifyCallDone(boolean success, String result);

    public abstract void notifyImsAvpUpgradeFailure(String error);

}