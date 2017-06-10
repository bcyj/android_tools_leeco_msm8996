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
 *  Copyright (C) 2010-2014 Broadcom Corporation
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

/*
 * We use NDEBUG to hide some debug traces and asserts. We use macros to output some additional information.
 * Use '#ifndef NDEBUG' to turn off additional logging.
 * Note: ramdump is not a production level feature.
 */
#ifdef NDEBUG
#undef NDEBUG
#define RAMDUMP
#define NFCC_STORAGE
#endif

#define T3T_EMU_ON_DH 0


#ifndef RAMDUMP_TEST_POKE
//#define RAMDUMP_TEST_POKE
#endif

#ifndef NFCC_STORAGE_TEST
//#define NFCC_STORAGE_TEST
#endif

/******************************************************************************
 *
 *  This file contains function of the NFC unit to receive/process NCI/VS
 *  commands/responses.
 *
 ******************************************************************************/
#include <errno.h>
#include <string.h>
#include <assert.h>
//#include <stringl/stringl.h>
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <cutils/properties.h>
#include "nfc_hal_int.h"
#include "nfc_hal_post_reset.h"
#include "userial.h"
#include "nci_defs.h"
#include "config.h"
#include <limits.h>
#include <sched.h> /* for sched_yield */

#include <DT_Nfc_link.h>
#include <DT_Nfc_types.h>
#include <DT_Nfc_status.h>
#include <DT_Nfc_i2c.h>
#include <DT_Nfc_log.h>
#include <DT_Nfc.h>
#ifdef DTA // <DTA>
#include "dta_flag.h"
#define LISTEN_MASK 8
#endif // </DTA>


/* UICC soft reset */
#include "qmi_client_instance_defs.h"
#include "user_identity_module_v01.h"



/*****************************************************************************
** Constants and types
*****************************************************************************/
extern char current_mode;
extern UINT8 wait_reset_rsp;
extern UINT8 reset_status;

/*
 * NFCC Storage constants
 */

enum tNfcc_Tlv
{
    /* Number of TLV header bytes */
    Nfcc_Storage_TlvHeaderBytes        = 2,
    /* Number of TLV 'Type' bytes */
    Nfcc_Storage_TlvTypeBytes          = 1,
    /* Number of TLV 'Length' bytes */
    Nfcc_Storage_TlvLengthBytes        = 1,
    /* Number of TLV 'Number of Type' bytes */
    Nfcc_Storage_TlvTypesCountBytes    = 1,
    /* TLV type representing a delete */
    Nfcc_Storage_TlvDeleteType         = 0xFF,
    /* Bit mask for the Most Significant Bit to aid header mask processing */
    Nfcc_Storage_Tlv_MSB               = 0x80
};

/* Maximum item type value, offset from zero. See NCI Notes, NFCC DH Storage Messages */
#define TLV_TYPE_MAX_COUNT ((0x2B) + 1)
/* Mask length in bytes */
#define TLV_MASK_LENGTH(tid) ((tid / CHAR_BIT) + 1)
/* Maximum TLV value length that fits in a control message NTF, i.e. one entry =249 */
#define TLV_MAX_VALUE_LENGTH (NFC_HAL_NCI_INIT_CTRL_PAYLOAD_SIZE - NCI_MSG_HDR_SIZE - Nfcc_Storage_TlvTypeBytes - Nfcc_Storage_TlvLengthBytes)

#define WORD_SIZE (LONG_BIT / CHAR_BIT)
/* divisible by WORD_SIZE for alignment */
#define TLV_STRUCT_PAD_BYTES ((WORD_SIZE - ((TLV_MAX_VALUE_LENGTH + Nfcc_Storage_TlvTypeBytes + Nfcc_Storage_TlvLengthBytes) % WORD_SIZE)) % WORD_SIZE)
/* divisible by WORD_SIZE for alignment */
#define TLV_HEADER_PAD_BYTES ((WORD_SIZE - (TLV_MASK_LENGTH(TLV_TYPE_MAX_COUNT) % WORD_SIZE)) % WORD_SIZE)

#define CONSTRUCT_MSG_TYPE_SHORT(mt, gid, opcode) ((UINT16)((((mt << NCI_MT_SHIFT) | gid) << 8) | (opcode)))
/*
 * NFCC Storage file header
 */
typedef struct __attribute__ ((__packed__)) sNfccStorageFileHeader /* packed to minimise memory usage */
{
    UINT8 item_mask[TLV_MASK_LENGTH(TLV_TYPE_MAX_COUNT)];  /* Mask indicating if item is present or deleted */
    UINT8 pad[TLV_HEADER_PAD_BYTES];                       /* Align */

} tNfccStorageFileHeader __attribute__ ((__aligned__(WORD_SIZE))); /* aligned to allow for run-time efficiency in copying (if supported) */
/*
 * NFCC Storage TLV entry structure
 */
typedef struct __attribute__ ((__packed__)) sTlvEntry      /* packed to minimise memory usage */
{
    UINT8 type;                                            /* Type for item 1 TLV (see below) */
    UINT8 valueLength;                                     /* Length for TLV Value */
    UINT8 value[TLV_MAX_VALUE_LENGTH];                     /* Value for TLV */
    UINT8 pad[TLV_STRUCT_PAD_BYTES];                       /* Align */

} tTlvEntry __attribute__ ((__aligned__(WORD_SIZE)));      /* aligned to allow for run-time efficiency in copying (if supported) */



enum tNfcc_FilePersistence
{
    /* Levels of recursion allowed */
    Nfcc_Storage_RecursionLevel             = 20,
    /* Value used to indicate an empty or processed storage buffer */
    Nfcc_Storage_No_Unprocessed_Entries     = 1,
    /* Error from nfc_hal_dm_send_prop_nfcc_storage_cmd() indicating the passed TLV Entry is too large for a packet */
    Nfcc_Storage_PayloadTooLarge            = -1,
    /* Error from nfc_hal_nci_initiate_nfcc_storage_check() indicating the result of checking for further persistence entries */
    Nfcc_Storage_NoPersistentFile           = -1,
    Nfcc_Storage_AllRecordsProcessed        = -2,
    Nfcc_Storage_NoUnprocessedEntriesFound  = -3,
    Nfcc_Storage_ProcessingOK               = 0
};

enum tNfcc_Test
{
    Nfcc_Storage_TestTypeCount  = 20,
    Nfcc_Storage_TestTypeStart  = 2,
    Nfcc_Storage_TestLength     = 0x23  /* 35 bytes, so 7 entries per packet before overflow occurs */
};

enum tNvm_Error
{
    Nvm_MemoryAllocationFailure = -1
};

typedef enum
{
    /* -- These entries can be configured from the config file so must match */
    Nfcc_Storage_ManyEntriesPerPacket = 0,
    Nfcc_Storage_OneEntryPerPacket    = 1,
    /* -- These cannot */
    Nfcc_Storage_NfccResetInProgress  = 2
} tNfcc_Check_Params;

/* Default of how we should package the entries into a packet, one per packet or many? */
static tNfcc_Check_Params gNfcc_Storage_CheckParam = Nfcc_Storage_ManyEntriesPerPacket;

#define NFCC_STORAGE_FILENAME_LEN 32
static char kNfcc_storage_filename[NFCC_STORAGE_FILENAME_LEN];
static FILE * gNfcc_storage_file_ptr = NULL;
static tNfccStorageFileHeader gNfccStorageFileHeader;
static BOOLEAN gTlvEnabled = FALSE;                        /* Indicates we can persist the TLV data. */
static size_t gNfcc_storage_filesize = 0;

/*
 * Identifies RAMDUMP NTF and RSPs' received from NFCC
 */
typedef enum {
    RamDump_Recv_Init_Ntf = 0,
    RamDump_Recv_Init_Rsp,
    RamDump_Recv_Get_Rsp,
    RamDump_Recv_End_Rsp,
    RamDump_Item_Count /* must remain last item in enum list */
} RamDumpState;

/*
 * Ramdump config file name and location
 */
static const char kRamdump_config_filename[] = "/etc/nfc/hardfault.cfg";

/*
 * Number of NFCC registers to obtain information on. Depends on chip version.
 * Default is version 2.1 with 20 registers.
 */
static UINT16 gMaxRegisterCount = 20;
static UINT16 gNfccChipVersion = 21;
extern uint8_t DT_Nfc_RamdumpPerformed;

#define REGISTER_FIELD_LENGTH 4
#define MAX_CONFIG_NAME 256
#define MAX_RECV_PACKET_SIZE 255
/*
 * Link list node of Dump Data blocks.
 */
typedef struct ConfigData_
{
    struct ConfigData_* next;
    /* obtained from config file */
    unsigned int dump_start_addr;
    int dump_length;
    char name[MAX_CONFIG_NAME];
} ConfigData;

/*
 * The Configuration params for a RAMDUMP
 * Ensure (int) ordering so no unnecessary padding bytes
 */
typedef struct
{
    BOOLEAN ramdump_in_progress; /* signifies that this structure is in use */
    FILE* output_file_ptr;
    UINT8* data;
    int data_length;
    int bytes_requested;
    int total_received;
    UINT8* config_buffer;
    UINT8* register_buffer;
    /* no ownership of payload data */
    const UINT8* msg_payload_ptr;
    UINT8 msg_payload_length;
    ConfigData* cd_list;
    ConfigData* cd_current_item;
} RamDump_Data;

static RamDump_Data gRamdump_Data = {0};

/* To get the reason of shut down from nfcservice*/
extern char shut_down_reason;


/*****************************************************************************
** Local function prototypes
*****************************************************************************/

/* Prototype for processing NFCC STORAGE events */
int nfc_hal_nci_initiate_nfcc_storage_check(tNfcc_Check_Params send_now);
static int nfc_hal_nci_open_and_read_nfcc_storage_file(UINT8** store_buffer_pptr);
static void nfc_hal_nci_recv_nfcc_storage_ntf(int payload_len, UINT8 * const payload_ptr);
static void nfc_hal_nci_persist_nfcc_storage_entry(int payload_len, UINT8 * const payload_ptr);
static int nfc_hal_nci_recv_nfcc_storage_rsp(UINT8 reason, BOOLEAN send_now);
static void recreate_hci_persistence_file();
void nfc_hal_nci_close_nfcc_storage_file();

/* Prototype for processing RamDump events */
#ifndef NDEBUG
void nfc_hal_nci_handle_ramdump_ntf(int pbf, int payload_len, int reason);
static void nfc_hal_nci_handle_ramdump_rsp(int mt, int op_code, int pbf, int payload_len, UINT8 * const payload_ptr);
#else
void nfc_hal_nci_handle_ramdump_ntf();
static void nfc_hal_nci_handle_ramdump_rsp(int op_code, int payload_len, UINT8 * const payload_ptr);
#endif

static void nfc_hal_nci_recv_ramdump_event(RamDumpState state);
static void nfc_hal_nci_recv_init_ramdump_ntf();
static void nfc_hal_nci_recv_init_ramdump_rsp();
static void nfc_hal_nci_recv_get_ramdump_rsp();
static void nfc_hal_nci_recv_end_ramdump_rsp();

/* File scope prototype for tidying after RamDump sequence */
static void nfc_hal_nci_close_ramdump();

/* File scope prototypes for manipulating the data */
static int open_new_nfc_ramdump_file();
static BOOLEAN nfc_hal_nci_parse_ramdump_config_create_dump_area(int buf_len);
static void nfc_hal_nci_invoke_ramdump_upload();
static void nfc_hal_nci_parse_and_dump_ramdump_register_data();
static void nfc_hal_nci_parse_and_dump_ramdump_data();


/* Other file scope prototypes */
static int nfc_hal_nci_send_nvm_updates(BOOLEAN fused, UINT8* nvmUpdateBuff, UINT8* nvmDataBuffLen, UINT8* nvmCmd, UINT8* nvmCmdLen);

/* UICC soft reset */
static qmi_client_qmux_instance_type find_modem_port(char *prop_value_ptr);

static void softUiccResetFireAndForget();

static void softUiccResetCallback(qmi_client_type                  user_handle,
                                  unsigned long                    msg_id,
                                  void                           * resp_c_struct,
                                  int                              resp_c_struct_len,
                                  void                           * resp_cb_data,
                                  qmi_client_error_type            transp_err);

static qmi_client_type uicc_init_client(qmi_client_qmux_instance_type   dev_id,
                                        int                           * qmi_err_code);

static void qmiClientInitInstanceCallback(qmi_client_type                user_handle,
                                          unsigned int                   msg_id,
                                          void                           *ind_buf,
                                          unsigned int                   ind_buf_len,
                                          void                           *ind_cb_data);


/*******************************************************************************
**
** Function         nfc_hal_nci_assemble_nci_msg
**
** Description      This function is called to reassemble the received NCI
**                  response/notification packet, if required.
**                  (The data packets are posted to NFC task for reassembly)
**
** Returns          void.
**
*******************************************************************************/
void nfc_hal_nci_assemble_nci_msg (void)
{
    NFC_HDR *p_msg = nfc_hal_cb.ncit_cb.p_rcv_msg;
    UINT8 u8;
    UINT8 *p, *pp;
    UINT8 hdr[2];
    UINT8   *ps, *pd;
    UINT16  size, needed;
    BOOLEAN disp_again = FALSE;

    if ((p_msg == NULL) || (p_msg->len < NCI_MSG_HDR_SIZE))
        return;

#ifdef DISP_NCI
    DISP_NCI ((UINT8 *) (p_msg + 1) + p_msg->offset, (UINT16) (p_msg->len), TRUE);
#endif

    p       = (UINT8 *) (p_msg + 1) + p_msg->offset;
    u8      = *p++;
    /* remove the PBF bit for potential reassembly later */
    hdr[0]  = u8 & ~NCI_PBF_MASK;
    if ((u8 & NCI_MT_MASK) == NCI_MT_DATA)
    {
        /* clear the RFU in octet1 */
        *(p) = 0;
        /* data packet reassembly is performed in NFC task */
        return;
    }
    else
    {
        *(p) &= NCI_OID_MASK;
    }

    hdr[1]  = *p;
    pp = hdr;
    /* save octet0 and octet1 of an NCI header in layer_specific for the received packet */
    STREAM_TO_UINT16 (p_msg->layer_specific, pp);

    if (nfc_hal_cb.ncit_cb.p_frag_msg)
    {
        if (nfc_hal_cb.ncit_cb.p_frag_msg->layer_specific != p_msg->layer_specific)
        {
            /* check if these fragments are of the same NCI message */
            HAL_TRACE_ERROR2 ("nfc_hal_nci_assemble_nci_msg() - different messages 0x%x, 0x%x!!", nfc_hal_cb.ncit_cb.p_frag_msg->layer_specific, p_msg->layer_specific);
            nfc_hal_cb.ncit_cb.nci_ras  |= NFC_HAL_NCI_RAS_ERROR;
        }
        else if (nfc_hal_cb.ncit_cb.nci_ras == 0)
        {
            disp_again = TRUE;
            /* if not previous reassembly error, append the new fragment */
            p_msg->offset   += NCI_MSG_HDR_SIZE;
            p_msg->len      -= NCI_MSG_HDR_SIZE;
            size    = GKI_get_buf_size (nfc_hal_cb.ncit_cb.p_frag_msg);
            needed  = (NFC_HDR_SIZE + nfc_hal_cb.ncit_cb.p_frag_msg->len + nfc_hal_cb.ncit_cb.p_frag_msg->offset + p_msg->len);
            if (size >= needed)
            {
                /* the buffer for reassembly is big enough to append the new fragment */
                ps   = (UINT8 *) (p_msg + 1) + p_msg->offset;
                pd   = (UINT8 *) (nfc_hal_cb.ncit_cb.p_frag_msg + 1) + nfc_hal_cb.ncit_cb.p_frag_msg->offset + nfc_hal_cb.ncit_cb.p_frag_msg->len;
                memcpy (pd, ps, p_msg->len);
                nfc_hal_cb.ncit_cb.p_frag_msg->len  += p_msg->len;
                /* adjust the NCI packet length */
                pd   = (UINT8 *) (nfc_hal_cb.ncit_cb.p_frag_msg + 1) + nfc_hal_cb.ncit_cb.p_frag_msg->offset + 2;
                *pd  = (UINT8) (nfc_hal_cb.ncit_cb.p_frag_msg->len - NCI_MSG_HDR_SIZE);
            }
            else
            {
                nfc_hal_cb.ncit_cb.nci_ras  |= NFC_HAL_NCI_RAS_TOO_BIG;
                HAL_TRACE_ERROR2 ("nfc_hal_nci_assemble_nci_msg() buffer overrun (%d + %d)!!", nfc_hal_cb.ncit_cb.p_frag_msg->len, p_msg->len);
            }
        }
        /* we are done with this new fragment, free it */
        GKI_freebuf (p_msg);
    }
    else
    {
        nfc_hal_cb.ncit_cb.p_frag_msg = p_msg;
    }


    if ((u8 & NCI_PBF_MASK) == NCI_PBF_NO_OR_LAST)
    {
        /* last fragment */
        p_msg               = nfc_hal_cb.ncit_cb.p_frag_msg;
        p                   = (UINT8 *) (p_msg + 1) + p_msg->offset;
        *p                  = u8; /* this should make the PBF flag as Last Fragment */
        nfc_hal_cb.ncit_cb.p_frag_msg  = NULL;

        p_msg->layer_specific = nfc_hal_cb.ncit_cb.nci_ras;
        /* still report the data packet, if the incoming packet is too big */
        if (nfc_hal_cb.ncit_cb.nci_ras & NFC_HAL_NCI_RAS_ERROR)
        {
            /* NFCC reported NCI fragments for different NCI messages and this is the last fragment - drop it */
            HAL_TRACE_ERROR0 ("nfc_hal_nci_assemble_nci_msg() clearing NCI_RAS_ERROR");
            GKI_freebuf (p_msg);
            p_msg = NULL;
        }
#ifdef DISP_NCI
        if ((nfc_hal_cb.ncit_cb.nci_ras == 0) && (disp_again))
        {
            DISP_NCI ((UINT8 *) (p_msg + 1) + p_msg->offset, (UINT16) (p_msg->len), TRUE);
        }
#endif
        /* clear the error flags, so the next NCI packet is clean */
        nfc_hal_cb.ncit_cb.nci_ras = 0;
    }
    else
    {
        /* still reassembling */
        p_msg = NULL;
    }

    nfc_hal_cb.ncit_cb.p_rcv_msg = p_msg;
}

/*****************************************************************************
**
** Function         nfc_hal_nci_receive_nci_msg
**
** Description
**      Handle incoming data (NCI events) from the serial port.
**
**      If there is data waiting from the serial port, this funciton reads the
**      data and parses it. Once an entire NCI message has been read, it sends
**      the message the the NFC_TASK for processing
**
*****************************************************************************/
static BOOLEAN nfc_hal_nci_receive_nci_msg (tNFC_HAL_NCIT_CB *p_cb)
{
        UINT16      len = 0;
        BOOLEAN     msg_received = FALSE;

        HAL_TRACE_DEBUG1 ("nfc_hal_nci_receive_nci_msg+ 0x%08X", p_cb);
        if (p_cb == NULL){
            GKI_TRACE_ERROR_0("  nfc_hal_nci_receive_nci_msg () Invalid input ");
            return msg_received;
        }
        /* Start of new message. Allocate a buffer for message */
        if ((p_cb->p_rcv_msg = (NFC_HDR *) GKI_getpoolbuf (NFC_HAL_NCI_POOL_ID)) != NULL)
        {
               /* Initialize NFC_HDR */
               p_cb->p_rcv_msg->len    = 0;
               p_cb->p_rcv_msg->event  = 0;
               p_cb->p_rcv_msg->offset = 0;
               p_cb->rcv_len = MAX_RECV_PACKET_SIZE;
               /* Read in the rest of the message */
               len = DT_Nfc_Read(USERIAL_NFC_PORT, ((UINT8 *) (p_cb->p_rcv_msg + 1) + p_cb->p_rcv_msg->offset + p_cb->p_rcv_msg->len),  p_cb->rcv_len);
               p_cb->p_rcv_msg->len    = len;

               /* Check if we read in entire message yet */
               if ( p_cb->p_rcv_msg->len != 0)
               {
                   msg_received    = TRUE;
                   p_cb->rcv_state = NFC_HAL_RCV_IDLE_ST;
               }


               if (len == 0){
                  GKI_freebuf(p_cb->p_rcv_msg);
                  GKI_TRACE_ERROR_0("nfc_hal_nci_receive_nci_msg: Read Length = 0 so freeing pool !!\n");
               }
        }
        else{
           GKI_TRACE_ERROR_0("nfc_hal_nci_receive_nci_msg: Unable to get pool buffer\n");
        }
        return msg_received;
}
/*****************************************************************************
**
** Function         nfc_hal_nci_receive_bt_msg
**
** Description
**      Handle incoming BRCM specific data from the serial port.
**
**      If there is data waiting from the serial port, this funciton reads the
**      data and parses it. Once an entire message has been read, it returns
**      TRUE.
**
*****************************************************************************/
static BOOLEAN nfc_hal_nci_receive_bt_msg (tNFC_HAL_NCIT_CB *p_cb, UINT8 byte)
{
    UINT16  len;
    BOOLEAN msg_received = FALSE;

    if(p_cb == NULL ){
        GKI_TRACE_ERROR_0(" BOOLEAN nfc_hal_nci_receive_bt_msg () Invalid input ");
        return msg_received;
    }
    switch (p_cb->rcv_state)
    {
    case NFC_HAL_RCV_BT_MSG_ST:

        /* Initialize rx parameters */
        p_cb->rcv_state = NFC_HAL_RCV_BT_HDR_ST;
        p_cb->rcv_len   = HCIE_PREAMBLE_SIZE;

        if ((p_cb->p_rcv_msg = (NFC_HDR *) GKI_getpoolbuf (NFC_HAL_NCI_POOL_ID)) != NULL)
        {
            /* Initialize NFC_HDR */
            p_cb->p_rcv_msg->len    = 0;
            p_cb->p_rcv_msg->event  = 0;
            p_cb->p_rcv_msg->offset = 0;

            *((UINT8 *) (p_cb->p_rcv_msg + 1) + p_cb->p_rcv_msg->offset + p_cb->p_rcv_msg->len++) = byte;
        }
        else
        {
            HAL_TRACE_ERROR0 ("[nfc] Unable to allocate buffer for incoming NCI message.");
        }
        p_cb->rcv_len--;
        break;

    case NFC_HAL_RCV_BT_HDR_ST:
        if (p_cb->p_rcv_msg)
        {
            *((UINT8 *) (p_cb->p_rcv_msg + 1) + p_cb->p_rcv_msg->offset + p_cb->p_rcv_msg->len++) = byte;
        }
        p_cb->rcv_len--;

        /* Check if we received entire preamble yet */
        if (p_cb->rcv_len == 0)
        {
            /* Received entire preamble. Length is in the last byte(s) of the preamble */
            p_cb->rcv_len = byte;
            if(p_cb->p_rcv_msg == NULL){
               GKI_TRACE_ERROR_0(" Invalid memory  ");
               break;
            }
            /* Verify that buffer is big enough to fit message */
            if ((sizeof (NFC_HDR) + HCIE_PREAMBLE_SIZE + byte) > GKI_get_buf_size (p_cb->p_rcv_msg))
            {
                /* Message cannot fit into buffer */
                GKI_freebuf (p_cb->p_rcv_msg);
                p_cb->p_rcv_msg     = NULL;

                HAL_TRACE_ERROR0 ("Invalid length for incoming BT HCI message.");
            }

            /* Message length is valid */
            if (byte)
            {
                /* Read rest of message */
                p_cb->rcv_state = NFC_HAL_RCV_BT_PAYLOAD_ST;
            }
            else
            {
                /* Message has no additional parameters. (Entire message has been received) */
                msg_received    = TRUE;
                p_cb->rcv_state = NFC_HAL_RCV_IDLE_ST;  /* Next, wait for packet type of next message */
            }
        }
        break;

    case NFC_HAL_RCV_BT_PAYLOAD_ST:
        p_cb->rcv_len--;
        if (p_cb->p_rcv_msg)
        {
            *((UINT8 *) (p_cb->p_rcv_msg + 1) + p_cb->p_rcv_msg->offset + p_cb->p_rcv_msg->len++) = byte;

            if (p_cb->rcv_len > 0)
            {
                /* Read in the rest of the message */
                len = DT_Nfc_Read (USERIAL_NFC_PORT, ((UINT8 *) (p_cb->p_rcv_msg + 1) + p_cb->p_rcv_msg->offset + p_cb->p_rcv_msg->len),  p_cb->rcv_len);
                p_cb->p_rcv_msg->len    += len;
                p_cb->rcv_len           -= len;
            }
        }

        /* Check if we read in entire message yet */
        if (p_cb->rcv_len == 0)
        {
            msg_received        = TRUE;
            p_cb->rcv_state     = NFC_HAL_RCV_IDLE_ST;      /* Next, wait for packet type of next message */
        }
        break;
    }

    /* If we received entire message */
#if (NFC_HAL_TRACE_PROTOCOL == TRUE)
    if (msg_received && p_cb->p_rcv_msg)
    {
        /* Display protocol trace message */
        DispHciEvt (p_cb->p_rcv_msg);
    }
#endif

    return msg_received;
}

