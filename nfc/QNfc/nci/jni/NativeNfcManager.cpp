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
#include <unistd.h>
#include <errno.h>
#include "OverrideLog.h"
#include "NfcJniUtil.h"
#include "NfcAdaptation.h"
#include "SyncEvent.h"
#include "PeerToPeer.h"
#include "RoutingManager.h"
#include "ApduGateManager.h"
#include "NfcTag.h"
#include "config.h"
#include "PowerSwitch.h"
#include "JavaClassConstants.h"
#include "Pn544Interop.h"
#include <ScopedLocalRef.h>
#include <ScopedUtfChars.h>
#include <ScopedPrimitiveArray.h>
#include <sys/endian.h>

extern "C"
{
    #include "nfa_api.h"
    #include "nfa_p2p_api.h"
    #include "rw_api.h"
    #include "nfa_ee_api.h"
    #include "nfc_brcm_defs.h"
    #include "ce_api.h"
    #ifdef DTA // <DTA>
    #include "dta_mode.h"
    #endif // </DTA>
}


#define DEVICE_POWER_CYCLED 0
#define STORE_INFO_DEBUG_ENABLE 1 // 2^0
#define STORE_INFO_NFC_DISABLED 2 // 2^1
#define NFCSERVICE_WATCHDOG_TIMER_EXPIRED 4 // 2^2
#define NFCSERVICE_GIVE_UP 8 // 2^3
#define CORE_RST_CMD_TIMEOUT_DURING_INIT 16 // 2^4

extern UINT8 *p_nfa_dm_start_up_cfg;
extern const UINT8 nfca_version_string [];
extern const UINT8 nfa_version_string [];
extern tNFA_DM_DISC_FREQ_CFG* p_nfa_dm_rf_disc_freq_cfg; //defined in stack

#define CRC_PARITY_PARAM_SET_DELAY        500
namespace android
{
    extern bool gIsTagDeactivating;
    extern bool gIsSelectingRfInterface;
    extern void nativeNfcTag_doTransceiveStatus (tNFA_STATUS status, uint8_t * buf, uint32_t buflen);
    extern void nativeNfcTag_notifyRfTimeout ();
    extern void nativeNfcTag_doConnectStatus (jboolean is_connect_ok);
    extern void nativeNfcTag_doDeactivateStatus (int status);
    extern void nativeNfcTag_doWriteStatus (jboolean is_write_ok);
    extern void nativeNfcTag_doCheckNdefResult (tNFA_STATUS status, uint32_t max_size, uint32_t current_size, uint8_t flags);
    extern void nativeNfcTag_doMakeReadonlyResult (tNFA_STATUS status);
    extern void nativeNfcTag_doPresenceCheckResult (tNFA_STATUS status);
    extern void nativeNfcTag_formatStatus (bool is_ok);
    extern void nativeNfcTag_resetPresenceCheck ();
    extern void nativeNfcTag_doReadCompleted (tNFA_STATUS status);
    extern void nativeNfcTag_abortWaits ();
    extern void nativeLlcpConnectionlessSocket_abortWait ();
    extern void nativeNfcTag_registerNdefTypeHandler ();
    extern void nativeLlcpConnectionlessSocket_receiveData (uint8_t* data, uint32_t len, uint32_t remote_sap);
    #ifdef DTA // <DTA>
    extern struct ResponseMessage* nfcDepTransceive(uint8_t* buf, uint32_t bufLen);
    extern void nfcDepTransceiveStatus(uint8_t* buf, uint32_t bufLen);
    extern void debugMessage(uint8_t* array, uint32_t size);
    #endif // </DTA>
}


/*****************************************************************************
**
** public variables and functions
**
*****************************************************************************/
bool                        gActivated = false;
SyncEvent                   gDeactivatedEvent;

namespace android
{
    jmethodID               gCachedNfcManagerNotifyNdefMessageListeners;
    jmethodID               gCachedNfcManagerNotifyTransactionListeners;
    jmethodID               gCachedNfcManagerNotifyRfIntfDeactivated;
    jmethodID               gCachedNfcManagerNotifyHciEventConnectivity;
    jmethodID               gCachedNfcManagerNotifyLlcpLinkActivation;
    jmethodID               gCachedNfcManagerNotifyLlcpLinkDeactivated;
    jmethodID               gCachedNfcManagerNotifyLlcpFirstPacketReceived;
    jmethodID               gCachedNfcManagerNotifyHostEmuActivated;
    jmethodID               gCachedNfcManagerNotifyHostEmuData;
    jmethodID               gCachedNfcManagerNotifyHostEmuDeactivated;
    jmethodID               gCachedNfcManagerNotifyRfFieldActivated;
    jmethodID               gCachedNfcManagerNotifyRfFieldDeactivated;
    jmethodID               gCachedNfcManagerNotifyNfccInfo;
    jmethodID               gCachedNfcManagerUpdateHostCallBack;
    jmethodID               gCachedNfcManagerUpdateResetCounter;
    jmethodID               gCachedNfcManagerNotifyRequestRestartNfc;
    const char*             gNativeP2pDeviceClassName                 = "com/android/nfc/dhimpl/NativeP2pDevice";
    const char*             gNativeLlcpServiceSocketClassName         = "com/android/nfc/dhimpl/NativeLlcpServiceSocket";
    const char*             gNativeLlcpConnectionlessSocketClassName  = "com/android/nfc/dhimpl/NativeLlcpConnectionlessSocket";
    const char*             gNativeLlcpSocketClassName                = "com/android/nfc/dhimpl/NativeLlcpSocket";
    const char*             gNativeNfcTagClassName                    = "com/android/nfc/dhimpl/NativeNfcTag";
    const char*             gNativeNfcManagerClassName                = "com/android/nfc/dhimpl/NativeNfcManager";
    void                    doStartupConfig ();
    void                    startStopPolling (bool isStartPolling);
    void                    startRfDiscovery (bool isStart);
    void                    restartPollingWithTechMask(int mask);
    void                    getNVM(UINT32 index);
    void                    setNVM(UINT32 index);
    void                    NVMCallback(UINT8 event, UINT16 param_len, UINT8 *p_param);
    void                    modifyIndexedNVM(UINT32 index, UINT8 value);
    void                    updateHostPresenceCallback (UINT8 event, UINT16 param_len, UINT8 *p_param);
    void                    nfcManager_notifyRfIntfDeactivated ();
    #ifdef DTA // </DTA>
    const int gNfcDepDslDeactivation = 1;
    const int gNfcDepRlsDeactivation = 2;
    const int gNfaDeactivationToSleep = 3;
    const int gNfaDeactivation = 4;
    const bool generalBytesOn = false; // Controls whether general bytes are included in the ATR_RES or not.
    #endif // </DTA>
}


/*****************************************************************************
**
** private variables and functions
**
*****************************************************************************/
namespace android
{
static jint                 sLastError = ERROR_BUFFER_TOO_SMALL;
static jmethodID            sCachedNfcManagerNotifySeApduReceived;
static jmethodID            sCachedNfcManagerNotifySeMifareAccess;
static jmethodID            sCachedNfcManagerNotifySeEmvCardRemoval;
static jmethodID            sCachedNfcManagerNotifyTargetDeselected;
static bool                 sNotifyDeactivatedRfInterface = false; // true if trigger sDeactivateRfInterfaceEvent
static SyncEvent            sDeactivateRfInterfaceEvent; // event for deactivation RF interface
static SyncEvent            sNfaEnableEvent;  //event for NFA_Enable()
static SyncEvent            sNfaDisableEvent;  //event for NFA_Disable()
static SyncEvent            sNfaEnableDisablePollingEvent;  //event for NFA_EnablePolling(), NFA_DisablePolling()
static SyncEvent            sNfaSetConfigEvent;  // event for Set_Config.
static SyncEvent            sNfaGetConfigEvent;  // event for Get_Config....
static SyncEvent            sNfaGetLMRTEvent;  // event for Get_ListenModeRoutingTable....
static SyncEvent            sNfaModifyNVM; // event for NVM modification
static SyncEvent            sNfaGetRamDumpEvent;  // event for RamDump....
static SyncEvent            sNfaVsCommandEvent;
static bool                 sIsNfaEnabled = false;
static bool                 sIsNfcPresent = true;  //whether NFC chip is present on device
static bool                 sPollingLocked = false;
static bool                 sIsDisabling = false;
static bool                 sReaderModeEnabled = false; // whether we're only reading tags, not allowing P2p/card emu
static bool                 sP2pEnabled = false;
static bool                 sP2pActive = false; // whether p2p was last active
static bool                 sAbortConnlessWait = false;
static bool                 sIsNfcRestart = false;  // Nfc stack re-starting
static UINT32               sNVMbuffer = 0;


#ifdef DTA // <DTA>
// static int          sDtaPatternNumber = 0x00;
static int sDtaPatternNumber = -1;
/* should be the same as above but just incase backup last pattern set */
// static int          LastPatternNumber = 0x00;
static int LastPatternNumber = -1;
#endif // </DTA>
#define CONFIG_UPDATE_TECH_MASK     (1 << 1)
#define DEFAULT_TECH_MASK           (NFA_TECHNOLOGY_MASK_A \
                                     | NFA_TECHNOLOGY_MASK_B \
                                     | NFA_TECHNOLOGY_MASK_F \
                                     | NFA_TECHNOLOGY_MASK_ISO15693 \
                                     | NFA_TECHNOLOGY_MASK_B_PRIME \
                                     | NFA_TECHNOLOGY_MASK_A_ACTIVE \
                                     | NFA_TECHNOLOGY_MASK_F_ACTIVE \
                                     | NFA_TECHNOLOGY_MASK_KOVIO)
#define DEFAULT_DISCOVERY_DURATION       500
#define READER_MODE_DISCOVERY_DURATION   200

static void nfaConnectionCallback (UINT8 event, tNFA_CONN_EVT_DATA *eventData);
static void nfaDeviceManagementCallback (UINT8 event, tNFA_DM_CBACK_DATA *eventData);
static bool isPeerToPeer (tNFA_ACTIVATED& activated);
static bool isListenMode(tNFA_ACTIVATED& activated);
static void enableDisableLptd (bool enable);
static tNFA_STATUS stopPolling_rfDiscoveryDisabled();
static tNFA_STATUS startPolling_rfDiscoveryDisabled(tNFA_TECHNOLOGY_MASK tech_mask);

static UINT16 sCurrentConfigLen;
#ifdef DTA // <DTA>
#define T1T_BLK_SIZE 8
#ifdef GETCONFIG_IMPLEMENT
static void getNciConfigurationValues();
#endif
static bool enablePassivePollMode = true;
static bool AnyPollModeSet = false;

static void myType4ListenLoopback(uint8_t* p_buf, uint32_t len);
UINT8 t1t_dyn_activated = FALSE;
extern UINT8 t1t_init_blocks[24];
extern UINT8*t1t_tag_data;
UINT16 data_idex = 0;
UINT8 packet_count=1;
UINT8 data_packets_cnt = 0;

/*
 * NVM 155: RFPOLLMACP_ENABLE_FEATURE
 */
typedef union
{
    struct // bits of NVM
    {
        // RF poll as a pre-condition to NFC poll
        UINT8 ENABLE_RF_POLL : 1;
        // enable power management firmware
        UINT8 ENABLE_PM : 1;
        // enable watchdog
        UINT8 ENABLE_WDOG : 1;
        UINT8 ENABLE_SHALLOW_SLEEP : 1;
        UINT8 RESERVED : 4;
    } reg;
    UINT8 raw; // full byte
} sNVM_155;


/*
 * JNI DISCOVER STATE
 */
typedef enum {
    DISCOVER_STATE_IDLE = 0, /*Stop to discover*/
    DISCOVER_STATE_DISCOVERY,/*Start to discover*/
    DISCOVER_STATE_POLL_DEACTIVATING,
    DISCOVER_STATE_POLL_ACTIVE, /* Polling is active other than p2p */
    DISCOVER_STATE_LISTEN_ACTIVE, /* Listening is active other than p2p */
    DISCOVER_STATE_POLL_P2P_ACTIVE,/* P2P polling is active */
    DISCOVER_STATE_LISTEN_P2P_ACTIVE,/* P2P listening is active */
}tJNI_DISCOVER_STATE;


static tJNI_DISCOVER_STATE  sDiscoverState;

#define RFPOLLMACP_ENABLE_FEATURE   155

#endif // </DTA>
/*
 * NVM 1763: RXAGCMAP_RXAGCENABLE
 */
typedef union
{
    UINT8 reg;
    UINT8 raw; // full byte
} sNVM_1763;

#define RXAGCMAP_RXAGCENABLE        1763

static UINT8 sConfig[256];
static UINT8 sLongGuardTime[] = { 0x00, 0x20 };
static UINT8 sDefaultGuardTime[] = { 0x00, 0x11 };
bool sIsDeviceReset = false;
int   sApduGateDelay = 0;
int   sShutdownReason = 0; //NFC_DISABLED_BY_SYSTEM = 0
bool  sIsRegion2Enable = false;
int   sDefaultActiveNfceeId = 0x01; // UICC1 as default
#define SIZE_OF_NFCEE_ID_TO_SE_NAME 6
struct {
    int  nfceeId;
    char *seName;
} sNfceeId2SeName[SIZE_OF_NFCEE_ID_TO_SE_NAME] = { {0x00, "DH"},
                                                   {0x01, "SIM1"},
                                                   {0x02, "SIM2"},
                                                   {0x03, "eSE1"},
                                                   {0x04, "eSE2"},
                                                   {0x05, "SD1"} };
typedef struct {
  UINT8 numEntries;
  UINT16 totalSize;
  UINT8 * pEntries;
} tLMRTData;

tLMRTData sLMRTData;

static char *nfcManager_getSeNameOfNfceeId (int nfceeId);
static int nfcManager_getNfceeIdOfSeName (const char *seName);
bool  sIsUiStateSupported = false;  // true if NFCC supports UI State update
static int sNfcChipVersion = 0;     // store the chip version
// Ramdump related variables;
// using static memory because this memory is shared between ramdump sectors
// There could be many sectors in a ramdump
static UINT8 ramDump[256];
bool valid_data=false;

static int sEeRoutingState = 2; // CE mode enabled when screen is unlocked
static bool sEeRoutingReloadAtReboot = false; //do not update setting from config file when reboot

#define CONFIG_MAX_UID_LEN 10
UINT8 saved_UID_size;
UINT8 saved_UID[CONFIG_MAX_UID_LEN];
UINT8 config_HCE_UID_size = 0;
//The +1 is to detect invalid UID sizes greater than 10
UINT8 config_HCE_UID[CONFIG_MAX_UID_LEN+1];
UINT8 last_uistate[3];
static bool  sUiStateNeedCallback = true;
static bool  sPendUiCmd = false;

void nfcManager_updateResetCount(bool reset);
void nfcManager_updateResetCount(JNIEnv *e, jobject o, bool reset);

/*
 * BERT/PRBS
 */
jboolean nfcManager_PrbsOn(JNIEnv* e, jobject o, int tech, int rate, bool init);
jboolean nfcManager_PrbsOff(JNIEnv* e, jobject o);
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


/*******************************************************************************
**
** Function:        getNative
**
** Description:     Get native data
**
** Returns:         Native data structure.
**
*******************************************************************************/
nfc_jni_native_data *getNative (JNIEnv* e, jobject o)
{
    static struct nfc_jni_native_data *sCachedNat = NULL;
    if (e)
    {
        sCachedNat = nfc_jni_get_nat(e, o);
    }
    return sCachedNat;
}


/*******************************************************************************
**
** Function:        handleRfDiscoveryEvent
**
** Description:     Handle RF-discovery events from the stack.
**                  discoveredDevice: Discovered device.
**
** Returns:         None
**
*******************************************************************************/
static void handleRfDiscoveryEvent (tNFC_RESULT_DEVT* discoveredDevice)
{
    if (discoveredDevice->more)
    {
        //there is more discovery notification coming
        return;
    }

    bool isP2p = NfcTag::getInstance ().isP2pDiscovered ();
    if (!sReaderModeEnabled && isP2p)
    {
        //select the peer that supports P2P
        NfcTag::getInstance ().selectP2p();
    }
    else
    {
        //select the first of multiple tags that is discovered
        NfcTag::getInstance ().selectFirstTag();
    }
}

/*******************************************************************************
**
** Function:        sendVsRamdumpCallback
**
** Description:     HAL call back with ramdump data
**                  event: type of event supplied by HAL
**                  param_len: total length of the response message
**                  p_param: pointer to the payload+header
**
** Returns:         None.
**
*******************************************************************************/
void sendVsRamdumpCallback(UINT8 event, UINT16 param_len, UINT8 *p_param)
{
    // HAL constructed event for identifying the data
    // 1 Byte with bit layout: rroooooo
    // rr = 2 bits for RSP, i.e. 0x40
    // oooooo = 6 bits for oid
    const UINT8 oid_mask = 0x3F;
    const UINT8 rsp_mask = 0xC0;
    const UINT8 oid_00 = 0x00;
    const UINT8 payload_start_idx = 6;
    //calculate oid from the event we got must be same as what we sent in command
    UINT8 oid = (event & oid_mask);
    ALOGD ("%s: event: 0x%02X, param_len: %d", __FUNCTION__,event, param_len);
    if(param_len > 6){
        valid_data = true;
    }
    //check if the event we got is of type response
    if (((event & rsp_mask) == NCI_RSP_BIT) && (oid == oid_00))
    {
            memcpy(ramDump, p_param+6, param_len-6);
            sNfaGetRamDumpEvent.notifyOne();
    }
}

/*******************************************************************************
**
** Function:        nfcManager_doGetRamDump
**
** Description:     JNI API responsible to collect RamDump from NFCC
**                  The caller should check the length of data to determine
**                  success or failure.
**                  e: JVM environment.
**                  o: Java object.
**                  address: RAM address of NFCC
**                  length:  length of data to collect from address
**
** Returns:         On Success or Failure the jbyteArray buffer with RamDump data.
**
*******************************************************************************/
static jbyteArray nfcManager_doGetRamDump(JNIEnv *e, jobject o,jint address , jint length)
{
        const UINT8 max_bytes_to_read = 0xF9;
        const UINT8 address_delta = 0x01;
        const UINT8 access_delay = 0x00;
        const UINT8 access_flag = 0x00;
        const UINT8 oid_00 = 0x00;
        const UINT8 cmd_length = 0x09;

        UINT32 bytes_read = 0;
        UINT8 fragment = 0;
        tNFA_STATUS stat = NFA_STATUS_FAILED;
        ALOGD ("[USERRAMDUMP] %s: enter", __FUNCTION__);

        // As per NCI specification; NCI Control messages table 39 for the format of above command
        // byte          0  1  2  3  4  5  6  7  8
        //      2F 00 09 00 00 00 04 00 F4 01 00 00
        //      |  |  |
        //    GID OID Len

        UINT8 ram_cmd_params[9];
        //access flag
        ram_cmd_params[0] = access_flag;

        //address delta
        ram_cmd_params[6] = address_delta;
        //access delay
        ram_cmd_params[7] = access_delay;
        ram_cmd_params[8] = access_delay;

        jbyteArray ramdump = e->NewByteArray(length);

        SyncEventGuard guard(sNfaGetRamDumpEvent);

        while (length != bytes_read)
        {
            //number of items to read from chip
            if ((length-bytes_read) < max_bytes_to_read)
            {
                fragment = (length-bytes_read);
            } else
            {
                fragment = max_bytes_to_read;
            }
            ram_cmd_params[5] = fragment;
            //fill the cmd_param with the address of the fragment
            ram_cmd_params[4] = (UINT8)(address >> 24); //MSB   byte 1
            ram_cmd_params[3] = (UINT8)(address >> 16); //      byte 2
            ram_cmd_params[2] = (UINT8)(address >> 8);  //      byte 3
            ram_cmd_params[1] = (UINT8)(address );      //LSB   byte 4
            stat = NFA_SendVsCommand(oid_00, cmd_length, ram_cmd_params, sendVsRamdumpCallback);
            if (stat == NFA_STATUS_OK)
            {
                sNfaGetRamDumpEvent.wait (1000);
                if (valid_data)
                {
                    e->SetByteArrayRegion(ramdump, (jsize)bytes_read, (jsize)fragment, (jbyte *)ramDump );
                    valid_data = false; // reset
                }
                //increment the address to point to next fragment
                address += fragment;
                bytes_read += fragment;
            } else
            {
                ALOGE("%s: NFA_SendVsCommand failed", __FUNCTION__);
                break; // out of while
            }
        }
        return ramdump;
}

#ifdef DTA // <DTA>
/*******************************************************************************
**
** Function:        nfcManager_dta_set_pattern_number
**
** Description:     Set DTA pattern number.
**                  e: JVM environment.
**                  o: Java object.
**        pattern: DTA pattern number.
**
** Returns:         None.
**
*******************************************************************************/
static void nfcManager_dta_set_pattern_number(JNIEnv *e, jobject o, jint pattern)
{
    tNFA_STATUS stat = NFA_STATUS_OK;

    ALOGD ("[DTA] %s: enter,%d", __FUNCTION__, pattern);
    sDtaPatternNumber = (int)pattern;
    stat = NFA_DTA_Set_Pattern_Number(sDtaPatternNumber);
    if (stat != NFA_STATUS_OK)
    {
        ALOGD ("[DTA] %s: fail,%d", __FUNCTION__, stat);
    }
}

/*******************************************************************************
**
** Function:        nfcManager_dta_get_pattern_number
**
** Description:     Get DTA pattern number.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         Pattern number.
**
*******************************************************************************/
static jint nfcManager_dta_get_pattern_number(JNIEnv *e, jobject o)
{
        ALOGD ("[DTA] %s: enter", __FUNCTION__);
        jint pattern = (jint)NFA_DTA_Get_Pattern_Number();
        ALOGD ("[DTA] pattern: %d", pattern);
        return pattern;
}

static void dta_ndef_callback(tNFA_NDEF_EVT event, tNFA_NDEF_EVT_DATA *p_data) {
    // This function is intentionally empty.
}

