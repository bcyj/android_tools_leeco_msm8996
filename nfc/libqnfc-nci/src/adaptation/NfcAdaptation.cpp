/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */
/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#include "OverrideLog.h"
#include "NfcAdaptation.h"
extern "C"
{
    #include "gki.h"
    #include "nfa_api.h"
    #include "nfc_int.h"
}
#include "config.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NfcAdapt"

extern "C" void GKI_shutdown();
extern void resetConfig();
extern "C" void verify_stack_non_volatile_store ();
extern "C" void delete_stack_non_volatile_store (BOOLEAN forceDelete);

NfcAdaptation* NfcAdaptation::mpInstance = NULL;
ThreadMutex NfcAdaptation::sLock;
nfc_nci_device_t* NfcAdaptation::mHalDeviceContext = NULL;
tHAL_NFC_CBACK* NfcAdaptation::mHalCallback = NULL;
tHAL_NFC_DATA_CBACK* NfcAdaptation::mHalDataCallback = NULL;
ThreadCondVar NfcAdaptation::mHalOpenCompletedEvent;
ThreadCondVar NfcAdaptation::mHalCloseCompletedEvent;
ThreadCondVar NfcAdaptation::mHalBertEvent;

UINT32 ScrProtocolTraceFlag = SCR_PROTO_TRACE_ALL; //0x017F00;
UINT8 appl_trace_level = 0xff;
char bcm_nfc_location[120];

static UINT8 nfa_dm_cfg[sizeof ( tNFA_DM_CFG ) ];
extern tNFA_DM_CFG *p_nfa_dm_cfg;
extern UINT8 nfa_ee_max_ee_cfg;
extern const UINT8  nfca_version_string [];
extern const UINT8  nfa_version_string [];

/*******************************************************************************
**
** Function:    NfcAdaptation::NfcAdaptation()
**
** Description: class constructor
**
** Returns:     none
**
*******************************************************************************/
NfcAdaptation::NfcAdaptation()
{
}

/*******************************************************************************
**
** Function:    NfcAdaptation::~NfcAdaptation()
**
** Description: class destructor
**
** Returns:     none
**
*******************************************************************************/
NfcAdaptation::~NfcAdaptation()
{
    mpInstance = NULL;
}

/*******************************************************************************
**
** Function:    NfcAdaptation::GetInstance()
**
** Description: access class singleton
**
** Returns:     pointer to the singleton object
**
*******************************************************************************/
NfcAdaptation& NfcAdaptation::GetInstance()
{
    AutoThreadMutex  a(sLock);

    if (!mpInstance)
        mpInstance = new NfcAdaptation;
    return *mpInstance;
}

/*******************************************************************************
**
** Function:    NfcAdaptation::Initialize()
**
** Description: class initializer
**
** Returns:     none
**
*******************************************************************************/
void NfcAdaptation::Initialize ()
{
    const char* func = "NfcAdaptation::Initialize";
    ALOGD("%s: enter", func);
    ALOGE("%s: ver=%s nfa=%s", func, nfca_version_string, nfa_version_string);
    unsigned long num;

    if ( !GetStrValue ( NAME_NFA_STORAGE, bcm_nfc_location, sizeof ( bcm_nfc_location ) ) )
    {
        memset (bcm_nfc_location, 0, sizeof(bcm_nfc_location));
        strncpy (bcm_nfc_location, "/data/nfc", 9);
    }
    if ( GetNumValue ( NAME_PROTOCOL_TRACE_LEVEL, &num, sizeof ( num ) ) )
        ScrProtocolTraceFlag = num;

    if ( GetStrValue ( NAME_NFA_DM_CFG, (char*)nfa_dm_cfg, sizeof ( nfa_dm_cfg ) ) )
        p_nfa_dm_cfg = ( tNFA_DM_CFG * )(void *) &nfa_dm_cfg[0];

    if ( GetNumValue ( NAME_NFA_MAX_EE_SUPPORTED, &num, sizeof ( num ) ) )
    {
        nfa_ee_max_ee_cfg = num;
        ALOGD("%s: Overriding NFA_EE_MAX_EE_SUPPORTED to use %d", func, nfa_ee_max_ee_cfg);
    }

    initializeGlobalAppLogLevel ();

    if ( GetNumValue ( NAME_PRESERVE_STORAGE, (char*)&num, sizeof ( num ) ) &&
            (num == 1) )
        ALOGD ("%s: preserve stack NV store", __FUNCTION__);
    else
    {
        delete_stack_non_volatile_store (FALSE);
    }

    GKI_init ();
    GKI_create_task ((TASKPTR)NFCA_TASK, BTU_TASK, (INT8*)"NFCA_TASK", 0, 0, (pthread_cond_t*)NULL, NULL);
    {
        AutoThreadMutex guard(mCondVar);
        GKI_create_task ((TASKPTR)Thread, MMI_TASK, (INT8*)"NFCA_THREAD", 0, 0, (pthread_cond_t*)NULL, NULL);
        mCondVar.wait();
    }

    mHalDeviceContext = NULL;
    mHalCallback =  NULL;
    memset (&mHalEntryFuncs, 0, sizeof(mHalEntryFuncs));
    InitializeHalDeviceContext ();
    ALOGD ("%s: exit", func);
}