/*******************************************************************************
**
** Function         nfc_hal_nci_proc_rx_bt_msg
**
** Description      Received BT message from NFCC
**
**                  Notify command complete if initializing NFCC
**                  Forward BT message to NFC task
**
** Returns          void
**
*******************************************************************************/
static void nfc_hal_nci_proc_rx_bt_msg (void)
{
    UINT8   *p;
    NFC_HDR *p_msg;
    UINT16  opcode, old_opcode;
    tNFC_HAL_BTVSC_CPLT       vcs_cplt_params;
    tNFC_HAL_BTVSC_CPLT_CBACK *p_cback = NULL;

    /* if complete BT message is received successfully */
    if (nfc_hal_cb.ncit_cb.p_rcv_msg)
    {
        p_msg   = nfc_hal_cb.ncit_cb.p_rcv_msg;
        HAL_TRACE_DEBUG1 ("nfc_hal_nci_proc_rx_bt_msg (): GOT an BT msgs init_sta:%d", nfc_hal_cb.dev_cb.initializing_state);
        HAL_TRACE_DEBUG2 ("event: 0x%x, wait_rsp:0x%x", p_msg->event, nfc_hal_cb.ncit_cb.nci_wait_rsp);
        /* increase the cmd window here */
        if (nfc_hal_cb.ncit_cb.nci_wait_rsp == NFC_HAL_WAIT_RSP_PROP)
        {
            p = (UINT8 *) (p_msg + 1) + p_msg->offset;
            if (*p == HCI_COMMAND_COMPLETE_EVT)
            {
                p  += 3; /* code, len, cmd window */
                STREAM_TO_UINT16 (opcode, p);
                p   = nfc_hal_cb.ncit_cb.last_hdr;
                STREAM_TO_UINT16 (old_opcode, p);
                if (opcode == old_opcode)
                {
                    nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;
                    p_cback = (tNFC_HAL_BTVSC_CPLT_CBACK *)nfc_hal_cb.ncit_cb.p_vsc_cback;
                    nfc_hal_cb.ncit_cb.p_vsc_cback  = NULL;
                    nfc_hal_main_stop_quick_timer (&nfc_hal_cb.ncit_cb.nci_wait_rsp_timer);
                }
            }
        }

        /* if initializing NFCC */
        if ((nfc_hal_cb.dev_cb.initializing_state == NFC_HAL_INIT_STATE_W4_APP_COMPLETE) ||
            (nfc_hal_cb.dev_cb.initializing_state == NFC_HAL_INIT_STATE_W4_CONTROL_DONE))
        {
            /* this is command complete event for baud rate update or download patch */
            p = (UINT8 *) (p_msg + 1) + p_msg->offset;

            p += 1;    /* skip opcode */
            STREAM_TO_UINT8  (vcs_cplt_params.param_len, p);

            p += 1;    /* skip num command packets */
            STREAM_TO_UINT16 (vcs_cplt_params.opcode, p);

            vcs_cplt_params.param_len -= 3;
            vcs_cplt_params.p_param_buf = p;

            if (nfc_hal_cb.dev_cb.initializing_state == NFC_HAL_INIT_STATE_W4_CONTROL_DONE)
            {
                NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_IDLE);
                nfc_hal_cb.p_stack_cback (HAL_NFC_RELEASE_CONTROL_EVT, HAL_NFC_STATUS_OK);
            }
            if (p_cback)
            {
                nfc_hal_cb.ncit_cb.p_vsc_cback = NULL;
                (*p_cback) (&vcs_cplt_params);
            }

            /* do not BT send message to NFC task */
            GKI_freebuf (p_msg);
        }
        else
        {
            /* do not BT send message to NFC task */
            GKI_freebuf(nfc_hal_cb.ncit_cb.p_rcv_msg);
        }
        nfc_hal_cb.ncit_cb.p_rcv_msg = NULL;
    }
}

/*****************************************************************************
**
** Function         nfc_hal_nci_receive_msg
**
** Description
**      Handle incoming data (NCI events) from the serial port.
**
**      If there is data waiting from the serial port, this funciton reads the
**      data and parses it. Once an entire NCI message has been read, it sends
**      the message the the NFC_TASK for processing
**
*****************************************************************************/
BOOLEAN nfc_hal_nci_receive_msg (void)
{
    tNFC_HAL_NCIT_CB *p_cb = &(nfc_hal_cb.ncit_cb);
    BOOLEAN msg_received = FALSE;

    msg_received = nfc_hal_nci_receive_nci_msg (p_cb);
    return msg_received;
}

/*******************************************************************************
**
** Function         nfc_hal_nci_preproc_rx_nci_msg
**
** Description      NFCC sends NCI message to DH while initializing NFCC
**                  processing low power mode
**
** Precondition     nfc_hal_cb.dev_cb.initializing_state != NFC_HAL_INIT_STATE_NFC_HAL_INIT
**
** Params           p_msg; the incoming packet
** Returns          TRUE, if NFC task need to receive NCI message
**
*******************************************************************************/
BOOLEAN nfc_hal_nci_preproc_rx_nci_msg (NFC_HDR *p_msg)
{
    UINT8 *p, *pp, cid;
    UINT8 mt, pbf, gid, op_code;
    UINT8 payload_len;
    UINT16 data_len;
    UINT8 nvmupdatebuff[260]={0},nvmdatabufflen=0;
    UINT8 *nvmcmd = NULL, nvmcmdlen = 0;
    UINT32 nvm_update_flag = 0;
    UINT32 pm_flag = 0;
    UINT8 *p1;

    HAL_TRACE_DEBUG1 ("nfc_hal_nci_preproc_rx_nci_msg(), %d", nfc_hal_cb.dev_cb.initializing_state);


    /* Initialise the NFCC and perform any other init processing */
    if (initNfcc(nfc_hal_cb.ncit_cb.p_rcv_msg) == FALSE)
    {
        return FALSE;
    }

    p = (UINT8 *) (p_msg + 1) + p_msg->offset;
    pp = p;
    NCI_MSG_PRS_HDR0 (p, mt, pbf, gid);
    NCI_MSG_PRS_HDR1 (p, op_code);
    payload_len = *p++;
    UINT8 reason = *p;

    if (mt == NCI_MT_DATA)
    {
        if (nfc_hal_cb.hci_cb.hcp_conn_id)
        {
            NCI_DATA_PRS_HDR(pp, pbf, cid, data_len);
            if (cid == nfc_hal_cb.hci_cb.hcp_conn_id)
            {
                nfc_hal_hci_handle_hcp_pkt_from_hc (pp);
            }

        }
    }

    switch (gid)
    {
    case NCI_GID_PROP:
    {
        /* Handle Ramdump */
        if ((mt == NCI_MT_RSP) && (op_code == NCI_OID_INIT || op_code == NCI_OID_GET || op_code == NCI_OID_END))
        {
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_preproc_rx_nci_msg: RAMDUMP: Got a NFCC NCI_MT_RSP");
#ifndef NDEBUG
            nfc_hal_nci_handle_ramdump_rsp(mt, op_code, pbf, payload_len, p);
#else
            nfc_hal_nci_handle_ramdump_rsp(op_code, payload_len, p);
#endif
            /*
             * Indicate no further processing as we are in ram dump.
             * i.e. do not send message to NFC task.
             * Different to 'RamDump_Data::ramdump_in_progress' in that
             * we only need to know this information for the first Ramdump RSP
             * so as to stop the NFC timer
             */
            static BOOLEAN first_ramdump_rsp = FALSE;
            if(!first_ramdump_rsp)
            {
                /*Send first time TRUE to upper layer to clear timer*/
                first_ramdump_rsp = TRUE;
                return TRUE;
            }
            else if((mt == NCI_MT_RSP) && (op_code == NCI_OID_END))
            {
                /* reset for next crash */
                first_ramdump_rsp = FALSE;
            }
            return FALSE;
        }
        /* Handle Network NTF */
        else if ((mt == NCI_MT_NTF) && (op_code == NCI_MSG_HCI_NETWK))
        {
            nfc_hal_hci_handle_hci_netwk_info ((UINT8 *) (p_msg + 1) + p_msg->offset);
        }
        /* Handle HCI NTF */
        else if ((mt == NCI_MT_NTF) && (op_code == NCI_OID_NFCC_STORE))
        {
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_preproc_rx_nci_msg: NFCC_STORAGE: Received NTF.");
            nfc_hal_nci_recv_nfcc_storage_ntf(payload_len, p);

            /* FALLTHRU and return */
        }
        else
        {
            HAL_TRACE_WARNING1 ("nfc_hal_nci_preproc_rx_nci_msg: WARNING: Not handling PROP msg: 0x%X", CONSTRUCT_MSG_TYPE_SHORT(mt, gid, op_code));
        }
        break;
    }
    case NCI_GID_RF_MANAGE:
    {
        if (mt == NCI_MT_NTF)
        {
            if (op_code == NCI_MSG_RF_INTF_ACTIVATED)
            {
                nfc_hal_cb.act_interface = NCI_INTERFACE_MAX + 1;
                nfc_hal_cb.listen_mode_activated = FALSE;
                nfc_hal_cb.kovio_activated = FALSE;
                /*check which interface is activated*/
                if((*(p+1) == NCI_INTERFACE_NFC_DEP))
                {
                    if((*(p+3) == NCI_DISCOVERY_TYPE_LISTEN_F) ||
                            (*(p+3) == NCI_DISCOVERY_TYPE_LISTEN_A) ||
                            (*(p+3) == NCI_DISCOVERY_TYPE_POLL_F)   ||
                            (*(p+3) == NCI_DISCOVERY_TYPE_POLL_A))
                    {
                        nfc_hal_cb.act_interface = NCI_INTERFACE_NFC_DEP;
                        nfc_hal_cb.listen_setConfig_rsp_cnt = 0;
                    }
                }
                if((*(p+3) == NCI_DISCOVERY_TYPE_POLL_KOVIO))
                {
                    HAL_TRACE_DEBUG0 ("Kovio Tag activated");
                    nfc_hal_cb.kovio_activated = TRUE;
                }
                if((*(p+3) == NCI_DISCOVERY_TYPE_LISTEN_F) ||
                        (*(p+3) == NCI_DISCOVERY_TYPE_LISTEN_A) ||
                        (*(p+3) == NCI_DISCOVERY_TYPE_LISTEN_B))
                {
                    HAL_TRACE_DEBUG0 ("Listen mode activated");
                    nfc_hal_dm_update_field_wake(TRUE);
                    nfc_hal_cb.listen_mode_activated = TRUE;
                }
                if ((nfc_hal_cb.max_rf_credits) && (payload_len > 5))
                {
                    /* API used wants to limit the RF data credits */
                    p += 5; /* skip RF disc id, interface, protocol, tech&mode, payload size */
                    if (*p > nfc_hal_cb.max_rf_credits)
                    {
                        HAL_TRACE_DEBUG2 ("RfDataCredits %d->%d", *p, nfc_hal_cb.max_rf_credits);
                        *p = nfc_hal_cb.max_rf_credits;
                    }
                }

                if((*(p + 1) == NCI_INTERFACE_EE_DIRECT_RF))
                {
                    nfc_hal_cb.act_interface = NCI_INTERFACE_EE_DIRECT_RF;
                }
                /* Don't wakeup in listen mode activation
                 * KOVIO tags are read only and have just ID to be read and
                 * DH or KOVIO tag do not send/recev data so need to wake up */
                else if(!nfc_hal_dm_is_field_wake() && nfc_hal_cb.kovio_activated != TRUE)
                {
                    nfc_hal_dm_set_nfc_wake (NFC_HAL_ASSERT_NFC_WAKE);
                }
                else
                {
                    HAL_TRACE_DEBUG1 ("don't wake up nfcc, nfc_hal_cb.dev_cb.sleep.raw : %x\n", nfc_hal_cb.dev_cb.sleep.raw);
                }
            }
            if (op_code == NCI_MSG_RF_DEACTIVATE)
            {
                if(*(p) == NCI_DEACTIVATE_TYPE_DISCOVERY)
                {
                    if(!nfc_hal_dm_is_field_wake() &&
                            (nfc_hal_cb.act_interface != NCI_INTERFACE_EE_DIRECT_RF) &&
                            (!nfc_hal_cb.kovio_activated) &&
                            (nfc_hal_cb.pending_screencmd == 0xFF) /*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */
                        )
                    {
                        nfc_hal_dm_set_nfc_wake (NFC_HAL_DEASSERT_NFC_WAKE);
                    }
                    else
                    {
                        nfc_hal_dm_update_field_wake(FALSE);
                        nfc_hal_cb.listen_mode_activated = FALSE;
                        HAL_TRACE_DEBUG0 ("Clearing listen mode activated flag");
                    }
                    /*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */
                    if (nfc_hal_cb.pending_screencmd != 0xFF)
                    {
                        nfc_hal_cb.pending_screencmd = 0xFF;
                        HAL_TRACE_DEBUG0 ("Clean pending screen cmd de-act");
                    }
                    /*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */
                }
                else
                {
                    nfc_hal_dm_update_field_wake(FALSE);
                    nfc_hal_cb.listen_mode_activated = FALSE;
                    HAL_TRACE_DEBUG0 ("Clearing listen mode activated flag");
                }
            }
        }
        if (mt == NCI_MT_RSP)
        {
            if (op_code == NCI_MSG_RF_DISCOVER)
            {
                if (*p  == 0x00) //status good
                {
                    if ((nfc_hal_cb.act_interface == NCI_INTERFACE_NFC_DEP) &&
                            (nfc_hal_cb.listen_setConfig_rsp_cnt != 0))
                    {
                        /* do not send sleep commmand to avoid waking up NFCC
                         * to enable RF field NTF while listen active
                         * for example, P2P initiator mode then immediately move CE
                         */
                        HAL_TRACE_DEBUG0 ("Wait for set config to enable RF field NTF");
                    }
                    else
                    {
                            nfc_hal_dm_set_nfc_wake (NFC_HAL_DEASSERT_NFC_WAKE);
                    }
                }
            }
            else if (op_code == NCI_MSG_RF_DISCOVER_SELECT)
            {
                if (*p == NCI_STATUS_OK) //status good
                {
                    /*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */
                    if (nfc_hal_cb.pending_screencmd != 0xFF)
                    {
                        nfc_hal_cb.pending_screencmd = 0xFF;
                        HAL_TRACE_DEBUG0 ("Clean pending screen cmd sel");
                    }
                    /*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */
                }
            }
        }

#ifdef RAMDUMP_TEST_POKE
        /*
         * Simulate a NFCC reset for test purposes.
         * Ensure we receive a RSP for a CORE_INIT_CMD. We can only get the NFCC into a RAMDUMP state when it is in the correct state!
         */
        static BOOLEAN poke_reset = TRUE;
        if (poke_reset && (mt == NCI_MT_RSP) && (nfc_hal_cb.dev_cb.initializing_state != NFC_HAL_INIT_STATE_RAMDUMP))
        {
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_preproc_rx_nci_msg: RAMDUMP: Receiving RSP to CORE_INIT_CMD, NFCC in correct state to request a ramdump. Simulating the NFCC reset...");
            poke_reset = FALSE;
            /* Reset nfcc for testing ramdump state sequence */
            nfc_hal_dm_send_prop_reset_nfcc_ramdump_poke();
        }
#endif
    }
        break;
    case NCI_GID_CORE:
    {
        if(current_mode == FTM_MODE)
        {
            return TRUE;
        }
        if (mt == NCI_MT_RSP)
        {
            if(op_code == NCI_MSG_CORE_SET_CONFIG)
            {
                if(nfc_hal_cb.act_interface == NCI_INTERFACE_NFC_DEP)
                {
                    nfc_hal_cb.listen_setConfig_rsp_cnt++;
                    if(nfc_hal_cb.listen_setConfig_rsp_cnt == 2)
                    {
                        /* This is to take care of sleep to be sent once
                          all config cmds are sent from JNI;
                          if we change JNI config cmds then this will change
                          */
                        HAL_TRACE_DEBUG0 ("Sending sleep command ..");
                        nfc_hal_dm_set_nfc_wake (NFC_HAL_DEASSERT_NFC_WAKE);
                        nfc_hal_cb.listen_setConfig_rsp_cnt = 0;
                        nfc_hal_cb.act_interface = NCI_INTERFACE_MAX + 1;
                    }
                }
            }
            else if (op_code == NCI_MSG_CORE_CONN_CREATE)
            {
                if (nfc_hal_cb.hci_cb.b_wait_hcp_conn_create_rsp)
                {
                    p++; /* skip status byte */
                    nfc_hal_cb.hci_cb.b_wait_hcp_conn_create_rsp = FALSE;
                    p++; /* skip buff size */
                    p++; /* num of buffers */
                    nfc_hal_cb.hci_cb.hcp_conn_id = *p;
                }
            }
            else
            {
                HAL_TRACE_WARNING1 ("nfc_hal_nci_preproc_rx_nci_msg: WARNING: Not handling CORE RSP msg: 0x%X", CONSTRUCT_MSG_TYPE_SHORT(mt, gid, op_code));
            }
        }
        else if ((mt == NCI_MT_NTF) && (op_code == NCI_MSG_CORE_RESET))
        {

            /* For NFCC HCI Storage we need to reset the procedure to process the persistence file from the
               beginning as the NFCC may have reset during normal operation and is now out of sync with
               the UICC. */
            nfc_hal_nci_initiate_nfcc_storage_check(Nfcc_Storage_NfccResetInProgress);

            if (NCI_HAL_RAMDUMP_REASON == reason)
            {
                HAL_TRACE_DEBUG0 ("nfc_hal_nci_preproc_rx_nci_msg: RAMDUMP: Got a NFCC CORE_RESET_NTF");
#ifndef NDEBUG
                nfc_hal_nci_handle_ramdump_ntf(pbf, payload_len, reason);
#else
                nfc_hal_nci_handle_ramdump_ntf();
#endif
                /* Indicate to the system an NFCC reset has occurred (during NFCC normal operation)*/
                reset_status = TRUE;

                /*
                 * Indicate no further processing as we are in ram dump.
                 * i.e. do not send message to NFC task.
                 */
                return FALSE;
            }
#ifdef RAMDUMP
            else
            {
                /* Nothing to be sent back to NFCC */
                HAL_TRACE_DEBUG1 ("nfc_hal_nci_preproc_rx_nci_msg: RAMDUMP: Got a NFCC CORE_RESET_NTF. Ramdump is 'NOT' available, reason given is 0x%x", reason);
            }
#endif
        }
        else
        {
            HAL_TRACE_WARNING1 ("nfc_hal_nci_preproc_rx_nci_msg: WARNING: Not handling CORE msg: 0x%X", CONSTRUCT_MSG_TYPE_SHORT(mt, gid, op_code));
        }
        break;
    }
    default:
        HAL_TRACE_ERROR1 ("nfc_hal_nci_preproc_rx_nci_msg: WARNING: Not handling msg: 0x%X", CONSTRUCT_MSG_TYPE_SHORT(mt, gid, op_code));
    };

    return TRUE;
}


