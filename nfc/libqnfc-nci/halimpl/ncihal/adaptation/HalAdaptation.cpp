/******************************************************************************
* Copyright (c) 2013, The Linux Foundation. All rights reserved.
* Not a Contribution.
 ******************************************************************************/

/******************************************************************************
 *
 *  Copyright (C) 2012 Broadcom Corporation
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

/******************************************************************************
 *
 *  HAL Adaptation Interface (HAI). This interface regulates the interaction
 *  between standard Android HAL and Broadcom-specific HAL.  It adapts
 *  Broadcom-specific features to the Android framework.
 *
 ******************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NfcHal"
#include "OverrideLog.h"
#include "HalAdaptation.h"
#include "SyncEvent.h"
#include "config.h"
#include "nfc_hal_int.h"
#include "nfc_hal_post_reset.h"
#include <errno.h>
#include <pthread.h>
#include "buildcfg.h"
extern void delete_hal_non_volatile_store (bool forceDelete);
extern void verify_hal_non_volatile_store ();
extern "C"
{
#include "userial.h"
}


///////////////////////////////////////
// private declaration, definition


static nfc_stack_callback_t* gAndroidHalCallback = NULL;
static nfc_stack_data_callback_t* gAndroidHalDataCallback = NULL;
static SyncEvent gOpenCompletedEvent;
static SyncEvent gPostInitCompletedEvent;
static SyncEvent gCloseCompletedEvent;

UINT32 ScrProtocolTraceFlag = SCR_PROTO_TRACE_ALL; //0x017F00;

static void NfcHalCallback (UINT8 event, tHAL_NFC_STATUS status);
static void NfcHalDataCallback (UINT16 data_len, UINT8* p_data);

extern tNFC_HAL_CFG *p_nfc_hal_cfg;
extern const UINT8  nfca_version_string [];
extern const UINT8  nfa_version_string [];

///////////////////////////////////////


int HaiInitializeLibrary (const nfc_dev_t* device)
{
    ALOGD ("%s: enter", __FUNCTION__);
    ALOGE ("%s: ver=%s nfa=%s", __FUNCTION__, nfca_version_string, nfa_version_string);
    int retval = EACCES;
    unsigned long freq = 0;
    unsigned long num = 0;
    char temp[120];
    UINT8 logLevel = 0;

    logLevel = InitializeGlobalAppLogLevel ();
    tUSERIAL_OPEN_CFG cfg;
    // Initialize protocol logging level
    if ( GetNumValue ( NAME_PROTOCOL_TRACE_LEVEL, &num, sizeof ( num ) ) )
        ScrProtocolTraceFlag = num;

    USERIAL_Init(&cfg);

    if ( GetNumValue ( NAME_NFCC_ENABLE_TIMEOUT, &num, sizeof ( num ) ) )
    {
        p_nfc_hal_cfg->nfc_hal_nfcc_enable_timeout = num;
    }

    if ( GetNumValue ( NAME_NFA_MAX_EE_SUPPORTED, &num, sizeof ( num ) ) && num == 0 )
    {
        // Since NFA_MAX_EE_SUPPORTED is explicetly set to 0, no UICC support is needed.
        p_nfc_hal_cfg->nfc_hal_hci_uicc_support = 0;
    }

    // Set 'first boot' flag based on static variable that will get set to false
    // after the stack has first initialized the EE.
  //  p_nfc_hal_cfg->nfc_hal_first_boot = isColdBoot ? TRUE : FALSE;
    // Initialize appliation logging level
    if ( GetNumValue ( NAME_APPL_TRACE_LEVEL, &num, sizeof ( num ) ) ) {
        HAL_NfcSetTraceLevel(num);
    }
    HAL_NfcInitialize ();
    HAL_NfcSetTraceLevel (logLevel); // Initialize HAL's logging level

    retval = 0;
    ALOGD ("%s: exit %d", __FUNCTION__, retval);
    return retval;
}


int HaiTerminateLibrary ()
{
    int retval = EACCES;
    ALOGD ("%s: enter", __FUNCTION__);

    HAL_NfcTerminate ();
    GKI_shutdown();  // For release HAL/DT/HAL's timer thread
    gAndroidHalCallback = NULL;
    gAndroidHalDataCallback = NULL;
    retval = 0;
    ALOGD ("%s: exit %d", __FUNCTION__, retval);
    return retval;
}


int HaiOpen (const nfc_dev_t* device, nfc_stack_callback_t* halCallbackFunc, nfc_stack_data_callback_t* halDataCallbackFunc, char mode, char reset_status)
{
    ALOGD ("%s: enter", __FUNCTION__);
    int retval = EACCES;

    gAndroidHalCallback = halCallbackFunc;
    gAndroidHalDataCallback = halDataCallbackFunc;

    SyncEventGuard guard (gOpenCompletedEvent);
    HAL_NfcOpen (NfcHalCallback, NfcHalDataCallback, mode, reset_status);
    gOpenCompletedEvent.wait ();

    retval = 0;
    ALOGD ("%s: exit %d", __FUNCTION__, retval);
    return retval;
}


void NfcHalCallback (UINT8 event, tHAL_NFC_STATUS status)
{
    ALOGD ("%s: enter; event=0x%X", __FUNCTION__, event);
    switch (event)
    {
    case HAL_NFC_OPEN_CPLT_EVT:
        {
            ALOGD ("%s: HAL_NFC_OPEN_CPLT_EVT; status=0x%X", __FUNCTION__, status);
            SyncEventGuard guard (gOpenCompletedEvent);
            gOpenCompletedEvent.notifyOne ();
            break;
        }

    case HAL_NFC_POST_INIT_CPLT_EVT:
        {
            ALOGD ("%s: HAL_NFC_POST_INIT_CPLT_EVT", __FUNCTION__);
            SyncEventGuard guard (gPostInitCompletedEvent);
            ALOGD ("%s: HAL_SYNC: Signalling waiting client...", __FUNCTION__);
            gPostInitCompletedEvent.notifyOne ();
            break;
        }

    case HAL_NFC_CLOSE_CPLT_EVT:
        {
            ALOGD ("%s: HAL_NFC_CLOSE_CPLT_EVT", __FUNCTION__);
            SyncEventGuard guard (gCloseCompletedEvent);
            gCloseCompletedEvent.notifyOne ();
            break;
        }

    case HAL_NFC_ERROR_EVT:
        {
            ALOGD ("%s: HAL_NFC_ERROR_EVT", __FUNCTION__);
            {
                SyncEventGuard guard (gOpenCompletedEvent);
                gOpenCompletedEvent.notifyOne ();
            }
            {
                SyncEventGuard guard (gPostInitCompletedEvent);
                ALOGD ("%s: HAL_SYNC: Signalling waiting client...", __FUNCTION__);
                gPostInitCompletedEvent.notifyOne ();
            }
            {
                SyncEventGuard guard (gCloseCompletedEvent);
                gCloseCompletedEvent.notifyOne ();
            }
            break;
        }
    }
    gAndroidHalCallback (event, status);
    ALOGD ("%s: exit; event=0x%X", __FUNCTION__, event);
}


void NfcHalDataCallback (UINT16 data_len, UINT8* p_data)
{
    ALOGD ("%s: enter; len=%u", __FUNCTION__, data_len);
    gAndroidHalDataCallback (data_len, p_data);
}


int HaiClose (const nfc_dev_t* device, UINT8 shutdown_reason)
{
    ALOGD ("%s: enter : shutdown_reason=%d", __FUNCTION__,shutdown_reason);
    int retval = EACCES;

    SyncEventGuard guard (gCloseCompletedEvent);
    HAL_NfcClose (shutdown_reason);
    gCloseCompletedEvent.wait ();
    retval = 0;
    ALOGD ("%s: exit %d", __FUNCTION__, retval);
    return retval;
}


int HaiCoreInitialized (const nfc_dev_t* device, uint8_t* coreInitResponseParams)
{
    ALOGD ("%s: enter", __FUNCTION__);
    int retval = EACCES;

    SyncEventGuard guard (gPostInitCompletedEvent);
    ALOGD ("%s: HAL_SYNC: Client indicating HAL Core Initialised and now Waiting for dm config...", __FUNCTION__);
    HAL_NfcCoreInitialized (coreInitResponseParams);
    gPostInitCompletedEvent.wait ();
    ALOGD ("%s: HAL_SYNC: Client signalled, continuing...", __FUNCTION__);
    retval = 0;
    ALOGD ("%s: exit %d", __FUNCTION__, retval);
    return retval;
}


int HaiWrite (const nfc_dev_t* dev, uint16_t dataLen, const uint8_t* data)
{
    ALOGD ("%s: enter; len=%u", __FUNCTION__, dataLen);
    int retval = EACCES;

    HAL_NfcWrite (dataLen, const_cast<UINT8*> (data));
    retval = 0;
    ALOGD ("%s: exit %d", __FUNCTION__, retval);
    return retval;
}


int HaiPreDiscover (const nfc_dev_t* device)
{
    ALOGD ("%s: enter", __FUNCTION__);
    int retval = EACCES;

    retval = HAL_NfcPreDiscover () ? 1 : 0;
    ALOGD ("%s: exit %d", __FUNCTION__, retval);
    return retval;
}


int HaiControlGranted (const nfc_dev_t* device)
{
    ALOGD ("%s: enter", __FUNCTION__);
    int retval = EACCES;

    HAL_NfcControlGranted ();
    retval = 0;
    ALOGD ("%s: exit %d", __FUNCTION__, retval);
    return retval;
}


int HaiPowerCycle (const nfc_dev_t* device)
{
    ALOGD ("%s: enter", __FUNCTION__);
    int retval = EACCES;

    HAL_NfcPowerCycle ();
    retval = 0;
    ALOGD ("%s: exit %d", __FUNCTION__, retval);
    return retval;
}


int HaiGetMaxNfcee (const nfc_dev_t* device, uint8_t* maxNfcee)
{
    ALOGD ("%s: enter", __FUNCTION__);
    int retval = EACCES;

    // This function is a clear indication that the stack is initializing
    // EE.  So we can reset the cold-boot flag here.
   // isColdBoot = false;

    if ( maxNfcee )
    {
        unsigned long num;

        // At this point we can see if there is a chip-specific value for max ee.
        if ( GetNumValue ( NAME_NFA_MAX_EE_SUPPORTED, &num, sizeof ( num ) ) )
        {
            *maxNfcee = num;
        }
        else
            *maxNfcee = HAL_NfcGetMaxNfcee ();

        ALOGD("%s: max_ee from HAL to use %d", __FUNCTION__, *maxNfcee);
        retval = 0;
    }
    ALOGD ("%s: exit %d", __FUNCTION__, retval);
    return retval;
}
