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

#include <cutils/log.h>
#include <ScopedLocalRef.h>
#include <JNIHelp.h>
#include "config.h"
#include "JavaClassConstants.h"
#include "RoutingManager.h"
#include "ApduGateManager.h"

extern "C"
{
    #include "nfa_ee_api.h"
    #include "nfa_ce_api.h"
}

const JNINativeMethod RoutingManager::sMethods [] =
{
    {"doGetDefaultRouteDestination", "()I", (void*) RoutingManager::com_android_nfc_cardemulation_doGetDefaultRouteDestination},
    {"doGetDefaultOffHostRouteDestination", "()I", (void*) RoutingManager::com_android_nfc_cardemulation_doGetDefaultOffHostRouteDestination},
    {"doGetAidMatchingMode", "()I", (void*) RoutingManager::com_android_nfc_cardemulation_doGetAidMatchingMode},
    {"doGetNumSecureElement", "()I", (void*) RoutingManager::com_android_nfc_cardemulation_doGetNumSecureElement}
};

#define CONFIG_MAX_NFCID2_LEN 8
#define CONFIG_MAX_SYSCODE_LEN 2
#define MAX_T3T_CMD_LEN 252

//Tag 1 - NFCID2 and SysCode
UINT8 test_app1_nfcid2_size = 0;
UINT8 test_app1_sysCode_size = 0;
//The +1 is to detect invalid NFCID2 and SysCode sizes greater than MAX
UINT8 test_app1_nfcid2[CONFIG_MAX_NFCID2_LEN + 1];
UINT8 test_app1_sysCode[CONFIG_MAX_SYSCODE_LEN + 1];

//Tag 2 - NFCID2 and SysCode
UINT8 test_app2_nfcid2_size = 0;
UINT8 test_app2_sysCode_size = 0;
//The +1 is to detect invalid NFCID2 and SysCode sizes greater than MAX
UINT8 test_app2_nfcid2[CONFIG_MAX_NFCID2_LEN + 1];
UINT8 test_app2_sysCode[CONFIG_MAX_SYSCODE_LEN + 1];

//Tag 3 - NFCID2 and SysCode
UINT8 test_app3_nfcid2_size = 0;
UINT8 test_app3_sysCode_size = 0;
//The +1 is to detect invalid NFCID2 and SysCode sizes greater than MAX
UINT8 test_app3_nfcid2[CONFIG_MAX_NFCID2_LEN + 1];
UINT8 test_app3_sysCode[CONFIG_MAX_SYSCODE_LEN + 1];

//Type3 Loopback
static void myType3ListenLoopback(uint8_t* p_buf, uint32_t len);

RoutingManager::RoutingManager ()
{
    static const char fn [] = "RoutingManager::RoutingManager()";
    unsigned long num = 0;

    // Get the "default" route
    if (GetNumValue("DEFAULT_ISODEP_ROUTE", &num, sizeof(num)))
        mDefaultEe = num;
    else
        mDefaultEe = 0x00;

    ALOGD("%s: default route is 0x%02X", fn, mDefaultEe);
    // Get the default "off-host" route.  This is hard-coded at the Java layer
    // but we can override it here to avoid forcing Java changes.
    if (GetNumValue("DEFAULT_OFFHOST_ROUTE", &num, sizeof(num)))
        mOffHostEe = num;
    else
        mOffHostEe = 0x01;

    if (GetNumValue("AID_MATCHING_MODE", &num, sizeof(num)))
        mAidMatchingMode = num;
    else
        mAidMatchingMode = AID_MATCHING_EXACT_ONLY;

    ALOGD("%s: mOffHostEe=0x%02X", fn, mOffHostEe);


    mActualNumEe = 0;
    mNumActivatedEe = 0;
    mIsEeRegistered = false;
}

RoutingManager::~RoutingManager ()
{
    NFA_EeDeregister (nfaEeCallback);
}

bool RoutingManager::initialize (nfc_jni_native_data* native)
{
    static const char fn [] = "RoutingManager::initialize()";
    //tNFA_TECHNOLOGY_MASK techMask = NFA_TECHNOLOGY_MASK_A;
    tNFA_STATUS nfaStat;

    mNativeData = native;
    mIsRoutingToHost = false;
    mIsRoutingToSE = false;
    mActiveSeTechMask = 0;
    mUiccListenTechHandle = 0;
    mCeAidHandle = NFA_HANDLE_INVALID;
    unsigned long num = 0;
    if (GetNumValue("UICC_LISTEN_TECH_MASK", &num, sizeof(num)))
        mUiccListenMask = num;
    else
    {
        mUiccListenMask = (NFA_TECHNOLOGY_MASK_A |
                           NFA_TECHNOLOGY_MASK_B |
                           NFA_TECHNOLOGY_MASK_F |
                           NFA_TECHNOLOGY_MASK_B_PRIME);
    }
    ALOGD ("%s: UICC_LISTEN_TECH_MASK = 0x%X", fn, mUiccListenMask);

    {
        SyncEventGuard guard (mEeRegisterEvent);
        ALOGD ("%s: try ee register", fn);
        nfaStat = NFA_EeRegister (nfaEeCallback);
        if (nfaStat != NFA_STATUS_OK)
        {
            ALOGE ("%s: fail ee register; error=0x%X", fn, nfaStat);
            return false;
        }
        mEeRegisterEvent.wait ();
        mIsEeRegistered = true;
    }

    // collect information of secure element
    getEeInfo();

    mRxDataBuffer.clear ();

    initializeT3TagEmulation();

    return true;
}

RoutingManager& RoutingManager::getInstance ()
{
    static RoutingManager manager;
    return manager;
}

void RoutingManager::enableRoutingToHost()
{
    static const char fn [] = "RoutingManager::enableRoutingToHost()";
    if (mIsRoutingToHost)
    {
        ALOGD ("%s: already routing to host", fn);
        return;
    }

    mIsRoutingToHost = true;
    updateRouting();
}

void RoutingManager::disableRoutingToHost()
{
    static const char fn [] = "RoutingManager::disableRoutingToHost";

    if (!mIsRoutingToHost)
    {
        ALOGD ("%s: not routing to host", fn);
        return;
    }

    mIsRoutingToHost = false;
    updateRouting();
}

