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

#include <cutils/log.h>
#include <ScopedLocalRef.h>
#include <JNIHelp.h>
#include "config.h"
#include "JavaClassConstants.h"
#include "ApduGateManager.h"
#include "PowerSwitch.h"
#include <ScopedPrimitiveArray.h>
#include "RoutingManager.h"

extern "C"
{
//    #include "nfa_ee_api.h"
//    #include "nfa_ce_api.h"
//    #include "nfa_hci_api.h"
}

extern bool gActivated;

const char* APP_NAME = "nfc_jni";

namespace android
{
    extern void startRfDiscovery (bool isStart);
    extern void setSwpInactivityTimer (bool enable, UINT8 NFCEE_ID);
}

jint ApduGateManager::nativeNfcSecureElement_doOpenSecureElementConnection (JNIEnv*, jobject, jint nfceeId)
{
    jint secElemHandle = -1;
    bool status = true;

    ALOGD("%s: enter", __FUNCTION__);

    //tell the controller to power up to get ready for sec elem operations
    PowerSwitch::getInstance ().setLevel (PowerSwitch::FULL_POWER);
    PowerSwitch::getInstance ().setModeOn (PowerSwitch::SE_CONNECTED);

    status = getInstance().open((tNFA_HANDLE)nfceeId);

    //if code fails to connect to the secure element, and nothing is active, then
    //tell the controller to power down
    if ((!status) && (! PowerSwitch::getInstance ().setModeOff (PowerSwitch::SE_CONNECTED)))
    {
        PowerSwitch::getInstance ().setLevel (PowerSwitch::LOW_POWER);
        return secElemHandle;
    }

    secElemHandle = nfceeId;
    ALOGD("%s: exit; return handle=0x%X", __FUNCTION__, secElemHandle);
    return secElemHandle;
}

jboolean ApduGateManager::nativeNfcSecureElement_doDisconnectSecureElementConnection (JNIEnv*, jobject, jint handle)
{
    bool status = true;

    ALOGD("%s: enter", __FUNCTION__);

    status = getInstance().close((tNFA_HANDLE)handle);

    //if nothing is active after this, then tell the controller to power down
    if (! PowerSwitch::getInstance ().setModeOff (PowerSwitch::SE_CONNECTED))
        PowerSwitch::getInstance ().setLevel (PowerSwitch::LOW_POWER);

    ALOGD("%s: exit", __FUNCTION__);
    return status ? JNI_TRUE : JNI_FALSE;
}

jbyteArray ApduGateManager::nativeNfcSecureElement_doTransceive (JNIEnv* e, jobject, jint handle, jbyteArray data)
{
    const INT32 recvBufferMaxSize = 1024;
    UINT8 recvBuffer [recvBufferMaxSize];
    INT32 recvBufferActualSize = 0;

    ScopedByteArrayRW bytes(e, data);

    ALOGD("%s: enter; handle=0x%X; buf len=%zu", __FUNCTION__, handle, bytes.size());
    getInstance().transceive(reinterpret_cast<UINT8*>(&bytes[0]), bytes.size(), recvBuffer, recvBufferMaxSize, recvBufferActualSize);

    //copy results back to java
    jbyteArray result = e->NewByteArray(recvBufferActualSize);
    if (result != NULL)
    {
        e->SetByteArrayRegion(result, 0, recvBufferActualSize, (jbyte *) recvBuffer);
    }

    ALOGD("%s: exit: recv len=%ld", __FUNCTION__, recvBufferActualSize);
    return result;
}

const JNINativeMethod ApduGateManager::sMethods [] =
{
   {"doNativeOpenSecureElementConnection", "(I)I", (void *) ApduGateManager::nativeNfcSecureElement_doOpenSecureElementConnection},
   {"doNativeDisconnectSecureElementConnection", "(I)Z", (void *) ApduGateManager::nativeNfcSecureElement_doDisconnectSecureElementConnection},
   {"doTransceive", "(I[B)[B", (void *) ApduGateManager::nativeNfcSecureElement_doTransceive},
};

int ApduGateManager::registerJniFunctions (JNIEnv* e)
{
    static const char fn [] = "ApduGateManager::registerJniFunctions";
    ALOGD ("%s", fn);
    return jniRegisterNativeMethods (e, "com/android/nfc/dhimpl/NativeNfcSecureElement", sMethods, NELEM(sMethods));
}

ApduGateManager::ApduGateManager ()
{
//    static const char fn [] = "ApduGateManager::ApduGateManager()";
//    unsigned long num = 0;
}