/*******************************************************************************
**
** Function         nfc_hal_nci_send_nvm_updates
**
** Description      Check the NVM file and send the CMD for the next entry.
**                  If any updates are to be processed the next one will be and
**                  the count of those updates remaining returned.
**                  When the last update is processed the file is closed and subsequent calls will
**                  reopen and reinitialise the count. Therefore the user should check the value of
**                  'nfc_hal_cb.nvm.last_cmd_sent' before calling this function.
**                  If no updates at all then 'no_of_updates == 0' and 'last_cmd_sent == FALSE'
**                  If last update sent then 'no_of_updates == 0' and 'last_cmd_sent == TRUE'
**
**                  Clears the Command Window before sending the next command.
**
**                  Once this function has processed all the NVM entries, it must not be used again unless the dm component
**                  resets the NVM. Behaviour is undefined otherwise!
**
** Returns          last_cmd_sent == TRUE    If the last CMD has been sent, expect a RSP,
**                  >=0                      Number of update CMDs left to be processed,
**                  -1                       On error
**
*******************************************************************************/
int nfc_hal_nci_send_nvm_updates(BOOLEAN fused, UINT8* nvmUpdateBuff, UINT8* nvmDataBuffLen, UINT8* nvmCmd, UINT8* nvmCmdLen)
{
    HAL_TRACE_DEBUG1 ("nfc_hal_nci_send_nvm_updates: is fused %d", fused);

    BOOLEAN updates_available = FALSE;

    if(fused == TRUE)
    {
        updates_available = nfc_hal_dm_check_fused_nvm_file(nvmUpdateBuff, nvmDataBuffLen);
    }
    else
    {
        updates_available = nfc_hal_dm_check_nvm_file(nvmUpdateBuff, nvmDataBuffLen);
    }

    if((updates_available == TRUE) && (nfc_hal_cb.nvm.no_of_updates > 0))
    {
        nvmCmd = (UINT8*) malloc(*nvmDataBuffLen + 10);
        if(nvmCmd)
        {
            if(fused == TRUE)
            {
                   nfc_hal_dm_frame_fused_mem_access_cmd(nvmCmd, nvmUpdateBuff, nvmCmdLen);
            }
            else
            {
                nfc_hal_dm_frame_mem_access_cmd(nvmCmd, nvmUpdateBuff, nvmCmdLen);
            }
            /* send nvm update cmd to NFCC*/
            HAL_TRACE_DEBUG1 ("nfc_hal_cb.nvm.no_of_updates remained %d ",nfc_hal_cb.nvm.no_of_updates);
            nfc_hal_cb.nvm.no_of_updates--;
            /* We should have already made sure this is the RSP we are waiting for before updating
               the command window, i.e. by calling this function. */
            nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;
            nfc_hal_dm_send_nci_cmd (nvmCmd, *nvmCmdLen, NULL);
            free(nvmCmd);
            if(nfc_hal_cb.nvm.no_of_updates == 0)
            {
                /*all updates sent so close file again*/
                HAL_TRACE_DEBUG0 ("nfc_hal_nci_send_nvm_updates() : Sent last update CMD");
                fclose( nfc_hal_cb.nvm.p_Nvm_file);
                nfc_hal_cb.nvm.p_Nvm_file = NULL;
                nfc_hal_cb.nvm.nvm_updated = TRUE;
                nfc_hal_cb.nvm.last_cmd_sent = TRUE;
            }
        }
        else
        {
            /*Memory allocation failed*/
            HAL_TRACE_ERROR0 ("nfc_hal_nci_send_nvm_updates() : **ERROR** Memory allocation failed for NVM CMD. Out of Memory!");
            return Nvm_MemoryAllocationFailure;
        }
    }
    /* Indicate update CMDs left to process */
    return nfc_hal_cb.nvm.no_of_updates;
}


/*******************************************************************************
**
** Function         nfc_hal_nci_add_nfc_pkt_type
**
** Description      Add packet type (HCIT_TYPE_NFC)
**
** Returns          None
**
*******************************************************************************/
void nfc_hal_nci_add_nfc_pkt_type (NFC_HDR *p_msg)
{
    UINT8   *p;
    UINT8   hcit;

    /* add packet type in front of NCI header */
    if (p_msg->offset > 0)
    {
        p_msg->offset--;
        p_msg->len++;

        p  = (UINT8 *) (p_msg + 1) + p_msg->offset;
        *p = HCIT_TYPE_NFC;
    }
    else
    {
        HAL_TRACE_ERROR0 ("nfc_hal_nci_add_nfc_pkt_type () : No space for packet type");
        hcit = HCIT_TYPE_NFC;

    }
}

/*******************************************************************************
**
** Function         nci_brcm_check_cmd_create_hcp_connection
**
** Description      Check if this is command to create HCP connection
**
** Returns          None
**
*******************************************************************************/
static void nci_brcm_check_cmd_create_hcp_connection (NFC_HDR *p_msg)
{
    UINT8 *p;
    UINT8 mt, pbf, gid, op_code;

    nfc_hal_cb.hci_cb.b_wait_hcp_conn_create_rsp = FALSE;

    p = (UINT8 *) (p_msg + 1) + p_msg->offset;

    if (nfc_hal_cb.dev_cb.initializing_state == NFC_HAL_INIT_STATE_IDLE)
    {
        NCI_MSG_PRS_HDR0 (p, mt, pbf, gid);
        NCI_MSG_PRS_HDR1 (p, op_code);

        if (gid == NCI_GID_CORE)
        {
            if (mt == NCI_MT_CMD)
            {
                if (op_code == NCI_MSG_CORE_CONN_CREATE)
                {
                    if (  ((NCI_CORE_PARAM_SIZE_CON_CREATE + 4) == *p++)
                        &&(NCI_DEST_TYPE_NFCEE == *p++)
                        &&(1 == *p++)
                        &&(NCI_CON_CREATE_TAG_NFCEE_VAL == *p++)
                        &&(2 == *p++)  )
                    {
                        p++;
                        if (NCI_NFCEE_INTERFACE_HCI_ACCESS == *p)
                        {
                            nfc_hal_cb.hci_cb.b_wait_hcp_conn_create_rsp = TRUE;
                            return;
                        }
                    }

                }
            }
        }
    }
}

/*******************************************************************************
**
** Function         nfc_hal_nci_send_cmd
**
** Description      Send NCI command to the transport
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_nci_send_cmd (NFC_HDR *p_buf)
{
    BOOLEAN continue_to_process = TRUE;
    UINT8   *ps, *pd;
    UINT16  max_len;
    UINT16  buf_len, offset;
    UINT8   *p;
    UINT8   hdr[NCI_MSG_HDR_SIZE];
    UINT8   nci_ctrl_size = nfc_hal_cb.ncit_cb.nci_ctrl_size;
    UINT8   delta = 0;
    UINT8 *p1;
    UINT8 mt, pbf, gid, op_code;
    UINT8 payload_len;
    UINT16 written_bytes = 0;
    if (  (nfc_hal_cb.hci_cb.hcp_conn_id == 0)
        &&(nfc_hal_cb.nvm_cb.nvm_type != NCI_SPD_NVM_TYPE_NONE)  )
        nci_brcm_check_cmd_create_hcp_connection ((NFC_HDR*) p_buf);

    /* check pending sleep response */
    continue_to_process = (nfc_hal_cb.dev_cb.sleep.state.W4_SLEEP_RSP == 0);

    if (!continue_to_process)
    {
        /* save the command to be sent until NFCC is free. */
        nfc_hal_cb.ncit_cb.p_pend_cmd   = p_buf;
        return;
    }

    max_len = nci_ctrl_size + NCI_MSG_HDR_SIZE;
    buf_len = p_buf->len;
    offset  = p_buf->offset;
#ifdef DISP_NCI
    if (buf_len > max_len)
    {
        /* this command needs to be fragmented. display the complete packet first */
        DISP_NCI ((UINT8 *) (p_buf + 1) + p_buf->offset, p_buf->len, FALSE);
    }
#endif
    ps      = (UINT8 *) (p_buf + 1) + p_buf->offset;
    memcpy (hdr, ps, NCI_MSG_HDR_SIZE);

    nfc_hal_dm_set_nfc_wake (NFC_HAL_ASSERT_NFC_WAKE);

    while (buf_len > max_len)
    {
        HAL_TRACE_DEBUG2 ("buf_len (%d) > max_len (%d)", buf_len, max_len);
        /* the NCI command is bigger than the NFCC Max Control Packet Payload Length
         * fragment the command */

        p_buf->len  = max_len;
        ps   = (UINT8 *) (p_buf + 1) + p_buf->offset;
        /* mark the control packet as fragmented */
        *ps |= NCI_PBF_ST_CONT;
        /* adjust the length of this fragment */
        ps  += 2;
        *ps  = nci_ctrl_size;

        /* add NCI packet type in front of message */

        /* send this fragment to transport */
        p = (UINT8 *) (p_buf + 1) + p_buf->offset;

#ifdef DISP_NCI
        delta = p_buf->len - max_len;
        DISP_NCI (p + delta, (UINT16) (p_buf->len - delta), FALSE);
#endif

        DT_Nfc_Write (USERIAL_NFC_PORT, p, p_buf->len);
        /* adjust the len and offset to reflect that part of the command is already sent */
        buf_len -= nci_ctrl_size;
        offset  += nci_ctrl_size;
        HAL_TRACE_DEBUG2 ("p_buf->len: %d buf_len (%d)", p_buf->len, buf_len);
        p_buf->len      = buf_len;
        p_buf->offset   = offset;
        pd   = (UINT8 *) (p_buf + 1) + p_buf->offset;
        /* restore the NCI header */
        memcpy (pd, hdr, NCI_MSG_HDR_SIZE);
        pd  += 2;
        *pd  = (UINT8) (p_buf->len - NCI_MSG_HDR_SIZE);
    }

    HAL_TRACE_DEBUG1 ("p_buf->len: %d", p_buf->len);

    /* add NCI packet type in front of message */

    /* send this fragment to transport */
    p = (UINT8 *) (p_buf + 1) + p_buf->offset;

    p1 = (UINT8 *) (p_buf + 1) + p_buf->offset;
    NCI_MSG_PRS_HDR0 (p1, mt, pbf, gid);
    NCI_MSG_PRS_HDR1 (p1, op_code);
    payload_len = p1[0];
    if (op_code == NCI_MSG_RF_DEACTIVATE)
    {
        if(p1[1]  == 0x00)
        {
           HAL_TRACE_DEBUG2 ("send deactivate in idle NCI_MSG_RF_DEACTIVATE: %d,payload_len=%d", p1[1], payload_len);
        }
        nfc_hal_cb.deact_type = p1[1];
    }
#ifdef DISP_NCI
    delta = p_buf->len - buf_len;
    DISP_NCI (p + delta, (UINT16) (p_buf->len - delta), FALSE);
#endif
    written_bytes = DT_Nfc_Write (USERIAL_NFC_PORT, p, p_buf->len);

    if(written_bytes && (mt == NCI_MT_CMD) && (gid == NCI_GID_PROP) && (op_code == NCI_MSG_PROP_SLEEP))
    {
        /* 2f 03 00 */
        if (nfc_hal_cb.dev_cb.sleep.state.CMD_SLEEP_JNI == 0) // if SLEEP_CMD was not sent from JNI
            nfc_hal_cb.dev_cb.sleep.state.W4_SLEEP_RSP = 1;
        HAL_TRACE_DEBUG3("%s:%d sleep flags: %x", __FUNCTION__, __LINE__, nfc_hal_cb.dev_cb.sleep.raw);
    }
    else if ((mt == NCI_MT_CMD) && (gid == NCI_GID_PROP) && (op_code == NCI_MSG_PROP_GENERIC) &&
               (payload_len == PROP_SCREEN_CMD_LEN))
    {
        /* 2f 01 03 p1[1] p1[2] p1[3] */
        if(p1[1] == NCI_MSG_PROP_GENERIC_SUBCMD_UI_STATE)
        {
            if (written_bytes)
                nfc_hal_dm_update_screen_flags(p1[3]); /* screen state */
            /*
             * need to response with fake response even if not sent to transport
             */
            nfc_hal_fake_screen_rsp();
            HAL_TRACE_DEBUG3("%s:%d sleep flags: %x", __FUNCTION__, __LINE__, nfc_hal_cb.dev_cb.sleep.raw);
        }
        else if (p1[1] == NCI_MSG_PROP_GENERIC_HAL_CMD)
        {
            nfc_hal_fake_screen_rsp();
        }
    }

    GKI_freebuf (p_buf);
}

/*******************************************************************************
**
** Function         nfc_hal_fake_screen_rsp
**
** Description      Fake nci response for screen command
**
** Returns          None
**
*******************************************************************************/
void nfc_hal_fake_screen_rsp()
{
    NFC_HDR  *p_msg;
    UINT8 *p, *ps;

    HAL_TRACE_DEBUG0 ("nfc_hal_fake_screen_rsp from HAL");

    /* Start of new message. Allocate a buffer for message */
    if ((p_msg = (NFC_HDR *) GKI_getpoolbuf (NFC_HAL_NCI_POOL_ID)) != NULL)
    {
        /* Initialize NFC_HDR */
        p_msg->event  = 0;
        p_msg->offset = 0;
        p_msg->layer_specific = 0;

        p = (UINT8 *) (p_msg + 1) + p_msg->offset;
        ps = p;
        p_msg->len    = NCI_DATA_HDR_SIZE + 0x01;
        NCI_MSG_BLD_HDR0(p, NCI_MT_RSP, NCI_GID_PROP);
        NCI_MSG_BLD_HDR1(p, 0x01);
        UINT8_TO_STREAM (p, 0x01);
        UINT8_TO_STREAM (p, NCI_STATUS_OK);
#ifdef DISP_NCI_FAKE
        DISP_NCI_FAKE (ps, (UINT16) p_msg->len, TRUE);
#endif
        nfc_hal_send_nci_msg_to_nfc_task (p_msg);
    }
    else
    {
        HAL_TRACE_ERROR0 ("nfc_hal_fake_screen_rsp: Unable to allocate buffer to fake response from HAL to stack.");
    }
}

/*******************************************************************************
**
** Function         nfc_hal_nci_cmd_timeout_cback
**
** Description      callback function for timeout
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_nci_cmd_timeout_cback (void *p_tle)
{
    TIMER_LIST_ENT  *p_tlent = (TIMER_LIST_ENT *)p_tle;

    HAL_TRACE_DEBUG0 ("nfc_hal_nci_cmd_timeout_cback ()");

    nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;

    if (p_tlent->event == NFC_HAL_TTYPE_NCI_WAIT_RSP)
    {
        if (nfc_hal_cb.dev_cb.initializing_state <= NFC_HAL_INIT_STATE_W4_PATCH_INFO)
        {
            NFC_HAL_SET_INIT_STATE (NFC_HAL_INIT_STATE_IDLE);
            nfc_hal_cb.ncit_cb.hw_error = TRUE;
            nfc_hal_main_pre_init_done (HAL_NFC_STATUS_ERR_CMD_TIMEOUT);
        }
        else if (nfc_hal_cb.dev_cb.initializing_state == NFC_HAL_INIT_STATE_W4_APP_COMPLETE)
        {
            if (nfc_hal_cb.prm.state != NFC_HAL_PRM_ST_IDLE)
            {
                nfc_hal_prm_process_timeout (NULL);
            }
            else
            {
                NFC_HAL_SET_INIT_STATE (NFC_HAL_INIT_STATE_IDLE);
                nfc_hal_cb.ncit_cb.hw_error = TRUE;
                nfc_hal_main_pre_init_done (HAL_NFC_STATUS_ERR_CMD_TIMEOUT);
            }
        }
        else if (nfc_hal_cb.dev_cb.initializing_state == NFC_HAL_INIT_STATE_W4_POST_INIT_DONE)
        {
            NFC_HAL_SET_INIT_STATE (NFC_HAL_INIT_STATE_IDLE);
            nfc_hal_cb.ncit_cb.hw_error = TRUE;
            nfc_hal_cb.p_stack_cback (HAL_NFC_POST_INIT_CPLT_EVT, HAL_NFC_STATUS_ERR_CMD_TIMEOUT);
        }
        else if (nfc_hal_cb.dev_cb.initializing_state == NFC_HAL_INIT_STATE_W4_CONTROL_DONE)
        {
            NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_IDLE);
            nfc_hal_cb.ncit_cb.hw_error = TRUE;
            nfc_hal_cb.p_stack_cback (HAL_NFC_RELEASE_CONTROL_EVT, HAL_NFC_STATUS_ERR_CMD_TIMEOUT);
        }
        else if (nfc_hal_cb.dev_cb.initializing_state == NFC_HAL_INIT_STATE_W4_PREDISCOVER_DONE)
        {
            NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_IDLE);
            nfc_hal_cb.ncit_cb.hw_error = TRUE;
            nfc_hal_cb.p_stack_cback (HAL_NFC_PRE_DISCOVER_CPLT_EVT, HAL_NFC_STATUS_ERR_CMD_TIMEOUT);
        }
        else if (nfc_hal_cb.dev_cb.initializing_state == NFC_HAL_INIT_STATE_RAMDUMP)
        {
            /* tidy up ramdump memory so no dangling pointers */
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_cmd_timeout_cback: RAMDUMP: RSP timer expired! Enabling PowerCycle");
            nfc_hal_nci_close_ramdump();

            /* Reset initialisation variables. Should be part of power cycle routine! */
            nfc_hal_cb.dev_cb.patch_applied = FALSE;
            nfc_hal_cb.dev_cb.pre_init_done = FALSE;
            nfc_hal_cb.dev_cb.pre_patch_signature_chk = FALSE;
            nfc_hal_cb.dev_cb.patch_signature_chk = FALSE;
            nfc_hal_cb.dev_cb.pre_patch_applied = FALSE;
            nfc_hal_cb.dev_cb.pre_patch_file_available = FALSE;
            nfc_hal_cb.dev_cb.fw_version_chk = FALSE;

            DT_Nfc_RamdumpPerformed = TRUE;
            GKI_send_event (NFC_HAL_TASK, NFC_HAL_TASK_EVT_POWER_CYCLE);
        }
    }
}


/*******************************************************************************
**
** Function         HAL_NfcSetMaxRfDataCredits
**
** Description      This function sets the maximum RF data credit for HAL.
**                  If 0, use the value reported from NFCC.
**
** Returns          none
**
*******************************************************************************/
void HAL_NfcSetMaxRfDataCredits (UINT8 max_credits)
{
    HAL_TRACE_DEBUG2 ("HAL_NfcSetMaxRfDataCredits %d->%d", nfc_hal_cb.max_rf_credits, max_credits);
    nfc_hal_cb.max_rf_credits   = max_credits;
}


/*******************************************************************************
**
** Function         nfc_hal_nci_handle_ramdump_ntf
**
** Description      On receiving a ramdump notification performs some initial checks before processing.
**
** Precondition     The reason code is NCI_HAL_RAMDUMP_REASON
**
** Returns          none
**
*******************************************************************************/
#ifndef NDEBUG
void nfc_hal_nci_handle_ramdump_ntf(int pbf, int payload_len, int reason)
#else
void nfc_hal_nci_handle_ramdump_ntf()
#endif
{
    /*
     * Two scenarios for arriving here;
     *
     *   1) We have issued a CORE_RESET_CMD, received back a CORE_RESET_NTF
     *   2) The NSFCC has reset with an internal error, receive a CORE_RESET_NTF
     *
     *   GID = CORE_ = 0x00
     *   60 00 RESET_NTF (see NFCForum-TS-NCI-1.0, Table 102, GID & OID definitions and NFC Middleware, chapter 4 NCI Summary)
     *   MT = 011b, i.e. (NCI_MT_NTF << NCI_MT_SHIFT) = 0x60
     *   OID = 000000b
     *
     * In either case, the reason code of the notification determines the scenario.
     * Segmentation will not occur on messages of maximum size 255 octets.
     *
     * Reason Code
     * 0x00                      Unspecified reason
     * 0x01-0x9F                 RFU
     * NCI_HAL_RAMDUMP_REASON    HardFault with RAM Dump available (see NFC Middleware/Post Mortem Debug HLD, Table 5.1)
     * 0xA1-0xFF                 For future use
     *
     * Configuration Status
     * 0x00                      NCI RF Configuration has been kept
     * 0x01                      NCI RF Configuration has been reset
     * 0x02-0xFF                 RFU
     */
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_handle_ramdump_ntf: RAMDUMP: Received ramdump NTF from NFCC");
    if(gRamdump_Data.ramdump_in_progress)
    {
        HAL_TRACE_DEBUG0 ("nfc_hal_nci_handle_ramdump_ntf: RAMDUMP: RamDump already in progress. Returning.");
        return;
    }
    /* Autonomous NFCC reset completed. (see Table 2: MT values) */
    /* Indicate Ramdump in progress */
    gRamdump_Data.ramdump_in_progress = TRUE;
#ifdef RAMDUMP
#define RAMDUMP_NOTIFY_PAYLOAD_LEN 2
    HAL_TRACE_DEBUG3 ("nfc_hal_nci_handle_ramdump_ntf: RAMDUMP: **DEBUG** pbf == %d (expecting == 0), reason == 0x%X (expecting 0x%X)", pbf, reason, NCI_HAL_RAMDUMP_REASON);
    HAL_TRACE_DEBUG2 ("nfc_hal_nci_handle_ramdump_ntf: RAMDUMP: **DEBUG** payload_len == %d (expecting <= %d)", payload_len, nfc_hal_cb.ncit_cb.nci_ctrl_size);
    HAL_TRACE_DEBUG2 ("nfc_hal_nci_handle_ramdump_ntf: RAMDUMP: **DEBUG** payload_len == %d (expecting == %d)", payload_len, RAMDUMP_NOTIFY_PAYLOAD_LEN);
    assert(pbf == 0); /* No segmentation */
    assert(payload_len <= nfc_hal_cb.ncit_cb.nci_ctrl_size);
    assert(payload_len == RAMDUMP_NOTIFY_PAYLOAD_LEN);
    assert(reason == NCI_HAL_RAMDUMP_REASON);
#endif
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_handle_ramdump_ntf: RAMDUMP: NFCC ramdump is available");
    /*
     * 4.1 Reset of NFCC (from the standard)
     * The NFCC MAY also reset itself (without having received a CORE_RESET_CMD); e.g., in the case of an internal error.
     * In these cases, the NFCC SHALL inform the DH with the CORE_RESET_NTF.
     *
     * The Reason code SHALL reflect the internal reset reason and the Configuration Status the status of the NCI RF Configuration.
     * We are interested in code NCI_HAL_RAMDUMP_REASON that states internal error.
     */

    /* Initiate a RAMDUMP if available */
    nfc_hal_nci_recv_ramdump_event(RamDump_Recv_Init_Ntf);
}

