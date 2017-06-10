/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/******************************************************************************
* Copyright (c) 2013, The Linux Foundation. All rights reserved.
* Not a Contribution.
 ******************************************************************************/
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
#include "config.h"
#include "nfc_hal_int.h"
#include "userial.h"
extern "C"
{
    #include <DT_Nfc_link.h>
    #include <DT_Nfc.h>
    #include "nfc_hal_post_reset.h"
    #ifdef DTA // <DTA>
    #include "dta_flag.h"
    #endif // </DTA>
}
#include <string>
#include <cutils/properties.h>
#include "StartupConfig.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NfcHal"
#ifndef NFCA_PATCHFILE_LOCATION
#define NFCA_PATCHFILE_LOCATION ("/system/vendor/firmware/")
#endif

#define FW_PRE_PATCH                        "FW_PRE_PATCH"
#define FW_PATCH                            "FW_PATCH"
#define MAX_RF_DATA_CREDITS                 "MAX_RF_DATA_CREDITS"

#define TOTAL_LENGTH_OCTETS                  4
#define PATCH_LENGTH_OCTETS                  4
#define FW_VERSION_OCTETS                    2
#define PATCH_OCTETS                         2
#define SIG_ALGORITHM_OCTETS                 1
#define RESERVED_OCTETS                      4
#define PUBLIC_KEY_LENGTH_OCTETS             2
#define SIGNATURE_LENGTH_OCTETS              2
#define PRE_PATCH_EXISTS_OCTETS              2
#define SIGNATURE_LENGTH                     72
#define FW_VERSION_OFFSET                    2
#define PATCH_NOT_UPDATED                    3
#define PATCH_UPDATED                        4
#define MAX_BUFFER      (512)
static char sPrePatchFn[MAX_BUFFER+1];
static char sPatchFn[MAX_BUFFER+1];
static void * sPrmBuf = NULL;
static void * sI2cFixPrmBuf = NULL;

#define CONFIG_MAX_LEN 256
static UINT8 sConfig [CONFIG_MAX_LEN];
static StartupConfig sStartupConfig;
static StartupConfig sLptdConfig;
static StartupConfig sPreDiscoveryConfig;
static UINT8 sDontSendLptd[] = { 0 };
extern UINT8 *p_nfc_hal_dm_start_up_cfg; //defined in the HAL
static UINT8 nfa_dm_start_up_vsc_cfg[CONFIG_MAX_LEN];
extern UINT8 *p_nfc_hal_dm_start_up_vsc_cfg; //defined in the HAL
extern UINT8 *p_nfc_hal_dm_lptd_cfg; //defined in the HAL
extern UINT8 *p_nfc_hal_pre_discover_cfg; //defined in the HAL

extern tSNOOZE_MODE_CONFIG gSnoozeModeCfg;
extern tNFC_HAL_CFG *p_nfc_hal_cfg;
static void mayDisableSecureElement (StartupConfig& config);

/* Default patchfile (in NCD format) */
#ifndef NFA_APP_DEFAULT_PATCHFILE_NAME
#define NFA_APP_DEFAULT_PATCHFILE_NAME      "\0"
#endif

/* Default patchfile (in NCD format) */
#ifndef NFA_APP_DEFAULT_I2C_PATCHFILE_NAME
#define NFA_APP_DEFAULT_I2C_PATCHFILE_NAME  "\0"
#endif

UINT32 patch_version = 0;
#define NFCC_VERSION_V20          20
#define NFCC_VERSION_V21          21
#define NFCC_VERSION_V30          30
#define NFCC_VERSION_V24          24
#define NFCC_VERSION_1990A        01

tNFC_POST_RESET_CB nfc_post_reset_cb =
{
    /* Default Patch & Pre-Patch */
    NFA_APP_DEFAULT_PATCHFILE_NAME,
    NULL,
    NFA_APP_DEFAULT_I2C_PATCHFILE_NAME,
    NULL,

    /* Default UART baud rate */
    NFC_HAL_DEFAULT_BAUD,

    {0, 0},

    /* Default low power mode settings */
    NFC_HAL_LP_SNOOZE_MODE_NONE,    /* Snooze Mode          */
    NFC_HAL_LP_IDLE_THRESHOLD_HOST, /* Idle Threshold Host  */
    NFC_HAL_LP_IDLE_THRESHOLD_HC,   /* Idle Threshold HC    */
    NFC_HAL_LP_ACTIVE_LOW,          /* NFC_WAKE Active Mode */
    NFC_HAL_LP_ACTIVE_HIGH,         /* DH_WAKE Active Mode  */

    NFA_APP_MAX_NUM_REINIT,         /* max retry to get NVM type */
    0,                              /* current retry count */
    TRUE,                           /* debug mode for downloading patchram */
    FALSE                           /* skip downloading patchram after reinit because of patch download failure */
};
/*******************************************************************************
**
** Function         getFileLength
**
** Description      return the size of a file
**
** Returns          file size in number of bytes
**
*******************************************************************************/
static long getFileLength(FILE* fp)
{
    long sz;
    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    return (sz > 0) ? sz : 0;
}