void RoutingManager::enableRoutingToOffHost(int nfceeId)
{
    static const char fn [] = "RoutingManager::enableRoutingToOffHost";
    tNFA_TECHNOLOGY_MASK techMask = getOffHostTechMask(nfceeId);

    if ((mIsRoutingToSE) &&
        (mOffHostEe == nfceeId) &&
        (mActiveSeTechMask == techMask))
    {
        ALOGD ("%s: already routing to SE", fn);
        return;
    }

    clearDefaultTechRouting(NFA_HANDLE_GROUP_EE|(tNFA_HANDLE)mOffHostEe);
    // clear any protocol routing including T2T
    clearDefaultProtoRouting(NFA_HANDLE_GROUP_EE|(tNFA_HANDLE)mOffHostEe);

    mOffHostEe   = nfceeId;
    mActiveSeTechMask = techMask;
    mIsRoutingToSE    = true;
    updateRouting();
}

void RoutingManager::disableRoutingToOffHost()
{
    static const char fn [] = "RoutingManager::disableRoutingToOffHost";

    if (!mIsRoutingToSE)
    {
        ALOGD ("%s: not routing to SE", fn);
        return;
    }

    clearDefaultTechRouting(NFA_HANDLE_GROUP_EE|(tNFA_HANDLE)mOffHostEe);
    // clear any protocol routing including T2T
    clearDefaultProtoRouting(NFA_HANDLE_GROUP_EE|(tNFA_HANDLE)mOffHostEe);

    mOffHostEe = 0;
    mIsRoutingToSE = false;
    updateRouting();
}

void RoutingManager::updateRouting()
{
    static const char fn [] = "RoutingManager::updateRouting()";
    tNFA_STATUS nfaStat;
    SyncEventGuard guard (mRoutingEvent);
    tNFA_TECHNOLOGY_MASK techMaskForDH;
    tNFA_TECHNOLOGY_MASK techMaskForSE;
    tNFA_PROTOCOL_MASK   protoMaskForDH;
    tNFA_PROTOCOL_MASK   protoMaskForSE;

    ALOGD ("%s: enter; mIsRoutingToHost=%d, mIsRoutingToSE=%d", fn, mIsRoutingToHost, mIsRoutingToSE);

    if (mIsRoutingToHost)
    {
        if (mIsRoutingToSE)
        {
            if ((mActiveSeTechMask & (NFA_TECHNOLOGY_MASK_A|NFA_TECHNOLOGY_MASK_B)) == 0)
                techMaskForDH = NFA_TECHNOLOGY_MASK_A;
            else
                techMaskForDH = 0;
            techMaskForSE  = mActiveSeTechMask;
            protoMaskForDH = NFA_PROTOCOL_MASK_NFC_DEP;
            protoMaskForSE = NFA_PROTOCOL_MASK_T2T;
        }
        else
        {
            techMaskForDH  = NFA_TECHNOLOGY_MASK_A;
            techMaskForSE  = 0;
            protoMaskForDH = NFA_PROTOCOL_MASK_NFC_DEP;
            protoMaskForSE = 0;
        }
    }
    else
    {
        if (mIsRoutingToSE)
        {
            techMaskForDH  = 0;
            techMaskForSE  = mActiveSeTechMask;
            protoMaskForDH = NFA_PROTOCOL_MASK_NFC_DEP;
            protoMaskForSE = NFA_PROTOCOL_MASK_T2T;
        }
        else
        {
            techMaskForDH  = 0;
            techMaskForSE  = 0;
            protoMaskForDH = NFA_PROTOCOL_MASK_NFC_DEP;
            protoMaskForSE = 0;
        }
    }

    // Default routing for NFC-A/B technology
    {
        nfaStat = NFA_EeSetDefaultTechRouting (NFA_EE_HANDLE_DH, techMaskForDH, 0, 0);
        if (nfaStat == NFA_STATUS_OK)
            mRoutingEvent.wait ();
        else
            ALOGE ("Fail to set default tech routing to DH");
    }

    if (mOffHostEe)
    {
        tNFA_TECHNOLOGY_MASK techMask = techMaskForSE&(NFA_TECHNOLOGY_MASK_A | NFA_TECHNOLOGY_MASK_B);
        nfaStat = NFA_EeSetDefaultTechRouting (mOffHostEe, techMask, techMask, 0);
        if (nfaStat == NFA_STATUS_OK)
            mRoutingEvent.wait ();
        else
            ALOGE ("Fail to set default tech routing to NFCEE=0x%X", mOffHostEe);
    }

    // Default routing for protocols
    {
        if (mDefaultEe == NFC_DH_ID)
            protoMaskForDH |= NFA_PROTOCOL_MASK_ISO_DEP;

        nfaStat = NFA_EeSetDefaultProtoRouting(NFA_EE_HANDLE_DH, protoMaskForDH, 0, 0);
        if (nfaStat == NFA_STATUS_OK)
            mRoutingEvent.wait ();
        else
            ALOGE ("Fail to set default proto routing to DH");
    }

    if ((mOffHostEe) || (mDefaultEe != NFC_DH_ID))
    {
        if (mDefaultEe != NFC_DH_ID)
        {
            if (mOffHostEe == mDefaultEe)
            {
                protoMaskForSE |= NFA_PROTOCOL_MASK_ISO_DEP;
            }
            else if (getOffHostTechMask(mDefaultEe)) // if secure element of default route is present
            {
                nfaStat = NFA_EeSetDefaultProtoRouting(mDefaultEe, NFA_PROTOCOL_MASK_ISO_DEP, NFA_PROTOCOL_MASK_ISO_DEP, 0);
                if (nfaStat == NFA_STATUS_OK)
                    mRoutingEvent.wait ();
                else
                    ALOGE ("Fail to set default ISO-DEP proto routing to NFCEE=0x%X", mDefaultEe);
            }
        }

        if (techMaskForSE)
        {
            nfaStat = NFA_EeSetDefaultProtoRouting(mOffHostEe, protoMaskForSE, protoMaskForSE, 0);
            if (nfaStat == NFA_STATUS_OK)
                mRoutingEvent.wait ();
            else
                ALOGE ("Fail to set default proto routing to NFCEE=0x%X", mOffHostEe);
        }
    }

    if (mIsRoutingToSE || mIsRoutingToHost)
    {
        tNFA_TECHNOLOGY_MASK techMask;
        techMask = ((techMaskForDH|techMaskForSE)&(NFA_TECHNOLOGY_MASK_A|NFA_TECHNOLOGY_MASK_B));
        if (techMask)
        {
            nfaStat = NFA_CeSetIsoDepListenTech(techMask);
            if (nfaStat != NFA_STATUS_OK)
                ALOGE ("Failed to configure CE IsoDep technologies");
        }
    }

    if ((mIsRoutingToSE) && (techMaskForDH|techMaskForSE))
    {
        if (mUiccListenTechHandle != (NFA_HANDLE_GROUP_EE|(tNFA_HANDLE)mOffHostEe))
        {
            if (mUiccListenTechHandle != 0)
            {
                // Tell the UICC to stop listening to tech
                nfaStat = NFA_CeConfigureUiccListenTech (mUiccListenTechHandle, 0);
                if (nfaStat == NFA_STATUS_OK)
                {
                    mRoutingEvent.wait ();
                    mUiccListenTechHandle = 0;
                }
                else
                    ALOGE ("Fail to stop UICC listen for old handle");
            }

            mUiccListenTechHandle = (NFA_HANDLE_GROUP_EE|(tNFA_HANDLE)mOffHostEe);

            // Tell the UICC to listen to tech
            nfaStat = NFA_CeConfigureUiccListenTech (mUiccListenTechHandle, techMaskForDH|techMaskForSE);
            if (nfaStat == NFA_STATUS_OK)
                mRoutingEvent.wait ();
            else
                ALOGE ("Failed to configure UICC listen technologies");
        }
    }
    else if (mUiccListenTechHandle)
    {
        // Tell the UICC to stop listening to tech
        nfaStat = NFA_CeConfigureUiccListenTech (mUiccListenTechHandle, 0);
        if (nfaStat == NFA_STATUS_OK)
        {
            mRoutingEvent.wait ();
            mUiccListenTechHandle = 0;
        }
        else
            ALOGE ("Fail to stop UICC listen");
    }

    //de-register and then register to remove added entry
    //of NFC_Tech_B for UICC/SE when switched back to HSE
    if (mCeAidHandle != NFA_HANDLE_INVALID)
    {
        // Deregister a wild-card for AIDs routed to the host
        nfaStat = NFA_CeDeregisterAidOnDH (mCeAidHandle);
        if (nfaStat == NFA_STATUS_OK)
            mRoutingEvent.wait ();
        else
            ALOGE("Failed to register wildcard AID for DH");
    }

    if (mIsRoutingToHost)
    {
        if (mCeAidHandle == NFA_HANDLE_INVALID)
        {
            // Register a wild-card for AIDs routed to the host
            nfaStat = NFA_CeRegisterAidOnDH (NULL, 0, stackCallback);
            if (nfaStat == NFA_STATUS_OK)
                mRoutingEvent.wait ();
            else
                ALOGE("Failed to register wildcard AID for DH");
        }
    }

    ALOGD ("%s: exit", fn);
}