ApduGateManager::~ApduGateManager ()
{
    if (mNfaHciHandle != NFA_HANDLE_INVALID)
    {
        NFA_HciDeregister (const_cast<char*>(APP_NAME));
    }
}

bool ApduGateManager::initialize (nfc_jni_native_data* native)
{
    static const char fn [] = "ApduGateManager::initialize()";
    tNFA_STATUS nfaStat;
    unsigned long num = 0;

    ALOGD ("%s: enter", fn);

    mNativeData = native;

    mRegistryToGet = NO_REGISTRY_TO_GET;
    mUseEvtSeSoftReset = false;
    mUseBwiForWtxRequest = false;
    mApduGateTransceiveTimeout = 10000;
    mUseEvtEndOfApduTransfer = false;
    mApduGateDelay = 100; // typical value is 100ms
    mUseOberthurWarmReset = false;
    mOberthurWarmResetCommand = OBERTHUR_WARM_RESET_CMD;
    mHoldSendingApdu = false;
    mApduGateDelayAfterSleep = 100; // 100ms as default
    mActivatedInListenMode = false;
    mNeedToRestartRfDiscovery = false;
    mHasStaticPipe = false;

    {
        SyncEventGuard guard (mHciRegisterEvent);

        nfaStat = NFA_HciRegister (const_cast<char*>(APP_NAME), nfaHciCallback, TRUE);
        if (nfaStat != NFA_STATUS_OK)
        {
            ALOGE ("%s: fail hci register; error=0x%X", fn, nfaStat);
            return (false);
        }
        mHciRegisterEvent.wait();
    }

    if (GetNumValue("APDU_GATE_USE_EVT_SE_SOFT_RESET", &num, sizeof(num)))
    {
        if (num > 0)
            mUseEvtSeSoftReset = true;
    }
    if (GetNumValue("APDU_GATE_USE_BWI_FOR_WTX", &num, sizeof(num)))
    {
        if (num > 0)
            mUseBwiForWtxRequest = true;
    }
    if (GetNumValue("APDU_GATE_TRANSCEIVE_TIMEOUT", &num, sizeof(num)))
    {
        mApduGateTransceiveTimeout = num;
    }
    if (GetNumValue("APDU_GATE_USE_EVT_END_OF_APDU_TRANSFER", &num, sizeof(num)))
    {
        if (num > 0)
            mUseEvtEndOfApduTransfer = true;
    }
    if (GetNumValue("APDU_GATE_DELAY", &mApduGateDelay, sizeof(mApduGateDelay)))
    {
        ALOGD ("%s: APDU_GATE_DELAY = %dms", __FUNCTION__, mApduGateDelay);
    }
    if (GetNumValue("APDU_GATE_OBERTHUR_WARM_RESET_COMMAND", &num, sizeof(num)))
    {
        mUseOberthurWarmReset = true;
        mOberthurWarmResetCommand = (UINT8) num;
    }
    if (GetNumValue("APDU_GATE_DELAY_AFTER_DEACT_IDLE", &num, sizeof(num)))
    {
        mApduGateDelayAfterSleep = num;
    }

    ALOGD ("%s: exit", fn);

    return true;
}

ApduGateManager& ApduGateManager::getInstance ()
{
    static ApduGateManager manager;
    return manager;
}

void ApduGateManager::reconnectHci(tNFA_HANDLE eeHandle)
{
    static const char fn [] = "ApduGateManager::reconnectHci()";
    tNFA_STATUS nfaStat;

    SyncEventGuard guard (mHciReconnectEvent);

    //NFA_HciReconnect() will check if reconnecting to the same NFCEE
    eeHandle = eeHandle | NFA_HANDLE_GROUP_EE;
    nfaStat = NFA_HciReconnect (mNfaHciHandle, eeHandle);
    if (nfaStat != NFA_STATUS_OK)
    {
        ALOGE ("%s: fail hci reconnect; error=0x%X", fn, nfaStat);
    }
    else
        mHciReconnectEvent.wait();
}