/*******************************************************************************
**
** Function:    NfcAdaptation::Finalize()
**
** Description: class finalizer
**
** Returns:     none
**
*******************************************************************************/
void NfcAdaptation::Finalize()
{
    const char* func = "NfcAdaptation::Finalize";
    AutoThreadMutex  a(sLock);

    ALOGD ("%s: enter", func);
    GKI_shutdown ();   /// For release NFC task thread/ NFCA task thread

    resetConfig();

    nfc_nci_close(mHalDeviceContext); //close the HAL's device context
    mHalDeviceContext = NULL;
    mHalCallback = NULL;
    memset (&mHalEntryFuncs, 0, sizeof(mHalEntryFuncs));

    ALOGD ("%s: exit", func);
    delete this;
}

/*******************************************************************************
**
** Function:    NfcAdaptation::signal()
**
** Description: signal the CondVar to release the thread that is waiting
**
** Returns:     none
**
*******************************************************************************/
void NfcAdaptation::signal ()
{
    mCondVar.signal();
}

/*******************************************************************************
**
** Function:    NfcAdaptation::NFCA_TASK()
**
** Description: NFCA_TASK runs the GKI main task
**
** Returns:     none
**
*******************************************************************************/
UINT32 NfcAdaptation::NFCA_TASK (UINT32 arg)
{
    const char* func = "NfcAdaptation::NFCA_TASK";
    ALOGD ("%s: enter", func);
    GKI_run (0);
    ALOGD ("%s: exit", func);
    return 0;
}

/*******************************************************************************
**
** Function:    NfcAdaptation::Thread()
**
** Description: Creates work threads
**
** Returns:     none
**
*******************************************************************************/
UINT32 NfcAdaptation::Thread (UINT32 arg)
{
    const char* func = "NfcAdaptation::Thread";
    UINT8 tid;
    ALOGD ("%s: enter", func);

    {
        ThreadCondVar    CondVar;
        AutoThreadMutex  guard(CondVar);
        GKI_create_task ((TASKPTR)nfc_task, NFC_TASK, (INT8*)"NFC_TASK", 0, 0, (pthread_cond_t*)CondVar, (pthread_mutex_t*)CondVar);
        CondVar.wait();
    }

    NfcAdaptation::GetInstance().signal();

    if ((tid = GKI_get_taskid ()) < GKI_MAX_TASKS)
        GKI_exit_task (tid);
    else
        ALOGD ("%s: can not exit the task with %d ", func,tid);

    ALOGD ("%s: exit", func);
    return 0;
}

/*******************************************************************************
**
** Function:    NfcAdaptation::GetHalEntryFuncs()
**
** Description: Get the set of HAL entry points.
**
** Returns:     Functions pointers for HAL entry points.
**
*******************************************************************************/
tHAL_NFC_ENTRY* NfcAdaptation::GetHalEntryFuncs ()
{
    return &mHalEntryFuncs;
}