#include "DtaHelper.h"
void sendtitdata(UINT8 i)
{
    UINT8 t1t_tag_reserved_data[24] = {0x55,0x55,0xAA,0xAA,0x12,0x49,0x06,0x00,0x01,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    ALOGD ("sendtitdata: i=%d",i);
    switch(i)
    {
        case 1:
        {
            ALOGD ("0th block: i=%d",i);
            NFA_RwT1tWrite8(0,t1t_init_blocks,1);
        }
        break;
        case 2:
        {
            ALOGD ("2nd block: i=%d",i);
            NFA_RwT1tWrite8(i,t1t_init_blocks+16,1);
        }
        break;
        default:
        {
            ALOGD ("now writing to block: i=%d",i);
            ALOGD ("t1t_tag_data=%X",t1t_tag_data);
            if((i==13) || (i==14) || (i==15) ||(i==0))
            {
                if(i == 13)
                {
                    NFA_RwT1tWrite8(i,t1t_tag_reserved_data,1);
                }
                else if(i==14)
                {
                    NFA_RwT1tWrite8(i,&t1t_tag_reserved_data[8],1);
                }
                else if(i == 15)
                {
                    NFA_RwT1tWrite8(i,&t1t_tag_reserved_data[16],1);
                }
                else
                {
                    //send CC byte E1 now
                    t1t_init_blocks[8] = 0xE1;
                    NFA_RwT1tWrite8(1,&t1t_init_blocks[8],1);
                }

            }
            else
            {
                NFA_RwT1tWrite8(i,(t1t_tag_data+data_idex),1);
                ALOGD ("i=%d: data_packets_cnt=%d",i,data_packets_cnt);
                if(i<(data_packets_cnt+5))
                {
                    data_idex = data_idex + 8;
                    ALOGD ("data_idex=%X",data_idex);
                }
                else
                {
                    //ALOGD ("Setting packet_count=%X",packet_count);
                    packet_count = 0;
                    ALOGD ("Setting packet_count=%X",packet_count);
                }
            }
        }
        break;
    }
}
#endif // </DTA>

/*******************************************************************************
**
** Function:        nfaConnectionCallback
**
** Description:     Receive connection-related events from stack.
**                  connEvent: Event code.
**                  eventData: Event data.
**
** Returns:         None
**
*******************************************************************************/
static void nfaConnectionCallback (UINT8 connEvent, tNFA_CONN_EVT_DATA* eventData)
{
    #ifdef DTA // <DTA>
    UINT8 x = 0;
    #endif // </DTA>
    tNFA_STATUS status = NFA_STATUS_FAILED;
    ALOGD("%s: event= %u", __FUNCTION__, connEvent);

    switch (connEvent)
    {
    case NFA_POLL_ENABLED_EVT: // whether polling successfully started
        {
            ALOGD("%s: NFA_POLL_ENABLED_EVT: status = %u", __FUNCTION__, eventData->status);

            SyncEventGuard guard (sNfaEnableDisablePollingEvent);
            sNfaEnableDisablePollingEvent.notifyOne ();
        }
        break;

    case NFA_POLL_DISABLED_EVT: // Listening/Polling stopped
        {
            ALOGD("%s: NFA_POLL_DISABLED_EVT: status = %u", __FUNCTION__, eventData->status);

            SyncEventGuard guard (sNfaEnableDisablePollingEvent);
            sNfaEnableDisablePollingEvent.notifyOne ();
        }
        break;

    #ifdef DTA // <DTA>
    case NFA_EXCLUSIVE_RF_CONTROL_STARTED_EVT:
       {
             ALOGD("%s: [DTA] NFA_EXCLUSIVE_RF_CONTROL_STARTED_EVT: status = %u", __FUNCTION__, eventData->status);
       }
    #endif // </DTA>
    case NFA_RF_DISCOVERY_STARTED_EVT: // RF Discovery started
        {
            ALOGD("%s: NFA_RF_DISCOVERY_STARTED_EVT: status = %u", __FUNCTION__, eventData->status);

            SyncEventGuard guard (sNfaEnableDisablePollingEvent);
            sNfaEnableDisablePollingEvent.notifyOne ();
        }
        break;
    #ifdef DTA // <DTA>
    case NFA_EXCLUSIVE_RF_CONTROL_STOPPED_EVT:
        {
            ALOGD("%s: [DTA] NFA_EXCLUSIVE_RF_CONTROL_STOPPED_EVT: status = %u", __FUNCTION__, eventData->status);
        }
    #endif // <DTA>

    case NFA_RF_DISCOVERY_STOPPED_EVT: // RF Discovery stopped event
        {
            ALOGD("%s: NFA_RF_DISCOVERY_STOPPED_EVT: status = %u", __FUNCTION__, eventData->status);

            SyncEventGuard guard (sNfaEnableDisablePollingEvent);
            sNfaEnableDisablePollingEvent.notifyOne ();
        }
        break;

    case NFA_DISC_RESULT_EVT: // NFC link/protocol discovery notificaiton
        status = eventData->disc_result.status;
        ALOGD("%s: NFA_DISC_RESULT_EVT: status = %d", __FUNCTION__, status);
        if (status != NFA_STATUS_OK)
        {
            ALOGE("%s: NFA_DISC_RESULT_EVT error: status = %d", __FUNCTION__, status);
        }
        else
        {
            NfcTag::getInstance().connectionEventHandler(connEvent, eventData);
            handleRfDiscoveryEvent(&eventData->disc_result.discovery_ntf);
        }
        break;

    case NFA_SELECT_RESULT_EVT: // NFC link/protocol discovery select response
        ALOGD("%s: NFA_SELECT_RESULT_EVT: status = %d, gIsSelectingRfInterface = %d, sIsDisabling=%d", __FUNCTION__, eventData->status, gIsSelectingRfInterface, sIsDisabling);

        if (sIsDisabling)
            break;

        if (eventData->status != NFA_STATUS_OK)
        {
            if (gIsSelectingRfInterface)
            {
                nativeNfcTag_doConnectStatus(false);
            }

            ALOGE("%s: NFA_SELECT_RESULT_EVT error: status = %d", __FUNCTION__, eventData->status);
            if(NfcTag::getInstance().mIsMifareClassicOp == true)
            {
                NfcTag::getInstance().mIsMifareClassicOp = false;
            }
            NFA_Deactivate (FALSE);
        }
        break;

    case NFA_DEACTIVATE_FAIL_EVT:
        sDiscoverState = DISCOVER_STATE_IDLE;
        ALOGD("%s: NFA_DEACTIVATE_FAIL_EVT: status = %d", __FUNCTION__, eventData->status);
        /* notify complete of deactivation of RF interface */
        if (sNotifyDeactivatedRfInterface)
        {
            SyncEventGuard guard (sDeactivateRfInterfaceEvent);
            sDeactivateRfInterfaceEvent.notifyOne();
        }
        break;

    case NFA_ACTIVATED_EVT: // NFC link/protocol activated
        ALOGD("%s: NFA_ACTIVATED_EVT: gIsSelectingRfInterface=%d, sIsDisabling=%d,0x%02x", __FUNCTION__, gIsSelectingRfInterface, sIsDisabling, sDiscoverState);
        if (sIsDisabling || !sIsNfaEnabled)
            break;
        if (isPeerToPeer(eventData->activated))
        {
            if (isListenMode(eventData->activated))
            {
                sDiscoverState = DISCOVER_STATE_LISTEN_P2P_ACTIVE;
            }
            else
            {
                sDiscoverState = DISCOVER_STATE_POLL_P2P_ACTIVE;
            }
        }
        else
        {
            if (isListenMode(eventData->activated))
            {
                sDiscoverState = DISCOVER_STATE_LISTEN_ACTIVE;
            }
            else
            {
                sDiscoverState = DISCOVER_STATE_POLL_ACTIVE;
            }
        }

        NfcTag::getInstance().setRfInterface(eventData->activated.activate_ntf.intf_param.type);
        if(!in_dta_mode())
        {
          NfcTag::getInstance().setActive(true);
        }
        if(!in_dta_mode())
        {
          gActivated = true;
        }
        #ifdef DTA // <DTA>
        if(in_dta_mode())
        {
            ALOGD("checking dta mode: %x", eventData->activated.activate_ntf.rf_tech_param.mode);
            /* Check if we're in POLL or LISTEN mode following activation CB */
            if (
                ( (eventData->activated.activate_ntf.rf_tech_param.mode) == NCI_DISCOVERY_TYPE_POLL_A) || // 00b
                ( (eventData->activated.activate_ntf.rf_tech_param.mode) == NCI_DISCOVERY_TYPE_POLL_B) || // 01b
                ( (eventData->activated.activate_ntf.rf_tech_param.mode) == NCI_DISCOVERY_TYPE_POLL_F) || // 10b
                ( (eventData->activated.activate_ntf.rf_tech_param.mode) == NCI_DISCOVERY_TYPE_POLL_A_ACTIVE) || // 11b
                ( (eventData->activated.activate_ntf.rf_tech_param.mode) == NCI_DISCOVERY_TYPE_POLL_F_ACTIVE) ||
                ( (eventData->activated.activate_ntf.rf_tech_param.mode) == NCI_DISCOVERY_TYPE_POLL_ISO15693) ||
                ( (eventData->activated.activate_ntf.rf_tech_param.mode) == NCI_DISCOVERY_TYPE_POLL_B_PRIME)  ||
                ( (eventData->activated.activate_ntf.rf_tech_param.mode) == NCI_DISCOVERY_TYPE_POLL_KOVIO)
               )
            {
                AnyPollModeSet = true;
                ALOGD("POLL MODE");
            }
            else
            {
                AnyPollModeSet = false;
                ALOGD("LISTEN MODE");
            }
            if(eventData->activated.params.t1t.hr[0] == 0x12)
            {
                ALOGD("%s: in DTA mode,Dynamic memory T1T activated", __FUNCTION__);
                t1t_dyn_activated = TRUE;
            }
        }
        #endif // </DTA>
        NfcTag::getInstance().setActivationState ();
        if (gIsSelectingRfInterface)
        {
            nativeNfcTag_doConnectStatus(true);
            break;
        }

        nativeNfcTag_resetPresenceCheck();
        if ((sDiscoverState == DISCOVER_STATE_POLL_P2P_ACTIVE) ||
            (sDiscoverState == DISCOVER_STATE_LISTEN_P2P_ACTIVE))
        {
            if (sReaderModeEnabled)
            {
                ALOGD("%s: ignoring peer target in reader mode.", __FUNCTION__);
                NFA_Deactivate (FALSE);
                break;
            }
            ALOGD("%s: NFA_ACTIVATED_EVT; is p2p", __FUNCTION__);
            // Disable RF field events in case of p2p
            #ifdef DTA // <DTA>
            /*Disable RF field info evt reset in DTA mode*/
            if (!( in_dta_mode() )) {
            #endif // <DTA>
            UINT8  nfa_disable_rf_events[] = { 0x00 };
            ALOGD ("%s: Disabling RF field events", __FUNCTION__);
            status = NFA_SetConfig(NCI_PARAM_ID_RF_FIELD_INFO, sizeof(nfa_disable_rf_events),
                    &nfa_disable_rf_events[0]);
            if (status == NFA_STATUS_OK) {
                ALOGD ("%s: Disabled RF field events", __FUNCTION__);
            } else {
                ALOGE ("%s: Failed to disable RF field events", __FUNCTION__);
            }
        #ifdef DTA // <DTA>
        }
        #endif // </DTA>
        #ifdef DTA // <DTA>
        ALOGD("eventData->activated.params.t1t.hr[0]=%X",eventData->activated.params.t1t.hr[0]);
        if (in_dta_mode() && !in_llcp_or_snep_dta_mode()) {
            ALOGD("%s: in DTA mode: eventData->activated.params.t1t.hr[0]=%X", __FUNCTION__, eventData->activated.params.t1t.hr[0]);
            NfcTag::getInstance().handleP2pConnection (NFA_ACTIVATED_EVT, eventData);
            NfcTag::getInstance().isP2pDiscovered();
            NfcTag::getInstance().selectP2p();
        }
        #endif // </DTA>
        }
        else if (pn544InteropIsBusy() == false)
        {
            NfcTag::getInstance().connectionEventHandler (connEvent, eventData);

            // We know it is not activating for P2P.  If it activated in
            // listen mode then it is likely for an SE transaction.
            // Send the RF Event.
            if (sDiscoverState == DISCOVER_STATE_LISTEN_ACTIVE)
            {
                ApduGateManager::getInstance().notifyListenModeState (true);
            }
        }
        nfcManager_updateResetCount(true);
        break;

    case NFA_DEACTIVATED_EVT: // NFC link/protocol deactivated
        ALOGD("%s: NFA_DEACTIVATED_EVT   Type: %u, gIsTagDeactivating:%d,0x%02x", __FUNCTION__, eventData->deactivated.type,gIsTagDeactivating, sDiscoverState );
        NfcTag::getInstance().setDeactivationState (eventData->deactivated);
        if (eventData->deactivated.type != NFA_DEACTIVATE_TYPE_SLEEP)
        {
            UINT8 sIsWaiting = FALSE;
            // Deactivation is done . Update waitstatus for nfcservice call to 0.
            NfcTag::getInstance().WaitStatus(&sIsWaiting);
            {
                SyncEventGuard g (gDeactivatedEvent);
                gActivated = false; //guard this variable from multi-threaded access
                gDeactivatedEvent.notifyOne ();
            }
            nativeNfcTag_resetPresenceCheck();
            NfcTag::getInstance().connectionEventHandler (connEvent, eventData);
            nativeNfcTag_abortWaits();
            NfcTag::getInstance().abort ();
            nfcManager_notifyRfIntfDeactivated();
        }
        else if (gIsTagDeactivating)
        {
            NfcTag::getInstance().setActive(false);
            gActivated = false;
            nativeNfcTag_doDeactivateStatus(0);
        }

        #ifdef DTA // <DTA> CR510341 the lines below were commented
        if (in_dta_mode() ) {
            ALOGD("%s: NFA_DEACTIVATED_EVT in DTA mode", __FUNCTION__);
            //NfcTag::getInstance().handleP2pConnection (NFA_ACTIVATED_EVT, eventData);
            //NfcTag::getInstance().selectP2p();

            //CR 579297 if nfcc is already sleeping leave it alone
            if ((eventData->deactivated.type != 0) && (AnyPollModeSet == true)) { // additional check to prevent WTX
               ALOGD("%s: send DEACT_CMD", __FUNCTION__);
               NFA_NFC_Deactivate(TRUE);
               // We should call NFA_Deactivate, since NFA_NFC_Deactivate doesn't set the disc flags
               // Need to test thoroughly dta before submitting the change
               // NFA_Deactivate(FALSE);
            }
            /* restart discovery */
            //NFA_StartRfDiscovery();
            //startRfDiscovery(true);
            //NFC_Deactivate(NFC_DEACTIVATE_TYPE_IDLE);
            if (t1t_dyn_activated == TRUE)
            {
                t1t_dyn_activated = FALSE;
            }
            packet_count = 1;
            data_idex = 0;
        }
        #endif // </DTA> CR510341
        // If RF is activated for what we think is a Secure Element transaction
        // and it is deactivated to either IDLE or DISCOVERY mode, notify w/event.
        if ((eventData->deactivated.type == NFA_DEACTIVATE_TYPE_IDLE)
                || (eventData->deactivated.type == NFA_DEACTIVATE_TYPE_DISCOVERY))
        {
            if ((sDiscoverState == DISCOVER_STATE_POLL_P2P_ACTIVE) ||
                (sDiscoverState == DISCOVER_STATE_LISTEN_P2P_ACTIVE))
            {
                // Make sure RF field events are re-enabled
                ALOGD("%s: NFA_DEACTIVATED_EVT; is p2p", __FUNCTION__);
                // Disable RF field events in case of p2p
                UINT8  nfa_enable_rf_events[] = { 0x01 };

                if (!sIsDisabling && sIsNfaEnabled)
                {
                #ifdef DTA // <DTA>
                /*Disbale RF field info evt set in DTA mode*/
                    if (!(in_dta_mode() )){
                #endif // </!DTA>
                    ALOGD ("%s: Enabling RF field events", __FUNCTION__);
                    status = NFA_SetConfig(NCI_PARAM_ID_RF_FIELD_INFO, sizeof(nfa_enable_rf_events),
                            &nfa_enable_rf_events[0]);
                    if (status == NFA_STATUS_OK) {
                        ALOGD ("%s: Enabled RF field events", __FUNCTION__);
                    } else {
                        ALOGE ("%s: Failed to enable RF field events", __FUNCTION__);
                #ifdef DTA // <DTA>
                        }
                #endif // </DTA>
                    }
                    // Consider the field to be off at this point
                }
            }
            else if (sDiscoverState == DISCOVER_STATE_LISTEN_ACTIVE)
            {
                if (!sIsDisabling && sIsNfaEnabled)
                {
                    ApduGateManager::getInstance().notifyListenModeState (false);
                }
            }
        }
        if (sPendUiCmd == true)
        {
            sUiStateNeedCallback = true;
            sPendUiCmd = false;
            /* NFCC goes to sleep without sending response */
            status = NFA_SendVsCommand (NCI_MSG_PROP_GENERIC, // oid
                                        0x03, // cmd_params_len,
                                        last_uistate,
                                        updateHostPresenceCallback);
            if (status != NFA_STATUS_OK)
            {
                UINT8  p_param[] = {0x4F, 0x01, 0x01, 0x00 };
                ALOGE("%s: NFA_SendVsCommand failed", __FUNCTION__);
                updateHostPresenceCallback (p_param[0], sizeof(p_param), p_param);
            }
        }
        sDiscoverState = DISCOVER_STATE_DISCOVERY;
        /* notify complete of deactivation of RF interface */
        if (sNotifyDeactivatedRfInterface)
        {
            SyncEventGuard guard (sDeactivateRfInterfaceEvent);
            sDeactivateRfInterfaceEvent.notifyOne();
        }
        break;

    case NFA_TLV_DETECT_EVT: // TLV Detection complete
        status = eventData->tlv_detect.status;
        ALOGD("%s: NFA_TLV_DETECT_EVT: status = %d, protocol = %d, num_tlvs = %d, num_bytes = %d",
             __FUNCTION__, status, eventData->tlv_detect.protocol,
             eventData->tlv_detect.num_tlvs, eventData->tlv_detect.num_bytes);
        if (status != NFA_STATUS_OK)
        {
            ALOGE("%s: NFA_TLV_DETECT_EVT error: status = %d", __FUNCTION__, status);
        }
        break;

    case NFA_NDEF_DETECT_EVT: // NDEF Detection complete;
        //if status is failure, it means the tag does not contain any or valid NDEF data;
        //pass the failure status to the NFC Service;
        status = eventData->ndef_detect.status;
        #ifdef DTA // <DTA>
        if(in_dta_mode() )
        {
            if(t1t_dyn_activated == TRUE)
            {
                /*calculate the number of data packets to be written in T1T_BV4*/
                 if(eventData->ndef_detect.cur_size != 0)
                 {
                     data_packets_cnt = (eventData->ndef_detect.cur_size)/T1T_BLK_SIZE;
                     ALOGD("data_packets_cnt= %d", data_packets_cnt);
                     x = (eventData->ndef_detect.cur_size)%T1T_BLK_SIZE;
                     ALOGD("x= %d", x);
                     if(x > 0)
                     {
                         data_packets_cnt += 1;
                         ALOGD("Final data_packets_cnt= %d", data_packets_cnt);
                     }
                 }
            }
        }
        #endif // <!DTA>

        ALOGD("%s: NFA_NDEF_DETECT_EVT: status = 0x%X, protocol = %u, "
             "max_size = %lu, cur_size = %lu, flags = 0x%X", __FUNCTION__,
             status,
             eventData->ndef_detect.protocol, eventData->ndef_detect.max_size,
             eventData->ndef_detect.cur_size, eventData->ndef_detect.flags);
        NfcTag::getInstance().connectionEventHandler (connEvent, eventData);
        nativeNfcTag_doCheckNdefResult(status,
            eventData->ndef_detect.max_size, eventData->ndef_detect.cur_size,
            eventData->ndef_detect.flags);
        break;

    case NFA_DATA_EVT: // Data message received (for non-NDEF reads)
        ALOGD("%s: NFA_DATA_EVT: status = 0x%X, len = %d", __FUNCTION__, eventData->status, eventData->data.len);
        #ifdef DTA // <DTA>
        if (in_dta_mode() && (sDiscoverState == DISCOVER_STATE_LISTEN_ACTIVE)) {
          ALOGD("%s: in DTA mode", __FUNCTION__);
          // If we get transceive data in listen mode, always assume Type 4 Listen Mode DTA.
          myType4ListenLoopback(eventData->data.p_data, eventData->data.len);
          break;
        }


        if (in_dta_mode() && !in_llcp_or_snep_dta_mode() && dta::nfcdepListenLoopbackOn) {
            ALOGD("%s: in DTA mode", __FUNCTION__);
            dta::nfcDepLoopBackInListenMode(eventData->data.p_data, eventData->data.len);
        }
        #endif // </DTA>
        nativeNfcTag_doTransceiveStatus(eventData->status, eventData->data.p_data, eventData->data.len);
        break;
    case NFA_RW_INTF_ERROR_EVT:
        ALOGD("%s: NFA_RW_INTF_ERROR_EVT", __FUNCTION__);
        {
            ALOGD("%s: NFA_RW_INTF_ERROR_EVT: status = %u", __FUNCTION__, eventData->status);
#ifdef DTA // <DTA>
            if (in_dta_mode()) {
                NFA_NFC_Deactivate(TRUE);
            }
            else
#endif // </DTA>
            {
                nativeNfcTag_notifyRfTimeout();
                NFA_Deactivate(FALSE);
            }
        }
        break;
    case NFA_SELECT_CPLT_EVT: // Select completed
        status = eventData->status;
        ALOGD("%s: NFA_SELECT_CPLT_EVT: status = %d", __FUNCTION__, status);
        if (status != NFA_STATUS_OK)
        {
            ALOGE("%s: NFA_SELECT_CPLT_EVT error: status = %d", __FUNCTION__, status);
        }
        break;

    case NFA_READ_CPLT_EVT: // NDEF-read or tag-specific-read completed
        ALOGD("%s: NFA_READ_CPLT_EVT: status = 0x%X", __FUNCTION__, eventData->status);
        nativeNfcTag_doReadCompleted (eventData->status);
        NfcTag::getInstance().connectionEventHandler (connEvent, eventData);
        break;

    case NFA_WRITE_CPLT_EVT: // Write completed
        ALOGD("%s: NFA_WRITE_CPLT_EVT: status = %d", __FUNCTION__, eventData->status);
        #ifdef DTA // <DTA>
        if (in_dta_mode())
        {
            if(t1t_dyn_activated == TRUE)
            {
               sendtitdata(packet_count);
               if(packet_count !=0)
               {
                   packet_count++;
               }
            }
            else
            {
        #endif // </DTA>
                nativeNfcTag_doWriteStatus (eventData->status == NFA_STATUS_OK);
        #ifdef DTA // <DTA>
            }
        }
        else
        {
        nativeNfcTag_doWriteStatus (eventData->status == NFA_STATUS_OK);
        }
        #endif // </DTA>
        break;

    case NFA_SET_TAG_RO_EVT: // Tag set as Read only
        ALOGD("%s: NFA_SET_TAG_RO_EVT: status = %d", __FUNCTION__, eventData->status);
        nativeNfcTag_doMakeReadonlyResult(eventData->status);
        break;

    case NFA_CE_NDEF_WRITE_START_EVT: // NDEF write started
        ALOGD("%s: NFA_CE_NDEF_WRITE_START_EVT: status: %d", __FUNCTION__, eventData->status);

        if (eventData->status != NFA_STATUS_OK)
            ALOGE("%s: NFA_CE_NDEF_WRITE_START_EVT error: status = %d", __FUNCTION__, eventData->status);
        break;

    case NFA_CE_NDEF_WRITE_CPLT_EVT: // NDEF write completed
        ALOGD("%s: FA_CE_NDEF_WRITE_CPLT_EVT: len = %lu", __FUNCTION__, eventData->ndef_write_cplt.len);
        break;

    case NFA_LLCP_ACTIVATED_EVT: // LLCP link is activated
        ALOGD("%s: NFA_LLCP_ACTIVATED_EVT: is_initiator: %d  remote_wks: %d, remote_lsc: %d, remote_link_miu: %d, local_link_miu: %d",
             __FUNCTION__,
             eventData->llcp_activated.is_initiator,
             eventData->llcp_activated.remote_wks,
             eventData->llcp_activated.remote_lsc,
             eventData->llcp_activated.remote_link_miu,
             eventData->llcp_activated.local_link_miu);

        PeerToPeer::getInstance().llcpActivatedHandler (getNative(0, 0), eventData->llcp_activated);
        break;

    case NFA_LLCP_DEACTIVATED_EVT: // LLCP link is deactivated
        ALOGD("%s: NFA_LLCP_DEACTIVATED_EVT", __FUNCTION__);
        if (sDiscoverState == DISCOVER_STATE_POLL_P2P_ACTIVE)
        {
            sDiscoverState = DISCOVER_STATE_POLL_DEACTIVATING;
        }
        PeerToPeer::getInstance().llcpDeactivatedHandler (getNative(0, 0), eventData->llcp_deactivated);
        break;
    case NFA_LLCP_FIRST_PACKET_RECEIVED_EVT: // Received first packet over llcp
        ALOGD("%s: NFA_LLCP_FIRST_PACKET_RECEIVED_EVT", __FUNCTION__);
        PeerToPeer::getInstance().llcpFirstPacketHandler (getNative(0, 0));
        break;
    case NFA_PRESENCE_CHECK_EVT:
        ALOGD("%s: NFA_PRESENCE_CHECK_EVT", __FUNCTION__);
        if (eventData->status != NFA_STATUS_OK)
        {
            if (sDiscoverState == DISCOVER_STATE_POLL_ACTIVE)
            {
                sDiscoverState = DISCOVER_STATE_POLL_DEACTIVATING;
            }
        }
        nativeNfcTag_doPresenceCheckResult (eventData->status);
        break;
    case NFA_FORMAT_CPLT_EVT:
        ALOGD("%s: NFA_FORMAT_CPLT_EVT: status=0x%X", __FUNCTION__, eventData->status);
        nativeNfcTag_formatStatus (eventData->status == NFA_STATUS_OK);
        break;

    case NFA_I93_CMD_CPLT_EVT:
        ALOGD("%s: NFA_I93_CMD_CPLT_EVT: status=0x%X", __FUNCTION__, eventData->status);
        break;

    case NFA_CE_UICC_LISTEN_CONFIGURED_EVT :
        ALOGD("%s: NFA_CE_UICC_LISTEN_CONFIGURED_EVT : status=0x%X", __FUNCTION__, eventData->status);
        RoutingManager::getInstance().connectionEventHandler (connEvent, eventData);
        break;

    case NFA_SET_P2P_LISTEN_TECH_EVT:
        ALOGD("%s: NFA_SET_P2P_LISTEN_TECH_EVT", __FUNCTION__);
        PeerToPeer::getInstance().connectionEventHandler (connEvent, eventData);
        break;
    default:
        ALOGE("%s: unknown event ????", __FUNCTION__);
        break;
    }
}
#ifdef DTA // <DTA>
/* Requests to get NCI configuration parameter values, which will then be printed. */
#ifdef GETCONFIG_IMPLEMENT
static void getNciConfigurationValues()
{
        ALOGD ("[DTA] %s: enter", __FUNCTION__);

        UINT8 query_size = 20;
        tNFA_PMID requested_ids[query_size];

        ALOGD("%s: [DTA][CFG] Getting standard NCI configuration parameters", __FUNCTION__);
        // Get all standard NCI configuration parameters:
        requested_ids[0] = NFC_PMID_PA_BAILOUT;
        requested_ids[1] = NFC_PMID_PB_AFI;
        requested_ids[2] = NFC_PMID_PB_BAILOUT;
        requested_ids[3] = NFC_PMID_PB_ATTRIB_PARAM1;
        requested_ids[4] = NFC_PMID_PF_BIT_RATE;
        requested_ids[5] = NFC_PMID_PB_H_INFO;
        requested_ids[6] = NFC_PMID_BITR_NFC_DEP;
        requested_ids[7] = NFC_PMID_ATR_REQ_GEN_BYTES;
        requested_ids[8] = NFC_PMID_ATR_REQ_CONFIG;
        requested_ids[9] = NFC_PMID_LA_HIST_BY;
        requested_ids[10] = NFC_PMID_LA_NFCID1;
        requested_ids[11] = NFC_PMID_PI_BIT_RATE;
        requested_ids[12] = NFC_PMID_LA_BIT_FRAME_SDD;
        requested_ids[13] = NFC_PMID_LA_PLATFORM_CONFIG;
        requested_ids[14] = NFC_PMID_LA_SEL_INFO;
        requested_ids[15] = NFC_PMID_LI_BIT_RATE;
        requested_ids[16] = NFC_PMID_LB_SENSB_INFO;
        requested_ids[17] = NFC_PMID_LB_PROTOCOL;
        requested_ids[18] = NFC_PMID_LB_H_INFO;
        requested_ids[19] = NFC_PMID_LB_NFCID0;
        NFA_GetConfig(query_size, (tNFA_PMID*) &requested_ids);

        requested_ids[0] = NFC_PMID_TOTAL_DURATION;
        requested_ids[1] = NFC_PMID_CON_DEVICES_LIMIT;
        requested_ids[2] = NFC_PMID_LB_APPDATA;
        requested_ids[3] = NFC_PMID_LB_SFGI;
        requested_ids[4] = NFC_PMID_LB_ADC_FO;
        requested_ids[5] = NFC_PMID_LF_T3T_ID1;
        requested_ids[6] = NFC_PMID_LF_T3T_ID2;
        requested_ids[7] = NFC_PMID_LF_T3T_ID3;
        requested_ids[8] = NFC_PMID_LF_T3T_ID4;
        requested_ids[9] = NFC_PMID_LF_T3T_ID5;
        requested_ids[10] = NFC_PMID_LF_T3T_ID6;
        requested_ids[11] = NFC_PMID_LF_T3T_ID7;
        requested_ids[12] = NFC_PMID_LF_T3T_ID8;
        requested_ids[13] = NFC_PMID_LF_T3T_ID9;
        requested_ids[14] = NFC_PMID_LF_T3T_ID10;
        requested_ids[15] = NFC_PMID_LF_T3T_ID11;
        requested_ids[16] = NFC_PMID_LF_T3T_ID12;
        requested_ids[17] = NFC_PMID_LF_T3T_ID13;
        requested_ids[18] = NFC_PMID_LF_T3T_ID14;
        requested_ids[19] = NFC_PMID_LF_T3T_ID15;
        NFA_GetConfig(query_size, (tNFA_PMID*) &requested_ids);

        requested_ids[0] = NFC_PMID_LF_T3T_ID16;
        requested_ids[1] = NFC_PMID_LF_PROTOCOL;
        requested_ids[2] = NFC_PMID_LF_T3T_PMM;
        requested_ids[3] = NFC_PMID_LF_T3T_MAX;
        requested_ids[4] = NFC_PMID_LF_T3T_FLAGS2;
        requested_ids[5] = NFC_PMID_FWI;
        requested_ids[6] = NFC_PMID_LF_CON_BITR_F;
        requested_ids[7] = NFC_PMID_WT;
        requested_ids[8] = NFC_PMID_ATR_RES_GEN_BYTES;
        requested_ids[9] = NFC_PMID_ATR_RSP_CONFIG;
        requested_ids[10] = NFC_PMID_RF_FIELD_INFO;
        requested_ids[11] = NFC_PMID_NFC_DEP_OP;
        //requested_ids[12] = NFC_PARAM_ID_RF_EE_ACTION;  // These two have no actual NCI pmid
        //requested_ids[13] = NFC_PARAM_ID_ISO_DEP_OP;
        NFA_GetConfig(14, (tNFA_PMID*) &requested_ids);

        ALOGD("%s: [DTA][CFG] Getting Broadcom proprietary NCI configuration parameters", __FUNCTION__);
        // Get all Broadcom proprietary NCI parameters:
        requested_ids[0] = NCI_PARAM_ID_LA_FSDI;
        requested_ids[1] = NCI_PARAM_ID_LB_FSDI;
        requested_ids[2] = NCI_PARAM_ID_HOST_LISTEN_MASK;
        requested_ids[3] = NCI_PARAM_ID_CHIP_TYPE;
        requested_ids[4] = NCI_PARAM_ID_PA_ANTICOLL;
        requested_ids[5] = NCI_PARAM_ID_CONTINUE_MODE;
        requested_ids[6] = NCI_PARAM_ID_LBP;
        requested_ids[7] = NCI_PARAM_ID_T1T_RDR_ONLY;
        requested_ids[8] = NCI_PARAM_ID_LA_SENS_RES;
        requested_ids[9] = NCI_PARAM_ID_PWR_SETTING_BITMAP;
        requested_ids[10] = NCI_PARAM_ID_WI_NTF_ENABLE;
        requested_ids[11] = NCI_PARAM_ID_LN_BITRATE;
        requested_ids[12] = NCI_PARAM_ID_LF_BITRATE;
        requested_ids[13] = NCI_PARAM_ID_SWP_BITRATE_MASK;
        requested_ids[14] = NCI_PARAM_ID_KOVIO;
        requested_ids[15] = NCI_PARAM_ID_UICC_NTF_TO;
        requested_ids[16] = NCI_PARAM_ID_NFCDEP;
        requested_ids[17] = NCI_PARAM_ID_CLF_REGS_CFG;
        requested_ids[18] = NCI_PARAM_ID_NFCDEP_TRANS_TIME;
        requested_ids[19] = NCI_PARAM_ID_CREDIT_TIMER;
        NFA_GetConfig(query_size, (tNFA_PMID*) &requested_ids);

        requested_ids[0] = NCI_PARAM_ID_CORRUPT_RX;
        requested_ids[1] = NCI_PARAM_ID_ISODEP;
        requested_ids[2] = NCI_PARAM_ID_LF_CONFIG;
        requested_ids[3] = NCI_PARAM_ID_I93_DATARATE;
        requested_ids[4] = NCI_PARAM_ID_CREDITS_THRESHOLD;
        requested_ids[5] = NCI_PARAM_ID_TAGSNIFF_CFG;
        requested_ids[6] = NCI_PARAM_ID_PA_FSDI;
        requested_ids[7] = NCI_PARAM_ID_PB_FSDI;
        requested_ids[8] = NCI_PARAM_ID_FRAME_INTF_RETXN;
        requested_ids[9] = NCI_PARAM_ID_UICC_RDR_PRIORITY;
        requested_ids[10] = NCI_PARAM_ID_GUARD_TIME;
        requested_ids[11] = NCI_PARAM_ID_MAXTRY2ACTIVATE;
        requested_ids[12] = NCI_PARAM_ID_SWPCFG;
        requested_ids[13] = NCI_PARAM_ID_CLF_LPM_CFG;
        requested_ids[14] = NCI_PARAM_ID_DCLB;
        requested_ids[15] = NCI_PARAM_ID_ACT_ORDER;
        requested_ids[16] = NCI_PARAM_ID_DEP_DELAY_ACT;
        requested_ids[17] = NCI_PARAM_ID_DH_PARITY_CRC_CTL;
        requested_ids[18] = NCI_PARAM_ID_PREINIT_DSP_CFG;
        requested_ids[19] = NCI_PARAM_ID_FW_WORKAROUND;
        NFA_GetConfig(query_size, (tNFA_PMID*) &requested_ids);

        requested_ids[0] = NCI_PARAM_ID_RFU_CONFIG;
        requested_ids[1] = NCI_PARAM_ID_EMVCO_ENABLE;
        requested_ids[2] = NCI_PARAM_ID_ANTDRIVER_PARAM;
        requested_ids[3] = NCI_PARAM_ID_PLL325_CFG_PARAM;
        requested_ids[4] = NCI_PARAM_ID_OPNLP_ADPLL_ENABLE;
        requested_ids[5] = NCI_PARAM_ID_CONFORMANCE_MODE;
        requested_ids[6] = NCI_PARAM_ID_LPO_ON_OFF_ENABLE;
        requested_ids[7] = NCI_PARAM_ID_FORCE_VANT;
        requested_ids[8] = NCI_PARAM_ID_COEX_CONFIG;
        requested_ids[9] = NCI_PARAM_ID_INTEL_MODE;
        NFA_GetConfig(10, (tNFA_PMID*) &requested_ids);

        requested_ids[0] = NCI_PARAM_ID_AID; // NOTE: getting this fails for some reason
        NFA_GetConfig(1, (tNFA_PMID*) &requested_ids);

        //requested_ids[0] = NCI_PARAM_ID_STDCONFIG; /* dont not use this config item */
        //requested_ids[1] = NCI_PARAM_ID_PROPCFG; /* dont not use this config item */
}
#endif
#endif // </DTA>


/*******************************************************************************
**
** Function:        nfcManager_initNativeStruc
**
** Description:     Initialize variables.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nfcManager_initNativeStruc (JNIEnv* e, jobject o)
{
    ALOGD ("%s: enter", __FUNCTION__);

    nfc_jni_native_data* nat = (nfc_jni_native_data*)malloc(sizeof(struct nfc_jni_native_data));
    if (nat == NULL)
    {
        ALOGE ("%s: fail allocate native data", __FUNCTION__);
        return JNI_FALSE;
    }

    // Set sIsDeviceReset to tru to indicate in further calls that it is device reset.
    sIsDeviceReset = true;
    memset (nat, 0, sizeof(*nat));
    e->GetJavaVM(&(nat->vm));
    nat->env_version = e->GetVersion();
    nat->manager = e->NewGlobalRef(o);

    ScopedLocalRef<jclass> cls(e, e->GetObjectClass(o));
    jfieldID f = e->GetFieldID(cls.get(), "mNative", "J");
    e->SetLongField(o, f, (jlong)nat);

    /* Initialize native cached references */
    gCachedNfcManagerNotifyNdefMessageListeners = e->GetMethodID(cls.get(),
            "notifyNdefMessageListeners", "(Lcom/android/nfc/dhimpl/NativeNfcTag;)V");
    gCachedNfcManagerNotifyTransactionListeners = e->GetMethodID(cls.get(),
            "notifyTransactionListeners", "([B)V");
    gCachedNfcManagerNotifyRfIntfDeactivated = e->GetMethodID(cls.get(),
            "notifyRfInterfaceDeactivated", "()V");
    gCachedNfcManagerNotifyHciEventConnectivity = e->GetMethodID(cls.get(),
            "notifyHciEventConnectivity", "(I)V");
    gCachedNfcManagerNotifyLlcpLinkActivation = e->GetMethodID(cls.get(),
            "notifyLlcpLinkActivation", "(Lcom/android/nfc/dhimpl/NativeP2pDevice;)V");
    gCachedNfcManagerNotifyLlcpLinkDeactivated = e->GetMethodID(cls.get(),
            "notifyLlcpLinkDeactivated", "(Lcom/android/nfc/dhimpl/NativeP2pDevice;)V");
    gCachedNfcManagerNotifyLlcpFirstPacketReceived = e->GetMethodID(cls.get(),
            "notifyLlcpLinkFirstPacketReceived", "(Lcom/android/nfc/dhimpl/NativeP2pDevice;)V");

    gCachedNfcManagerNotifyHostEmuActivated = e->GetMethodID(cls.get(),
            "notifyHostEmuActivated", "()V");

    gCachedNfcManagerNotifyHostEmuData = e->GetMethodID(cls.get(),
            "notifyHostEmuData", "([B)V");

    gCachedNfcManagerNotifyHostEmuDeactivated = e->GetMethodID(cls.get(),
            "notifyHostEmuDeactivated", "()V");

    gCachedNfcManagerNotifyRfFieldActivated = e->GetMethodID(cls.get(),
            "notifyRfFieldActivated", "()V");
    gCachedNfcManagerNotifyRfFieldDeactivated = e->GetMethodID(cls.get(),
            "notifyRfFieldDeactivated", "()V");

    gCachedNfcManagerNotifyNfccInfo = e->GetMethodID(cls.get(),
            "notifyNfccInfo", "([B)V");

    gCachedNfcManagerUpdateHostCallBack = e->GetMethodID(cls.get(),
        "updateHostCallBack", "()V");

    gCachedNfcManagerUpdateResetCounter = e->GetMethodID(cls.get(),
            "notifyUpdateResetCounter", "(Z)V");

    gCachedNfcManagerNotifyRequestRestartNfc = e->GetMethodID(cls.get(),
            "notifyRequestRestartNfc", "()V");


    if (nfc_jni_cache_object(e, gNativeNfcTagClassName, &(nat->cached_NfcTag)) == -1)
    {
        ALOGE ("%s: fail cache NativeNfcTag", __FUNCTION__);
        return JNI_FALSE;
    }

    if (nfc_jni_cache_object(e, gNativeP2pDeviceClassName, &(nat->cached_P2pDevice)) == -1)
    {
        ALOGE ("%s: fail cache NativeP2pDevice", __FUNCTION__);
        return JNI_FALSE;
    }

    getNative(e,o); // cache native environment

    ALOGD ("%s: exit", __FUNCTION__);
    return JNI_TRUE;
}