void ApduGateManager::getWtx()
{
    static const char fn [] = "ApduGateManager::getWtx()";
    int apduGateBWI = 4; //as default
    tNFA_STATUS nfaStat;

    // get ATR(up to 33 bytes) from APDU gete registry '01'
    mApduGateAtrLength = 0;

    ALOGD ("%s: enter", fn);
    SyncEventGuard guard (mRegistryEvent);
    nfaStat = NFA_HciGetRegistry (mNfaHciHandle, STATIC_APDU_PIPE_ID, 0x01); // ATR
    if (nfaStat != NFA_STATUS_OK)
    {
        ALOGE ("%s: fail to get ATR; error=0x%X", fn, nfaStat);
    }
    else
    {
        mRegistryToGet = APDU_GATE_REGISTRY_ATR;
        mRegistryEvent.wait ();

        ALOGD ("%s: mApduGateAtrLength=%d", fn, mApduGateAtrLength);
    }

    // BWI is upper 4 bits in TB(3), valid range is 0 - 9
    if (mApduGateAtrLength > 2)
    {
        int xx = 1; //skip TS
        int numBytes;
        int group = 0;
        int i;

        while(mApduGateAtrLength > 2)
        {
            numBytes = 0;
            for (i = 0; i < 4; i++)
            {
                if (mApduGateAtr[xx] & ((UINT8)0x10 << i))
                     numBytes++;
            }
            if (xx + numBytes >= mApduGateAtrLength)
                break;

            group++;
            if (group == 3)
            {
                numBytes = 0;
                if (mApduGateAtr[xx] & 0x10) // if TA(3)
                    numBytes++;
                if (mApduGateAtr[xx] & 0x20) // if TB(3)
                {
                    numBytes++;
                    apduGateBWI = (mApduGateAtr[xx+numBytes] >> 4);
                }
                break;
            }
            xx += numBytes;
        }
    }
    if (apduGateBWI > 9)
        apduGateBWI = 9;

    mApduGateTimeout = 1;
    for (int i = 0; i < apduGateBWI; i++)
        mApduGateTimeout *= 2;

    mApduGateTimeout *= 100; //ms
    ALOGD ("%s: exit, BWI=%d, mApduGateTimeout=%dms", fn, apduGateBWI, mApduGateTimeout);
}

void ApduGateManager::sendVscCallback (UINT8 event, UINT16 param_len, UINT8 *p_param)
{
    UINT8 oid = (event & 0x3F);
    ApduGateManager& apduGateManager = ApduGateManager::getInstance();
    SyncEventGuard guard (apduGateManager.mNfaVsCommandEvent);

    if ((event & 0xC0) == NCI_RSP_BIT)
    {
        if ((oid == 0x01)||(oid == 0x05))
        {
            if ((param_len >= 4)&&(*(p_param + 3) == NFA_STATUS_OK))
            {
                ALOGD("%s: event = 0x%x success", __FUNCTION__, event);
            }
            else
            {
                ALOGE("%s: event = 0x%x failed", __FUNCTION__, event);
            }
            apduGateManager.mNfaVsCommandEvent.notifyOne();
        }
        else if (oid == 0x04)
        {
            if ((param_len >= 4)&&(*(p_param + 3) == NFA_STATUS_OK))
            {
                ALOGD("%s: event = 0x%x success", __FUNCTION__, event);
            }
            else
            {
                ALOGE("%s: event = 0x%x failed", __FUNCTION__, event);
                apduGateManager.mNfaVsCommandEvent.notifyOne(); // don't wait for NTF
            }
        }
    }
    else if ((event & 0xC0) == NCI_NTF_BIT)
    {
        if (oid == 0x04)
        {
            if ((param_len >= 4)&&(*(p_param + 3) == NFA_STATUS_OK))
            {
                ALOGD("%s: event = 0x%x success", __FUNCTION__, event);
            }
            else
            {
                ALOGE("%s: event = 0x%x failed", __FUNCTION__, event);
            }
            apduGateManager.mNfaVsCommandEvent.notifyOne();
        }
    }
}

tNFA_STATUS ApduGateManager::setSwpInactivityTimer (bool enable, UINT8 nfceeId)
{
    UINT8 cmd_params[5];
    tNFA_STATUS stat = NFA_STATUS_OK;

    ALOGD("%s: enable = %d, nfceeId = %d", __FUNCTION__, enable, nfceeId);

    cmd_params[0] = 0x0A; // NFCEE Control Command
    cmd_params[1] = 0x01; // number of TLV
    if (enable)
        cmd_params[2] = 0x01; // type = enable P5 Timer
    else
        cmd_params[2] = 0x00; // type = disable P5 Timer

    cmd_params[3] = 0x01;     // length
    cmd_params[4] = nfceeId;

    SyncEventGuard guard (mNfaVsCommandEvent);
    stat = NFA_SendVsCommand (0x01, // generic command oid
                              0x05, // cmd_params_len,
                              cmd_params,
                              sendVscCallback);

    if (stat != NFA_STATUS_OK)
    {
        ALOGE("%s: NFA_SendVsCommand failed", __FUNCTION__);
        return stat;
    }
    mNfaVsCommandEvent.wait ();
    return stat;
}