/*******************************************************************************
**
** Function         nfc_hal_nci_handle_ramdump_rsp
**
** Description      On receiving a ramdump responce performs some initial checks before processing.
**
** Returns          none
**
*******************************************************************************/
#ifndef NDEBUG
void nfc_hal_nci_handle_ramdump_rsp(int mt, int op_code, int pbf, int payload_len, UINT8 * const payload_ptr)
#else
void nfc_hal_nci_handle_ramdump_rsp(int op_code, int payload_len, UINT8 * const payload_ptr)
#endif
{
#ifdef RAMDUMP
    HAL_TRACE_DEBUG2 ("nfc_hal_nci_handle_ramdump_rsp: RAMDUMP: Receiving MT %d, OID %d from NFCC during ramdump communication", mt, op_code);
    HAL_TRACE_DEBUG3 ("nfc_hal_nci_handle_ramdump_rsp: RAMDUMP: **DEBUG** op_code-%d = %d (expecting < %d)", NCI_OID_INIT, op_code-NCI_OID_INIT, RamDump_Item_Count-1);
    HAL_TRACE_DEBUG1 ("nfc_hal_nci_handle_ramdump_rsp: RAMDUMP: **DEBUG** pbf = %d (expecting 0)", pbf);
    HAL_TRACE_DEBUG2 ("nfc_hal_nci_handle_ramdump_rsp: RAMDUMP: **DEBUG** payload_len = %d (expecting <= %d)", payload_len, nfc_hal_cb.ncit_cb.nci_ctrl_size);

    assert((op_code-NCI_OID_INIT) < RamDump_Item_Count-1);
    assert(pbf == 0); /* No segmentation */
    assert(payload_len <= nfc_hal_cb.ncit_cb.nci_ctrl_size);
#else
    HAL_TRACE_DEBUG1 ("nfc_hal_nci_handle_ramdump_rsp: RAMDUMP: Receiving OID %d from NFCC during ramdump communication", op_code);
#endif

    /* Not interested in reason or config status */
    /*
     * Determine what state we should be in for this event.
     * Correlate the opcode to the correct RAMDUMP state.
     */
    static const RamDumpState prop_ramdump_state[RamDump_Item_Count-1] = {RamDump_Recv_Init_Rsp, RamDump_Recv_Get_Rsp, RamDump_Recv_End_Rsp};
    if (RamDump_Recv_Init_Rsp == prop_ramdump_state[op_code-NCI_OID_INIT] || RamDump_Recv_Get_Rsp == prop_ramdump_state[op_code-NCI_OID_INIT])
    {
        /* remember the payload attributes for data extraction later */
        gRamdump_Data.msg_payload_ptr = payload_ptr;
        gRamdump_Data.msg_payload_length = payload_len;
    }
    nfc_hal_nci_recv_ramdump_event(prop_ramdump_state[op_code-NCI_OID_INIT]);
}

/*******************************************************************************
**
** Function         nfc_hal_nci_recv_ramdump_response
**
** Description      On receiving a notification from the NFCC we need to ascertain what next needs to be done.
**
**
** Returns          none
**
*******************************************************************************/
void nfc_hal_nci_recv_ramdump_event(RamDumpState state)
{
    /*
     * NFCC will reset its internal state only if it does not receive the expected event pertaining to that internal state.
     *
     * It should NEVER happen that these values do not correlate as that would mean our RAMDUMP state machine is incorrect!
     * If it does happen the default behaviour id to Reset the device!
     */
    HAL_TRACE_DEBUG1 ("nfc_hal_nci_recv_ramdump_event: RAMDUMP: Passed in state %d.", state);

    /*
     * Stop NFC RSP timer from expiring.
     * This will avoid any already sent CMD from not receiving a response.
     * The timer will be started automatically when we send our ramdump commands but will NOT be associated with that previous CMD.
     */
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_ramdump_event: RAMDUMP: Stopping timer for outstanding RSPs");

    /*
     * Stop the NFC RSP timer and indicate no RSP outstanding (So we can continue to send CMDs).
     * Note: Any other CMD that has been sent prior to here and after the ramdump NTF has been received will effectively be deadlocked!
     * The upper layers should handle this case of them never receiving a RSP as the NFCC will not remember the CMD after it's reset.
     */
    nfc_hal_main_stop_quick_timer (&nfc_hal_cb.ncit_cb.nci_wait_rsp_timer);
    nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;
    /*
     * We are called from nfc_hal_nci_preproc_rx_nci_msg() when receiving RAMDUMP events from the NFCC
     */

    static void (* const stateFunctionPtr[])(void) = {nfc_hal_nci_recv_init_ramdump_ntf,    /* RamDump_Recv_Init_Ntf */
                                                         nfc_hal_nci_recv_init_ramdump_rsp, /* RamDump_Recv_Init_Rsp */
                                                         nfc_hal_nci_recv_get_ramdump_rsp,  /* RamDump_Recv_Get_Rsp */
                                                         nfc_hal_nci_recv_end_ramdump_rsp}; /* RamDump_Recv_End_Rsp */

    /*
     * The first time we are called will be in due to a NTF being received from the NFCC.
     * All other times are due to RSP being received from the NFCC.
     * We do not have a state machine, we instead respond to the NFCC's state machine requests (NTF and RSPs').
     * Within our function that handles the NFCC event, depending on the payload we will respond accordingly.
     */
    /* handle the state */
    stateFunctionPtr[state]();
}

/***
 * Create/open with truncate, the ramdump output file.
 *
 * Attempts to use a time/date formatted string for the file name.
 * On timestamp functionality failure use a counter to replace the year field instead.
 *
 * With the time/date format, we capture crashes occurring within the same minute in the same file,
 * so dumps occurring within the same minute are overwritten.
 *
 * If the timestamp functionality fails then we probably have other more serious problems in the system.
 * To handle this we use a counter in the year field for each crash, we therefore store every crash that occurs!
 *
 * As this is an extreme case, it is a suitable alternative as the system may not operate correctly anyway!
 *
 * returns 0 on success; the file has been created, -1 on failure.
 */
int open_new_nfc_ramdump_file()
{
#define MAX_FILENAME_SIZE FILENAME_MAX
#define RAMDUMP_FILENAME_FORMAT "/data/nfc/ramdump_%H_%M__%d_%m_%Y.cfg"
#define RAMDUMP_FILENAME_FAILFORMAT "/data/nfc/ramdump_0_0__0_0_%d.cfg"
#define RAMDUMP_FILENAME_FAILFORMAT2 "/data/nfc/ramdump_0_0__0_0_0.cfg"

    const char * filename = NULL;
    char buffer [MAX_FILENAME_SIZE] = {};

    /* file time stamp */
    time_t now = time(NULL);
    struct tm *now_gmtime;

    if (now == -1
        || ((now_gmtime = gmtime(&now)) == NULL)
        || (strftime(buffer, MAX_FILENAME_SIZE, RAMDUMP_FILENAME_FORMAT, now_gmtime) == 0))
    {
        /* These files will start from a counter value of 1 */
        static int ramdump_counter = 1;
        memset(buffer, 0, MAX_FILENAME_SIZE); /* zero out */
        HAL_TRACE_WARNING0 ("open_new_nfc_ramdump_file(): RAMDUMP: **WARNING** Unable to read/write timestamp, trying incremental counter in filename instead. Continuing...");
        if (snprintf(buffer, MAX_FILENAME_SIZE, RAMDUMP_FILENAME_FAILFORMAT, ramdump_counter++) < 0)
        {
            /* This file will always have a counter value of 0 */
            memset(buffer, 0, MAX_FILENAME_SIZE); /* zero out */
            HAL_TRACE_WARNING0 ("open_new_nfc_ramdump_file(): RAMDUMP: **WARNING** Unable to format filename with count. This will result in a single crash being stored. Continuing...");
            memcpy(buffer, RAMDUMP_FILENAME_FAILFORMAT2, strlen(RAMDUMP_FILENAME_FAILFORMAT2));
        }
    }
    filename = buffer;

    /* create/truncate the file */
    gRamdump_Data.output_file_ptr = fopen (filename, "w+");
    if (gRamdump_Data.output_file_ptr == NULL)
    {
        HAL_TRACE_ERROR1 ("open_new_nfc_ramdump_file(): RAMDUMP: **ERROR** Unable to create/open ramdump file '%s'", filename);
        return -1;
    }

    HAL_TRACE_DEBUG1 ("open_new_nfc_ramdump_file(): RAMDUMP: Filename '%s' used for this ramdump", filename);
    return 0;
}

/*******************************************************************************
**
** Function         nfc_hal_nci_recv_init_ramdump_ntf
**
** Description      On receiving CORE_RESET_NTF notification from NFCC
**                  we need to initiate a RAMDUMP. This involves configuration files and a file for saving uploaded data.
**
** Returns          none
**
*******************************************************************************/
void nfc_hal_nci_recv_init_ramdump_ntf()
{
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: Received a CORE_RESET_NTF from NFCC");

    BOOLEAN error = FALSE;

    /*
     * Stop packets being sent to NFCC by anyone else.
     * Stop packets being received from NFCC to anyone else.
     */
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: Entering state NFC_HAL_INIT_STATE_RAMDUMP");
    NFC_HAL_SET_INIT_STATE (NFC_HAL_INIT_STATE_RAMDUMP);

    /***
     * Create a buffer large enough to store the config data.
     */
    FILE * config_file_ptr = fopen (kRamdump_config_filename, "r");
    if (config_file_ptr == NULL)
    {
        HAL_TRACE_DEBUG1 ("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: **ERROR** Unable to open config file \"%s\".", kRamdump_config_filename);
        error = TRUE;
    }
    if (!error && fseek(config_file_ptr, 0L, SEEK_END) != 0)
    {
        HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: **ERROR** Unable to position to end of config file.");
        error = TRUE;
    }
    size_t sz = 0;
    if (!error && ((sz = ftell(config_file_ptr)) <= 0))
    {
        HAL_TRACE_DEBUG1 ("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: **ERROR** Config file error obtaining size, or empty (size reported is %d)", sz);
        error = TRUE;
    }
    if (!error && fseek(config_file_ptr, 0L, SEEK_SET) != 0)
    {
        HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: **ERROR** Unable to position to beginning of config file.");
        error = TRUE;
    }
    gRamdump_Data.config_buffer = (UINT8*) malloc(sz);
    if (!error && gRamdump_Data.config_buffer == NULL)
    {
        HAL_TRACE_DEBUG1 ("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: **ERROR** Unable to malloc config buffer of %d length.", sz);
        error = TRUE;
    }

    /* copy the data*/
    size_t bytes_read = 0;
    if (!error && (((bytes_read = fread (gRamdump_Data.config_buffer , sizeof(char), sz, config_file_ptr)) == 0) || (bytes_read != sz)))
    {
        HAL_TRACE_DEBUG2 ("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: **ERROR** Wrong number of expected bytes read from config file (expected %d, read %d), or file empty", sz, bytes_read);
        error = TRUE;
    }
    if (!error && fclose (config_file_ptr) == EOF)
    {
        HAL_TRACE_WARNING0 ("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: **WARNING** Unable to close config file. Continuing...");
    }
    /***
     * Create the register buffer.
     */
    if (!error)
    {
        /* Get NFCC version */
        UINT16 chip_version = DT_Get_Nfcc_Version(NFCC_CHIP_VERSION_REG);
        UINT16 chip_version_major = ((chip_version >> 4)&(0xF));
        GKI_delay( 2 );
        UINT16 chip_revid = DT_Get_Nfcc_Version(NFCC_CHIP_REVID_REG);
        UINT16 metal_version = (chip_revid & (0xF));
        /* Based on the NFCC version we get sent some data that we need to parse, make room for it */
        if (chip_version_major == 2 && metal_version == 4)
        {
            gMaxRegisterCount = 21;
            gNfccChipVersion = 24;
        }
        HAL_TRACE_DEBUG3("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: Gathering Register data. NFCC chip version = %d.%d. Register count is %d", chip_version_major, metal_version, gMaxRegisterCount);
        const int kMax_register_size = gMaxRegisterCount*REGISTER_FIELD_LENGTH;
        gRamdump_Data.register_buffer = (UINT8*) malloc(kMax_register_size);
        if (gRamdump_Data.register_buffer == NULL)
        {
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: **ERROR** Unable to malloc ramdump register buffer.");
            error = TRUE;
        }
    }
    /***
     * Create/open with truncate, the output file.
     */
    if(!error && (open_new_nfc_ramdump_file() != 0))
    {
        error = FALSE;
    }

    /***
     * Extract the config data.
     * This involves creating a linked list of Data segment nodes.
     */
    if (!error)
    {
        error = nfc_hal_nci_parse_ramdump_config_create_dump_area(sz);
    }
    /***
     * Initiate or stop the ramdump process
     */
    if (!error)
    {
        /*
         * We have succesfully setup memory, files & variables to begin a RAMPDUMP process.
         * Initiate the RAMDUMP process.
         */
        HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: Initiating ramdump communication with NFCC. Sending NFCC PROP_INIT_RAMDUMP_CMD");
        nfc_hal_dm_send_prop_init_ramdump_cmd();
    }
    else
    {
        /* End the RamDump prematurely */
        HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_init_ramdump_ntf: RAMDUMP: Prematurely ending ramdump communication with NFCC. Sending PROP END CMD.");
        nfc_hal_dm_send_prop_end_ramdump_cmd();
    }
}

/*******************************************************************************
**
** Function         nfc_hal_nci_invoke_ramdump_upload
**
** Description      Initiate the next ramdump upload.
**                  Ask for a max of 252 buffers to avoid segmentation.
**                  This means we manage the request and assembly of those parts
**                  ourselves, however, this saves on memory and copying.
**                  If we didn't care about segmentation the Reader thread would
**                  assemble the fragments into one big buffer then pass it to us.
**                  We do not get ownership of this buffer and would need to copy
**                  it locally as the Reader thread needs its limited buffers for
**                  other async events.
**                  Alternatively, create the local buffer as we know its max size
**                  and copy the fragments into it as they arrive. This saves one
**                  big copy and two large buffers required in the segmentation
**                  solution.
**
** Returns          none
**
*******************************************************************************/
static void nfc_hal_nci_invoke_ramdump_upload()
{
    /* check for config errors */
    if (gRamdump_Data.cd_current_item == NULL)
    {
        HAL_TRACE_DEBUG0("nfc_hal_nci_invoke_ramdump_upload: RAMDUMP: Config node non-existant! Ending ramdump communications.");
        nfc_hal_dm_send_prop_end_ramdump_cmd();
        return;
    }
    HAL_TRACE_DEBUG3 ("nfc_hal_nci_invoke_ramdump_upload: RAMDUMP: Initiating ramdump upload for item %p, NAME %s, total LENGTH %d bytes",
                        gRamdump_Data.cd_current_item, gRamdump_Data.cd_current_item->name, gRamdump_Data.cd_current_item->dump_length);
    /* to avoid segmentation we limit the number of bytes requested in each payload */
    int length_left = gRamdump_Data.cd_current_item->dump_length - gRamdump_Data.total_received;
    length_left = (length_left < nfc_hal_cb.ncit_cb.nci_ctrl_size) ? length_left : nfc_hal_cb.ncit_cb.nci_ctrl_size;

    HAL_TRACE_DEBUG1 ("nfc_hal_nci_invoke_ramdump_upload: RAMDUMP: length_left = %d",length_left);
    //assert (length_left % 4 == 0); /* requirement! */
    gRamdump_Data.bytes_requested = length_left;
    HAL_TRACE_DEBUG2 ("nfc_hal_nci_invoke_ramdump_upload: RAMDUMP: Sending NFCC PROP_GET_RAMDUMP_CMD requesting %d (of %d) bytes in this segment", length_left, (gRamdump_Data.cd_current_item->dump_length - gRamdump_Data.total_received));
    nfc_hal_dm_send_prop_get_ramdump_cmd((gRamdump_Data.cd_current_item->dump_start_addr + gRamdump_Data.total_received), length_left);
}

/*******************************************************************************
**
** Function         nfc_hal_nci_recv_init_ramdump_rsp
**
** Description      Perform action following receiving PROP_INIT_RAMDUMP_RSP notification from NFCC.
**
** Returns          none
**
*******************************************************************************/
void nfc_hal_nci_recv_init_ramdump_rsp()
{
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_init_ramdump_rsp: RAMDUMP: Received a PROP_INIT_RAMDUMP_RSP from NFCC");

    /* copy the event payload register data */
    memcpy(gRamdump_Data.register_buffer, gRamdump_Data.msg_payload_ptr, gRamdump_Data.msg_payload_length);
    HAL_TRACE_DEBUG1 ("nfc_hal_nci_recv_init_ramdump_rsp: RAMDUMP: Saved %d register data bytes to buffer.", gRamdump_Data.msg_payload_length);
    /***
     * Parse and place the data into the output file.
     */
    nfc_hal_nci_parse_and_dump_ramdump_register_data();
    /* We are finished with the register data as we now have it in the output file */
    free (gRamdump_Data.register_buffer);
    gRamdump_Data.register_buffer = NULL;
    /***
     * Start the RAMDUMP upload
     */

    /* start from beginning */
    gRamdump_Data.total_received = 0;
    gRamdump_Data.cd_current_item = gRamdump_Data.cd_list;
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_init_ramdump_rsp: RAMDUMP: Requesting first segment of data for initial config file entry");
    nfc_hal_nci_invoke_ramdump_upload();
}

/*******************************************************************************
**
** Function         nfc_hal_nci_recv_get_ramdump_rsp
**
** Description      Perform action following receiving PROP_GET_RAMDUMP_RSP notification from NFCC.
**
** Returns          none
**
*******************************************************************************/
void nfc_hal_nci_recv_get_ramdump_rsp()
{
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_get_ramdump_rsp: RAMDUMP: Received a PROP_GET_RAMDUMP_RSP from NFCC");
    /*
     * Send the following;
     * Start Address    4    32 bit start address for data to upload. This value is interpreted in Big Endian format
     * Length           1    Number of bytes starting (inclusive) at Start Address to transfer
     *                       This value is required to be a multiple of 4 (as we always transfer 32 bit words) between 4 and 252
     *
     */
    HAL_TRACE_DEBUG3 ("nfc_hal_nci_recv_get_ramdump_rsp: RAMDUMP: Received %d bytes of expected %d bytes for '%s'",
                        gRamdump_Data.msg_payload_length, gRamdump_Data.bytes_requested, gRamdump_Data.cd_current_item->name);

    assert(gRamdump_Data.msg_payload_length == gRamdump_Data.bytes_requested);
    assert((gRamdump_Data.total_received + gRamdump_Data.msg_payload_length) <= gRamdump_Data.cd_current_item->dump_length);

    /* copy the event payload register data for later expansion */
    memcpy(gRamdump_Data.data + gRamdump_Data.total_received, gRamdump_Data.msg_payload_ptr, gRamdump_Data.msg_payload_length);

    HAL_TRACE_DEBUG4 ("nfc_hal_nci_recv_get_ramdump_rsp: RAMDUMP: Buffer start %p + offset %d = %p (buffer boundary %p)",
                        gRamdump_Data.data, gRamdump_Data.total_received, (gRamdump_Data.data + gRamdump_Data.total_received), (gRamdump_Data.data + gRamdump_Data.data_length));
    HAL_TRACE_DEBUG1 ("nfc_hal_nci_recv_get_ramdump_rsp: RAMDUMP: Saved %d register data bytes to buffer", gRamdump_Data.msg_payload_length);
                        gRamdump_Data.total_received += gRamdump_Data.msg_payload_length;

    /* Are there more segments for this dump?  */
    if ((gRamdump_Data.cd_current_item->dump_length - gRamdump_Data.total_received) > 0)
    {
        /***
         * Obtain next segment
         */
        HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_get_ramdump_rsp: RAMDUMP: Requesting next segment of data...");
        nfc_hal_nci_invoke_ramdump_upload();
    }
    else
    {
        /***
         * Segments complete! Process the data.
         * Merge the ramdump data into the output file.
         */
        nfc_hal_nci_parse_and_dump_ramdump_data();

        /***
         * Are there further ram dumps to upload?
         */
        if (gRamdump_Data.cd_current_item->next != NULL)
        {
        /***
         * Start the upload process again
         */
            gRamdump_Data.total_received = 0;
            gRamdump_Data.cd_current_item = gRamdump_Data.cd_current_item->next;
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_get_ramdump_rsp: RAMDUMP: Requesting first segment of data for next config file entry");
            nfc_hal_nci_invoke_ramdump_upload();
        }
        else
        {
            /***
             * No more dumps to upload. Initiate end of session
             */
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_get_ramdump_rsp: RAMDUMP: Obtained all ramdump data. Sending NFCC PROP_END_RAMDUMP_CMD");
            nfc_hal_dm_send_prop_end_ramdump_cmd();
        }
    }
}