#ifdef DTA // <DTA>
void print_nci_get_cfg_tlvs(UINT16 tlv_list_len, UINT8 *p_tlv_list) {

    UINT8 type, len, *p_value;
    UINT8 xx = 0;
    UINT8 num_param_fields = *p_tlv_list;
    p_tlv_list += 1;

    ALOGD ("[DTA][CFG] number of parameter fields: %d", num_param_fields);

    UINT8 count = 0;
    while (tlv_list_len - xx >= 2 && count < num_param_fields )
    {
        type    = *(p_tlv_list + xx);
        len     = *(p_tlv_list + xx + 1);
        p_value = p_tlv_list + xx + 2;

        //ALOGD  ("[DTA] type = 0x%02x", type);
        ALOGD("[DTA][CFG] %s =", nci_pmid_to_string(type));
        int y;
        for (y = 0; y < len; y++) {
            ALOGD ("[DTA][CFG] 0x%02x", (UINT8) *(p_value+y));
        }
        if (len == 0){
        ALOGD ("[DTA][CFG] <no value> ");
        }
            xx += len + 2;  /* move to next TLV */
            count++;
        }

}
#endif // </DTA>

/*************************************************************************************
**
** Function:        nfcManager_storeReasonBeforeAbort
**
** Description:     Stores the reason of abort as WATCHDOG trigger so that during
**                  next init , host will do only NCI wake and no disable pin toggle
**
** Returns:         None
**
************************************************************************************/
void nfcManager_storeReasonBeforeAbort(int reason)
{
    /* store information of watchdog trigger in file directly */
    ALOGD("%s: enter", __FUNCTION__);
    static const char shutdown_reason_file_name [] = "/data/nfc/nvstorage.bin";
    char shutdown_reason[] = { '0' };
    FILE* shutdown_reason_file = NULL;
    bool error = false;

    switch(reason)
    {
    case DEVICE_POWER_CYCLED: // 0
        ALOGD("%s: DEVICE_POWER_CYCLED", __FUNCTION__);
        shutdown_reason[0] = '0' + DEVICE_POWER_CYCLED;
        break;
    case STORE_INFO_DEBUG_ENABLE: // 2^0
        ALOGD("%s: STORE_INFO_DEBUG_ENABLE", __FUNCTION__);
        shutdown_reason[0] = '0' + STORE_INFO_DEBUG_ENABLE;
        break;
    case STORE_INFO_NFC_DISABLED: // 2^1
        ALOGD("%s: STORE_INFO_NFC_DISABLED", __FUNCTION__);
        shutdown_reason[0] = '0' + STORE_INFO_NFC_DISABLED;
        break;
    case NFCSERVICE_WATCHDOG_TIMER_EXPIRED: // 2^2
        ALOGD("%s: NFCSERVICE_WATCHDOG_TIMER_EXPIRED", __FUNCTION__);
        shutdown_reason[0] = '0' + NFCSERVICE_WATCHDOG_TIMER_EXPIRED;
        break;
    case NFCSERVICE_GIVE_UP: // 2^3
        ALOGD("%s: NFCSERVICE_GIVE_UP", __FUNCTION__);
        shutdown_reason[0] = '0' + NFCSERVICE_GIVE_UP;
        break;
    case CORE_RST_CMD_TIMEOUT_DURING_INIT: // 2^4
        ALOGD("%s: CORE_RST_CMD_TIMEOUT_DURING_INIT", __FUNCTION__);
        shutdown_reason[0] = '0' + CORE_RST_CMD_TIMEOUT_DURING_INIT;
        break;
    default:
        ALOGE("%s: unsupported shutdown reason", __FUNCTION__);
        return;
        break;
    }

    shutdown_reason_file = fopen(shutdown_reason_file_name, "wb");
    if(shutdown_reason_file != NULL)
    {
        ALOGD("%s: Write shutdown reason to file", __FUNCTION__);
        if(fwrite (shutdown_reason, sizeof(char), sizeof(shutdown_reason), shutdown_reason_file) != sizeof(shutdown_reason))
        {
            ALOGE("%s: Error in writing in %s file", __FUNCTION__,shutdown_reason_file_name);
        }
        if(fclose (shutdown_reason_file) != 0)
        {
            ALOGE("%s: Error in closing  %s file", __FUNCTION__,shutdown_reason_file_name);
        }
        shutdown_reason_file = NULL;
    }
    else
    {
        ALOGE("%s: Error in opening %s file", __FUNCTION__,shutdown_reason_file_name);
    }
    ALOGD("%s: exit", __FUNCTION__);
}

void nfcManager_updateResetCount(bool reset) {
    nfc_jni_native_data *nat = getNative(0,0);
    JNIEnv* e = NULL;

    ScopedAttach attach(nat->vm, &e);

    if (e == NULL)
    {
        ALOGE("%s: JNIEnv is null", __FUNCTION__);
        return;
    }
    jobject o = nat->manager;

    nfcManager_updateResetCount(e, o, reset);

    return;
}

void nfcManager_updateResetCount(JNIEnv* e, jobject o, bool reset) {

    nfc_jni_native_data *nat = getNative(e,o);

    if (e == NULL)
    {
        ALOGE("%s: JNIEnv is null", __FUNCTION__);
        return;
    }

    e->CallVoidMethod(o, android::gCachedNfcManagerUpdateResetCounter, (jboolean)reset);

    if (e->ExceptionCheck())
        e->ExceptionClear();

    return;
}

/*******************************************************************************
**
** Function:        nfaDeviceManagementCallback
**
** Description:     Receive device management events from stack.
**                  dmEvent: Device-management event ID.
**                  eventData: Data associated with event ID.
**
** Returns:         None
**
*******************************************************************************/
void nfaDeviceManagementCallback (UINT8 dmEvent, tNFA_DM_CBACK_DATA* eventData)
{
    UINT8 *p = NULL;
    UINT8 offsetLMRTEntry;
    ALOGD ("%s: enter; event=0x%X", __FUNCTION__, dmEvent);

    switch (dmEvent)
    {
    case NFA_DM_ENABLE_EVT: /* Result of NFA_Enable */
        {
            SyncEventGuard guard (sNfaEnableEvent);
            ALOGD ("%s: NFA_DM_ENABLE_EVT; status=0x%X",
                    __FUNCTION__, eventData->status);
            sIsNfaEnabled = eventData->status == NFA_STATUS_OK;
            sIsDisabling = false;
            sNfaEnableEvent.notifyOne ();
        }
        break;

    case NFA_DM_DISABLE_EVT: /* Result of NFA_Disable */
        {
            SyncEventGuard guard (sNfaDisableEvent);
            ALOGD ("%s: NFA_DM_DISABLE_EVT", __FUNCTION__);
            sIsNfaEnabled = false;
            sIsDisabling = false;
            sNfaDisableEvent.notifyOne ();
        }
        break;

    case NFA_DM_SET_CONFIG_EVT: //result of NFA_SetConfig
        ALOGD ("%s: NFA_DM_SET_CONFIG_EVT", __FUNCTION__);
        {
            #ifdef DTA // <DTA>
            if(in_dta_mode())
            {
                ALOGD ("%s: [DTA][CFG] NFA_DM_SET_CONFIG_EVT: set config status=%d: number of params:%d",
                        __FUNCTION__, eventData->set_config.status, eventData->set_config.num_param_id);
            }
            #endif // </DTA>
            SyncEventGuard guard (sNfaSetConfigEvent);
            sNfaSetConfigEvent.notifyOne();
        }
        break;

    case NFA_DM_GET_CONFIG_EVT: /* Result of NFA_GetConfig */
        ALOGD ("%s: NFA_DM_GET_CONFIG_EVT", __FUNCTION__);
        {
            #ifdef DTA // <DTA>
            if(in_dta_mode())
            {
                tNFA_GET_CONFIG *p_get_config = (tNFA_GET_CONFIG*) eventData;
                ALOGD ("%s: [DTA][CFG] NFA_DM_GET_CONFIG_EVT: get config status=%d:", __FUNCTION__, p_get_config->status);

                if (p_get_config->status == NFA_STATUS_OK) {
                    print_nci_get_cfg_tlvs(p_get_config->tlv_size, p_get_config->param_tlvs);
                }
            }
            #endif // </DTA>
            SyncEventGuard guard (sNfaGetConfigEvent);
            if (eventData->status == NFA_STATUS_OK &&
                    eventData->get_config.tlv_size <= sizeof(sConfig))
            {
                sCurrentConfigLen = eventData->get_config.tlv_size;
                memcpy(sConfig, eventData->get_config.param_tlvs, eventData->get_config.tlv_size);
            }
            else
            {
                ALOGE("%s: NFA_DM_GET_CONFIG failed", __FUNCTION__);
                sCurrentConfigLen = 0;
            }
            sNfaGetConfigEvent.notifyOne();
        }
        break;

    case NFA_DM_GET_ROUTING_EVT: /* NTFs in response to NFA_GetRouting */
      ALOGD("%s: NFA_DM_GET_ROUTING_EVT in JNI called", __FUNCTION__);
      {
          SyncEventGuard guard(sNfaGetLMRTEvent);
          ALOGD("%s: NFA_DM_GET_ROUTING_EVT: Event status=%d:", __FUNCTION__, eventData->status);
          tNFA_GET_ROUTING *p_get_routing = (tNFA_GET_ROUTING *)eventData;

          //If NFC_STATUS is fail, notify nfcManager_doGetLMRT
          if (eventData->status != NFA_STATUS_OK && eventData->status != NFA_STATUS_CONTINUE)
          {
              sLMRTData.numEntries = 0;
              sLMRTData.totalSize = 0;
              sNfaGetLMRTEvent.notifyOne();
          }
          //IF NFC_STATUS is OK or CONTINUE, we process the latest RF_GET_LISTEN_MODE_ROUTING_NTF
          else
          {
              //Collect Routing Table Entries from latest RF_GET_LISTEN_MODE_ROUTING_NTF
              //Increment the num of Routing Table entries by the numEntries in the last RF_GET_LISTEN_MODE_ROUTING_NTF
              sLMRTData.numEntries += p_get_routing->numEntries;

              //Then, add the routing entries from the last RF_GET_LISTEN_MODE_ROUTING_NTF
              if (sLMRTData.totalSize > 1)
                  offsetLMRTEntry = sLMRTData.totalSize;
              else
                  offsetLMRTEntry = 0;
              memcpy(&(sLMRTData.pEntries[offsetLMRTEntry]), &p_get_routing->param_tlvs, p_get_routing->tlv_size);
              sLMRTData.totalSize += p_get_routing->tlv_size;

              //Notify as Complete ONLY upon receipt of LAST RF_GET_LISTEN_MODE_ROUTING_NTF.
              if (eventData->status == NFA_STATUS_OK )
              {
                  ALOGE("%s: NFA_DM_GET_ROUTING success. Last LMRT_NTF recvd", __FUNCTION__);
                  sNfaGetLMRTEvent.notifyOne();
              }
              else
              {
                  ALOGE("%s: NFA_DM_GET_ROUTING more NTFs remaining", __FUNCTION__);
              }
          }
      }
      break;

    case NFA_DM_RF_FIELD_EVT:
        ALOGD ("%s: NFA_DM_RF_FIELD_EVT; status=0x%X; field status=%u", __FUNCTION__,
              eventData->rf_field.status, eventData->rf_field.rf_field_status);
        if ((sDiscoverState != DISCOVER_STATE_POLL_P2P_ACTIVE) &&
            (sDiscoverState != DISCOVER_STATE_LISTEN_P2P_ACTIVE))
        {
            if (eventData->rf_field.status == NFA_STATUS_OK)
            {
                struct nfc_jni_native_data *nat = getNative(NULL, NULL);
                JNIEnv* e = NULL;
                ScopedAttach attach(nat->vm, &e);
                if (e == NULL)
                {
                    ALOGE ("jni env is null");
                    return;
                }
                if (eventData->rf_field.rf_field_status == NFA_DM_RF_FIELD_ON)
                    e->CallVoidMethod (nat->manager, android::gCachedNfcManagerNotifyRfFieldActivated);
                else
                    e->CallVoidMethod (nat->manager, android::gCachedNfcManagerNotifyRfFieldDeactivated);
            }
        }
        break;

    case NFA_DM_NFCC_TRANSPORT_ERR_EVT:
    case NFA_DM_NFCC_TIMEOUT_EVT:
        {
            sIsNfcRestart = true;
            sDiscoverState = DISCOVER_STATE_IDLE;

            if (dmEvent == NFA_DM_NFCC_TIMEOUT_EVT) {
                ALOGE ("%s: NFA_DM_NFCC_TIMEOUT_EVT; abort", __FUNCTION__);
                nfcManager_storeReasonBeforeAbort(NFCSERVICE_WATCHDOG_TIMER_EXPIRED);
                nfcManager_updateResetCount(false);
            }
            else if (dmEvent == NFA_DM_NFCC_TRANSPORT_ERR_EVT) {
                sIsNfcPresent = false;
                ALOGE ("%s: NFA_DM_NFCC_TRANSPORT_ERR_EVT; abort", __FUNCTION__);
            }

            nativeNfcTag_abortWaits();
            NfcTag::getInstance().abort ();
            sAbortConnlessWait = true;
            nativeLlcpConnectionlessSocket_abortWait();
            {
                ALOGD ("%s: aborting  sNfaEnableDisablePollingEvent", __FUNCTION__);
                SyncEventGuard guard (sNfaEnableDisablePollingEvent);
                sNfaEnableDisablePollingEvent.notifyOne();
            }
            {
                ALOGD ("%s: aborting  sNfaEnableEvent", __FUNCTION__);
                SyncEventGuard guard (sNfaEnableEvent);
                sNfaEnableEvent.notifyOne();
            }
            {
                ALOGD ("%s: aborting  sNfaDisableEvent", __FUNCTION__);
                SyncEventGuard guard (sNfaDisableEvent);
                sNfaDisableEvent.notifyOne();
            }
            PowerSwitch::getInstance ().abort ();
            gActivated = false;
            sIsNfcRestart = false;
            if (dmEvent == NFA_DM_NFCC_TIMEOUT_EVT)
            {
                ALOGE ("%s: end of NFA_DM_NFCC_TIMEOUT_EVT handling", __FUNCTION__);
                struct nfc_jni_native_data *nat = getNative(NULL, NULL);
                JNIEnv* e = NULL;
                ScopedAttach attach(nat->vm, &e);

                if (e == NULL)
                {
                    ALOGE ("jni env is null");
                    if (!sIsDisabling && sIsNfaEnabled)
                    {
                        NFA_Disable(FALSE);
                        sIsDisabling = true;
                    }
                    else
                    {
                        sIsNfaEnabled = false;
                        sIsDisabling = false;
                    }
                }
                else
                {
                    e->CallVoidMethod (nat->manager, android::gCachedNfcManagerNotifyRequestRestartNfc);
                }
            }
            else if (dmEvent == NFA_DM_NFCC_TRANSPORT_ERR_EVT)
            {
                ALOGE ("%s: end of NFA_DM_NFCC_TRANSPORT_ERR_EVT", __FUNCTION__);
                if (!sIsDisabling && sIsNfaEnabled)
                {
                    NFA_Disable(FALSE);
                    sIsDisabling = true;
                }
                else
                {
                    sIsNfaEnabled = false;
                    sIsDisabling = false;
                }
            }

        }
        break;

    case NFA_DM_PWR_MODE_CHANGE_EVT:
        PowerSwitch::getInstance ().deviceManagementCallback (dmEvent, eventData);
        break;

    case NFA_DM_NFCC_INFO:
        ALOGD ("%s: NFA_DM_NFCC_INFO = %d",
               __FUNCTION__, eventData->nfcc_info.len);
        if(eventData->nfcc_info.len != 0)
        {
            p = (UINT8*)malloc(eventData->nfcc_info.len);
            if(p != NULL)
            {
                memcpy(p,eventData->nfcc_info.value,eventData->nfcc_info.len);
                NfcTag::getInstance().SendNfccInfo(getNative(0,0),
                        p,eventData->nfcc_info.len);

                int i = 0;
                int type;
                int length;
                int numTLV = p[i++];
                for (int xx = 0; (xx < numTLV) &&
                        (i < eventData->nfcc_info.len); xx++)
                {
                    type = p[i++];
                    length = p[i++];
                    if (type == HAL_NFC_TYPE_NFCC_MFG_SPECIFIC_INFO)
                    {
                        if (length == HAL_NFC_LEN_NFCC_MFG_SPECIFIC_INFO)
                        {
                            sNfcChipVersion =  p[i];
                            if (sNfcChipVersion >= 30) // if version 3.0 or later
                                sIsUiStateSupported = true;

                            ALOGD ("%s: sNfcChipVersion=%d, sIsUiStateSupported=%d",
                                   __FUNCTION__, sNfcChipVersion, sIsUiStateSupported);
                            break;
                        }
                        else
                        {
                            ALOGE ("%s: length(%d) is not for HAL_NFC_LEN_NFCC_MFG_SPECIFIC_INFO",
                                    __FUNCTION__, length);
                            break;
                        }
                    }
                    else
                    {
                        i += length;
                    }
                }

                //free acquired memory
                free(p);
            }
        }
        break;
    case NFA_DM_POLL_ACTIVE_ERR_EVT:
        ALOGD ("%s: NFA_DM_POLL_ACTIVE_ERR_EVT = %d",
               __FUNCTION__, eventData->error_evt.evt);
        if (eventData->error_evt.evt == POLL_ACTIVE_ERR_REQ_DEACT)
        {
            NFA_Deactivate (FALSE);
        }
        else if (eventData->error_evt.evt == POLL_ACTIVE_ERR_RESEND_UICMD)
        {
            UINT8 status = NFA_STATUS_OK;

            sUiStateNeedCallback = false;
            /* NFCC goes to sleep without sending response */
            status = NFA_SendVsCommand (NCI_MSG_PROP_GENERIC, // oid
                                        0x03, // cmd_params_len,
                                        last_uistate,
                                        updateHostPresenceCallback);
            if (status != NFA_STATUS_OK)
            {
                ALOGE("%s: NFA_SendVsCommand failed", __FUNCTION__);
            }
        }
        break;
    default:
        ALOGD ("%s: unhandled event", __FUNCTION__);
        break;
    }
}

/*******************************************************************************
**
** Function:        nfcManager_sendRawFrame
**
** Description:     Send a raw frame.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nfcManager_sendRawFrame (JNIEnv* e, jobject, jbyteArray data)
{
    ScopedByteArrayRO bytes(e, data);
    uint8_t* buf = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&bytes[0]));
    size_t bufLen = bytes.size();
    tNFA_STATUS status = NFA_SendRawFrame (buf, bufLen, 0);

    return (status == NFA_STATUS_OK);
}

/*******************************************************************************
**
** Function:        nfcManager_routeAid
**
** Description:     Route an AID to an EE
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nfcManager_routeAid (JNIEnv* e, jobject, jbyteArray aid, jint route,
                                     jboolean isSubSet, jboolean isSuperSet)
{
    ScopedByteArrayRO bytes(e, aid);
    uint8_t* buf = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&bytes[0]));
    size_t bufLen = bytes.size();
    bool result = RoutingManager::getInstance().addAidRouting(buf, bufLen, route, isSubSet, isSuperSet);
    return result;
}

/*******************************************************************************
**
** Function:        nfcManager_unrouteAid
**
** Description:     Remove a AID routing
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nfcManager_unrouteAid (JNIEnv* e, jobject, jbyteArray aid)
{
    ScopedByteArrayRO bytes(e, aid);
    uint8_t* buf = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&bytes[0]));
    size_t bufLen = bytes.size();
    bool result = RoutingManager::getInstance().removeAidRouting(buf, bufLen);
    return result;
}
/*******************************************************************************
**
** Function:        nfcManager_doInitNfceeIdSeMap
**
** Description:     Initialise NFCEE IDs Secure Element mapping
**                  by reading from Conf file.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         None.
**
*******************************************************************************/
static void nfcManager_doInitNfceeIdSeMap(JNIEnv* e, jobject o)
{
    int nfceeId;
    int numConfig;

    ALOGD ("%s: enter", __FUNCTION__);

    if (GetNumValue("CE_SCREEN_STATE_CONFIG", &numConfig, sizeof(numConfig)))
    {
        sEeRoutingState = numConfig;
    }
    if (GetNumValue("CE_SCREEN_STATE_CONFIG_LOAD_AT_BOOT", &numConfig, sizeof(numConfig)))
    {
        sEeRoutingReloadAtReboot = (numConfig != 0);
    }
    if (GetNumValue("DEFAULT_OFFHOST_ROUTE", &nfceeId, sizeof(nfceeId)))
    {
        //old name is DEFAULT_ACTIVE_SECURE_ELEMENT
        ALOGD ("%s: DEFAULT_OFFHOST_ROUTE = 0x%02x", __FUNCTION__, nfceeId);
        sDefaultActiveNfceeId = nfceeId;
    }
    // override mapping NFCEE ID to SE name
    if (GetNumValue("NFCEE_ID_OF_SIM1", &nfceeId, sizeof(nfceeId)))
    {
        ALOGD ("%s: NFCEE_ID_OF_SIM1 = 0x%02x", __FUNCTION__, nfceeId);
        sNfceeId2SeName[1].nfceeId = nfceeId;
    }

    if (GetNumValue("NFCEE_ID_OF_SIM2", &nfceeId, sizeof(nfceeId)))
    {
        ALOGD ("%s: NFCEE_ID_OF_SIM2 = 0x%02x", __FUNCTION__, nfceeId);
        sNfceeId2SeName[2].nfceeId = nfceeId;
    }

    if (GetNumValue("NFCEE_ID_OF_ESE1", &nfceeId, sizeof(nfceeId)))
    {
        ALOGD ("%s: NFCEE_ID_OF_ESE1 = 0x%02x", __FUNCTION__, nfceeId);
        sNfceeId2SeName[3].nfceeId = nfceeId;
    }

    if (GetNumValue("NFCEE_ID_OF_ESE2", &nfceeId, sizeof(nfceeId)))
    {
        ALOGD ("%s: NFCEE_ID_OF_ESE2 = 0x%02x", __FUNCTION__, nfceeId);
        sNfceeId2SeName[4].nfceeId = nfceeId;
    }

    if (GetNumValue("NFCEE_ID_OF_SD1", &nfceeId, sizeof(nfceeId)))
    {
        ALOGD ("%s: NFCEE_ID_OF_SD1 = 0x%02x", __FUNCTION__, nfceeId);
        sNfceeId2SeName[5].nfceeId = nfceeId;
    }

    ALOGD ("%s: exit", __FUNCTION__);
}