bool ApduGateManager::open(tNFA_HANDLE eeHandle)
{
    static const char fn [] = "ApduGateManager::open()";
    tNFA_STATUS nfaStat;
    bool retVal;

    ALOGD("%s: enter eeHandle = 0x%X", fn, eeHandle);
    if (mIsApduOpenned)
    {
        ALOGE("%s: APDU pipe is already openned", fn);
        return (false);
    }
    if (eeHandle == NFA_HANDLE_INVALID)
    {
        ALOGE ("%s: invalid handle 0x%X", fn, eeHandle);
        return (false);
    }

    if ((!mActivatedInListenMode)&&(!gActivated))
    {
        // Disable RF discovery completely while the DH is connected
        android::startRfDiscovery(false);
        mNeedToRestartRfDiscovery = true; //restart RF discovery when APDU exchange is done

        // send sleep CMD to NFCC
        NFA_SendVsCommand (NCI_MSG_PROP_SLEEP, // oid, sleep cmd
                           0x00, // cmd_params_len,
                           NULL,
                           NULL);

        if (mApduGateDelayAfterSleep)
        {
            // need to wait for entering sleep mode
            // clock switching is required in case that device was under RF field
            ALOGD ("%s: wait %dms for complete deactivate to idle", fn, mApduGateDelayAfterSleep);
            SyncEventGuard guard (mHoldTransceiveEvent);
            mHoldTransceiveEvent.wait (mApduGateDelayAfterSleep);
        }
    }

    reconnectHci(eeHandle);

    // if connected to other than mOffHostEe then reconnect HCI when status()
    mHciConnectedEeHandle = eeHandle;

    // Disable UICC idle timeout while the DH is connected
    setSwpInactivityTimer (false, (UINT8)(eeHandle & 0x00FF)); 

    if(!mHasStaticPipe)
    {
        nfaStat = NFA_HciAddStaticPipe(mNfaHciHandle, DEST_HOST, DEST_APDU_GATE, STATIC_APDU_PIPE_ID);
        if (nfaStat != NFA_STATUS_OK)
        {
            ALOGE ("%s: fail create static pipe; error=0x%X", fn, nfaStat);
            retVal = false;
            goto TheEnd;
        }
        else
        {
            SyncEventGuard guard (mHciAddStaticPipeEvent);
            if(!mHasStaticPipe)
            {
                 mHciAddStaticPipeEvent.wait(100);
                 if(!mHasStaticPipe)
                 {
                      ALOGE ("%s: fail create static pipe", fn);
                      retVal = false;
                      goto TheEnd;
                 }
            }
        }
    }

    if (mUseBwiForWtxRequest)
    {
        getWtx();
    }

    if (mUseEvtSeSoftReset)
    {
        ALOGD ("%s: UseEvtSeSoftReset", fn);
        SyncEventGuard guard (mHciSentEvent);

        nfaStat = NFA_HciSendEvent (mNfaHciHandle,
                                    STATIC_APDU_PIPE_ID,
                                    EVT_SOFT_RESET,
                                    0,    //xmitBufferSize,
                                    NULL, //xmitBuffer,
                                    0,    //sizeof(mResponseData),
                                    NULL, //mResponseData,
                                    0);

        if (nfaStat == NFA_STATUS_OK)
            mHciSentEvent.wait ();

        if (mApduGateDelay)
        {
            ALOGD("%s: Start delay for %dms", fn, mApduGateDelay);
            mHciSentEvent.wait (mApduGateDelay); // wait for eSE to complete reset operation
        }
    }

    retVal = true;

TheEnd:
    mIsApduOpenned = retVal;
    if (!retVal)
    {
        // if open failed we need to de-allocate the gate
        //close(0);
    }

    ALOGD ("%s: exit; retVal=%u", fn, retVal);
    return retVal;
}