/*******************************************************************************
**
** Function         isFileExist
**
** Description      Check if file name exists (android does not support fexists)
**
** Returns          TRUE if file exists
**
*******************************************************************************/
static BOOLEAN isFileExist(const char *pFilename)
{
    FILE *pf;
    pf = fopen(pFilename, "r");
    if (pf != NULL)
    {
        fclose(pf);
        return TRUE;
    }
    return FALSE;
}

/*******************************************************************************
**
** Function         findPatchramFile
**
** Description      Find the patchram file name specified in the .conf
**
** Returns          pointer to the file name
**
*******************************************************************************/
static const char* findPatchramFile(const char * pConfigName, char * pBuffer, int bufferLen)
{
    ALOGD("%s: config=%s", __FUNCTION__, pConfigName);

    if (pConfigName == NULL)
    {
        ALOGD("%s No patchfile defined\n", __FUNCTION__);
        return NULL;
    }

    if (GetStrValue(pConfigName, &pBuffer[0], bufferLen))
    {
        ALOGD("%s found patchfile %s\n", __FUNCTION__, pBuffer);
        return (pBuffer[0] == '\0') ? NULL : pBuffer;
    }

    ALOGD("%s Cannot find patchfile '%s'\n", __FUNCTION__, pConfigName);
    return NULL;
}

/*******************************************************************************
**
** Function:    continueAfterSetSnoozeMode
**
** Description: Called after Snooze Mode is enabled.
**
** Returns:     none
**
*******************************************************************************/
static void continueAfterSetSnoozeMode(tHAL_NFC_STATUS status)
{
    ALOGD("%s: status=%u", __FUNCTION__, status);
    //let stack download firmware during next initialization
    nfc_post_reset_cb.spd_skip_on_power_cycle = FALSE;
    if (status == NCI_STATUS_OK)
        HAL_NfcPreInitDone (HAL_NFC_STATUS_OK);
    else
        HAL_NfcPreInitDone (HAL_NFC_STATUS_FAILED);
}

/*******************************************************************************
**
** Function:    postDownloadPatchram
**
** Description: Called after patch download
**
** Returns:     none
**
*******************************************************************************/
static void postDownloadPatchram(tHAL_NFC_STATUS status)
{
    ALOGD("%s: status=%i", __FUNCTION__, status);

    if (status != HAL_NFC_STATUS_OK)
    {
        ALOGE("Patch download failed");
    }
    /* Set snooze mode here */
    else if (gSnoozeModeCfg.snooze_mode != NFC_HAL_LP_SNOOZE_MODE_NONE)
    {
        status = HAL_NfcSetSnoozeMode(gSnoozeModeCfg.snooze_mode,
                                       gSnoozeModeCfg.idle_threshold_dh,
                                       gSnoozeModeCfg.idle_threshold_nfcc,
                                       gSnoozeModeCfg.nfc_wake_active_mode,
                                       gSnoozeModeCfg.dh_wake_active_mode,
                                       continueAfterSetSnoozeMode);
        if (status != NCI_STATUS_OK)
        {
            ALOGE("%s: Setting snooze mode failed, status=%i", __FUNCTION__, status);
            HAL_NfcPreInitDone(HAL_NFC_STATUS_FAILED);
        }
    }
    else
    {
        ALOGD("%s: Not using Snooze Mode", __FUNCTION__);
        HAL_NfcPreInitDone(HAL_NFC_STATUS_OK);
    }
}


/*******************************************************************************
**
** Function:    prmCallback
**
** Description: Patchram callback (for static patchram mode)
**
** Returns:     none
**
*******************************************************************************/
void prmCallback(UINT8 event)
{
    ALOGD("%s: event=0x%x", __FUNCTION__, event);
    switch (event)
    {
    case NFC_HAL_PRM_CONTINUE_EVT:
        /* This event does not occur if static patchram buf is used */
        break;

    case NFC_HAL_PRM_COMPLETE_EVT:
        postDownloadPatchram(HAL_NFC_STATUS_OK);
        break;

    case NFC_HAL_PRM_ABORT_EVT:
        postDownloadPatchram(HAL_NFC_STATUS_FAILED);
        break;

    case NFC_HAL_PRM_ABORT_INVALID_PATCH_EVT:
        ALOGD("%s: invalid patch...skipping patch download", __FUNCTION__);
        postDownloadPatchram(HAL_NFC_STATUS_REFUSED);
        break;

    case NFC_HAL_PRM_ABORT_BAD_SIGNATURE_EVT:
        ALOGD("%s: patch authentication failed", __FUNCTION__);
        postDownloadPatchram(HAL_NFC_STATUS_REFUSED);
        break;

    case NFC_HAL_PRM_ABORT_NO_NVM_EVT:
        ALOGD("%s: No NVM detected", __FUNCTION__);
        HAL_NfcPreInitDone(HAL_NFC_STATUS_FAILED);
        break;

    default:
        ALOGD("%s: not handled event=0x%x", __FUNCTION__, event);
        break;
    }
}


