/* Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 */

package org.codeaurora.btmultisim;

/**
 * Interface used to interact with BluetoothPhoneService to handle DSDA.
 *
 * {@hide}
 */
interface IBluetoothDsdaService {
    void setCurrentSub(int sub);
    void phoneSubChanged();
    void handleMultiSimPreciseCallStateChange(int ForegroundCallState,
            int RingingCallState, String RingingNumber, int NumberType,
            int BackgroundCallState, int numHeldCallsonSub);
    void processQueryPhoneState();
    int getTotalCallsOnSub(int subId);
    boolean isSwitchSubAllowed();
    void switchSub();
    boolean canDoCallSwap();
    boolean hasCallsOnBothSubs();
    boolean isFakeMultiPartyCall();
    boolean answerOnThisSubAllowed();
}