bool ApduGateManager::close(tNFA_HANDLE eeHandle)
{
    static const char fn [] = "ApduGateManager::close()";
    tNFA_STATUS nfaStat;

    ALOGD("%s: enter eeHandle = 0x%X", fn, eeHandle);
    if (!mIsApduOpenned)
    {
        ALOGD("%s: APDU pipe was not openned", fn);
        return (false);
    }
    if (eeHandle == NFA_HANDLE_INVALID)
    {
        ALOGE ("%s: invalid handle 0x%X", fn, eeHandle);
        return (false);
    }

    ALOGD("%s: handle=0x%04x", fn, eeHandle);

    {   // complete transceive function if pending
        SyncEventGuard guard (mTransceiveEvent);
        if (mHoldSendingApdu)
        {
            SyncEventGuard guard (mHoldTransceiveEvent);
            mHoldTransceiveEvent.notifyOne ();
            mHoldSendingApdu = false;
        }
        mTransceiveEvent.notifyOne ();
    }

    if (mUseOberthurWarmReset)
    {
        //send warm-reset command to Oberthur secure element which deselects the applet;
        //this is an Oberthur-specific command;
        ALOGD("%s: try warm-reset on pipe id 0x%X; cmd=0x%X", fn, STATIC_APDU_PIPE_ID, mOberthurWarmResetCommand);
        SyncEventGuard guard (mRegistryEvent);
        nfaStat = NFA_HciSetRegistry (mNfaHciHandle, STATIC_APDU_PIPE_ID, 1, 1, &mOberthurWarmResetCommand);
        if (nfaStat == NFA_STATUS_OK)
        {
            mRegistryEvent.wait ();
            ALOGD("%s: completed warm-reset on pipe 0x%X", fn, STATIC_APDU_PIPE_ID);
        }
    }

    if (mUseEvtEndOfApduTransfer)
    {
        SyncEventGuard guard (mHciSentEvent);

        nfaStat = NFA_HciSendEvent (mNfaHciHandle,
                                    STATIC_APDU_PIPE_ID,
                                    EVT_END_OF_APDU_TRANSFER,
                                    0,    //xmitBufferSize,
                                    NULL, //xmitBuffer,
                                    0,    //sizeof(mResponseData),
                                    NULL, //mResponseData,
                                    0);

        if (nfaStat == NFA_STATUS_OK)
            mHciSentEvent.wait ();
    }

    // Re-enable UICC low-power mode
    setSwpInactivityTimer (true, (UINT8)(eeHandle & 0x00FF)); 

    mIsApduOpenned = false;

    // if connected to other than mOffHostEe then reconnect HCI
    if ((mHciConnectedEeHandle & 0x00FF) != RoutingManager::getInstance().getDefaultOffHostRouteDestination())
    {
        mHciConnectedEeHandle = RoutingManager::getInstance().getDefaultOffHostRouteDestination();

        int numNfcee, nfceeIds[NFA_EE_MAX_EE_SUPPORTED], xx;
        numNfcee = RoutingManager::getInstance().getSecureElementIdList (nfceeIds);
        for (xx = 0; xx < numNfcee; xx++)
        {
            if (mHciConnectedEeHandle == nfceeIds[xx])
            {
                reconnectHci(mHciConnectedEeHandle);
                break;
            }
        }
    }

    if (mNeedToRestartRfDiscovery)
    {
        mNeedToRestartRfDiscovery = false;
        android::startRfDiscovery(true);

        ALOGD ("%s: wait for complete start discovery", fn);
        SyncEventGuard guard (mHoldTransceiveEvent);
        mHoldTransceiveEvent.wait (100);
    }

    ALOGD("%s: exit", fn);
    return true;
}

bool ApduGateManager::transceive (UINT8* xmitBuffer, INT32 xmitBufferSize, 
                                  UINT8* recvBuffer, INT32 recvBufferMaxSize, INT32& recvBufferActualSize)
{
    static const char fn [] = "ApduGateManager::transceive";
    tNFA_STATUS nfaStat = NFA_STATUS_FAILED;
    bool isSuccess = false;
    bool waitOk = false;
    int  timeoutMillisec = mApduGateTransceiveTimeout;

    ALOGD ("%s: enter; xmitBufferSize=%ld; recvBufferMaxSize=%ld; timeout=%ld", fn, xmitBufferSize, recvBufferMaxSize, timeoutMillisec);

    if (mHoldSendingApdu)
    {
        ALOGD ("%s: hold sending APDU while deactivating to idle", fn);
        SyncEventGuard guard (mHoldTransceiveEvent);
        mHoldTransceiveEvent.wait ();
    }

    {
        SyncEventGuard guard (mTransceiveEvent);
        mActualResponseSize = 0;
        //memset (mResponseData, 0, sizeof(mResponseData));
        // static pipe for APDU exchange
        nfaStat = NFA_HciSendEvent (mNfaHciHandle, STATIC_APDU_PIPE_ID, EVT_SEND_DATA,
                                    xmitBufferSize, xmitBuffer, recvBufferMaxSize, recvBuffer, 0);
        if (nfaStat == NFA_STATUS_OK)
        {
            int maxWtx = 10;
            int numWtx = 0;

            if (mApduGateTimeout > 99)
            {
                maxWtx = (timeoutMillisec/mApduGateTimeout + 1);
                timeoutMillisec = mApduGateTimeout + 100; //add 100ms latency
            }

            ALOGD ("%s: maxWtx=%d, WTX timeout=%dms", fn, maxWtx, timeoutMillisec);
            do {
                mReceivedWtx = false;
                waitOk = mTransceiveEvent.wait (timeoutMillisec);
                if (waitOk == false) //timeout occurs
                {
                    if (mReceivedWtx == false)
                    {
                        ALOGE ("%s: wait response timeout", fn);
                        goto TheEnd;
                    }
                    else
                    {
                       if (++numWtx > maxWtx)
                       {
                           ALOGD ("%s: got max WTX (%d)", fn, numWtx);
                           mReceivedWtx = false;
                       }
                    }
                }
                else
                {
                    mReceivedWtx = false; // got data, clear and exit from loop
                }
            } while (mReceivedWtx);
        }
        else
        {
            ALOGE ("%s: fail send data; error=0x%X", fn, nfaStat);
            goto TheEnd;
        }

        recvBufferActualSize = mActualResponseSize;
    }

    isSuccess = true;

TheEnd:
    ALOGD ("%s: exit; isSuccess: %d; recvBufferActualSize: %ld", fn, isSuccess, recvBufferActualSize);
    return (isSuccess);

}