void RoutingManager::connectionEventHandler (UINT8 event, tNFA_CONN_EVT_DATA* )
{
    RoutingManager& routingManager = RoutingManager::getInstance();

    switch (event)
    {
    case NFA_CE_UICC_LISTEN_CONFIGURED_EVT:
        {
            SyncEventGuard guard(routingManager.mRoutingEvent);
            routingManager.mRoutingEvent.notifyOne();
        }
        break;
    }
}

bool RoutingManager::setDefaultRoute(int nfceeId)
{
    static const char fn [] = "RoutingManager::setDefaultRoute";
    ALOGD ("%s: defaultRoute = %d", fn, nfceeId);

    if (nfceeId < 0)
    {
        ALOGE ("%s: invalid route", fn);
        return false;
    }

    if (mDefaultEe == nfceeId)
    {
        ALOGE ("%s: default is not changed", fn);
        return false;
    }
    clearDefaultProtoRouting((NFA_HANDLE_GROUP_EE|(tNFA_HANDLE)mDefaultEe));

    mDefaultEe = nfceeId;
    if (mIsEeRegistered)
    {
        updateRouting();
    }
    return true;
}

int RoutingManager::getDefaultRoute()
{
    return mDefaultEe;
}

void RoutingManager::clearDefaultTechRouting(tNFA_HANDLE eeHandle)
{
    tNFA_STATUS nfaStat;

    SyncEventGuard guard (mRoutingEvent);

    nfaStat = NFA_EeSetDefaultTechRouting (eeHandle, 0, 0, 0);
    if (nfaStat == NFA_STATUS_OK)
        mRoutingEvent.wait ();
    else
        ALOGE ("Fail to clear default tech routing to 0x%x", eeHandle);
}

void RoutingManager::clearDefaultProtoRouting(tNFA_HANDLE eeHandle)
{
    tNFA_STATUS nfaStat;

    SyncEventGuard guard (mRoutingEvent);

    nfaStat = NFA_EeSetDefaultProtoRouting(eeHandle, 0, 0, 0);
    if (nfaStat == NFA_STATUS_OK)
        mRoutingEvent.wait ();
    else
        ALOGE ("Fail to clear default proto routing to 0x%x", eeHandle);
}

#define SUB_SET_MATCH   0x10 //a match is allowed when the SELECT AID is shorter than the AID in this routing table entry
#define SUPER_SET_MATCH 0x08 //a match is allowed when the SELECT AID is longer than the AID in this routing table entry

bool RoutingManager::addAidRouting(const UINT8* aid, UINT8 aidLen, int route, bool isSubSet, bool isSuperSet)
{
    static const char fn [] = "RoutingManager::addAidRouting";
    tNFA_STATUS nfaStat;
    ALOGD ("%s: enter", fn);
    tNFA_EE_PWR_STATE power_match;

    if (route == NFC_DH_ID)
        power_match = NFA_EE_PWR_STATE_ON;
    else
        power_match = (NFA_EE_PWR_STATE_ON | NFA_EE_PWR_STATE_SWITCH_OFF);

    if (isSubSet)
        power_match |= SUB_SET_MATCH;

    if (isSuperSet)
        power_match |= SUPER_SET_MATCH;

    nfaStat = NFA_EeAddAidRouting(route, aidLen, (UINT8*) aid, power_match);
    if (nfaStat == NFA_STATUS_OK)
    {
        ALOGD ("%s: routed AID", fn);
        return true;
    } else
    {
        ALOGE ("%s: failed to route AID", fn);
        return false;
    }
}

bool RoutingManager::removeAidRouting(const UINT8* aid, UINT8 aidLen)
{
    static const char fn [] = "RoutingManager::removeAidRouting";
    ALOGD ("%s: enter", fn);
    tNFA_STATUS nfaStat = NFA_EeRemoveAidRouting(aidLen, (UINT8*) aid);
    if (nfaStat == NFA_STATUS_OK)
    {
        ALOGD ("%s: removed AID", fn);
        return true;
    } else
    {
        ALOGE ("%s: failed to remove AID", fn);
        return false;
    }
}

