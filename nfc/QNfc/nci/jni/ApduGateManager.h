/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2012 The Android Open Source Project
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

#pragma once
#include "SyncEvent.h"
#include "NfcJniUtil.h"
extern "C"
{
    #include "nfa_api.h"
    #include "nfa_hci_api.h"
    #include "nfa_hci_defs.h"
}

class ApduGateManager
{
public:
    static ApduGateManager& getInstance ();
    bool initialize(nfc_jni_native_data* native);
    int registerJniFunctions (JNIEnv* e);
    void notifyListenModeState (bool isActivated);
    void notifyRfIntfDeactivated ();
    bool isExchangingApduWithEse () { return mIsApduOpenned; };
    bool isListenModeActivated () { return mActivatedInListenMode; };

    bool open(tNFA_HANDLE eeHandle);
    bool close(tNFA_HANDLE eeHandle);
    bool transceive (UINT8* xmitBuffer, INT32 xmitBufferSize, UINT8* recvBuffer, INT32 recvBufferMaxSize, INT32& recvBufferActualSize);
private:
    ApduGateManager();
    ~ApduGateManager();
    ApduGateManager(const ApduGateManager&);
    ApduGateManager& operator=(const ApduGateManager&);

    static jint nativeNfcSecureElement_doOpenSecureElementConnection (JNIEnv*, jobject, jint nfceeId);
    static jboolean nativeNfcSecureElement_doDisconnectSecureElementConnection (JNIEnv*, jobject, jint handle);
    static jbyteArray nativeNfcSecureElement_doTransceive (JNIEnv* e, jobject, jint handle, jbyteArray data);
    static const JNINativeMethod sMethods [];

    nfc_jni_native_data* mNativeData;

    static const UINT8 DEST_HOST = 0x02;
    static const UINT8 DEST_APDU_GATE = 0xF0;
    static const UINT8 STATIC_APDU_PIPE_ID = 0x72;
    static const UINT8 EVT_SOFT_RESET  = 0x11;           // soft reset request on APDU Application gate
    static const UINT8 EVT_SEND_DATA   = 0x10;           // APDU gate and APDU Application gate
    static const UINT8 EVT_END_OF_APDU_TRANSFER = 0x21;  // APDU session is compelete
    static const UINT8 EVT_WTX_REQUEST = 0x11;           // wait time extension request on APDU gate
    static const UINT8 OBERTHUR_WARM_RESET_CMD  = 0x03;

    static const int   NO_REGISTRY_TO_GET = 0;
    static const int   APDU_GATE_REGISTRY_ATR = 1;

    SyncEvent mHciRegisterEvent;
    SyncEvent mHciReconnectEvent;
    SyncEvent mHciSentEvent;
    SyncEvent mRegistryEvent;
    SyncEvent mNfaVsCommandEvent;
    SyncEvent mHciAddStaticPipeEvent;
    SyncEvent mTransceiveEvent;
    SyncEvent mHoldTransceiveEvent;


    bool        mUseEvtSeSoftReset;        //whether to use EVT_SOFT_RESET
    bool        mUseBwiForWtxRequest;      //whether to use BWI in ATR of APDU gate
    bool        mUseEvtEndOfApduTransfer;  //whether to use EVT_END_OF_APDU_TRANSFER
    bool        mUseOberthurWarmReset;     //whether to use warm-reset command
    UINT8       mOberthurWarmResetCommand; //warm-reset command byte

    bool        mHasStaticPipe;

    tNFA_HANDLE mNfaHciHandle;
    tNFA_HANDLE mHciConnectedEeHandle;
    int         mRegistryToGet;
    int         mApduGateAtrLength; // length of ATR of APDU gate
    UINT8       mApduGateAtr[40];   // ATR of APDU gate
    int         mApduGateBWI;       // BWI in ATR of APDU gate
    int         mApduGateTimeout;   // timeout for WTX based on BWI of ATR
    int         mApduGateDelay;     // time for complete reset of eSE after EVT_SOFT_RESET
    int         mApduGateTransceiveTimeout; // timeout transceive function on Apdu Gate
    bool        mIsApduOpenned;
    bool        mReceivedWtx;       // true if received WTX request from eSE
    int         mActualResponseSize;//number of bytes in the response received from secure element
    int         mApduGateDelayAfterSleep; // delay for NFCC to enter sleep mode for Apdu Gate after out of RF field
    bool        mHoldSendingApdu;   // hold sending APDU while deactivating to idle
    bool        mActivatedInListenMode;
    bool        mNeedToRestartRfDiscovery;

    void reconnectHci(tNFA_HANDLE eeHandle);
    void getWtx();
    static void sendVscCallback (UINT8 event, UINT16 param_len, UINT8 *p_param);
    tNFA_STATUS setSwpInactivityTimer (bool enable, UINT8 nfceeId);

    void handleHciRegisterEvt(tNFA_HCI_REGISTER& hciRegister);
    void handleHciGetRegistryRspEvt(tNFA_HCI_REGISTRY& hciRegistry);
    void handleHciAddStaticPipeEvt(tNFA_HCI_ADD_STATIC_PIPE_EVT& hciRegistry);
    static void nfaHciCallback (tNFA_HCI_EVT event, tNFA_HCI_EVT_DATA* eventData);
};

