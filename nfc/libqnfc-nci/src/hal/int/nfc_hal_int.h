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
 *  Copyright (C) 2009-2014 Broadcom Corporation
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
 *  this file contains the NCI transport internal definitions and functions.
 *
 ******************************************************************************/

#ifndef NFC_HAL_INT_H
#define NFC_HAL_INT_H

#include "nfc_hal_target.h"
#include "gki.h"
#include "nci_defs.h"
#include "nfc_brcm_defs.h"
#include "nfc_hal_api.h"
#include "nfc_hal_int_api.h"
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
** NFC HAL TASK transport definitions
****************************************************************************/
/* NFC HAL Task event masks */
#define NFC_HAL_TASK_EVT_DATA_RDY               EVENT_MASK (APPL_EVT_0)
#define NFC_HAL_TASK_EVT_INITIALIZE             EVENT_MASK (APPL_EVT_5)
#define NFC_HAL_TASK_EVT_TERMINATE              EVENT_MASK (APPL_EVT_6)
#define NFC_HAL_TASK_EVT_POWER_CYCLE            EVENT_MASK (APPL_EVT_7)

#define NFC_HAL_TASK_EVT_MBOX                   (TASK_MBOX_0_EVT_MASK)

/* NFC HAL Task mailbox definitions */
#define NFC_HAL_TASK_MBOX                       (TASK_MBOX_0)

/* NFC HAL Task Timer events */
#ifndef NFC_HAL_QUICK_TIMER_EVT_MASK
#define NFC_HAL_QUICK_TIMER_EVT_MASK            (TIMER_0_EVT_MASK)
#endif

#ifndef NFC_HAL_QUICK_TIMER_ID
#define NFC_HAL_QUICK_TIMER_ID                  (TIMER_0)
#endif

/* NFC HAL Task Timer types */
#define NFC_HAL_TTYPE_NCI_WAIT_RSP              0
#define NFC_HAL_TTYPE_POWER_CYCLE               1
#define NFC_HAL_TTYPE_NFCC_ENABLE               2

/* NFC HAL Task Wait Response flag */
#define NFC_HAL_WAIT_RSP_CMD                    0x10    /* wait response on an NCI command                  */
#define NFC_HAL_WAIT_RSP_VSC                    0x20    /* wait response on an NCI vendor specific command  */
#define NFC_HAL_WAIT_RSP_PROP                   0x40    /* wait response on a proprietary command           */
#define NFC_HAL_WAIT_RSP_NONE                   0x00    /* not waiting for anything                         */

typedef UINT8 tNFC_HAL_WAIT_RSP;

typedef UINT16 tNFC_HAL_HCI_EVT;


#define NFC_HAL_HCI_DH_TARGET_HANDLE            0xF2
#define NFC_HAL_HCI_UICC0_TARGET_HANDLE         0xF3
#define NFC_HAL_HCI_UICC1_TARGET_HANDLE         0xF4

#define NFC_HAL_HCI_SESSION_ID_LEN              0x08
#define NFC_HAL_HCI_NETWK_INFO_SIZE             184
#define NFC_HAL_HCI_DH_NETWK_INFO_SIZE          111
#define NFC_HAL_HCI_MIN_NETWK_INFO_SIZE         12
#define NFC_HAL_HCI_MIN_DH_NETWK_INFO_SIZE      11
#define NFC_HAL_HCI_PIPE_INFO_SIZE              5

#define NFC_HAL_HCI_ANY_SET_PARAMETER           0x01
#define NFC_HAL_HCI_ANY_GET_PARAMETER           0x02
#define NFC_HAL_HCI_ADM_NOTIFY_ALL_PIPE_CLEARED 0x15

#define NFC_HAL_HCI_SESSION_IDENTITY_INDEX      0x01
#define NFC_HAL_HCI_WHITELIST_INDEX             0x03

#define NFC_HAL_HCI_ADMIN_PIPE                  0x01
#define NFC_HAL_HCI_HOST_ID_UICC0               0x02        /* Host ID for UICC 0 */
#define NFC_HAL_HCI_HOST_ID_UICC1               0x03        /* Host ID for UICC 1 */
#define NFC_HAL_HCI_HOST_ID_UICC2               0x04        /* Host ID for UICC 2 */
#define NFC_HAL_HCI_COMMAND_TYPE                0x00
#define NFC_HAL_HCI_RESPONSE_TYPE               0x02

/* NFC HAL HCI responses */
#define NFC_HAL_HCI_ANY_OK                      0x00

/* Flag defintions for tNFC_HAL_NVM */
#define NFC_HAL_NVM_FLAGS_NO_NVM                0x01    /* No NVM available                     */
#define NFC_HAL_NVM_FLAGS_LPM_BAD               0x02    /* FPM patch in NVM failed CRC check    */
#define NFC_HAL_NVM_FLAGS_FPM_BAD               0x04    /* LPM patch in NVM failed CRC check    */
#define NFC_HAL_NVM_FLAGS_PATCH_PRESENT         0x08    /* Patch is present in NVM              */

