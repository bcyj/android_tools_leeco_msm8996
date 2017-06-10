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

#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "OverrideLog.h"
#include "NfcJniUtil.h"
#include "NfcTag.h"
#include "config.h"
#include "Mutex.h"
#include "IntervalTimer.h"
#include "JavaClassConstants.h"
#include "Pn544Interop.h"
#include <ScopedLocalRef.h>
#include <ScopedPrimitiveArray.h>
#include <string>
#include "ApduGateManager.h"

extern "C"
{
    #include "nfa_api.h"
    #include "nfa_rw_api.h"
    #include "ndef_utils.h"
    #include "rw_api.h"
    #ifdef DTA // <DTA>
    #include "dta_mode.h"
    #include "rw_int.h"
    #endif // </DTA>
}
namespace android
{
    extern nfc_jni_native_data* getNative(JNIEnv *e, jobject o);
    extern bool nfcManager_isNfcActive();
    extern bool setPropNciParameter(uint8_t tag, uint8_t *val, uint8_t len);
}

extern bool         gActivated;
extern SyncEvent    gDeactivatedEvent;

/*****************************************************************************
**
** public variables and functions
**
*****************************************************************************/
namespace android
{
    bool    gIsTagDeactivating = false;    // flag for nfa callback indicating we are deactivating for RF interface switch
    bool    gIsSelectingRfInterface = false; // flag for nfa callback indicating we are selecting for RF interface switch
}