/*******************************************************************************
**
** Function         nfc_hal_nci_recv_end_ramdump_rsp
**
** Description      Perform action following receiving PROP_END_RAMDUMP_RSP notification from NFCC.
**
** Returns          none
**
*******************************************************************************/
void nfc_hal_nci_recv_end_ramdump_rsp()
{
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_end_ramdump_rsp: RAMDUMP: Ending ramdump and restarting services");

    /* Indicate Ramdump complete by cleaning up */
    nfc_hal_nci_close_ramdump();

    /*
     * Power cycle the NFC Service.
     * Start pre-initializing NFCC.
     */
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_end_ramdump_rsp: RAMDUMP: Ending ramdump cycle. Enabling PowerCycle");
    /* Reset initialisation variables. Should be part of power cycle routine! */
    nfc_hal_cb.dev_cb.patch_applied = FALSE;
    nfc_hal_cb.dev_cb.pre_init_done = FALSE;
    nfc_hal_cb.dev_cb.pre_patch_signature_chk = FALSE;
    nfc_hal_cb.dev_cb.patch_signature_chk = FALSE;
    nfc_hal_cb.dev_cb.pre_patch_applied = FALSE;
    nfc_hal_cb.dev_cb.pre_patch_file_available = FALSE;
    nfc_hal_cb.dev_cb.fw_version_chk = FALSE;

    DT_Nfc_RamdumpPerformed = TRUE;
    GKI_send_event (NFC_HAL_TASK, NFC_HAL_TASK_EVT_POWER_CYCLE);
}

/*******************************************************************************
**
** Function         nfc_hal_nci_parse_ramdump_config_create_dump_area
**
** Description      Looks into the config buffer and pulls out the START addrs and LENGTHs.
**
** Returns          Largest buffer length
**
*******************************************************************************/
static BOOLEAN nfc_hal_nci_parse_ramdump_config_create_dump_area(int buf_len)
{
    HAL_TRACE_DEBUG1 ("nfc_hal_nci_parse_ramdump_config_create_dump_area: RAMDUMP: Extracting config data and creating data buffers, buffer size %d", buf_len);
    /**
     *
     * int GetStrValue(const char* name, char* p_value, unsigned long len);
     * int GetNumValue(const char* name, void* p_value, unsigned long len);
     *
     * Format of a line from the config file, e.g. 'NAME "P1 stack" START 0x10000 LENGTH 512'
     */
    static const char kRamdump_config_line_format[] = "%*[\t\r\n ]NAME %s START 0x%x LENGTH %d%n";
    static const char kRamdump_config_alt1_line_format[] = "NAME %s START 0x%x LENGTH %d%n";
    static const char kRamdump_config_alt2_line_format[] = "NAME %s START 0x%x LENGTH %d%n%*[\t\r\n ]";
    static const char kRamdump_config_alt3_line_format[] = "%*[\t\r\n ]NAME %s START 0x%x LENGTH %d%n%*[\t\r\n ]";
    static const char kRamdump_config_alt4_line_format[] = "%*[\t\r\n0 ]";
    /*
     * We test against this to determine if we want to parse anymore. Should never get data less than this length as a START address will always be at least 8 digits (4 bytes).
     * The most we can expect from this string is six digits;
     * I.e. 1 char from NAME (if name is one char), 3 chars from LENGTH (if length is one digit). Add these to 2 chars from START, gives us 6 digits for an address!
     */
    int format_len = strlen(kRamdump_config_alt1_line_format) + 2; /* 6 digits + 2 = 8 digits for the START address */

    const int kExpected_config_inputs = 3;
    BOOLEAN error = FALSE;
    int items_filled = 0;
    int max_length = 0;
    int buffer_offset = 0;
    ConfigData** cd_pptr = &gRamdump_Data.cd_list;
#ifdef RAMDUMP
    int count = 0;
#endif
    /* Begin parsing */
    do
    {
        BOOLEAN parsed_something = TRUE;
        unsigned int dump_start_addr = 0;
        int dump_length = 0;
        char name[MAX_CONFIG_NAME];
#ifdef RAMDUMP
        HAL_TRACE_DEBUG0 ("-------------------");
        HAL_TRACE_DEBUG1 ("%s\n", gRamdump_Data.config_buffer + buffer_offset);
        HAL_TRACE_DEBUG0 ("-------------------");
#endif
#define MATCHED_SCAN1 1
#define MATCHED_SCAN2 2
#define MATCHED_SCAN3 3
#define MATCHED_SCAN4 4
#define MATCHED_SCAN5 5
        int bytes_read = 0;
        int matched_scan = MATCHED_SCAN1;
        if ((items_filled = sscanf ((const char *)gRamdump_Data.config_buffer + buffer_offset, kRamdump_config_line_format, &name[0], &dump_start_addr, &dump_length, &bytes_read)) <= 0)
        {
            matched_scan = MATCHED_SCAN2;
            if ((items_filled = sscanf ((const char *)gRamdump_Data.config_buffer + buffer_offset, kRamdump_config_alt1_line_format, &name[0], &dump_start_addr, &dump_length, &bytes_read)) <= 0)
            {
                matched_scan = MATCHED_SCAN3;
                if ((items_filled = sscanf ((const char *)gRamdump_Data.config_buffer + buffer_offset, kRamdump_config_alt2_line_format, &name[0], &dump_start_addr, &dump_length, &bytes_read)) <= 0)
                {
                    matched_scan = MATCHED_SCAN4;
                    if ((items_filled = sscanf ((const char *)gRamdump_Data.config_buffer + buffer_offset, kRamdump_config_alt3_line_format, &name[0], &dump_start_addr, &dump_length, &bytes_read)) <= 0)
                    {
                        matched_scan = MATCHED_SCAN5;
                        if ((items_filled = sscanf ((const char *)gRamdump_Data.config_buffer + buffer_offset, kRamdump_config_alt4_line_format)) <= 0)
                        {
                            if (items_filled <= 0 || items_filled == EOF)
                            {
                                HAL_TRACE_DEBUG0("nfc_hal_nci_parse_ramdump_config_create_dump_area: RAMDUMP: can not parse rest of file");
                                parsed_something = FALSE;
                            }
                        }
                    }
                }
            }
        }
        buffer_offset += bytes_read;
        if (parsed_something)
        {
            HAL_TRACE_DEBUG3 ("*** Bytes read %d, Buffer length %d, Buffer offset %d", bytes_read, buf_len, buffer_offset);
            HAL_TRACE_DEBUG3 ("*** Found string: \"NAME %s START 0x%X LENGTH %d\"", name, dump_start_addr, dump_length);
            HAL_TRACE_DEBUG1 ("*** Using scan pattern %d", matched_scan);
            if (dump_length != 0)
            {
                /* obtain largest buffer length */
                max_length = (dump_length > max_length) ? dump_length : max_length;
                /* create the node */
                HAL_TRACE_DEBUG0("Creating config node...");
                if ((*cd_pptr = (ConfigData*) malloc(sizeof(ConfigData))) == NULL)
                {
                    HAL_TRACE_DEBUG0 ("nfc_hal_nci_parse_ramdump_config_create_dump_area: **ERROR** RAMDUMP: Unable to create place holder for buffer");
                    error = TRUE;
                }
                if (!error)
                {
                    (*cd_pptr)->dump_start_addr = dump_start_addr;
                    (*cd_pptr)->dump_length = dump_length;
                    memcpy((*cd_pptr)->name, name, MAX_CONFIG_NAME);
                    HAL_TRACE_DEBUG0("nfc_hal_nci_parse_ramdump_config_create_dump_area: RAMDUMP: Inserting node into config list");
                    /* tail */
                    (*cd_pptr)->next = NULL;
                    cd_pptr = &((*cd_pptr)->next); /* the reason we use a **, make the inserted node's next ptr the head */

#ifdef RAMDUMP
                    ++count;
#endif
                }
            }
            else
            {
                HAL_TRACE_WARNING0("nfc_hal_nci_parse_ramdump_config_create_dump_area: RAMDUMP: **WARNING** Zero LENGTH found. CFG is not well formed! Ignoring entry...");
            }
        }
    } while (!error && (items_filled == kExpected_config_inputs) && ((buf_len-buffer_offset) >= format_len));
    /*
     * Check for config errors, a config with only a zero LENGTH entry or a comment.
     * We do this because if we send an INIT CMD to the NFCC and we have no nodes, the NFCC is in 'RAM Dump Requested' state, and
     * we can not end the sequence as the design states the NFCC will only accept ramdump GETs.
     * However, we also can not send GETs with zero LENGTHS!
     */
    if (gRamdump_Data.cd_list == NULL)
    {
        HAL_TRACE_DEBUG0("nfc_hal_nci_parse_ramdump_config_create_dump_area: RAMDUMP: No config nodes created!");
        error = TRUE;
    }
#ifdef RAMDUMP
    if (!error)
    {
        ConfigData* node_ptr = gRamdump_Data.cd_list;
        int new_count = 0;
        while(node_ptr != NULL)
        {
            node_ptr = node_ptr->next;
            ++new_count;
        }
        if (new_count != count)
        {
            HAL_TRACE_DEBUG2("nfc_hal_nci_parse_ramdump_config_create_dump_area: RAMDUMP: **ERROR** Nodes count do not match (expected %d, got %d)", count, new_count);
        }
    }
#endif
    /* create the largest payload data buffer */
    if (!error && (gRamdump_Data.data = (UINT8*) malloc(max_length)) == NULL)
    {
        HAL_TRACE_DEBUG1 ("nfc_hal_nci_parse_ramdump_config_create_dump_area: RAMDUMP: **ERROR** Unable to malloc ramdump dump buffer of %d bytes", max_length);
        error = TRUE;
    }
    else
    {
        gRamdump_Data.data_length = max_length;
    }
    /* We no longer need the config memory area */
    free(gRamdump_Data.config_buffer);
    gRamdump_Data.config_buffer = NULL;
    /* if any error, say on the nth + 1 created node, we cleanup later */
    return error;
}

/*******************************************************************************
**
** Function         nfc_hal_nci_parse_and_dump_ramdump_register_data
**
** Description      Looks into the register buffer and merges the data into the required format into the output file.
**
** Returns          none
**
*******************************************************************************/
void nfc_hal_nci_parse_and_dump_ramdump_register_data()
{
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_parse_and_dump_ramdump_register_data: RAMDUMP: Dump the register data to output");
    /*
     * We know the registers and their order from the standard. If the length changes in any way we are out of sync.
     * When we come to output this register data we depend on the standard's definition for the correlation.
     * At runtime we can only ensure we have the correct amount of data. Ouput what we have.
     *
     * Format of a line from the register file, e.g. 'R0 0x%x |' the continuation bar is replaced by newline for the last entry.
     * Note: Before printing your hex values you must arrange them into the correct Endianness
     */
    static const char kRamdump_register_dump_format[] = "%s 0x%02x%02x%02x%02x\n";
#define MAX_REGISTER_COUNT 50
    char* kRamdump_register_names[MAX_REGISTER_COUNT] = {"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12", "MSP", "PSP", "LR", "PC", "xPSR"};
    if(gNfccChipVersion == 24)
    {
        kRamdump_register_names[18] = "HARD_FAULT_LR";
        kRamdump_register_names[19] = "PRIMASK";
        kRamdump_register_names[20] = "CONTROL";
    }
    else
    {
        /* gNfccChipVersion == 21 - default case, collect minimum */
        kRamdump_register_names[18] = "PRIMASK";
        kRamdump_register_names[19] = "CONTROL";
    }

    int buffer_offset = 0;
    int i = 0;
    for (; i < gMaxRegisterCount; ++i)
    {
        /* register_buffer is a UINT8*. Cast the ptr as an int* and use ptr arithmetic to obtain the offset */
        if (fprintf (gRamdump_Data.output_file_ptr, kRamdump_register_dump_format, kRamdump_register_names[i], gRamdump_Data.register_buffer[buffer_offset+3],
                                                                                                               gRamdump_Data.register_buffer[buffer_offset+2],
                                                                                                               gRamdump_Data.register_buffer[buffer_offset+1],
                                                                                                               gRamdump_Data.register_buffer[buffer_offset]) < 0)
        {
            HAL_TRACE_WARNING2 ("nfc_hal_nci_parse_and_dump_ramdump_register_data: RAMDUMP: **WARNING** Problem writing register data for register \"%s\" to output file (%s)",
                               kRamdump_register_names[i], strerror(errno));
        }
        buffer_offset += 4;
    }
    if (fflush (gRamdump_Data.output_file_ptr) != 0)
    {
        HAL_TRACE_WARNING1 ("nfc_hal_nci_parse_and_dump_ramdump_register_data: RAMDUMP: **WARNING** Problem flushing output file (%s)",
                           strerror(errno));
    }
}

/*******************************************************************************
**
** Function         nfc_hal_nci_parse_and_dump_ramdump_data
**
** Description      For the current dump, looks into the data buffer and merges the data into the required format into the output file.
**
** Returns          none
**
*******************************************************************************/
void nfc_hal_nci_parse_and_dump_ramdump_data()
{
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_parse_and_dump_ramdump_data: RAMDUMP: Dump ramdump data to file");
    /*
     * Format of a line from the ramdump file, e.g. 'NAME \"%s\" START 0x%x DATA 0x%x'
     * Note: Before printing your hex values you must arrange them into the correct Endianness
     */
    if (fprintf (gRamdump_Data.output_file_ptr, "NAME \"%s\" START 0x%x DATA 0x", gRamdump_Data.cd_current_item->name, gRamdump_Data.cd_current_item->dump_start_addr) < 0)
    {
        HAL_TRACE_WARNING2 ("nfc_hal_nci_parse_and_dump_ramdump_data: RAMDUMP: **WARNING** Problem writing dump header for NAME \"%s\" to output file (%s)",
                           gRamdump_Data.cd_current_item->name, strerror(errno));
    }
    /* followed by the data */
    int i = 0;
    for (; i < gRamdump_Data.cd_current_item->dump_length; ++i)
    {
        if (fprintf (gRamdump_Data.output_file_ptr, "%02x", gRamdump_Data.data[i]) < 0)
        {
            HAL_TRACE_WARNING2 ("nfc_hal_nci_parse_and_dump_ramdump_data: RAMDUMP: **WARNING** Problem writing dump data for NAME \"%s\" to output file (%s)",
                               gRamdump_Data.cd_current_item->name, strerror(errno));
        }
    }
    /* Lets separate our data entries one per line */
    if (fprintf (gRamdump_Data.output_file_ptr, "%c", '\n') < 0)
    {
        HAL_TRACE_WARNING2 ("nfc_hal_nci_parse_and_dump_ramdump_register_data: RAMDUMP: **WARNING** Problem writing terminal for register data name %s to output file (%s)",
                           gRamdump_Data.cd_current_item->name, strerror(errno));
    }
    if (fflush (gRamdump_Data.output_file_ptr) != 0)
    {
        HAL_TRACE_WARNING1 ("nfc_hal_nci_parse_and_dump_ramdump_data: RAMDUMP: **WARNING** Problem flushing output file (%s)", strerror(errno));
    }
}

/*******************************************************************************
**
** Function         nfc_hal_nci_close_ramdump
**
** Description      Housekeeping function to tidy data and files.
**
** Returns          none
**
*******************************************************************************/
void nfc_hal_nci_close_ramdump()
{
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_close_ramdump: RAMDUMP: Performing cleanup");
    if (gRamdump_Data.ramdump_in_progress)
    {
        if (gRamdump_Data.output_file_ptr && fclose (gRamdump_Data.output_file_ptr) == EOF)
        {
            HAL_TRACE_WARNING0 ("nfc_hal_nci_close_ramdump: RAMDUMP: **WARNING** Unable to close ramdump file. Continuing...");
        }
        else
        {
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_close_ramdump: RAMDUMP: ramdump file closed.");
            gRamdump_Data.output_file_ptr = NULL;
        }
        /* house keeping */
        if (gRamdump_Data.data != NULL)
        {
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_close_ramdump: RAMDUMP: freeing gRamdump_Data.data");
            free (gRamdump_Data.data);
            gRamdump_Data.data = NULL;
        }
        /* release the config data linked list */
        if (gRamdump_Data.cd_list != NULL)
        {
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_close_ramdump: RAMDUMP: freeing gRamdump_Data.cd_list");
            ConfigData* node = gRamdump_Data.cd_list;
            while (node != NULL)
            {
                ConfigData* next_node = node->next;
                HAL_TRACE_DEBUG2("freeing config node '%s', length %d...", node->name, node->dump_length);
                free(node);
                node = NULL;
                node = next_node;
            }
        }
        /* Depending on when we are called, this part may already be freed */
        if (gRamdump_Data.register_buffer != NULL)
        {
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_close_ramdump: RAMDUMP: freeing gRamdump_Data.register_buffer");
            free (gRamdump_Data.register_buffer);
            gRamdump_Data.register_buffer = NULL;
        }
        /* Depending on when we are called, this part may already be freed */
        if (gRamdump_Data.config_buffer != NULL)
        {
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_close_ramdump: RAMDUMP: freeing gRamdump_Data.config_buffer");
            free(gRamdump_Data.config_buffer);
            gRamdump_Data.config_buffer = NULL;
        }
    }
    /* Zero out. May not be last ram dump */
    memset(&gRamdump_Data, 0, sizeof(RamDump_Data));
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_close_ramdump: RAMDUMP: Cleanup complete!");
}