/* NFC HAL transport configuration */
typedef struct
{
    BOOLEAN         shared_transport;           /* TRUE if using shared HCI/NCI transport */
    UINT8           userial_baud;
    UINT8           userial_fc;
} tNFC_HAL_TRANS_CFG;

#ifdef TESTER
#define NFC_HAL_TRANS_CFG_QUALIFIER               /* For Insight, ncit_cfg is runtime-configurable */
#else
#define NFC_HAL_TRANS_CFG_QUALIFIER   const       /* For all other platforms, ncit_cfg is constant */
#endif
extern NFC_HAL_TRANS_CFG_QUALIFIER tNFC_HAL_TRANS_CFG nfc_hal_trans_cfg;

/*****************************************************************************
* BT HCI definitions
*****************************************************************************/
#define BT_HDR      NFC_HDR

/* Tranport message type */
#define HCIT_TYPE_COMMAND   0x01
#define HCIT_TYPE_EVENT     0x04
#define HCIT_TYPE_NFC       0x10

/* Vendor-Specific BT HCI definitions */
#define HCI_SUCCESS                         0x00
#define HCI_GRP_VENDOR_SPECIFIC             (0x3F << 10)            /* 0xFC00 */
#define HCI_BRCM_WRITE_SLEEP_MODE           (0x0027 | HCI_GRP_VENDOR_SPECIFIC)
#define HCI_GRP_HOST_CONT_BASEBAND_CMDS     (0x03 << 10)            /* 0x0C00 */
#define HCI_RESET                           (0x0003 | HCI_GRP_HOST_CONT_BASEBAND_CMDS)
#define HCI_COMMAND_COMPLETE_EVT            0x0E
#define HCI_BRCM_WRITE_SLEEP_MODE_LENGTH    12
#define HCI_BRCM_UPDATE_BAUD_RATE_UNENCODED_LENGTH      0x06
#define HCIE_PREAMBLE_SIZE                  2

/***************************************************************************************
** Battery status file system path
****************************************************************************************/
#define POWER_SUPPLY_PATH_BATTERY_CAPACITY    "/sys/class/power_supply/battery/capacity"
#define POWER_SUPPLY_PATH_BATTERY_PRESENT     "/sys/class/power_supply/battery/present"
#define CHECK_BATTERY_PRESENCE                1
#define CHECK_BATTERY_CAPACITY                2
#define REGION2_DEBUG_DISABLE                 0
#define REGION2_DEBUG_ENABLE                  1
#define ERROR                                 3
#define REGION2_CONTROL_DISABLE               0
#define REGION2_CONTROL_ENABLE                1
#define STORE_INFO_DEBUG_ENABLE               '1'
#define DEVICE_POWER_CYCLED                   '0'
#define REMOVE_INFO                           '0'
#define STORE_INFO_NFC_DISABLED               '2'
#define NFCSERVICE_WATCHDOG_TIMER_EXPIRED     '4'
#define NFCSERVICE_GIVE_UP                    '8'

/****************************************************************************************
** Defnitions used to track the type of shutdown
*****************************************************************************************/
#define NFC_DISABLED_FROM_UI            1
#define NFC_DISABLED_BY_AIRPLANEMODE    2
/****************************************************************************************
** HAL Device Screen States and Prop cmd GID , OID definitions
*****************************************************************************************/
#define PROP_SCREEN_CMD_LEN             0x03
#define AFTER_SCREEN_STATE_CMD_DELAY    50
/* after NCI_WAKE_TRIES report error */
#define NCI_WAKE_TRIES                  3

/****************************************************************************************
** Internal constants and definitions
*****************************************************************************************/

/* NFC HAL receiving states */
enum
{
    NFC_HAL_RCV_IDLE_ST,            /* waiting for packet type byte             */
    NFC_HAL_RCV_NCI_MSG_ST,         /* waiting for the first byte of NCI header */
    NFC_HAL_RCV_NCI_HDR_ST,         /* reading NCI header                       */
    NFC_HAL_RCV_NCI_PAYLOAD_ST,     /* reading NCI payload                      */
    NFC_HAL_RCV_BT_MSG_ST,          /* waiting for the first byte of BT header  */
    NFC_HAL_RCV_BT_HDR_ST,          /* reading BT HCI header                    */
    NFC_HAL_RCV_BT_PAYLOAD_ST       /* reading BT HCI payload                   */
};

/* errors during NCI packet reassembly process */
#define NFC_HAL_NCI_RAS_TOO_BIG             0x01
#define NFC_HAL_NCI_RAS_ERROR               0x02
typedef UINT8 tNFC_HAL_NCI_RAS;

