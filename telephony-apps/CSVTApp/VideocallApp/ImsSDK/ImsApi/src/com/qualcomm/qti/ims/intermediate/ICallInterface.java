/*
 *Copyright (c) 2014 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.intermediate;

public interface ICallInterface {

    int dial(String number, int callType);

    boolean acceptCall(int callType);

    boolean hangup();

    void registerCallback(ICallInterfaceListener listener);

    int getActiveCallType();

    void setRegistrationState(int imsRegState);

    void changeConnectionType(int callType);

    void acceptConnectionTypeChange(boolean accept);

    void addParticipant(String dialString, boolean isConference);

    boolean isImsPhoneActive();

    boolean isImsPhoneIdle();

    void mergeCalls();

    void setMute(boolean muted);

    void startDtmf(char c);

    String[] getUriListConf();

    void switchHoldingAndActive();

    void hangupUri(String uri);

    void endConference();

    void createSdkService();

    int getRegistrationState();
}