bool RoutingManager::commitRouting()
{
    static const char fn [] = "RoutingManager::commitRouting";
    tNFA_STATUS nfaStat = 0;
    ALOGD ("%s", fn);
    {
        SyncEventGuard guard (mEeUpdateEvent);
        nfaStat = NFA_EeUpdateNow();
        if (nfaStat == NFA_STATUS_OK)
        {
            mEeUpdateEvent.wait (); //wait for NFA_EE_UPDATED_EVT
        }
    }
    return (nfaStat == NFA_STATUS_OK);
}

void RoutingManager::notifyActivated ()
{
    ApduGateManager::getInstance().notifyListenModeState (true);

    JNIEnv* e = NULL;
    ScopedAttach attach(mNativeData->vm, &e);
    if (e == NULL)
    {
        ALOGE ("jni env is null");
        return;
    }

    e->CallVoidMethod (mNativeData->manager, android::gCachedNfcManagerNotifyHostEmuActivated);
    if (e->ExceptionCheck())
    {
        e->ExceptionClear();
        ALOGE ("fail notify");
    }
}

void RoutingManager::notifyDeactivated ()
{
    ApduGateManager::getInstance().notifyListenModeState (false);

    mRxDataBuffer.clear();
    JNIEnv* e = NULL;
    ScopedAttach attach(mNativeData->vm, &e);
    if (e == NULL)
    {
        ALOGE ("jni env is null");
        return;
    }

    e->CallVoidMethod (mNativeData->manager, android::gCachedNfcManagerNotifyHostEmuDeactivated);
    if (e->ExceptionCheck())
    {
        e->ExceptionClear();
        ALOGE ("fail notify");
    }
}

void RoutingManager::handleData (const UINT8* data, UINT32 dataLen, tNFA_STATUS status)
{
    if (dataLen <= 0)
    {
        ALOGE("no data");
        goto TheEnd;
    }

    if (status == NFA_STATUS_CONTINUE)
    {
        mRxDataBuffer.insert (mRxDataBuffer.end(), &data[0], &data[dataLen]); //append data; more to come
        return; //expect another NFA_CE_DATA_EVT to come
    }
    else if (status == NFA_STATUS_OK)
    {
        mRxDataBuffer.insert (mRxDataBuffer.end(), &data[0], &data[dataLen]); //append data
        //entire data packet has been received; no more NFA_CE_DATA_EVT
    }
    else if (status == NFA_STATUS_FAILED)
    {
        ALOGE("RoutingManager::handleData: read data fail");
        goto TheEnd;
    }

    {
        JNIEnv* e = NULL;
        ScopedAttach attach(mNativeData->vm, &e);
        if (e == NULL)
        {
            ALOGE ("jni env is null");
            goto TheEnd;
        }

        ScopedLocalRef<jobject> dataJavaArray(e, e->NewByteArray(mRxDataBuffer.size()));
        if (dataJavaArray.get() == NULL)
        {
            ALOGE ("fail allocate array");
            goto TheEnd;
        }

        e->SetByteArrayRegion ((jbyteArray)dataJavaArray.get(), 0, mRxDataBuffer.size(),
                (jbyte *)(&mRxDataBuffer[0]));
        if (e->ExceptionCheck())
        {
            e->ExceptionClear();
            ALOGE ("fail fill array");
            goto TheEnd;
        }

        e->CallVoidMethod (mNativeData->manager, android::gCachedNfcManagerNotifyHostEmuData, dataJavaArray.get());
        if (e->ExceptionCheck())
        {
            e->ExceptionClear();
            ALOGE ("fail notify");
        }
    }
TheEnd:
    mRxDataBuffer.clear();
}

int RoutingManager::getSecureElementIdList(int nfceeId[])
{
    int xx;

    for (xx = 0; xx < mActualNumEe; xx++)
    {
        nfceeId[xx] = mEeInfo[xx].ee_handle & ~NFA_HANDLE_GROUP_EE;
    }
    return mActualNumEe;
}

int RoutingManager::getDefaultOffHostRouteDestination()
{
    return mOffHostEe;
}

void RoutingManager::getEeInfo()
{
    static const char fn [] = "RoutingManager::getEeInfo";
    tNFA_STATUS nfaStat;

    ALOGD("%s: enter", fn);

    mActualNumEe = NFA_EE_MAX_EE_SUPPORTED;
    mNumActivatedEe = 0;

    if ((nfaStat = NFA_EeGetInfo (&mActualNumEe, mEeInfo)) != NFA_STATUS_OK)
    {
        ALOGE ("%s: fail get info; error=0x%X", fn, nfaStat);
        mActualNumEe = 0;
    }
    else
    {
        ALOGD ("%s: num EEs discovered: %u", fn, mActualNumEe);
        for (UINT8 xx = 0; xx < mActualNumEe; xx++)
        {
            ALOGD ("%s: EE[%u] Handle: 0x%04x  Status: %s",
                   fn, xx, mEeInfo[xx].ee_handle, eeStatusToString(mEeInfo[xx].ee_status));

            if (mEeInfo[xx].ee_status == NFC_NFCEE_STATUS_INACTIVE)
            {
                SyncEventGuard guard (mEeSetModeEvent);
                ALOGD ("%s: set EE mode activate; h=0x%X", fn, mEeInfo[xx].ee_handle);
                if ((nfaStat = NFA_EeModeSet (mEeInfo[xx].ee_handle, NFA_EE_MD_ACTIVATE)) == NFA_STATUS_OK)
                {
                    mEeSetModeEvent.wait (); //wait for NFA_EE_MODE_SET_EVT
                    if (mEeInfo[xx].ee_status == NFC_NFCEE_STATUS_ACTIVE)
                        mNumActivatedEe++;
                }
                else
                    ALOGE ("%s: NFA_EeModeSet failed; error=0x%X", fn, nfaStat);
            }
            else if (mEeInfo[xx].ee_status == NFC_NFCEE_STATUS_ACTIVE)
            {
                mNumActivatedEe++;
            }
        }
    }

    ALOGD ("%s: exit; mActualNumEe=%d, mNumActivatedEe=%d", fn, mActualNumEe, mNumActivatedEe);
}