/*******************************************************************************
**
** Function         ConstructTestPersistenceFile
**
** Description      Creates test data into the the persistence buffer.
**
** Param            buffer_ptr    Buffer pointer to insert data to.
**
*******************************************************************************/
void ConstructTestPersistenceFile(UINT8 *buffer_ptr)
{
    /* Useful to have if you have no 3.0 NFCC chip at hand.
       Creates a new test file for reading at start up. */

    /* Tests to run to verify code...

       Two variants; (v1) Nfcc_Storage_OneEntryPerPacket and (v2) Nfcc_Storage_ManyEntriesPerPacket.
       The third variant, Nfcc_Storage_NfccResetInProgress, is not considered here and requires an NFCC reset.

       pre-3.0 and 3.0 NFCC chip tests.
       ------------------------
       Test 1: File doesn't exist.
       Test 2: Reboot device, test for empty existing file.
       Test 3: Existing file with one entry, CMDs should be sent. Syntax error in RSP.
       Test 4: Existing file with approx. 6 entries, or entries that do not overflow a packet.
       Test 5: Existing file with entries that may overflow several packets.

       3.0 NFCC chip tests.
       -----------------------------
       Test 6: File doesn't exist. Empty file created and NTFs arriving that populate it.
       Test 7: Reboot device. CMDs sent for those NTFs stored previously.
       Test 8: Deletion or Delete ALL NTF. Should be removed from table
       Test 9: Reboot device. CMDs should NOT be sent fro those entries deleted. */

    HAL_TRACE_DEBUG0 ("PROVIDING TEST BUFFER INSTEAD OF READING FROM FILE...");

    /* construct the test records */
   UINT8 test_type = Nfcc_Storage_TestTypeStart;
   int rec_count = 0;
   for (; rec_count < Nfcc_Storage_TestTypeCount; ++rec_count, ++test_type)
   {
        /* which byte is this bit in? */
        int byte = TLV_MASK_LENGTH(test_type)-1; /* Offset from 0 */

        /* set or clear the bit */
        HAL_TRACE_DEBUG3 ("ConstructTestPersistenceFile: NFCC_STORAGE: BEFORE: NFCC Storage type: 0x%02X, byte: %d, byte_mask: 0x%02X", test_type, byte, gNfccStorageFileHeader.item_mask[byte]);
        /* create mask for type bit */

        UINT8 bit_mask = Nfcc_Storage_Tlv_MSB >> (test_type % CHAR_BIT);
        /* Turn bit on */
        gNfccStorageFileHeader.item_mask[byte] |= bit_mask;
        HAL_TRACE_DEBUG3 ("ConstructTestPersistenceFile: NFCC_STORAGE: AFTER:  NFCC Storage type: 0x%02X, byte: %d, byte_mask: 0x%02X", test_type, byte, gNfccStorageFileHeader.item_mask[byte]);

        /* Copy it into the file */
        memmove(buffer_ptr, &gNfccStorageFileHeader, sizeof(struct sNfccStorageFileHeader));

        UINT8* payload_ptr = (UINT8*) buffer_ptr + sizeof(struct sNfccStorageFileHeader) + (test_type * sizeof(struct sTlvEntry));
        HAL_TRACE_DEBUG4 ("Placing new item at offset %d in the file (%d +(%d * %d)))",
           sizeof(struct sNfccStorageFileHeader) + (test_type * sizeof(struct sTlvEntry)),
           sizeof(struct sNfccStorageFileHeader), test_type, sizeof(struct sTlvEntry));

        *payload_ptr++ = test_type;                              /* UINT8   type */
        *payload_ptr++ = Nfcc_Storage_TestLength;                /* UINT8   value length */
        memset(payload_ptr, 0xBB, Nfcc_Storage_TestLength);      /* dummy values for payload */

        HAL_TRACE_DEBUG0 ("HEADER...");
        DISP_NCI(buffer_ptr, sizeof(struct sNfccStorageFileHeader), FALSE);
        HAL_TRACE_DEBUG1 ("WORD_SIZE is: %d", WORD_SIZE);
        HAL_TRACE_DEBUG1 ("item_mask size in bytes is: %d", sizeof(gNfccStorageFileHeader.item_mask));
        HAL_TRACE_DEBUG1 ("pad size in bytes is: %d", sizeof(gNfccStorageFileHeader.pad));
        HAL_TRACE_DEBUG1 ("ENTRY AT %d...", sizeof(struct sNfccStorageFileHeader) + (test_type * sizeof(struct sTlvEntry)));
        DISP_NCI(buffer_ptr + sizeof(struct sNfccStorageFileHeader) + (test_type * sizeof(struct sTlvEntry)), sizeof(struct sTlvEntry), FALSE);
    }
}
/*******************************************************************************
**
** Function         nfc_hal_nci_open_and_read_nfcc_storage_file
**
** Description      Looks for the NFCC storage file and reads it into a buffer.
**                  Creates and populates the file if not found.
**                  If the file is created, or opened and read successfully then gTlvEnabled is set to TRUE.
**                  gTlvEnabled signifies persistence of NFCC storage entries and leaves the file open.
**                  The caller takes ownership of the buffer which should be deleted.
**                  It is not used for persistence but to send CMDs efficiently.
**
** Param            Out: Address of buffer pointer if file exists and successfully read, NULL otherwise.
**
** Return           -1 on Error, 0 otherwise (i.e. out param could also be NULL when no persistence file initially created)
*******************************************************************************/
int nfc_hal_nci_open_and_read_nfcc_storage_file (UINT8** store_buffer_pptr)
{
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file");

    int ret = 0;
    *store_buffer_pptr = NULL;

    gTlvEnabled = FALSE;
    BOOLEAN error = FALSE;
    BOOLEAN file_exists = FALSE;
    BOOLEAN recreate_file = FALSE;

    if(GetStrValue("NFCC_STORAGE_FILE", &kNfcc_storage_filename[0], NFCC_STORAGE_FILENAME_LEN) == FALSE)
    {
        HAL_TRACE_ERROR0("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: **ERROR** NFCC_STORAGE_FILE attribute not found in NFC config file");
        error = TRUE;
        ret = -1;
    }

    if (!error)
    {
#define MAX_RECREATE_RETRIES 3
        static int recreate_tries = 0;

        do
        {
            /***
             * Open the file checking if it exists. Open for read/write
             */
            gNfcc_storage_file_ptr = fopen (kNfcc_storage_filename, "r+b");
            if (gNfcc_storage_file_ptr == NULL)
            {
                /* check file does not exist */
                if (ENOENT == errno)
                {
                    /* file doesn't exist, so we need to create it */
                }
                else
                {
                    /* file exists but can't be opened */
                    HAL_TRACE_ERROR2 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: **ERROR** Unable to open file \"%s\" (%s).", kNfcc_storage_filename, strerror(errno));
                    file_exists = TRUE;
                    recreate_file = TRUE;
                    error = TRUE;
                }
            }
            else
            {
                /* file opened fine */
                file_exists = TRUE;
            }

            if (!error)
            {
                if (file_exists)
                {
                    /*
                     * Obtain size of file.
                     */
                    if (fseek(gNfcc_storage_file_ptr, 0L, SEEK_END) != 0)
                    {
                        HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: **ERROR** Unable to position to end of file.");
                        error = TRUE;
                    }

                    if (!error && ((gNfcc_storage_filesize = ftell(gNfcc_storage_file_ptr)) <= 0))
                    {
                        HAL_TRACE_WARNING0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: **WARNING** Unable to obtain file size, may be corrupt.");
                        error = TRUE;
                    }
                    if (!error)
                    {
                        if (fseek(gNfcc_storage_file_ptr, 0L, SEEK_SET) != 0)
                        {
                            HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: **ERROR** Unable to position to beginning of file.");
                            error = TRUE;
                        }

                        /* Read the file into the buffer for passing back to the caller */
                        if (!error)
                        {
                            /* Create output buffer to write */
                            *store_buffer_pptr = (UINT8*) malloc(gNfcc_storage_filesize);
                            if (*store_buffer_pptr == NULL)
                            {
                                HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: Can not malloc output data buffer for existing file.");
                                error = TRUE;
                            }

                            if (!error)
                            {
                                HAL_TRACE_DEBUG1 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: Reading existing NFCC Storage file. Bytes %d", gNfcc_storage_filesize);

                                size_t bytes_read = 0;
                                if (((bytes_read = fread (*store_buffer_pptr, sizeof(UINT8), gNfcc_storage_filesize, gNfcc_storage_file_ptr)) == 0) || (bytes_read != gNfcc_storage_filesize))
                                {
                                    HAL_TRACE_DEBUG2 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: **ERROR** Wrong number of expected bytes read from file (expected %d, read %d), or file empty", gNfcc_storage_filesize, bytes_read);
                                    error = TRUE;
                                }
                            }

                            if (!error)
                            {
                                /* Copy the file header into our global variable.
                                 * Even though we have the file data in memory we use the global header variable always, as this file is only temporary for sending the NFCC CMDs.
                                 * This global is especially useful in persisting the NTFs. We only need to update and write it as it's always in memory,
                                 * as opposed to reading it from file each time!
                                 *
                                 * TBD: Add version control to header as the size may be different, i.e. we could be using a 32-bit version on a 64-bit platform!!!
                                 */
                                memcpy(&gNfccStorageFileHeader, *store_buffer_pptr, sizeof(struct sNfccStorageFileHeader));
                                HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: File header data copied to global.");

                                gTlvEnabled = TRUE;
                            }
                        }
                    }

                    /* If error reading the existing file */
                    if (error)
                    {
                        /*
                         * At this point we give up so we delete the file and start a new.
                         * Maybe a redesign to always use buffers but that may affect performance when persisting the whole file.
                         */
                        HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: **ERROR** Existing file is corrupt OR cannot malloc memory."
                                          "Can not update NFCC. Cleaning up and starting a new!");

                        fclose(gNfcc_storage_file_ptr);
                        remove(kNfcc_storage_filename);

                        free(*store_buffer_pptr);
                        *store_buffer_pptr = NULL;

                        file_exists = FALSE;
                        error = FALSE; /* reset the error status so as to retry recreating the file */
                    }
                }

                /* doesn't exist, create and open for read/write */
                if (!file_exists)
                {
                    HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: NFCC Storage file does not exist. Creating...");

                    /* As it's a new file, zero out the global header */
                    memset(&gNfccStorageFileHeader, 0, sizeof(struct sNfccStorageFileHeader));
                    HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: Global header data cleared as file is new.");

                    gNfcc_storage_file_ptr = fopen (kNfcc_storage_filename, "w+b");
                    if (gNfcc_storage_file_ptr == NULL)
                    {
                        HAL_TRACE_DEBUG2 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: **ERROR** Unable to create file \"%s\". (%s)",
                                          kNfcc_storage_filename, strerror(errno));

                        error = TRUE;
                        file_exists = FALSE;
                        remove(kNfcc_storage_filename);
                        recreate_file = TRUE;
                    }
                    else
                    {
                        error = FALSE;
                        file_exists = TRUE;
                    }

                    if (!error)
                    {
                        /* New file. Calculate data size to write to the new file */
                        gNfcc_storage_filesize = sizeof(struct sNfccStorageFileHeader) + (TLV_TYPE_MAX_COUNT * sizeof(struct sTlvEntry));
                        HAL_TRACE_DEBUG1 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: Constructing temp buffer for storage new file (size %d bytes).", gNfcc_storage_filesize);

                        /* Create temporary buffer to write */
                        UINT8 *temp = (UINT8*) malloc(gNfcc_storage_filesize);
                        if (temp != NULL)
                        {
                            memset(temp, 0, gNfcc_storage_filesize);

    #ifdef NFCC_STORAGE_TEST
                            /* Useful to have if you have no 3.0 NFCC chip at hand.
                               Creates a new test file for reading at start up. */
                            ConstructTestPersistenceFile(temp);
    #endif /* NFCC_STORAGE_TEST */

                            /* Fill new file with empty records */
                            size_t bytes_written = 0;
                            if (((bytes_written = fwrite (temp, sizeof(UINT8), gNfcc_storage_filesize, gNfcc_storage_file_ptr)) == 0) || (bytes_written != gNfcc_storage_filesize))
                            {
                                HAL_TRACE_DEBUG2 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: **ERROR** Unable to write expected %d bytes to newly created file, actually wrote %d", gNfcc_storage_filesize, bytes_written);
                                error = TRUE;
                            }
                            else
                            {
                                /* Wrote OK, now flush */
                                if (fflush (gNfcc_storage_file_ptr) != 0)
                                {
                                    /* NOTE: Not flushing data to file affects our ability to persist later as we persist NTFs directly to file.
                                     * We can't do anything about that here, but only when the we send the CMD after reboot. I.e. If we haven't managed
                                     * to persist the data will be incorrect and the RSP will indcate to start afresh! */
                                    HAL_TRACE_WARNING1 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: **WARNING** Problem flushing output file (%s). Continuing...",
                                                       strerror(errno));
                                }
                                gTlvEnabled = TRUE;
                                HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: New file created.");
                            }
                            free (temp);
                            temp = NULL;
                        }
                        else
                        {
                            HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: Can not malloc temp data buffer for new file.");
                            error = TRUE;
                        }
                    }
                }
            }

            /* If we couldn't open or read the file we need to delete it.
             * Resetting the UICC has no effect now as we can not create the file! */
            if (error)
            {
                if (file_exists)
                {
                    HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: Persistence is DISABLED due to a file access problem!!! Closing and removing file.");

                    /* Tidy up. */
                    fclose(gNfcc_storage_file_ptr);
                    remove(kNfcc_storage_filename);

                    free(*store_buffer_pptr);
                    *store_buffer_pptr = NULL;

                    file_exists = FALSE;
                    gTlvEnabled = FALSE;
                }
                ret = -1;
            }
            else
            {
                HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: File left open for persistence.");
            }

            HAL_TRACE_DEBUG1 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: gTlvEnabled %d", gTlvEnabled);

            if (recreate_file)
            {
                if (recreate_tries < MAX_RECREATE_RETRIES)
                {
                    /* Recursively call to recreate the file and then to read. */
                    ++recreate_tries;
                    HAL_TRACE_WARNING2 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: **WARNING** Problem creating/opening persistence file, attempting to recreate (%d/%d)...",
                                        recreate_tries, MAX_RECREATE_RETRIES);
                    nfc_hal_nci_initiate_nfcc_storage_check(Nfcc_Storage_NfccResetInProgress);
                    HAL_TRACE_DEBUG0 ("nfc_hal_nci_open_and_read_nfcc_storage_file: Removing persistence file.");
                    remove(kNfcc_storage_filename);
                }
                else
                {
                    gTlvEnabled = FALSE;
                    HAL_TRACE_ERROR1 ("nfc_hal_nci_open_and_read_nfcc_storage_file: NFCC_STORAGE: **ERROR** Maximum recreate attempts reached! gTlvEnabled %d", gTlvEnabled);
                }
            }
        } while ((recreate_file) && (recreate_tries < MAX_RECREATE_RETRIES));
    }
    return ret;
}


/*******************************************************************************
**
** Function         nfc_hal_nci_initiate_nfcc_storage_check
**
** Description      Check for any persisted tlv values and populate the NFCC with them
**                  Called in three ways;
**                  First to initiate sending the CMD,
**                  Secondly to send the next CMD once a RSP comes in.
**                  Third to reset this function on NFCC reset, i.e. static variables.
**                  Once the file is processed calling this function again will have no effect unless reset.
**
**                  Clears the Command Window before sending the next command.
**
**                  Entries are packed into a packet.
**                     -A packet can be specifed to be sent immediately - Nfcc_Storage_OneEntryPerPacket
**                     -A packet can be specified to be sent later - Nfcc_Storage_ManyEntriesPerPacket
**                        When Nfcc_Storage_ManyEntriesPerPacket the packet can be sent on two occasions:
**                           -When the recursion level is reached
**                           -When the packet overflows
**
** Params           send_now, if Nfcc_Storage_OneEntryPerPacket try and send one entry per packet,
**                            if Nfcc_Storage_ManyEntriesPerPacket then many entries per packet.
**                            if Nfcc_Storage_NfccResetInProgress then reinitialise this function to process
**                            the file again.
**
** Returns          Nfcc_Storage_NoPersistentFile; if no persistant file
**                  Nfcc_Storage_AllRecordsProcessed; we have just parsed the file and now all entries have been processed. Expect a RSP.
**                  Nfcc_Storage_No_Unprocessed_Entries; we have just parsed the file and there are no entries to be processed.
**                  Nfcc_Storage_ProcessingOK otherwise.
**
*******************************************************************************/
int nfc_hal_nci_initiate_nfcc_storage_check(tNfcc_Check_Params send_now)
{
    HAL_TRACE_DEBUG1 ("nfc_hal_nci_initiate_nfcc_storage_check; send_now %d", send_now);

#define NEXT_BIT_IN_MASK {                       \
   bit_mask >>= 1;                               \
   type_num++;                                   \
   /* If we have processed all bits in the byte,
    * reset the bit mask for further bytes */    \
   if (!bit_mask)                                \
   {                                             \
      bit_mask = 0x80;                           \
      byte_pos++;                                \
   }                                             \
}

    /* Obtain the data from the file, creating an empty file if it doesn't exist */

    /* Initial values for processing the persistence file from the beginning.
     * Remember across multiple calls of this API.  */
    static UINT8* store_buffer = NULL;
    static int payload_bytes_max = 0;
    static int bytes_remaining = 0;
    /* Initial values for testing the header of the persistence file */
    static int byte_pos = 0;
    static int type_num = 0;
    static UINT8 bit_mask = 0x80;
    static BOOLEAN entry_present = FALSE;

    int ret = Nfcc_Storage_ProcessingOK;

    /* If this is an NFCC reset we need to reinitialise this function so as to parse the persistence file
       from the start. */
    if (send_now == Nfcc_Storage_NfccResetInProgress)
    {
        HAL_TRACE_DEBUG0 ("nfc_hal_nci_initiate_nfcc_storage_check; NFCC_STORAGE: Function reset invoked. Reinitialising to process persistence file again on next call!");

        /* Close the file so as to start parsing again on next open */
        nfc_hal_nci_close_nfcc_storage_file();

        /* On a core_reset_ntf we need to resend HCI to the NFCC.
         * This is because unless there is a power cycle of the UICC the NFCC will not receive this information again.
         * We simply rewind our file parameters to start again.
         * These values should match the initial values for the static variables above */
        if (store_buffer != NULL)
        {
            HAL_TRACE_DEBUG1 ("nfc_hal_nci_initiate_nfcc_storage_check; NFCC_STORAGE: Releasing existing buffer pointer after reset: %p", store_buffer);
            /* After processing store_buffer it will be assigned value Nfcc_Storage_No_Unprocessed_Entries,
             * in the middle of processing it will be a valid pointer!
             * NOTE: NTFs are handled synchronously, so there is no chance that we free the ptr whilst parsing the buffer. */
            if (store_buffer != (UINT8*) Nfcc_Storage_No_Unprocessed_Entries)
            {
                free(store_buffer);
            }
            store_buffer = NULL;
        }
        payload_bytes_max = 0;
        bytes_remaining = 0;
        byte_pos = 0;
        type_num = 0;
        bit_mask = 0x80;
        entry_present = FALSE;

        return ret;
    }

    /* Running for first time? */
    if (store_buffer == NULL)
    {
        int err = nfc_hal_nci_open_and_read_nfcc_storage_file (&store_buffer);
        if (store_buffer == NULL)
        {
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: There was no previous persistent file, so no CMDs will be sent!");
            if (err == 0)
            {
                /* new empty file created */
                ret = Nfcc_Storage_NoUnprocessedEntriesFound;
                store_buffer = ((UINT8*) Nfcc_Storage_No_Unprocessed_Entries);
            }
            else
            {
                /* errors in opening/recreating/reading existing file */
                ret = Nfcc_Storage_NoPersistentFile;
                store_buffer = ((UINT8*) Nfcc_Storage_No_Unprocessed_Entries);
            }
        }
        else
        {
            /* First time so update bytes remaining to fill */
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: Obtaining initial payload count.");
            nfc_hal_dm_send_prop_nfcc_storage_cmd (NULL, FALSE, &payload_bytes_max);
            bytes_remaining = payload_bytes_max;
            HAL_TRACE_DEBUG1 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: Initial payload free is %d bytes.", bytes_remaining);
        }
    }
    else if (store_buffer == ((UINT8*) Nfcc_Storage_No_Unprocessed_Entries))
    {
        /* On subsequent calls we simply want to exit, i.e. new file or processed file. */
        ret = Nfcc_Storage_NoUnprocessedEntriesFound;
    }

    /* Only send CMDs if we managed to obtain persisted data and hadn't processed them previously.
     * We initiate the sequence here but further TLV entries
     * get processed after RSP arrives.
     */
    if (store_buffer != ((UINT8*) Nfcc_Storage_No_Unprocessed_Entries))
    {
        int previously_used = 0;
        BOOLEAN pckt_sent = FALSE;
        const int kLast_byte_pos = TLV_MASK_LENGTH (TLV_TYPE_MAX_COUNT);

        do
        {
            previously_used = payload_bytes_max - bytes_remaining;
            HAL_TRACE_DEBUG1 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: Previously used %d bytes.", previously_used);

            HAL_TRACE_DEBUG0 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: Checking data for any existing records.");

            /*
             * Stop the NFC RSP timer and indicate no RSP outstanding.
             * This is because we know we have received all RSPs upto now and we are hijacking to send Storage CMDs.
             * Once we are done we can restart the timer and give control back.
             */
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: stopping timer.");
            nfc_hal_main_stop_quick_timer (&nfc_hal_cb.ncit_cb.nci_wait_rsp_timer);
            /* We should have already made sure this is the RSP we are waiting for before updating
               the command window, i.e. by calling this function. */
            nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;

            /* For each bit set in each byte of mask send a CMD */

            /* Find next set bit if any.
               Remember, this function is called 1) to initiate the CMD, and 2) once a RSP has arrived.
               Therefore, we haven't an enclosing loop to process all the persistant entries at once. */
            while ((byte_pos < kLast_byte_pos) && !(bit_mask & gNfccStorageFileHeader.item_mask[byte_pos]))
            {
                NEXT_BIT_IN_MASK;
            }

            /* Found something or past the end? We do not want to look past our buffer! */
            if ((byte_pos < kLast_byte_pos) && (bit_mask & gNfccStorageFileHeader.item_mask[byte_pos]))
            {
                entry_present = TRUE;

                /* Send a CMD for this type
                 * Send as many CMDs as we can within a MSG (non-truncated) */
                tTlvEntry *tlvEntry_ptr = (tTlvEntry*) (store_buffer + sizeof(struct sNfccStorageFileHeader) + (type_num * sizeof(struct sTlvEntry)));

                /* Pack as many TLV entries into a MSG before sending. Each entry should NEVER be larger than a packet! */
                HAL_TRACE_DEBUG3 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: Found header item 0x%02X at Byte: %d, Bitmask 0x%02X.", type_num, byte_pos, bit_mask);

            /* Limit recursion levels.
               When 'send_now = TRUE' send straight away.
               When 'send_now = FALSE' pack as much as possible before sending */
                int prev_remaining = bytes_remaining;
                pckt_sent = nfc_hal_dm_send_prop_nfcc_storage_cmd ((UINT8*) tlvEntry_ptr, send_now, &bytes_remaining);
                HAL_TRACE_DEBUG3 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: Bytes free (or '-1' for error): %d. "
                                   "Previously free: %d. Diff: %d (not counting 3 header & 1 TLV count bytes)",
                                   bytes_remaining, prev_remaining, abs(prev_remaining-bytes_remaining));

                /* Check the results */
                if ((pckt_sent == FALSE) && (bytes_remaining == Nfcc_Storage_PayloadTooLarge))
                {
                    /* This should never happen unless the format of Packets or TLV have changed or the entry length was too long */
                    HAL_TRACE_ERROR0("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: **ERROR** Single entry will not fit into packet!!! Ignoring entry and continuing...");
                    /* ignore this entry */
                }
                else
                {
                    /* Have we overflowed the packet and sent previous items, leaving passed entry in fresh packet? */
                    if (pckt_sent == TRUE)
                    {
                        HAL_TRACE_DEBUG0 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: Packet Overflowed so sent, last entry buffered.");
                        /* As we sent the packet we do not want to recurse */
                        previously_used = 0;
                    }

                    /* Ensure our calculations are correct when packing many entries.
                       For one entry per packet (send_now = TRUE) we will always have the whole packet to fill as the packet
                       is sent immediately. */
                    if (send_now == FALSE)
                    {
                        int currently_used = payload_bytes_max - bytes_remaining;
                        int this_tlv = (tlvEntry_ptr->valueLength + Nfcc_Storage_TlvTypeBytes + Nfcc_Storage_TlvLengthBytes);

                        if (this_tlv != (currently_used - previously_used))
                        {
                            HAL_TRACE_ERROR3("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: **ERROR** Bytes used (currently used %d - previously used %d) not equal to TLV size (%d)",
                                currently_used, previously_used, this_tlv);
                        }
                    }
                    else if (bytes_remaining != payload_bytes_max)
                    {
                        HAL_TRACE_ERROR1("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: **ERROR** Sending only one entry per packet should let us use the whole packet! (%d free bytes)",
                            bytes_remaining);
                    }
                }

                /* Progress the bit mask by one position so that we do not process the same entry next time we are called */
                NEXT_BIT_IN_MASK;
            }

            /* Processed all? If so send, even if we supplied FALSE. */
            if (byte_pos == kLast_byte_pos)
            {
                /* As we have processed all entries, send any entries that we have packed but haven't sent yet */
                if (bytes_remaining < payload_bytes_max)
                {
                    HAL_TRACE_DEBUG0 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: No more entries to process, forcing send of packed data.");
                    pckt_sent = nfc_hal_dm_send_prop_nfcc_storage_cmd (NULL, TRUE, &bytes_remaining);
                    HAL_TRACE_DEBUG2("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: Bytes free or '-1' for error: %d (of %d).", bytes_remaining, payload_bytes_max);

                    /* We should have no more bytes to send */
                    if (bytes_remaining != (payload_bytes_max))
                    {
                        HAL_TRACE_ERROR3("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: **ERROR** Bytes remaining in buffer after send (%d)! (%d bytes vs %d expected)", pckt_sent, bytes_remaining, (payload_bytes_max));
                    }

                    /* At the end of the file we need to determine between
                     * 1) a new file
                     * 2) an existing file with no entries
                     * 3) a processed file
                     *
                     * When 'send_now == TRUE' we will not know about previous calls when we reach the end of the buffer
                     * as packets are sent immediately. */
                    if (entry_present == FALSE)
                    {
                        /* Indicate no entries to process in this call, no RSP to expect */
                        ret = Nfcc_Storage_NoUnprocessedEntriesFound;
                    }
                    else
                    {
                        /* Indicate we have just processed all entries. We should expect a RSP */
                        ret = Nfcc_Storage_AllRecordsProcessed;
                    }
                }
                else
                {
                    /* Once all entries processed the only way we end up here is if it is a file with no entries
                       and this is the first time it has been processed. */
                    HAL_TRACE_DEBUG0 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: File has no entries. Exiting...");
                    ret = Nfcc_Storage_NoUnprocessedEntriesFound;
                }

                HAL_TRACE_DEBUG1 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: All items processed. Cleaning up (%p)...", store_buffer);
                free (store_buffer);

                /* Reset 'store_buffer' so if we are called again it is not confused with an actual pointer value or NULL */
                store_buffer = ((UINT8*) Nfcc_Storage_No_Unprocessed_Entries);
            }

#ifdef NFCC_STORAGE
            HAL_TRACE_ERROR2 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: send_now %d, pckt_sent %d",
                               send_now, pckt_sent);