/*******************************************************************************
**
** Function         getNfaValues
**
** Description      Get configuration values needed by NFA layer
**
** Returns:         None
**
*******************************************************************************/
static void getNfaValues()
{
    unsigned long num = 0;
    int actualLen = 0;

    p_nfc_hal_cfg->nfc_hal_prm_nvm_required = TRUE; //don't download firmware if controller cannot detect EERPOM
    sStartupConfig.initialize ();
    sLptdConfig.initialize ();
    sPreDiscoveryConfig.initialize();

#ifdef DTA // <DTA>
    if(!nfc_hal_in_dta_mode())
    {
#endif // </DTA>
        actualLen = GetStrValue (NAME_NFA_DM_START_UP_CFG, (char*)sConfig, sizeof(sConfig));
#ifdef DTA // <DTA>
    }
    else
    {
        actualLen = GetStrValue (NAME_NFA_DM_START_UP_CFG_DTA, (char*)sConfig, sizeof(sConfig));
    }
#endif // </DTA>
    if (actualLen)
        sStartupConfig.append (sConfig, actualLen);

    // Set antenna tuning configuration if configured.
    actualLen = GetStrValue(NAME_PREINIT_DSP_CFG, (char*)sConfig, sizeof(sConfig));
    if (actualLen)
        sStartupConfig.append (sConfig, actualLen);

    if ( GetStrValue ( NAME_NFA_DM_START_UP_VSC_CFG, (char*)nfa_dm_start_up_vsc_cfg, sizeof (nfa_dm_start_up_vsc_cfg) ) )
    {
        p_nfc_hal_dm_start_up_vsc_cfg = &nfa_dm_start_up_vsc_cfg[0];
        ALOGD ( "START_UP_VSC_CFG[0] = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
                                                                            nfa_dm_start_up_vsc_cfg[0],
                                                                            nfa_dm_start_up_vsc_cfg[1],
                                                                            nfa_dm_start_up_vsc_cfg[2],
                                                                            nfa_dm_start_up_vsc_cfg[3],
                                                                            nfa_dm_start_up_vsc_cfg[4],
                                                                            nfa_dm_start_up_vsc_cfg[5],
                                                                            nfa_dm_start_up_vsc_cfg[6],
                                                                            nfa_dm_start_up_vsc_cfg[7] );
    }

    actualLen = GetStrValue(NAME_LPTD_CFG, (char*)sConfig, sizeof(sConfig));
    if (actualLen)
    {
        sLptdConfig.append (sConfig, actualLen);
        p_nfc_hal_dm_lptd_cfg = const_cast<UINT8*> (sLptdConfig.getInternalBuffer ());
    }
    else
    {
        // Default to not sending any LPTD setting.
        p_nfc_hal_dm_lptd_cfg = sDontSendLptd;
    }

    mayDisableSecureElement (sStartupConfig);
    p_nfc_hal_dm_start_up_cfg = const_cast<UINT8*> (sStartupConfig.getInternalBuffer ());

    actualLen = GetStrValue(NAME_NFA_DM_PRE_DISCOVERY_CFG, (char*)sConfig, sizeof(sConfig));
    if (actualLen)
    {
        sPreDiscoveryConfig.append (sConfig, actualLen);
        mayDisableSecureElement (sPreDiscoveryConfig);
        p_nfc_hal_pre_discover_cfg = const_cast<UINT8*> (sPreDiscoveryConfig.getInternalBuffer ());
    }
}