/* NFC HAL power mode */
enum
{
    NFC_HAL_POWER_MODE_FULL,            /* NFCC is full power mode      */
    NFC_HAL_POWER_MODE_LAST
};
typedef UINT8 tNFC_HAL_POWER_MODE;


/* NFC HAL event for low power mode */
enum
{
    NFC_HAL_LP_TX_DATA_EVT,                 /* DH is sending data to NFCC   */
    NFC_HAL_LP_RX_DATA_EVT,                 /* DH received data from NFCC   */
    NFC_HAL_LP_TIMEOUT_EVT,                 /* Timeout                      */
    NFC_HAL_LP_LAST_EVT
};
typedef UINT8 tNFC_HAL_LP_EVT;

#define NFC_HAL_ASSERT_NFC_WAKE      0x00   /* assert NFC_WAKE      */
#define NFC_HAL_DEASSERT_NFC_WAKE    0x01   /* deassert NFC_WAKE    */

#define NFC_HAL_BT_HCI_CMD_HDR_SIZE     3   /* opcode (2) +  length (1)    */
#define NFC_HAL_CMD_TOUT            (5000)  /* timeout for NCI CMD (in ms) */

#define NFC_HAL_SAVED_HDR_SIZE          (2)
#define NFC_HAL_SAVED_CMD_SIZE          (2)

#ifndef NFC_HAL_DEBUG
#define NFC_HAL_DEBUG  TRUE
#endif

#if (NFC_HAL_DEBUG == TRUE)
extern const char * const nfc_hal_init_state_str[];
#define NFC_HAL_SET_INIT_STATE(state)  HAL_TRACE_DEBUG3 ("init state: %d->%d(%s)", nfc_hal_cb.dev_cb.initializing_state, state, nfc_hal_init_state_str[state]); nfc_hal_cb.dev_cb.initializing_state = state;
#else
#define NFC_HAL_SET_INIT_STATE(state)  nfc_hal_cb.dev_cb.initializing_state = state;
#endif
#define FTM_MODE 1

/*****************************************************************************
*  NVM Update related defnitions
*****************************************************************************/
#define NUM_OF_BYTES_ACCESS_FLAG         0x01
#define NUM_OF_BYTES_START_ADDR          0x04
#define NUM_OF_BYTES_NUMBER_OF_ITEMS     0x01
#define NUM_OF_BYTES_ADD_DELTA           0x01
#define NUM_OF_BYTES_ACCESS_DELAY        0x02
#define NUM_OF_BYTES_READ_SUB_OPCODE     0x02
#define NUM_OF_BYTES_WRITE_SUB_OPCODE    0x01

/*Defnitions for NFCC version and metal revision*/
#define NFCC_METAL_REV_0                 0x00
#define NFCC_METAL_REV_1                 0x01
#define NFCC_METAL_REV_4                 0x04

/*Defnitions for NFCC chip version major and minor*/
#define QCA1990_CHIP_VERSION_MINOR       0x00
#define QCA1990A_CHIP_VERSION_MINOR      0x01

enum
{
    QCA1990 = 1,
    QCA1990A
};
/* NFCC CHIP VERSION*/
enum
{
    NFCC_CHIP_VERSION_LEN = 1,
    NFCC_CHIP_VERSION_2,
    NFCC_CHIP_VERSION_3,
    NFCC_CHIP_QCA1990A_VERSION_1 = 1,
    NFCC_CHIP_QCA1990A_VERSION_2 = 2
};


/* NFC HAL - NFCC initializing state */
enum
{
    NFC_HAL_INIT_STATE_IDLE,               /* Initialization is done                */
    NFC_HAL_INIT_STATE_W4_XTAL_SET,        /* Waiting for crystal setting rsp       */
    NFC_HAL_INIT_STATE_POST_XTAL_SET,      /* Waiting for reset ntf after xtal set  */
    NFC_HAL_INIT_STATE_W4_NFCC_ENABLE,     /* Waiting for reset ntf atter REG_PU up */
    NFC_HAL_INIT_STATE_W4_BUILD_INFO,      /* Waiting for build info rsp            */
    NFC_HAL_INIT_STATE_W4_PATCH_INFO,      /* Waiting for patch info rsp            */
    NFC_HAL_INIT_STATE_W4_APP_COMPLETE,    /* Waiting for complete from application */
    NFC_HAL_INIT_STATE_W4_POST_INIT_DONE,  /* Waiting for complete of post init     */
    NFC_HAL_INIT_STATE_W4_CONTROL_DONE,    /* Waiting for control release           */
    NFC_HAL_INIT_STATE_W4_PREDISCOVER_DONE,/* Waiting for complete of prediscover   */
    NFC_HAL_INIT_STATE_CLOSING,            /* Shutting down                         */
    NFC_HAL_INIT_FOR_PATCH_DNLD,           /* hal init for patch download to happen */
    NFC_HAL_INIT_STATE_W4_RE_INIT,         /* Waiting for reset rsp on ReInit       */
    NFC_HAL_INIT_STATE_RAMDUMP,            /* NFCC reset. Ramdump initiation        */
    NFC_HAL_INIT_STATE_NFC_HAL_INIT        /* HAL-NFCC upload of initialisation data*/
};