void ApduGateManager::notifyRfIntfDeactivated ()
{
    static const char fn [] = "ApduGateManager::notifyRfIntfDeactivated";

    ALOGD ("%s: enter, mIsApduOpenned=%d, mActivatedInListenMode=%d", fn, mIsApduOpenned, mActivatedInListenMode);

    if (mIsApduOpenned)
    {
        // hold sending APDU while deactivating to idle
        mHoldSendingApdu = true;

        if ((mActivatedInListenMode)&&(mApduGateDelayAfterSleep))
        {
            mActivatedInListenMode = false;

            // need to wait for entering sleep mode
            // clock switching is required because device was under RF field
            ALOGD ("%s: wait %dms for complete deactivate to idle", fn, mApduGateDelayAfterSleep);
            SyncEventGuard guard (mHoldTransceiveEvent);
            mHoldTransceiveEvent.wait (mApduGateDelayAfterSleep);
        }

        // Disable RF discovery completely while the DH is connected
        android::startRfDiscovery(false);

        // send sleep CMD to NFCC
        NFA_SendVsCommand (NCI_MSG_PROP_SLEEP, // oid, sleep cmd
                           0x00, // cmd_params_len,
                           NULL,
                           NULL);

        // disable P5 timer for NFCC not to enter sleep mode
        android::setSwpInactivityTimer (false, UINT8(mHciConnectedEeHandle & 0x00FF));

        SyncEventGuard guard (mHoldTransceiveEvent);
        mHoldTransceiveEvent.notifyOne ();
        mHoldSendingApdu = false;
    }

    ALOGD ("%s: exit", fn);
}

void ApduGateManager::notifyListenModeState (bool isActivated) {
    static const char fn [] = "ApduGateManager::notifyListenMode";

    ALOGD ("%s: listen mode active=%u", fn, isActivated);
    mActivatedInListenMode = isActivated;
}

void ApduGateManager::handleHciRegisterEvt(tNFA_HCI_REGISTER& hciRegister)
{
    SyncEventGuard guard (mHciRegisterEvent);
    mNfaHciHandle = hciRegister.hci_handle;
    mHciRegisterEvent.notifyOne();
}

void ApduGateManager::handleHciGetRegistryRspEvt(tNFA_HCI_REGISTRY& hciRegistry)
{
    static const char fn [] = "ApduGateManager::nfaHciCallback()";

    switch (mRegistryToGet)
    {
    case APDU_GATE_REGISTRY_ATR:
        if (hciRegistry.status == NFA_STATUS_OK)
        {
            memcpy(mApduGateAtr, hciRegistry.reg_data, hciRegistry.data_len);
            mApduGateAtrLength = hciRegistry.data_len;
        }
        else
        {
            ALOGE ("%s: failed to get ATR from APDU gate", fn);
            mApduGateAtrLength = 0;
        }
        break;
    default:
        ALOGE ("%s: unexpected event", fn);
        mRegistryToGet = NO_REGISTRY_TO_GET;
        // do not call notifyOne()
        return;
    }

    SyncEventGuard guard (mRegistryEvent);
    mRegistryToGet = NO_REGISTRY_TO_GET;
    mRegistryEvent.notifyOne ();
}

void ApduGateManager::handleHciAddStaticPipeEvt(tNFA_HCI_ADD_STATIC_PIPE_EVT& addStaticPipe)
{
    if(addStaticPipe.status == NFA_STATUS_OK)
        mHasStaticPipe = true;
    SyncEventGuard guard (mHciAddStaticPipeEvent);
    mHciAddStaticPipeEvent.notifyOne();
}