/*******************************************************************************
**
** Function         StartPatchDownload
**
** Description      Reads configuration settings, and begins the download
**                  process if patch files are configured.
**
** Returns:         None
**
*******************************************************************************/
static void StartPatchDownload(UINT32 chipid)
{
    ALOGD ("%s: chipid=%lx",__FUNCTION__, chipid);

    char chipID[30];
    snprintf(chipID, 30, "%lx", chipid);
    ALOGD ("%s: chidId=%s", __FUNCTION__, chipID);

    readOptionalConfig(chipID);     // Read optional chip specific settings
    readOptionalConfig("fime");     // Read optional FIME specific settings
    getNfaValues();                 // Get NFA configuration values into variables

    findPatchramFile(FW_PATCH, sPatchFn, sizeof(sPatchFn));
    findPatchramFile(FW_PRE_PATCH, sPrePatchFn, sizeof(sPatchFn));

    {
        FILE *fd;
        /* If an I2C fix patch file was specified, then tell the stack about it */
        if (sPrePatchFn[0] != '\0')
        {
            fd = fopen(sPrePatchFn, "rb");
            if (fd != NULL)
            {
                UINT32 lenPrmBuffer = getFileLength(fd);

                sI2cFixPrmBuf = malloc(lenPrmBuffer);
                if (sI2cFixPrmBuf != NULL)
                {
                    size_t actualLen = fread(sI2cFixPrmBuf, 1, lenPrmBuffer, fd);
                    if (actualLen == lenPrmBuffer)
                    {
                        ALOGD("%s Setting I2C fix to %s (size: %lu)", __FUNCTION__, sPrePatchFn, lenPrmBuffer);
                        HAL_NfcPrmSetI2cPatch((UINT8*)sI2cFixPrmBuf, (UINT16)lenPrmBuffer, 0);
                    }
                    else
                        ALOGE("%s fail reading i2c fix; actual len=%u; expected len=%lu", __FUNCTION__, actualLen, lenPrmBuffer);
                }
                else
                {
                    ALOGE("%s Unable to get buffer to i2c fix (%lu bytes)", __FUNCTION__, lenPrmBuffer);
                }

                fclose(fd);
            }
            else
            {
                ALOGE("%s Unable to open i2c fix patchfile %s", __FUNCTION__, sPrePatchFn);
            }
        }
    }

    {
        FILE *fd;

        /* If a patch file was specified, then download it now */
        if (sPatchFn[0] != '\0')
        {
            UINT32 bDownloadStarted = false;
            fd = fopen(sPatchFn, "rb");
            /* open patchfile, read it into a buffer */
            if (fd != NULL)
            {
                UINT32 lenPrmBuffer = getFileLength(fd);
                ALOGD("%s Downloading patchfile %s (size: %lu) format=%u", __FUNCTION__, sPatchFn, lenPrmBuffer, NFC_HAL_PRM_FORMAT_NCD);
                if ((sPrmBuf = malloc(lenPrmBuffer)) != NULL)
                {
                    fread(sPrmBuf, lenPrmBuffer, 1, fd);

                    {
                        /* Download patch using static memeory mode */
                        HAL_NfcPrmDownloadStart(NFC_HAL_PRM_FORMAT_NCD, 0, (UINT8*)sPrmBuf, lenPrmBuffer, 0, prmCallback);
                        bDownloadStarted = true;
                    }
                }
                else
                    ALOGE("%s Unable to buffer to hold patchram (%lu bytes)", __FUNCTION__, lenPrmBuffer);

                fclose(fd);
            }
            else
                ALOGE("%s Unable to open patchfile %s", __FUNCTION__, sPatchFn);

            /* If the download never got started */
            if (!bDownloadStarted)
            {
                /* If debug mode, fail in an obvious way, otherwise try to start stack */
                postDownloadPatchram(HAL_NFC_STATUS_OK);
            }
        }
        else
        {
            ALOGE("%s: No patchfile specified or disabled. Proceeding to post-download procedure...", __FUNCTION__);
            postDownloadPatchram(HAL_NFC_STATUS_OK);
        }
    }

    ALOGD ("%s: exit", __FUNCTION__);
}