/*****************************************************************************
**
** private variables and functions
**
*****************************************************************************/
namespace android
{


// Pre-defined tag type values. These must match the values in
// framework Ndef.java for Google public NFC API.
#define NDEF_UNKNOWN_TYPE          -1
#define NDEF_TYPE1_TAG             1
#define NDEF_TYPE2_TAG             2
#define NDEF_TYPE3_TAG             3
#define NDEF_TYPE4_TAG             4
#define NDEF_MIFARE_CLASSIC_TAG    101

#define STATUS_CODE_TARGET_LOST    146  // this error code comes from the service

//Mifare Classic operation states
#define MIFR_CLASSIC_MUTUAL_AUTHENTICATION_KEY_A      0x60
#define MIFR_CLASSIC_MUTUAL_AUTHENTICATION_KEY_B      0x61
#define MIFR_CLASSIC_CARD_WRITE                       0xA0
#define MIFR_CLASSIC_CARD_READ                        0x30
#define MIFR_CLASSIC_CARD_INCREMENT                   0xC1
#define MIFR_CLASSIC_CARD_DECREMENT                   0xC0
#define MIFR_CLASSIC_CARD_RESTORE                     0xC2
#define MIFR_CLASSIC_CARD_TRANSFER                    0xB0
#define MIFR_END_READER_MODE_SESSION                  0x00

//Mifare classic related definitions
#define NUM_OF_AUTH_STEPS                             0x05
#define THREE_PASS_AUTH_CMD_LEN                       0x0B
#define START_MIFARE_READER_MODE_SESSION              0x10
#define START_SESSION                                 0x01
#define MIFARE_READER_MODE_AUTHENTICATION             0x11
#define ENCRYPT_CHALLENG_PHASE                        0x00
#define FINISH_THREE_PASS_AUTH                        0x01
#define DATA_RETURNED                                 0x09
#define NO_DATA_RETURNED                              0x00
#define APPLET_AID_LENGTH                             0x10
#define SELECT_CMD_HEADER_LEN                         0x05
#define ENCRYPT_DECRYPT_DATA                          0x12
#define READ_ENCRYPT_CAPDU_LEN                        0x08
#define RFU                                           0x00
#define ENCRYPT_CMD                                   0x00
#define DECRYPT_RSP                                   0x01
#define CMD_DATA_OFFSET                               0x05
#define ENCRYPT_WRITE_CMD                             0x02
#define WRITE_DATA_RETURN                             0x1C
#define WRITE_ADDR_LEN                                0x05
#define WRITE_DATA_LEN                                0x15
#define ACK1_LEN                                      0x01
#define INC_DEC_RESTORE                               0x03
#define INC_DEC_RES_DATA_RETURN                       0x0D
#define INC_DEC_RES_DATA_LEN                          0x07
#define TRANSFER                                      0x04
#define TRANSFER_DATA_RETURN                          0x06
#define END_MIFARE_READER_MODE_SESSION                0x00
#define END_MIFARE_CAPDU_LEN                          0x05
#define NORMAL_ENDING_CMD_BYTE                        0x90
#define MIFARE_AUTH_CMD_LEN                           0x0C
#define STATUS_BYTES_LEN                              0x02
#define FILLED_CMD_LEN                                0x05
#define LE_LEN                                        0x01
#define MIFR_CLASSIC_TAG_LOST                         0x92
#define MIFR_RESTORE_CMD_LEN                          0x02

//Mifare Authentication states
enum
{
    START_READER_MODE_SESSION = 1,
    MIFARE_TAG_AUTH,
    START_READER_MODE_THREE_PASS_AUTH,
    END_READER_MODE_THREE_PASS_AUTH
};

// Write states
enum{
    MIFARE_WRITE_ENCRYPT_CMD_APDU = 1,
    MIFARE_WRITE_SEND_ADDR,
    MIFARE_WRITE_CHECK_ACK1,
    MIFARE_WRITE_SEND_DATA,
    MIFARE_WRITE_CHECK_ACK2
};

//Increment , decrement , restore and transfer states.
enum{
    MIFARE_ENCRYPT_CMD_APDU = 1,
    MIFARE_SEND_ADDR,
    MIFARE_CHECK_ACK,
    MIFARE_SEND_VALUE,
    MIFARE_TRANS_TIMEOUT
};
static uint32_t     sCheckNdefCurrentSize = 0;
static tNFA_STATUS  sCheckNdefStatus = 0; //whether tag already contains a NDEF message
static bool         sCheckNdefCapable = false; //whether tag has NDEF capability
static tNFA_HANDLE  sNdefTypeHandlerHandle = NFA_HANDLE_INVALID;
static std::basic_string<UINT8> sRxDataBuffer;
static tNFA_STATUS  sRxDataStatus = NFA_STATUS_OK;
static bool         sWaitingForTransceive = false;
static bool         sTransceiveRfTimeout = false;
static bool         sNeedToSwitchRf = false;
static Mutex        sRfInterfaceMutex;
static uint32_t     sReadDataLen = 0;
static uint8_t*     sReadData = NULL;
static bool         sIsReadingNdefMessage = false;
static SyncEvent    sReadEvent;
static sem_t        sWriteSem;
static sem_t        sFormatSem;
static SyncEvent    sTransceiveEvent;
static SyncEvent    sReconnectEvent;
static sem_t        sCheckNdefSem;
static SyncEvent    sPresenceCheckEvent;
static sem_t        sMakeReadonlySem;
static IntervalTimer sSwitchBackTimer; // timer used to tell us to switch back to ISO_DEP frame interface
static jboolean     sWriteOk = JNI_FALSE;
static jboolean     sWriteWaitingForComplete = JNI_FALSE;
static bool         sFormatOk = false;
static jboolean     sConnectOk = JNI_FALSE;
static jboolean     sConnectWaitingForComplete = JNI_FALSE;
static bool         sGotDeactivate = false;
static uint32_t     sCheckNdefMaxSize = 0;
static bool         sCheckNdefCardReadOnly = false;
static jboolean     sCheckNdefWaitingForComplete = JNI_FALSE;
static bool         sIsTagPresent = true;
static tNFA_STATUS  sMakeReadonlyStatus = NFA_STATUS_FAILED;
static jboolean     sMakeReadonlyWaitingForComplete = JNI_FALSE;
static int          sCurrentConnectedTargetType = TARGET_TYPE_UNKNOWN;

static int reSelect (tNFA_INTF_TYPE rfInterface, bool fSwitchIfNeeded);
static bool switchRfInterface(tNFA_INTF_TYPE rfInterface);
// Variables declaration.
static bool         sEseInit = false;
static bool         sCrcOffParamSet = false;
static bool         sSendtoSE = false;
static uint8_t      sMifareClassicState = 0;
static uint8_t      sMifareClassicInAuthState = START_READER_MODE_SESSION;
static bool         isMifareClassicPresent = false;
uint8_t*            gCmd = NULL;
INT32               gCmdlen = 0;
static bool         sMifareClassicOpDone = false;
bool                gReturnData = true;
bool                gMifareWriteCompleted = false;
bool                sErrorOccured = false;
bool                gFirstAuth = false;
uint8_t             sMifareClAuthSteps = 0;
uint8_t             gMifareClReadState = ENCRYPT_CMD;
uint8_t             buf_len = 0;
uint8_t             gMifareWriteState = MIFARE_WRITE_ENCRYPT_CMD_APDU;
uint8_t             gMifareState = MIFARE_ENCRYPT_CMD_APDU;
uint8_t             *gTagFrame = NULL,gTagFrameLen=0;
const INT32         sRecvBufferMaxSize = 50;
uint8_t             *gRecvBuffer=NULL;
uint32_t            sEseId = 0;

#ifdef DTA // <DTA>

#define T1T_INIT_BLK_SIZE 24

//static int currTimeout = 0;
UINT16 data_len=0;
UINT8 * t1t_tag_data = NULL;
extern UINT8 t1t_dyn_activated;
UINT8 t1t_init_blocks[T1T_INIT_BLK_SIZE];

#endif // </DTA>

/*******************************************************************************
**
** Function:        nativeNfcTag_abortWaits
**
** Description:     Unblock all thread synchronization objects.
**
** Returns:         None
**
*******************************************************************************/
void nativeNfcTag_abortWaits ()
{
    ALOGD ("%s", __FUNCTION__);
    {
        SyncEventGuard g (sReadEvent);
        sReadEvent.notifyOne ();
    }
    sem_post (&sWriteSem);
    sem_post (&sFormatSem);
    {
        SyncEventGuard g (sTransceiveEvent);
        sTransceiveEvent.notifyOne ();
    }
    {
        SyncEventGuard g (sReconnectEvent);
        sReconnectEvent.notifyOne ();
    }

    sem_post (&sCheckNdefSem);
    {
        SyncEventGuard guard (sPresenceCheckEvent);
        sPresenceCheckEvent.notifyOne ();
    }
    sem_post (&sMakeReadonlySem);
    NfcTag::getInstance().setRfInterface(NFA_INTERFACE_ISO_DEP);
    sCurrentConnectedTargetType = TARGET_TYPE_UNKNOWN;
}

/*******************************************************************************
**
** Function:        nativeNfcTag_doReadCompleted
**
** Description:     Receive the completion status of read operation.  Called by
**                  NFA_READ_CPLT_EVT.
**                  status: Status of operation.
**
** Returns:         None
**
*******************************************************************************/
void nativeNfcTag_doReadCompleted (tNFA_STATUS status)
{
    ALOGD ("%s: status=0x%X; is reading=%u", __FUNCTION__, status, sIsReadingNdefMessage);

    if (sIsReadingNdefMessage == false)
        return; //not reading NDEF message right now, so just return

    if (status != NFA_STATUS_OK)
    {
        sReadDataLen = 0;
        if (sReadData)
            free (sReadData);
        sReadData = NULL;
    }
    SyncEventGuard g (sReadEvent);
    sReadEvent.notifyOne ();
}


/*******************************************************************************
**
** Function:        ndefHandlerCallback
**
** Description:     Receive NDEF-message related events from stack.
**                  event: Event code.
**                  p_data: Event data.
**
** Returns:         None
**
*******************************************************************************/
static void ndefHandlerCallback (tNFA_NDEF_EVT event, tNFA_NDEF_EVT_DATA *eventData)
{
    ALOGD ("%s: event=%u, eventData=%p", __FUNCTION__, event, eventData);

    switch (event)
    {
    case NFA_NDEF_REGISTER_EVT:
        {
            tNFA_NDEF_REGISTER& ndef_reg = eventData->ndef_reg;
            ALOGD ("%s: NFA_NDEF_REGISTER_EVT; status=0x%X; h=0x%X", __FUNCTION__, ndef_reg.status, ndef_reg.ndef_type_handle);
            sNdefTypeHandlerHandle = ndef_reg.ndef_type_handle;
        }
        break;

    case NFA_NDEF_DATA_EVT:
        {
            ALOGD ("%s: NFA_NDEF_DATA_EVT; data_len = %lu", __FUNCTION__, eventData->ndef_data.len);
            sReadDataLen = eventData->ndef_data.len;
            if(sReadDataLen > 0)
            {
                sReadData = (uint8_t*) malloc (sReadDataLen);
                if(sReadData != NULL)
                {
                    memcpy (sReadData, eventData->ndef_data.p_data, eventData->ndef_data.len);
                }
            }
        }
        break;

    default:
        ALOGE ("%s: Unknown event %u ????", __FUNCTION__, event);
        break;
    }
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doRead
**
** Description:     Read the NDEF message on the tag.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         NDEF message.
**
*******************************************************************************/
static jbyteArray nativeNfcTag_doRead (JNIEnv* e, jobject)
{
    ALOGD ("%s: enter", __FUNCTION__);
    tNFA_STATUS status = NFA_STATUS_FAILED;
    jbyteArray buf = NULL;

    sReadDataLen = 0;
    if (sReadData != NULL)
    {
        free (sReadData);
        sReadData = NULL;
    }

    #ifdef DTA // <DTA>
    // This piece of code prevents call to NFC_RwReadNdef() in case the tag is already deactivated.
    // What happens in this case is that the sReadEvent.wait() gets stuck because NFA_READ_CPLT_EVT
    // is not received at all.
    // This issue was reproducible with DTA Automation loop running and reading tags as quickly as
    // possible, with regular usage this issue most likely is not a problem
    if (in_dta_mode() && NfcTag::getInstance().getActivationState() != NfcTag::Active)
    {
        ALOGE ("[DTA] tag already deactivated, not trying to read!");
        ALOGD ("%s: create emtpy buffer", __FUNCTION__);
        static uint8_t* empty = (uint8_t*) "";
        sReadDataLen = 0;
        sReadData = (uint8_t*) malloc (1);
        buf = e->NewByteArray (sReadDataLen);
        e->SetByteArrayRegion (buf, 0, sReadDataLen, (jbyte*) sReadData);
    }
    else{
    #endif // </DTA>
    if (sCheckNdefCurrentSize > 0)
    {
        {
            SyncEventGuard g (sReadEvent);
            sIsReadingNdefMessage = true;
            status = NFA_RwReadNDef ();
            sReadEvent.wait (); //wait for NFA_READ_CPLT_EVT
        }
        sIsReadingNdefMessage = false;

        if (sReadDataLen > 0) //if stack actually read data from the tag
        {
            ALOGD ("%s: read %u bytes", __FUNCTION__, sReadDataLen);
            buf = e->NewByteArray (sReadDataLen);
            e->SetByteArrayRegion (buf, 0, sReadDataLen, (jbyte*) sReadData);
        }
    }
    else
    {
        ALOGD ("%s: create empty buffer", __FUNCTION__);
        sReadDataLen = 0;
        sReadData = (uint8_t*) malloc (1);
        buf = e->NewByteArray (sReadDataLen);
        e->SetByteArrayRegion (buf, 0, sReadDataLen, (jbyte*) sReadData);
    }
    #ifdef DTA // <DTA>
    }
    #endif // </DTA>

    if (sReadData)
    {
        free (sReadData);
        sReadData = NULL;
    }
    sReadDataLen = 0;

    ALOGD ("%s: exit", __FUNCTION__);
    return buf;
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doWriteStatus
**
** Description:     Receive the completion status of write operation.  Called
**                  by NFA_WRITE_CPLT_EVT.
**                  isWriteOk: Status of operation.
**
** Returns:         None
**
*******************************************************************************/
void nativeNfcTag_doWriteStatus (jboolean isWriteOk)
{
    if (sWriteWaitingForComplete != JNI_FALSE)
    {
        sWriteWaitingForComplete = JNI_FALSE;
        sWriteOk = isWriteOk;
        sem_post (&sWriteSem);
    }
}


/*******************************************************************************
**
** Function:        nativeNfcTag_formatStatus
**
** Description:     Receive the completion status of format operation.  Called
**                  by NFA_FORMAT_CPLT_EVT.
**                  isOk: Status of operation.
**
** Returns:         None
**
*******************************************************************************/
void nativeNfcTag_formatStatus (bool isOk)
{
    sFormatOk = isOk;
    sem_post (&sFormatSem);
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doWrite
**
** Description:     Write a NDEF message to the tag.
**                  e: JVM environment.
**                  o: Java object.
**                  buf: Contains a NDEF message.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nativeNfcTag_doWrite (JNIEnv* e, jobject, jbyteArray buf)
{
    jboolean result = JNI_FALSE;
    tNFA_STATUS status = 0;
    const int maxBufferSize = 1024;
    UINT8 buffer[maxBufferSize] = { 0 };
    UINT32 curDataSize = 0;
    #ifdef DTA // <DTA>
    UINT32 p=0;
    #endif // <DTA>

    ScopedByteArrayRO bytes(e, buf);
    UINT8* p_data = const_cast<UINT8*>(reinterpret_cast<const UINT8*>(&bytes[0])); // TODO: const-ness API bug in NFA_RwWriteNDef!

    ALOGD ("%s: enter; len = %zu", __FUNCTION__, bytes.size());

    /* Create the write semaphore */
    if (sem_init (&sWriteSem, 0, 0) == -1)
    {
        ALOGE ("%s: semaphore creation failed (errno=0x%08x)", __FUNCTION__, errno);
        return JNI_FALSE;
    }

    sWriteWaitingForComplete = JNI_TRUE;
    if (sCheckNdefStatus == NFA_STATUS_FAILED)
    {
        //if tag does not contain a NDEF message
        //and tag is capable of storing NDEF message
        if (sCheckNdefCapable)
        {
            ALOGD ("%s: try format", __FUNCTION__);
            sem_init (&sFormatSem, 0, 0);
            sFormatOk = false;
            status = NFA_RwFormatTag ();
            sem_wait (&sFormatSem);
            sem_destroy (&sFormatSem);
            if (sFormatOk == false) //if format operation failed
                goto TheEnd;
        }
        ALOGD ("%s: try write", __FUNCTION__);
        status = NFA_RwWriteNDef (p_data, bytes.size());
    }
    else if (bytes.size() == 0)
    {
        //if (NXP TagWriter wants to erase tag) then create and write an empty ndef message
        NDEF_MsgInit (buffer, maxBufferSize, &curDataSize);
        status = NDEF_MsgAddRec (buffer, maxBufferSize, &curDataSize, NDEF_TNF_EMPTY, NULL, 0, NULL, 0, NULL, 0);
        ALOGD ("%s: create empty ndef msg; status=%u; size=%lu", __FUNCTION__, status, curDataSize);
        status = NFA_RwWriteNDef (buffer, curDataSize);
    }
    else
    {
    #ifdef DTA // <DTA>
        if (in_dta_mode())
        {
            //T1T BV4 write req
            data_len = bytes.size();
            if((t1t_dyn_activated==TRUE) && (data_len == 254))
            {
                t1t_tag_data = (UINT8*)malloc(data_len+2);
                if(t1t_tag_data != NULL)
                {
                    memset(t1t_tag_data, 0x00, sizeof((data_len + 2)));
                    ALOGD ("%s: copy data to be sent for dynamic blocks", __FUNCTION__);
                    memcpy (t1t_tag_data,p_data,data_len);
                    t1t_tag_data[data_len]=0xFE;
                    data_len++;
                    t1t_tag_data[data_len]=0x00;
                    /* make CC0 = 0x00 as write starts*/


                    t1t_init_blocks[8] = 0x00;
                    NFA_RwT1tWrite8(1,t1t_init_blocks+8,1);
                }
            }
            else
            {
                ALOGD ("%s: NFA_RwWriteNDef", __FUNCTION__);
                status = NFA_RwWriteNDef (p_data, bytes.size());
            }
        }
        else
        {
    #endif
            ALOGD ("%s: NFA_RwWriteNDef", __FUNCTION__);
            status = NFA_RwWriteNDef (p_data, bytes.size());
    #ifdef DTA // <DTA>
        }
    #endif // </DTA>
    }

    if (status != NFA_STATUS_OK)
    {
        ALOGE ("%s: write/format error=%d", __FUNCTION__, status);
        goto TheEnd;
    }

    /* Wait for write completion status */
    sWriteOk = false;
    if (sem_wait (&sWriteSem))
    {
        ALOGE ("%s: wait semaphore (errno=0x%08x)", __FUNCTION__, errno);
        goto TheEnd;
    }

    result = sWriteOk;

TheEnd:
    /* Destroy semaphore */
    if (t1t_tag_data != NULL)
    {
        free(t1t_tag_data);
        t1t_tag_data = NULL;
    }
    if (sem_destroy (&sWriteSem))
    {
        ALOGE ("%s: failed destroy semaphore (errno=0x%08x)", __FUNCTION__, errno);
    }
    sWriteWaitingForComplete = JNI_FALSE;
    ALOGD ("%s: exit; result=%d", __FUNCTION__, result);
    return result;
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doConnectStatus
**
** Description:     Receive the completion status of connect operation.
**                  isConnectOk: Status of the operation.
**
** Returns:         None
**
*******************************************************************************/
void nativeNfcTag_doConnectStatus (jboolean isConnectOk)
{
    if (sConnectWaitingForComplete != JNI_FALSE)
    {
        sConnectWaitingForComplete = JNI_FALSE;
        sConnectOk = isConnectOk;
        SyncEventGuard g (sReconnectEvent);
        sReconnectEvent.notifyOne ();
    }
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doDeactivateStatus
**
** Description:     Receive the completion status of deactivate operation.
**
** Returns:         None
**
*******************************************************************************/
void nativeNfcTag_doDeactivateStatus (int status)
{
    sGotDeactivate = (status == 0);

    SyncEventGuard g (sReconnectEvent);
    sReconnectEvent.notifyOne ();
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doConnect
**
** Description:     Connect to the tag in RF field.
**                  e: JVM environment.
**                  o: Java object.
**                  targetHandle: Handle of the tag.
**
** Returns:         Must return NXP status code, which NFC service expects.
**
*******************************************************************************/
static jint nativeNfcTag_doConnect (JNIEnv*, jobject, jint targetHandle)
{
    ALOGD ("%s: targetHandle = %d", __FUNCTION__, targetHandle);
    int i = targetHandle;
    NfcTag& natTag = NfcTag::getInstance ();
    int retCode = NFCSTATUS_SUCCESS;

    sNeedToSwitchRf = false;
    if (i >= NfcTag::MAX_NUM_TECHNOLOGY)
    {
        ALOGE ("%s: Handle not found", __FUNCTION__);
        retCode = NFCSTATUS_FAILED;
        goto TheEnd;
    }

    if (natTag.getActivationState() != NfcTag::Active)
    {
        ALOGE ("%s: tag already deactivated", __FUNCTION__);
        retCode = NFCSTATUS_FAILED;
        goto TheEnd;
    }

    sCurrentConnectedTargetType = natTag.mTechList[i];
    if (natTag.mTechLibNfcTypes[i] != NFC_PROTOCOL_ISO_DEP)
    {
        ALOGD ("%s() Nfc type = %d, do nothing for non ISO_DEP", __FUNCTION__, natTag.mTechLibNfcTypes[i]);
        retCode = NFCSTATUS_SUCCESS;
        goto TheEnd;
    }

    if (natTag.mTechList[i] == TARGET_TYPE_ISO14443_3A || natTag.mTechList[i] == TARGET_TYPE_ISO14443_3B)
    {
        ALOGD ("%s: switching to tech: %d need to switch rf intf to frame", __FUNCTION__, natTag.mTechList[i]);
        sNeedToSwitchRf = true;
    }
    else
    {
        retCode = switchRfInterface(NFA_INTERFACE_ISO_DEP) ? NFA_STATUS_OK : NFA_STATUS_FAILED;
    }

TheEnd:
    ALOGD ("%s: exit 0x%X", __FUNCTION__, retCode);
    return retCode;
}

/*******************************************************************************
**
** Function:        reSelect
**
** Description:     Deactivates the tag and re-selects it with the specified
**                  rf interface.
**
** Returns:         status code, 0 on success, 1 on failure,
**                  146 (defined in service) on tag lost
**
*******************************************************************************/
static int reSelect (tNFA_INTF_TYPE rfInterface, bool fSwitchIfNeeded)
{
    tNFA_INTF_TYPE currentRfInterface = NfcTag::getInstance().getRfInterface();

    ALOGD ("%s: enter; rf intf = %d, current intf = %d", __FUNCTION__, rfInterface, currentRfInterface);

    sRfInterfaceMutex.lock ();

    if (fSwitchIfNeeded && (rfInterface == currentRfInterface))
    {
        // already in the requested interface
        sRfInterfaceMutex.unlock ();
        return 0;   // success
    }

    NfcTag& natTag = NfcTag::getInstance ();

    tNFA_STATUS status;
    int rVal = 1;

    do
    {
        //if tag has shutdown, abort this method
        if (NfcTag::getInstance ().isNdefDetectionTimedOut())
        {
            ALOGD ("%s: ndef detection timeout; break", __FUNCTION__);
            rVal = STATUS_CODE_TARGET_LOST;
            break;
        }

        {
            SyncEventGuard g (sReconnectEvent);
            gIsTagDeactivating = true;
            sGotDeactivate = false;
            ALOGD ("%s: deactivate to sleep", __FUNCTION__);
            if (NFA_STATUS_OK != (status = NFA_Deactivate (TRUE))) //deactivate to sleep state
            {
                ALOGE ("%s: deactivate failed, status = %d", __FUNCTION__, status);
                break;
            }

            if (sReconnectEvent.wait (1000) == false) //if timeout occurred
            {
                ALOGE ("%s: timeout waiting for deactivate", __FUNCTION__);
            }
        }

        if (!sGotDeactivate)
        {
            rVal = STATUS_CODE_TARGET_LOST;
            break;
        }

        if (NfcTag::getInstance ().getActivationState () != NfcTag::Sleep)
        {
            ALOGE ("%s: tag is not in sleep", __FUNCTION__);
            rVal = STATUS_CODE_TARGET_LOST;
            break;
        }

        gIsTagDeactivating = false;

        {
            SyncEventGuard g2 (sReconnectEvent);

            sConnectWaitingForComplete = JNI_TRUE;
            ALOGD ("%s: select interface %u", __FUNCTION__, rfInterface);
            gIsSelectingRfInterface = true;
            if (NFA_STATUS_OK != (status = NFA_Select (natTag.mTechHandles[0], natTag.mTechLibNfcTypes[0], rfInterface)))
            {
                ALOGE ("%s: NFA_Select failed, status = %d", __FUNCTION__, status);
                break;
            }

            sConnectOk = false;
            if (sReconnectEvent.wait (1000) == false) //if timeout occured
            {
                ALOGE ("%s: timeout waiting for select", __FUNCTION__);
                break;
            }
        }

        ALOGD("%s: select completed; sConnectOk=%d", __FUNCTION__, sConnectOk);
        if (NfcTag::getInstance ().getActivationState () != NfcTag::Active)
        {
            ALOGE("%s: tag is not active", __FUNCTION__);
            rVal = STATUS_CODE_TARGET_LOST;
            break;
        }
        if (sConnectOk)
        {
            rVal = 0;   // success
        }
        else
        {
            rVal = 1;
        }
    } while (0);

    sConnectWaitingForComplete = JNI_FALSE;
    gIsTagDeactivating = false;
    gIsSelectingRfInterface = false;
    sRfInterfaceMutex.unlock ();
    ALOGD ("%s: exit; status=%d", __FUNCTION__, rVal);
    return rVal;
}

/*******************************************************************************
**
** Function:        switchRfInterface
**
** Description:     Switch controller's RF interface to frame, ISO-DEP, or NFC-DEP.
**                  rfInterface: Type of RF interface.
**
** Returns:         True if ok.
**
*******************************************************************************/
static bool switchRfInterface (tNFA_INTF_TYPE rfInterface)
{
    ALOGD ("%s: rf intf = %d", __FUNCTION__, rfInterface);
    NfcTag& natTag = NfcTag::getInstance ();

    if (natTag.mTechLibNfcTypes[0] != NFC_PROTOCOL_ISO_DEP)
    {
        ALOGD ("%s: protocol: %d not ISO_DEP, do nothing", __FUNCTION__, natTag.mTechLibNfcTypes[0]);
        return true;
    }

    ALOGD ("%s: new rf intf = %d, cur rf intf = %d", __FUNCTION__, rfInterface, NfcTag::getInstance().getRfInterface());

    return (0 == reSelect(rfInterface, true));
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doReconnect
**
** Description:     Re-connect to the tag in RF field.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         Status code.
**
*******************************************************************************/
static jint nativeNfcTag_doReconnect (JNIEnv*, jobject)
{
    ALOGD ("%s: enter", __FUNCTION__);
    int retCode = NFCSTATUS_SUCCESS;
    tNFA_STATUS status = NFCSTATUS_FAILED;
    NfcTag& natTag = NfcTag::getInstance ();
    uint8_t value = ENABLE_CRC_PARITY_ADDITION_IN_NFCC;

    if (natTag.getActivationState() != NfcTag::Active)
    {
        ALOGE ("%s: tag already deactivated", __FUNCTION__);
        retCode = NFCSTATUS_FAILED;
        goto TheEnd;
    }

    // special case for Kovio
    if (NfcTag::getInstance ().mTechList [0] == TARGET_TYPE_KOVIO_BARCODE)
    {
        ALOGD ("%s: fake out reconnect for Kovio", __FUNCTION__);
        goto TheEnd;
    }

     // this is only supported for type 2 or 4 (ISO_DEP) tags
    if (natTag.mTechLibNfcTypes[0] == NFA_PROTOCOL_ISO_DEP)
    {
        ALOGD ("%s: reSelect for ISO-DEP if necessary", __FUNCTION__);

        retCode = reSelect(NFA_INTERFACE_ISO_DEP, true);
    }
    else if (natTag.mTechLibNfcTypes[0] == NFA_PROTOCOL_T2T)
    {
        if(NfcTag::getInstance ().mTechList [1] == TARGET_TYPE_MIFARE_CLASSIC)
        {
            if(sCrcOffParamSet == true)
            {
                if(setPropNciParameter(NCI_PARAM_ID_OFF_CRC_PARITY_IN_NFCC, &value, CRC_PARITY_PARAM_LEN))
                {
                    sCrcOffParamSet = false;
                }
                else
                {
                    ALOGE ("%s: CRC Param set failed", __FUNCTION__);
                    retCode = NFCSTATUS_FAILED;
                    goto TheEnd;
                }
            }
        }
        retCode = reSelect(NFA_INTERFACE_FRAME, false);
    }
    if(retCode == STATUS_CODE_TARGET_LOST)
    {
        if(NfcTag::getInstance ().mTechList [1] == TARGET_TYPE_MIFARE_CLASSIC)
        {
            ALOGD ("%s: Mifare Classic tag absent", __FUNCTION__);
            // Presence check has failed so reset crc param and disconnect EE.
            if(sCrcOffParamSet == true)
            {
                if(setPropNciParameter(NCI_PARAM_ID_OFF_CRC_PARITY_IN_NFCC, &value, CRC_PARITY_PARAM_LEN))
                {
                    sEseInit = false;
                    sCrcOffParamSet = false;
                }
                else
                {
                    ALOGE ("%s: CRC Param set failed", __FUNCTION__);
                    retCode = NFCSTATUS_FAILED;
                    goto TheEnd;
                }
            }
            if(sEseInit == true)
            {
                ALOGD("%s : Disconnecting eSE", __FUNCTION__);
                ApduGateManager::getInstance().close((tNFA_HANDLE)sEseId);
            }
        }
    }

TheEnd:
    ALOGD ("%s: exit 0x%X", __FUNCTION__, retCode);
    return retCode;
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doHandleReconnect
**
** Description:     Re-connect to the tag in RF field.
**                  e: JVM environment.
**                  o: Java object.
**                  targetHandle: Handle of the tag.
**
** Returns:         Status code.
**
*******************************************************************************/
static jint nativeNfcTag_doHandleReconnect (JNIEnv *e, jobject o, jint targetHandle)
{
    uint8_t sIsWaiting = FALSE;
    NfcTag::getInstance().WaitStatus(&sIsWaiting);
    ALOGD ("%s: targetHandle = %d", __FUNCTION__, targetHandle);
    return nativeNfcTag_doConnect (e, o, targetHandle);
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doDisconnect
**
** Description:     Deactivate the RF field.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nativeNfcTag_doDisconnect (JNIEnv*, jobject)
{
    ALOGD ("%s: enter", __FUNCTION__);

    NfcTag::getInstance().resetAllTransceiveTimeouts ();

    if (NfcTag::getInstance ().getActivationState () != NfcTag::Active)
    {
        ALOGE ("%s: tag already deactivated", __FUNCTION__);
        goto TheEnd;
    }

TheEnd:
    ALOGD ("%s: exit", __FUNCTION__);
    return  (JNI_TRUE);
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doTransceiveStatus
**
** Description:     Receive the completion status of transceive operation.
**                  status: operation status.
**                  buf: Contains tag's response.
**                  bufLen: Length of buffer.
**
** Returns:         None
**
*******************************************************************************/
void nativeNfcTag_doTransceiveStatus (tNFA_STATUS status, uint8_t* buf, uint32_t bufLen)
{
    SyncEventGuard g (sTransceiveEvent);
    ALOGD ("%s: data len=%d", __FUNCTION__, bufLen);
    if (!sWaitingForTransceive)
    {
        ALOGE ("%s: drop data", __FUNCTION__);
        return;
    }
    sRxDataStatus = status;
    if (sRxDataStatus == NFA_STATUS_OK || sRxDataStatus == NFA_STATUS_CONTINUE)
        sRxDataBuffer.append (buf, bufLen);

    if (sRxDataStatus == NFA_STATUS_OK)
        sTransceiveEvent.notifyOne ();
}


void nativeNfcTag_notifyRfTimeout ()
{
    SyncEventGuard g (sTransceiveEvent);
    ALOGD ("%s: waiting for transceive: %d", __FUNCTION__, sWaitingForTransceive);
    if (!sWaitingForTransceive)
        return;

    sTransceiveRfTimeout = true;

    sTransceiveEvent.notifyOne ();
}
/*******************************************************************************************************
**
** Function:        nativeNfcTag_MifareClassicAuthenticate
**
** Description:     Handles mifare classic authentication sequence as per pearl eSE sequence.frames
**                  C-APDU , decodes R-APDu and frames tag frames(cmds) also(main mifare encrypted cmds)
**
** Returns:         None.
**
*******************************************************************************************************/
void nativeNfcTag_MifareClassicAuthenticate(void)
{
     uint8_t i = 0;

     ALOGD ("%s:  sMifareClassicState = %X",__FUNCTION__,sMifareClassicState);

    if((sMifareClassicState == MIFR_CLASSIC_MUTUAL_AUTHENTICATION_KEY_A) ||
       (sMifareClassicState == MIFR_CLASSIC_MUTUAL_AUTHENTICATION_KEY_B))
    {
        ALOGD ("%s:  sMifareClassicInAuthState  = %X",__FUNCTION__,sMifareClassicInAuthState);
        switch(sMifareClassicInAuthState)
        {
             case START_READER_MODE_SESSION:
                 sMifareClassicInAuthState = MIFARE_TAG_AUTH;
                 if(gCmd)
                 {
                     gCmd[0] = 0x00;                                  // CLA
                     gCmd[1] = START_MIFARE_READER_MODE_SESSION;      // INS
                     gCmd[2] = START_SESSION;                         // P1
                     gCmd[3] = 0;                                     // P2 (RFU)
                     gCmd[4] = gTagFrameLen;                           // data field length
                     // make the cmd packet as per OBT pearl eSE as from framework we will recieve
                     // UID(from byte 2 until byte 7) before key.
                     for(i=0;i<MIFARE_AUTH_CMD_LEN;i++)
                     {
                         if((i >=2) && (i<=7))
                         {
                             gCmd[i+FILLED_CMD_LEN] = gTagFrame[i+4];
                         }
                         else if(i<2)
                         {
                             gCmd[i+FILLED_CMD_LEN] = gTagFrame[i];
                         }
                         else
                         {
                             gCmd[i+FILLED_CMD_LEN] = gTagFrame[i-6];
                         }
                     }

                     gCmd[i+FILLED_CMD_LEN] = NO_DATA_RETURNED;        // Le

                     gCmdlen = FILLED_CMD_LEN+gTagFrameLen+LE_LEN;
                     sMifareClassicInAuthState = MIFARE_TAG_AUTH;

                     sMifareClassicOpDone = true;
                  }
                  else
                  {
                      ALOGE("%s : Mem Allocation failed in start reader mode pass auth ",__FUNCTION__);
                  }
                  break;
             case MIFARE_TAG_AUTH:
                  //Tag command framed in transcieve.
                  if(!gFirstAuth)
                  {
                      sMifareClassicInAuthState = START_READER_MODE_THREE_PASS_AUTH;
                      gFirstAuth = true;
                  }
                  else
                  {
                      sMifareClassicInAuthState = END_READER_MODE_THREE_PASS_AUTH;
                      gFirstAuth = false;
                  }
                  break;
             case START_READER_MODE_THREE_PASS_AUTH:
                  if(gCmd)
                  {
                      gCmd[0] = 0x00;                                  // CLA
                      gCmd[1] = MIFARE_READER_MODE_AUTHENTICATION;     // INS
                      gCmd[2] = ENCRYPT_CHALLENG_PHASE;                // P1
                      gCmd[3] = 0;                                     // P2 (RFU)
                      gCmd[4] = gTagFrameLen;                           // data field length
                      for(i=0;i<gTagFrameLen;i++)                      // cmd data:(Mifare Auth Rsp1)
                      {
                          gCmd[i+FILLED_CMD_LEN] = gTagFrame[i];
                      }

                      gCmd[i+FILLED_CMD_LEN] = DATA_RETURNED;          // Le

                      gCmdlen = THREE_PASS_AUTH_CMD_LEN;
                      sMifareClassicInAuthState = MIFARE_TAG_AUTH;
                  }
                  else
                  {
                      ALOGE("%s : cmd buff NULL in START_READER_MODE_THREE_PASS_AUTH",__FUNCTION__);
                  }
                  break;
             case END_READER_MODE_THREE_PASS_AUTH:
                  if(gCmd)
                  {
                      gCmd[0] = 0x00;                                  // CLA
                      gCmd[1] = MIFARE_READER_MODE_AUTHENTICATION;     // INS
                      gCmd[2] = FINISH_THREE_PASS_AUTH;                // P1
                      gCmd[3] = 0;                                     // P2 (RFU)
                      gCmd[4] = gTagFrameLen;                           // data field length
                      for(i=0;i<gTagFrameLen;i++)                      // cmd data:(Mifare Auth Rsp1)
                      {
                         gCmd[i+FILLED_CMD_LEN] = gTagFrame[i];
                      }

                      gCmd[i+FILLED_CMD_LEN] = NO_DATA_RETURNED;       // Le
                      gCmdlen = THREE_PASS_AUTH_CMD_LEN;
                      sMifareClassicOpDone = false;
                      gReturnData = false;
                   }
                   else
                   {
                      ALOGE("%s :cmd buff NULL in END_READER_MODE_THREE_PASS_AUTH ",__FUNCTION__);
                   }
                   break;
         }
    }
}
/*************************************************************************************************
**
** Function:        nativeNfcTag_MifareClassicRead
**
** Description:     frames the C-APDU for pearl eSE to get the encrypted/decrypted read cmd/rsp
**                   which will have the read cmd and block number in it(coming from framework).
**
** Returns:         None.
**
**************************************************************************************************/
void nativeNfcTag_MifareClassicRead(void)
{
    uint8_t i = 0;

    ALOGD ("%s: gMifareClReadState = %X ",__FUNCTION__,gMifareClReadState);

    //Check current state whether encrypt or decrypt
    switch(gMifareClReadState)
    {
        case ENCRYPT_CMD :
            gCmd[0] = 0;
            gCmd[1] = ENCRYPT_DECRYPT_DATA;
            gCmd[2] = ENCRYPT_CMD;
            gCmd[3] = RFU;
            gCmd[4] = gTagFrameLen;
            for(i=0;i<gTagFrameLen;i++)
            {
                gCmd[i+FILLED_CMD_LEN] = gTagFrame[i];
            }

            gCmd[i+FILLED_CMD_LEN] = NO_DATA_RETURNED;
            gCmdlen = READ_ENCRYPT_CAPDU_LEN;

            //change state to decrypt.
            gMifareClReadState = DECRYPT_RSP;
            sMifareClassicOpDone = true;
            break;
        case DECRYPT_RSP:
            // forward this packet to eSE for decryption
            gCmd[0] = 0;
            gCmd[1] = ENCRYPT_DECRYPT_DATA;
            gCmd[2] = DECRYPT_RSP;
            gCmd[3] = RFU;
            gCmd[4] = gTagFrameLen;
            for(i=0;i<gTagFrameLen;i++)
            {
                gCmd[i+CMD_DATA_OFFSET] = gTagFrame[i];
            }

            gCmd[i+FILLED_CMD_LEN] = NO_DATA_RETURNED;
            gCmdlen = CMD_DATA_OFFSET+gTagFrameLen+LE_LEN;

            //forward the ese rsp back to java
            sMifareClassicOpDone = false;
            // nxt will be read encrypt
            gMifareClReadState = ENCRYPT_CMD;
            sMifareClAuthSteps = 0;
            break;
        default :
            break;
    }
}
/*****************************************************************************************************
**
** Function:        nativeNfcTag_MifareClassicWrite
**
** Description:     frames C-APDU and tag frames to handle all write sequence invoked from
**                  app/framework. Supports extended write.
**
** Returns:         None.
**
******************************************************************************************************/
void nativeNfcTag_MifareClassicWrite(void)
{
    uint8_t i = 0;
    ALOGD ("%s: gMifareWriteState = %X ",__FUNCTION__,gMifareWriteState);

    switch(gMifareWriteState)
    {
        case MIFARE_WRITE_ENCRYPT_CMD_APDU :
            gCmd[0] = 0;
            gCmd[1] = ENCRYPT_DECRYPT_DATA;
            gCmd[2] = ENCRYPT_WRITE_CMD;
            gCmd[3] = RFU;
            gCmd[4] = gTagFrameLen;
            for(i=0;i<gTagFrameLen;i++)
            {
                gCmd[i+FILLED_CMD_LEN] = gTagFrame[i];
            }

            gCmd[i+FILLED_CMD_LEN] = WRITE_DATA_RETURN;
            gCmdlen = i+FILLED_CMD_LEN+LE_LEN;

            //change state to MIFARE_WRITE_SEND_ADDR.
             gMifareWriteState = MIFARE_WRITE_SEND_ADDR;

             sMifareClassicOpDone = true;
             break;
        case MIFARE_WRITE_SEND_ADDR:
             // decode write r-apdu here and store ack1 , data and ack2 to be sent later(extended write);
             //initial 5 bytes are write addr cmd
             // ack1 1 byte
             //data 21 data
             // ack2 1 byte
             if((gTagFrameLen != 0) && (gTagFrameLen <= sRecvBufferMaxSize))
             {
                 memcpy(gCmd,gTagFrame,(uint32_t)gTagFrameLen);
             }
             else
             {
                ALOGE ("%s: gTagFrameLen is 0",__FUNCTION__);
                return;
             }
             gCmdlen = (uint32_t)gTagFrameLen;
             buf_len = WRITE_ADDR_LEN;

             //change state to MIFARE_WRITE_CHECK_ACK1.
             gMifareWriteState = MIFARE_WRITE_CHECK_ACK1;
             break;
        case MIFARE_WRITE_CHECK_ACK1:
             // check if the response recieved from tag is same as ack1
             if((gTagFrame[0] & 0x0F) == *(gCmd+WRITE_ADDR_LEN))
             {
                 ALOGD ("%s: ACK1 is same/correct ",__FUNCTION__);
                 //change state to MIFARE_WRITE_SEND_DATA.
                 gMifareWriteState = MIFARE_WRITE_SEND_DATA;
             }
             else
             {
                  // error :NACK
                  ALOGD ("%s: AKC1 is incorrect ",__FUNCTION__);
                  sErrorOccured = true;
                  sMifareClassicOpDone = false;
                  break;
             }
        case MIFARE_WRITE_SEND_DATA:
             // ACK1 is correct so send data now.
             memcpy(gTagFrame,gCmd+WRITE_ADDR_LEN+ACK1_LEN,WRITE_DATA_LEN);
             buf_len = WRITE_DATA_LEN;
             //change state to MIFARE_WRITE_CHECK_ACK2.
             gMifareWriteState = MIFARE_WRITE_CHECK_ACK2;
             break;
        case MIFARE_WRITE_CHECK_ACK2:
             //check if ACK2 is the one which is expected and if yes then write result
             if((gTagFrame[0] & 0x0F) == *(gCmd+WRITE_ADDR_LEN+ACK1_LEN+WRITE_DATA_LEN))
             {
                 ALOGD ("%s: ACK2 is same/correct ",__FUNCTION__);
             }
             else
             {
                  // Error : NACK.
                  ALOGD ("%s: AKC2 is incorrect ",__FUNCTION__);
                  sErrorOccured = true;
             }
             sMifareClassicOpDone = false;
             gMifareWriteCompleted = true;
             gReturnData = false;
             break;
         default:
             break;
    }
}
/*****************************************************************************************************
**
** Function:        nativeNfcTag_MifareClassicIncDecRes
**
** Description:     Frames C-APDU and tag frames to handle Increment or decrement or restore operation
**                  with Mifare classic tag based on app/framework request.Frames the C-APDU , decodes
**                  R-APDU and frames raw frame to be sent to mifare classic tags.
*
** Returns:         None.
**
******************************************************************************************************/
bool nativeNfcTag_MifareClassicIncDecRes(void)
{
    uint8_t i = 0;
    ALOGD ("%s: gMifareState = %X gTagFrameLen=%X",__FUNCTION__,gMifareState,gTagFrameLen);

     if((gTagFrameLen<MIFR_RESTORE_CMD_LEN) && (sMifareClassicState == MIFR_CLASSIC_CARD_RESTORE)
         && (gMifareState != MIFARE_CHECK_ACK))
     {
         ALOGD ("%s: Restore Cmd Len is wrong",__FUNCTION__);
         return false;
     }

    // Special case of Restore cmd : nfc framework will send cmd 2 bytes long but DH needs to
    // send 6 bytes of data as per OBT doc so pad other 4 bytes with value 0 and increse length to 6.
    if((sMifareClassicState == MIFR_CLASSIC_CARD_RESTORE) && (gMifareState == MIFARE_ENCRYPT_CMD_APDU))
    {
        gTagFrameLen += 4;
        gTagFrame[2] = 0;
        gTagFrame[3] = 0;
        gTagFrame[4] = 0;
        gTagFrame[5] = 0;
    }
    switch(gMifareState)
    {
        case MIFARE_ENCRYPT_CMD_APDU :
            gCmd[0] = 0;
            gCmd[1] = ENCRYPT_DECRYPT_DATA;
            gCmd[2] = INC_DEC_RESTORE;
            gCmd[3] = RFU;
            gCmd[4] = gTagFrameLen;
            for(i=0;i<gTagFrameLen;i++)
            {
                gCmd[i+FILLED_CMD_LEN] = gTagFrame[i];
            }

            gCmd[i+FILLED_CMD_LEN] = INC_DEC_RES_DATA_RETURN;
            gCmdlen = i+FILLED_CMD_LEN+LE_LEN;

            //change state to MIFARE_SEND_ADDR.
             gMifareState = MIFARE_SEND_ADDR;

             sMifareClassicOpDone = true;
             break;
         case MIFARE_SEND_ADDR:
             // decode write r-apdu here and store ack1 , data and ack2 to be sent later;
             //initial 5 bytes are write addr cmd
             // ack1 1 byte
             // val 7 byte
             // store data in already allocated cmd buff
             if((gTagFrameLen != 0) && (gTagFrameLen <= sRecvBufferMaxSize))
             {
                 memcpy(gCmd,gTagFrame,(uint32_t)gTagFrameLen);
             }
             else
             {
                 ALOGE ("%s: gTagFrameLen is 0 ",__FUNCTION__);
                 return false;
             }
             gCmdlen = (uint32_t)gTagFrameLen;
             buf_len = WRITE_ADDR_LEN;
             //change state to MIFARE_CHECK_ACK.
             gMifareState = MIFARE_CHECK_ACK;
             break;
        case MIFARE_CHECK_ACK:
             // check if the response recieved from tag is same as ack1
             if((gTagFrame[0] & 0x0F) == *(gCmd+WRITE_ADDR_LEN))
             {
                 ALOGD ("%s: ACK1 is same/correct ",__FUNCTION__);
                 //change state to MIFARE_SEND_VALUE.
                 gMifareState = MIFARE_SEND_VALUE;
             }
             else
             {
                 // error
                 sMifareClassicOpDone = false;
                 sErrorOccured = true;
                 ALOGD ("%s: AKC1 is incorrect ",__FUNCTION__);
                 break;
             }
        case MIFARE_SEND_VALUE:
             // ACK1 is correct so send data now.
             memcpy(gTagFrame,gCmd+WRITE_ADDR_LEN+ACK1_LEN,INC_DEC_RES_DATA_LEN);
             buf_len = INC_DEC_RES_DATA_LEN;
             //change state to MIFARE_TRANS_TIMEOUT.
             gMifareState = MIFARE_TRANS_TIMEOUT;
             gReturnData = false;
             break;
        case MIFARE_TRANS_TIMEOUT:
             sMifareClassicOpDone = false;
             gMifareWriteCompleted = true;
             break;
        default:
             break;
    }
    return true;
}
/**************************************************************************************************
**
** Function:        nativeNfcTag_MifareClassicTransfer
**
** Description:     This function does the trasfer operation with mifare classic tag and handles all
**                  possible states of operation as per the transfer sequence.
**
** Returns:         None.
**
*************************************************************************************************/
void nativeNfcTag_MifareClassicTransfer(void)
{
    uint8_t i = 0;
    ALOGD ("%s: gMifareState = %X ",__FUNCTION__,gMifareState);

    switch(gMifareState)
    {
        case MIFARE_ENCRYPT_CMD_APDU :
            gCmd[0] = 0;
            gCmd[1] = ENCRYPT_DECRYPT_DATA;
            gCmd[2] = TRANSFER;
            gCmd[3] = RFU;
            gCmd[4] = gTagFrameLen;
            for(i=0;i<gTagFrameLen;i++)
            {
                gCmd[i+FILLED_CMD_LEN] = gTagFrame[i];
            }

            gCmd[i+FILLED_CMD_LEN] = INC_DEC_RES_DATA_RETURN;
            gCmdlen = i+FILLED_CMD_LEN+LE_LEN;

            //change state to MIFARE_SEND_ADDR.
             gMifareState = MIFARE_SEND_ADDR;

            sMifareClassicOpDone = true;
            break;
        case MIFARE_SEND_ADDR:
             // decode write r-apdu here and store ack1 , data and ack2 to be sent later;
             //initial 5 bytes are write addr cmd
             // ack1 1 byte
             // val 7 byte
             // store data in already allocated cmd buff
             if((gTagFrameLen != 0) && (gTagFrameLen <= sRecvBufferMaxSize))
             {
                 memcpy(gCmd,gTagFrame,(uint32_t)gTagFrameLen);
             }
             else
             {
                 ALOGE ("%s: gTagFrameLen is 0",__FUNCTION__);
                 return;
             }
             gCmdlen = (uint32_t)gTagFrameLen;
             buf_len = WRITE_ADDR_LEN;

             //change state to MIFARE_CHECK_ACK.
             gMifareState = MIFARE_CHECK_ACK;
             break;
        case MIFARE_CHECK_ACK:
             // check if the response recieved from tag is same as ack1
             if((gTagFrame[0] & 0x0F) == *(gCmd+WRITE_ADDR_LEN))
             {
                 ALOGD ("%s: ACK1 is same/correct ",__FUNCTION__);
             }
             else
                 ALOGE ("%s: ACK1 is incorrect ",__FUNCTION__);

             gMifareWriteCompleted = true;
             sMifareClassicOpDone = false;
             gReturnData = false;
             break;
         default:
             break;
    }
}
/*************************************************************************************************
**
** Function:        nativeNfcTag_resetPearleSECryptoEngine
**
** Description:     frames End MifareReaderModeSession C-APDU to be sent to eSE.Pearl eSE crypto
**                  engine needs to be reset if the operation is done or any error occured e.g
**                  no response from tag or eSE error status.
**
** Returns:         None.
**
**************************************************************************************************/
void nativeNfcTag_resetPearleSECryptoEngine(void)
{
    gCmd[0] = 0;
    gCmd[1] = START_MIFARE_READER_MODE_SESSION;
    gCmd[2] = END_MIFARE_READER_MODE_SESSION;
    gCmd[3] = RFU;
    gCmd[4] = 0;
    gCmdlen = END_MIFARE_CAPDU_LEN;
    sMifareClassicOpDone = true;
}
/*****************************************************************************************
**
** Function:        nativeNfcTag_checkMifareclassicState
**
** Description:     Checks the current command coming from app/framework for mifare
**                  classic tag(e.g authenticate,read,write,increment, decrement etc)
**                  and frames the corresponding C-APDU for pearl eSE.R-APDU contains
**                  the RF frame to be sent to mifare classic tag.
**
** Returns:         None.
**
*******************************************************************************************/
bool nativeNfcTag_checkMifareclassicState()
{
    ALOGD ("%s: sMifareClassicState = %X",__FUNCTION__,sMifareClassicState);
    switch(sMifareClassicState)
    {
        case MIFR_CLASSIC_MUTUAL_AUTHENTICATION_KEY_A:
        case MIFR_CLASSIC_MUTUAL_AUTHENTICATION_KEY_B:
            nativeNfcTag_MifareClassicAuthenticate();
            break;
        case MIFR_CLASSIC_CARD_WRITE:
            nativeNfcTag_MifareClassicWrite();
            break;
        case MIFR_CLASSIC_CARD_READ:
            if(sMifareClAuthSteps == 1)
            {
                sMifareClAuthSteps++;
                return true;
            }
            sMifareClAuthSteps++;
            nativeNfcTag_MifareClassicRead();
            break;
        case MIFR_CLASSIC_CARD_INCREMENT:
        case MIFR_CLASSIC_CARD_DECREMENT:
        case MIFR_CLASSIC_CARD_RESTORE:
            if(nativeNfcTag_MifareClassicIncDecRes() == false)
            {
                return false;
            }
            break;
        case MIFR_CLASSIC_CARD_TRANSFER:
            nativeNfcTag_MifareClassicTransfer();
            break;
       case MIFR_END_READER_MODE_SESSION:
            nativeNfcTag_resetPearleSECryptoEngine();
            break;
        default:
            return false;
    }
    return true;
}

/*******************************************************************************
**
** Function:        nativeNfcTag_initeSEForMifareClassic
**
** Description:     Initializes pearl eSE when the mifare classic tag is detected
**                  and activated.
**
** Returns:         Status  TRUE - Init Success  OR FALSE -- init Fail
**
*******************************************************************************/
bool nativeNfcTag_initeSEForMifareClassic(void)
{
    INT32 recvBufferActualSize = 0;
    uint8_t *selectCapduBuff = NULL;
    uint8_t value = 0;
    tNFA_STATUS status = NFA_STATUS_FAILED;
    NfcTag::getInstance().mIsMifareClassicOp = true;
    //AID of reader applet of pearl eSE.
    uint8_t obtReaderAppletId[APPLET_AID_LENGTH] = {0xA0,0x00,0x00,0x00,0x77, \
                                                    0x01,0x07,0x00,0x11,0x10,0x00, \
                                                    0x01,0x00,0x00,0x00,0x1D};


    if(!GetNumValue("MIFARE_CLASSIC_SUPPORT_SECURE_ELEMENT", &sEseId, sizeof(sEseId)))
    {
        return false;
    }

    ALOGD ("%s: Connecting to eSE ",__FUNCTION__);
   // if(SecureElement::getInstance().connectEE())
    if(ApduGateManager::getInstance().open((tNFA_HANDLE)sEseId))
    {
        ALOGD ("%s: Connect passed",__FUNCTION__);
        // this parameter will disable CRC and parity bit addition in the frame by NFCC.
        // this parameter must be rest to 0x00 once there is mifare tag operation error or
        // it completes.
        value = DISABLE_CRC_PARITY_ADDITION_IN_NFCC;
        if(setPropNciParameter(NCI_PARAM_ID_OFF_CRC_PARITY_IN_NFCC, &value, CRC_PARITY_PARAM_LEN))
        {
            sCrcOffParamSet = true;
        }
        else
        {
            ALOGE("%s: Could not configure no CRC addition", __FUNCTION__);
            return false;
        }
        // send SELECT commnd first to select applet
        selectCapduBuff = new uint8_t [APPLET_AID_LENGTH+SELECT_CMD_HEADER_LEN+1];
        if(selectCapduBuff)
        {
            selectCapduBuff[0] = 0x00;                                                          //CLA
            selectCapduBuff[1] = 0xA4;                                                          //SELECT
            selectCapduBuff[2] = 0x04;                                                          //P1
            selectCapduBuff[3] = 0x00;                                                          //P2
            selectCapduBuff[4] = APPLET_AID_LENGTH;
            memcpy(selectCapduBuff+SELECT_CMD_HEADER_LEN,obtReaderAppletId,sizeof(obtReaderAppletId));

            selectCapduBuff[SELECT_CMD_HEADER_LEN+APPLET_AID_LENGTH] = 0x00;
            gCmdlen = SELECT_CMD_HEADER_LEN+APPLET_AID_LENGTH+LE_LEN;                            //Le
            if(!ApduGateManager::getInstance().transceive(selectCapduBuff,gCmdlen, gRecvBuffer, sRecvBufferMaxSize, recvBufferActualSize))
            {
                ALOGE (" %s: Select of OBT eSE reader mode Applet failed  ",__FUNCTION__);      // Presence check has failed so reset crc param and disconnect EE.puneet
                ALOGD("%s : Disconnecting eSE", __FUNCTION__);
               // SecureElement::getInstance().disconnectEE(gSEId);
                ApduGateManager::getInstance().close((tNFA_HANDLE)sEseId);
                if(sCrcOffParamSet == true)
                {
                    value = ENABLE_CRC_PARITY_ADDITION_IN_NFCC;
                    if(setPropNciParameter(NCI_PARAM_ID_OFF_CRC_PARITY_IN_NFCC, &value, CRC_PARITY_PARAM_LEN))
                    {
                        sEseInit = false;
                        sCrcOffParamSet = false;
                    }
                    else
                    {
                        ALOGD("%s : CRC parameter set failed", __FUNCTION__);
                        return false;
                    }
                }
                NfcTag::getInstance().mIsMifareClassicOp = false;
                delete [] selectCapduBuff;
                gCmdlen =0;
                return false;
            }
            delete [] selectCapduBuff;
            gCmdlen =0;
        }
        else
        {
            ALOGE (" %s: Mem allocation failed while sending SELECT commnd  ",__FUNCTION__);
            sEseInit = false;
            return false;
        }
        sEseInit = true;
    }
    else
    {
        ALOGE ("%s: Connect failed",__FUNCTION__);
        return false;
    }
    return true;
}
/***********************************************************************************************
**
** Function:        nativeNfcTag_doMifareClassicPresenceCheck
**
** Description:     Checks that if tag is present in proximity when authentication failing
**                   because of wrong key.
**
** Returns:         false if tag is in proximity(and tarnsaction failed due to I/O error)
**                  STATUS_CODE_TARGET_LOST if tag is not in proximity now.
********************************************************************************************/
uint8_t nativeNfcTag_doMifareClassicPresenceCheck(void)
{
    UINT8 retCode = 0;
    UINT8 value = ENABLE_CRC_PARITY_ADDITION_IN_NFCC;
    if(setPropNciParameter(NCI_PARAM_ID_OFF_CRC_PARITY_IN_NFCC, &value, CRC_PARITY_PARAM_LEN))
    {
        sCrcOffParamSet = false;
    }
    else
    {
        return false;
    }

    retCode = reSelect(NFA_INTERFACE_FRAME, false);
    if(retCode != STATUS_CODE_TARGET_LOST)
    {
        value = DISABLE_CRC_PARITY_ADDITION_IN_NFCC;
        if(setPropNciParameter(NCI_PARAM_ID_OFF_CRC_PARITY_IN_NFCC, &value, CRC_PARITY_PARAM_LEN))
        {
            sCrcOffParamSet = true;
        }
        /*return false only to framework/app as tag is still in proximity*/
        return false;
    }
    else if(retCode == STATUS_CODE_TARGET_LOST)
    {
        /*Tag is not in proximity*/
        return STATUS_CODE_TARGET_LOST;
    }
    return false;
}
/***********************************************************************************************
**
** Function:        nativeNfcTag_doMifareTransaction
**
** Description:     Transacts with pearl eSE as well as miafer classic tag depending on the
**                  states and further sequence under it.
**                  data - frame recieved from app framework or tag or eSE.
**                  dataLen - frame length
**
** Returns:         none.
********************************************************************************************/
uint8_t nativeNfcTag_doMifareTransaction(uint8_t *data,uint8_t *dataLen)
{
    INT32 recvBufferActualSize = 0;
    tNFA_STATUS status = NFA_STATUS_FAILED;
    int i =0;
    jint *targetLost = NULL;
    bool waitOk = false;
    bool isNack = false;
    bool capdu_failed = false;
    int timeout = 200;
    if(!sEseInit)
    {
        if(!nativeNfcTag_initeSEForMifareClassic())
            return false;                                           // Error condition
    }
    if(sCrcOffParamSet == false)
    {
        UINT8 value = DISABLE_CRC_PARITY_ADDITION_IN_NFCC;
        if(setPropNciParameter(NCI_PARAM_ID_OFF_CRC_PARITY_IN_NFCC, &value, CRC_PARITY_PARAM_LEN))
        {
            sCrcOffParamSet = true;
        }
    }
    sMifareClAuthSteps = 0;
    sSendtoSE = false;
    sMifareClassicState = gTagFrame[0];                             // cmd will tell the state.
    gMifareWriteState = MIFARE_WRITE_ENCRYPT_CMD_APDU;
    gMifareState = MIFARE_ENCRYPT_CMD_APDU ;
    gMifareWriteCompleted = false;
    isMifareClassicPresent = true;
    gMifareClReadState = ENCRYPT_CMD;
    sMifareClassicInAuthState = START_READER_MODE_SESSION;
    gMifareWriteState = MIFARE_WRITE_ENCRYPT_CMD_APDU;
    gFirstAuth = false;
    sErrorOccured = false;
    do{
          if(isMifareClassicPresent == false)
          {
              ALOGD ("%s:Mifare presence check failed . end reader mode session",__FUNCTION__);
              sSendtoSE = true;
              sMifareClassicState = MIFR_END_READER_MODE_SESSION;
              capdu_failed = true;
          }
          if(nativeNfcTag_checkMifareclassicState() == false)
          {
              ALOGD ("%s: Mifare Classic Unknown Command..",__FUNCTION__);
              return false;
          }

          if(sErrorOccured == true)
          {
              sSendtoSE = true;
              sMifareClassicState = MIFR_END_READER_MODE_SESSION;
              if(nativeNfcTag_checkMifareclassicState() == false)
              {
                  ALOGD ("%s: Mifare Classic Unknown Command..return false",__FUNCTION__);
                  return false;
              }
              capdu_failed = true;
              sMifareClassicOpDone = true;
              *dataLen = 0;
          }

          if(!sSendtoSE)
              sSendtoSE = true;
          else
              sSendtoSE = false;

          // if its write operation , then change data to be sent to tag back to cater write sequence req.
          if((gMifareWriteState == MIFARE_WRITE_CHECK_ACK2) || ( gMifareState == MIFARE_TRANS_TIMEOUT))
          {
              ALOGD ("%s: Send data to tag",__FUNCTION__);
              sSendtoSE = false;
          }
          // no else if because we do not know at which sequence the tag may go away..
          if(sMifareClassicState == MIFR_END_READER_MODE_SESSION)
          {
              sSendtoSE = true;
              sMifareClassicOpDone = false;
              gMifareWriteCompleted = false;
              capdu_failed = false;
          }
          if(gMifareWriteCompleted == false)                                                   // check for write is completed if its write state.
          {
              if(!sSendtoSE)
              {
                  sTransceiveRfTimeout = false;
                  sWaitingForTransceive = true;
                //  sTransceiveDataLen = 0;

                  sRxDataStatus = NFA_STATUS_OK;
                  sRxDataBuffer.clear ();

                  if((sMifareClassicState != MIFR_CLASSIC_CARD_READ) &&
                     (sMifareClassicState != MIFR_CLASSIC_MUTUAL_AUTHENTICATION_KEY_B) &&
                     (sMifareClassicState!=MIFR_CLASSIC_MUTUAL_AUTHENTICATION_KEY_A))
                  {
                      gTagFrameLen = buf_len;
                  }
                  SyncEventGuard g (sTransceiveEvent);
                  uint8_t mifareClassicTimeout = 750;                                          // Presence check timeout
                  status = NFA_SendRawFrame (gTagFrame, gTagFrameLen, mifareClassicTimeout);

                  sMifareClassicOpDone = true;                                                 // as tag rsp nvr to be reported directly to java .
                  if (status != NFA_STATUS_OK)
                  {
                      ALOGE ("%s: fail send; error=%d", __FUNCTION__, status);
                      break;
                  }
                  waitOk = sTransceiveEvent.wait (timeout);

                  if (waitOk == false || sTransceiveRfTimeout)                                 //if timeout occurred
                  {
                      ALOGE ("%s: wait response timeout timeout=%d", __FUNCTION__,timeout);

                      if(((sMifareClassicState == MIFR_CLASSIC_CARD_INCREMENT) ||
                          (sMifareClassicState == MIFR_CLASSIC_CARD_DECREMENT) ||
                          (sMifareClassicState == MIFR_CLASSIC_CARD_RESTORE)))
                      {
                          // this is special case of miafre classic where you will not get the value inc dec
                          // or res result and timeout will happen.(Success)
                          if( gMifareState == MIFARE_TRANS_TIMEOUT)
                             sMifareClassicOpDone = false;                                    // report back to java .
                      }
                      else
                      {
                          *dataLen = 0;
                          capdu_failed = true;
                          sMifareClassicState = MIFR_END_READER_MODE_SESSION;
                      }
                  }
              }
              else
              {
                   ApduGateManager::getInstance().transceive(gCmd,gCmdlen, gRecvBuffer, sRecvBufferMaxSize, recvBufferActualSize);
                   if(gRecvBuffer[recvBufferActualSize-2] != 0x90)
                   {
                       sMifareClassicOpDone = true;                                          // to cater the cases of last c-apdu failures.
                   }
              }
          }

          if(sMifareClassicOpDone == true)
          {
              if(sSendtoSE)
              {
                  // if response from eSE.
                  if((((gRecvBuffer[recvBufferActualSize-1] == 0x01) && (gRecvBuffer[recvBufferActualSize-2] == NORMAL_ENDING_CMD_BYTE)) ||
                     ((gRecvBuffer[recvBufferActualSize-1] == 0x03) && (gRecvBuffer[recvBufferActualSize-2] == NORMAL_ENDING_CMD_BYTE)) ||
                     ((gRecvBuffer[recvBufferActualSize-1] == 0x00) && (gRecvBuffer[recvBufferActualSize-2] == NORMAL_ENDING_CMD_BYTE))) && (capdu_failed == false))
                  {
                      gTagFrameLen = recvBufferActualSize-2;                                   // remove status bytes
                      if(gTagFrameLen != 0)
                      {
                          memcpy(gTagFrame,gRecvBuffer,gTagFrameLen);                            // tag cmd.
                      }
                      else
                      {
                          // error case as
                          ALOGE("%s  : gTagFrameLen is 0",__FUNCTION__);
                          return false;
                      }
                      ALOGD("%s  : eSE response - gTagFrameLen=%d  : %x %x ",__FUNCTION__,gTagFrameLen,gTagFrame[0]);
                  }
                  else
                  {
                      // If during the transaction , Tag moves away then tag crypto engine will reset so DH should send
                      // end mifare reader mode session c-apdu to pearl eSE to reset its crypto engine.
                      ALOGD("%s  : eSE R_APDU returns wrong status...",__FUNCTION__);

                      if(sMifareClassicState != MIFR_END_READER_MODE_SESSION)
                      {
                          capdu_failed = true;
                          sMifareClassicOpDone = true;                                        // wait for r-apdu
                          sMifareClassicState = MIFR_END_READER_MODE_SESSION;
                      }
                      else
                      {
                          sMifareClassicOpDone = false;
                          capdu_failed = false;
                           *dataLen = 0;
                      }
                  }
              } // sSendtoSE
              else
              {
                  // rsp from mifare classic tag
            //      gTagFrameLen = sTransceiveDataLen;
                  gTagFrameLen = sRxDataBuffer.size();
                  if((gTagFrameLen != 0) && (gTagFrameLen <= sRecvBufferMaxSize))
                  {
                      ALOGD("%s : Copy tag response",__FUNCTION__);
                      //  memcpy(gTagFrame,sTransceiveData,gTagFrameLen);
                      memcpy(gTagFrame,sRxDataBuffer.data(),gTagFrameLen);
                  }
              }
         } // sMifareClassicOpDone = true
         else
         {
             //transaction with mifare tag is done. return data.
             if((sMifareClassicState == MIFR_CLASSIC_CARD_READ) && (recvBufferActualSize > STATUS_BYTES_LEN))
             {
                 //only in read operation ,  data is to be sent to upper layers. In other operation return
                 //only one byte to full fill transceive req.
                 memcpy(data,gRecvBuffer,(recvBufferActualSize-STATUS_BYTES_LEN));
                 *dataLen = recvBufferActualSize-STATUS_BYTES_LEN;
             }
             else
             {
                 if((capdu_failed == false) && (sMifareClassicState == MIFR_END_READER_MODE_SESSION))
                 {
                     /* Some Error occured .Check if Tag is still in proximity so that host can send further request
                        from app to mifare classic tag*/
                     return nativeNfcTag_doMifareClassicPresenceCheck();
                 }
                 else if(capdu_failed == false)
                 {
                     *data = 0x00;
                     *dataLen = 1;
                 }
             }
         }
    }while(sMifareClassicOpDone);
    return true;
}

static void switchBackTimerProc (union sigval)
{
     ALOGD ("%s", __FUNCTION__);
     switchRfInterface(NFA_INTERFACE_ISO_DEP);
}
/*******************************************************************************
**
** Function:        nativeNfcTag_doTransceive
**
** Description:     Send raw data to the tag; receive tag's response.
**                  e: JVM environment.
**                  o: Java object.
**                  raw: Not used.
**                  statusTargetLost: Whether tag responds or times out.
**
** Returns:         Response from tag.
**
*******************************************************************************/
static jbyteArray nativeNfcTag_doTransceive (JNIEnv* e, jobject, jbyteArray data, jboolean raw, jintArray statusTargetLost)
{
    int timeout = NfcTag::getInstance ().getTransceiveTimeout (sCurrentConnectedTargetType);
    uint8_t *transdata={0},transdataLen=0;
    jbyteArray mifareClassicTagRsp = NULL;
    #ifdef DTA // <DTA>
    /* in case of invalid timeout value use default */
    if (in_dta_mode())
    {
        if (timeout == 0)
            timeout = 20000;
    }
    #endif // </DTA>
    ALOGD ("%s: enter; raw=%u; timeout = %d", __FUNCTION__, raw, timeout);
    bool fNeedToSwitchBack = false;
    bool waitOk = false;
    bool isNack = false;
    jint *targetLost = NULL;

    if ((NfcTag::getInstance ().getActivationState () != NfcTag::Active)&& (NfcTag::getInstance().mIsMifareClassicOp != true))
    {
        if (statusTargetLost)
        {
            targetLost = e->GetIntArrayElements (statusTargetLost, 0);
            if (targetLost)
                *targetLost = 1; //causes NFC service to throw TagLostException
            e->ReleaseIntArrayElements (statusTargetLost, targetLost, 0);
        }
        ALOGD ("%s: tag not active", __FUNCTION__);
        return NULL;
    }

    NfcTag& natTag = NfcTag::getInstance ();

    // get input buffer and length from java call
    ScopedByteArrayRO bytes(e, data);
    uint8_t* buf = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&bytes[0])); // TODO: API bug; NFA_SendRawFrame should take const*!
    size_t bufLen = bytes.size();

    if (statusTargetLost)
    {
        targetLost = e->GetIntArrayElements (statusTargetLost, 0);
        if (targetLost)
            *targetLost = 0; //success, tag is still present
    }

    sSwitchBackTimer.kill ();
    ScopedLocalRef<jbyteArray> result(e, NULL);
    if(NfcTag::getInstance().mIsMifareClassicOp == true)
    {
        if(raw == true)
        {
            return NULL;
        }
        uint8_t mifareclassicopstatus = 0;
         uint32_t mifrclassic_support = 0;
        //check if Oberthur mifare classic is supported.
         if(!GetNumValue("MIFARE_CLASSIC_SUPPORT_VIA_OBT_eSE", &mifrclassic_support, sizeof(mifrclassic_support)))
         {
             // if not then return false(No Support for the feature).
             return NULL;
         }

        //allocate buffer to send frames to mifare tag.
        gTagFrame = new uint8_t [sRecvBufferMaxSize];
        if(gTagFrame == NULL)
        {
             ALOGE("%s: gTagFrame mem allocation failed", __FUNCTION__);
             return NULL;
        }
        gCmd = new uint8_t [sRecvBufferMaxSize];
        if(gCmd == NULL)
        {
             ALOGE("%s: gCmd mem allocation failed", __FUNCTION__);
             return NULL;
        }
        gRecvBuffer = new uint8_t [sRecvBufferMaxSize];
        if(gRecvBuffer == NULL)
        {
             ALOGE("%s: gRecvBuffer mem allocation failed", __FUNCTION__);
             return NULL;
        }
        transdata = new uint8_t [sRecvBufferMaxSize];
        if(transdata == NULL)
        {
             ALOGE("%s: transdata mem allocation failed", __FUNCTION__);
             return NULL;
        }
        gTagFrameLen = bufLen;
        if((gTagFrameLen != 0) && (gTagFrameLen <= sRecvBufferMaxSize))
        {
            memcpy(gTagFrame,buf,bufLen);
        }
        else
        {
            ALOGE("%s: Transcieve data is NULL", __FUNCTION__);
            return NULL;
        }
        mifareclassicopstatus = nativeNfcTag_doMifareTransaction(transdata,&transdataLen);
        if(mifareclassicopstatus == false)
        {
            ALOGE("%s: Cmd failed..return NULL", __FUNCTION__);
            return NULL;
        }
        else if(mifareclassicopstatus == MIFR_CLASSIC_TAG_LOST)
        {
            ALOGE ("%s: Tag lost...", __FUNCTION__);
            if (targetLost)
                *targetLost = 1; //causes NFC service to throw TagLostException

            return NULL;
        }
        ALOGD ("%s: Return read data to java   : transdataLen=%X", __FUNCTION__,transdataLen);
        if(transdataLen !=0)
        {

           result.reset(e->NewByteArray(transdataLen));
           if (result.get() != NULL)
            {
                e->SetByteArrayRegion(result.get(), 0, transdataLen, (const jbyte *) transdata);
                e->ReleaseIntArrayElements (statusTargetLost, targetLost, 0);
                sRxDataBuffer.clear();
            }
        }
        delete [] gTagFrame;
        delete [] gCmd;
        delete [] gRecvBuffer;
        delete [] transdata;
        transdata = NULL;
        gTagFrame = NULL;
        gCmd = NULL;
        gRecvBuffer = NULL;
        transdataLen = 0;
        ALOGD ("%s: exit", __FUNCTION__);
        return result.release();
    }
    do
    {
        if(sNeedToSwitchRf) {
            if (!switchRfInterface (NFA_INTERFACE_FRAME)) //NFA_INTERFACE_ISO_DEP
            {
                 break;
            }
            fNeedToSwitchBack = true;
        }
        {
            SyncEventGuard g (sTransceiveEvent);
            sTransceiveRfTimeout = false;
            sWaitingForTransceive = true;
            sRxDataStatus = NFA_STATUS_OK;
            sRxDataBuffer.clear ();
            tNFA_STATUS status = NFA_SendRawFrame (buf, bufLen,
                    NFA_DM_DEFAULT_PRESENCE_CHECK_START_DELAY);
            if (status != NFA_STATUS_OK)
            {
                ALOGE ("%s: fail send; error=%d", __FUNCTION__, status);
                break;
            }
            waitOk = sTransceiveEvent.wait (timeout);
        }

        if (waitOk == false || sTransceiveRfTimeout) //if timeout occurred
        {
            ALOGE ("%s: wait response timeout", __FUNCTION__);
            if (targetLost)
                *targetLost = 1; //causes NFC service to throw TagLostException
            break;
        }

        if (NfcTag::getInstance ().getActivationState () != NfcTag::Active)
        {
            ALOGE ("%s: already deactivated", __FUNCTION__);
            if (targetLost)
                *targetLost = 1; //causes NFC service to throw TagLostException
            break;
        }

        ALOGD ("%s: response %d bytes", __FUNCTION__, sRxDataBuffer.size());

        if ((natTag.getProtocol () == NFA_PROTOCOL_T2T) &&
            natTag.isT2tNackResponse (sRxDataBuffer.data(), sRxDataBuffer.size()))
        {
            isNack = true;
        }

        if (sRxDataBuffer.size() > 0)
        {
            if (isNack)
            {
                //Some Mifare Ultralight C tags enter the HALT state after it
                //responds with a NACK.  Need to perform a "reconnect" operation
                //to wake it.
                ALOGD ("%s: try reconnect", __FUNCTION__);
                nativeNfcTag_doReconnect (NULL, NULL);
                ALOGD ("%s: reconnect finish", __FUNCTION__);
            }
            else
            {
                // marshall data to java for return
                result.reset(e->NewByteArray(sRxDataBuffer.size()));
                if (result.get() != NULL)
                {
                    e->SetByteArrayRegion(result.get(), 0, sRxDataBuffer.size(), (const jbyte *) sRxDataBuffer.data());
                }
                else
                    ALOGE ("%s: Failed to allocate java byte array", __FUNCTION__);
            } // else a nack is treated as a transceive failure to the upper layers

            sRxDataBuffer.clear();
        }
    } while (0);

    sWaitingForTransceive = false;
    if (targetLost)
        e->ReleaseIntArrayElements (statusTargetLost, targetLost, 0);

    if(fNeedToSwitchBack)
    {
        sSwitchBackTimer.set (1500, switchBackTimerProc);
    }
    ALOGD ("%s: exit", __FUNCTION__);
    return result.release();
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doGetNdefType
**
** Description:     Retrieve the type of tag.
**                  e: JVM environment.
**                  o: Java object.
**                  libnfcType: Type of tag represented by JNI.
**                  javaType: Not used.
**
** Returns:         Type of tag represented by NFC Service.
**
*******************************************************************************/
static jint nativeNfcTag_doGetNdefType (JNIEnv*, jobject, jint libnfcType, jint javaType)
{
    ALOGD ("%s: enter; libnfc type=%d; java type=%d", __FUNCTION__, libnfcType, javaType);
    jint ndefType = NDEF_UNKNOWN_TYPE;

    // For NFA, libnfcType is mapped to the protocol value received
    // in the NFA_ACTIVATED_EVT and NFA_DISC_RESULT_EVT event.
    switch (libnfcType) {
    case NFA_PROTOCOL_T1T:
        ndefType = NDEF_TYPE1_TAG;
        break;
    case NFA_PROTOCOL_T2T:
        ndefType = NDEF_TYPE2_TAG;;
        break;
    case NFA_PROTOCOL_T3T:
        ndefType = NDEF_TYPE3_TAG;
        break;
    case NFA_PROTOCOL_ISO_DEP:
        ndefType = NDEF_TYPE4_TAG;
        break;
    case NFA_PROTOCOL_ISO15693:
        ndefType = NDEF_UNKNOWN_TYPE;
        break;
    case NFA_PROTOCOL_INVALID:
        ndefType = NDEF_UNKNOWN_TYPE;
        break;
    default:
        ndefType = NDEF_UNKNOWN_TYPE;
        break;
    }
    ALOGD ("%s: exit; ndef type=%d", __FUNCTION__, ndefType);
    return ndefType;
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doCheckNdefResult
**
** Description:     Receive the result of checking whether the tag contains a NDEF
**                  message.  Called by the NFA_NDEF_DETECT_EVT.
**                  status: Status of the operation.
**                  maxSize: Maximum size of NDEF message.
**                  currentSize: Current size of NDEF message.
**                  flags: Indicate various states.
**
** Returns:         None
**
*******************************************************************************/
void nativeNfcTag_doCheckNdefResult (tNFA_STATUS status, uint32_t maxSize, uint32_t currentSize, uint8_t flags)
{
    //this function's flags parameter is defined using the following macros
    //in nfc/include/rw_api.h;
    //#define RW_NDEF_FL_READ_ONLY  0x01    /* Tag is read only              */
    //#define RW_NDEF_FL_FORMATED   0x02    /* Tag formated for NDEF         */
    //#define RW_NDEF_FL_SUPPORTED  0x04    /* NDEF supported by the tag     */
    //#define RW_NDEF_FL_UNKNOWN    0x08    /* Unable to find if tag is ndef capable/formated/read only */
    //#define RW_NDEF_FL_FORMATABLE 0x10    /* Tag supports format operation */

    #ifdef DTA // <DTA>
    if(in_dta_mode() )
    {
        tRW_T1T_CB *p_t1t  = &rw_cb.tcb.t1t;
        if(currentSize != 0)
        {
            if(p_t1t->mem[1] != 0x00 && p_t1t->mem[2] != 0x00 )
            {
                memcpy(t1t_init_blocks,p_t1t->mem,T1T_INIT_BLK_SIZE);
            }
            t1t_init_blocks[T1T_INIT_BLK_SIZE-1] = currentSize; //fill length in last byte
        }
    }
    #endif // </DTA>

    if (status == NFC_STATUS_BUSY)
    {
        ALOGE ("%s: stack is busy", __FUNCTION__);
        return;
    }

    if (!sCheckNdefWaitingForComplete)
    {
        ALOGE ("%s: not waiting", __FUNCTION__);
        return;
    }

    if (flags & RW_NDEF_FL_READ_ONLY)
        ALOGD ("%s: flag read-only", __FUNCTION__);
    if (flags & RW_NDEF_FL_FORMATED)
        ALOGD ("%s: flag formatted for ndef", __FUNCTION__);
    if (flags & RW_NDEF_FL_SUPPORTED)
        ALOGD ("%s: flag ndef supported", __FUNCTION__);
    if (flags & RW_NDEF_FL_UNKNOWN)
        ALOGD ("%s: flag all unknown", __FUNCTION__);
    if (flags & RW_NDEF_FL_FORMATABLE)
        ALOGD ("%s: flag formattable", __FUNCTION__);

    sCheckNdefWaitingForComplete = JNI_FALSE;
    sCheckNdefStatus = status;
    if (sCheckNdefStatus != NFA_STATUS_OK && sCheckNdefStatus != NFA_STATUS_TIMEOUT)
        sCheckNdefStatus = NFA_STATUS_FAILED;
    sCheckNdefCapable = false; //assume tag is NOT ndef capable
    if (sCheckNdefStatus == NFA_STATUS_OK)
    {
        //NDEF content is on the tag
        sCheckNdefMaxSize = maxSize;
        sCheckNdefCurrentSize = currentSize;
        sCheckNdefCardReadOnly = flags & RW_NDEF_FL_READ_ONLY;
        sCheckNdefCapable = true;
    }
    else if (sCheckNdefStatus == NFA_STATUS_FAILED)
    {
        //no NDEF content on the tag
        sCheckNdefMaxSize = 0;
        sCheckNdefCurrentSize = 0;
        sCheckNdefCardReadOnly = flags & RW_NDEF_FL_READ_ONLY;
        if ((flags & RW_NDEF_FL_UNKNOWN) == 0) //if stack understands the tag
        {
            if (flags & RW_NDEF_FL_SUPPORTED) //if tag is ndef capable
                sCheckNdefCapable = true;
        }
    }
    else
    {
        ALOGE ("%s: unknown status=0x%X", __FUNCTION__, status);
        sCheckNdefMaxSize = 0;
        sCheckNdefCurrentSize = 0;
        sCheckNdefCardReadOnly = false;
    }
    sem_post (&sCheckNdefSem);
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doCheckNdef
**
** Description:     Does the tag contain a NDEF message?
**                  e: JVM environment.
**                  o: Java object.
**                  ndefInfo: NDEF info.
**
** Returns:         Status code; 0 is success.
**
*******************************************************************************/
static jint nativeNfcTag_doCheckNdef (JNIEnv* e, jobject, jintArray ndefInfo)
{
    tNFA_STATUS status = NFA_STATUS_FAILED;
    jint* ndef = NULL;

    ALOGD ("%s: enter", __FUNCTION__);

    // special case for Kovio
    if (NfcTag::getInstance ().mTechList [0] == TARGET_TYPE_KOVIO_BARCODE)
    {
        ALOGD ("%s: Kovio tag, no NDEF", __FUNCTION__);
        ndef = e->GetIntArrayElements (ndefInfo, 0);
        ndef[0] = 0;
        ndef[1] = NDEF_MODE_READ_ONLY;
        e->ReleaseIntArrayElements (ndefInfo, ndef, 0);
        return NFA_STATUS_FAILED;
    }

    // special case for Kovio
    if (NfcTag::getInstance ().mTechList [0] == TARGET_TYPE_KOVIO_BARCODE)
    {
        ALOGD ("%s: Kovio tag, no NDEF", __FUNCTION__);
        ndef = e->GetIntArrayElements (ndefInfo, 0);
        ndef[0] = 0;
        ndef[1] = NDEF_MODE_READ_ONLY;
        e->ReleaseIntArrayElements (ndefInfo, ndef, 0);
        return NFA_STATUS_FAILED;
    }

    /* Create the write semaphore */
    if (sem_init (&sCheckNdefSem, 0, 0) == -1)
    {
        ALOGE ("%s: Check NDEF semaphore creation failed (errno=0x%08x)", __FUNCTION__, errno);
        return JNI_FALSE;
    }
    if(sCrcOffParamSet == true)
    {
        ALOGD ("%s: Setting CRC on param", __FUNCTION__);
        uint8_t value = ENABLE_CRC_PARITY_ADDITION_IN_NFCC;
        setPropNciParameter(NCI_PARAM_ID_OFF_CRC_PARITY_IN_NFCC, &value, CRC_PARITY_PARAM_LEN);
        sCrcOffParamSet = false;
    }

    if (NfcTag::getInstance ().getActivationState () != NfcTag::Active)
    {
        ALOGE ("%s: tag already deactivated", __FUNCTION__);
        goto TheEnd;
    }

    ALOGD ("%s: try NFA_RwDetectNDef", __FUNCTION__);
    sCheckNdefWaitingForComplete = JNI_TRUE;
    status = NFA_RwDetectNDef ();

    if (status != NFA_STATUS_OK)
    {
        ALOGE ("%s: NFA_RwDetectNDef failed, status = 0x%X", __FUNCTION__, status);
        goto TheEnd;
    }

    /* Wait for check NDEF completion status */
    if (sem_wait (&sCheckNdefSem))
    {
        ALOGE ("%s: Failed to wait for check NDEF semaphore (errno=0x%08x)", __FUNCTION__, errno);
        goto TheEnd;
    }

    if (sCheckNdefStatus == NFA_STATUS_OK)
    {
        //stack found a NDEF message on the tag
        ndef = e->GetIntArrayElements (ndefInfo, 0);
        if (NfcTag::getInstance ().getProtocol () == NFA_PROTOCOL_T1T)
            ndef[0] = NfcTag::getInstance ().getT1tMaxMessageSize ();
        else
            ndef[0] = sCheckNdefMaxSize;
        if (sCheckNdefCardReadOnly)
            ndef[1] = NDEF_MODE_READ_ONLY;
        else
            ndef[1] = NDEF_MODE_READ_WRITE;
        e->ReleaseIntArrayElements (ndefInfo, ndef, 0);
        status = NFA_STATUS_OK;
    }
    else if (sCheckNdefStatus == NFA_STATUS_FAILED)
    {
        //stack did not find a NDEF message on the tag;
        ndef = e->GetIntArrayElements (ndefInfo, 0);
        if (NfcTag::getInstance ().getProtocol () == NFA_PROTOCOL_T1T)
            ndef[0] = NfcTag::getInstance ().getT1tMaxMessageSize ();
        else
            ndef[0] = sCheckNdefMaxSize;
        if (sCheckNdefCardReadOnly)
            ndef[1] = NDEF_MODE_READ_ONLY;
        else
            ndef[1] = NDEF_MODE_READ_WRITE;
        e->ReleaseIntArrayElements (ndefInfo, ndef, 0);
        status = NFA_STATUS_FAILED;
    }
    else
    {
        ALOGD ("%s: unknown status 0x%X", __FUNCTION__, sCheckNdefStatus);
        status = sCheckNdefStatus;
    }

TheEnd:
    /* Destroy semaphore */
    if (sem_destroy (&sCheckNdefSem))
    {
        ALOGE ("%s: Failed to destroy check NDEF semaphore (errno=0x%08x)", __FUNCTION__, errno);
    }
    sCheckNdefWaitingForComplete = JNI_FALSE;
    ALOGD ("%s: exit; status=0x%X", __FUNCTION__, status);
    return status;
}


/*******************************************************************************
**
** Function:        nativeNfcTag_resetPresenceCheck
**
** Description:     Reset variables related to presence-check.
**
** Returns:         None
**
*******************************************************************************/
void nativeNfcTag_resetPresenceCheck ()
{
    sIsTagPresent = true;
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doPresenceCheckResult
**
** Description:     Receive the result of presence-check.
**                  status: Result of presence-check.
**
** Returns:         None
**
*******************************************************************************/
void nativeNfcTag_doPresenceCheckResult (tNFA_STATUS status)
{
    SyncEventGuard guard (sPresenceCheckEvent);
    sIsTagPresent = status == NFA_STATUS_OK;
    sPresenceCheckEvent.notifyOne ();
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doPresenceCheck
**
** Description:     Check if the tag is in the RF field.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if tag is in RF field.
**
*******************************************************************************/
static jboolean nativeNfcTag_doPresenceCheck (JNIEnv*, jobject)
{
    ALOGD ("%s", __FUNCTION__);
    tNFA_STATUS status = NFA_STATUS_OK;
    jboolean isPresent = JNI_FALSE;
    // Special case for Kovio.  The deactivation would have already occurred
    // but was ignored so that normal tag opertions could complete.  Now we
    // want to process as if the deactivate just happened.
    if (NfcTag::getInstance ().mTechList [0] == TARGET_TYPE_KOVIO_BARCODE)
    {
        ALOGD ("%s: Kovio, force deactivate handling", __FUNCTION__);
        tNFA_DEACTIVATED deactivated = {NFA_DEACTIVATE_TYPE_IDLE};
        {
            SyncEventGuard g (gDeactivatedEvent);
            gActivated = false; //guard this variable from multi-threaded access
            gDeactivatedEvent.notifyOne ();
        }

        NfcTag::getInstance().setDeactivationState (deactivated);
        nativeNfcTag_resetPresenceCheck();
        NfcTag::getInstance().connectionEventHandler (NFA_DEACTIVATED_EVT, NULL);
        nativeNfcTag_abortWaits();
        NfcTag::getInstance().abort ();

        return JNI_FALSE;
    }

    if (nfcManager_isNfcActive() == false)
    {
        ALOGD ("%s: NFC is no longer active.", __FUNCTION__);
        return JNI_FALSE;
    }

    if (!sRfInterfaceMutex.tryLock())
    {
        ALOGD ("%s: tag is being reSelected assume it is present", __FUNCTION__);
        return JNI_TRUE;
    }

    sRfInterfaceMutex.unlock();

    if (NfcTag::getInstance ().isActivated () == false)
    {
        ALOGD ("%s: tag already deactivated", __FUNCTION__);
        return JNI_FALSE;
    }

    if (sWaitingForTransceive == true)
    {
        ALOGD ("%s : doing transceive", __FUNCTION__);
        return JNI_TRUE;
    }

    {
        SyncEventGuard guard (sPresenceCheckEvent);
        status = NFA_RwPresenceCheck (NfcTag::getInstance().getPresenceCheckAlgorithm());
        if (status == NFA_STATUS_OK)
        {
            sPresenceCheckEvent.wait ();
            isPresent = sIsTagPresent ? JNI_TRUE : JNI_FALSE;
        }
    }

    if (isPresent == JNI_FALSE)
    {
        if(NfcTag::getInstance ().mTechList [1] == TARGET_TYPE_MIFARE_CLASSIC)
        {
            isMifareClassicPresent = false;
            ALOGD ("%s: Mifare Classic tag absent", __FUNCTION__);
            // Presence check has failed so reset crc param and disconnect EE.
            if(sCrcOffParamSet == true)
            {
                uint8_t value = ENABLE_CRC_PARITY_ADDITION_IN_NFCC;
                setPropNciParameter(NCI_PARAM_ID_OFF_CRC_PARITY_IN_NFCC, &value, CRC_PARITY_PARAM_LEN);
                sCrcOffParamSet = false;
            }
            if(sEseInit == true)
            {
                ALOGD("%s : Disconnecting eSE", __FUNCTION__);
                ApduGateManager::getInstance().close((tNFA_HANDLE)sEseId);
                sEseInit = false;
            }
        }

        ALOGD ("%s: tag absent", __FUNCTION__);
    }
    return isPresent;
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doIsNdefFormatable
**
** Description:     Can tag be formatted to store NDEF message?
**                  e: JVM environment.
**                  o: Java object.
**                  libNfcType: Type of tag.
**                  uidBytes: Tag's unique ID.
**                  pollBytes: Data from activation.
**                  actBytes: Data from activation.
**
** Returns:         True if formattable.
**
*******************************************************************************/
static jboolean nativeNfcTag_doIsNdefFormatable (JNIEnv*,
        jobject, jint /*libNfcType*/, jbyteArray, jbyteArray,
        jbyteArray)
{
    jboolean isFormattable = JNI_FALSE;

    switch (NfcTag::getInstance().getProtocol())
    {
    case NFA_PROTOCOL_T1T:
    case NFA_PROTOCOL_ISO15693:
        isFormattable = JNI_TRUE;
        break;

    case NFA_PROTOCOL_T3T:
        isFormattable = NfcTag::getInstance().isFelicaLite() ? JNI_TRUE : JNI_FALSE;
        break;

    case NFA_PROTOCOL_T2T:
        isFormattable = ( NfcTag::getInstance().isMifareUltralight() |
                          NfcTag::getInstance().isInfineonMyDMove() |
                          NfcTag::getInstance().isKovioType2Tag() )
                        ? JNI_TRUE : JNI_FALSE;
        break;
    }
    ALOGD("%s: is formattable=%u", __FUNCTION__, isFormattable);
    return isFormattable;
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doIsIsoDepNdefFormatable
**
** Description:     Is ISO-DEP tag formattable?
**                  e: JVM environment.
**                  o: Java object.
**                  pollBytes: Data from activation.
**                  actBytes: Data from activation.
**
** Returns:         True if formattable.
**
*******************************************************************************/
static jboolean nativeNfcTag_doIsIsoDepNdefFormatable (JNIEnv *e, jobject o, jbyteArray pollBytes, jbyteArray actBytes)
{
    uint8_t uidFake[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
    ALOGD ("%s", __FUNCTION__);
    jbyteArray uidArray = e->NewByteArray (8);
    e->SetByteArrayRegion (uidArray, 0, 8, (jbyte*) uidFake);
    return nativeNfcTag_doIsNdefFormatable (e, o, 0, uidArray, pollBytes, actBytes);
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doNdefFormat
**
** Description:     Format a tag so it can store NDEF message.
**                  e: JVM environment.
**                  o: Java object.
**                  key: Not used.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nativeNfcTag_doNdefFormat (JNIEnv*, jobject, jbyteArray)
{
    ALOGD ("%s: enter", __FUNCTION__);
    tNFA_STATUS status = NFA_STATUS_OK;

    // Do not try to format if tag is already deactivated.
    if (NfcTag::getInstance ().isActivated () == false)
    {
        ALOGD ("%s: tag already deactivated(no need to format)", __FUNCTION__);
        return JNI_FALSE;
    }

    sem_init (&sFormatSem, 0, 0);
    sFormatOk = false;
    status = NFA_RwFormatTag ();
    if (status == NFA_STATUS_OK)
    {
        ALOGD ("%s: wait for completion", __FUNCTION__);
        sem_wait (&sFormatSem);
        status = sFormatOk ? NFA_STATUS_OK : NFA_STATUS_FAILED;
    }
    else
        ALOGE ("%s: error status=%u", __FUNCTION__, status);
    sem_destroy (&sFormatSem);

    ALOGD ("%s: exit", __FUNCTION__);
    return (status == NFA_STATUS_OK) ? JNI_TRUE : JNI_FALSE;
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doMakeReadonlyResult
**
** Description:     Receive the result of making a tag read-only. Called by the
**                  NFA_SET_TAG_RO_EVT.
**                  status: Status of the operation.
**
** Returns:         None
**
*******************************************************************************/
void nativeNfcTag_doMakeReadonlyResult (tNFA_STATUS status)
{
    if (sMakeReadonlyWaitingForComplete != JNI_FALSE)
    {
        sMakeReadonlyWaitingForComplete = JNI_FALSE;
        sMakeReadonlyStatus = status;

        sem_post (&sMakeReadonlySem);
    }
}


/*******************************************************************************
**
** Function:        nativeNfcTag_doMakeReadonly
**
** Description:     Make the tag read-only.
**                  e: JVM environment.
**                  o: Java object.
**                  key: Key to access the tag.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nativeNfcTag_doMakeReadonly (JNIEnv*, jobject, jbyteArray)
{
    jboolean result = JNI_FALSE;
    tNFA_STATUS status;

    ALOGD ("%s", __FUNCTION__);

    /* Create the make_readonly semaphore */
    if (sem_init (&sMakeReadonlySem, 0, 0) == -1)
    {
        ALOGE ("%s: Make readonly semaphore creation failed (errno=0x%08x)", __FUNCTION__, errno);
        return JNI_FALSE;
    }

    sMakeReadonlyWaitingForComplete = JNI_TRUE;

    // Hard-lock the tag (cannot be reverted)
    status = NFA_RwSetTagReadOnly(TRUE);
    if (status == NFA_STATUS_REJECTED)
    {
        status = NFA_RwSetTagReadOnly (FALSE); //try soft lock
        if (status != NFA_STATUS_OK)
        {
            ALOGE ("%s: fail soft lock, status=%d", __FUNCTION__, status);
            goto TheEnd;
        }
    }
    else if (status != NFA_STATUS_OK)
    {
        ALOGE ("%s: fail hard lock, status=%d", __FUNCTION__, status);
        goto TheEnd;
    }

    /* Wait for check NDEF completion status */
    if (sem_wait (&sMakeReadonlySem))
    {
        ALOGE ("%s: Failed to wait for make_readonly semaphore (errno=0x%08x)", __FUNCTION__, errno);
        goto TheEnd;
    }

    if (sMakeReadonlyStatus == NFA_STATUS_OK)
    {
        result = JNI_TRUE;
    }

TheEnd:
    /* Destroy semaphore */
    if (sem_destroy (&sMakeReadonlySem))
    {
        ALOGE ("%s: Failed to destroy read_only semaphore (errno=0x%08x)", __FUNCTION__, errno);
    }
    sMakeReadonlyWaitingForComplete = JNI_FALSE;
    return result;
}


/*******************************************************************************
**
** Function:        nativeNfcTag_registerNdefTypeHandler
**
** Description:     Register a callback to receive NDEF message from the tag
**                  from the NFA_NDEF_DATA_EVT.
**
** Returns:         None
**
*******************************************************************************/
//register a callback to receive NDEF message from the tag
//from the NFA_NDEF_DATA_EVT;
void nativeNfcTag_registerNdefTypeHandler ()
{
    ALOGD ("%s", __FUNCTION__);
    sNdefTypeHandlerHandle = NFA_HANDLE_INVALID;
    NFA_RegisterNDefTypeHandler (TRUE, NFA_TNF_DEFAULT, (UINT8 *) "", 0, ndefHandlerCallback);
}


/*******************************************************************************
**
** Function:        nativeNfcTag_deregisterNdefTypeHandler
**
** Description:     No longer need to receive NDEF message from the tag.
**
** Returns:         None
**
*******************************************************************************/
void nativeNfcTag_deregisterNdefTypeHandler ()
{
    ALOGD ("%s", __FUNCTION__);
    NFA_DeregisterNDefTypeHandler (sNdefTypeHandlerHandle);
    sNdefTypeHandlerHandle = NFA_HANDLE_INVALID;
}


/*****************************************************************************
**
** JNI functions for Android 4.0.3
**
*****************************************************************************/
static JNINativeMethod gMethods[] =
{
   {"doConnect", "(I)I", (void *)nativeNfcTag_doConnect},
   {"doDisconnect", "()Z", (void *)nativeNfcTag_doDisconnect},
   {"doReconnect", "()I", (void *)nativeNfcTag_doReconnect},
   {"doHandleReconnect", "(I)I", (void *)nativeNfcTag_doHandleReconnect},
   {"doTransceive", "([BZ[I)[B", (void *)nativeNfcTag_doTransceive},
   {"doGetNdefType", "(II)I", (void *)nativeNfcTag_doGetNdefType},
   {"doCheckNdef", "([I)I", (void *)nativeNfcTag_doCheckNdef},
   {"doRead", "()[B", (void *)nativeNfcTag_doRead},
   {"doWrite", "([B)Z", (void *)nativeNfcTag_doWrite},
   {"doPresenceCheck", "()Z", (void *)nativeNfcTag_doPresenceCheck},
   {"doIsIsoDepNdefFormatable", "([B[B)Z", (void *)nativeNfcTag_doIsIsoDepNdefFormatable},
   {"doNdefFormat", "([B)Z", (void *)nativeNfcTag_doNdefFormat},
   {"doMakeReadonly", "([B)Z", (void *)nativeNfcTag_doMakeReadonly},
};


/*******************************************************************************
**
** Function:        register_com_android_nfc_NativeNfcTag
**
** Description:     Regisgter JNI functions with Java Virtual Machine.
**                  e: Environment of JVM.
**
** Returns:         Status of registration.
**
*******************************************************************************/
int register_com_android_nfc_NativeNfcTag (JNIEnv *e)
{
    ALOGD ("%s", __FUNCTION__);
    return jniRegisterNativeMethods (e, gNativeNfcTagClassName, gMethods, NELEM (gMethods));
}


} /* namespace android */