typedef struct
{
    /*nvm update related */
    FILE  *p_Nvm_file;
    UINT16 no_of_updates;
    UINT8 nvm_updated;
    BOOLEAN last_cmd_sent;
} tNFC_HAL_NVM_CB;


typedef UINT8 tNFC_HAL_INIT_STATE;

/* NFC HAL - NFCC config items during post initialization */
enum
{
    NFC_HAL_DM_CONFIG_LPTD,
    NFC_HAL_DM_CONFIG_PLL_325,
    NFC_HAL_DM_CONFIG_START_UP,
    NFC_HAL_DM_CONFIG_I93_DATA_RATE,
    NFC_HAL_DM_CONFIG_FW_FSM,
    NFC_HAL_DM_CONFIG_START_UP_VSC,
    NFC_HAL_DM_CONFIG_NONE
};
typedef UINT8 tNFC_HAL_DM_CONFIG;

/* callback function prototype */
typedef struct
{
    UINT16  opcode;
    UINT16  param_len;
    UINT8   *p_param_buf;
} tNFC_HAL_BTVSC_CPLT;

typedef void (tNFC_HAL_BTVSC_CPLT_CBACK) (tNFC_HAL_BTVSC_CPLT *p1);


/* data type for NFC_HAL_HCI_RSP_NV_READ_EVT */
typedef struct
{
    NFC_HDR           hdr;
    UINT8             block;
    UINT16            size;
    tHAL_NFC_STATUS   status;
} tNFC_HAL_HCI_RSP_NV_READ_EVT;

/* data type for NFC_HAL_HCI_RSP_NV_WRITE_EVT */
typedef struct
{
    NFC_HDR           hdr;
    tHAL_NFC_STATUS   status;
} tNFC_HAL_HCI_RSP_NV_WRITE_EVT;


/* union of all event data types */
typedef union
{
    NFC_HDR                         hdr;
    /* Internal events */
    tNFC_HAL_HCI_RSP_NV_READ_EVT    nv_read;
    tNFC_HAL_HCI_RSP_NV_WRITE_EVT   nv_write;
} tNFC_HAL_HCI_EVENT_DATA;

/*****************************************************************************
** Control block for NFC HAL
*****************************************************************************/

/* Patch RAM Download Control block */

/* PRM states */
enum
{
    NFC_HAL_PRM_ST_IDLE,

    /* Secure patch download stated */
    NFC_HAL_PRM_ST_SPD_COMPARE_VERSION,
    NFC_HAL_PRM_ST_SPD_GET_PATCH_HEADER,
    NFC_HAL_PRM_ST_SPD_DOWNLOADING,
    NFC_HAL_PRM_ST_SPD_AUTHENTICATING,
    NFC_HAL_PRM_ST_SPD_AUTH_DONE,
    NFC_HAL_PRM_ST_W4_GET_VERSION
};
typedef UINT8 tNFC_HAL_PRM_STATE;

/* Maximum number of patches (currently 2: LPM and FPM) */
#define NFC_HAL_PRM_MAX_PATCH_COUNT    2
#define NFC_HAL_PRM_PATCH_MASK_ALL     0xFFFFFFFF
#define NFC_HAL_PRM_MAX_CHIP_VER_LEN   8

/* Structures for PRM Control Block */
typedef struct
{
    UINT8               power_mode;
    UINT16              len;
} tNFC_HAL_PRM_PATCHDESC;

typedef struct
{
    tNFC_HAL_PRM_STATE  state;                  /* download state */
    UINT32              flags;                  /* internal flags */
    UINT16              cur_patch_len_remaining;/* bytes remaining in patchfile to process     */
    const UINT8*        p_cur_patch_data;       /* pointer to patch currently being downloaded */
    UINT16              cur_patch_offset;       /* offset of next byte to process              */
    UINT32              dest_ram;
    TIMER_LIST_ENT      timer;                  /* Timer for patch download                    */

    /* Secure Patch Download */
    UINT32              spd_patch_needed_mask;  /* Mask of patches that need to be downloaded */
    UINT8               spd_patch_count;        /* Number of patches left to download */
    UINT8               spd_cur_patch_idx;      /* Current patch being downloaded */

    tNFC_HAL_PRM_PATCHDESC spd_patch_desc[NFC_HAL_PRM_MAX_PATCH_COUNT];

    /* I2C-patch */
    UINT8               *p_spd_patch;           /* pointer to spd patch             */
    UINT16              spd_patch_len_remaining;/* patch length                     */
    UINT16              spd_patch_offset;       /* offset of next byte to process   */

    tNFC_HAL_PRM_FORMAT format;                 /* format of patch ram              */
    tNFC_HAL_PRM_CBACK  *p_cback;               /* Callback for download status notifications */
    UINT32              patchram_delay;         /* the dealy after patch */
} tNFC_HAL_PRM_CB;