void RoutingManager::storeEeDiscReqInfo (tNFA_EE_DISCOVER_REQ& discReq)
{
    static const char fn [] = "RoutingManager::storeEeDiscReqInfo";
    ALOGD ("%s:  Status: %u   Num EE: %u", fn, discReq.status, discReq.num_ee);

    SyncEventGuard guard (mEeDiscReqEvent);
    memcpy (&mEeDiscReqInfo, &discReq, sizeof(mEeDiscReqInfo));
    for (UINT8 xx = 0; xx < discReq.num_ee; xx++)
    {
        //for each technology (A, B, F, B'), print the bit field that shows
        //what protocol(s) is support by that technology
        ALOGD ("%s   EE[%u] Handle: 0x%04x  techA: 0x%02x  techB: 0x%02x  techF: 0x%02x  techBprime: 0x%02x",
                fn, xx, discReq.ee_disc_info[xx].ee_handle,
                discReq.ee_disc_info[xx].la_protocol,
                discReq.ee_disc_info[xx].lb_protocol,
                discReq.ee_disc_info[xx].lf_protocol,
                discReq.ee_disc_info[xx].lbp_protocol);
    }
    mEeDiscReqEvent.notifyOne ();
}

tNFA_TECHNOLOGY_MASK RoutingManager::getOffHostTechMask(int nfceeId)
{
    static const char fn [] = "RoutingManager::getOffHostTechMask";
    tNFA_TECHNOLOGY_MASK techMask = 0;

    ALOGD ("%s:  nfceeId: 0x%02x", fn, nfceeId);
    for (UINT8 xx = 0; xx < mEeDiscReqInfo.num_ee; xx++)
    {
        if ((mEeDiscReqInfo.ee_disc_info[xx].ee_handle & NFA_HANDLE_MASK) == nfceeId )
        {
            if (mEeDiscReqInfo.ee_disc_info[xx].la_protocol)
                techMask |= NFA_TECHNOLOGY_MASK_A;
            if (mEeDiscReqInfo.ee_disc_info[xx].lb_protocol)
                techMask |= NFA_TECHNOLOGY_MASK_B;
            if (mEeDiscReqInfo.ee_disc_info[xx].lf_protocol)
                techMask |= NFA_TECHNOLOGY_MASK_F;
            if (mEeDiscReqInfo.ee_disc_info[xx].lbp_protocol)
                techMask |= NFA_TECHNOLOGY_MASK_B_PRIME;
        }
    }

    ALOGD ("%s: techMask: 0x%02x, (techMask & mUiccListenMask): 0x%02x", fn, techMask, (techMask & mUiccListenMask));
    return (techMask & mUiccListenMask);
}

void RoutingManager::handleModeSetRsp(tNFA_EE_MODE_SET& mode_set)
{
    static const char fn [] = "RoutingManager::handleModeSetRsp";
    ALOGD ("%s:  ee_handle: %04x, status:%s", fn, mode_set.ee_handle, eeStatusToString(mode_set.status));

    for (UINT8 xx = 0; xx < mActualNumEe; xx++)
    {
        if (mEeInfo[xx].ee_handle == mode_set.ee_handle)
        {
            mEeInfo[xx].ee_status = mode_set.status;
            break;
        }
    }
    SyncEventGuard guard (mEeSetModeEvent);
    mEeSetModeEvent.notifyOne();
}

const char* RoutingManager::eeStatusToString (UINT8 status)
{
    switch (status)
    {
    case NFC_NFCEE_STATUS_ACTIVE:
        return("Connected/Active");
    case NFC_NFCEE_STATUS_INACTIVE:
        return("Connected/Inactive");
    case NFC_NFCEE_STATUS_REMOVED:
        return("Removed");
    }
    return("?? Unknown ??");
}

void RoutingManager::notifyTransactionListenersOfAid (const UINT8* aidBuffer, UINT8 aidBufferLen)
{
    static const char fn [] = "RoutingManager::notifyTransactionListenersOfAid";
    ALOGD ("%s: enter; aid len=%u", fn, aidBufferLen);

    if (aidBufferLen == 0) {
        return;
    }

    JNIEnv* e = NULL;
    ScopedAttach attach(mNativeData->vm, &e);
    if (e == NULL)
    {
        ALOGE ("%s: jni env is null", fn);
        return;
    }

    const UINT16 tlvMaxLen = aidBufferLen + 10;
    UINT8* tlv = new UINT8 [tlvMaxLen];
    if (tlv == NULL)
    {
        ALOGE ("%s: fail allocate tlv", fn);
        return;
    }

    memcpy (tlv, aidBuffer, aidBufferLen);
    UINT16 tlvActualLen = aidBufferLen;

    ScopedLocalRef<jobject> tlvJavaArray(e, e->NewByteArray(tlvActualLen));
    if (tlvJavaArray.get() == NULL)
    {
        ALOGE ("%s: fail allocate array", fn);
        goto TheEnd;
    }

    e->SetByteArrayRegion ((jbyteArray)tlvJavaArray.get(), 0, tlvActualLen, (jbyte *)tlv);
    if (e->ExceptionCheck())
    {
        e->ExceptionClear();
        ALOGE ("%s: fail fill array", fn);
        goto TheEnd;
    }

    e->CallVoidMethod (mNativeData->manager, android::gCachedNfcManagerNotifyTransactionListeners, tlvJavaArray.get());
    if (e->ExceptionCheck())
    {
        e->ExceptionClear();
        ALOGE ("%s: fail notify", fn);
        goto TheEnd;
    }

TheEnd:
    delete [] tlv;
    ALOGD ("%s: exit", fn);
}

/*******************************************************************************
**
** Function:        notifyHciEventConnectivity
**
** Description:     Notify the NFC service about EVT_CONNECTIVITY from secure element.
**                  NFCEE ID: secure element
**
** Returns:         None
**
*******************************************************************************/
void RoutingManager::notifyHciEventConnectivity (UINT8 nfceeId)
{
    static const char fn [] = "RoutingManager::notifyHciEventConnectivity";
    jint seId = nfceeId;

    ALOGD ("%s: enter; nfceeId=%u", fn, nfceeId);

    JNIEnv* e = NULL;
    ScopedAttach attach(mNativeData->vm, &e);
    if (e == NULL)
    {
        ALOGE ("%s: jni env is null", fn);
        return;
    }

    e->CallVoidMethod (mNativeData->manager, android::gCachedNfcManagerNotifyHciEventConnectivity, seId);
    if (e->ExceptionCheck())
    {
        e->ExceptionClear();
        ALOGE ("%s: fail notify", fn);
    }

    ALOGD ("%s: exit", fn);
}