/*******************************************************************************
**
** Function:        nfcManager_commitRouting
**
** Description:     Sends the AID routing table to the controller
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nfcManager_commitRouting (JNIEnv* e, jobject)
{
    return RoutingManager::getInstance().commitRouting();
}

/*******************************************************************************
**
** Function:        nfcManager_doInitialize
**
** Description:     Turn on NFC.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nfcManager_doInitialize (JNIEnv* e, jobject o)
{
    UINT32 region2_enable = 0;

    ALOGD ("%s: enter; ver=%s nfa=%s NCI_VERSION=0x%02X",
        __FUNCTION__, nfca_version_string, nfa_version_string, NCI_VERSION);
    tNFA_STATUS stat = NFA_STATUS_OK;

    PowerSwitch & powerSwitch = PowerSwitch::getInstance ();

    if (sIsNfaEnabled)
    {
        ALOGD ("%s: already enabled", __FUNCTION__);
        goto TheEnd;
    }

    if(GetNumValue("REGION2_ENABLE", &region2_enable, sizeof(region2_enable)))
    {
         ALOGD ("%s: REGION2_ENABLE = 0x%02x", __FUNCTION__, region2_enable);
         if (region2_enable == 0)
             sIsRegion2Enable = false;
         else
             sIsRegion2Enable = true;
    }

    // Add delay before sending APDU after SWP link is activated
    sApduGateDelay = 0;
    if (GetNumValue("APDU_GATE_DELAY", &sApduGateDelay, sizeof(sApduGateDelay)))
    {
        ALOGD ("%s: APDU_GATE_DELAY = %dms", __FUNCTION__, sApduGateDelay);
    }

    if (config_HCE_UID_size = GetStrValue("FIXED_UID_FOR_HCE", (char*)config_HCE_UID, sizeof(config_HCE_UID)))
    {
        if ((config_HCE_UID_size == 4) || (config_HCE_UID_size == 7) || (config_HCE_UID_size == 10))
            ALOGD("%s FIXED_UID_FOR_HCE with size %x ", __FUNCTION__, config_HCE_UID_size);
        else
        {
            ALOGE("%s FIXED_UID_FOR_HCE with invalid size %x ", __FUNCTION__, config_HCE_UID_size);
            config_HCE_UID_size = 0;
        }
    }

    /* set it to false until getting NFCC version */
    sIsUiStateSupported = false;
    powerSwitch.initialize (PowerSwitch::FULL_POWER);

    {
        unsigned long num = 0;

        NfcAdaptation& theInstance = NfcAdaptation::GetInstance();
        theInstance.Initialize(); //start GKI, NCI task, NFC task

        {
            SyncEventGuard guard (sNfaEnableEvent);
            tHAL_NFC_ENTRY* halFuncEntries = theInstance.GetHalEntryFuncs ();

            NFA_Init (halFuncEntries);

            NFA_CheckDeviceResetStatus(sIsDeviceReset);
            sIsDeviceReset = false;
            stat = NFA_Enable (nfaDeviceManagementCallback, nfaConnectionCallback);
            if (stat == NFA_STATUS_OK)
            {
                num = initializeGlobalAppLogLevel ();
                CE_SetTraceLevel (num);
                LLCP_SetTraceLevel (num);
                NFC_SetTraceLevel (num);
                RW_SetTraceLevel (num);
                NFA_SetTraceLevel (num);
                NFA_P2pSetTraceLevel (num);
                sNfaEnableEvent.wait(); //wait for NFA command to finish
            }
        }

        if (stat == NFA_STATUS_OK)
        {
            //sIsNfaEnabled indicates whether stack started successfully
            if (sIsNfaEnabled)
            {
                RoutingManager::getInstance().initialize(getNative(e, o));
                ApduGateManager::getInstance().initialize(getNative(e, o));
                nativeNfcTag_registerNdefTypeHandler ();
                NfcTag::getInstance().initialize (getNative(e, o));
                PeerToPeer::getInstance().initialize ();
                PeerToPeer::getInstance().handleNfcOnOff (true);

                /////////////////////////////////////////////////////////////////////////////////
                // Add extra configuration here (work-arounds, etc.)

                struct nfc_jni_native_data *nat = getNative(e, o);

                if ( nat )
                {
                    if (GetNumValue(NAME_POLLING_TECH_MASK, &num, sizeof(num)))
                        nat->tech_mask = num;
                    else
                        nat->tech_mask = DEFAULT_TECH_MASK;
                    ALOGD ("%s: tag polling tech mask=0x%X", __FUNCTION__, nat->tech_mask);
                }

                // if this value exists, set polling interval.
                if (GetNumValue(NAME_NFA_DM_DISC_DURATION_POLL, &num, sizeof(num)))
                    nat->discovery_duration = num;
                else
                    nat->discovery_duration = DEFAULT_DISCOVERY_DURATION;

                NFA_SetRfDiscoveryDuration(nat->discovery_duration);

                // Do custom NFCA startup configuration.
                doStartupConfig();
                goto TheEnd;
            }
        }

        ALOGE ("%s: fail nfa enable; error=0x%X", __FUNCTION__, stat);
        ALOGD("%s sIsNfcPresent = %s ", __FUNCTION__, sIsNfcPresent ? "TRUE" : "FALSE");
        if(sIsNfcPresent)
        {
            //Initialization did not succeed so recovery by doing a cold reset
            nfcManager_storeReasonBeforeAbort(CORE_RST_CMD_TIMEOUT_DURING_INIT);
            abort();
        }

        if (sIsNfaEnabled)
            stat = NFA_Disable (FALSE /* ungraceful */);

        theInstance.Finalize();
    }

TheEnd:
    if (sIsNfaEnabled)
        PowerSwitch::getInstance ().setLevel (PowerSwitch::LOW_POWER);
    ALOGD ("%s: exit", __FUNCTION__);
    return sIsNfaEnabled ? JNI_TRUE : JNI_FALSE;
}


/*******************************************************************************
**
** Function:        nfcManager_enableDiscovery
**
** Description:     Start polling and listening for devices.
**                  e: JVM environment.
**                  o: Java object.
**                  technologies_mask: the bitmask of technologies for which to enable discovery
**                  enable_lptd: whether to enable low power polling (default: false)
**
** Returns:         Status of Discovery
**
*******************************************************************************/
static bool nfcManager_enableDiscovery (JNIEnv* e, jobject o, jint technologies_mask, \
    jboolean enable_lptd, jboolean reader_mode, jboolean enable_host_routing, jboolean enable_p2p, \
    jint offhost_routing, jboolean restart)
{
    tNFA_TECHNOLOGY_MASK tech_mask = DEFAULT_TECH_MASK;
    UINT32 nfc_p2p_test_mask = 0;
    struct nfc_jni_native_data *nat = getNative(e, o);

    if (technologies_mask == -1 && nat)
        tech_mask = (tNFA_TECHNOLOGY_MASK)nat->tech_mask;
    else if (technologies_mask != -1)
        tech_mask = (tNFA_TECHNOLOGY_MASK) technologies_mask;
    ALOGD ("%s: enter; tech_mask = %02x", __FUNCTION__, tech_mask);

    if (sIsNfcRestart == true)
    {
        ALOGD ("%s: restart exit", __FUNCTION__);
        return false;
    }

    if ( (sDiscoverState >= DISCOVER_STATE_DISCOVERY)&&
        (!restart))
    {
        ALOGE ("%s: already discovering", __FUNCTION__);
        return true;
    }

    tNFA_STATUS stat = NFA_STATUS_OK;

    PowerSwitch::getInstance ().setLevel (PowerSwitch::FULL_POWER);

    if (sDiscoverState >= DISCOVER_STATE_DISCOVERY) {
        // Stop RF discovery to reconfigure
        startRfDiscovery(false);
    }

    // Check polling configuration
    if (tech_mask != 0)
    {
        stopPolling_rfDiscoveryDisabled();
        //enableDisableLptd(enable_lptd);
        startPolling_rfDiscoveryDisabled(tech_mask);

        // Start P2P listening if tag polling was enabled
        ALOGD ("%s: Enable p2pListening", __FUNCTION__);

        if (enable_p2p && !sP2pEnabled) {
            sP2pEnabled = true;
            PeerToPeer::getInstance().enableP2pListening (true);
            NFA_ResumeP2p();
        } else if (!enable_p2p && sP2pEnabled) {
            sP2pEnabled = false;
            PeerToPeer::getInstance().enableP2pListening (false);
            NFA_PauseP2p();
        }

        if (reader_mode && !sReaderModeEnabled)
        {
            sReaderModeEnabled = true;
            NFA_DisableListening();
            NFA_SetRfDiscoveryDuration(READER_MODE_DISCOVERY_DURATION);
        }
        else if (!reader_mode && sReaderModeEnabled)
        {
            struct nfc_jni_native_data *nat = getNative(e, o);
            sReaderModeEnabled = false;
            NFA_EnableListening();
            NFA_SetRfDiscoveryDuration(nat->discovery_duration);
        }
    }
    else
    {
        if(GetNumValue("TEST_L_UPDATE_P2P_FEATURE", &nfc_p2p_test_mask, sizeof(nfc_p2p_test_mask)))
        {
            // For test purpose only
            if(nfc_p2p_test_mask == true)
            {
                ALOGD ("%s: Enable p2pListening for test only", __FUNCTION__);
                PeerToPeer::getInstance().enableP2pListening (true);
            }
        }

        // No technologies configured, stop polling
        stopPolling_rfDiscoveryDisabled();
    }

    // Check listen configuration
    if (enable_host_routing)
    {
        RoutingManager::getInstance().enableRoutingToHost();
        //RoutingManager::getInstance().commitRouting();
    }
    else
    {
        RoutingManager::getInstance().disableRoutingToHost();
        //RoutingManager::getInstance().commitRouting();
    }

    if (offhost_routing != -1)
    {
        RoutingManager::getInstance().enableRoutingToOffHost(offhost_routing);
    }
    else
    {
        RoutingManager::getInstance().disableRoutingToOffHost();
    }

    RoutingManager::getInstance().commitRouting();

    // Actually start discovery.
    startRfDiscovery (true);

    PowerSwitch::getInstance ().setModeOn (PowerSwitch::DISCOVERY);

    ALOGD ("%s: exit", __FUNCTION__);
    if (sDiscoverState >= DISCOVER_STATE_DISCOVERY)
    {
        return true;
    }
    else
    {
       return false;
    }

}


/*******************************************************************************
**
** Function:        nfcManager_disableDiscovery
**
** Description:     Stop polling and listening for devices.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         Status of Discovery
**
*******************************************************************************/
bool nfcManager_disableDiscovery (JNIEnv* e, jobject o)
{
    UINT32 nfc_screen_off_polling_on = 0;
    tNFA_STATUS status = NFA_STATUS_OK;
    UINT8            cmd_params_len = 0;
    UINT8            *p_cmd_params = NULL;
    UINT8            oid = 0x3;
    ALOGD ("%s: enter;", __FUNCTION__);

    if (sIsNfcRestart == true)
    {
        ALOGD ("%s in restart", __FUNCTION__);
        goto TheEnd;
    }

    GetNumValue("NFC_SCREEN_OFF_POLL_ON", &nfc_screen_off_polling_on, sizeof(nfc_screen_off_polling_on));
    if(nfc_screen_off_polling_on == 0x01)
    {
        goto TheEnd;
    }
    pn544InteropAbortNow ();

    if (sDiscoverState == DISCOVER_STATE_IDLE)
    {
#ifdef DTA // <DTA>
        if(!(in_dta_mode() ) ) {
#endif
            NFA_SendVsCommand(oid, cmd_params_len, p_cmd_params, NULL);
        goto TheEnd;
#ifdef DTA // <DTA>
       }
#endif // </DTA>
    }

    // Stop RF Discovery.
    startRfDiscovery (false);

    status = stopPolling_rfDiscoveryDisabled();

    PeerToPeer::getInstance().enableP2pListening (false);
    sP2pEnabled = false;
    //if nothing is active after this, then tell the controller to power down
    if (! PowerSwitch::getInstance ().setModeOff (PowerSwitch::DISCOVERY))
        PowerSwitch::getInstance ().setLevel (PowerSwitch::LOW_POWER);
TheEnd:
    ALOGD ("%s: exit", __FUNCTION__);
    if (sDiscoverState == DISCOVER_STATE_IDLE)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/******************************************************************************
 **
 ** Function:        sendMseVscCallback
 **
 ** Description:     Callback function for MultiSe
 **
 ** Returns:         None.
 **
 ******************************************************************************/

void sendMseVscCallback (UINT8 event, UINT16 param_len, UINT8 *p_param)
{
    UINT8 oid = (event & 0x3F);
    SyncEventGuard guard (sNfaVsCommandEvent);

    ALOGD("%s: event = 0x%x and oid=0x%x", __FUNCTION__, event,(event & 0x3F));

    if ((event & 0xC0) == NCI_RSP_BIT)
    {
        if (oid == NCI_MSG_PROP_MULTISE)
        {
            if ((param_len >= 4)&&(*(p_param + 4) == NFA_STATUS_OK))
            {
                ALOGD("%s: event = 0x%x success", __FUNCTION__, event);
            }
            else
            {
                ALOGE("%s: event = 0x%x failed", __FUNCTION__, event);
            }
            sNfaVsCommandEvent.notifyOne();
        }
    }
}
/*******************************************************************************
**
** Function:        nfcManager_mseControlcmd
**
** Description:     Send Multi SE Control Command to NFCC.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         None
**
*******************************************************************************/
static jboolean nfcManager_multiSeControlCmd(JNIEnv* e, jobject o, jbyteArray ppse_rsp, jint op_code)
{
    UINT8 cmd_data[1024];
    UINT8 *p_cmd_params = cmd_data;
    UINT16 cmd_params_len=0;
    bool rfWasEnabled = false;
    uint8_t* buf = NULL;
    size_t bufLen = 0;

    if(ppse_rsp != NULL ){
        ScopedByteArrayRO bytes(e, ppse_rsp);
        buf = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&bytes[0]));
        bufLen = bytes.size();
    }

    ALOGD ("%s: enter; bufLen:%d \t op_code:%d", __FUNCTION__,bufLen,op_code);

    if (sDiscoverState >= DISCOVER_STATE_DISCOVERY) {
        ALOGD ("%s: enter; stop rf desc",__FUNCTION__);
        rfWasEnabled = true;
        //Stop RF discovery to reconfigure
        startRfDiscovery(false);
    }


    switch(op_code)
    {
        case NCI_SET_PPSE_RSP:
            /* send PPSE response */
            ALOGD ("%s:Send PPSE rsp to NFCC", __FUNCTION__);
            cmd_data[0] = NCI_SET_PPSE_RSP;
            cmd_data[1] = bufLen;
            memcpy(&cmd_data[2],buf,bufLen);
            cmd_params_len = bufLen + 2;

            NFA_SendVsCommand(NCI_MSG_PROP_MULTISE,
                              cmd_params_len,
                              p_cmd_params,
                              sendMseVscCallback);

           sNfaVsCommandEvent.wait ();

        break;

        case NCI_CLR_PRV_PPSE_RSP:
            ALOGD ("%s:Clear previous PPSE rsp from NFCC", __FUNCTION__);
            cmd_data[0] = NCI_CLR_PRV_PPSE_RSP;

            /* Clear Prv PPSE response */
            NFA_SendVsCommand(NCI_MSG_PROP_MULTISE,
                              1,
                              p_cmd_params,
                              sendMseVscCallback);

            /* callback:wait before sending next NCI cmd */
            sNfaVsCommandEvent.wait ();

            /* send new PPSE rsp if 'ppse_rsp' is not NULL */
            if(ppse_rsp != NULL) {
                ALOGD ("%s:Send PPSE rsp to NFCC after clearing prv PPSE", __FUNCTION__);
                cmd_data[0] = NCI_SET_PPSE_RSP;
                cmd_data[1] = bufLen;
                memcpy(&cmd_data[2],buf,bufLen);
                cmd_params_len = bufLen + 2;

                NFA_SendVsCommand(NCI_MSG_PROP_MULTISE,
                                  cmd_params_len,
                                  p_cmd_params,
                                  sendMseVscCallback);

                sNfaVsCommandEvent.wait ();
            }
        break;

        default:
            ALOGD ("%s:wrong op_code type", __FUNCTION__);
    }


    if (rfWasEnabled)
    {
        ALOGD ("%s: enter; start rf desc",__FUNCTION__);
        startRfDiscovery(true);
    }

    ALOGD ("%s: exit", __FUNCTION__);
    return true;
}
/*******************************************************************************
**
** Function:        nfcManager_deactivateRfInterface
**
** Description:     deactivate RF interface if activated
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         None
**
*******************************************************************************/
void nfcManager_deactivateRfInterface (JNIEnv*, jobject)
{
    ALOGD ("%s: enter; gActivated=%d, disc state,0x%02x", __FUNCTION__, gActivated, sDiscoverState);

    if (gActivated)
    {
        sNotifyDeactivatedRfInterface = true;
        if ((sDiscoverState == DISCOVER_STATE_POLL_ACTIVE) ||
            (sDiscoverState == DISCOVER_STATE_POLL_P2P_ACTIVE) ||
            (sDiscoverState == DISCOVER_STATE_LISTEN_P2P_ACTIVE))
        {
            if (NFA_Deactivate (FALSE) == NFA_STATUS_OK)
            {
                sDeactivateRfInterfaceEvent.wait();
            }
        }

        sNotifyDeactivatedRfInterface = false;
        // wait for sleep cmd/rsp
        sDeactivateRfInterfaceEvent.wait(40);
    }
    ALOGD ("%s: exit", __FUNCTION__);
}
void enableDisableLptd (bool enable)
{
    // This method is *NOT* thread-safe. Right now
    // it is only called from the same thread so it's
    // not an issue.
    static bool sCheckedLptd = false;
    static bool sHasLptd = false;

    tNFA_STATUS stat = NFA_STATUS_OK;
    if (!sCheckedLptd)
    {
        sCheckedLptd = true;
        SyncEventGuard guard (sNfaGetConfigEvent);
        tNFA_PMID configParam[1] = {NCI_PARAM_ID_TAGSNIFF_CFG};
        stat = NFA_GetConfig(1, configParam);
        if (stat != NFA_STATUS_OK)
        {
            ALOGE("%s: NFA_GetConfig failed", __FUNCTION__);
            return;
        }
        sNfaGetConfigEvent.wait ();
        if (sCurrentConfigLen < 4 || sConfig[1] != NCI_PARAM_ID_TAGSNIFF_CFG) {
            ALOGE("%s: Config TLV length %d returned is too short", __FUNCTION__,
                    sCurrentConfigLen);
            return;
        }
        if (sConfig[3] == 0) {
            ALOGE("%s: LPTD is disabled, not enabling in current config", __FUNCTION__);
            return;
        }
        sHasLptd = true;
    }
    // Bail if we checked and didn't find any LPTD config before
    if (!sHasLptd) return;
    UINT8 enable_byte = enable ? 0x01 : 0x00;

    SyncEventGuard guard(sNfaSetConfigEvent);

    stat = NFA_SetConfig(NCI_PARAM_ID_TAGSNIFF_CFG, 1, &enable_byte);
    if (stat == NFA_STATUS_OK)
        sNfaSetConfigEvent.wait ();
    else
        ALOGE("%s: Could not configure LPTD feature", __FUNCTION__);
    return;
}

#ifndef CONFIG_UICC_IDLE_TIMEOUT_SUPPORTED
void sendVscCallback (UINT8 event, UINT16 param_len, UINT8 *p_param)
{
    UINT8 oid = (event & 0x3F);
    SyncEventGuard guard (sNfaVsCommandEvent);

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
            sNfaVsCommandEvent.notifyOne();
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
                sNfaVsCommandEvent.notifyOne(); // don't wait for NTF
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
            sNfaVsCommandEvent.notifyOne();
        }
    }
}
#endif

tNFA_STATUS setSwpInactivityTimer (bool enable, UINT8 NFCEE_ID)
{
    UINT8 cmd_params[5];
    tNFA_STATUS stat = NFA_STATUS_OK;

    ALOGD("%s: enable = %d", __FUNCTION__, enable);

    cmd_params[0] = 0x0A; // NFCEE Control Command
    cmd_params[1] = 0x01; // number of TLV
    if (enable)
        cmd_params[2] = 0x01; // type = enable P5 Timer
    else
        cmd_params[2] = 0x00; // type = disable P5 Timer

    cmd_params[3] = 0x01;     // length
    cmd_params[4] = NFCEE_ID;

    SyncEventGuard guard (sNfaVsCommandEvent);
    stat = NFA_SendVsCommand (0x01, // generic command oid
                              0x05, // cmd_params_len,
                              cmd_params,
                              sendVscCallback);

    if (stat != NFA_STATUS_OK)
    {
        ALOGE("%s: NFA_SendVsCommand failed", __FUNCTION__);
        return stat;
    }
    sNfaVsCommandEvent.wait ();
    return stat;
}

static tNFA_STATUS activateSwp (UINT8 NFCEE_ID);

static jboolean nfcManager_doActivateSwp (JNIEnv*, jobject, jbyte slot_id)
{
    if(slot_id<0 || slot_id>1)
    {
        ALOGE("%s: bad slot id %d", __FUNCTION__, slot_id);
        return false;
    }
    UINT8 NFCEE_ID = sNfceeId2SeName[1+slot_id].nfceeId;
    if(NFCEE_ID == 0xFF)
    {
        ALOGE("%s: NFCEE not initialized", __FUNCTION__);
        return false;
    }
    return (activateSwp(NFCEE_ID)==NFA_STATUS_OK);
}

static tNFA_STATUS activateSwp (UINT8 NFCEE_ID)
{
    tNFA_STATUS stat = NFA_STATUS_OK;

    // Add delay before sending APDU after SWP link is activated
    SyncEventGuard guard (sNfaVsCommandEvent);

    // Activate SWP link

    stat = NFA_SendVsCommand (0x04, // NFCEE_ACTIVATE_CMD oid
                              0x01, // cmd_params_len,
                              &NFCEE_ID,
                              sendVscCallback);

    if (stat == NFA_STATUS_OK)
    {
        stat = NFA_RegVSCback (true, sendVscCallback);
        if (stat != NFA_STATUS_OK)
        {
            ALOGE("%s: NFA_RegVSCback failed", __FUNCTION__);
            return stat;
        }
    }
    else
    {
        ALOGE("%s: NFA_SendVsCommand failed", __FUNCTION__);
        return stat;
    }

    // Wait for activation of SWP
    sNfaVsCommandEvent.wait (1000); // continue even if no NTF
    NFA_RegVSCback (false, sendVscCallback);

    // send sleep CMD to NFCC
    NFA_SendVsCommand (NCI_MSG_PROP_SLEEP, // oid, sleep cmd
                       0x00, // cmd_params_len,
                       NULL,
                       NULL);

    ALOGD("%s: Start delay for %dms", __FUNCTION__, sApduGateDelay);
    sNfaVsCommandEvent.wait (sApduGateDelay); // let add delay
    return stat;
}


void setUiccIdleTimeout (bool enable)
{
    // This method is *NOT* thread-safe. Right now
    // it is only called from the same thread so it's
    // not an issue.

#ifndef CONFIG_UICC_IDLE_TIMEOUT_SUPPORTED
    UINT8 NFCEE_ID = RoutingManager::getInstance().getDefaultOffHostRouteDestination();
    if(setSwpInactivityTimer (enable, NFCEE_ID) != NFA_STATUS_OK) return;

    if ((!enable) && (sApduGateDelay > 0))
    {
        if(activateSwp ( NFCEE_ID ) != NFA_STATUS_OK) return;
    }
    return;
#else

    tNFA_STATUS stat = NFA_STATUS_OK;
    UINT8 swp_cfg_byte0 = 0x00;
    {
        SyncEventGuard guard (sNfaVsCommandEvent);
        tNFA_PMID configParam[1] = {0xC2};
        stat = NFA_GetConfig(1, configParam);
        if (stat != NFA_STATUS_OK)
        {
            ALOGE("%s: NFA_GetConfig failed", __FUNCTION__);
            return;
        }
        sNfaVsCommandEvent.wait ();
        if (sCurrentConfigLen < 4 || sConfig[1] != 0xC2) {
            ALOGE("%s: Config TLV length %d returned is too short", __FUNCTION__,
                    sCurrentConfigLen);
            return;
        }
        swp_cfg_byte0 = sConfig[3];
    }
    SyncEventGuard guard(sNfaSetConfigEvent);
    if (enable)
        swp_cfg_byte0 |= 0x01;
    else
        swp_cfg_byte0 &= ~0x01;

    stat = NFA_SetConfig(0xC2, 1, &swp_cfg_byte0);
    if (stat == NFA_STATUS_OK)
        sNfaSetConfigEvent.wait ();
    else
        ALOGE("%s: Could not configure UICC idle timeout feature", __FUNCTION__);
    return;
#endif
}

#if 0 //relocated to NfcJniUtil
/*******************************************************************************
**
** Function         nfc_jni_cache_object_local
**
** Description      Allocates a java object and calls it's constructor
**
** Returns          -1 on failure, 0 on success
**
*******************************************************************************/
static int nfc_jni_cache_object_local (JNIEnv *e, const char *className, jobject *cachedObj)
{
    ScopedLocalRef<jclass> cls(e, e->FindClass(className));
    if(cls.get() == NULL) {
        ALOGE ("%s: find class error", __FUNCTION__);
        return -1;
    }

    jmethodID ctor = e->GetMethodID(cls.get(), "<init>", "()V");
    jobject obj = e->NewObject(cls.get(), ctor);
    if (obj == NULL) {
       ALOGE ("%s: create object error", __FUNCTION__);
       return -1;
    }

    *cachedObj = obj;
    if (*cachedObj == NULL) {
        ALOGE ("%s: global ref error", __FUNCTION__);
        return -1;
    }
    return 0;
}
#endif