/* Information about current patch in NVM */
typedef struct
{
    UINT16              project_id;             /* Current project_id of patch in nvm       */
    UINT16              ver_major;              /* Current major version of patch in nvm    */
    UINT16              ver_minor;              /* Current minor version of patch in nvm    */
    UINT16              fpm_size;               /* Current size of FPM patch in nvm         */
    UINT16              lpm_size;               /* Current size of LPM patch in nvm         */
    UINT8               flags;                  /* See NFC_HAL_NVM_FLAGS_* flag definitions */
    UINT8               nvm_type;               /* Current NVM Type - UICC/EEPROM           */
    UINT8               chip_ver[NFC_HAL_PRM_MAX_CHIP_VER_LEN]; /* patch chip version       */
} tNFC_HAL_NVM;

/* Patch for I2C fix */
typedef struct
{
    UINT8               *p_patch;               /* patch for i2c fix                */
    UINT32              prei2c_delay;           /* the dealy after preI2C patch */
    UINT16              len;                    /* i2c patch length                 */
} tNFC_HAL_PRM_I2C_FIX_CB;

/* Control block for NCI transport */
typedef struct
{
    UINT8               nci_ctrl_size;      /* Max size for NCI messages                              */
    UINT8               rcv_state;          /* current rx state                                       */
    UINT16              rcv_len;            /* bytes remaining to be received in current rx state     */
    NFC_HDR             *p_rcv_msg;         /* buffer to receive NCI message                          */
    NFC_HDR             *p_frag_msg;        /* fragmented NCI message; waiting for last fragment      */
    NFC_HDR             *p_pend_cmd;        /* pending NCI message; waiting for NFCC state to be free */
    tNFC_HAL_NCI_RAS    nci_ras;            /* nci reassembly error status                            */
    TIMER_LIST_ENT      nci_wait_rsp_timer; /* Timer for waiting for nci command response             */
    tNFC_HAL_WAIT_RSP   nci_wait_rsp;       /* nci wait response flag                                 */
    UINT8               last_hdr[NFC_HAL_SAVED_HDR_SIZE];/* part of last NCI command header           */
    UINT8               last_cmd[NFC_HAL_SAVED_CMD_SIZE];/* part of last NCI command payload          */
    void                *p_vsc_cback;       /* the callback function for last VSC command             */
    BOOLEAN             hw_error;           /* hardware error status: timeout or no nfcc*/
} tNFC_HAL_NCIT_CB;

#define SLEEP_STATE     1
#define WAKE_STATE      0

/*
 * Data structure for sleep and screen states of NFCC
 */
typedef union
{
    struct
    {
        UINT8 CMD_SLEEP : 1; /* NFCC asleep by command */
        UINT8 W4_SLEEP_RSP : 1; /* wait for sleep response from NFCC */
        UINT8 NFCC_FIELD_WAKE : 1; /* NFCC awake from field */
        UINT8 CMD_SLEEP_JNI : 1; /* Sleep command by JNI */

        UINT8 IS_SCREEN_UNLOCKED : 1;
        UINT8 IS_SCREEN_LOCKED : 1;
        UINT8 IS_SCREEN_OFF : 1;
        UINT8 SCREEN_CMD : 1; /* Any screen cmd, enforce NCI wake */
    } state;
    UINT8 raw; /* full byte */
} tNFC_HAL_SLEEP;