#endif

            /* Do not loop if a packet is sent, wait for the RSP for further processing. */
        } while ((store_buffer != ((UINT8*) Nfcc_Storage_No_Unprocessed_Entries)) && (pckt_sent == FALSE));
    }

#ifdef NFCC_STORAGE
    HAL_TRACE_DEBUG2 ("nfc_hal_nci_initiate_nfcc_storage_check: NFCC_STORAGE: Returning (%p -> %d)", store_buffer, ret);
#endif
    return ret;
}


/*******************************************************************************
**
** Function         nfc_hal_nci_recv_nfcc_storage_rsp
**
** Description      Perform action following receiving NFCC_DH_STORAGE_RSP notification from NFCC
**
**                  If the return status is anything other than OK will attempt a soft reset of the UICC.
**
** Returns          See nfc_hal_nci_initiate_nfcc_storage_check.
**
*******************************************************************************/
int nfc_hal_nci_recv_nfcc_storage_rsp(UINT8 reason, BOOLEAN send_now)
{
    HAL_TRACE_DEBUG1 ("nfc_hal_nci_recv_nfcc_storage_rsp: status 0x%0x", reason);

    /* Error codes to handle
     *
        0x00 NCI STATUS OK
        0x01 NCI STATUS REJECTED
        0x02 NCI STATUS RF FRAME CORRUPTED
        0x03 NCI STATUS FAILED
        0x04 NCI STATUS NOT INITIALISED
        0x05 NCI STATUS SYNTAX ERROR
        0x06 NCI STATUS SEMANTIC ERROR
        0x07 RFU (Formerly NCI STATUS UNKNOWN GID)
        0x08 RFU (Formerly NCI STATUS UNKNOWN OID)
        0x09 NCI STATUS INVALID PARAM
        0x0A NCI STATUS MESSAGE SIZE EXCEEDED
        0x0B-0x9F RFU
        RF Discovery Specific Status Codes
        0xA0 NCI STATUS DISCOVERY ALREADY STARTED
        0xA1 NCI STATUS DISCOVERY TARGET ACTIVATION FAILED
        0xA2 NCI STATUS DISCOVERY TEAR DOWN
        0xA3-0xAF RFU
        RF Interface Specific Status Codes
        0xB0 NCI STATUS RF TRANSMISSION ERROR
        0xB1 NCI STATUS RF PROTOCOL ERROR
        0xB2 NCI STATUS RF TIMEOUT ERROR
        0xB3-0xBF RFU
        NFCEE Interface Specific Status Codes
        0xC0 NCI STATUS NFCEE INTERFACE ACTIVATION FAILED
        0xC1 NCI STATUS NFCEE TRANSMISSION ERROR
        0xC2 NCI STATUS NFCEE PROTOCOL ERROR
        0xC3 NCI STATUS NFCEE TIMEOUT ERROR
        0xC4-0xDF RFU
        Proprietary Status Codes
        0xE0 NCI STATUS LLCP TIMEOUT ERROR
        0xE1 NCI STATUS PATCH OUT OF BOUNDS MEM COPY
        0xE2 NCI STATUS PATCH AUTHENTICAION FAILURE
        0xE3 NCI STATUS PATCH FILE LENGTH FAILURE
        0xE4 NCI STATUS PATCH KEY SHA FAILURE
        0xE5 NCI STATUS PATCH INTERCEPTOR FULL
        0xE6 NCI STATUS PATCH TOO MANY INCREMENTAL PATCHES
        0xE7 NCI STATUS MEMTEST FAILURE
        0xE8 NCI STATUS PATCH ALGORITHM FAILURE
        0xE9 NCI STATUS PATCH OUT OF BOUNDS POINTER
        0xEA NCI STATUS PATCH UNEXPECTED KEY SIZE
        0xEB-0xFF RFU (Proprietary)
     */

    if (reason != NCI_STATUS_OK)
    {
        /* UICC warm reset as we can no longer sync the NFCC. */
        HAL_TRACE_ERROR1 ("nfc_hal_nci_recv_nfcc_storage_rsp(): NFCC_STORAGE: **ERROR**: Reason is something other than NCI_STATUS_OK. "
                          "Attempting to soft reset UICC! Reason code: 0x%02X", reason);
        recreate_hci_persistence_file();
        softUiccResetFireAndForget();
    }

    HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_nfcc_storage_rsp(): NFCC_STORAGE: checking for further persistent entries.");
    return nfc_hal_nci_initiate_nfcc_storage_check(send_now);
}


/*******************************************************************************
**
** Function         nfc_hal_nci_recv_nfcc_storage_ntf
**
** Description      Perform action following receiving NFCC_DH_STORAGE_NTF notification from NFCC
**
** Returns          none.
**
*******************************************************************************/
void nfc_hal_nci_recv_nfcc_storage_ntf(int payload_len, UINT8 * const payload_ptr)
{
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_nfcc_storage_ntf");

    if (gTlvEnabled == TRUE)
    {
        nfc_hal_nci_persist_nfcc_storage_entry(payload_len, payload_ptr);
    }
    else
    {
        HAL_TRACE_DEBUG0 ("nfc_hal_nci_recv_nfcc_storage_ntf: Not persisting HCI NTF as we were unable to open the persistence file.");
    }
}


/*******************************************************************************
**
** Function         nfc_hal_nci_persist_nfcc_storage_entry
**
** Description      Persists the current NFCC Storage entry and header to file.
**                  If the NTF is invalid we ignore it, do not persist it.
**                  The NFCC should be sending us valid packets, so reseting the UICC
**                  won't fix the problem. We simply log it.
**
** Prerequisite     gTlvEnabled is TRUE
**
** Returns          none.
**
*******************************************************************************/
void nfc_hal_nci_persist_nfcc_storage_entry(int payload_len, UINT8 * const payload_ptr)
{
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_persist_nfcc_storage_entry");

    BOOLEAN error = FALSE;
    size_t bytes_written = 0; /* acts as bytes traversed also */
    size_t bytes_to_write = 0;

    if (gNfcc_storage_file_ptr)
    {
        /* Get the number of items in this NTF */
        UINT8 num_items = 0;
        if (payload_ptr)
        {
            num_items = *payload_ptr;
        }
        else
        {
            HAL_TRACE_ERROR1 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **ERROR** Payload pointer is NULL (%p)", payload_ptr);
            error = TRUE;
        }
        HAL_TRACE_DEBUG1 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: Payload states %d items in this NTF", num_items);

        /* cast our payload as a TLV entry */
        tTlvEntry *tlvEntry_ptr = (tTlvEntry *) (payload_ptr + Nfcc_Storage_TlvTypesCountBytes);

        /*
         * Go through the payload and determine the types of entries we have, taking the appropriate action for each.
         */
        int item = 0;
        for (; item < num_items; ++item)
        {
            /* Check the type coming in does not exceed what we expect otherwise we could Buffer Overflow!*/
            if ((tlvEntry_ptr->type > (TLV_TYPE_MAX_COUNT-1)) && (tlvEntry_ptr->type != Nfcc_Storage_TlvDeleteType))
            {
                HAL_TRACE_ERROR3 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **ERROR** Entry Type Out of Bounds! (%d). Only allowed 0x0 - 0x%X and 0x%X",
                                   tlvEntry_ptr->type, (TLV_TYPE_MAX_COUNT-1), Nfcc_Storage_TlvDeleteType);
                return;
            }

            BOOLEAN set_mask = TRUE;
            /* A Delete or Delete All can appear at anytime, embedded or not */
            if (tlvEntry_ptr->type == Nfcc_Storage_TlvDeleteType)
            {
                 /* Indicate delete */
                set_mask = FALSE;
            }

            /* Not a Delete All? (could be just a delete or amendment) */
            if (set_mask)
            {
                /* Amend the header bit to represent the entry */
                /* This is an actual TLV amendment entry so simply write it into our persistence file. */
                HAL_TRACE_DEBUG0 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: Amendment!");

                /* which byte is this type bit in? */
                int byte = TLV_MASK_LENGTH(tlvEntry_ptr->type)-1; /* Offset from 0 */
                if (byte >= TLV_MASK_LENGTH(TLV_TYPE_MAX_COUNT))
                {
                    HAL_TRACE_ERROR1 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **ERROR** Received type %d is invalid and results in an out-of-bounds byte", tlvEntry_ptr->type);
                    return;
                }

                /* Is the value length too long? OK it has to fit in the payload but there could be a discrepancy in the length */
                if (tlvEntry_ptr->valueLength > TLV_MAX_VALUE_LENGTH)
                {
                    HAL_TRACE_ERROR2 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **ERROR** Received type %d with value length %d bytes!", tlvEntry_ptr->type, tlvEntry_ptr->valueLength);
                    return;
                }

                HAL_TRACE_DEBUG3 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: -BEFORE- type: 0x%02X, byte: %d, byte_mask: 0x%02X", tlvEntry_ptr->type, byte, gNfccStorageFileHeader.item_mask[byte]);

                /* create mask for type bit */
                UINT8 bit_mask = Nfcc_Storage_Tlv_MSB >> (tlvEntry_ptr->type % CHAR_BIT);

                if (!(gNfccStorageFileHeader.item_mask[byte] & bit_mask))
                {
                    HAL_TRACE_DEBUG0 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: New entry!");

                    /* Turn bit on in header */
                    gNfccStorageFileHeader.item_mask[byte] |= bit_mask;
                }
                else
                {
                    HAL_TRACE_DEBUG0 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: Existing entry!");
                }

                HAL_TRACE_DEBUG3 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: -AFTER- type: 0x%02X, byte: %d, byte_mask: 0x%02X", tlvEntry_ptr->type, byte, gNfccStorageFileHeader.item_mask[byte]);

                /* Seek to TLV position.
                 * No chance of stale data as the data length protects against this.
                 */
                unsigned int item_position = sizeof(struct sNfccStorageFileHeader) + (tlvEntry_ptr->type * sizeof(struct sTlvEntry));
                HAL_TRACE_DEBUG2 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: Byte position of record: %d (within filesize of %d bytes)", item_position, gNfcc_storage_filesize);

                if (fseek(gNfcc_storage_file_ptr, item_position, SEEK_SET) != 0)
                {
                    HAL_TRACE_ERROR0 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **ERROR** Unable to position to NFCC Storage entry in file.");
                    error = TRUE;
                }

                if (!error)
                {
                    HAL_TRACE_DEBUG0 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: Writing entry data to file");
                    bytes_to_write = Nfcc_Storage_TlvTypeBytes + Nfcc_Storage_TlvLengthBytes + tlvEntry_ptr->valueLength;

                    /* Write TLV */
                    if (  ((bytes_written = fwrite(tlvEntry_ptr, sizeof(UINT8), bytes_to_write, gNfcc_storage_file_ptr)) == 0)
                        || (bytes_written != bytes_to_write))
                    {
                        HAL_TRACE_ERROR1 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **ERROR** Unable to write NFCC Storage entry back to file (%d).", bytes_written);
                        error = TRUE;
                    }
                    else
                    {
                        HAL_TRACE_DEBUG1 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: Wrote %d TLV bytes.", bytes_written);
                    }

                    /* Due to wear levelling allow the higher priority DT thread to read any incoming packets so as to have enough GKI buffers to action them in one go */
                    sched_yield();
                }
            }
            else
            {
                /* What type of delete is this? */
                /* Delete all? */
                if (tlvEntry_ptr->valueLength == 0)
                {
                    HAL_TRACE_DEBUG0 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: 'Delete All' NFCC Storage entry found");
                    /* zero out mask */
                    memset(gNfccStorageFileHeader.item_mask, 0, TLV_MASK_LENGTH(TLV_TYPE_MAX_COUNT));
                    /* point past this entry;  */
                    /* We do not write any TLV bytes for deletion (except for the header). But we need to keep count */
                    bytes_written = Nfcc_Storage_TlvTypeBytes + Nfcc_Storage_TlvLengthBytes;
                }
                else
                {
                    /* This is an actual TLV deletion entry so simply clear the global header mask of its bit. */
                    /* The 2 byte CRC is not a deleted type, ignore it */
                    HAL_TRACE_DEBUG1 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: Deletion! Found %d entries to delete (ignore CRC).", tlvEntry_ptr->valueLength - 2);

                    /* A deletion is followed by list of types to mark as delete */
                    int j = 0;
                    for (; j < tlvEntry_ptr->valueLength - 2; j++)
                    {
                        if(tlvEntry_ptr->value[j] > TLV_TYPE_MAX_COUNT - 1)
                        {
                            HAL_TRACE_ERROR1("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **ERROR** ignore invalid value 0x%02X", tlvEntry_ptr->value[j]);
                            continue;
                        }

                        /* which byte is this type bit in? */
                        int byte = TLV_MASK_LENGTH(tlvEntry_ptr->value[j])-1; /* Offset from 0 */

                        /* create mask for type bit */
                        UINT8 bit_mask = Nfcc_Storage_Tlv_MSB >> (tlvEntry_ptr->value[j] % CHAR_BIT);

                        /* Turn bit off */
                          HAL_TRACE_DEBUG3 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: -BEFORE- type 0x%02X, byte %d, byte_mask: 0x%02X", tlvEntry_ptr->value[j], byte, gNfccStorageFileHeader.item_mask[byte]);
                          gNfccStorageFileHeader.item_mask[byte] &= ~bit_mask;
                          HAL_TRACE_DEBUG3 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: -AFTER- type 0x%02X, byte %d, byte_mask: 0x%02X", tlvEntry_ptr->value[j], byte, gNfccStorageFileHeader.item_mask[byte]);
                    }
                    /* We do not write any TLV bytes for deletion (except for the header). But we need to keep count of our position for ptr checking */
                    /* CRC is part of payload length, don't subtract here */
                    bytes_written = Nfcc_Storage_TlvTypeBytes + Nfcc_Storage_TlvLengthBytes + tlvEntry_ptr->valueLength;
                }
            } /* Delete */

            HAL_TRACE_DEBUG1 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: Bytes traversed so far %d", bytes_written);

            /* No point continuing if there is an error with the file system */
            if (error)
            {
                HAL_TRACE_ERROR0 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **ERROR** Filesystem error. Ignoring futher entries!");
                break;
            }

            /* Increment pointer to point to next record in the payload, if any
               Remember that we move ahead by bytes and not structure sizes! */
            tlvEntry_ptr = (tTlvEntry*) (((UINT8*) tlvEntry_ptr) + bytes_written);
        } /* for */

#ifdef NFCC_STORAGE
        /* Check ptr positioning is correct */
        if (!error && ((UINT8*)(payload_ptr + payload_len)) != ((UINT8*)(tlvEntry_ptr)))
        {
            HAL_TRACE_ERROR4 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **ERROR** At end of NTF, payload_ptr + length != offset_ptr (%p + %d (%p) != %p). Continuing...",
                payload_ptr, payload_len, (payload_ptr + payload_len), tlvEntry_ptr);
        }
#endif

        /* Write header to file now that we're done with these NTFs */
        if (!error && fseek(gNfcc_storage_file_ptr, 0L, SEEK_SET) != 0)
        {
            HAL_TRACE_ERROR0 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **ERROR** Unable to position to beginning of file.");
            error = TRUE;
        }

        if (!error)
        {
            HAL_TRACE_DEBUG0 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: Writing header data to file");

            if (  ((bytes_written = fwrite(&gNfccStorageFileHeader, sizeof(UINT8), sizeof(struct sNfccStorageFileHeader), gNfcc_storage_file_ptr)) == 0)
                || (bytes_written != (sizeof(gNfccStorageFileHeader))))
            {
                HAL_TRACE_ERROR1 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **ERROR** Unable to write NFCC Storage file header back to file (ret %d).", bytes_written);
                error = TRUE;
            }
            HAL_TRACE_DEBUG1 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: Wrote %d header bytes.", bytes_written);
        }

        /* Due to wear levelling allow the higher priority DT thread to read any incoming packets so as to have enough GKI buffers to action them in one go */
        sched_yield();

        /* Push what we have to file */
        if (!error && fflush (gNfcc_storage_file_ptr) != 0)
        {
            HAL_TRACE_WARNING1 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **WARNING** Problem flushing output file (%s). Continuing...",
                                 strerror(errno));
        }
    }
    else
    {
        HAL_TRACE_WARNING0 ("nfc_hal_nci_persist_nfcc_storage_entry: NFCC_STORAGE: **WARNING** Previous problems opening file. Ignoring NTF.");
    }
}


/*******************************************************************************
**
** Function         nfc_hal_nci_close_nfcc_storage_file
**
** Description      Closes the NFCC Storage file.
**
** Returns          none.
**
*******************************************************************************/
void nfc_hal_nci_close_nfcc_storage_file()
{
    HAL_TRACE_DEBUG0 ("nfc_hal_nci_close_nfcc_storage_file.");

    if (gNfcc_storage_file_ptr)
    {
        if (fflush (gNfcc_storage_file_ptr) != 0)
        {
            HAL_TRACE_WARNING1 ("nfc_hal_nci_close_nfcc_storage_file: NFCC_STORAGE: **WARNING** Problem flushing output file (%s). Continuing...",
                               strerror(errno));
        }

        if (fclose (gNfcc_storage_file_ptr) == EOF)
        {
            HAL_TRACE_WARNING0 ("nfc_hal_nci_close_nfcc_storage_file: NFCC_STORAGE: **WARNING** Unable to close file. Continuing...");
        }
    }
}


/*******************************************************************************
**
** Function         find_modem_port
**
** Description      Maps the read Android property value to a QMI port for the
**                  modem that has the physical connection to the card. If there
**                  is no match, a default port is returned.
**
** Returns          Mapped port string value defined by QMI service.
**
*******************************************************************************/
qmi_client_qmux_instance_type find_modem_port(char *prop_value_ptr)
{
    qmi_client_qmux_instance_type qmi_modem_port = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;
    return qmi_modem_port;
}


/*******************************************************************************
**
** Function         qmiClientInitInstanceCallback
**
** Description      Callback for UICC qmi_client_init_instance().
**
** Returns          none.
**
*******************************************************************************/
void qmiClientInitInstanceCallback(qmi_client_type                user_handle,
                                   unsigned int                   msg_id,
                                   void                           *ind_buf,
                                   unsigned int                   ind_buf_len,
                                   void                           *ind_cb_data)
{
    HAL_TRACE_DEBUG0 ("qmiClientInitInstanceCallback(): do nothing!");
}


/*******************************************************************************
**
** Function         srvc_init_client
**
** Description      Initialise the QMI client
**
** Returns          qmi_client_type
**
*******************************************************************************/
qmi_client_type uicc_init_client (qmi_client_qmux_instance_type dev_id, int *qmi_err_code)
{
    HAL_TRACE_DEBUG0 ("uicc_init_client()");

#define INIT_TIMEOUT    4
    qmi_client_os_params                  os_params;
    qmi_idl_service_object_type           client_service;
    qmi_client_type                       qmi_uim_svc_client = NULL;

    if(qmi_err_code == NULL)
    {
        HAL_TRACE_ERROR0 ("uicc_init_client: UICC_RESET: Invalid input, cannot proceed");
        return NULL;
    }

    memset(&os_params,    0, sizeof(qmi_client_os_params));

    client_service = uim_get_service_object_v01();
    if (client_service == NULL)
    {
        HAL_TRACE_ERROR0("uicc_init_client: UICC_RESET: **ERROR** uim_get_service_object_v01 failed. UIM service object not available");
        return NULL;
    }

    /* Call common client layer initialization function */
    *qmi_err_code = qmi_client_init_instance(client_service,
                                             dev_id,
                                             /*(qmi_client_ind_cb)*/ qmiClientInitInstanceCallback,
                                             NULL,
                                             &os_params,
                                             INIT_TIMEOUT,
                                             &qmi_uim_svc_client);

    if (*qmi_err_code != QMI_NO_ERR)
    {
        HAL_TRACE_ERROR1("uicc_init_client: UICC_RESET: **ERROR** qmi_client_init returned failure(%d) for UIM ", (*qmi_err_code));
        return NULL;
    }

    return qmi_uim_svc_client;
}