void ApduGateManager::nfaHciCallback (tNFA_HCI_EVT event, tNFA_HCI_EVT_DATA* eventData)
{
    static const char fn [] = "ApduGateManager::nfaHciCallback()";
    ALOGD ("%s: event=0x%X", fn, event);
    ApduGateManager& apduGateManager = ApduGateManager::getInstance();

    switch (event)
    {
    case NFA_HCI_REGISTER_EVT:
        {
            ALOGD ("%s: NFA_HCI_REGISTER_EVT; status=0x%X; handle=0x%X", fn,
                    eventData->hci_register.status, eventData->hci_register.hci_handle);
            apduGateManager.handleHciRegisterEvt(eventData->hci_register);
        }
        break;

    case NFA_HCI_RECONNECT_EVT:
        {
            ALOGD ("%s: NFA_HCI_RECONNECT_EVT; status=0x%X", fn, eventData->reconnected.status);
            SyncEventGuard guard (apduGateManager.mHciReconnectEvent);
            apduGateManager.mHciReconnectEvent.notifyOne();
        }
        break;

/*
    case NFA_HCI_ALLOCATE_GATE_EVT:
        {
            ALOGD ("%s: NFA_HCI_ALLOCATE_GATE_EVT; status=0x%X; gate=0x%X", fn, eventData->status, eventData->allocated.gate);
            SyncEventGuard guard (sSecElem.mAllocateGateEvent);
            sSecElem.mCommandStatus = eventData->status;
            sSecElem.mNewSourceGate = (eventData->allocated.status == NFA_STATUS_OK) ? eventData->allocated.gate : 0;
            sSecElem.mAllocateGateEvent.notifyOne();
        }
        break;

    case NFA_HCI_DEALLOCATE_GATE_EVT:
        {
            tNFA_HCI_DEALLOCATE_GATE& deallocated = eventData->deallocated;
            ALOGD ("%s: NFA_HCI_DEALLOCATE_GATE_EVT; status=0x%X; gate=0x%X", fn, deallocated.status, deallocated.gate);
            SyncEventGuard guard (sSecElem.mDeallocateGateEvent);
            sSecElem.mDeallocateGateEvent.notifyOne();
        }
        break;

    case NFA_HCI_GET_GATE_PIPE_LIST_EVT:
        {
            ALOGD ("%s: NFA_HCI_GET_GATE_PIPE_LIST_EVT; status=0x%X; num_pipes: %u  num_gates: %u", fn,
                    eventData->gates_pipes.status, eventData->gates_pipes.num_pipes, eventData->gates_pipes.num_gates);
            SyncEventGuard guard (sSecElem.mPipeListEvent);
            sSecElem.mCommandStatus = eventData->gates_pipes.status;
            sSecElem.mHciCfg = eventData->gates_pipes;
            sSecElem.mPipeListEvent.notifyOne();
        }
        break;

    case NFA_HCI_CREATE_PIPE_EVT:
        {
            ALOGD ("%s: NFA_HCI_CREATE_PIPE_EVT; status=0x%X; pipe=0x%X; src gate=0x%X; dest host=0x%X; dest gate=0x%X", fn,
                    eventData->created.status, eventData->created.pipe, eventData->created.source_gate, eventData->created.dest_host, eventData->created.dest_gate);
            SyncEventGuard guard (sSecElem.mCreatePipeEvent);
            sSecElem.mCommandStatus = eventData->created.status;
            sSecElem.mNewPipeId = eventData->created.pipe;
            sSecElem.mCreatePipeEvent.notifyOne();
        }
        break;

    case NFA_HCI_OPEN_PIPE_EVT:
        {
            ALOGD ("%s: NFA_HCI_OPEN_PIPE_EVT; status=0x%X; pipe=0x%X", fn, eventData->opened.status, eventData->opened.pipe);
            SyncEventGuard guard (sSecElem.mPipeOpenedEvent);
            sSecElem.mCommandStatus = eventData->opened.status;
            sSecElem.mPipeOpenedEvent.notifyOne();
        }
        break;
*/
    case NFA_HCI_EVENT_SENT_EVT:
        {
            ALOGD ("%s: NFA_HCI_EVENT_SENT_EVT; status=0x%X", fn, eventData->evt_sent.status);

            SyncEventGuard guard (apduGateManager.mHciSentEvent);
            apduGateManager.mHciSentEvent.notifyOne();
        }
        break;
/*
    case NFA_HCI_RSP_RCVD_EVT: //response received from secure element
        {
            tNFA_HCI_RSP_RCVD& rsp_rcvd = eventData->rsp_rcvd;
            ALOGD ("%s: NFA_HCI_RSP_RCVD_EVT; status: 0x%X; code: 0x%X; pipe: 0x%X; len: %u", fn,
                    rsp_rcvd.status, rsp_rcvd.rsp_code, rsp_rcvd.pipe, rsp_rcvd.rsp_len);
        }
        break;
*/
    case NFA_HCI_GET_REG_RSP_EVT :
        ALOGD ("%s: NFA_HCI_GET_REG_RSP_EVT; status: 0x%X; pipe: 0x%X, len: %d", fn,
                eventData->registry.status, eventData->registry.pipe, eventData->registry.data_len);
        {
            apduGateManager.handleHciGetRegistryRspEvt(eventData->registry);
        }
        break;

    case NFA_HCI_EVENT_RCVD_EVT:
        ALOGD ("%s: NFA_HCI_EVENT_RCVD_EVT; code: 0x%X; pipe: 0x%X; data len: %u", fn,
                eventData->rcvd_evt.evt_code, eventData->rcvd_evt.pipe, eventData->rcvd_evt.evt_len);

        if (eventData->rcvd_evt.pipe == STATIC_APDU_PIPE_ID)
        {
            if (eventData->rcvd_evt.status != NFA_STATUS_OK)
            {
                ALOGD ("%s: NFA_HCI_EVENT_RCVD_EVT; failed to get response from static pipe", fn);
                SyncEventGuard guard (apduGateManager.mTransceiveEvent);
                apduGateManager.mTransceiveEvent.notifyOne ();
            }
            else if (eventData->rcvd_evt.evt_code == EVT_SEND_DATA)
            {
                ALOGD ("%s: NFA_HCI_EVENT_RCVD_EVT; data from static pipe", fn);
                SyncEventGuard guard (apduGateManager.mTransceiveEvent);
                apduGateManager.mActualResponseSize = eventData->rcvd_evt.evt_len;
                apduGateManager.mTransceiveEvent.notifyOne ();
            }
            else if (eventData->rcvd_evt.evt_code == EVT_WTX_REQUEST)
            {
                ALOGD ("%s: NFA_HCI_EVENT_RCVD_EVT; EVT_WTX_REQUEST from static pipe", fn);
                apduGateManager.mReceivedWtx = true;
            }
        }
        else if (eventData->rcvd_evt.evt_code == NFA_HCI_EVT_POST_DATA)
        {
            ALOGD ("%s: NFA_HCI_EVENT_RCVD_EVT; NFA_HCI_EVT_POST_DATA", fn);
            //SyncEventGuard guard (sSecElem.mTransceiveEvent);
            //sSecElem.mActualResponseSize = (eventData->rcvd_evt.evt_len > MAX_RESPONSE_SIZE) ? MAX_RESPONSE_SIZE : eventData->rcvd_evt.evt_len;
            //sSecElem.mTransceiveEvent.notifyOne ();
        }
        else if (eventData->rcvd_evt.evt_code == NFA_HCI_EVT_TRANSACTION)
        {
            ALOGD ("%s: NFA_HCI_EVENT_RCVD_EVT; NFA_HCI_EVT_TRANSACTION", fn);
            // If we got an AID, notify any listeners
            //if ((eventData->rcvd_evt.evt_len > 3) && (eventData->rcvd_evt.p_evt_buf[0] == 0x81) )
            //    sSecElem.notifyTransactionListenersOfAid (&eventData->rcvd_evt.p_evt_buf[2], eventData->rcvd_evt.p_evt_buf[1]);
        }
        break;

    case NFA_HCI_SET_REG_RSP_EVT: //received response to write registry command
        {
            tNFA_HCI_REGISTRY& registry = eventData->registry;
            ALOGD ("%s: NFA_HCI_SET_REG_RSP_EVT; status=0x%X; pipe=0x%X", fn, registry.status, registry.pipe);
            SyncEventGuard guard (apduGateManager.mRegistryEvent);
            apduGateManager.mRegistryEvent.notifyOne();
            break;
        }

    case NFA_HCI_ADD_STATIC_PIPE_EVT: //static pipe is added
        {
            tNFA_HCI_ADD_STATIC_PIPE_EVT& pipe_added = eventData->pipe_added;
            ALOGD ("%s: NFA_HCI_ADD_STATIC_PIPE_EVT; status=0x%X;", fn, pipe_added.status);
            apduGateManager.handleHciAddStaticPipeEvt(eventData->pipe_added);
            break;
        }
    default:
        ALOGE ("%s: unknown event code=0x%X ????", fn, event);
        break;
    }
}