void RoutingManager::stackCallback (UINT8 event, tNFA_CONN_EVT_DATA* eventData)
{
    static const char fn [] = "RoutingManager::stackCallback";
    ALOGD("%s: event=0x%X", fn, event);
    RoutingManager& routingManager = RoutingManager::getInstance();

    switch (event)
    {
    case NFA_CE_REGISTERED_EVT:
        {
            tNFA_CE_REGISTERED& ce_registered = eventData->ce_registered;
            ALOGD("%s: NFA_CE_REGISTERED_EVT; status=0x%X; h=0x%X", fn, ce_registered.status, ce_registered.handle);
            routingManager.mCeAidHandle = ce_registered.handle;
            SyncEventGuard guard(routingManager.mRoutingEvent);
            routingManager.mRoutingEvent.notifyOne();
        }
        break;

    case NFA_CE_DEREGISTERED_EVT:
        {
            tNFA_CE_DEREGISTERED& ce_deregistered = eventData->ce_deregistered;
            ALOGD("%s: NFA_CE_DEREGISTERED_EVT; h=0x%X", fn, ce_deregistered.handle);
            routingManager.mCeAidHandle = NFA_HANDLE_INVALID;
            SyncEventGuard guard(routingManager.mRoutingEvent);
            routingManager.mRoutingEvent.notifyOne();
        }
        break;

    case NFA_CE_ACTIVATED_EVT:
        {
            routingManager.notifyActivated();
        }
        break;
    case NFA_DEACTIVATED_EVT:
    case NFA_CE_DEACTIVATED_EVT:
        {
            routingManager.notifyDeactivated();
        }
        break;
    case NFA_CE_DATA_EVT:
        {
            tNFA_CE_DATA& ce_data = eventData->ce_data;
            ALOGD("%s: NFA_CE_DATA_EVT; stat=0x%X; h=0x%X; data len=%u", fn, ce_data.status, ce_data.handle, ce_data.len);
            getInstance().handleData(ce_data.p_data, ce_data.len, ce_data.status);
        }
        break;
    }
}
/*******************************************************************************
**
** Function:        nfaEeCallback
**
** Description:     Receive execution environment-related events from stack.
**                  event: Event code.
**                  eventData: Event data.
**
** Returns:         None
**
*******************************************************************************/
void RoutingManager::nfaEeCallback (tNFA_EE_EVT event, tNFA_EE_CBACK_DATA* eventData)
{
    static const char fn [] = "RoutingManager::nfaEeCallback";

    RoutingManager& routingManager = RoutingManager::getInstance();

    switch (event)
    {
    case NFA_EE_REGISTER_EVT:
        {
            SyncEventGuard guard (routingManager.mEeRegisterEvent);
            ALOGD ("%s: NFA_EE_REGISTER_EVT; status=%u", fn, eventData->ee_register);
            routingManager.mEeRegisterEvent.notifyOne();
        }
        break;

    case NFA_EE_MODE_SET_EVT:
        {
            ALOGD ("%s: NFA_EE_MODE_SET_EVT; status: 0x%04X  handle: 0x%04X  ", fn,
                    eventData->mode_set.status, eventData->mode_set.ee_handle);
            routingManager.handleModeSetRsp(eventData->mode_set);
        }
        break;

    case NFA_EE_SET_TECH_CFG_EVT:
        {
            ALOGD ("%s: NFA_EE_SET_TECH_CFG_EVT; status=0x%X", fn, eventData->status);
            SyncEventGuard guard(routingManager.mRoutingEvent);
            routingManager.mRoutingEvent.notifyOne();
        }
        break;

    case NFA_EE_SET_PROTO_CFG_EVT:
        {
            ALOGD ("%s: NFA_EE_SET_PROTO_CFG_EVT; status=0x%X", fn, eventData->status);
            SyncEventGuard guard(routingManager.mRoutingEvent);
            routingManager.mRoutingEvent.notifyOne();
        }
        break;

    case NFA_EE_ACTION_EVT:
        {
            tNFA_EE_ACTION& action = eventData->action;
            if (action.trigger == NFC_EE_TRIG_SELECT)
                ALOGD ("%s: NFA_EE_ACTION_EVT; h=0x%X; trigger=select (0x%X)", fn, action.ee_handle, action.trigger);
            else if (action.trigger == NFC_EE_TRIG_APP_INIT)
            {
                int i;

                tNFC_APP_INIT& app_init = action.param.app_init;
                ALOGD ("%s: NFA_EE_ACTION_EVT; h=0x%X; trigger=app-init (0x%X); aid len=%u; data len=%u", fn,
                        action.ee_handle, action.trigger, app_init.len_aid, app_init.len_data);

                // NFCEE ID (1 byte) + length of AID(1 byte) + AID + length of Param(1 byte) + Param
                UINT8* dataBuf = new UINT8 [2 + app_init.len_aid + 1 + app_init.len_data];
                // NFCEE ID (1 byte)
                dataBuf[0] = (UINT8)(action.ee_handle & 0x00FF);
                // length of AID (1 byte)
                dataBuf[1] = app_init.len_aid;
                // AID
                for (i = 0; i < app_init.len_aid; i++)
                    dataBuf[2+i] = app_init.aid[i];

                // length of Param(1 byte)
                dataBuf[2+app_init.len_aid] = app_init.len_data;
                // Param
                for (i = 0; i < app_init.len_data; i++)
                    dataBuf[2+app_init.len_aid+1+i] = app_init.data[i];

                routingManager.notifyTransactionListenersOfAid (dataBuf, 3 + app_init.len_aid + app_init.len_data);
                delete [] dataBuf;
            }
            else if (action.trigger == NCI_EE_TRIG_CONNECTIVITY)
            {
                ALOGD ("%s: NFA_EE_ACTION_EVT; h=0x%X; trigger=connectivity (0x%X)", fn, action.ee_handle, action.trigger);
                routingManager.notifyHciEventConnectivity ((UINT8) (NFA_HANDLE_MASK & action.ee_handle));
            }
            else if (action.trigger == NFC_EE_TRIG_RF_PROTOCOL)
                ALOGD ("%s: NFA_EE_ACTION_EVT; h=0x%X; trigger=rf protocol (0x%X)", fn, action.ee_handle, action.trigger);
            else if (action.trigger == NFC_EE_TRIG_RF_TECHNOLOGY)
                ALOGD ("%s: NFA_EE_ACTION_EVT; h=0x%X; trigger=rf tech (0x%X)", fn, action.ee_handle, action.trigger);
            else
                ALOGE ("%s: NFA_EE_ACTION_EVT; h=0x%X; unknown trigger (0x%X)", fn, action.ee_handle, action.trigger);
        }
        break;

    case NFA_EE_DISCOVER_REQ_EVT:
        ALOGD ("%s: NFA_EE_DISCOVER_REQ_EVT; status=0x%X; num ee=%u", __FUNCTION__,
                eventData->discover_req.status, eventData->discover_req.num_ee);
        routingManager.storeEeDiscReqInfo (eventData->discover_req);
        break;

    case NFA_EE_NO_CB_ERR_EVT:
        ALOGD ("%s: NFA_EE_NO_CB_ERR_EVT  status=%u", fn, eventData->status);
        break;

    case NFA_EE_ADD_AID_EVT:
        {
            ALOGD ("%s: NFA_EE_ADD_AID_EVT  status=%u", fn, eventData->status);
        }
        break;

    case NFA_EE_REMOVE_AID_EVT:
        {
            ALOGD ("%s: NFA_EE_REMOVE_AID_EVT  status=%u", fn, eventData->status);
        }
        break;

    case NFA_EE_NEW_EE_EVT:
        {
            ALOGD ("%s: NFA_EE_NEW_EE_EVT  h=0x%X; status=%u", fn,
                eventData->new_ee.ee_handle, eventData->new_ee.ee_status);
        }
        break;

    case NFA_EE_UPDATED_EVT:
        {
            ALOGD("%s: NFA_EE_UPDATED_EVT", fn);
            SyncEventGuard guard(routingManager.mEeUpdateEvent);
            routingManager.mEeUpdateEvent.notifyOne();
        }
        break;

    default:
        ALOGE ("%s: unknown event=%u ????", fn, event);
        break;
    }
}

