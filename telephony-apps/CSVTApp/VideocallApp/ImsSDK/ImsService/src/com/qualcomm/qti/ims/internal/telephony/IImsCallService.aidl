/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.internal.telephony;

import java.util.Map;
import com.qualcomm.qti.ims.internal.telephony.IImsCallServiceListener;

interface IImsCallService {

    int dial(String number, int callType);

    boolean acceptCall(int callType);

    boolean hangup();

    void registerCallback(IImsCallServiceListener imsServListener);

    void unregisterCallback(IImsCallServiceListener imsServListener);

    void setRegistrationState(int imsRegState);

    int getRegistrationState();

    void hangupUri(String userUri);

    void hangupWithReason(int connectionId, String userUri, String confUri,
            boolean mpty, in int failCause, in String errorInfo);

    String[] getCallDetailsExtrasinCall(int callId);

    String getImsDisconnectCauseInfo(int callId);

    String[] getUriListinConf();

    boolean isVTModifyAllowed();

    boolean getProposedConnectionFailed(int connIndex);

    boolean isAddParticipantAllowed();

    void addParticipant(String dialString, int clir, int callType, in String[] extra);

    void acceptConnectionTypeChange(boolean accept);

    void changeConnectionType(int callType);

    boolean startDtmf(char c);

    int getActiveCallType();

    void mergeCalls();

    boolean isImsPhoneActive();

    boolean isImsPhoneIdle();

    void setMute(boolean muted);

    void endConference();

    void switchHoldingAndActive();

}