/* Control block for device initialization */
typedef struct
{
    tNFC_HAL_INIT_STATE     initializing_state;     /* state of initializing NFCC               */

    UINT32                  m_hw_id;                /* NFCC HW ID                          */
    tNFC_HAL_DM_CONFIG      next_dm_config;         /* next config in post initialization       */
    UINT8                   next_startup_vsc;       /* next start-up VSC offset in post init    */

    tNFC_HAL_POWER_MODE     power_mode;             /* NFCC power mode                          */
    UINT8                   snooze_mode;            /* current snooze mode                      */
    UINT8                   new_snooze_mode;        /* next snooze mode after receiving cmpl    */
    UINT8                   nfc_wake_active_mode;   /* NFC_HAL_LP_ACTIVE_LOW/HIGH               */
    TIMER_LIST_ENT          lp_timer;               /* timer for low power mode                 */


    tHAL_NFC_STATUS_CBACK   *p_prop_cback;          /* callback to notify complete of proprietary update */
    UINT8                    patch_applied;
    UINT8                    fw_version_chk;                  /*Flag to indicate if FW version check is done or not */
    UINT8                    pre_patch_signature_chk;         /*Flag to indicate if pre patch signature check is done or not */
    UINT8                    patch_signature_chk;             /*Flag to indicate if patch signature check is done or not */
    UINT8                    pre_patch_applied;               /* Flag to keep track if pre patch applied or not */
    UINT8                    pre_init_done;                   /* Flag to keep track on pre init process completion in case of patch dnld*/
    UINT32                   patch_data_offset;               /* Offset of patch data*/
    UINT32                   number_of_patch_data_buffers;
    UINT8                    pre_patch_file_available;       /* This flag will be set if pre patch file is available*/
    UINT8                    patch_file_available;           /* This flag will be set if patch file is available*/
    UINT8                    patch_dnld_conn_close_delay;    /* This flag will enable 4 sec or configured delay for the conn close after
                                                                patch is downloaded*/
    UINT8                    nfcc_chip_version;               /* Chip version used in propogating patch*/
    UINT8                    nfcc_chip_metal_version;         /* Chip Metal version used in propogating patch*/
    UINT8                    store_path;                      /* flag to keep track whether path of relevent patch file as per NFCC version
                                                                 is stored or not */
    UINT8                    efuse_value;                     /*value 1= fused device value 0 = unfused device*/
    UINT8                    nfcc_chip_type;                  /* if nfcc is qca1990 or qca1990A*/
    tNFC_HAL_SLEEP           sleep;
} tNFC_HAL_DEV_CB;

/* data members for NFC_HAL-HCI */
typedef struct
{
    TIMER_LIST_ENT          hci_timer;                /* Timer to avoid indefinitely waiting for response */
    UINT8                   *p_hci_netwk_info_buf;    /* Buffer for reading HCI Network information */
    UINT8                   *p_hci_netwk_dh_info_buf; /* Buffer for reading HCI Network DH information */
    UINT8                   hci_netwk_config_block;   /* Rsp awaiting for hci network configuration block */
    BOOLEAN                 b_wait_hcp_conn_create_rsp; /* Waiting for hcp connection create response */
    BOOLEAN                 clear_all_pipes_to_uicc1; /* UICC1 was restarted for patch download */
    BOOLEAN                 update_session_id;        /* Next response from NFCC is to Get Session id cmd */
    BOOLEAN                 hci_fw_workaround;        /* HAL HCI Workaround need */
    BOOLEAN                 hci_fw_validate_netwk_cmd;/* Flag to indicate if hci network ntf to validate */
    UINT8                   hcp_conn_id;              /* NCI Connection id for HCP */
    UINT8                   dh_session_id[1];         /* Byte 0 of DH Session ID */
} tNFC_HAL_HCI_CB;


typedef struct
{
    tHAL_NFC_CBACK          *p_stack_cback;     /* Callback for HAL event notification  */
    tHAL_NFC_DATA_CBACK     *p_data_cback;      /* Callback for data event notification  */

    TIMER_LIST_Q            quick_timer_queue;  /* timer list queue                 */
    TIMER_LIST_ENT          timer;              /* timer for NCI transport task     */

    tNFC_HAL_NCIT_CB        ncit_cb;            /* NCI transport */
    tNFC_HAL_DEV_CB         dev_cb;             /* device initialization */
    tNFC_HAL_NVM            nvm_cb;             /* Information about current patch in NVM */

    /* Patchram control block */
    tNFC_HAL_PRM_CB         prm;
    tNFC_HAL_PRM_I2C_FIX_CB prm_i2c;

    /* data members for NFC_HAL-HCI */
    tNFC_HAL_HCI_CB         hci_cb;

    UINT8                   pre_discover_done;  /* TRUE, when the prediscover config is complete */

    UINT8                   max_rf_credits;     /* NFC Max RF data credits */
    UINT8                   max_ee;             /* NFC Max number of NFCEE supported by NFCC */
    UINT8                   trace_level;        /* NFC HAL trace level */
    tNFC_HAL_NVM_CB         nvm;                /* NVM update CB(control block)*/

    UINT8                   deact_type;
    UINT8                   listen_setConfig_rsp_cnt;
    UINT8                   act_interface;
    UINT8                   listen_mode_activated;
    BOOLEAN                 poll_activated_event;/*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */
    BOOLEAN                 sent_uioff;/*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */
    BOOLEAN                 poll_locked_mode;
    UINT8                   kovio_activated;
    UINT8                   pending_screencmd;/*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */


} tNFC_HAL_CB;

/****************************************************************************
** NFCC information
****************************************************************************/
/*param len*/
#define CHIP_VERSION_LEN             2
#define FW_VERSION_LEN               2

/*param ids*/
#define PARAM_NFCC_MFG_ID            1
#define PARAM_MFG_SPECIFIC_INFO      2
#define PARAM_NCI_VERSION            3
#define PARAM_FW_PATCH_APPLIED       4
#define PARAM_FW_PATCH_VERSION       5
#define PARAM_EFUSE_VALUE            6
#define PARAM_NFCC_FEATURES          7