int RoutingManager::registerJniFunctions (JNIEnv* e)
{
    static const char fn [] = "RoutingManager::registerJniFunctions";
    ALOGD ("%s", fn);
    return jniRegisterNativeMethods (e, "com/android/nfc/cardemulation/AidRoutingManager", sMethods, NELEM(sMethods));
}

int RoutingManager::com_android_nfc_cardemulation_doGetDefaultRouteDestination (JNIEnv*)
{
    return getInstance().mDefaultEe;
}

int RoutingManager::com_android_nfc_cardemulation_doGetDefaultOffHostRouteDestination (JNIEnv*)
{
    return getInstance().mOffHostEe;
}

int RoutingManager::com_android_nfc_cardemulation_doGetAidMatchingMode (JNIEnv*)
{
    return getInstance().mAidMatchingMode;
}

int RoutingManager::com_android_nfc_cardemulation_doGetNumSecureElement (JNIEnv*)
{
    return (int)getInstance().mNumActivatedEe;
}

//This Function initializes T3TagEmulation on Host, based on the inputs in the config file
void RoutingManager::initializeT3TagEmulation()
{
    static const char fn[] = "RoutingManager::initializeT3TagEmulation()";
    ALOGD("%s: enter", fn);

    //Test App1 - Emulates 1 Tag
    test_app1_nfcid2_size = GetStrValue("TEST_APP1_NFCID2", (char*)test_app1_nfcid2, sizeof(test_app1_nfcid2));
    if (test_app1_nfcid2_size > 0)
    {
      if (test_app1_nfcid2_size == 8)
        ALOGD("%s TEST_APP1_NFCID2 with size %x ", __FUNCTION__, test_app1_nfcid2_size);
      else
      {
        ALOGE("%s TEST_APP1_NFCID2 with invalid size %x ", __FUNCTION__, test_app1_nfcid2_size);
        test_app1_nfcid2_size = 0;
      }
    }

    test_app1_sysCode_size = GetStrValue("TEST_APP1_SYSTEM_CODE", (char*)test_app1_sysCode, sizeof(test_app1_sysCode));
    if (test_app1_sysCode_size > 0)
    {
      if (test_app1_sysCode_size == 2)
        ALOGD("%s TEST_APP1_SYSTEM_CODE with size %x ", __FUNCTION__, test_app1_sysCode_size);
      else
      {
        ALOGE("%s TEST_APP1_SYSTEM_CODE with invalid size %x ", __FUNCTION__, test_app1_sysCode_size);
        test_app1_sysCode_size = 0;
      }
    }

    if (test_app1_sysCode_size && test_app1_nfcid2_size)
    {
      UINT16 i = 0, sysCode = 0;

      //Storing the 2byte system code into 1 syscode integer
      for (i = 0; i < 2; i++)
        sysCode = (sysCode << (8 * i)) | test_app1_sysCode[i];

      SyncEventGuard guard(mRoutingT3tEvent);
      tNFA_STATUS nfaStat = NFA_CeRegisterFelicaSystemCodeOnDH(sysCode, test_app1_nfcid2, stackCallbackForT3T);
      if (nfaStat == NFA_STATUS_OK)
        mRoutingT3tEvent.wait();
      else
        ALOGE("Failed to register FIRST T3T emulation on DH");
    }
    ALOGD("TEST_APP1 Registered");

    //Test App2- > Emulates second Tag.
    test_app2_nfcid2_size = GetStrValue("TEST_APP2_NFCID2", (char*)test_app2_nfcid2, sizeof(test_app2_nfcid2));
    if (test_app2_nfcid2_size > 0)
    {
      if (test_app2_nfcid2_size == 8)
        ALOGD("%s TEST_APP2_NFCID2 with size %x ", __FUNCTION__, test_app2_nfcid2_size);
      else
      {
        ALOGE("%s TEST_APP2_NFCID2 with invalid size %x ", __FUNCTION__, test_app2_nfcid2_size);
        test_app2_nfcid2_size = 0;
      }
    }

    test_app2_sysCode_size = GetStrValue("TEST_APP2_SYSTEM_CODE", (char*)test_app2_sysCode, sizeof(test_app2_sysCode));
    if (test_app2_sysCode_size > 0)
    {
      if (test_app2_sysCode_size == 2)
        ALOGD("%s TEST_APP2_SYSTEM_CODE with size %x ", __FUNCTION__, test_app2_sysCode_size);
      else
      {
        ALOGE("%s TEST_APP2_SYSTEM_CODE with invalid size %x ", __FUNCTION__, test_app2_sysCode_size);
        test_app2_sysCode_size = 0;
      }
    }

    if (test_app2_sysCode_size && test_app2_nfcid2_size)
    {
      UINT16 i = 0, sysCode = 0;
      for (i = 0; i < 2; i++)
        sysCode = (sysCode << (8 * i)) | test_app2_sysCode[i];

      SyncEventGuard guard(mRoutingT3tEvent);
      tNFA_STATUS nfaStat = NFA_CeRegisterFelicaSystemCodeOnDH(sysCode, test_app2_nfcid2, stackCallbackForT3T);
      if (nfaStat == NFA_STATUS_OK)
        mRoutingT3tEvent.wait();
      else
        ALOGE("Failed to register SECOND T3T emulation on DH");
    }
    ALOGD("TEST_APP2 Registered");

    //Test App3
    test_app3_nfcid2_size = GetStrValue("TEST_APP3_NFCID2", (char*)test_app3_nfcid2, sizeof(test_app3_nfcid2));
    if (test_app3_nfcid2_size > 0)
    {
      if (test_app3_nfcid2_size == 8)
        ALOGD("%s TEST_APP3_NFCID2 with size %x ", __FUNCTION__, test_app3_nfcid2_size);
      else
      {
        ALOGE("%s TEST_APP3_NFCID2 with invalid size %x ", __FUNCTION__, test_app3_nfcid2_size);
        test_app3_nfcid2_size = 0;
      }
    }

    test_app3_sysCode_size = GetStrValue("TEST_APP3_SYSTEM_CODE", (char*)test_app3_sysCode, sizeof(test_app3_sysCode));
    if (test_app3_sysCode_size > 0)
    {
      if (test_app3_sysCode_size == 2)
        ALOGD("%s TEST_APP3_SYSTEM_CODE with size %x ", __FUNCTION__, test_app3_sysCode_size);
      else
      {
        ALOGE("%s TEST_APP3_SYSTEM_CODE with invalid size %x ", __FUNCTION__, test_app3_sysCode_size);
        test_app3_sysCode_size = 0;
      }
    }

    if (test_app3_sysCode_size && test_app3_nfcid2_size)
    {
      UINT16 i = 0, sysCode = 0;
      for (i = 0; i < 2; i++)
        sysCode = (sysCode << (8 * i)) | test_app3_sysCode[i];

      SyncEventGuard guard(mRoutingT3tEvent);
      tNFA_STATUS nfaStat = NFA_CeRegisterFelicaSystemCodeOnDH(sysCode, test_app3_nfcid2, stackCallbackForT3T);
      if (nfaStat == NFA_STATUS_OK)
        mRoutingT3tEvent.wait();
      else
        ALOGE("Failed to register Third T3T emulation on DH");
    }
    ALOGD("TEST_APP3 Registered");

    ALOGD("%s: exit", fn);
}