/*******************************************************************************
**
** Function:        nfcManager_doCreateLlcpServiceSocket
**
** Description:     Create a new LLCP server socket.
**                  e: JVM environment.
**                  o: Java object.
**                  nSap: Service access point.
**                  sn: Service name
**                  miu: Maximum information unit.
**                  rw: Receive window size.
**                  linearBufferLength: Max buffer size.
**
** Returns:         NativeLlcpServiceSocket Java object.
**
*******************************************************************************/
static jobject nfcManager_doCreateLlcpServiceSocket (JNIEnv* e, jobject, jint nSap, jstring sn, jint miu, jint rw, jint linearBufferLength)
{
    PeerToPeer::tJNI_HANDLE jniHandle = PeerToPeer::getInstance().getNewJniHandle ();

    ScopedUtfChars serviceName(e, sn);

    ALOGD ("%s: enter: sap=%i; name=%s; miu=%i; rw=%i; buffLen=%i", __FUNCTION__, nSap, serviceName.c_str(), miu, rw, linearBufferLength);

    /* Create new NativeLlcpServiceSocket object */
    jobject serviceSocket = NULL;
    if (nfc_jni_cache_object_local(e, gNativeLlcpServiceSocketClassName, &(serviceSocket)) == -1)
    {
        ALOGE ("%s: Llcp socket object creation error", __FUNCTION__);
        return NULL;
    }

    /* Get NativeLlcpServiceSocket class object */
    ScopedLocalRef<jclass> clsNativeLlcpServiceSocket(e, e->GetObjectClass(serviceSocket));
    if (e->ExceptionCheck())
    {
        e->ExceptionClear();
        ALOGE("%s: Llcp Socket get object class error", __FUNCTION__);
        return NULL;
    }
    if (serviceName.c_str() == NULL) {
        ALOGE("%s: Unable to read service name", __FUNCTION__);
        return NULL;
    }
    if (!PeerToPeer::getInstance().registerServer (jniHandle, serviceName.c_str()))
    {
        ALOGE("%s: RegisterServer error", __FUNCTION__);
        return NULL;
    }

    jfieldID f;

    /* Set socket handle to be the same as the NfaHandle*/
    f = e->GetFieldID(clsNativeLlcpServiceSocket.get(), "mHandle", "I");
    e->SetIntField(serviceSocket, f, (jint) jniHandle);
    ALOGD ("%s: socket Handle = 0x%X", __FUNCTION__, jniHandle);

    /* Set socket linear buffer length */
    f = e->GetFieldID(clsNativeLlcpServiceSocket.get(), "mLocalLinearBufferLength", "I");
    e->SetIntField(serviceSocket, f,(jint)linearBufferLength);
    ALOGD ("%s: buffer length = %d", __FUNCTION__, linearBufferLength);

    /* Set socket MIU */
    f = e->GetFieldID(clsNativeLlcpServiceSocket.get(), "mLocalMiu", "I");
    e->SetIntField(serviceSocket, f,(jint)miu);
    ALOGD ("%s: MIU = %d", __FUNCTION__, miu);

    /* Set socket RW */
    f = e->GetFieldID(clsNativeLlcpServiceSocket.get(), "mLocalRw", "I");
    e->SetIntField(serviceSocket, f,(jint)rw);
    ALOGD ("%s:  RW = %d", __FUNCTION__, rw);

    sLastError = 0;
    ALOGD ("%s: exit", __FUNCTION__);
    return serviceSocket;
}


/*******************************************************************************
**
** Function:        nfcManager_doGetLastError
**
** Description:     Get the last error code.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         Last error code.
**
*******************************************************************************/
static jint nfcManager_doGetLastError(JNIEnv*, jobject)
{
    ALOGD ("%s: last error=%i", __FUNCTION__, sLastError);
    return sLastError;
}


/*******************************************************************************
**
** Function:        nfcManager_doDeinitialize
**
** Description:     Turn off NFC.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nfcManager_doDeinitialize (JNIEnv* e, jobject o)
{
    ALOGD ("%s: enter", __FUNCTION__);

    sIsDisabling = true;
    pn544InteropAbortNow ();
    PowerSwitch::getInstance ().initialize (PowerSwitch::UNKNOWN_LEVEL);

    nfcManager_updateResetCount(e,o,true);

    if (sIsNfaEnabled)
    {
        SyncEventGuard guard (sNfaDisableEvent);
        tNFA_STATUS stat;
        if ((sShutdownReason == 0) && //NFC_DISABLED_BY_SYSTEM = 0
            (sIsRegion2Enable))
        {
            // do not disable discovery because CE mode in switch off may be enabled
            stat = NFA_Disable (FALSE /* ungraceful */);
        }
        else
        {
            stat = NFA_Disable (TRUE /* graceful */);
        }

        if (stat == NFA_STATUS_OK)
        {
            ALOGD ("%s: wait for completion", __FUNCTION__);
            sNfaDisableEvent.wait (); //wait for NFA command to finish
            PeerToPeer::getInstance ().handleNfcOnOff (false);
        }
        else
        {
            ALOGE ("%s: fail disable; error=0x%X", __FUNCTION__, stat);
        }
    }
    nativeNfcTag_abortWaits();
    NfcTag::getInstance().abort ();
    sAbortConnlessWait = true;
    nativeLlcpConnectionlessSocket_abortWait();
    sIsNfaEnabled = false;
    sDiscoverState = DISCOVER_STATE_IDLE;
    sIsDisabling = false;
    gActivated = false;

    {
        //unblock NFA_EnablePolling() and NFA_DisablePolling()
        SyncEventGuard guard (sNfaEnableDisablePollingEvent);
        sNfaEnableDisablePollingEvent.notifyOne ();
    }

    NfcAdaptation& theInstance = NfcAdaptation::GetInstance();
    theInstance.Finalize();

    ALOGD ("%s: exit", __FUNCTION__);
    return JNI_TRUE;
}


/*******************************************************************************
**
** Function:        nfcManager_doCreateLlcpSocket
**
** Description:     Create a LLCP connection-oriented socket.
**                  e: JVM environment.
**                  o: Java object.
**                  nSap: Service access point.
**                  miu: Maximum information unit.
**                  rw: Receive window size.
**                  linearBufferLength: Max buffer size.
**
** Returns:         NativeLlcpSocket Java object.
**
*******************************************************************************/
static jobject nfcManager_doCreateLlcpSocket (JNIEnv* e, jobject, jint nSap, jint miu, jint rw, jint linearBufferLength)
{
    ALOGD ("%s: enter; sap=%d; miu=%d; rw=%d; buffer len=%d", __FUNCTION__, nSap, miu, rw, linearBufferLength);

    PeerToPeer::tJNI_HANDLE jniHandle = PeerToPeer::getInstance().getNewJniHandle ();
    PeerToPeer::getInstance().createClient (jniHandle, miu, rw);

    /* Create new NativeLlcpSocket object */
    jobject clientSocket = NULL;
    if (nfc_jni_cache_object_local(e, gNativeLlcpSocketClassName, &(clientSocket)) == -1)
    {
        ALOGE ("%s: fail Llcp socket creation", __FUNCTION__);
        return clientSocket;
    }

    /* Get NativeConnectionless class object */
    ScopedLocalRef<jclass> clsNativeLlcpSocket(e, e->GetObjectClass(clientSocket));
    if (e->ExceptionCheck())
    {
        e->ExceptionClear();
        ALOGE ("%s: fail get class object", __FUNCTION__);
        return clientSocket;
    }

    jfieldID f;

    /* Set socket SAP */
    f = e->GetFieldID (clsNativeLlcpSocket.get(), "mSap", "I");
    e->SetIntField (clientSocket, f, (jint) nSap);

    /* Set socket handle */
    f = e->GetFieldID (clsNativeLlcpSocket.get(), "mHandle", "I");
    e->SetIntField (clientSocket, f, (jint) jniHandle);

    /* Set socket MIU */
    f = e->GetFieldID (clsNativeLlcpSocket.get(), "mLocalMiu", "I");
    e->SetIntField (clientSocket, f, (jint) miu);

    /* Set socket RW */
    f = e->GetFieldID (clsNativeLlcpSocket.get(), "mLocalRw", "I");
    e->SetIntField (clientSocket, f, (jint) rw);

    ALOGD ("%s: exit", __FUNCTION__);
    return clientSocket;
}


/*******************************************************************************
**
** Function:        nfcManager_doCreateLlcpConnectionlessSocket
**
** Description:     Create a connection-less socket.
**                  e: JVM environment.
**                  o: Java object.
**                  nSap: Service access point.
**                  sn: Service name.
**
** Returns:         NativeLlcpConnectionlessSocket Java object.
**
*******************************************************************************/
static jobject nfcManager_doCreateLlcpConnectionlessSocket (JNIEnv *, jobject, jint nSap, jstring /*sn*/)
{
    ALOGD ("%s: nSap=0x%X", __FUNCTION__, nSap);
    return NULL;
}

/*******************************************************************************
**
** Function:        nfcManager_getSeNameOfNfceeId
**
** Description:     Get Secure Element Name from NFCEE ID
**
** Returns:         Secure Element Name
**
*******************************************************************************/
static char *nfcManager_getSeNameOfNfceeId (int nfceeId)
{
    for (int i = 0; i < SIZE_OF_NFCEE_ID_TO_SE_NAME; i++) {
        if (sNfceeId2SeName[i].nfceeId == nfceeId) {
            return sNfceeId2SeName[i].seName;
        }
    }
    ALOGE ("%s: cannot find SE Name for NFCEE ID = 0x%x", __FUNCTION__, nfceeId);
    return NULL;
}

/*******************************************************************************
**
** Function:        nfcManager_getNfceeIdOfSeName
**
** Description:     Get NFCEE ID from Secure Element Name
**
** Returns:         NFCEE ID
**
*******************************************************************************/
static int nfcManager_getNfceeIdOfSeName (const char *seName)
{
    if (seName == NULL)
        return -1;

    if (!strcmp(seName, "SIM - UICC")) // same as "SIM1"
    {
        if (sNfceeId2SeName[1].nfceeId == 0xFF)
            return -1;
        else
            return sNfceeId2SeName[1].nfceeId;
    }

    for (int i = 0; i < SIZE_OF_NFCEE_ID_TO_SE_NAME; i++) {
        if (!strcmp(sNfceeId2SeName[i].seName, seName)) {
            if (sNfceeId2SeName[i].nfceeId == 0xFF)
                return -1;
            else
                return sNfceeId2SeName[i].nfceeId;
        }
    }
    ALOGE ("%s: cannot find NFCEE ID for SE Name:%s", __FUNCTION__, seName);
    return -1;
}

/*******************************************************************************
**
** Function:        nfcManager_doGetSecureElementList
**
** Description:     Get a list of secure element names.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         List of secure element names with ','.
**
*******************************************************************************/
static jstring nfcManager_doGetSecureElementList(JNIEnv* e, jobject)
{
    char buffer[100];
    char *seName;
    int  numNfcee, nfceeId[5];
    ALOGD ("%s enter", __FUNCTION__);

    numNfcee = RoutingManager::getInstance().getSecureElementIdList (nfceeId);
    if (numNfcee == 0)
    {
        ALOGD ("%s exit; No secure element", __FUNCTION__);
        return NULL;
    }

    buffer[0] = 0;
    for (int i = 0; i < numNfcee; ++i) {
        seName = nfcManager_getSeNameOfNfceeId (nfceeId[i]);
        if (seName != NULL) {
            if (strlen(buffer) > 0) {
                strlcat (buffer, ",", sizeof(buffer));
            }
            strlcat (buffer, seName, sizeof(buffer));
        }
    }
    ALOGD ("%s exit; seNameList = %s", __FUNCTION__, buffer);
    return e->NewStringUTF(buffer);
}

/*******************************************************************************
**
** Function:        nfcManager_doGetSecureElementName
**
** Description:     get Secure Element Name of NFCEE ID
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         Secure Element Name
**
*******************************************************************************/
static jstring nfcManager_doGetSecureElementName(JNIEnv* e, jobject, jint nfceeId)
{
    return e->NewStringUTF(nfcManager_getSeNameOfNfceeId (nfceeId));
}

/*******************************************************************************
**
** Function:        nfcManager_doGetNfceeId
**
** Description:     get NFCEE ID of Secure Element Name
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         Secure Element Name
**
*******************************************************************************/
static jint nfcManager_doGetNfceeId(JNIEnv* e, jobject, jstring seName)
{
    ScopedUtfChars activeSEName(e, seName);
    return nfcManager_getNfceeIdOfSeName (activeSEName.c_str());
}

/*******************************************************************************
**
** Function:        nfcManager_doGetDefaultActiveSecureElement
**
** Description:     get Secure Element Name for default active set by configuration
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         Secure Element Name
**
*******************************************************************************/
static jstring nfcManager_doGetDefaultActiveSecureElement(JNIEnv* e, jobject)
{
    return e->NewStringUTF(nfcManager_getSeNameOfNfceeId (sDefaultActiveNfceeId));
}

/*******************************************************************************
**
** Function:        nfcManager_doIsSwitchOffCeModeEnabled
**
** Description:     Card Emulation mode is enabled or not in switch off mode
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         true if enabled
**
*******************************************************************************/
static jboolean nfcManager_doIsSwitchOffCeModeEnabled(JNIEnv* e, jobject)
{
    return sIsRegion2Enable;
}

/*******************************************************************************
**
** Function:        nfcManager_isMultiSeEnabled
**
** Description:     Multi SE feature is enabled or not
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         false if not enabled
**
*******************************************************************************/
static jboolean nfcManager_isMultiSeEnabled(JNIEnv* e, jobject)
{
    int isMultiSeEnable = 0;

    if (GetNumValue("MULTI_SE_FEATURE", &isMultiSeEnable, sizeof(isMultiSeEnable)))
    {
        ALOGD ("%s: MULTI_SE_FEATURE = 0x%x", __FUNCTION__, isMultiSeEnable);
        if (isMultiSeEnable == 1)
            return true;
    }
    return false;
}

/*******************************************************************************
**
** Function:        nfcManager_doIsUiStateSupported
**
** Description:     UI State update is supported in NFCC
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         true if supported
**
*******************************************************************************/
static jboolean nfcManager_doIsUiStateSupported(JNIEnv* e, jobject)
{
    return sIsUiStateSupported;
}

/*******************************************************************************
**
** Function:        nfcManager_enableRoutingToHost
**
** Description:     NFC controller starts routing data to host.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         None
**
*******************************************************************************/
static void nfcManager_enableRoutingToHost(JNIEnv*, jobject)
{
    ALOGD ("%s: enter", __FUNCTION__);

    if (sIsNfcRestart == true)
    {
        ALOGD ("%s in restart: exit", __FUNCTION__);
        return;
    }

    PowerSwitch::getInstance ().setLevel (PowerSwitch::FULL_POWER);
    PowerSwitch::getInstance ().setModeOn (PowerSwitch::HOST_ROUTING);
    if (sDiscoverState >= DISCOVER_STATE_DISCOVERY) {
        // Stop RF discovery to reconfigure
        startRfDiscovery(false);
    }

    // if UID for HCE is configured
    if (config_HCE_UID_size)
    {
        // read UID for UICC and store
        {
            SyncEventGuard guard(sNfaGetConfigEvent);
            tNFA_PMID configParam[1] = { NFC_PMID_LA_NFCID1 };
            tNFA_STATUS stat = NFA_GetConfig(1, configParam);
            if (stat != NFA_STATUS_OK)
            {
                ALOGE("%s: NFA_GetConfig failed", __FUNCTION__);
            }
            else
            {
                sNfaGetConfigEvent.wait();

                UINT8 uid_type, uid_len, *p_value;
                UINT8 num_param_fields = sConfig[0];

                ALOGD("%s: Number of parameter fields in here: %d", __FUNCTION__, num_param_fields);

                uid_type = sConfig[1];
                uid_len = sConfig[2];
                p_value = &sConfig[3];
                ALOGD("%s:The type %s = ", __FUNCTION__, nci_pmid_to_string(uid_type));
                if (uid_len == 0){
                  ALOGD("%s: <no value> ", __FUNCTION__);
                }

                //There is NO NFCEE, so the UID is random. Do not save this UID- just skip to setting HCE UID
                if ((uid_len == 4 && p_value[0] == 0x08) ||
                    ((uid_len != 4) && (uid_len != 7) && (uid_len != 10)))
                {
                    ALOGE("%s: Not Saving this UID since Config TLV length %d returned is invalid OR UID is random", __FUNCTION__, uid_len);
                    saved_UID_size = 0;
                }
                //NFCEE exists and we store this value
                else
                {
                    ALOGD("%s: NFCEE Exists here , so we store UID of length: %d", __FUNCTION__, uid_len);
                    saved_UID_size = uid_len;
                    for (UINT16 i = 0; i < uid_len; ++i)
                    {
                        saved_UID[i] = p_value[i];
                    }
                }
            }
        }
        {
            SyncEventGuard guard(sNfaSetConfigEvent);
            ALOGD("%s: On HCE enable, setting LA_NFCID1 params ", __FUNCTION__);
            tNFA_STATUS stat = NFA_SetConfig(NFC_PMID_LA_NFCID1, config_HCE_UID_size, &config_HCE_UID[0]);
            if (stat == NFA_STATUS_OK)
                sNfaSetConfigEvent.wait();
        }
    }
    RoutingManager::getInstance().enableRoutingToHost();
    RoutingManager::getInstance().commitRouting();
    startRfDiscovery(true);
    ALOGD ("%s: exit", __FUNCTION__);
}

/*******************************************************************************
**
** Function:        nfcManager_disableRoutingToHost
**
** Description:     NFC controller stops routing data to host.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         None
**
*******************************************************************************/
static void nfcManager_disableRoutingToHost(JNIEnv*, jobject)
{
    ALOGD ("%s: enter", __FUNCTION__);
    bool rfWasEnabled = false;

    if (sIsNfcRestart == true)
    {
        ALOGD ("%s in restart", __FUNCTION__);
        goto TheEnd;
    }

    if (PowerSwitch::getInstance ().getLevel() == PowerSwitch::LOW_POWER)
    {
        ALOGD ("%s: no need to disable routing while power is OFF", __FUNCTION__);
        goto TheEnd;
    }

    if (sDiscoverState >= DISCOVER_STATE_DISCOVERY) {
        rfWasEnabled = true;
        // Stop RF discovery to reconfigure
        startRfDiscovery(false);
    }

    // if UID was set for HCE
    if (config_HCE_UID_size)
    {
        SyncEventGuard guard (sNfaSetConfigEvent);
        ALOGD ("%s: On HCE disable, resetting NCIPARAM_LA_NFCID1 params based on non-zero saved UID size %d", __FUNCTION__, saved_UID_size);
        tNFA_STATUS stat = NFA_SetConfig(NFC_PMID_LA_NFCID1, saved_UID_size, &saved_UID[0]);
        if (stat == NFA_STATUS_OK)
            sNfaSetConfigEvent.wait();
    }

    RoutingManager::getInstance().disableRoutingToHost();
    RoutingManager::getInstance().commitRouting();
    if (rfWasEnabled)
    {
        startRfDiscovery(true);
    }
    if (! PowerSwitch::getInstance ().setModeOff (PowerSwitch::HOST_ROUTING))
        PowerSwitch::getInstance ().setLevel (PowerSwitch::LOW_POWER);
TheEnd:
    ALOGD ("%s: exit", __FUNCTION__);
}

#if 0 //deprecated
/*******************************************************************************
**
** Function:        nfcManager_doSelectSecureElement
**
** Description:     NFC controller starts routing data in listen mode.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         JNI_TRUE if successful, JNI_FALSE, otherwise
**
*******************************************************************************/
static jboolean nfcManager_doSelectSecureElement(JNIEnv* e, jobject, jstring activeSE)
{
    int nfceeId;
    bool stat = true;

    if (activeSE == NULL)
    {
        ALOGE ("%s: Active SE Name is not provided", __FUNCTION__);
        return JNI_FALSE;
    }

    ScopedUtfChars activeSEName(e, activeSE);
    ALOGD ("%s: %s enter", __FUNCTION__, activeSEName.c_str());

    nfceeId = nfcManager_getNfceeIdOfSeName (activeSEName.c_str());
    if (nfceeId < 0)
    {
        ALOGD ("%s: cannot find NFCEE ID for %s", __FUNCTION__, activeSEName.c_str());
        stat = false;
        goto TheEnd;
    }

    if (sIsSecElemSelected)
    {
        int activeNfceeId = SecureElement::getInstance().getActiveNfceeId();
        if (activeNfceeId == nfceeId)
        {
            ALOGD ("%s: already selected", __FUNCTION__);
            stat = true;
            goto TheEnd;
        }
    }

    PowerSwitch::getInstance ().setLevel (PowerSwitch::FULL_POWER);

    if (sRfEnabled) {
        // Stop RF Discovery if we were polling
        startRfDiscovery (false);
    }

    stat = SecureElement::getInstance().activate (nfceeId);
    if (stat) {
        sIsSecElemSelected = true;
        SecureElement::getInstance().addRouteToSecureElement ();
    } else {
        ALOGD ("%s: failed to activate", __FUNCTION__);
    }

    startRfDiscovery (true);
    PowerSwitch::getInstance ().setModeOn (PowerSwitch::SE_ROUTING);
TheEnd:
    ALOGD ("%s: exit", __FUNCTION__);

    if (stat)
        return JNI_TRUE;
    else
        return JNI_FALSE;
}


/*******************************************************************************
**
** Function:        nfcManager_doDeselectSecureElement
**
** Description:     NFC controller stops routing data in listen mode.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         None
**
*******************************************************************************/
static void nfcManager_doDeselectSecureElement(JNIEnv*, jobject)
{
    ALOGD ("%s: enter", __FUNCTION__);
    bool bRestartDiscovery = false;

    if (! sIsSecElemSelected)
    {
        ALOGE ("%s: already deselected", __FUNCTION__);
        goto TheEnd2;
    }

    if (PowerSwitch::getInstance ().getLevel() == PowerSwitch::LOW_POWER)
    {
        ALOGD ("%s: do not deselect while power is OFF", __FUNCTION__);
        sIsSecElemSelected = false;
        goto TheEnd;
    }

    if (sRfEnabled) {
        // Stop RF Discovery if we were polling
        startRfDiscovery (false);
        bRestartDiscovery = true;
    }

    SecureElement::getInstance().removeRouteToSecureElement ();

    //if controller is not routing to sec elems AND there is no pipe connected,
    //then turn off the sec elems
    if (SecureElement::getInstance().isBusy() == false)
        SecureElement::getInstance().deactivate (0xABCDEF);

    sIsSecElemSelected = false;
TheEnd:
    if (bRestartDiscovery)
        startRfDiscovery (true);

    //if nothing is active after this, then tell the controller to power down
    if (! PowerSwitch::getInstance ().setModeOff (PowerSwitch::SE_ROUTING))
        PowerSwitch::getInstance ().setLevel (PowerSwitch::LOW_POWER);

TheEnd2:
    ALOGD ("%s: exit", __FUNCTION__);
}
#endif //deprecated

/*******************************************************************************
**
** Function:        isPeerToPeer
**
** Description:     Whether the activation data indicates the peer supports NFC-DEP.
**                  activated: Activation data.
**
** Returns:         True if the peer supports NFC-DEP.
**
*******************************************************************************/
static bool isPeerToPeer (tNFA_ACTIVATED& activated)
{
    #ifdef DTA // <DTA>
    bool isP2p = activated.activate_ntf.protocol == NFA_PROTOCOL_NFC_DEP;

    if (isP2p) {
        ALOGD("%s: P2P CONNECTION ACTIVATED", __FUNCTION__);
    } else {
        ALOGD("%s: Non-p2p connection activated", __FUNCTION__);
    }

    if (in_dta_mode()) {
        ALOGD("%s: in DTA mode", __FUNCTION__);
        dta::nfcdepListenLoopbackOn = isP2p && isListenMode(activated);
        if (dta::nfcdepListenLoopbackOn) {

            /* Get Wait Time */
            struct {
                UINT16 link_miu;
                UINT8  opt;
                UINT8  wt;
                UINT16 link_timeout;
                UINT16 inact_timeout_init;
                UINT16 inact_timeout_target;
                UINT16 symm_delay;
                UINT16 data_link_timeout;
                UINT16 delay_first_pdu_timeout;
            } llcp_conf;

            NFA_P2pGetLLCPConfig (&llcp_conf.link_miu,
                &llcp_conf.opt,
                &llcp_conf.wt,
                &llcp_conf.link_timeout,
                &llcp_conf.inact_timeout_init,
                &llcp_conf.inact_timeout_target,
                &llcp_conf.symm_delay,
                &llcp_conf.data_link_timeout,
                &llcp_conf.delay_first_pdu_timeout);

            ALOGD("%s: P2P LOOPBACK IN LISTEN-MODE ACTIVATED", __FUNCTION__);
            dta::setResponseWaitTime(llcp_conf.wt);
        }
    }

    return isP2p;
    #else
    return activated.activate_ntf.protocol == NFA_PROTOCOL_NFC_DEP;
    #endif // </DTA>
}

/*******************************************************************************
**
** Function:        isListenMode
**
** Description:     Indicates whether the activation data indicates it is
**                  listen mode.
**
** Returns:         True if this listen mode.
**
*******************************************************************************/
static bool isListenMode(tNFA_ACTIVATED& activated)
{
    return ((NFC_INTERFACE_EE_DIRECT_RF == activated.activate_ntf.intf_param.type)
            || (NFC_DISCOVERY_TYPE_LISTEN_A == activated.activate_ntf.rf_tech_param.mode)
            || (NFC_DISCOVERY_TYPE_LISTEN_B == activated.activate_ntf.rf_tech_param.mode)
            || (NFC_DISCOVERY_TYPE_LISTEN_F == activated.activate_ntf.rf_tech_param.mode)
            || (NFC_DISCOVERY_TYPE_LISTEN_A_ACTIVE == activated.activate_ntf.rf_tech_param.mode)
            || (NFC_DISCOVERY_TYPE_LISTEN_F_ACTIVE == activated.activate_ntf.rf_tech_param.mode)
            || (NFC_DISCOVERY_TYPE_LISTEN_ISO15693 == activated.activate_ntf.rf_tech_param.mode)
            || (NFC_DISCOVERY_TYPE_LISTEN_B_PRIME == activated.activate_ntf.rf_tech_param.mode));
}

/*******************************************************************************
**
** Function:        nfcManager_doCheckLlcp
**
** Description:     Not used.
**
** Returns:         True
**
*******************************************************************************/
static jboolean nfcManager_doCheckLlcp(JNIEnv*, jobject)
{
    ALOGD("%s", __FUNCTION__);
    return JNI_TRUE;
}


/*******************************************************************************
**
** Function:        nfcManager_doActivateLlcp
**
** Description:     Not used.
**
** Returns:         True
**
*******************************************************************************/
static jboolean nfcManager_doActivateLlcp(JNIEnv*, jobject)
{
    ALOGD("%s", __FUNCTION__);
    return JNI_TRUE;
}


/*******************************************************************************
**
** Function:        nfcManager_doAbortCount
**
** Description:     Not used.
**
** Returns:         None
**
*******************************************************************************/
static void nfcManager_doAbortCount(JNIEnv* e, jobject o, int count)
{
    ALOGE("%s: JNI abort(): %d", __FUNCTION__, count);

    nfcManager_updateResetCount(e,o, false);
    if(count < 2)
    {
    nfcManager_storeReasonBeforeAbort(NFCSERVICE_WATCHDOG_TIMER_EXPIRED);
    }
    else if(count == 2)
    {
        nfcManager_storeReasonBeforeAbort(DEVICE_POWER_CYCLED);
    }
    else
    {
        nfcManager_storeReasonBeforeAbort(NFCSERVICE_GIVE_UP);
    }
    abort();
}


/*******************************************************************************
**
** Function:        nfcManager_doDownload
**
** Description:     Download firmware patch files.  Do not turn on NFC.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nfcManager_doDownload(JNIEnv*, jobject)
{
    ALOGD ("%s: enter", __FUNCTION__);
#ifdef JNI_FIRMWARE_DOWNLOAD
    NfcAdaptation& theInstance = NfcAdaptation::GetInstance();

    theInstance.Initialize(); //start GKI, NCI task, NFC task
    theInstance.DownloadFirmware ();
    theInstance.Finalize();
#endif
    ALOGD ("%s: exit", __FUNCTION__);
    return JNI_TRUE;
}


/*******************************************************************************
**
** Function:        nfcManager_doResetTimeouts
**
** Description:     Not used.
**
** Returns:         None
**
*******************************************************************************/
static void nfcManager_doResetTimeouts(JNIEnv*, jobject)
{
    ALOGD ("%s", __FUNCTION__);
    NfcTag::getInstance().resetAllTransceiveTimeouts ();
}