/*NFCC metal revision defnitions*/
#define NFCC_CHIP_METAL_VERSION_0    0
#define NFCC_CHIP_METAL_VERSION_1    1
#define NFCC_CHIP_METAL_VERSION_4    4

/*NFCC versions*/
#define NFCC_V_20                    20
#define NFCC_V_21                    21
#define NFCC_V_24                    24
#define NFCC_V_30                    30

/* Manufacturer specific info*/
typedef struct{
    UINT8 type;
    UINT8 len;
    UINT8 chip_version[CHIP_VERSION_LEN];
    UINT8 fw_version[FW_VERSION_LEN];
} mfg_specific_info_tlv;

/* NFCC feature info*/
typedef struct{
    UINT8 type;
    UINT8 len;
    UINT8 disc_config_info;
    UINT8 routing_info;
    UINT8 state;
} nfcc_features_info_tlv;

/*firmware patch version*/
typedef struct{
    UINT8 type;
    UINT8 len;
    UINT8 patch_version[HAL_NFC_LEN_NFCC_PATCH_VERSION];
} fw_patch_version_tlv;

typedef struct{
    UINT8 type;
    UINT8 len;
    UINT8 value;
} nfcc_info_single_tlv;

/* data structures for nfcc info*/
typedef struct{
    UINT8                   num_of_tlvs;                            /* number of TLVs == 6                   */
    nfcc_info_single_tlv    nfcc_mfg_id;                            /* nfc controller Manufacturer ID        */
    mfg_specific_info_tlv   mfg_info;                               /* Manufactturer specific information
                                                                     e.g  chip version     LSB 2 bytes
                                                                          firmware version MSB 2 bytes      */

    nfcc_info_single_tlv    nci_version;                           /* NCI version supported by nfcc         */
    nfcc_info_single_tlv    fw_patch_applied;                      /* TRUE = patch applied successfully
                                                                      FALSE = patch application failed      */
    fw_patch_version_tlv    fw_patch_version;                      /* Patch version which has been applied
                                                                      to nfcc                               */
    nfcc_info_single_tlv    efuse_tlv;                             /* Device type fused or unfused          */
    nfcc_features_info_tlv  nfcc_features;                         /* Discovery config modes, frequency
                                                                      routing and power information         */
}nfc_hal_nfcc_info_CB;


/*****************************************************************************/

/* Global NCI data */
#if NFC_DYNAMIC_MEMORY == FALSE
extern tNFC_HAL_CB   nfc_hal_cb;
#else
#define nfc_hal_cb (*nfc_hal_cb_ptr)
extern tNFC_HAL_CB *nfc_hal_cb_ptr;
#endif
extern UINT8 *p_nfc_hal_pre_discover_cfg;

/****************************************************************************
** Internal nfc functions
****************************************************************************/