/**
* Type 3 Tag Emulation Loopback handling. The only change made by this Loopback code is to change Command code to Response code.
*/
static void myType3ListenLoopback(uint8_t* p_buf, uint32_t len)
{
    static const char fn[] = "RoutingManager::myType3ListenLoopback";
    tNFA_STATUS sendStatus = 0;
    ALOGD("%s: Entering T3 LISTEN-LOOPBACK", __FUNCTION__);

    // Error Checking : Min len for T3T CMD is 11 bytes (1 byte command code +10 bytes command parameters)
    if (len < 11)
    {
        ALOGD("LOOPBACK - Received T3T command is too short: got %d bytes.", len);
        return;
    }

    if (p_buf == NULL)
    {
        ALOGD("Empty buffer received");
        return;
    }

    //Response code is Command Code+1;
    p_buf[1] += 1;

    //Copy this into Response byte array and send the response.
    static UINT8 t3t_rsp[MAX_T3T_CMD_LEN];
    memcpy(t3t_rsp, p_buf, len);
    sendStatus = NFA_SendRawFrame(t3t_rsp, len, NFA_DM_DEFAULT_PRESENCE_CHECK_START_DELAY);
    if (sendStatus != NFA_STATUS_OK)
    {
        ALOGD("%s: T3T LISTEN-LOOPBACK NFA_SendRawFrame(), retVal=NFA_STATUS_FAILED", fn);
    }
}

//Helper Function to match NFCID2s
bool matchNFCID(UINT8 * id1,UINT8* id2)
{
    for (int ii = 0; ii < CONFIG_MAX_NFCID2_LEN; ii++)
    {
      if (id1[ii] != id2[ii])
        return false;
    }
    return true;
}

//This Callback is invoked when T3T is Registered or when there is an RF_INTF_ACT/DATA Evt related to T3T:
void RoutingManager::stackCallbackForT3T(UINT8 event, tNFA_CONN_EVT_DATA* eventData)
{
    static const char fn[] = "RoutingManager::stackCallbackForT3T";
    ALOGD("%s: event=0x%X", fn, event);
    RoutingManager& routingManager = RoutingManager::getInstance();
    static UINT8 intf_act_nfcid2[CONFIG_MAX_NFCID2_LEN] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    if(eventData==NULL)
    {
        ALOGD("No event Data. stackCallbackforT3T failed");
        return;
    }
    switch (event)
    {
        case NFA_CE_REGISTERED_EVT:
        {
            ALOGD("%s: T3T CE Registered event=0x%X", fn, event);
            SyncEventGuard guard(routingManager.mRoutingT3tEvent);
            routingManager.mRoutingT3tEvent.notifyOne();
        }
        break;
        case NFA_CE_ACTIVATED_EVT:
        {
            //No checks here, coz system_code and nfcid2 checked against activation params in nfa_ce_act.c
            tNFA_CE_ACTIVATED& ce_activated = eventData->ce_activated;
            ALOGD("%s: T3T CE ACTIVATED event=0x%X", fn, event);
            routingManager.notifyActivated();

            //Store NFCID2 here
            memcpy(intf_act_nfcid2, &(ce_activated.activate_ntf.rf_tech_param.param.lf.nfcid2[0]), CONFIG_MAX_NFCID2_LEN);
        }
        break;
        case NFA_CE_DATA_EVT:
        {
            ALOGD("%s: T3T Data event=0x%X", fn, event);
            tNFA_CE_DATA& ce_data = eventData->ce_data;
            ALOGD("%s: NFA_CE_DATA_EVT; h=0x%X; data len=%u", fn, ce_data.handle, ce_data.len);

            //Check for NFCID2 match only for enabling Loopback Testing. This will change once Felica Test Application is available.
            UINT8* p_buf = eventData->ce_data.p_data;
            if(matchNFCID(&(p_buf[2]),intf_act_nfcid2))
            {
                myType3ListenLoopback(eventData->ce_data.p_data, eventData->ce_data.len);
            }
            else
            {
                ALOGD("%s: NFA_CE_DATA_EVT; Match Unsuccessful", fn);
            }
        }
        break;
    }
}