/*******************************************************************************
**
** Function:    nfc_hal_post_reset_init
**
** Description: Called by the NFC HAL after controller has been reset.
**              Begin to download firmware patch files.
**
** Returns:     none
**
*******************************************************************************/
void nfc_hal_post_reset_init (UINT32 hw_id, UINT8 nvm_type)
{
    ALOGD("%s: hw_id=0x%lu, nvm_type=%d", __FUNCTION__, hw_id, nvm_type);
    tHAL_NFC_STATUS stat = HAL_NFC_STATUS_FAILED;
    UINT8 max_credits = 1;

    if (nvm_type == NCI_SPD_NVM_TYPE_NONE)
    {
        ALOGD("%s: No NVM detected, FAIL the init stage to force a retry", __FUNCTION__);
        stat = HAL_NfcReInit ();
    }
    else
    {
        /* Start downloading the patch files */
        StartPatchDownload(hw_id);

        if (GetNumValue(MAX_RF_DATA_CREDITS, &max_credits, sizeof(max_credits)) && (max_credits > 0))
        {
            ALOGD("%s : max_credits=%d", __FUNCTION__, max_credits);
            HAL_NfcSetMaxRfDataCredits(max_credits);
        }
    }
}
/**************************************************************************************************
**
** Function         ReadPatchFile
**
** Description      Read function of the available patch and prepatch file
**
** Returns          TRUE if operation successful
**                  FALSE if fails
**
****************************************************************************************************/
int ReadPatchFile(const char* pPatchFilePath,UINT8 **patchdata,UINT32 *patchdatalen)
{
    UINT32 patchdatalength  = 0,totalreadbytes=0;
    FILE *pPatchfile        = NULL;
    UINT8 ret = FALSE;

    getNfaValues();

    if((pPatchFilePath == NULL))
    {
        /* NULL Checks*/
        ret = FALSE;
    }
    else
    {
        pPatchfile = fopen(pPatchFilePath,"rb");
        if(!pPatchfile)
        {
            HAL_TRACE_DEBUG0("File Open Failed... No file in the directory");
            ret = FALSE;
            goto done;
        }
        else
        {
            /*read the length of total patch data to allocate the buffer*/
            if(fseek(pPatchfile, 0, SEEK_END) == 0)
            {
                patchdatalength =  ftell(pPatchfile);
                *patchdatalen = patchdatalength;
                fseek(pPatchfile, 0, SEEK_SET);
                *patchdata = (UINT8 *)malloc(patchdatalength);
                if(!(*patchdata))
                {
                    /*Memory allocation failed*/
                    HAL_TRACE_DEBUG0("Memory allocation failed for patch buffer");
                    ret = FALSE;
                    goto done;
                }
                /*Read patch data to be sent to the NFCC*/
                totalreadbytes = fread((*patchdata),sizeof(UINT8),patchdatalength, pPatchfile);
                if(patchdatalength != totalreadbytes)
                {
                    HAL_TRACE_DEBUG0("Patch data read failed");
                    ret = FALSE;
                    goto done;
                }
                HAL_TRACE_DEBUG0("Patch data read success");
                ret = TRUE;
                goto done;
            }
            else
            {
                /*either file is empty or corrupt*/
                ret = FALSE;
            }
        }
    }
done:
    if(pPatchfile)
        fclose(pPatchfile);

    return ret;
}

/**************************************************************************************************
**
** Function         nfc_hal_patch_read
**
** Description      Read function of the available patch and prepatch file
**
** Returns          TRUE if operation successful
**                  FALSE if fails
**
****************************************************************************************************/
int nfc_hal_patch_read(const char* pPatchFilePath,UINT8 **patchdata,UINT32 *patchdatalength)
{
    return ReadPatchFile(pPatchFilePath,patchdata,patchdatalength);
}