/* From nfc_hal_main.c */
UINT32 nfc_hal_main_task (UINT32 param);
void   nfc_hal_main_init (void);
void   nfc_hal_main_pre_init_done (tHAL_NFC_STATUS);
void   nfc_hal_main_start_quick_timer (TIMER_LIST_ENT *p_tle, UINT16 type, UINT32 timeout);
void   nfc_hal_main_stop_quick_timer (TIMER_LIST_ENT *p_tle);
void   nfc_hal_main_send_error (tHAL_NFC_STATUS status);
void   nfc_hal_send_nci_msg_to_nfc_task (NFC_HDR * p_msg);
void   check_patch_version(UINT32 *version);
void   nfc_hal_main_collect_nfcc_info(UINT8 param_id,UINT8 *p,UINT8 param_len);
#ifdef DTA
BOOLEAN nfc_hal_in_dta_mode(void);
long int nfc_hal_dta_read_pattern_from_file(void);
#endif
/* nfc_hal_nci.c */
BOOLEAN nfc_hal_nci_receive_msg (void);
BOOLEAN nfc_hal_nci_preproc_rx_nci_msg (NFC_HDR *p_msg);
void    nfc_hal_handle_initialisation_events(NFC_HDR *p_msg);
NFC_HDR* nfc_hal_nci_postproc_rx_nci_msg (void);
void    nfc_hal_nci_assemble_nci_msg (void);
void    nfc_hal_nci_add_nfc_pkt_type (NFC_HDR *p_msg);
void    nfc_hal_nci_send_cmd (NFC_HDR *p_buf);
void    nfc_hal_nci_cmd_timeout_cback (void *p_tle);
void    nfc_hal_dm_send_prop_sleep_cmd (void);
int     nfc_hal_dm_get_nfc_sleep_state (void);
void    nfc_hal_fake_screen_rsp();
BOOLEAN initNfcc(NFC_HDR *p_msg);
/* nfc_hal_dm.c */
void nfc_hal_dm_init (void);
void nfc_hal_dm_set_xtal_freq_index (void);
void nfc_hal_dm_send_get_build_info_cmd (void);
void nfc_hal_dm_proc_msg_during_init (NFC_HDR *p_msg);
void nfc_hal_dm_config_nfcc (void);
void nfc_hal_dm_send_nci_cmd (const UINT8 *p_data, UINT16 len, tNFC_HAL_NCI_CBACK *p_cback);
void nfc_hal_dm_send_bt_cmd (const UINT8 *p_data, UINT16 len, tNFC_HAL_BTVSC_CPLT_CBACK *p_cback);
void nfc_hal_dm_set_nfc_wake (UINT8 cmd);
void nfc_hal_dm_pre_init_nfcc (void);
void nfc_hal_dm_shutting_down_nfcc (void);
BOOLEAN nfc_hal_dm_power_mode_execute (tNFC_HAL_LP_EVT event);
void nfc_hal_dm_send_pend_cmd (void);
tHAL_NFC_STATUS nfc_hal_dm_set_config (UINT8 tlv_size, UINT8 *p_param_tlvs, tNFC_HAL_NCI_CBACK *p_cback);
void nfc_hal_dm_send_prop_init_ramdump_cmd (void);
void nfc_hal_dm_send_prop_get_ramdump_cmd (int ramdump_start_addr, int ramdump_length);
void nfc_hal_dm_send_prop_end_ramdump_cmd (void);
void nfc_hal_dm_send_prop_reset_nfcc_ramdump_poke (void);
BOOLEAN nfc_hal_dm_send_prop_nfcc_storage_cmd (const UINT8 *payload_ptr, BOOLEAN send_it, int *bytes_remaining);
void nfc_hal_store_info(UINT8 operation);
void nfc_hal_dm_send_prop_nci_region2_enable_cmd(UINT8 debug_status);
char nfc_hal_retrieve_info(void);
void nfc_hal_dm_send_prop_nci_region2_control_enable_cmd(UINT8 debug_status);
BOOLEAN nfc_hal_dm_is_nfcc_awake (void);
BOOLEAN nfc_hal_dm_is_field_wake(void);
void nfc_hal_dm_update_field_wake(BOOLEAN present);
void nfc_hal_dm_update_screen_flags(UINT8 screen_state);

/* nfc_hal_prm.c */
void nfc_hal_prm_spd_reset_ntf (UINT8 reset_reason, UINT8 reset_type);
void nfc_hal_prm_nci_command_complete_cback (tNFC_HAL_NCI_EVT event, UINT16 data_len, UINT8 *p_data);
void nfc_hal_prm_process_timeout (void *p_tle);
int nfc_hal_dm_check_nvm_file(UINT8 *nvmupdatebuff,UINT8 *nvmupdatebufflen);
void nfc_hal_dm_frame_mem_access_cmd(UINT8 *nvmpokecmd,UINT8 *nvmupdatebuff,UINT8 *nvmpokecmdlen);
BOOLEAN nfc_hal_dm_check_fused_nvm_file(UINT8 *nvmupdatebuff,UINT8 *nvmupdatebufflen);
void nfc_hal_dm_frame_fused_mem_access_cmd(UINT8 *nvmcmd,UINT8 *nvmupdatebuff,UINT8 *nvmcmdlen);

/* nfc_hal_hci.c */
void nfc_hal_hci_enable (void);
void nfc_hal_hci_evt_hdlr (tNFC_HAL_HCI_EVENT_DATA *p_evt_data);
void nfc_hal_hci_handle_hci_netwk_info (UINT8 *p_data);
void nfc_hal_hci_handle_hcp_pkt_from_hc (UINT8 *p_data);
NFC_HDR* nfc_hal_hci_postproc_hcp (void);
BOOLEAN nfc_hal_hci_handle_hcp_pkt_to_hc (UINT8 *p_data);
void nfc_hal_hci_timeout_cback (void *p_tle);


/* Define default NCI protocol trace function (if protocol tracing is enabled) */
#if (defined(NFC_HAL_TRACE_PROTOCOL) && (NFC_HAL_TRACE_PROTOCOL == TRUE))
#if !defined (DISP_NCI)
#define DISP_NCI    (DispNci)
void DispNci (UINT8 *p, UINT16 len, BOOLEAN is_recv);
#endif  /* DISP_NCI */

#if !defined (DISP_NCI_FAKE)
#define DISP_NCI_FAKE (DispNciFake)
void DispNciFake (UINT8 *data, UINT16 len, BOOLEAN is_recv);
#endif /* DISP_NCI_FAKE */

/* For displaying vendor-specific HCI commands */
void DispHciCmd (BT_HDR *p_buf);
void DispHciEvt (BT_HDR *p_buf);
#endif /* NFC_HAL_TRACE_PROTOCOL */

#ifdef __cplusplus
}
#endif

#endif /* NFC_HAL_INT_H */