/*******************************************************************************
**
** Function         softUiccResetCallback
**
** Description      Callback for UICC soft reset status.
**
** Returns          none.
**
*******************************************************************************/
void softUiccResetCallback(qmi_client_type                  user_handle,
                           unsigned long                    msg_id,
                           void                           * resp_c_struct,
                           int                              resp_c_struct_len,
                           void                           * resp_cb_data,
                           qmi_client_error_type            transp_err)
{
    HAL_TRACE_DEBUG0 ("softUiccResetCallback()");

    uim_recovery_resp_msg_v01 *qmi_response = resp_c_struct;
    if ((qmi_response) && (qmi_response->resp.result == QMI_RESULT_FAILURE_V01))
    {
        HAL_TRACE_ERROR1 ("softUiccResetCallback: UICC_RESET: **ERROR** UICC soft reset failed with err: 0x%X!", qmi_response->resp.error);
    }
    else
    {
        HAL_TRACE_DEBUG1 ("softUiccResetCallback: UICC_RESET: Request for soft reset of UICC successful. qmi_response %p", qmi_response);
    }

    if (qmi_response)
    {
        free(qmi_response);
        qmi_response = NULL;
    }

    if (resp_cb_data)
    {
        free(resp_cb_data);
        resp_cb_data = NULL;
    }
}


/*******************************************************************************
**
** Function         softUiccResetFireAndForget
**
** Description      Attempts to soft reset the UICC.
**
** Returns          none.
**
*******************************************************************************/
void softUiccResetFireAndForget()
{
    HAL_TRACE_DEBUG0 ("softUiccResetFireAndForget()");

    int                                   qmi_err_code = QMI_NO_ERR;
    qmi_client_type                       qmi_uim_svc_client = NULL;
    qmi_client_qmux_instance_type         qmi_modem_port;

    uim_recovery_req_msg_v01              qmi_request;
    uim_recovery_resp_msg_v01           * qmi_response = NULL;
    qmi_txn_handle                        txn_handle;

    /* ro.baseband is a property that is set during power up, you have to read it & get the string,
     * then the string tells us what QMI port enum to use for that particular device
     * we then pass that QMI port enum to qmi_client_init_instance.
     */
    unsigned long num = 0;
    char prop_value [PROPERTY_VALUE_MAX] = {0};
    int len = property_get ("ro.baseband", prop_value, "");
    if (len > 0)
    {
        HAL_TRACE_DEBUG1 ("softUiccResetFireAndForget(): UICC_RESET: Attempting to find modem port for property value '%s'", prop_value);
        qmi_modem_port = find_modem_port(prop_value);
        HAL_TRACE_DEBUG1 ("softUiccResetFireAndForget(): UICC_RESET: Obtained port %d", (int) qmi_modem_port);
    }
    else
    {
        HAL_TRACE_ERROR0 ("softUiccResetFireAndForget: UICC_RESET: **ERROR** unable to obtain baseband port.");
        return;
    }


    /* Init retry related defines */
#define INIT_MAX_RETRIES            10
#define INIT_RETRY_INTERVAL          1
    int num_retries = 0;
    do
    {
        if (num_retries > 0)
        {
            sleep(INIT_RETRY_INTERVAL);
        }

        HAL_TRACE_DEBUG1("softUiccResetFireAndForget: UICC_RESET: Trying qcril_qmi_uim_srvc_init_client() try # %d", num_retries);
        qmi_uim_svc_client = uicc_init_client(qmi_modem_port,
                                              &qmi_err_code);
        num_retries++;
    } while (   (qmi_uim_svc_client == NULL)
             && (qmi_err_code != QMI_NO_ERR)
             && (num_retries < INIT_MAX_RETRIES));

    if ((qmi_uim_svc_client == NULL) || (qmi_err_code != QMI_NO_ERR))
    {
        HAL_TRACE_ERROR1("softUiccResetFireAndForget: UICC_RESET: **ERROR** Could not register successfully with QMI UIM Service. Error: %d", qmi_err_code);
        return;
    }

    qmi_response = (uim_recovery_resp_msg_v01*) malloc (sizeof (uim_recovery_resp_msg_v01));
    if(qmi_response == NULL)
    {
        HAL_TRACE_ERROR0 ("softUiccResetFireAndForget: UICC_RESET: **ERROR** Cannot malloc response buffer!");
        return;
    }

    memset(&qmi_request, 0x00, sizeof(qmi_request));
    memset(qmi_response, 0x00, sizeof(uim_recovery_resp_msg_v01));

    qmi_request.slot = UIM_SLOT_1_V01;

    qmi_err_code = qmi_client_send_msg_async( qmi_uim_svc_client,
                                              QMI_UIM_RECOVERY_RESP_V01,
                                              (void*) &qmi_request,
                                              sizeof(qmi_request),
                                              (void*) qmi_response,
                                              sizeof(uim_recovery_resp_msg_v01),
                                              (qmi_client_recv_msg_async_cb) softUiccResetCallback, /* callback */
                                              NULL, /* callback data */
                                              &txn_handle);

    if (qmi_err_code != QMI_NO_ERR)
    {
        HAL_TRACE_ERROR1 ("softUiccResetFireAndForget: UICC_RESET: **ERROR** Cannot soft reset UICC (RC 0x%X)", qmi_err_code);
        if (qmi_response != NULL)
        {
            free(qmi_response);
            qmi_response = NULL;
        }
        return;
    }
}

/*******************************************************************************
**
** Function         recreate_hci_persistence_file
**
** Description      Deletes then recreates the HCI persistence file.
**
** Returns          none.
**
*******************************************************************************/
void recreate_hci_persistence_file()
{
    HAL_TRACE_DEBUG0 ("recreate_hci_persistence_file()");

    /* recreate the file */
    nfc_hal_nci_initiate_nfcc_storage_check(Nfcc_Storage_NfccResetInProgress);
    HAL_TRACE_DEBUG0 ("recreate_hci_persistence_file: Removing persistence file.");
    remove(kNfcc_storage_filename);
    nfc_hal_nci_initiate_nfcc_storage_check(gNfcc_Storage_CheckParam);
}


/*******************************************************************************
**
** Function         initNfcc
**
** Description      Move through the NFCC initialisation steps
**
** Params           p_msg; the incoming packet
**
** Precondition     nfc_hal_cb.dev_cb.initializing_state != NFC_HAL_INIT_STATE_NFC_HAL_INIT
**
** Returns          False is the calling routine should not continue processing the msg,
**                  True if it should.
*******************************************************************************/
BOOLEAN initNfcc(NFC_HDR *p_msg)
{
    HAL_TRACE_DEBUG1 ("initNfcc: state %d", nfc_hal_cb.dev_cb.initializing_state);

    UINT8 mt, pbf, gid, op_code;
    UINT8 *p1;

    p1 = (UINT8 *) (p_msg + 1) + p_msg->offset;
    NCI_MSG_PRS_HDR0 (p1, mt, pbf, gid);
    NCI_MSG_PRS_HDR1 (p1, op_code);

    HAL_TRACE_DEBUG1 ("initNfcc: wait_reset_rsp %d", wait_reset_rsp);
    if(wait_reset_rsp)
    {
        if((gid == NCI_GID_CORE) && (op_code == NCI_MSG_CORE_CONN_CREDITS) && (mt == NCI_MT_NTF))
        {
            HAL_TRACE_DEBUG0 ("core_con_credit ntf ignored...");
            return FALSE;
        }
        if((gid == NCI_GID_CORE) && (op_code == NCI_MSG_CORE_RESET) && (mt == NCI_MT_RSP))
        {
            HAL_TRACE_DEBUG0 (" core reset rsp recieved...");
            wait_reset_rsp = FALSE;
        }
    }

    if ((mt == NCI_MT_RSP) && (gid == NCI_GID_PROP) && (op_code == NCI_MSG_PROP_SLEEP))
    {
        if (nfc_hal_cb.dev_cb.sleep.state.W4_SLEEP_RSP == 1)
        {
            HAL_TRACE_DEBUG2 ("%s:%d: Received SLEEP RSP", __FUNCTION__, __LINE__);
            nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;
            nfc_hal_main_stop_quick_timer (&nfc_hal_cb.ncit_cb.nci_wait_rsp_timer);
            nfc_hal_cb.dev_cb.sleep.state.W4_SLEEP_RSP = 0;
            nfc_hal_cb.dev_cb.sleep.state.CMD_SLEEP = 1;
            HAL_TRACE_DEBUG3("%s:%d sleep flags: %x", __FUNCTION__, __LINE__, nfc_hal_cb.dev_cb.sleep.raw);
            nfc_hal_dm_send_pend_cmd();
            return FALSE;
        }
        else if (nfc_hal_cb.dev_cb.sleep.state.CMD_SLEEP_JNI == 1)
        {
            nfc_hal_cb.dev_cb.sleep.state.CMD_SLEEP = 1;
            nfc_hal_cb.dev_cb.sleep.state.CMD_SLEEP_JNI = 0;
            HAL_TRACE_DEBUG3("%s:%d sleep flags: %x", __FUNCTION__, __LINE__, nfc_hal_cb.dev_cb.sleep.raw);
        }
    }
    /// QNCI_FEATURE_NFCC_SLEEP
    /*
     * Initializing NFCC and not in ramdump mode.
     * If in ramdump mode then the NFCC probably crashed during initialisation so we need to handle it here.
     * The NFCC should be in ramdump state so no other messages should come through until we power cycle it.
     */
    if (   (nfc_hal_cb.dev_cb.initializing_state != NFC_HAL_INIT_STATE_IDLE)
        && (nfc_hal_cb.dev_cb.initializing_state != NFC_HAL_INIT_STATE_RAMDUMP)
        && (nfc_hal_cb.dev_cb.initializing_state != NFC_HAL_INIT_STATE_CLOSING)
        && ((current_mode != FTM_MODE) || (/*(current_mode == FTM_MODE) &&*/ (nfc_hal_cb.dev_cb.initializing_state != NFC_HAL_INIT_STATE_W4_POST_INIT_DONE))) )
    {
        HAL_TRACE_DEBUG0 ("initNfcc: Initializing NFCC");
        nfc_hal_dm_proc_msg_during_init (p_msg);
        /* do not send message to NFC task while initializing NFCC */
        return  FALSE;
    }
    return TRUE;
}


/*******************************************************************************
**
** Function         nfc_hal_handle_initialisation_events
**
** Description      Invokes and handles Region2 Control, NVM and HCI messages.
**
**
*******************************************************************************/
void nfc_hal_handle_initialisation_events(NFC_HDR *p_msg)
{
    UINT8 *p = NULL, *pp = NULL;
    UINT8 mt = 0, pbf = 0, gid = 0, op_code = 0;
    UINT8 payload_len = 0;
    UINT8 reason = 0;

    static enum {
        HalInitRegion2 = 0,
        HalInitNvm,
        HalInitHci,
        HalInitComplete,
        HalInitDone
    } hal_init_state = HalInitRegion2;

    HAL_TRACE_DEBUG1 ("nfc_hal_handle_initialisation_events: %d", nfc_hal_cb.dev_cb.initializing_state);

    if (p_msg)
    {
        p = (UINT8 *) (p_msg + 1) + p_msg->offset;
        pp = p;
        NCI_MSG_PRS_HDR0 (p, mt, pbf, gid);
        NCI_MSG_PRS_HDR1 (p, op_code);
        payload_len = *p++;
        reason = *p;
    }
    else
    {
        /* NULL resets the state */
        HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: Resetting Hal initialisation states.");
        hal_init_state = HalInitRegion2;
        mt = NCI_MT_CMD;
    }

    switch (hal_init_state)
    {
        case HalInitRegion2:
        {
            /* Invoke the sequence? */
            if (mt == NCI_MT_CMD)
            {
                UINT32 region2_enable = 0;

                /* Send Region2 */
                if (GetNumValue("REGION2_ENABLE", &region2_enable, sizeof(region2_enable)))
                {
                    if(region2_enable)
                    {
                        /* Clear the cmd window before sending Region2.
                           We made sure this is the RSP we are waiting for before updating
                           the command window */
                        nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;

                        /* Send NciRegionControlEnable command to tell FW to give Region2 control to MW.*/
                        HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: Sending NciRegionControlEnable command.");
                        nfc_hal_dm_send_prop_nci_region2_control_enable_cmd(REGION2_CONTROL_ENABLE);
                    }
                }
                else
                {
                    HAL_TRACE_WARNING0 ("nfc_hal_handle_initialisation_events: REGION2_ENABLE not specified in config file, assuming disabled.");
                    hal_init_state = HalInitNvm;
                }

                if (hal_init_state == HalInitRegion2)
                {
                    break;
                }
                /* FALLTHRU to handle NVM */
            }
            else if ((mt == NCI_MT_RSP) && (gid == NCI_GID_PROP) && (op_code == NCI_OID_GENERIC))
            {
                HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: REGION2_ENABLE RSP received, moving onto NVM.");

                /* Indicate that we have received a RSP to our CMD. This allows us to send further CMDs. */
                nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;
                nfc_hal_main_stop_quick_timer (&nfc_hal_cb.ncit_cb.nci_wait_rsp_timer);

                mt = NCI_MT_CMD;
                hal_init_state = HalInitNvm;
                /* FALLTHRU to handle NVM as we only expect one RSP */
            }
            /* FALLTHRU */
        }
        case HalInitNvm:
        {
            /* For unfused NCI_MSG_PROP_MEMACCESS op_code is used, for fused NCI_MSG_PROP_GENERIC is used. */
            if (    (mt == NCI_MT_CMD)
                || ((mt == NCI_MT_RSP) && (gid == NCI_GID_PROP) && ((op_code == NCI_OID_GENERIC) || (op_code == NCI_MSG_PROP_MEMACCESS))))
            {
                UINT8 nvmupdatebuff[260]={0},nvmdatabufflen=0;
                UINT8 *nvmcmd = NULL, nvmcmdlen = 0;
                UINT32 nvm_update_flag = 0;

                GetNumValue("NVM_UPDATE_ENABLE_FLAG", &nvm_update_flag, sizeof(nvm_update_flag));

                /* Send NVM.
                   Clear the cmd window before sending other cmds.
                   We made sure this is the RSP we are waiting for before updating
                   the command window.
                   However, this is done within the NVM and HCI code.*/
                if (nvm_update_flag)
                {
                    if (nfc_hal_cb.nvm.last_cmd_sent == FALSE)
                    {
                        if (mt == NCI_MT_CMD)
                        {
                            HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: Initial checking for any NVMs.");
                        }
                        else
                        {
                            HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: Further checking for any NVMs.");
                            nfc_hal_main_stop_quick_timer (&nfc_hal_cb.ncit_cb.nci_wait_rsp_timer);
                        }

                        /* If there are NVM updates to tune the NFCC, we should apply those now!
                           Frame the cmd */
                        HAL_TRACE_DEBUG1 ("nfc_hal_handle_initialisation_events: Sending NVM updates. (nvm_updated %d)", nfc_hal_cb.nvm.nvm_updated);
                        int ret = nfc_hal_nci_send_nvm_updates(nfc_hal_cb.dev_cb.efuse_value, &nvmupdatebuff[0], &nvmdatabufflen, nvmcmd, &nvmcmdlen);

                        if ((ret > 0) || (nfc_hal_cb.nvm.last_cmd_sent == TRUE))
                        {
                            /* NVM sent, exit to wait for RSP */
                        }
                        else if (ret < 0)
                        {
                            /* An error has occurred */
                            HAL_TRACE_ERROR1 ("nfc_hal_handle_initialisation_events: ERROR: Whilst trying to send NVMs: %d, moving onto HCI.", ret);
                            hal_init_state = HalInitHci;
                            mt = NCI_MT_CMD;
                        }
                    }
                    else
                    {
                        HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: Last NVM RSP received, moving onto HCI.");
                        nfc_hal_main_stop_quick_timer (&nfc_hal_cb.ncit_cb.nci_wait_rsp_timer);
                        hal_init_state = HalInitHci;
                        mt = NCI_MT_CMD;
                    }

                    if (hal_init_state == HalInitNvm)
                    {
                        break;
                    }
                    /* FALLTHRU to HCI processing*/
                }
                else
                {
                    HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: No NVM updates to process, moving onto HCI.");
                    hal_init_state = HalInitHci;
                    mt = NCI_MT_CMD;
                }
            }
            else
            {
                HAL_TRACE_ERROR1 ("nfc_hal_handle_initialisation_events: **ERROR** Invalid msg: 0x%X, during HAL initialisation of NVM. Continuing...",
                                  CONSTRUCT_MSG_TYPE_SHORT(mt, gid, op_code));
                break;
            }
            /* FALLTHRU */
        }
        case HalInitHci:
        {
            if (     (mt == NCI_MT_CMD)
                || (((mt == NCI_MT_NTF) || (mt == NCI_MT_RSP)) && (gid == NCI_GID_PROP) && (op_code == NCI_OID_NFCC_STORE)))
            {
                /* Received a NTF from the NFCC. */
                if (mt == NCI_MT_NTF)
                {
                    /* This is a spurious NTF from the NFCC for HCI that gets sent during HAL init state.
                       We need to ensure not to configure the NFCC until after we receive the last expected RSP.
                       This is sent due to the implementation of the FW. Ideally we shouldn't see it but
                       currently we are so ignore (but still persist).*/
                    HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: NFCC_STORAGE: Received intial spurious NTF. Ignoring.");
                    nfc_hal_nci_recv_nfcc_storage_ntf(payload_len, p);

                    /* FALLTHRU and return */
                }
                else
                {
                    int ret = Nfcc_Storage_ProcessingOK;
                    if (mt == NCI_MT_CMD)
                    {
                        /* Obtain a user specified value if any. Keep default if invalid */
                        UINT32 temp;
                        if (GetNumValue("NFCC_STORAGE_SEND_MODE", &temp, sizeof(temp)))
                        {
                             /* Make everything unsigned to avoid compiler warnings and simplify comparison */
                             if (temp <= ((UINT32) Nfcc_Storage_OneEntryPerPacket))
                             {
                                 gNfcc_Storage_CheckParam = (tNfcc_Check_Params) temp;
                             }
                             else
                             {
                                 HAL_TRACE_WARNING2 ("nfc_hal_handle_initialisation_events: NFCC_STORAGE: **WARNING**: "
                                                     "Config file has invalid setting (0x%X) for NFCC_STORAGE_SEND_MODE. Using default value of 0x%X",
                                                     temp, gNfcc_Storage_CheckParam);
                             }
                        }
                        /* else use default value */

                        HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: NFCC_STORAGE: Checking for NFCC STORAGE updates.");
                        ret = nfc_hal_nci_initiate_nfcc_storage_check(gNfcc_Storage_CheckParam);
                    }
                    else /* RSP */
                    {
                        HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: NFCC_STORAGE: RSP received.");
                        nfc_hal_main_stop_quick_timer (&nfc_hal_cb.ncit_cb.nci_wait_rsp_timer);
                        ret = nfc_hal_nci_recv_nfcc_storage_rsp(reason, gNfcc_Storage_CheckParam);
                    }

                    if (ret == Nfcc_Storage_AllRecordsProcessed)
                    {
                        HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: NFCC_STORAGE: No further persistent entries to process. Waiting for last RSP.");
                        nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;
                        /* FALLTHRU to wait for last RSP */
                    }
                    else if (ret == Nfcc_Storage_NoPersistentFile)
                    {
                        HAL_TRACE_ERROR0 ("nfc_hal_handle_initialisation_events: NFCC_STORAGE: **ERROR** Persistant file unavailable any longer!!! But continuing...");

                        /* Let system come up.*/
                        nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;
                        hal_init_state = HalInitComplete;
                        /* FALLTHRU to post completion event */
                    }
                    else if (ret == Nfcc_Storage_NoUnprocessedEntriesFound)
                    {
                        HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: NFCC_STORAGE: No persistent entries in file to process.");

                        /* Let system come up.*/
                        nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;
                        hal_init_state = HalInitComplete;
                        /* FALLTHRU to post completion event */
                    }
                }

                if (hal_init_state == HalInitHci)
                {
                    break;
                }
            }
            else
            {
                HAL_TRACE_ERROR1 ("nfc_hal_handle_initialisation_events: **ERROR** Invalid msg: 0x%X, during HAL initialisation of HCI. Continuing...",
                                  CONSTRUCT_MSG_TYPE_SHORT(mt, gid, op_code));
                break;
            }
            /* FALLTHRU */
        }
        case HalInitComplete:
        {
            /* To come into here we should receive an NVM or HCI RSP (depending on what's enabled) */
            HAL_TRACE_DEBUG0 ("nfc_hal_handle_initialisation_events: Hal initialisation complete, continuing to bring up dm.");

            /* After everything has finished */
            NFC_HDR *p_msg;
            UINT16  size = NCI_MSG_HDR_SIZE;
            p_msg = (NFC_HDR *)GKI_getpoolbuf (NFC_HAL_NCI_POOL_ID);
            /* Send message to NFC_HAL_TASK */
            if (p_msg != NULL)
            {
                p_msg->event  = NFC_HAL_EVT_TO_CONFIG_DM;
                p_msg->offset = 0;
                p_msg->len    = size;
                p_msg->layer_specific = 0;

                GKI_send_msg (NFC_HAL_TASK, NFC_HAL_TASK_MBOX, p_msg);
            }

            /* Ensure we do not repeat HalInitComplete scenario if a spurious RSP/NTF arrives whilst in HAL initialisation state */
            hal_init_state = HalInitDone;
            break;
        }
        case HalInitDone:
            HAL_TRACE_ERROR1 ("nfc_hal_handle_initialisation_events: **ERROR** RSP/NTF arrived after HAL initialisation completed: 0x%X. Continuing...",
                              CONSTRUCT_MSG_TYPE_SHORT(mt, gid, op_code));
            break;
        default:
        {
            HAL_TRACE_ERROR0 ("nfc_hal_handle_initialisation_events: **ERROR** Invalid state during Hal initialisation. Continuing...");
        }
    }
}