/*******************************************************************************
**
** Function:        nfcManager_doSetTimeout
**
** Description:     Set timeout value.
**                  e: JVM environment.
**                  o: Java object.
**                  tech: technology ID.
**                  timeout: Timeout value.
**
** Returns:         True if ok.
**
*******************************************************************************/
static bool nfcManager_doSetTimeout(JNIEnv*, jobject, jint tech, jint timeout)
{
    if (timeout <= 0)
    {
        ALOGE("%s: Timeout must be positive.",__FUNCTION__);
        return false;
    }
    ALOGD ("%s: tech=%d, timeout=%d", __FUNCTION__, tech, timeout);
    NfcTag::getInstance().setTransceiveTimeout (tech, timeout);
    return true;
}


/*******************************************************************************
**
** Function:        nfcManager_doGetTimeout
**
** Description:     Get timeout value.
**                  e: JVM environment.
**                  o: Java object.
**                  tech: technology ID.
**
** Returns:         Timeout value.
**
*******************************************************************************/
static jint nfcManager_doGetTimeout(JNIEnv*, jobject, jint tech)
{
    int timeout = NfcTag::getInstance().getTransceiveTimeout (tech);
    ALOGD ("%s: tech=%d, timeout=%d", __FUNCTION__, tech, timeout);
    return timeout;
}

/*******************************************************************************
**
** Function:        nfcManager_updateHostPresence
**
** Description:     update UI state of connectivity gate
**                  e: JVM environment.
**                  o: Java object.
**                  uiState: UI State defined by HCI
**                  nfccUiState: UI State for NFCC
**
** Returns:         void
**
*******************************************************************************/
static void nfcManager_updateHostPresence(JNIEnv*, jobject, jint hciUiState, jint nfccUiState)
{
    tNFA_STATUS status = NFA_STATUS_OK;
    bool  bfailcase = false;
    UINT8 cmd_params[3];

    ALOGD ("%s: hciUiState=%d, nfccUiState=%d,disc state,0x%02x,poll lock,%d", __FUNCTION__, hciUiState, nfccUiState, sDiscoverState, sPollingLocked);
    if (sIsNfcRestart == true)
    {
        bfailcase = true;
    }
    else
    {
        cmd_params[0] = NCI_MSG_PROP_GENERIC_SUBCMD_UI_STATE;
        cmd_params[1] = (UINT8)hciUiState;
        cmd_params[2] = (UINT8)nfccUiState;
        memset(last_uistate, 0x00, sizeof(last_uistate));
        memcpy(last_uistate, cmd_params, sizeof(cmd_params));

        if ((sDiscoverState == DISCOVER_STATE_POLL_DEACTIVATING) ||
            (sDiscoverState == DISCOVER_STATE_POLL_ACTIVE) ||
            (sDiscoverState == DISCOVER_STATE_POLL_P2P_ACTIVE))
        {
            if (nfccUiState == NCI_PROP_NFCC_UI_STATE_UNLOCKED)
            {
                sPendUiCmd = true;
            }
            else
            {
                if (sPollingLocked == true)
                {
                    if (nfccUiState == NCI_PROP_NFCC_UI_STATE_OFF)
                    {
                        sPendUiCmd = true;
                    }
                }
                else
                {
                    sPendUiCmd = true;
                }
            }
        }

        if (sPendUiCmd == false)
        {
            /* NFCC goes to sleep without sending response */
            status = NFA_SendVsCommand (NCI_MSG_PROP_GENERIC, // oid
                                        0x03, // cmd_params_len,
                                        cmd_params,
                                        updateHostPresenceCallback);

            if (status != NFA_STATUS_OK)
            {
                ALOGE("%s: NFA_SendVsCommand failed", __FUNCTION__);
                bfailcase = true;
            }
        }
    }

    if (bfailcase == true)
    {
        UINT8  p_param[] = {0x4F, 0x01, 0x01, 0x00 };
        ALOGD ("%s fake ui rsp", __FUNCTION__);
        updateHostPresenceCallback (p_param[0], sizeof(p_param), p_param);

    }
    return;

}

/*******************************************************************************
**
** Function:        nfcManager_updateLockScreenPollingMode
**
** Description:     Decide whether NFCC should poll in lock screen or not
**                  e: JVM environment.
**                  o: Java object.
**                  enable = true:  poll in lock screen
**                  enable = false: Do not poll in lock screen
**
** Returns:         void
**
*******************************************************************************/
bool nfcManager_updateLockScreenPollingMode(JNIEnv* e, jobject * o, bool enable)
{
    ALOGD("%s: Enter, enable: %d", __FUNCTION__, enable);

    tNFA_STATUS stat = NFA_STATUS_FAILED;
    tNFA_PMID configParam[1] = {NCI_PARAM_CON_DISCOVERY_PARAM};
    const UINT8 pollScreenLocked = 0x40;
    UINT8 fakecmd[3];


    UINT8 params[1] = {0};
    UINT8 currentValue = 0;

    SyncEventGuard getGuard (sNfaGetConfigEvent);
    stat = NFA_GetConfig(sizeof(configParam), configParam);

    if (stat != NFA_STATUS_OK)
    {
        ALOGE("%s: NFA_GetConfig failed", __FUNCTION__);
        sPollingLocked = false;
        return sPollingLocked;
    }
    sNfaGetConfigEvent.wait ();

    if (sConfig[1] !=  NCI_PARAM_CON_DISCOVERY_PARAM || sCurrentConfigLen < 5)
    {
        ALOGE("%s: Got unexpected TLVs 0x%02x with length %d", __FUNCTION__,
                sConfig[1], sCurrentConfigLen);
        sPollingLocked = false;
        return sPollingLocked;
    }

    currentValue = sConfig[3];

    if(enable)
        params[0] = currentValue | pollScreenLocked;
    else
        params[0] = currentValue & ~pollScreenLocked;

    if(currentValue != params[0]) {

        SyncEventGuard setGuard (sNfaSetConfigEvent);

        stat = NFA_SetConfig(NCI_PARAM_CON_DISCOVERY_PARAM, sizeof(params), params);

        if(stat == NFA_STATUS_OK)
        {
            ALOGD("%s: changed NCI_PARAM_CON_DISCOVERY_PARAM to: 0x%02x", __FUNCTION__, params[0]);
            sNfaSetConfigEvent.wait();
        }
        else
        {
            ALOGE("%s: NFA_SetConfig failed", __FUNCTION__);
            sPollingLocked = false;
            return sPollingLocked;
        }

        fakecmd[0] = NCI_MSG_PROP_GENERIC_HAL_CMD;
        fakecmd[1] = NCI_PROP_HAL_POLLING_IN_LOCKED;
        if(enable)
        {
            fakecmd[2] = NCI_PROP_HAL_ENABLE_POLL;
        }
        else
        {
            fakecmd[2] = NCI_PROP_HAL_DISABLE_POLL;
        }

        stat  = NFA_SendVsCommand (NCI_MSG_PROP_GENERIC, 0x03,fakecmd,NULL);

        if (stat != NFA_STATUS_OK)
        {
            ALOGE("%s: NFA_SendVsCommand failed", __FUNCTION__);
        }
    }
    else
    {
        ALOGD("%s: no update required, NCI_PARAM_CON_DISCOVERY_PARAM: 0x%02x", __FUNCTION__, params[0]);
    }

    sPollingLocked = enable;

    ALOGD("%s: Exit, sPollingLocked: %d", __FUNCTION__, sPollingLocked);
    return sPollingLocked;
}

/*******************************************************************************
**
** Function:        updateHostPresenceCallback
**
** Description:     callback for updateHostPresence
**
** Returns:         void
**
*******************************************************************************/
void updateHostPresenceCallback (UINT8 event, UINT16 param_len, UINT8 *p_param)
{
    /*
     * event is (NCI_RSP_BIT|oid)
     * NCI response:
     * 4f010100
     */
    const UINT8 oid_mask = 0x3F;
    const UINT8 rsp_mask = 0xC0;
    const UINT8 oid_01 = 0x01;
    const int status_byte_index = 3;

    UINT8 oid = (event & oid_mask); // mask out oid

    if (sUiStateNeedCallback == true)
    {
        if((event & rsp_mask) == NCI_RSP_BIT) // mask out NCI_RSP_BIT
        {
            if(oid == oid_01)
            {
                if((param_len >= (UINT16)status_byte_index) && (*(p_param + status_byte_index) == NFA_STATUS_OK))
                {
                    JNIEnv* e = NULL;
                    ScopedAttach attach(getNative(0,0)->vm, &e);
                    if (e == NULL)
                    {
                        ALOGE("%s: jni env is null", __FUNCTION__);
                        return;
                    }
                    else
                    {
                        e->CallVoidMethod(getNative(0,0)->manager, android::gCachedNfcManagerUpdateHostCallBack);
                        if (e->ExceptionCheck())
                            e->ExceptionClear();
                    }
                }
            }
        }
    }
    else
    {
        ALOGD("%s: alreay announced", __FUNCTION__);
        sUiStateNeedCallback = true;
    }
    return;
}

/*******************************************************************************
**
** Function:        nfcManager_notifyRfIntfDeactivated
**
** Description:     notify RF interface is deactivated
**
** Returns:         void
**
*******************************************************************************/
void nfcManager_notifyRfIntfDeactivated ()
{
    JNIEnv* e = NULL;
    ScopedAttach attach(getNative(0,0)->vm, &e);
    if (e == NULL)
    {
        ALOGE("%s: jni env is null", __FUNCTION__);
        return;
    }
    else
    {
        e->CallVoidMethod(getNative(0,0)->manager, android::gCachedNfcManagerNotifyRfIntfDeactivated);

        if (e->ExceptionCheck())
            e->ExceptionClear();
    }
}

/*******************************************************************************
**
** Function:        nfcManager_doNotifyApduGateRfIntfDeactivated
**
** Description:     notify ApduGateManager that RF interface has been deactivated
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         void
**
*******************************************************************************/
static void nfcManager_doNotifyApduGateRfIntfDeactivated(JNIEnv*, jobject)
{
    ApduGateManager::getInstance().notifyRfIntfDeactivated ();
}


#ifdef DTA // <DTA>
/*******************************************************************************
**
** Function:        nfcManager_doNfcDeactivate
**
** Description:     Deactivate NFC target with selected command.
**                  e: JVM environment.
**                  o: Java object.
**                  deactivationType: 1 = NFC-DEP DSL request
**                                    2 = NFC-DEP RLS request
**                                    3 = General deactivation to sleep mode
**                                    4 = General deactivation
**
** Returns:         Whether the command is succesfully passed to lower level.
**
*******************************************************************************/
static jboolean nfcManager_doNfcDeactivate(JNIEnv *e, jobject o, jint deactivationType)
{
    ALOGD ("%s: deactivationType=%d", __FUNCTION__, deactivationType);

    tNFA_STATUS stat = NFA_STATUS_FAILED;

    switch (deactivationType)
    {
    case gNfcDepDslDeactivation:
    {
        // false = Sleep mode, DSL request
        stat = NFA_NFC_Deactivate(false);
        break;
    }
    case gNfcDepRlsDeactivation:
    {
        // true = Full deactivation, RLS request
        stat = NFA_NFC_Deactivate(true);
        break;
    }
    case gNfaDeactivationToSleep:
    {
        // sleep_mode = true
        stat = NFA_Deactivate(true);
        break;
    }
    case gNfaDeactivation:
    {
        // sleep_mode = false
        stat = NFA_Deactivate(false);
        break;
    }
    default:
    {
        break;
    }
    }

    if (stat == NFA_STATUS_OK) {
        // Command sent but most likely not finished yet
        return true;
    }
    else {
        ALOGE ("%s: failed to deactivate.", __FUNCTION__);
        return false;
    }
}
#endif // </DTA>
/*******************************************************************************
**
** Function:        nfcManager_doDump
**
** Description:     Not used.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         Text dump.
**
*******************************************************************************/
static jstring nfcManager_doDump(JNIEnv* e, jobject)
{
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "libnfc llc error_count=%u", /*libnfc_llc_error_count*/ 0);
    return e->NewStringUTF(buffer);
}


/*******************************************************************************
**
** Function:        nfcManager_doSetP2pInitiatorModes
**
** Description:     Set P2P initiator's activation modes.
**                  e: JVM environment.
**                  o: Java object.
**                  modes: Active and/or passive modes.  The values are specified
**                          in external/libnfc-nxp/inc/phNfcTypes.h.  See
**                          enum phNfc_eP2PMode_t.
**
** Returns:         None.
**
*******************************************************************************/
static void nfcManager_doSetP2pInitiatorModes (JNIEnv *e, jobject o, jint modes)
{
    ALOGD ("%s: modes=0x%X", __FUNCTION__, modes);
    struct nfc_jni_native_data *nat = getNative(e, o);

    tNFA_TECHNOLOGY_MASK mask = 0;
    if (modes & 0x01) mask |= NFA_TECHNOLOGY_MASK_A;
    if (modes & 0x02) mask |= NFA_TECHNOLOGY_MASK_F;
    if (modes & 0x04) mask |= NFA_TECHNOLOGY_MASK_F;
    if (modes & 0x08) mask |= NFA_TECHNOLOGY_MASK_A_ACTIVE;
    if (modes & 0x10) mask |= NFA_TECHNOLOGY_MASK_F_ACTIVE;
    if (modes & 0x20) mask |= NFA_TECHNOLOGY_MASK_F_ACTIVE;
    nat->tech_mask = mask;
}


/*******************************************************************************
**
** Function:        nfcManager_doSetP2pTargetModes
**
** Description:     Set P2P target's activation modes.
**                  e: JVM environment.
**                  o: Java object.
**                  modes: Active and/or passive modes.
**
** Returns:         None.
**
*******************************************************************************/
static void nfcManager_doSetP2pTargetModes (JNIEnv*, jobject, jint modes)
{
    ALOGD ("%s: modes=0x%X", __FUNCTION__, modes);
    // Map in the right modes
    tNFA_TECHNOLOGY_MASK mask = 0;
    if (modes & 0x01) mask |= NFA_TECHNOLOGY_MASK_A;
    if (modes & 0x02) mask |= NFA_TECHNOLOGY_MASK_F;
    if (modes & 0x04) mask |= NFA_TECHNOLOGY_MASK_F;
    if (modes & 0x08) mask |= NFA_TECHNOLOGY_MASK_A_ACTIVE | NFA_TECHNOLOGY_MASK_F_ACTIVE;

    PeerToPeer::getInstance().setP2pListenMask(mask);
}

static void nfcManager_doEnableScreenOffSuspend(JNIEnv* e, jobject o)
{
    PowerSwitch::getInstance().setScreenOffPowerState(PowerSwitch::POWER_STATE_FULL);
}

static void nfcManager_doDisableScreenOffSuspend(JNIEnv* e, jobject o)
{
    PowerSwitch::getInstance().setScreenOffPowerState(PowerSwitch::POWER_STATE_OFF);
}
/********************************************************************************************************
**
** Function:        nfcManager_doReportReason
**
** Description:     Gets reason of Shutdown from Java layer(Shutdown due to NFC disabled by User or not)
**                  e: JVM environment.
**                  o: Java object.
**                  shutdownReason: if 1 - NFC disabled by User otherwise 0.
**
** Returns:         None.
**
******************************************************************************************************/
static void nfcManager_doReportReason(JNIEnv *e, jobject o, jint shutdownReason)
{
    ALOGD ("%s: shutdownReason=%d", __FUNCTION__, shutdownReason);
    sShutdownReason = shutdownReason;
    NFA_StoreShutdownReason (shutdownReason);
}

/*******************************************************************************
**
** Function:        nfcManager_doGetEeRoutingState
**
** Description:     Get EeRouting State value from the conf file.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         EeRouting State.
**
*******************************************************************************/
static jint nfcManager_doGetEeRoutingState(JNIEnv*, jobject)
{
    return sEeRoutingState;
}


/*******************************************************************************
**
** Function:        nfcManager_doGetEeRoutingReloadAtReboot
**
** Description:     Use conf file over prefs file at boot.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         pick routing state from conf file at NFC Service load
**
*******************************************************************************/
static jboolean nfcManager_doGetEeRoutingReloadAtReboot(JNIEnv*, jobject)
{
    return sEeRoutingReloadAtReboot;
}

/*******************************************************************************
**
** Function:        nfcManager_doGetLMRT
**
** Description:     JNI API responsible to get Listen Mode routing Table
**                  e: JVM environment.
**                  o: Java object.
**
**
** Returns:         On Success or Failure the jbyteArray buffer with Routing Table data.
**                  Table is reported in : NFA_DM_GET_ROUTING_EVT.
*******************************************************************************/
static jbyteArray nfcManager_doGetLMRT(JNIEnv *e, jobject o)
{

    ALOGE("%s: Enter", __FUNCTION__);

    //Initialize to zero each time the GetLMRTd is sent.
    sLMRTData.numEntries = 0;
    sLMRTData.totalSize = 0;

    SyncEventGuard guard(sNfaGetLMRTEvent);

    tNFA_STATUS stat = NFA_GetRouting();
    if (stat == NFA_STATUS_OK)
    {
        UINT16 maxLmrtSize = NFA_EeGetMaxLmrtSize();
        ALOGD("%s: Maximum LMRT size indicated by NFCC is =%d", __FUNCTION__, maxLmrtSize);
        //Dynamically allocate buffer based on maxLMRTsize specified by NFCC
        sLMRTData.pEntries = new UINT8[maxLmrtSize];

        //Wait for READ_LISTEN_MODE_ROUTING_NTF reported via NFA_DM_GET_ROUTING_EVT
        sNfaGetLMRTEvent.wait();
    }
    else
    {
        ALOGE("%s: Could not get Listen Mode routing table", __FUNCTION__);
        return NULL;
    }

    ALOGD("%s: total LMRT Size=%d", __FUNCTION__, sLMRTData.totalSize);
    ALOGD("%s: total Number of LMRT Entries=%d", __FUNCTION__, sLMRTData.numEntries);

    //Converting data from Native to Java format
    jbyteArray result = e->NewByteArray(sLMRTData.totalSize + 1);
    e->SetByteArrayRegion(result, (jsize)0, (jsize)1, (jbyte *)&(sLMRTData.numEntries));
    if (sLMRTData.totalSize > 0)
    {
        jbyte *pEntries = (jbyte *) &(sLMRTData.pEntries[0]);
        e->SetByteArrayRegion(result, (jsize)1, (jsize)sLMRTData.totalSize, (jbyte *)pEntries);
    }
    delete[] sLMRTData.pEntries;
    sLMRTData.pEntries = NULL;

    ALOGE("%s: Exit", __FUNCTION__);
    return result;
}

/*******************************************************************************
**
** Function:        nfcManager_doSetDefaultRoute
**
** Description:     Set default ISO-DEP protocol route
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nfcManager_doSetDefaultRoute (JNIEnv* e, jobject, jint nfceeId)
{
    bool rfWasEnabled = false;
    bool result;

    result = RoutingManager::getInstance().setDefaultRoute(nfceeId);
    if (result)
    {
        if (sDiscoverState >= DISCOVER_STATE_DISCOVERY) {
            rfWasEnabled = true;
            startRfDiscovery(false);
        }

        RoutingManager::getInstance().commitRouting();

        if (rfWasEnabled)
            startRfDiscovery (true);
    }
    return result;
}

/*******************************************************************************
**
** Function:        nfcManager_doGetDefaultRoute
**
** Description:     Get default ISO-DEP protocol route
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         NFCEE ID
**
*******************************************************************************/
static jint nfcManager_doGetDefaultRoute (JNIEnv* e, jobject)
{
    return RoutingManager::getInstance().getDefaultRoute();
}

/*******************************************************************************
**
** Function:        nfcManager_doIsExchangingApduWithEse
**
** Description:     get status of exchanging APDU with eSE
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         true if exchanging APDU with eSE
**
*******************************************************************************/
static jboolean nfcManager_doIsExchangingApduWithEse(JNIEnv*, jobject)
{
    return ApduGateManager::getInstance().isExchangingApduWithEse();
}

/*******************************************************************************
**
** Function:        nfcManager_doIsRfInterfaceActivated
**
** Description:     get status of RF interface activation
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         true if RF interface is activated
**
*******************************************************************************/
static jboolean nfcManager_doIsRfInterfaceActivated(JNIEnv*, jobject)
{
    return (gActivated || ApduGateManager::getInstance().isListenModeActivated());
}

/*******************************************************************************
**
** Function:        aidFilterVscCallback
**
** Description:     callback for AID filter
**
** Returns:         void
**
*******************************************************************************/
void aidFilterVscCallback (UINT8 event, UINT16 param_len, UINT8 *p_param)
{
    UINT8 oid = (event & 0x3F);
    SyncEventGuard guard (sNfaVsCommandEvent);

    if ((event & 0xC0) == NCI_RSP_BIT)
    {
        if (oid == 0x01)
        {
            if ((param_len >= 4)&&(*(p_param + 3) == NFA_STATUS_OK))
            {
                ALOGD("%s: event = 0x%x success", __FUNCTION__, event);
            }
            else
            {
                ALOGE("%s: event = 0x%x failed", __FUNCTION__, event);
            }
            sNfaVsCommandEvent.notifyOne();
        }
    }
}

/*******************************************************************************
**
** Function:        nfcManager_doInitClfAidFilterList
**
** Description:     Initialize AID FilterList to NFCC
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nfcManager_doInitClfAidFilterList (JNIEnv* e, jobject )
{
    static UINT8 cmdBuf[75] = { 0x13, // sub_cmd
                                0x05, // number of entries
                                // entry#1
                                0x05, // length of AID
                                0xA0, 0x00, 0x00, 0x00, 0x03, // AID
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // mask
                                // entry#2
                                0x05, // length of AID
                                0xA0, 0x00, 0x00, 0x00, 0x04, // AID
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // mask
                                // entry#3
                                0x05, // length of AID
                                0xA0, 0x00, 0x00, 0x00, 0x25, // AID
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // mask
                                // entry#4
                                0x05, // length of AID
                                0xA0, 0x00, 0x00, 0x00, 0x24, // AID
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // mask
                                // entry#5
                                0x0E, // length of AID
                                0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, // AID
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF // mask
                              };
    tNFA_STATUS status;

    ALOGD("%s enter", __FUNCTION__);

    SyncEventGuard guard (sNfaVsCommandEvent);

    status = NFA_SendVsCommand (0x01,           //OID
                                sizeof(cmdBuf), //cmd_params_len
                                cmdBuf,         //p_cmd_params
                                aidFilterVscCallback);

    if (status == NFA_STATUS_OK)
    {
        sNfaVsCommandEvent.wait();
        ALOGD("%s exit: success to send VSC", __FUNCTION__);
        return true;
    }
    else
    {
        ALOGE("%s exit: fail to send VSC", __FUNCTION__);
        return false;
    }
}

/*******************************************************************************
**
** Function:        nfcManager_doSetClfAidFilterList
**
** Description:     Set AID FilterList to NFCC
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nfcManager_doSetClfAidFilterList (JNIEnv* e, jobject, jbyteArray filterList)
{
    ScopedByteArrayRO bytes(e, filterList);
    uint8_t* buf = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&bytes[0]));
    size_t bufLen = bytes.size();
    UINT8 *cmdBuf = NULL;
    UINT8 numEnteries = 0;
    int lengthOfCmdBuf = 2;  //sub_cmd and number of entries
    tNFA_STATUS status;

    ALOGD("%s enter: length = %d", __FUNCTION__, bufLen);

    if (bufLen == 0)
    {
        //clear filter list
        cmdBuf = (UINT8 *)malloc(lengthOfCmdBuf);
        if (cmdBuf == NULL)
        {
            ALOGE("%s exit: fail to malloc", __FUNCTION__);
            return false;
        }
    }
    else
    {
        cmdBuf = (UINT8 *)malloc(bufLen);
        if (cmdBuf == NULL)
        {
            ALOGE("%s exit: fail to malloc", __FUNCTION__);
            return false;
        }

        int indexOfClfFilterDo = 0;
        int lengthOfAidRange;
        int xx;
        bool parseError = false;
        while ((parseError == false) &&
               (indexOfClfFilterDo + buf[indexOfClfFilterDo + 1] + 2 <= bufLen)) //length includes only payload
        {
            if ((buf[indexOfClfFilterDo] == 0xFE) &&   //CLF-FILTER-DO
                (buf[indexOfClfFilterDo+2] == 0xA1) && //FILTER-ENTRY
                (buf[indexOfClfFilterDo+4] == 0xC2))   //AID-RANGE
            {
                lengthOfAidRange = buf[indexOfClfFilterDo + 5];
                cmdBuf[lengthOfCmdBuf++] = lengthOfAidRange;
                // copy AID
                for (xx = 0; xx < lengthOfAidRange; xx++)
                {
                    cmdBuf[lengthOfCmdBuf++] = buf[indexOfClfFilterDo + 6 + xx];
                }
                // copy MASK without Tag(0xC3) and length
                for (xx = 0; xx < lengthOfAidRange; xx++)
                {
                    cmdBuf[lengthOfCmdBuf++] = buf[indexOfClfFilterDo + 6 + lengthOfAidRange + 2 + xx];
                }

                numEnteries++;
                // move to next CLF-FILTER-DO
                indexOfClfFilterDo += buf[indexOfClfFilterDo + 1] + 2;
            }
            else
            {
                ALOGE("%s: parse error indexOfClfFilterDo = %d", __FUNCTION__, indexOfClfFilterDo);
                parseError = true;
            }
        }

        if ((parseError == false)&&
            (indexOfClfFilterDo != bufLen)) // if more or less data
        {
            parseError = true;
        }

        if (parseError)
        {
            // parse error
            ALOGE("%s exit: fail to parse", __FUNCTION__);
            if (cmdBuf) free(cmdBuf);
            return false;
        }
    }

    SyncEventGuard guard (sNfaVsCommandEvent);

    cmdBuf[0] = 0x13; // sub_cmd
    cmdBuf[1] = numEnteries; // number of entries
    status = NFA_SendVsCommand (0x01,           //OID
                                lengthOfCmdBuf, //cmd_params_len
                                cmdBuf,         //p_cmd_params
                                aidFilterVscCallback);

    if (status == NFA_STATUS_OK)
    {
        sNfaVsCommandEvent.wait();
        ALOGD("%s exit: success to send VSC", __FUNCTION__);
        return true;
    }
    else
    {
        ALOGE("%s exit: fail to send VSC", __FUNCTION__);
        return false;
    }
}

/*******************************************************************************
**
** Function:        nfcManager_doEnableDisableClfAidFilterCondition
**
** Description:     Enable or disable AID Filter Condition in NFCC
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nfcManager_doEnableDisableClfAidFilterCondition (JNIEnv*, jobject, jboolean enable, jbyte filterConditionTag)
{
    UINT8 cmdBuf[2];
    tNFA_STATUS status;

    ALOGD("%s: enter enable = %d, filterConditionTag = 0x%02x", __FUNCTION__, enable, filterConditionTag);

    SyncEventGuard guard (sNfaVsCommandEvent);

    cmdBuf[0] = 0x14; // sub_cmd
    cmdBuf[1] = (enable ? 1 : 0);
    status = NFA_SendVsCommand (0x01,   //OID
                                2,      //cmd_params_len
                                cmdBuf, //p_cmd_params
                                aidFilterVscCallback);

    if (status == NFA_STATUS_OK)
    {
        sNfaVsCommandEvent.wait();
        ALOGD("%s exit: success to send VSC", __FUNCTION__);
        return true;
    }
    else
    {
        ALOGE("%s exit: fail to send VSC", __FUNCTION__);
        return false;
    }
}


#ifdef DTA // <DTA>
/**
 * Type 4 DTA Loopback handling.
 */