/**************************************************************************************************
**
** Function         getlength
**
** Description      Convert hex array values stored in buffer in to decimal integer number
**
** Returns          Return length in decimal.
**
****************************************************************************************************/
UINT32 getlength(UINT8 * buffer,UINT8 len)
{
    UINT32 length = 0;
    char *end;
    UINT8 *input = buffer;
    UINT8 str[12]={0};
    const char * hex = "0123456789ABCDEF";
    UINT8 * output = str;
    int i = 0;
    if((input == NULL) || len < 1 || len > 12)
    {
        return 0;
    }
    for(;i < len-1; ++i){
        *output++ = hex[(*input>>4)&0xF];
        *output++ = hex[(*input++)&0xF];
    }
    *output++ = hex[(*input>>4)&0xF];
    *output++ = hex[(*input)&0xF];
    *output = 0;
    length = strtoul((const char*)str,&end, 16);
    return length;
}
/***********************************************************************************************************
**
** Function         nfc_hal_patch_validate
**
** Description      This function will check the PrePatch file and the Patch file
**                  in 3 important aspects of validity i.e -1)PrePatch ID 2)FW Version
**                  3)Signature.
**
** Returns          TRUE if both files are valid for each other
**                  FALSE if both file are not valid for each other
************************************************************************************************************/
int nfc_hal_patch_validate(UINT8 *patchdata,UINT32 patchdatalen,UINT8 *prepatchdata,UINT32 prepatchdatalen)
{
    UINT8 patch_update = FALSE, patchlengthinfo[4] = {0};
    UINT8 public_key_length[2] = {0}, signature_length_info[2] = {0};
    UINT32 public_key_length_offset = 0, signature_length_offset = 0;
    UINT32 patch_length = 0, prepatch_length = 0;

    /* find the length of prepatch data */
    memcpy(patchlengthinfo,(prepatchdata+TOTAL_LENGTH_OCTETS),PATCH_LENGTH_OCTETS);
    HAL_TRACE_DEBUG4("patchlengthinfo[] : %X %X %X %X",patchlengthinfo[0],patchlengthinfo[1],patchlengthinfo[2],patchlengthinfo[3]);

    prepatch_length = getlength(patchlengthinfo,PATCH_LENGTH_OCTETS);
    HAL_TRACE_DEBUG1("patch_length : %d",prepatch_length);

    memset((void*)patchlengthinfo,0,4);

    /* find the length of patch data */
    memcpy(patchlengthinfo,(patchdata+TOTAL_LENGTH_OCTETS),PATCH_LENGTH_OCTETS);
    HAL_TRACE_DEBUG4("patchlengthinfo[] : %X %X %X %X",patchlengthinfo[0],patchlengthinfo[1],patchlengthinfo[2],patchlengthinfo[3]);

    patch_length = getlength(patchlengthinfo,PATCH_LENGTH_OCTETS);
    HAL_TRACE_DEBUG1("patch_length : %d",patch_length);

    /* check first if  FW version is same in both files( Patch file and prepatch file)*/
    if(memcmp((patchdata+TOTAL_LENGTH_OCTETS+PATCH_LENGTH_OCTETS+patch_length-FW_VERSION_OCTETS-PATCH_OCTETS),(prepatchdata + \
               TOTAL_LENGTH_OCTETS+PATCH_LENGTH_OCTETS+prepatch_length-FW_VERSION_OCTETS-PATCH_OCTETS),FW_VERSION_OCTETS) == 0)
    {
        HAL_TRACE_DEBUG0("FW version is same in patch file and prepatch file");
    }
    else
   {
        HAL_TRACE_DEBUG0("FW version is not same in patch file and prepatch file");
        return FALSE;
   }

   /* Check if Patch Version is same in both files*/
    if(memcmp((patchdata+TOTAL_LENGTH_OCTETS+PATCH_LENGTH_OCTETS+patch_length+FW_VERSION_OCTETS-PATCH_OCTETS),(prepatchdata + \
               TOTAL_LENGTH_OCTETS+PATCH_LENGTH_OCTETS+prepatch_length+FW_VERSION_OCTETS-PATCH_OCTETS),PATCH_OCTETS) == 0)
    {
        HAL_TRACE_DEBUG0("Patch version is same in patch file and prepatch file");
    }
    else
   {
        HAL_TRACE_DEBUG0("Patch version is not same in patch file and prepatch file");
        return FALSE;
   }
   return TRUE;
}