/*******************************************************************************
**
** Function:    NfcAdaptation::InitializeHalDeviceContext
**
** Description: Ask the generic Android HAL to find the Broadcom-specific HAL.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::InitializeHalDeviceContext ()
{
    const char* func = "NfcAdaptation::InitializeHalDeviceContext";
    ALOGD ("%s: enter", func);
    int ret = 0; //0 means success
    const hw_module_t* hw_module = NULL;

    mHalEntryFuncs.initialize = HalInitialize;
    mHalEntryFuncs.terminate = HalTerminate;
    mHalEntryFuncs.open = HalOpen;
    mHalEntryFuncs.close = HalClose;
    mHalEntryFuncs.core_initialized = HalCoreInitialized;
    mHalEntryFuncs.write = HalWrite;
    mHalEntryFuncs.prediscover = HalPrediscover;
    mHalEntryFuncs.control_granted = HalControlGranted;
    mHalEntryFuncs.power_cycle = HalPowerCycle;
    mHalEntryFuncs.get_max_ee = HalGetMaxNfcee;

    ret = hw_get_module (NFC_NCI_HARDWARE_MODULE_ID, &hw_module);
    if (ret == 0)
    {
        ret = nfc_nci_open (hw_module, &mHalDeviceContext);
        if (ret != 0)
            ALOGE ("%s: nfc_nci_open fail", func);
    }
    else
        ALOGE ("%s: fail hw_get_module", func);
    ALOGD ("%s: exit", func);
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalInitialize
**
** Description: Not implemented because this function is only needed
**              within the HAL.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalInitialize ()
{
    const char* func = "NfcAdaptation::HalInitialize";
    ALOGD ("%s", func);
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalTerminate
**
** Description: Not implemented because this function is only needed
**              within the HAL.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalTerminate ()
{
    const char* func = "NfcAdaptation::HalTerminate";
    ALOGD ("%s", func);
}

/*******************************************************************************
**
** Function:    NfcAdaptation::BertInit
**
** Description: Open HAL for BERT testing
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::BertInit()
{
    HalBertOpen();
}

/*******************************************************************************
**
** Function:    NfcAdaptation::BertDeInit
**
** Description: Close HAL after BERT testing
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::BertDeInit()
{
    HalBertClose();
}

/*******************************************************************************
**
** Function:    NfcAdaptation::DoBert
**
** Description: Send BERT commands to HAL
**
** Returns:     None.
**
*******************************************************************************/
bool NfcAdaptation::DoBert(tNFA_BERT_COMMAND& cmd)
{
    return SendBertCommands(cmd);
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalOpen
**
** Description: Turn on controller, download firmware.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalOpen (tHAL_NFC_CBACK *p_hal_cback, tHAL_NFC_DATA_CBACK* p_data_cback, UINT8 reset_status)
{
    const char* func = "NfcAdaptation::HalOpen";
    ALOGD ("%s", func);
    if (mHalDeviceContext)
    {
        mHalDeviceContext->common.reserved[0] = ANDROID_MODE;
        mHalDeviceContext->common.reserved[1] = reset_status;
        mHalCallback = p_hal_cback;
        mHalDataCallback = p_data_cback;
        mHalDeviceContext->open (mHalDeviceContext, HalDeviceContextCallback, HalDeviceContextDataCallback);
    }
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalClose
**
** Description: Turn off controller.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalClose (UINT8 close_reason)
{
    const char* func = "NfcAdaptation::HalClose";
    ALOGD ("%s", func);

    if(mHalDeviceContext == NULL)
        return;

    mHalDeviceContext->common.reserved[0] = close_reason;
    mHalDeviceContext->close (mHalDeviceContext);
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalDeviceContextCallback
**
** Description: Translate generic Android HAL's callback into Broadcom-specific
**              callback function.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalDeviceContextCallback (nfc_event_t event, nfc_status_t event_status)
{
    const char* func = "NfcAdaptation::HalDeviceContextCallback";
    ALOGD ("%s: event=%u", func, event);
    if (mHalCallback)
        mHalCallback (event, (tHAL_NFC_STATUS) event_status);
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalDeviceContextDataCallback
**
** Description: Translate generic Android HAL's callback into Broadcom-specific
**              callback function.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalDeviceContextDataCallback (uint16_t data_len, uint8_t* p_data)
{
    const char* func = "NfcAdaptation::HalDeviceContextDataCallback";
    ALOGD ("%s: len=%u", func, data_len);
    if (mHalDataCallback)
        mHalDataCallback (data_len, p_data);
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalWrite
**
** Description: Write NCI message to the controller.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalWrite (UINT16 data_len, UINT8* p_data)
{
    const char* func = "NfcAdaptation::HalWrite";
    ALOGD ("%s", func);
    if (mHalDeviceContext)
    {
        mHalDeviceContext->write (mHalDeviceContext, data_len, p_data);
    }
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalCoreInitialized
**
** Description: Adjust the configurable parameters in the controller.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalCoreInitialized (UINT8* p_core_init_rsp_params)
{
    const char* func = "NfcAdaptation::HalCoreInitialized";
    ALOGD ("%s", func);
    if (mHalDeviceContext)
    {
        mHalDeviceContext->core_initialized (mHalDeviceContext, p_core_init_rsp_params);
    }
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalPrediscover
**
** Description:     Perform any vendor-specific pre-discovery actions (if needed)
**                  If any actions were performed TRUE will be returned, and
**                  HAL_PRE_DISCOVER_CPLT_EVT will notify when actions are
**                  completed.
**
** Returns:          TRUE if vendor-specific pre-discovery actions initialized
**                  FALSE if no vendor-specific pre-discovery actions are needed.
**
*******************************************************************************/
BOOLEAN NfcAdaptation::HalPrediscover ()
{
    const char* func = "NfcAdaptation::HalPrediscover";
    ALOGD ("%s", func);
    BOOLEAN retval = FALSE;

    if (mHalDeviceContext)
    {
        retval = mHalDeviceContext->pre_discover (mHalDeviceContext);
    }
    return retval;
}

/*******************************************************************************
**
** Function:        HAL_NfcControlGranted
**
** Description:     Grant control to HAL control for sending NCI commands.
**                  Call in response to HAL_REQUEST_CONTROL_EVT.
**                  Must only be called when there are no NCI commands pending.
**                  HAL_RELEASE_CONTROL_EVT will notify when HAL no longer
**                  needs control of NCI.
**
** Returns:         void
**
*******************************************************************************/
void NfcAdaptation::HalControlGranted ()
{
    const char* func = "NfcAdaptation::HalControlGranted";
    ALOGD ("%s", func);
    if (mHalDeviceContext)
    {
        mHalDeviceContext->control_granted (mHalDeviceContext);
    }
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalPowerCycle
**
** Description: Turn off and turn on the controller.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalPowerCycle ()
{
    const char* func = "NfcAdaptation::HalPowerCycle";
    ALOGD ("%s", func);
    if (mHalDeviceContext)
    {
        mHalDeviceContext->power_cycle (mHalDeviceContext);
    }
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalGetMaxNfcee
**
** Description: Turn off and turn on the controller.
**
** Returns:     None.
**
*******************************************************************************/
UINT8 NfcAdaptation::HalGetMaxNfcee()
{
    const char* func = "NfcAdaptation::HalPowerCycle";
    UINT8 maxNfcee = 0;
    if (mHalDeviceContext)
    {
        // TODO maco call into HAL when we figure out binary compatibility.
        return nfa_ee_max_ee_cfg;
    }

    return maxNfcee;
}

/*******************************************************************************
**
** Function:    NfcAdaptation::DownloadFirmware
**
** Description: Download firmware patch files.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::DownloadFirmware ()
{
    const char* func = "NfcAdaptation::DownloadFirmware";
    ALOGD ("%s: enter", func);
    HalInitialize ();

    mHalOpenCompletedEvent.lock ();
    ALOGD ("%s: try open HAL", func);
    HalOpen (HalDownloadFirmwareCallback, HalDownloadFirmwareDataCallback, 0);
    mHalOpenCompletedEvent.wait ();

    mHalCloseCompletedEvent.lock ();
    ALOGD ("%s: try close HAL", func);
    HalClose (0);
    mHalCloseCompletedEvent.wait ();

    HalTerminate ();
    ALOGD ("%s: exit", func);
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalDownloadFirmwareCallback
**
** Description: Receive events from the HAL.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalDownloadFirmwareCallback (nfc_event_t event, nfc_status_t event_status)
{
    const char* func = "NfcAdaptation::HalDownloadFirmwareCallback";
    ALOGD ("%s: event=0x%X", func, event);
    switch (event)
    {
    case HAL_NFC_OPEN_CPLT_EVT:
        {
            ALOGD ("%s: HAL_NFC_OPEN_CPLT_EVT", func);
            mHalOpenCompletedEvent.signal ();
            break;
        }
    case HAL_NFC_CLOSE_CPLT_EVT:
        {
            ALOGD ("%s: HAL_NFC_CLOSE_CPLT_EVT", func);
            mHalCloseCompletedEvent.signal ();
            break;
        }
    }
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalDownloadFirmwareDataCallback
**
** Description: Receive data events from the HAL.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalDownloadFirmwareDataCallback (uint16_t data_len, uint8_t* p_data)
{
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalBertOpen
**
** Description: Open HAL for BERT testing
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalBertOpen()
{
    const char* func = "NfcAdaptation::HalBertOpen";
    ALOGD ("%s: enter", func);
    HalInitialize ();

    mHalOpenCompletedEvent.lock ();
    ALOGD ("%s: try open HAL", func);
    HalOpen (HalBertCallback, HalBertDataCallback, 0);
    mHalOpenCompletedEvent.wait ();

    ALOGD ("%s: exit", func);
    return;
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalBertClose
**
** Description: Close HAL after BERT testing
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalBertClose()
{
    const char* func = "NfcAdaptation::HalStopBert";
    ALOGD ("%s: enter", func);
    mHalCloseCompletedEvent.lock ();
    ALOGD ("%s: try close HAL", func);
    HalClose (0);
    mHalCloseCompletedEvent.wait ();
    HalTerminate ();
    ALOGD ("%s: exit", func);
}

/******************************************************************************
 **
 ** Function:        NfcAdaptation::SendBertCommands
 **
 ** Precondition:    NfcService off
 **
 ** Description:     This function sends carrier related commands to the NFCC.
 **
 **                  1.) Carrier On/Off
 **                  2.) BERT_RX
 **                  3.) BERT_TX
 **                  4.) BERT_LOOPBACK
 **                  5.) BERT_GET
 **
 ** Returns:         true on success,
 **                  false otherwise.
 **
 ******************************************************************************/
bool NfcAdaptation::SendBertCommands(tNFA_BERT_COMMAND& cmd)
{
    const UINT8 cmd_prop =  NCI_MTS_CMD|NCI_GID_PROP;
    const UINT8 oid_07 = 0x07; /* CARRIER oid */
    const UINT8 oid_08 = 0x08; /* BERT oid */

    const UINT8 cmd_params_run_test = 0x00; /* sub-oid for mode */

    const UINT8 cmd_params_len_rx = 0x0B; /* length for RX command */
    const UINT8 cmd_params_len_tx = 0x12; /* length for TX command */
    const UINT8 cmd_params_len_loopback = 0x13; /* length for loopback command */
    const UINT8 cmd_params_len_commands = 0x01; /* length for carrier on/off and get command */

    UINT8 cmd_params_carrier_on[] = {cmd_prop,oid_07, cmd_params_len_commands, 0x01};
    UINT8 cmd_params_carrier_off[] = {cmd_prop, oid_07, cmd_params_len_commands, 0x00};
    UINT8 cmd_params_get[] = {cmd_prop, oid_08, cmd_params_len_commands, 0x01};

    UINT8 cmd_params_rx[cmd_params_len_rx + NCI_MSG_HDR_SIZE];
    UINT8 cmd_params_tx[cmd_params_len_tx + NCI_MSG_HDR_SIZE];
    UINT8 cmd_params_loopback[cmd_params_len_loopback + NCI_MSG_HDR_SIZE];

    UINT8 *ptr = NULL; /* for byte access */

    UINT8 cmd_params_len = 0x00;
    UINT8 *p_cmd_params = NULL;

    bool error = false;

    switch(cmd.cmd)
    {
    case CARRIER_ON: /* carrier on command */
        /* 2F 07 01 01 */
        cmd_params_len = cmd_params_len_commands;
        p_cmd_params = cmd_params_carrier_on;
        break;
    case CARRIER_OFF: /* carrier off command */
        /* 2F 07 01 00 */
        cmd_params_len = cmd_params_len_commands;
        p_cmd_params = cmd_params_carrier_off;
        break;
    case BERT_RX: /* RX related BERT tests */
        /* e.g. 2F 08 0B 00 00 00 03 00 00 00 FF FF FF FF */
        cmd_params_len = cmd_params_len_rx;
        p_cmd_params = cmd_params_rx;
        ptr = p_cmd_params;        /* byte access to p_cmd_params */
        *ptr++ = cmd_prop;
        *ptr++ = oid_08;
        *ptr++ = cmd_params_len_rx;
        *ptr++ = cmd_params_run_test;
        *ptr++ = cmd.mode;         /* rx/tx/loopback */
        *ptr++ = cmd.pollListen;   /* poll/listen */
        *ptr++ = cmd.pattern;      /* Which pattern to send? */
        *ptr++ = cmd.technology;   /* Which technology? */
        *ptr++ = cmd.bitRx;        /* RX bitrate */
        *ptr++ = cmd.bitTx;        /* TX bitrate */
        *ptr++ = cmd.sofThreshold; /* start of frame threshold */
        /* bit count */
        *ptr++ = (UINT8)((cmd.bitCount & 0xFF00) >> 8); /* MSB - byte 1 */
        *ptr++ = (UINT8)(cmd.bitCount & 0x00FF);        /* LSB - byte 2 */
        /* RX error threshold */
        *ptr++ = cmd.rx.errorThreshold; /* error threshold */
        break;
    case BERT_TX:
        /* e.g. 2F 08 0E 00 01 00 03 00 00 00 01 FF FF 04 03 02 01 */
        cmd_params_len = cmd_params_len_tx;
        p_cmd_params = cmd_params_tx;
        ptr = p_cmd_params; /* byte access to p_cmd_params */
        *ptr++ = cmd_prop;
        *ptr++ = oid_08;
        *ptr++ = cmd_params_len_tx;
        *ptr++ = cmd_params_run_test;
        *ptr++ = cmd.mode;         /* rx/tx/loopback */
        *ptr++ = cmd.pollListen;   /* poll/listen */
        *ptr++ = cmd.pattern;      /* Which pattern to send? */
        *ptr++ = cmd.technology;   /* Which technology? */
        *ptr++ = cmd.bitRx;        /* RX bitrate */
        *ptr++ = cmd.bitTx;        /* TX bitrate */
        *ptr++ = cmd.sofThreshold; /* start of frame threshold */
        /* bit count */
        *ptr++ = (UINT8)((cmd.bitCount & 0xFF00) >> 8); /* MSB - byte 1 */
        *ptr++ = (UINT8)(cmd.bitCount & 0x00FF);        /* LSB - byte 2 */
        /* TX time between packets in ms */
        *ptr++ = (UINT8)((cmd.tx.interPacketTime & 0xFF000000) >> 24); /* MSB - byte 1 */
        *ptr++ = (UINT8)((cmd.tx.interPacketTime & 0x00FF0000) >> 16); /*       byte 2 */
        *ptr++ = (UINT8)((cmd.tx.interPacketTime & 0x0000FF00) >> 8);  /*       byte 3 */
        *ptr++ = (UINT8)(cmd.tx.interPacketTime & 0x000000FF);         /* LSB - byte 4 */
        /* TX total duration */
        *ptr++ = (UINT8)((cmd.tx.nTotalDuration & 0xFF000000) >> 24); /* MSB - byte 1 */
        *ptr++ = (UINT8)((cmd.tx.nTotalDuration & 0x00FF0000) >> 16); /*       byte 2 */
        *ptr++ = (UINT8)((cmd.tx.nTotalDuration & 0x0000FF00) >> 8);  /*       byte 3 */
        *ptr++ = (UINT8)(cmd.tx.nTotalDuration & 0x000000FF);         /* LSB - byte 4 */
        break;
    case BERT_LOOPBACK:
        /* e.g. 2F 08 0F 00 01 00 03 00 00 00 01 FF FF 04 03 02 01 FF */
        cmd_params_len = cmd_params_len_loopback;
        p_cmd_params = cmd_params_loopback;
        ptr = p_cmd_params;
        *ptr++ = cmd_prop;
        *ptr++ = oid_08;
        *ptr++ = cmd_params_len_loopback;
        *ptr++ = cmd_params_run_test;
        *ptr++ = cmd.mode;         /* rx/tx/loopback */
        *ptr++ = cmd.pollListen;   /* poll/listen */
        *ptr++ = cmd.pattern;      /* Which pattern to send? */
        *ptr++ = cmd.technology;   /* Which technology? */
        *ptr++ = cmd.bitRx;        /* RX bitrate */
        *ptr++ = cmd.bitTx;        /* TX bitrate */
        *ptr++ = cmd.sofThreshold; /* start of frame threshold */
        /* bit count */
        *ptr++ = (UINT8)((cmd.bitCount & 0xFF00) >> 8); /* MSB - byte 1 */
        *ptr++ = (UINT8)(cmd.bitCount & 0x00FF);        /* LSB - byte 2 */
        /* TX time between packets in ms */
        *ptr++ = (UINT8)((cmd.loopback.tx.interPacketTime & 0xFF000000) >> 24); /* MSB - byte 1 */
        *ptr++ = (UINT8)((cmd.loopback.tx.interPacketTime & 0x00FF0000) >> 16); /*       byte 2 */
        *ptr++ = (UINT8)((cmd.loopback.tx.interPacketTime & 0x0000FF00) >> 8);  /*       byte 3 */
        *ptr++ = (UINT8)(cmd.loopback.tx.interPacketTime & 0x000000FF);         /* LSB - byte 4 */
        /* TX total duration */
        *ptr++ = (UINT8)((cmd.tx.nTotalDuration & 0xFF000000) >> 24); /* MSB - byte 1 */
        *ptr++ = (UINT8)((cmd.tx.nTotalDuration & 0x00FF0000) >> 16); /*       byte 2 */
        *ptr++ = (UINT8)((cmd.tx.nTotalDuration & 0x0000FF00) >> 8);  /*       byte 3 */
        *ptr++ = (UINT8)(cmd.tx.nTotalDuration & 0x000000FF);         /* LSB - byte 4 */
        /* RX error threshold */
        *ptr++ = cmd.loopback.rx.errorThreshold;
        error = true; /* not yet supported in HW */
        ALOGE("%s: unsupported BERT command", __FUNCTION__);
        break;
    case BERT_GET: /* get stats */
        /* 2F 08 01 01 */
        cmd_params_len = cmd_params_len_commands;
        p_cmd_params = cmd_params_get;
        break;
    default:
        error = true;
        ALOGE("%s: unsupported BERT command", __FUNCTION__);
        break;
    }

    if(!error)
    {
        mHalBertEvent.lock();
        HalWrite(cmd_params_len + NCI_MSG_HDR_SIZE,  p_cmd_params);
        mHalBertEvent.wait();
    }


    if(error)
    {
        ALOGE("%s: BERT failed", __FUNCTION__);
        return false;
    }

    return true;
}


/*******************************************************************************
**
** Function:    NfcAdaptation::HalBertCallback
**
** Description: Receive events from the HAL.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalBertCallback (nfc_event_t event, nfc_status_t event_status)
{
    const char* func = "NfcAdaptation::HalBertCallback";
    ALOGD ("%s: event=0x%X", func, event);
    switch (event)
    {
    case HAL_NFC_OPEN_CPLT_EVT:
        {
            ALOGD ("%s: HAL_NFC_OPEN_CPLT_EVT", func);
            mHalOpenCompletedEvent.signal ();
            break;
        }
    case HAL_NFC_CLOSE_CPLT_EVT:
        {
            ALOGD ("%s: HAL_NFC_CLOSE_CPLT_EVT", func);
            mHalCloseCompletedEvent.signal ();
            break;
        }
    }
}

/*******************************************************************************
**
** Function:    NfcAdaptation::HalBertDataCallback
**
** Description: Receive data events from the HAL.
**
** Returns:     None.
**
*******************************************************************************/
void NfcAdaptation::HalBertDataCallback (uint16_t data_len, uint8_t* p_data)
{
    const char* func = "NfcAdaptation::HalBertDataCallback";

    const UINT8 oid_mask = 0x3F; /* to mask out OID */
    const UINT8 rsp_mask = 0xC0; /* to mask out NCI_RSP_BIT */

    const UINT8 oid_07 = 0x07;   /* CARRIER oid */
    const UINT8 oid_08 = 0x08;   /* BERT oid */

    const UINT16 start_length = 0x04;   /* response for RX/TX/Loopback */
    const UINT16 carrier_length = 0x04; /* response for carrier on/off */
    const UINT16 get_length = 0x06;     /* response for get command */

    const int status_byte_index = 3;   /* status byte is at position 3 */

    if(data_len < 2)
        return;

    UINT8 oid = (p_data[1] & oid_mask); /* mask out oid */
    UINT8 event = (p_data[0] & rsp_mask) | (p_data[1] & oid_mask);

    if((p_data[0] & rsp_mask) == NCI_RSP_BIT) /* mask out NCI_RSP_BIT */
    {
        if(oid == oid_08) /* BERT */
        {
            if ((data_len >= get_length) &&
                    (*(p_data + status_byte_index) == NFA_STATUS_OK))
            {
                ALOGD("%s: get event = 0x%02x ok", __FUNCTION__, event);
            } else if ((data_len >= get_length) &&
                    (*(p_data + status_byte_index) != NFA_STATUS_OK)) {
                ALOGE("%s: get event = 0x%02x failed", __FUNCTION__, event);
            } else if ((data_len >= start_length) &&
                    (*(p_data + status_byte_index) == NFA_STATUS_OK))
            {
                ALOGD("%s: start event = 0x%02x ok", __FUNCTION__, event);
            } else if ((data_len >= start_length) &&
                    (*(p_data + status_byte_index) != NFA_STATUS_OK)) {
                ALOGE("%s: start event = 0x%02x failed", __FUNCTION__, event);
            } else {
                ALOGE("%s: unknown event = 0x%02x failed",
                        __FUNCTION__, event);
            }
        }
        else if(oid == oid_07) /* CARRIER */
        {
            if ((data_len >= carrier_length) &&
                    (*(p_data + status_byte_index) == NFA_STATUS_OK))
            {
                ALOGD("%s: carrier event = 0x%02x ok", __FUNCTION__, event);
            } else if ((data_len >= carrier_length) &&
                    (*(p_data + status_byte_index) != NFA_STATUS_OK)) {
                ALOGE("%s: carrier event = 0x%02x failed", __FUNCTION__, event);
            } else {
                ALOGE("%s: unknown event = 0x%02x failed",
                        __FUNCTION__, event);
            }
        }
    }
    mHalBertEvent.signal();
}

/*******************************************************************************
**
** Function:    ThreadMutex::ThreadMutex()
**
** Description: class constructor
**
** Returns:     none
**
*******************************************************************************/
ThreadMutex::ThreadMutex()
{
    pthread_mutexattr_t mutexAttr;

    pthread_mutexattr_init(&mutexAttr);
    pthread_mutex_init(&mMutex, &mutexAttr);
    pthread_mutexattr_destroy(&mutexAttr);
}

/*******************************************************************************
**
** Function:    ThreadMutex::~ThreadMutex()
**
** Description: class destructor
**
** Returns:     none
**
*******************************************************************************/
ThreadMutex::~ThreadMutex()
{
    pthread_mutex_destroy(&mMutex);
}

/*******************************************************************************
**
** Function:    ThreadMutex::lock()
**
** Description: lock kthe mutex
**
** Returns:     none
**
*******************************************************************************/
void ThreadMutex::lock()
{
    pthread_mutex_lock(&mMutex);
}

/*******************************************************************************
**
** Function:    ThreadMutex::unblock()
**
** Description: unlock the mutex
**
** Returns:     none
**
*******************************************************************************/
void ThreadMutex::unlock()
{
    pthread_mutex_unlock(&mMutex);
}

/*******************************************************************************
**
** Function:    ThreadCondVar::ThreadCondVar()
**
** Description: class constructor
**
** Returns:     none
**
*******************************************************************************/
ThreadCondVar::ThreadCondVar()
{
    pthread_condattr_t CondAttr;

    pthread_condattr_init(&CondAttr);
    pthread_cond_init(&mCondVar, &CondAttr);

    pthread_condattr_destroy(&CondAttr);
}

/*******************************************************************************
**
** Function:    ThreadCondVar::~ThreadCondVar()
**
** Description: class destructor
**
** Returns:     none
**
*******************************************************************************/
ThreadCondVar::~ThreadCondVar()
{
    pthread_cond_destroy(&mCondVar);
}

/*******************************************************************************
**
** Function:    ThreadCondVar::wait()
**
** Description: wait on the mCondVar
**
** Returns:     none
**
*******************************************************************************/
void ThreadCondVar::wait()
{
    pthread_cond_wait(&mCondVar, *this);
    pthread_mutex_unlock(*this);
}

/*******************************************************************************
**
** Function:    ThreadCondVar::signal()
**
** Description: signal the mCondVar
**
** Returns:     none
**
*******************************************************************************/
void ThreadCondVar::signal()
{
    AutoThreadMutex  a(*this);
    pthread_cond_signal(&mCondVar);
}

/*******************************************************************************
**
** Function:    AutoThreadMutex::AutoThreadMutex()
**
** Description: class constructor, automatically lock the mutex
**
** Returns:     none
**
*******************************************************************************/
AutoThreadMutex::AutoThreadMutex(ThreadMutex &m)
    : mm(m)
{
    mm.lock();
}

/*******************************************************************************
**
** Function:    AutoThreadMutex::~AutoThreadMutex()
**
** Description: class destructor, automatically unlock the mutex
**
** Returns:     none
**
*******************************************************************************/
AutoThreadMutex::~AutoThreadMutex()
{
    mm.unlock();
}