static void myType4ListenLoopback(uint8_t* p_buf, uint32_t len)
{
    tNFA_STATUS sendStatus = 0;
    static const uint8_t select_dta_capdu[] = {0x00, 0xA4, 0x04, 0x00, 0x0E,
              0x31, 0x4E, 0x46, 0x43, 0x2E, 0x53,
              0x59, 0x53, 0x2E, 0x44, 0x44, 0x46,
              0x30, 0x31,
              0x00};
    static uint8_t select_dta_rapdu[] = {0x01, 0x00, 0x90, 0x00};

    // DTA 6.4.2.2 - data received from IUT
    if (0 == memcmp(p_buf, select_dta_capdu, len)) {
        ALOGD("LOOPBACK - Responding to SELECT DTA.");

        // C-APDU equals SELECT DTA, proceed to symbol 3
        sendStatus = NFA_SendRawFrame (select_dta_rapdu, sizeof(select_dta_rapdu), NFA_DM_DEFAULT_PRESENCE_CHECK_START_DELAY );
    } else {
        // Symbol 4: Convert received C-APDU into the R-APDU
        // C-APDU format: '80 EE 00 00' + Lc + Data_C + '00'
        uint8_t Data_C_len = 0;
        uint8_t rapdu[252];

        ALOGD("Starting LOOPBACK for Type 4 Listen.");

        // Min len for C-APDU is 6 bytes (Lc = 0)
        if (len < 6) {
            ALOGD("LOOPBACK - Received C-APDU is too short: got %d bytes.", len);
            return;
}

        Data_C_len = p_buf[4];

        // Max len for Data_C is 250 bytes as defined in DTA
        if (Data_C_len > 250) {
            ALOGD("LOOPBACK - Detected Data_C len in  C-APDU is too long: %d bytes.", Data_C_len);
            return;
}

        // OK, update the R-APDU.
        memcpy(rapdu, &(p_buf[5]), Data_C_len);
        // End R-APDU with STATUS OK.
        rapdu[Data_C_len] = 0x90;
        rapdu[Data_C_len + 1] = 0x00;

        sendStatus = NFA_SendRawFrame (rapdu, Data_C_len + 2, NFA_DM_DEFAULT_PRESENCE_CHECK_START_DELAY );
    }

    if (sendStatus == NFA_STATUS_OK) {
        ALOGD ("%s: [DTA] T4 LISTEN-LOOPBACK NFA_SendRawFrame(), retVal=NFA_STATUS_OK", __FUNCTION__);
    }
    else {
        ALOGD ("%s: [DTA] T4 LISTEN-LOOPBACK NFA_SendRawFrame(), retVal=NFA_STATUS_FAILED", __FUNCTION__);
    }

}
#endif // </DTA>

/*****************************************************************************
**
** JNI functions for android-4.0.1_r1
**
*****************************************************************************/
static JNINativeMethod gMethods[] =
{
    {"doDownload", "()Z",
            (void *)nfcManager_doDownload},

    {"initializeNativeStructure", "()Z",
            (void*) nfcManager_initNativeStruc},

    {"doInitNfceeIdSeMap", "()V",
            (void*) nfcManager_doInitNfceeIdSeMap},

    {"doInitialize", "()Z",
            (void*) nfcManager_doInitialize},

    {"doDeinitialize", "()Z",
            (void*) nfcManager_doDeinitialize},

    {"multiSeControlCmd", "([BI)Z",
            (void *)nfcManager_multiSeControlCmd},
    {"isMultiSeEnabled", "()Z",
            (void *)nfcManager_isMultiSeEnabled},
    {"sendRawFrame", "([B)Z",
            (void*) nfcManager_sendRawFrame},

    {"routeAid", "([BIZZ)Z",
            (void*) nfcManager_routeAid},

    {"unrouteAid", "([B)Z",
            (void*) nfcManager_unrouteAid},

    {"commitRouting", "()Z",
            (void*) nfcManager_commitRouting},

    {"doEnableDiscovery", "(IZZZZIZ)Z",
            (void*) nfcManager_enableDiscovery},

    {"enableRoutingToHost", "()V",
            (void*) nfcManager_enableRoutingToHost},

    {"disableRoutingToHost", "()V",
            (void*) nfcManager_disableRoutingToHost},

    {"doIsUiStateSupported", "()Z",
            (void *)nfcManager_doIsUiStateSupported},

#if 0 //deprecated
    {"doIsSwitchOffCeModeEnabled", "()Z",
            (void *)nfcManager_doIsSwitchOffCeModeEnabled},
#endif

    { "doActivateSwp", "(B)Z",
            (void *)nfcManager_doActivateSwp},

    {"doGetSecureElementList", "()Ljava/lang/String;",
            (void *)nfcManager_doGetSecureElementList},

    {"doGetSecureElementName", "(I)Ljava/lang/String;",
            (void *)nfcManager_doGetSecureElementName},

    {"doGetNfceeId", "(Ljava/lang/String;)I",
            (void *)nfcManager_doGetNfceeId},

#if 0 //deprecated
    {"doSelectSecureElement", "(Ljava/lang/String;)Z",
            (void *)nfcManager_doSelectSecureElement},

    {"doDeselectSecureElement", "()V",
            (void *)nfcManager_doDeselectSecureElement},
#endif //deprecated

    {"doCheckLlcp", "()Z",
            (void *)nfcManager_doCheckLlcp},

    {"doActivateLlcp", "()Z",
            (void *)nfcManager_doActivateLlcp},

    {"doCreateLlcpConnectionlessSocket", "(ILjava/lang/String;)Lcom/android/nfc/dhimpl/NativeLlcpConnectionlessSocket;",
            (void *)nfcManager_doCreateLlcpConnectionlessSocket},

    {"doCreateLlcpServiceSocket", "(ILjava/lang/String;III)Lcom/android/nfc/dhimpl/NativeLlcpServiceSocket;",
            (void*) nfcManager_doCreateLlcpServiceSocket},

    {"doCreateLlcpSocket", "(IIII)Lcom/android/nfc/dhimpl/NativeLlcpSocket;",
            (void*) nfcManager_doCreateLlcpSocket},

    {"doGetLastError", "()I",
            (void*) nfcManager_doGetLastError},

    {"disableDiscovery", "()Z",
            (void*) nfcManager_disableDiscovery},

#if 0 //using line from master branch
    {"doIsUiStateSupported", "()Z",
            (void *)nfcManager_doIsUiStateSupported},
#endif

    {"deactivateRfInterface", "()V",
            (void*) nfcManager_deactivateRfInterface},

    {"updateHostPresence", "(II)V",
            (void*) nfcManager_updateHostPresence},

    {"updateLockScreenPollingMode", "(Z)Z",
            (void*) nfcManager_updateLockScreenPollingMode},

    {"PrbsOn", "(IIZ)Z",
            (void*)nfcManager_PrbsOn},

    {"PrbsOff", "()Z",
        (void*)nfcManager_PrbsOff},

    {"doSetTimeout", "(II)Z",
            (void *)nfcManager_doSetTimeout},

    {"doGetTimeout", "(I)I",
            (void *)nfcManager_doGetTimeout},

    {"doResetTimeouts", "()V",
            (void *)nfcManager_doResetTimeouts},

    {"doAbortCount", "(I)V",
            (void *)nfcManager_doAbortCount},

    {"doSetP2pInitiatorModes", "(I)V",
            (void *)nfcManager_doSetP2pInitiatorModes},

    {"doSetP2pTargetModes", "(I)V",
            (void *)nfcManager_doSetP2pTargetModes},

    {"doEnableScreenOffSuspend", "()V",
            (void *)nfcManager_doEnableScreenOffSuspend},

    {"doDisableScreenOffSuspend", "()V",
            (void *)nfcManager_doDisableScreenOffSuspend},

    {"doDump", "()Ljava/lang/String;",
            (void *)nfcManager_doDump},

    {"doReportReason", "(I)V",
            (void *)nfcManager_doReportReason},

    {"doGetRamDump", "(II)[B",
            (void *)nfcManager_doGetRamDump},

    {"doGetEeRoutingState", "()I",
            (void *)nfcManager_doGetEeRoutingState},

    {"doGetEeRoutingReloadAtReboot", "()Z",
            (void *)nfcManager_doGetEeRoutingReloadAtReboot},

    {"doGetDefaultActiveSecureElement", "()Ljava/lang/String;",
            (void *)nfcManager_doGetDefaultActiveSecureElement},

    {"doGetSecureElementList", "()Ljava/lang/String;",
            (void *)nfcManager_doGetSecureElementList},

    {"doGetSecureElementName", "(I)Ljava/lang/String;",
            (void *)nfcManager_doGetSecureElementName},

    {"doGetNfceeId", "(Ljava/lang/String;)I",
            (void *)nfcManager_doGetNfceeId},

    {"doGetLMRT", "()[B",
            (void *)nfcManager_doGetLMRT},

    {"doSetDefaultRoute", "(I)Z",
            (void*) nfcManager_doSetDefaultRoute},

    {"doGetDefaultRoute", "()I",
            (void*) nfcManager_doGetDefaultRoute},

    {"doIsExchangingApduWithEse", "()Z",
            (void *)nfcManager_doIsExchangingApduWithEse},

    {"doIsRfInterfaceActivated", "()Z",
            (void *)nfcManager_doIsRfInterfaceActivated},

    {"doNotifyApduGateRfIntfDeactivated", "()V",
            (void *)nfcManager_doNotifyApduGateRfIntfDeactivated},

    {"doInitClfAidFilterList", "()Z",
            (void*) nfcManager_doInitClfAidFilterList},

    {"doSetClfAidFilterList", "([B)Z",
            (void*) nfcManager_doSetClfAidFilterList},

    { "doEnableDisableClfAidFilterCondition", "(ZB)Z",
            (void *)nfcManager_doEnableDisableClfAidFilterCondition},

#ifdef DTA // <DTA>
    {"do_dta_set_pattern_number", "(I)V",
        (void *)nfcManager_dta_set_pattern_number},

    {"do_dta_get_pattern_number", "()I",
            (void *)nfcManager_dta_get_pattern_number},

    {"doNfcDeactivate", "(I)Z",
     (void *)nfcManager_doNfcDeactivate},
#endif // </DTA>

};


/*******************************************************************************
**
** Function:        register_com_android_nfc_NativeNfcManager
**
** Description:     Regisgter JNI functions with Java Virtual Machine.
**                  e: Environment of JVM.
**
** Returns:         Status of registration.
**
*******************************************************************************/
int register_com_android_nfc_NativeNfcManager (JNIEnv *e)
{
    ALOGD ("%s: enter", __FUNCTION__);
    PowerSwitch::getInstance ().initialize (PowerSwitch::UNKNOWN_LEVEL);
    ALOGD ("%s: exit", __FUNCTION__);
    return jniRegisterNativeMethods (e, gNativeNfcManagerClassName, gMethods, NELEM (gMethods));
}