/**********************************************************************************************************
**
** Function         nfc_hal_check_firmware_version
**
** Description      Checks if the FW version on NFCC and prepatch file is compatible or not.
**                  This will decide if the prepatch file is relevant for currently running FW or not.
** Returns          Return length in decimal.
**
************************************************************************************************************/
UINT8 nfc_hal_check_firmware_version(UINT8 *genproprsp,UINT8 resplen,UINT8 *patchdata,UINT8 patchdatalen)
{
    UINT32 patch_len = 0;
    UINT8 patchlengthinfo[4] = {0};
    UINT8 *ver_ptr = NULL;
    UINT8 chip_version = 0;
    UINT8 chip_revision_id = 0;
    UINT8 metal_revision_id = 0;

    if(patchdata == NULL || genproprsp == NULL )
    {
        return FALSE;
    }

    memcpy(patchlengthinfo,(patchdata+TOTAL_LENGTH_OCTETS),PATCH_LENGTH_OCTETS);
    patch_len = getlength(patchlengthinfo,PATCH_LENGTH_OCTETS);

    /* We are unable to read from hardware directly so read from conf. file */
    if (nfc_hal_cb.dev_cb.nfcc_chip_version == NFCC_CHIP_VERSION_INVALID)
    {
      check_patch_version(&patch_version);
    }
    else
    {
        /* Minor version flags in chip version register NOT currently set in NFCC.
           We're using metal revisions in chip revision ID register for sub-versions */
        switch(nfc_hal_cb.dev_cb.nfcc_chip_type)
        {
            case QCA1990:
                /* Major version 2 */
                if (nfc_hal_cb.dev_cb.nfcc_chip_version == NFCC_VERSION_MAJOR_V2)
                {
                    if (nfc_hal_cb.dev_cb.nfcc_chip_metal_version == NFCC_METAL_MASK0)
                    {
                        patch_version = NFCC_VERSION_V20;
                    }
                    if (nfc_hal_cb.dev_cb.nfcc_chip_metal_version == NFCC_METAL_MASK1)
                    {
                        patch_version = NFCC_VERSION_V21;
                    }
                    if (nfc_hal_cb.dev_cb.nfcc_chip_metal_version == NFCC_METAL_MASK4)
                    {
                        patch_version = NFCC_VERSION_V24;
                    }
                }
                /* Major version 3 */
                else if (nfc_hal_cb.dev_cb.nfcc_chip_version == NFCC_VERSION_MAJ0R_V3)
                {
                    patch_version = NFCC_VERSION_V30;
                }
                /* If we are reading back a different Major chip version - add future versions here..
                   As no others currently supported read from conf. file for now   */
                else if((nfc_hal_cb.dev_cb.nfcc_chip_version != NFCC_VERSION_MAJOR_V2) && (nfc_hal_cb.dev_cb.nfcc_chip_version != NFCC_VERSION_MAJ0R_V3))
                {
                    check_patch_version(&patch_version);
                }
                break;
            case QCA1990A:
                /* Binary patch file mapping is same as that of QCA1990 and will be same for further versions of QCA1990A*/
                patch_version = NFCC_VERSION_1990A;
                break;
        }
    }
    ALOGD("chip version = %d.%d\n", nfc_hal_cb.dev_cb.nfcc_chip_version, nfc_hal_cb.dev_cb.nfcc_chip_metal_version);
    ALOGD("patch version selected = %lu\n", patch_version);
    switch (patch_version)
    {
        case NFCC_VERSION_V20:
        {
            HAL_TRACE_DEBUG0("PATCH Update : FW_2.0 enabled");
            ver_ptr = (patchdata+TOTAL_LENGTH_OCTETS+PATCH_LENGTH_OCTETS + \
                       patch_len-FW_VERSION_OCTETS-PATCH_OCTETS);
            ALOGD("PATCH Version : %X %X",*(ver_ptr+2),*(ver_ptr+3));
            break;
        }
        case NFCC_VERSION_1990A:
        case NFCC_VERSION_V30:
        case NFCC_VERSION_V21:
        case NFCC_VERSION_V24:
        {
            ver_ptr = (patchdata+TOTAL_LENGTH_OCTETS+PATCH_LENGTH_OCTETS + \
                       patch_len);
            ALOGD("PATCH Version : %X %X",*(ver_ptr+2),*(ver_ptr+3));
            break;
        }
        default:
        {//v2.1 or greater
            ver_ptr = (patchdata+TOTAL_LENGTH_OCTETS+PATCH_LENGTH_OCTETS + \
                       patch_len);
            ALOGD("PATCH Version : %X %X",*(ver_ptr+2),*(ver_ptr+3));
            break;
        }
    }
    if((memcmp((genproprsp+FW_VERSION_OFFSET),ver_ptr ,FW_VERSION_OCTETS) == 0))
    {
       return TRUE;
    }
    else
    {
       return FALSE;
    }
}
/**********************************************************************************************************
**
** Function         nfc_hal_check_fw_signature
**
** Description      Checks if the patch applied successfully or not.
**
** Returns          Return length in decimal.
**
************************************************************************************************************/
UINT8 nfc_hal_check_fw_signature(UINT8 *genproprsp,UINT8 resplen,UINT8 *patchdata,UINT32 patchdatalen)
{
    UINT32 patch_len = 0,i=0;
    UINT16 public_key_len = 0;
    UINT8 patchlengthinfo[4] = {0},public_key_len_info[2]={0};
    UINT8 *ver_ptr = NULL;
    UINT8 chip_version = 0;
    UINT8 chip_revision_id = 0;
    UINT8 metal_revision_id = 0,patch_update_status = 0;
    UINT8 totallengthinfo[4] = {0};
    UINT32 total_len=0;

    HAL_TRACE_DEBUG1("patch version :%d", patch_version);

    /* First read the NFCC hardware flavour */
    chip_version = (UINT8)DT_Get_Nfcc_Version(NFCC_CHIP_VERSION_REG);
    GKI_delay( 2 );
    chip_revision_id = (UINT8)DT_Get_Nfcc_Version(NFCC_CHIP_REVID_REG);
    metal_revision_id = ((chip_revision_id) & (NFCC_METAL_REVISION_MASK));

    /* We are unable to read from hardware directly so read from conf file */
    if (chip_version == NFCC_CHIP_VERSION_INVALID)
    {
      check_patch_version(&patch_version);
    }
    else
    {
        /* Minor version flags in chip version register NOT currently set in NFCC.
           We're using metal revisions in chip revision ID register for sub-versions */
        chip_version &= (NFCC_VERSION_MAJOR_MASK);
        /* Major version 2 */
        if (chip_version == NFCC_VERSION_MAJOR_V2)
        {
            if (metal_revision_id == NFCC_METAL_MASK0)
            {
                patch_version = NFCC_VERSION_V20;
            }
            if (metal_revision_id == NFCC_METAL_MASK1)
            {
                patch_version = NFCC_VERSION_V21;
            }
            if (metal_revision_id == NFCC_METAL_MASK4)
            {
                patch_version = NFCC_VERSION_V24;
            }
        }
        /* Major version 3 */
        if (chip_version == NFCC_VERSION_MAJ0R_V3)
        {
            patch_version = NFCC_VERSION_V30;
        }
        /* If we are reading back a different Major chip version - add future versions here..
           As no others currently supported read from conf. file for now   */
        if ((chip_version != NFCC_VERSION_MAJOR_V2) && (chip_version != NFCC_VERSION_MAJ0R_V3))
        {
            check_patch_version(&patch_version);
        }
    }
    ALOGD("CHIP_REVISION_ID = %d\n", chip_revision_id);
    ALOGD("CHIP_VERSION = %d.%d\n", (chip_version>>4), metal_revision_id);

    memcpy(patchlengthinfo,(patchdata+TOTAL_LENGTH_OCTETS),PATCH_LENGTH_OCTETS);
    patch_len = getlength(patchlengthinfo,PATCH_LENGTH_OCTETS);

    memcpy(public_key_len_info,(patchdata+TOTAL_LENGTH_OCTETS+PATCH_LENGTH_OCTETS + \
           patch_len+SIG_ALGORITHM_OCTETS+RESERVED_OCTETS),PUBLIC_KEY_LENGTH_OCTETS);

    public_key_len = getlength(public_key_len_info,PUBLIC_KEY_LENGTH_OCTETS);

    switch (patch_version)
    {
       case NFCC_VERSION_V20:
       {
           /*1 deducted from rsplen to remove length byte*/
           ver_ptr = (patchdata+TOTAL_LENGTH_OCTETS+PATCH_LENGTH_OCTETS + \
                      patch_len+SIG_ALGORITHM_OCTETS+RESERVED_OCTETS+PUBLIC_KEY_LENGTH_OCTETS+ \
                      public_key_len+SIGNATURE_LENGTH_OCTETS-1);
           break;
       }
       case NFCC_VERSION_V30:
       case NFCC_VERSION_V21:
       case NFCC_VERSION_V24:
       {
           memcpy(totallengthinfo,patchdata,TOTAL_LENGTH_OCTETS);
           total_len = getlength(totallengthinfo,TOTAL_LENGTH_OCTETS);
           HAL_TRACE_DEBUG1("PATCH Update :%d",total_len);
           ver_ptr = (patchdata+total_len-SIGNATURE_LENGTH);
           break;
       }
       default:
       {
           memcpy(totallengthinfo,patchdata,TOTAL_LENGTH_OCTETS);
           total_len = getlength(totallengthinfo,TOTAL_LENGTH_OCTETS);
           ver_ptr = (patchdata+total_len-SIGNATURE_LENGTH);
           break;
       }
    }

    if(memcmp((genproprsp+(resplen-SIGNATURE_LENGTH)),ver_ptr ,SIGNATURE_LENGTH) != 0)
    {
        /* set FALSE as patch has not applied*/
        patch_update_status = FALSE;
        nfc_hal_main_collect_nfcc_info(PARAM_FW_PATCH_APPLIED,&patch_update_status,HAL_NFC_LEN_NFCC_PATCH_APPLIED_STATUS);
        return PATCH_NOT_UPDATED;
    }
    else
    {
        return PATCH_UPDATED;
    }
}
/**********************************************************************************************************

** Function:        mayDisableSecureElement
**
** Description:     Optionally adjust a TLV to disable secure element.  This feature
**                  is enabled by setting the system property
**                  nfc.disable_secure_element to a bit mask represented by a hex
**                  octet: C0 = do not detect any secure element.
**                         40 = do not detect secure element in slot 0.
**                         80 = do not detect secure element in slot 1.
**
**                  config: a sequence of TLV's.
**
*******************************************************************************/
void mayDisableSecureElement (StartupConfig& config)
{
    unsigned int bitmask = 0;
    char valueStr [PROPERTY_VALUE_MAX] = {0};
    int len = property_get ("nfc.disable_secure_element", valueStr, "");
    if (len > 0)
    {
        sscanf (valueStr, "%x", &bitmask); //read system property as a hex octet
        ALOGD ("%s: disable 0x%02X", __FUNCTION__, (UINT8) bitmask);
        config.disableSecureElement ((UINT8) (bitmask & 0xC0));
    }
}

void check_patch_version(UINT32 *version)
{
   GetNumValue("PATCH_VERSION", version, sizeof(*version));
}
