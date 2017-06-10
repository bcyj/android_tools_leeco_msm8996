/*
 * Copyright (C) 2013 The Android Open Source Project
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

/*
 *  Manage the listen-mode routing table.
 */
#pragma once
#include "SyncEvent.h"
#include "NfcJniUtil.h"
#include "RouteDataSet.h"
#include <vector>
extern "C"
{
    #include "nfa_api.h"
    #include "nfa_ee_api.h"
}

class RoutingManager
{
public:
    static const int ROUTE_HOST = 0;
    static const int ROUTE_ESE = 1;

    static RoutingManager& getInstance ();
    bool initialize(nfc_jni_native_data* native);
    void enableRoutingToHost();
    void disableRoutingToHost();
    void enableRoutingToOffHost(int nfcee_id);
    void disableRoutingToOffHost();
    bool addAidRouting(const UINT8* aid, UINT8 aidLen, int route, bool isSubSet, bool isSuperSet);
    bool removeAidRouting(const UINT8* aid, UINT8 aidLen);
    bool commitRouting();
    int  getSecureElementIdList (int nfceeId[]);
    int  getDefaultOffHostRouteDestination ();
    void connectionEventHandler (UINT8 event, tNFA_CONN_EVT_DATA* );
    bool setDefaultRoute(int nfceeId);
    int  getDefaultRoute();
    int  registerJniFunctions (JNIEnv* e);
    void notifyActivated ();
    void notifyDeactivated ();

private:
    RoutingManager();
    ~RoutingManager();
    RoutingManager(const RoutingManager&);
    RoutingManager& operator=(const RoutingManager&);

    void handleData (const UINT8* data, UINT32 dataLen, tNFA_STATUS status);
    void notifyTransactionListenersOfAid (const UINT8* aidBuffer, UINT8 aidBufferLen);
    void notifyHciEventConnectivity (UINT8 nfceeId);

    void getEeInfo ();
    void storeEeDiscReqInfo (tNFA_EE_DISCOVER_REQ& discReq);
    tNFA_TECHNOLOGY_MASK getOffHostTechMask(int nfceeId);
    void handleModeSetRsp(tNFA_EE_MODE_SET& mode_set);
    const char* eeStatusToString (UINT8 status);
    void updateRouting();
    void clearDefaultTechRouting(tNFA_HANDLE eeHandle);
    void clearDefaultProtoRouting(tNFA_HANDLE eeHandle);
    void initializeT3TagEmulation();

    // See AidRoutingManager.java for corresponding
    // AID_MATCHING_ constants

    // Every routing table entry is matched exact (BCM20793)
    static const int AID_MATCHING_EXACT_ONLY = 0x00;
    // Every routing table entry can be matched either exact or prefix
    static const int AID_MATCHING_EXACT_OR_PREFIX = 0x01;
    // Every routing table entry is matched as a prefix
    static const int AID_MATCHING_PREFIX_ONLY = 0x02;

    static void nfaEeCallback (tNFA_EE_EVT event, tNFA_EE_CBACK_DATA* eventData);
    static void stackCallback (UINT8 event, tNFA_CONN_EVT_DATA* eventData);
    static void stackCallbackForT3T(UINT8 event, tNFA_CONN_EVT_DATA* eventData);
    static int com_android_nfc_cardemulation_doGetDefaultRouteDestination (JNIEnv* e);
    static int com_android_nfc_cardemulation_doGetDefaultOffHostRouteDestination (JNIEnv* e);
    static int com_android_nfc_cardemulation_doGetAidMatchingMode (JNIEnv* e);
    static int com_android_nfc_cardemulation_doGetNumSecureElement (JNIEnv* e);

    std::vector<UINT8> mRxDataBuffer;

    // Fields below are final after initialize()
    nfc_jni_native_data* mNativeData;
    int mDefaultEe;
    int mOffHostEe;
    static const JNINativeMethod sMethods [];
    SyncEvent mEeRegisterEvent;
    SyncEvent mRoutingEvent;
    SyncEvent mEeUpdateEvent;
    SyncEvent mEeSetModeEvent;
    SyncEvent mEeDiscReqEvent;
    SyncEvent mRoutingT3tEvent;

    int mAidMatchingMode;
    tNFA_EE_INFO mEeInfo [NFA_EE_MAX_EE_SUPPORTED];
    tNFA_EE_DISCOVER_REQ mEeDiscReqInfo;
    UINT8 mActualNumEe;
    UINT8 mNumActivatedEe;
    bool mIsRoutingToHost;
    bool mIsRoutingToSE;
    bool mIsEeRegistered;
    tNFA_TECHNOLOGY_MASK mActiveSeTechMask;
    tNFA_TECHNOLOGY_MASK mUiccListenMask;
    tNFA_HANDLE mCeAidHandle;
    tNFA_HANDLE mUiccListenTechHandle;
};