/*******************************************************************************
**
** Function:        startRfDiscovery
**
** Description:     Ask stack to start polling and listening for devices.
**                  isStart: Whether to start.
**
** Returns:         None
**
*******************************************************************************/
void startRfDiscovery(bool isStart)
{
    tNFA_STATUS status = NFA_STATUS_FAILED;
    #ifdef DTA // <DTA>
    UINT8 nfc_f_condevlimit_param[] = { 0x00 };
    UINT8 con_bailout_param[1]={0};
    //UINT32 config_fwi = 0;
    #endif // </DTA>

    ALOGD ("%s: is start=%d", __FUNCTION__, isStart);
    SyncEventGuard guard (sNfaEnableDisablePollingEvent);

    #ifdef DTA // <DTA>
    // <DTA> Set NFC DEP options for LLCP operation:
    // Enable all bits specified in NCI specification table 81.
    // Set NFC-DEP SPEED to "maintain bitrate" so that PSL_REQ is not sent
    if (isStart && in_llcp_or_snep_dta_mode()) {
        SyncEventGuard guard2 (sNfaSetConfigEvent);
        ALOGD ("%s: [DTA] pattern 0xffff selected, setting NFC_DEP_OP params for LLCP operation", __FUNCTION__);
        UINT8 nfc_dep_op[] = { 0x0F };
        tNFA_STATUS stat = NFA_SetConfig(NFC_PMID_NFC_DEP_OP, sizeof(nfc_dep_op),
                                          &nfc_dep_op[0]);
        if (stat == NFA_STATUS_OK)
            sNfaSetConfigEvent.wait();


        // Maintain bitrate:
        ALOGD ("%s: [DTA] pattern 0xffff selected, setting NFC-DEP maintain bitrate", __FUNCTION__);
        UINT8 bitr_nfc_dep_param[] = { 0x01 };
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                sizeof(bitr_nfc_dep_param),
                &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
            sNfaSetConfigEvent.wait();

        /* in case of llcp/snep set CON_DEVICES_LIMIT to 0 */
        nfc_f_condevlimit_param[0] = 0x00;
        stat = NFA_SetConfig(NCI_PARAM_ID_CON_DEVICES_LIMIT, sizeof(nfc_f_condevlimit_param),
                             &nfc_f_condevlimit_param[0]);
        if (stat == NFA_STATUS_OK)
            sNfaSetConfigEvent.wait();
    }

    if (!in_dta_mode() || in_llcp_or_snep_dta_mode()) {
        ALOGD ("%s: normal operation", __FUNCTION__);
    #endif // </DTA>
    if (isStart == true)
    {
        status = NFA_StartRfDiscovery ();
    }
    else
    {
        status = NFA_StopRfDiscovery ();
    }

    #ifdef DTA // <DTA>
    } else {
        ALOGD ("%s: [DTA] DTA mode", __FUNCTION__);

        ALOGD ("%s: [DTA] Use exclusive RF control instead of normal RF discovery", __FUNCTION__);


        /** DTA Specification v2.0 configuration parameter mapping
      ----------------------------------------------------------

      Information of each parameter
      -ok/update needeed/correct by default
      -reference number that points to a place in the code below


      CON_LISTEN_DEP_A
      -correct by default (verified when testing CON_LISTEN_DEP_F)
      -ref: (DTA_CFG_01)

      CON_LISTEN_DEP_F
      -ok
      -configured in tNFA_LISTEN_CFG: lf_protocol_type
      -ref: (DTA_CFG_02)

      CON_LISTEN_T3TP
      -ok
      -configured in tNFA_LISTEN_CFG: lf_t3t_flags
      -ref: (DTA_CFG_03)

      CON_LISTEN4ATP
      -ok
      -configured in tNFA_LISTEN_CFG: la_sel_info
      -ref: (DTA_CFG_04)

      CON_LISTEN_T4BTP
      -ok
      -configured in tNFA_LISTEN_CFG: lb_sensb_info
      -ref (DTA_CFG_05)

      CON_ADV_FEAT
      -correct by default
      -ref (DTA_CFG_06)

      CON_SYS_CODE[2]
      -ok
      -the FFFF version is configured with lf_t3t_identifier and the other one is not
       needed as tag type 3 listen mode is not supported
      -note: NFA_LF_MAX_SC_NFCID2 is defined as 1 (related to the lf_t3t_identifier)
      -ref (DTA_CFG_07)

      CON_SENSF_RES[2]
      -ok
      -NFCID2: generated automatically
      -PAD0, PAD1, MRTIcheck, MRTIupdate and PAD2: configured in tNFA_LISTEN_CFG: lf_t3t_pmm
        -PAD0 is left as it is in stock Nexus 4 (20h 79h)
      -ref (DTA_CFG_08)

      CON_ATR_RES
      -NFCC handles this
      -ref (DTA_CFG_09)

      CON_ATS
      -ok
      -FWI and SFGI configured in tNFA_LISTEN_CFG: li_fwi and lb_sfgi
      -other parameters are in the default values of the chip
      -ref (DTA_CFG_10)

      CON_SENSB_RES
      -ok
      -Byte1: is what it is, not found from structs
      -Byte2: configured in tNFA_LISTEN_CFG: lb_sensb_info
      -Byte3: configured in tNFA_LISTEN_CFG: lb_adc_fo
        -current way of setting the value has no effect based on logs (set to 0x75, is 0x05 in logs)
        -could be set with NFA_SetConfig, which works
        -is left as it is currently as the value is 0x05 in stock Nexus 4 is also
      -ref: (DTA_CFG_11)

      CON_ATTRIB_RES
      -is formed from B parameters, nothing done to this one specifically
      -ref: (DTA_CFG_12)

      CON_BITR_F
      -ok
      -set with NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE,...
      -ref: (DTA_CFG_13)

      CON_POLL_A
      -ok
      -configured in tNFA_TECHNOLOGY_MASK
      -ref: (DTA_CFG_14)

      CON_POLL_B
      -ok, verified by trying to read type B tags
      -configured in tNFA_TECHNOLOGY_MASK
      -ref: (DTA_CFG_15)

      CON_POLL_F
      -ok
      -configured in tNFA_TECHNOLOGY_MASK
      -ref: (DTA_CFG_16)

      CON_POLL_P
      -ok
      -NFA_TECHNOLOGY_MASK_B_PRIME and NFA_TECHNOLOGY_MASK_KOVIO disabled in tNFA_TECHNOLOGY_MASK
      -ref: (DTA_CFG_17)

      CON_BAIL_OUT_A
      -ok, verified from logs
      -set with NFA_SetConfig(NCI_PARAM_ID_PA_BAILOUT,...
      -ref: (DTA_CFG_18)

      CON_BAIL_OUT_B
      -ok, verified from logs
      -set with NFA_SetConfig(NCI_PARAM_ID_PB_BAILOUT,...
      -ref: (DTA_CFG_19)

      CON_DEVICES_LIMIT
      -value defined in ICS, assuming that chip default is fine
      -can be set with NFA_SetConfig(NCI_PARAM_ID_CON_DEVICES_LIMIT,...
      -ref: (DTA_CFG_20)

      CON_ADV_FEAT
      -correct by default
      -ref: (DTA_CFG_21)

      CON_ANTICOLL
      -ok, verified from logs
      -set with NFA_SetConfig(NCI_PARAM_ID_PA_ANTICOLL,...
      -note: brcm proprietary param id
      -ref: (DTA_CFG_22)

      CON_ATR
      -is formed from NFC-DEP parameters, nothing done to this one specifically
      -ref: (DTA_CFG_23)

      CON_GB
      -is formed from ISO-DEP parameters, nothing done to this one specifically
      -note: NCI_PARAM_ID_ATR_REQ_GEN_BYTES is available
      -ref: (DTA_CFG_24)

      CON_RATS
      -no changes made, handled automatically
      -note: LA_FSDI, LB_FSDI, PA_FSDI and PB_FSDI NCI parameters are available
      -ref: (DTA_CFG_25)

      CON_ATTRIB
      -no changes made, handled automatically
      -ref: (DTA_CFG_26)

      CON_BITR_NFC_DEP
      -ok, verified from logs
      -configured by calling NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,...
      -ref: (DTA_CFG_27)
         **/

        // Use tNFA_TECHNOLOGY_MASK, tNFA_LISTEN_CFG and NCI parameters to set the parameters.
    tNFA_TECHNOLOGY_MASK poll_mask;
    poll_mask = DEFAULT_TECH_MASK;  // This one polls for all techs. (DTA_CFG_14), (DTA_CFG_16)
    poll_mask &= ~NFA_TECHNOLOGY_MASK_A_ACTIVE;
    poll_mask &= ~NFA_TECHNOLOGY_MASK_F_ACTIVE;
    poll_mask &= ~NFA_TECHNOLOGY_MASK_B_PRIME;  // (DTA_CFG_17)
    poll_mask &= ~NFA_TECHNOLOGY_MASK_KOVIO;  // (DTA_CFG_17)
    poll_mask &= ~NFA_TECHNOLOGY_MASK_ISO15693; // disabling poll mask for ISO15693 as this is not supported in QCA 2.1/2.0.

    /*TODO: Remove it later. only to disable poll for listen mode test*/
    /*poll_mask &= ~NFA_TECHNOLOGY_MASK_A;
    poll_mask &= ~NFA_TECHNOLOGY_MASK_B;
    poll_mask &= ~NFA_TECHNOLOGY_MASK_F;*/

    tNFA_LISTEN_CFG listen_cfg = { };
    tNFA_STATUS stat = NFA_STATUS_FAILED;
    SyncEventGuard guard(sNfaSetConfigEvent);

    /* backup last pattern number */
    LastPatternNumber = sDtaPatternNumber;
    sDtaPatternNumber = NFA_DTA_Get_Pattern_Number();
    ALOGD("[DTA] sDtaPatternNumber = %d LastPatternNumber = %d", sDtaPatternNumber, LastPatternNumber);
    if (!isStart)
    {
        ALOGD("[DTA] isStart = %d , disable discovery", isStart);
        goto TheEnd;
    }
    // Set the pattern number dependant parameters.
    switch (sDtaPatternNumber) {
      case 0x00: {
        ALOGD("%s: [DTA] pattern number 0000h selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly

        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        /* in case of pattern_no 0 reset CON_DEVICES_LIMIT to default value*/
        nfc_f_condevlimit_param[0] = 0x03;
        stat = NFA_SetConfig(NCI_PARAM_ID_CON_DEVICES_LIMIT, sizeof(nfc_f_condevlimit_param),
                             &nfc_f_condevlimit_param[0]);

        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();


        /* Enable RTOX */
        UINT8 bitr_nfc_dep_param2[] = { 0x0E };
        stat = NFA_SetConfig(NCI_PARAM_ID_NFC_DEP_OP,
                sizeof(bitr_nfc_dep_param2),
                &bitr_nfc_dep_param2[0]);
        if (stat == NFA_STATUS_OK)
            sNfaSetConfigEvent.wait();

        if (enablePassivePollMode) {
            poll_mask &= ~NFA_TECHNOLOGY_MASK_A_ACTIVE;
            poll_mask &= ~NFA_TECHNOLOGY_MASK_F_ACTIVE;
        }

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
            sNfaSetConfigEvent.wait();


        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - not needed for this pattern number

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        ALOGD("%s: [DTA] pattern number 0000h selected : exit", __FUNCTION__);

        break;
    }

      case 0x01: {
        ALOGD("%s : [DTA] pattern number 0001h selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();


        // NFC_PMID_CON_DEVICES_LIMIT
     //   UINT8 nfc_f_condevlimit_param[] = { 0x00 };
        nfc_f_condevlimit_param[0] = 0x00;
        stat = NFA_SetConfig(NCI_PARAM_ID_CON_DEVICES_LIMIT, sizeof(nfc_f_condevlimit_param),
                             &nfc_f_condevlimit_param[0]);

        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - this call is done later in a separate function.

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No
        break;
      }

      case 0x02: {
        ALOGD("%s : [DTA] pattern number 0002h selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        //   UINT8 nfc_f_condevlimit_param[] = { 0x00 };
        nfc_f_condevlimit_param[0] = 0x00;
        stat = NFA_SetConfig(NCI_PARAM_ID_CON_DEVICES_LIMIT, sizeof(nfc_f_condevlimit_param),
                             &nfc_f_condevlimit_param[0]);

        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - this call is done later in a separate function.

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        break;
      }

      case 0x03: {
        ALOGD("%s : [DTA] pattern number 0003h selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        nfc_f_condevlimit_param[0] = 0x00;
        stat = NFA_SetConfig(NCI_PARAM_ID_CON_DEVICES_LIMIT, sizeof(nfc_f_condevlimit_param),
                             &nfc_f_condevlimit_param[0]);

        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;  //(DTA_CFG_13)

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - this call is done later in a separate function.

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        break;
      }

      case 0x04: {
        ALOGD("%s : [DTA] pattern number 0004h selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        nfc_f_condevlimit_param[0] = 0x00;
        stat = NFA_SetConfig(NCI_PARAM_ID_CON_DEVICES_LIMIT, sizeof(nfc_f_condevlimit_param),
                             &nfc_f_condevlimit_param[0]);

        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;  //(DTA_CFG_13)

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - this call is done later in a separate function.

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        break;
      }

      case 0x05: {
        ALOGD("%s : [DTA] pattern number 0005h selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        /*TODO: Change name of param for b. like nfc_b_condevlimit_param*/
        nfc_f_condevlimit_param[0] = 0x01;
        stat = NFA_SetConfig(NCI_PARAM_ID_CON_DEVICES_LIMIT, sizeof(nfc_f_condevlimit_param),
                             &nfc_f_condevlimit_param[0]);

        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();


        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;  //(DTA_CFG_13)

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - this call is done later in a separate function.

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        break;
      }

      case 0x06: {
        ALOGD("%s : [DTA] pattern number 0006h selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_424 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_424;  //(DTA_CFG_13)

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - this call is done later in a separate function.

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        break;
      }

      case 0x07: {
        ALOGD("%s : [DTA] pattern number 0007h selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        /* CON_DEVICES_LIMIT = 0x00 for
           TC_POL_NFCF_UND_BV_1
           TC_AN_POL_NFCF_BV_02 LM min
           TC_AN_POL_NFCF_BV_02 LM max */
        nfc_f_condevlimit_param[0] = 0x00;
        stat = NFA_SetConfig(NCI_PARAM_ID_CON_DEVICES_LIMIT, sizeof(nfc_f_condevlimit_param),
                             &nfc_f_condevlimit_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;  //(DTA_CFG_13)

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - this call is done later in a separate function.

        // SENSF_REQ Reactivation - ?

        // Reactivation - Yes

        break;
      }

      case 0x08: {
        ALOGD("%s : [DTA] pattern number 0008h selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_424 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_424;  //(DTA_CFG_13)

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - this call is done later in a separate function.

        // SENSF_REQ Reactivation - ?

        // Reactivation - NFC-F only

        break;
      }

      case 0x09: {
        ALOGD("%s : [DTA] pattern number 0009h selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { NFC_BIT_RATE_424 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;  //(DTA_CFG_13)

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - not needed for this pattern number

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        break;
      }

      case 0x0A: {
        ALOGD("%s : [DTA] pattern number 000Ah selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK
        poll_mask &= ~NFA_TECHNOLOGY_MASK_B;  // (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_424 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_424;  //(DTA_CFG_13)

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - not needed for this pattern number

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        break;
      }

      case 0x0B: {
        ALOGD("%s : [DTA] pattern number 000Bh selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK
        poll_mask &= ~NFA_TECHNOLOGY_MASK_B;  // (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;  //(DTA_CFG_13)

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - not needed for this pattern number

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        break;
      }

      case 0x0C: {
        ALOGD("%s : [DTA] pattern number 000Ch selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;  //(DTA_CFG_13)

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = 0x00;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - not needed for this pattern number

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        break;
      }

      case 0x0D: {
        ALOGD("%s : [DTA] pattern number 000Dh selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly
        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;  //(DTA_CFG_13)

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - not needed for this pattern number

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        break;
      }

      case 0x28: { // 40 for listen mode w. DID support
        ALOGD("%s: [DTA] pattern number 0028h selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly

        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        /* in case of pattern_no 0 reset CON_DEVICES_LIMIT to default value*/
        nfc_f_condevlimit_param[0] = 0x03;
        stat = NFA_SetConfig(NCI_PARAM_ID_CON_DEVICES_LIMIT, sizeof(nfc_f_condevlimit_param),
                             &nfc_f_condevlimit_param[0]);

        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();


        /* Enable RTOX */
        UINT8 bitr_nfc_dep_param2[] = { 0x0E };
        stat = NFA_SetConfig(NCI_PARAM_ID_NFC_DEP_OP,
                sizeof(bitr_nfc_dep_param2),
                &bitr_nfc_dep_param2[0]);
        if (stat == NFA_STATUS_OK)
            sNfaSetConfigEvent.wait();

        /* Enable DID */
        UINT8 la_did_required[] = { 0x01 };
        stat = NFA_SetConfig(NCI_PARAM_LA_DID_REQUIRED,
                sizeof(la_did_required),
                &la_did_required[0]);
        if(stat == NFA_STATUS_OK)
            sNfaSetConfigEvent.wait();

        if (enablePassivePollMode) {
            poll_mask &= ~NFA_TECHNOLOGY_MASK_A_ACTIVE;
            poll_mask &= ~NFA_TECHNOLOGY_MASK_F_ACTIVE;
        }

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
            sNfaSetConfigEvent.wait();


        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - not needed for this pattern number

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        ALOGD("%s: [DTA] pattern number 0028h selected : exit", __FUNCTION__);

        break;
    }
      case 0x2B: { // 43 for Poll B w. Sleep Response
        ALOGD("%s: [DTA] pattern number 002Bh selected", __FUNCTION__);

        // CON_POLL_B - defined in tNFA_TEHCNOLOGY_MASK - no need to disable in this pattern (DTA_CFG_15)

        // CON_BITR_NFC_DEP - set via NCI parameter directly

        UINT8 bitr_nfc_dep_param[] = { 0x01 };  //(DTA_CFG_27)
        stat = NFA_SetConfig(NCI_PARAM_ID_BITR_NFC_DEP,
                             sizeof(bitr_nfc_dep_param),
                             &bitr_nfc_dep_param[0]);
        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();

        /* in case of pattern_no 43, set CON_DEVICES_LIMIT to 0x01 */
        nfc_f_condevlimit_param[0] = 0x01;
        stat = NFA_SetConfig(NCI_PARAM_ID_CON_DEVICES_LIMIT, sizeof(nfc_f_condevlimit_param),
                             &nfc_f_condevlimit_param[0]);

        if (stat == NFA_STATUS_OK)
          sNfaSetConfigEvent.wait();


        /* Enable RTOX */
        UINT8 bitr_nfc_dep_param2[] = { 0x0E };
        stat = NFA_SetConfig(NCI_PARAM_ID_NFC_DEP_OP,
                sizeof(bitr_nfc_dep_param2),
                &bitr_nfc_dep_param2[0]);
        if (stat == NFA_STATUS_OK)
            sNfaSetConfigEvent.wait();

        if (enablePassivePollMode) {
            poll_mask &= ~NFA_TECHNOLOGY_MASK_A_ACTIVE;
            poll_mask &= ~NFA_TECHNOLOGY_MASK_F_ACTIVE;
        }

        // NFC-F_BIT_RATE
        UINT8 nfc_f_bitr_param[] = { NFC_BIT_RATE_212 };  //(DTA_CFG_13)
        stat = NFA_SetConfig(NCI_PARAM_ID_PF_BIT_RATE, sizeof(nfc_f_bitr_param),
                             &nfc_f_bitr_param[0]);
        if (stat == NFA_STATUS_OK)
            sNfaSetConfigEvent.wait();


        listen_cfg.lf_con_bitr_f = NFC_BIT_RATE_212;

        // CON_LISTEN_DEP_F - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_protocol_type = NCI_LISTEN_PROTOCOL_NFC_DEP;  //(DTA_CFG_02)

        // CON_LISTEN_T3TP3 - defined in tNFA_LISTEN_CFG
        listen_cfg.lf_t3t_flags = 0x0000;  //(DTA_CFG_03)

        // SENSF_REQ - not needed for this pattern number

        // SENSF_REQ Reactivation - not needed for this pattern number

        // Reactivation - No

        ALOGD("%s: [DTA] pattern number 002Bh selected : exit", __FUNCTION__);

        break;
      }

      default: {
        ALOGD("%s : [DTA] no supported pattern number selected", __FUNCTION__);
        break;
      }

    }


    ALOGD("[DTA] Have set Pattern -  Continue");

    // Set other DTA parameters (not pattern number dependant).
    // Set which tag types to listen to.
    listen_cfg.la_enable = true;
    listen_cfg.lb_enable = true;
    listen_cfg.lf_enable = true;
    listen_cfg.li_enable = true;
    listen_cfg.ln_enable = true;

    // Simulate Lower Tester?
    // Disable listening altoghether:
    if (dta::simulateLowerTester) {
      listen_cfg.la_enable = false;
      listen_cfg.lb_enable = false;
      listen_cfg.lf_enable = false;
      listen_cfg.li_enable = false;
      listen_cfg.ln_enable = false;
    }

    listen_cfg.la_bit_frame_sdd = 0x04;
    listen_cfg.la_platform_config = 0x00;

    // CON_LISTEN4ATP (DTA_CFG_04)
    listen_cfg.la_sel_info = NCI_PARAM_SEL_INFO_NFCDEP
        | NCI_PARAM_SEL_INFO_ISODEP;

    listen_cfg.la_nfcid1_len = 0x00;

    //listen_cfg.la_nfcid1          // Leave empty -> NFCC will decide

    // Byte 11 of SENSB_RES (DTA_CFG_11), (DTA_CFG_05):
    listen_cfg.lb_sensb_info = 0x81;

    listen_cfg.lb_nfcid0_len = 0x00;
    //listen_cfg.lb_nfcid0          // Leave empty -> NFCC will decide

    int i;
    for (i = 0; i < NCI_PARAM_LEN_LB_APPDATA; i++) {
      listen_cfg.lb_app_data[i] = 0x00;
    }

    // CON_ATS (DTA_CFG_10):
    listen_cfg.lb_sfgi = 0x00;

    /*TODO: make FWI=8 after test .makeing FWI configurable for NOW for Testing only*/
    /*GetNumValue("FWI", &config_fwi, sizeof(config_fwi));
    listen_cfg.li_fwi = config_fwi;*/
    listen_cfg.li_fwi = 0x08; // DTA? changed from 7 to 8 need for nfc forum
    ALOGD("[DTA] : listen_cfg.li_fwi=%X",listen_cfg.li_fwi);

    // Byte 12 of SENSB_RES (DTA_CFG_11):
    /*note:  Setting the value like this has no effect according to logs.
     The value seems to be 0x05. The value could be set via NFA_SetConfig, but
     it's left as it is for now as it's like that in stock version of Nexus 4 also. */
    listen_cfg.lb_adc_fo = 0x05;

    /*
     UINT8  lb_adc_fo_param[] = { 0x75 };
     stat = NFA_SetConfig(NCI_PARAM_ID_LB_ADC_FO, sizeof(lb_adc_fo_param), &lb_adc_fo_param[0]);
     if (stat == NFA_STATUS_OK)
     sNfaSetConfigEvent.wait ();
     */

    // CON_SYS_CODE[2] (DTA_CFG_07)
    int j;
    for (i = 0; i < NFA_LF_MAX_SC_NFCID2; i++) {
      for (j = 0; j < NCI_SYSTEMCODE_LEN + NCI_NFCID2_LEN; j++) {
        if (j <= 1) {
          listen_cfg.lf_t3t_identifier[i][j] = 0xFF;
        } else if (j == 2) {
          listen_cfg.lf_t3t_identifier[i][j] = 0x02;
        } else if (j == 3) {
          listen_cfg.lf_t3t_identifier[i][j] = 0xFE;
        } else {
          listen_cfg.lf_t3t_identifier[i][j] = 0x00;
        }
      }
    }

    // CON_SENSF_RES[2] (DTA_CFG_08):
    listen_cfg.lf_t3t_pmm[0] = 0x20;
    listen_cfg.lf_t3t_pmm[1] = 0x79;
    for (i = 2; i < NCI_T3T_PMM_LEN; i++) {
      listen_cfg.lf_t3t_pmm[i] = 0xFF;
    }

    listen_cfg.la_hist_bytes_len = 0x00;
    //listen_cfg.la_hist_bytes[] - default value is empty
    listen_cfg.lb_h_info_resp_len = 0x00;
    //listen_cfg.lb_h_info_resp[] - default resp is empty

    listen_cfg.ln_wt = 0x08;
    ALOGD("[DTA] : generalBytesOn = %d", generalBytesOn);
    if (generalBytesOn) {
    ALOGD("[DTA] : generalBytesOn");
    listen_cfg.ln_atr_res_gen_bytes_len = 0x20;
    listen_cfg.ln_atr_res_gen_bytes[0] = 0x46;
    listen_cfg.ln_atr_res_gen_bytes[1] = 0x66;
    listen_cfg.ln_atr_res_gen_bytes[2] = 0x6D;
    listen_cfg.ln_atr_res_gen_bytes[3] = 0x01;
    listen_cfg.ln_atr_res_gen_bytes[4] = 0x01;
    listen_cfg.ln_atr_res_gen_bytes[5] = 0x11;
    listen_cfg.ln_atr_res_gen_bytes[6] = 0x02;
    listen_cfg.ln_atr_res_gen_bytes[7] = 0x02;
    listen_cfg.ln_atr_res_gen_bytes[8] = 0x07;
    listen_cfg.ln_atr_res_gen_bytes[9] = 0xFF;
    listen_cfg.ln_atr_res_gen_bytes[10] = 0x03;
    listen_cfg.ln_atr_res_gen_bytes[11] = 0x02;
    listen_cfg.ln_atr_res_gen_bytes[12] = 0x00;
    listen_cfg.ln_atr_res_gen_bytes[13] = 0x13;
    listen_cfg.ln_atr_res_gen_bytes[14] = 0x04;
    listen_cfg.ln_atr_res_gen_bytes[15] = 0x01;
    listen_cfg.ln_atr_res_gen_bytes[16] = 0x64;
    listen_cfg.ln_atr_res_gen_bytes[17] = 0x07;
    listen_cfg.ln_atr_res_gen_bytes[18] = 0x01;
    listen_cfg.ln_atr_res_gen_bytes[19] = 0x03;
    }
    else {
      listen_cfg.ln_atr_res_gen_bytes_len = 0;
    }

    listen_cfg.ln_atr_res_config = 0x30;

    // CON_BAIL_OUT_A (DTA_CFG_18)
    con_bailout_param[0] = { 0x01 };
    stat = NFA_SetConfig(NCI_PARAM_ID_PA_BAILOUT, sizeof(con_bailout_param),
                         &con_bailout_param[0]);
    if (stat == NFA_STATUS_OK)
      sNfaSetConfigEvent.wait();

    // CON_BAIL_OUT_B (DTA_CFG_19)
    stat = NFA_SetConfig(NCI_PARAM_ID_PB_BAILOUT, sizeof(con_bailout_param),
                         &con_bailout_param[0]);
    if (stat == NFA_STATUS_OK)
      sNfaSetConfigEvent.wait();

    // CON_ANTICOLL (DTA_CFG_22)
   /* UINT8 con_anticoll_param[] = { 0x01 };
    stat = NFA_SetConfig(NCI_PARAM_ID_PA_ANTICOLL, sizeof(con_anticoll_param),
                         &con_anticoll_param[0]);
    if (stat == NFA_STATUS_OK)
      sNfaSetConfigEvent.wait();*/

TheEnd:

    if (isStart) {
      ALOGD(
          "%s : [DTA] Requesting exclusive RF control instead of normal RF discovery",
          __FUNCTION__);
      status = NFA_RequestExclusiveRfControl(poll_mask, &listen_cfg,
                                             nfaConnectionCallback,
                                             dta_ndef_callback);
    } else {
      ALOGD(
          "%s : [DTA] Releasing exclusive RF control instead of normal RF discovery",
          __FUNCTION__);
      status = NFA_ReleaseExclusiveRfControl();
    }

  }
  #endif // </DTA>
    if (status == NFA_STATUS_OK)
    {
        sNfaEnableDisablePollingEvent.wait (); //wait for NFA_RF_DISCOVERY_xxxx_EVT
        if (isStart == true)
        {
            sDiscoverState = DISCOVER_STATE_DISCOVERY;
        }
        else
        {
            sDiscoverState = DISCOVER_STATE_IDLE;

        }
       // sNfaEnableDisablePollingEvent.wait (20); //wait for sending sleep cmd in HAL
    }
    else
    {
        ALOGE ("%s: Failed to start/stop RF discovery; error=0x%X", __FUNCTION__, status);
    }
}


/*******************************************************************************
**
** Function:        doStartupConfig
**
** Description:     Configure the NFC controller.
**
** Returns:         None
**
*******************************************************************************/
void doStartupConfig()
{
    int actualLen = 0;
    UINT32 num = 0;
#if defined(FEATURE_STARTUP_CONFIG_FLAG)
    struct nfc_jni_native_data *nat = getNative(0, 0);

    // If polling for Active mode, set the ordering so that we choose Active over Passive mode first.
    if (nat && (nat->tech_mask & (NFA_TECHNOLOGY_MASK_A_ACTIVE | NFA_TECHNOLOGY_MASK_F_ACTIVE)))
    {
        UINT8  act_mode_order_param[] = { 0x01 };
        SyncEventGuard guard (sNfaSetConfigEvent);
        stat = NFA_SetConfig(NCI_PARAM_ID_ACT_ORDER, sizeof(act_mode_order_param), &act_mode_order_param[0]);
        if (stat == NFA_STATUS_OK)
            sNfaSetConfigEvent.wait ();
    }
#endif /* End FEATURE_STARTUP_CONFIG_FLAG */
    //configure RF polling frequency for each technology
    static tNFA_DM_DISC_FREQ_CFG nfa_dm_disc_freq_cfg;
    //values in the polling_frequency[] map to members of nfa_dm_disc_freq_cfg
    UINT8 polling_frequency [8] = {1, 1, 1, 1, 1, 1, 1, 1};
    actualLen = GetStrValue(NAME_POLL_FREQUENCY, (char*)polling_frequency, 8);
    if (actualLen == 8)
    {
        ALOGD ("%s: polling frequency", __FUNCTION__);
        memset (&nfa_dm_disc_freq_cfg, 0, sizeof(nfa_dm_disc_freq_cfg));
        nfa_dm_disc_freq_cfg.pa = polling_frequency [0];
        nfa_dm_disc_freq_cfg.pb = polling_frequency [1];
        nfa_dm_disc_freq_cfg.pf = polling_frequency [2];
        nfa_dm_disc_freq_cfg.pi93 = polling_frequency [3];
        nfa_dm_disc_freq_cfg.pbp = polling_frequency [4];
        nfa_dm_disc_freq_cfg.pk = polling_frequency [5];
        nfa_dm_disc_freq_cfg.paa = polling_frequency [6];
        nfa_dm_disc_freq_cfg.pfa = polling_frequency [7];
        p_nfa_dm_rf_disc_freq_cfg = &nfa_dm_disc_freq_cfg;
    }
    ALOGD("%s: sNfcChipVersion = %d", __FUNCTION__,
            sNfcChipVersion);

    if (sNfcChipVersion >= 30)
    {
#ifdef DTA // <DTA>
        ALOGD("%s: in_dta_mode(): %d", __FUNCTION__, in_dta_mode());
        if(in_dta_mode())
        {
            /*
             * If chip version 3.0 or later and in DTA mode
             * Disable RF polling by modifying NVM RFPOLLMACP_ENABLE_FEATURE
             */
            modifyIndexedNVM(RFPOLLMACP_ENABLE_FEATURE, 0);

            if (GetNumValue("DTA_AGC_ENABLE_MASK", &num, sizeof(num)) != 0)
            {
                /*
                 * 0 = disable
                 */
                ALOGD("%s: Set value %d to AGC NVM in DTA mode", __FUNCTION__, num);
                modifyIndexedNVM(RXAGCMAP_RXAGCENABLE, num);
            }
        }
        else
        {
#endif // </DTA>
        if (GetNumValue("NORMAL_AGC_ENABLE_MASK", &num, sizeof(num)) != 0)
        {
            /*
             * 3 = report DC values
             */
            ALOGD("%s: Set value %d to AGC NVM in normal mode", __FUNCTION__, num);
            modifyIndexedNVM(RXAGCMAP_RXAGCENABLE, num);
        }
#ifdef DTA // <DTA>
        }
#endif // </DTA>
    }

}


/*******************************************************************************
**
** Function:        nfcManager_isNfcActive
**
** Description:     Used externaly to determine if NFC is active or not.
**
** Returns:         'true' if the NFC stack is running, else 'false'.
**
*******************************************************************************/
bool nfcManager_isNfcActive()
{
    return sIsNfaEnabled;
}

/*******************************************************************************
**
** Function:        startStopPolling
**
** Description:     Start or stop polling.
**                  isStartPolling: true to start polling; false to stop polling.
**
** Returns:         None.
**
*******************************************************************************/
void startStopPolling (bool isStartPolling)
{
    ALOGD ("%s: enter; isStart=%u", __FUNCTION__, isStartPolling);
    tNFA_STATUS stat = NFA_STATUS_FAILED;

    startRfDiscovery (false);

    if (isStartPolling) startPolling_rfDiscoveryDisabled(0);
    else stopPolling_rfDiscoveryDisabled();

    startRfDiscovery (true);
    ALOGD ("%s: exit", __FUNCTION__);
}


static tNFA_STATUS startPolling_rfDiscoveryDisabled(tNFA_TECHNOLOGY_MASK tech_mask) {
    tNFA_STATUS stat = NFA_STATUS_FAILED;

    unsigned long num = 0;

    if (tech_mask == 0 && GetNumValue(NAME_POLLING_TECH_MASK, &num, sizeof(num)))
        tech_mask = num;
    else if (tech_mask == 0) tech_mask = DEFAULT_TECH_MASK;


    if (sIsNfcRestart == true)
    {
        ALOGD ("%s in restart", __FUNCTION__);
        return stat;
    }

    SyncEventGuard guard (sNfaEnableDisablePollingEvent);
    ALOGD ("%s: enable polling", __FUNCTION__);
    stat = NFA_EnablePolling (tech_mask);
    if (stat == NFA_STATUS_OK)
    {
        ALOGD ("%s: wait for enable event", __FUNCTION__);
        sNfaEnableDisablePollingEvent.wait (); //wait for NFA_POLL_ENABLED_EVT
    }
    else
    {
        ALOGE ("%s: fail enable polling; error=0x%X", __FUNCTION__, stat);
    }

    return stat;
}

static tNFA_STATUS stopPolling_rfDiscoveryDisabled() {
    tNFA_STATUS stat = NFA_STATUS_FAILED;

    if (sIsNfcRestart == true)
    {
        ALOGD ("%s in restart", __FUNCTION__);
        return stat;
    }

    SyncEventGuard guard (sNfaEnableDisablePollingEvent);
    ALOGD ("%s: disable polling", __FUNCTION__);
    stat = NFA_DisablePolling ();
    if (stat == NFA_STATUS_OK) {
        sNfaEnableDisablePollingEvent.wait (); //wait for NFA_POLL_DISABLED_EVT
    } else {
        ALOGE ("%s: fail disable polling; error=0x%X", __FUNCTION__, stat);
    }

    return stat;
}

/******************************************************************************
 **
 ** Function:        NVMCallback
 **
 ** Description:     Callback function for NVM
 **
 ** Returns:         None.
 **
 ******************************************************************************/
void NVMCallback (UINT8 event, UINT16 param_len, UINT8 *p_param)
{
    /*
     * event is (NCI_RSP_BIT|oid)
     * NCI response:
     * 4f 01 05 00 00 00 00 00 for read (4f01 length status nvm)
     * 4f 01 01 00 for write (4f01 length status)
     */
    const UINT8 oid_mask = 0x3F;
    const UINT8 rsp_mask = 0xC0;
    const UINT8 oid_01 = 0x01;
    const UINT16 read_len = 8;
    const UINT16 write_len = 4;
    const int status_byte_index = 3;

    UINT8 oid = (event & oid_mask); // mask out oid
#ifdef NVM_DEBUG
    ALOGD ("%s: event: 0x%02X, param_len: %d", __FUNCTION__,event, param_len);
    for(UINT16 i = 0; i < param_len; ++i)
    {
        ALOGD("%s: p_param[%d] = 0x%02X", __FUNCTION__, i, p_param[i]);
    }
#endif
    sNVMbuffer = 0;
    UINT8 *nvm = (UINT8 *)&sNVMbuffer; // byte access for sNVMbuffer

    if((event & rsp_mask) == NCI_RSP_BIT) // mask out NCI_RSP_BIT
    {
        if(oid == oid_01)
        {
            // read answer is 8 bytes, status is in 4th byte
            if ((param_len >= read_len) &&
                    (*(p_param + status_byte_index) == NFA_STATUS_OK))
            {
                ALOGD("%s: read event = 0x%02x success", __FUNCTION__, event);
                // store nvm byte values
                nvm[0] = p_param[4];
                nvm[1] = p_param[5];
                nvm[2] = p_param[6];
                nvm[3] = p_param[7];
            } else if ((param_len >= read_len) &&
                    (*(p_param + status_byte_index) != NFA_STATUS_OK)) {
                ALOGE("%s: read event = 0x%02x failed", __FUNCTION__, event);
            // write answer is 4 bytes, status is in 4th byte
            } else if ((param_len == write_len)
                    && (*(p_param + status_byte_index) == NFA_STATUS_OK)) {
                ALOGD("%s: write event = 0x%02x success", __FUNCTION__, event);
            } else if ((param_len == write_len ) &&
                    (*(p_param + status_byte_index) != NFA_STATUS_OK)){
                ALOGE("%s: write event = 0x%02x failed", __FUNCTION__, event);
            // neither read nor write
            } else {
                ALOGE("%s: unknown event = 0x%02x failed",
                        __FUNCTION__, event);
            }
        }
    }
    sNfaModifyNVM.notifyOne();
}


/******************************************************************************
 **
 ** Function:        getNVM
 **
 ** Description:     read NVM index
 **                  result is written in NVMCallback()
 **
 ** Returns:         None.
 **
 ******************************************************************************/
void getNVM (UINT32 index)
{
    tNFA_STATUS stat = NFA_STATUS_FAILED;
    UINT8            cmd_params_len = 0x06;
    /*
     * NCI command is 2f010600 + index of tag in big endian format
     * cmd_params[] to store
     *      sub-opcode[2] (0x06, 0x00), index[4]
     * opcode id = 0x01
     */
    UINT8            cmd_params[] = {0x06,0x00,0x00,0x00,0x00,0x00};
    UINT8            *p_cmd_params = cmd_params;
    UINT8            oid_01 = 0x01;

    UINT32 nlindex = htonl(index); // little --> big endian
    UINT8 *ix = (UINT8 *)&nlindex; // byte access on nlindex

    for(UINT8 i = 2; i < 6; ++i) {
        cmd_params[i] = ix[i-2]; // 2-5, 0-3
    }

    SyncEventGuard guard(sNfaModifyNVM); // initialize synchronization var

    stat = NFA_SendVsCommand(oid_01, cmd_params_len,
            p_cmd_params, NVMCallback);
    if (stat == NFA_STATUS_OK)
    {
        // wait 1000 ms for NCI response in NVMCallback
        bool wait = sNfaModifyNVM.wait(1000);
        if(!wait)
            ALOGE("%s: read NVM timed out", __FUNCTION__);
    } else {
        ALOGE("%s: read NVM failed", __FUNCTION__);
        return;
    }
}

/******************************************************************************
 **
 ** Function:        setNVM
 **
 ** Description:     write NVM index
 **                  response checked in NVMCallback()
 **
 ** Returns:         None.
 **
 ******************************************************************************/
void setNVM (UINT32 index)
{
    tNFA_STATUS stat = NFA_STATUS_FAILED;
    UINT8   cmd_params_len = 0x09;
    /*
     * NCI command is 2f0104 + index of tag in big endian format + data
     * cmd_params[] to store
     *      sub-opcode[1] (0x04), index[4], data[4]
     * opcode id = 0x01
     */
    UINT8   cmd_params[] = {0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    UINT8   oid_01 = 0x01;

    SyncEventGuard guard(sNfaModifyNVM); // initialize synchronization var

    UINT8 *nvm = (UINT8 *)&sNVMbuffer; //byte access on sNVMbuffer
    UINT32 nlindex = htonl(index); // little --> big endian
    UINT8 *ix = (UINT8 *)&nlindex; // byte access on nlindex

    for(UINT8 i = 1; i < 5; ++i) {
        cmd_params[i] = ix[i-1]; // 1-4, 0-3
    }

    for(UINT8 i = 5; i < 9; ++i) {
        cmd_params[i] = nvm[i-5]; // 5-8, 0-3
    }

    UINT8   *p_cmd_params = cmd_params;

    stat = NFA_SendVsCommand(oid_01, cmd_params_len,
            p_cmd_params, NVMCallback);
    if(stat == NFA_STATUS_OK) {
        // wait 1000 ms for NCI response in NVMCallback
        bool wait = sNfaModifyNVM.wait(1000);
        if(!wait)
            ALOGE("%s: write NVM timed out", __FUNCTION__);
    } else {
        ALOGE("%s: write NVM failed", __FUNCTION__);
        return;
    }
}

/******************************************************************************
 **
 ** Function:        modifyIndexedNVM
 **
 ** Description:     read tag-indexed NVM from NFCC,
 **                  modify NVM accordingly
 **                  write NVM to NFCC
 **
 ** Returns:         None.
 **
 ******************************************************************************/
void modifyIndexedNVM(UINT32 index, UINT8 value) {
    getNVM(index);
    // result is in sNVMbuffer
    UINT8* nvm = (UINT8 *)&sNVMbuffer; // byte access on sNVMbuffer

    if(index == RFPOLLMACP_ENABLE_FEATURE) {
        /*
         * NVM 155: RFPOLLMACP_ENABLE_FEATURE
         */
        sNVM_155 sNVM_155_storage;
        sNVM_155_storage.raw = nvm[3]; // result is in nvm[3]
        sNVM_155_storage.reg.ENABLE_RF_POLL = value;
        nvm[3] = sNVM_155_storage.raw;
    }
    else if(index == RXAGCMAP_RXAGCENABLE)
    {
        sNVM_1763 sNVM_1763_storage;
        sNVM_1763_storage.raw = nvm[3]; // result is in nvm[3]
        sNVM_1763_storage.reg = value;
        nvm[3] = sNVM_1763_storage.raw;
    }
    else
    {
        ALOGE("%s : unsupported NVM: %ld", __FUNCTION__, index);
    }

    setNVM(index);
}

/******************************************************************************
 **
 ** Function:        nfcManager_PrbsOn
 **
 ** Precondition:    NfcService off
 **
 ** Description:     This function initializes the BERT HAL,
 **                  turns the carrier on and sends a PRBS pattern
 **                  with selected technology and rate.
 **
 ** Returns:         true on success,
 **                  false otherwise.
 **
 ******************************************************************************/
jboolean nfcManager_PrbsOn(JNIEnv* e, jobject o, int tech, int rate, bool init)
{
    ALOGD ("%s: enter", __FUNCTION__);

    NfcAdaptation& theInstance = NfcAdaptation::GetInstance();

    tNFA_BERT_COMMAND cmd;

    if(init)
    {
        theInstance.Initialize(); //start GKI, NCI task, NFC task

        theInstance.BertInit();

        /* send carrier on command */
        cmd.cmd = CARRIER_ON;
        theInstance.DoBert(cmd);

    } else {

        /* configure NFCC to send PRBS pattern in supplied technology and bitrate */
        cmd.cmd = BERT_TX;
        cmd.mode = MODE_TX;
        cmd.pollListen = POLL;
        cmd.pattern = PRBS;
        cmd.technology = tech;
        cmd.bitRx = rate;
        cmd.bitTx = rate;
        cmd.sofThreshold = 0x01;
        cmd.bitCount = 0xFFFF;
        UINT32 interPacketTime = htonl(5); // little --> big endian
        cmd.tx.interPacketTime = interPacketTime; // in us
        UINT32 nTotalDuration = htonl(0); // little --> big endian
        cmd.tx.nTotalDuration = nTotalDuration;

        theInstance.DoBert(cmd);

    }

    ALOGD ("%s: exit", __FUNCTION__);
    return JNI_TRUE;
}

/******************************************************************************
 **
 ** Function:        nfcManager_Prbsoff
 **
 ** Description:     This function disables the carrier,
 **                  and gets results (if applicable).
 **                  1.) Disable carrier
 **                  2.) Get Results
 **                  3.) Deinitalize Bert HAL
 **
 ** Returns:         true on success,
 **                  false otherwise.
 **
 ******************************************************************************/
jboolean nfcManager_PrbsOff(JNIEnv* e, jobject o)
{
    ALOGD ("%s: enter", __FUNCTION__);
    NfcAdaptation& theInstance = NfcAdaptation::GetInstance();

    tNFA_BERT_COMMAND cmd;

    /* turn carrier off */
    cmd.cmd = CARRIER_OFF;
    theInstance.DoBert(cmd);

    /* get results */
    cmd.cmd = BERT_GET;
    theInstance.DoBert(cmd);

    theInstance.BertDeInit();
    theInstance.Finalize();

    ALOGD ("%s: exit", __FUNCTION__);
    return JNI_TRUE;
}
/*******************************************************************************
**
** Function:        setPropNciParameter
**
** Description:     Sets Proprietory NCI parameter in nfcc.
**
** Returns:         Status code.
**
*******************************************************************************/
bool setPropNciParameter(uint8_t tag, uint8_t *val, uint8_t len)
{
    tNFA_STATUS status = NFCSTATUS_FAILED;
    SyncEventGuard g(sNfaSetConfigEvent);
    status = NFA_SetConfig(tag, len, val);
    if (status == NFA_STATUS_OK)
    {
        sNfaSetConfigEvent.wait (CRC_PARITY_PARAM_SET_DELAY);
        return true;
    }
    return false;
}
} /* namespace android */
