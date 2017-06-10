/******************************************************************************
  @file    qcril_qmi_sms.h
  @brief   qcril qmi - SMS

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI SMS.

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#ifndef QCRIL_QMI_SMS_H
#define QCRIL_QMI_SMS_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <pthread.h>
#include "comdef.h"
#include "ril.h"
#include "ril_cdma_sms.h"
#include "qcrili.h"
#include "qcril_log.h"
#include "qmi_idl_lib.h"
#include "qmi_client.h"
#include "wireless_messaging_service_v01.h"

/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/// sms_pre_init to be called at bootup, but not on SSR
void qcril_qmi_sms_pre_init(void);

/// sms_destroy, cleanup resources allocated at bootup (qcril_qmi_sms_pre_init)
void qcril_qmi_sms_destroy(void);

qmi_client_error_type qcril_qmi_sms_init(void);

void qcril_qmi_sms_unsol_ind_cb
(
    qmi_client_type                user_handle,
    unsigned int                   msg_id,
    void                          *ind_buf,
    unsigned int                   ind_buf_len,
    void                          *ind_cb_data
);

void qcril_qmi_sms_command_cb
(
    qmi_client_type             user_handle,
    unsigned int                msg_id,
    void                       *resp_c_struct,
    unsigned int                resp_c_struct_len,
    void                       *resp_cb_data,
    qmi_client_error_type       transp_err
);


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/


#define QCRIL_SMS_BC_MM_TABLE_SIZE  50

/* Size of the buffer to hold a received GW SMS message or Status Report.
   Multiply the size of the QMI payload by two since it will be translated to ASCII hex format. */
#define QCRIL_SMS_BUF_MAX_SIZE  (WMS_MESSAGE_LENGTH_MAX_V01 * 2)

/* Maximum number of special chars that can be added to the SMSC address (quotes, comma, plus) */
#define QCRIL_SMS_MAX_SMSC_SPECIAL_CHARS              4
/* Max size of SMSC Address is sizeof(address_type) + maximum number of address digits +
   special characters */
#define QCRIL_SMS_MAX_SMSC_ADDRESS_SIZE (WMS_ADDRESS_TYPE_MAX_V01 + \
                                         WMS_ADDRESS_DIGIT_MAX_V01 + \
                                         QCRIL_SMS_MAX_SMSC_SPECIAL_CHARS)

typedef struct
{
  /* GW Ack Info */
  boolean gw_ack_pending;                       /* Indicates whether QCRIL is waiting for Android
                                                   to ack an MT SMS message */
  boolean gw_ack_needed;                        /* Indicates whether QCRIL will drop the Android ack
                                                   (QCRIL will drop the ack if the modem indicates
                                                   that no ack is needed) */
  uint32 gw_transaction_id;                     /* Transaction id for the GW SMS message to be acked */
  boolean gw_send_ack_on_ims;                   /* Indicates whether the ack must be sent on IMS */

  /* CDMA Ack Info */
  boolean cdma_ack_pending;                     /* Indicates whether QCRIL is waiting for Android
                                                   to ack an MT SMS message */
  boolean cdma_ack_needed;                      /* Indicates whether QCRIL will drop the Android ack
                                                   (QCRIL will drop the ack if the modem indicates
                                                   that no ack is needed) */
  uint32 cdma_transaction_id;                   /* Transaction id for the CDMA SMS message to be acked */
  boolean cdma_send_ack_on_ims;                 /* Indicates whether the ack must be sent on IMS */
} qcril_sms_ack_info_type;

typedef struct
{
  uint8_t block_sms_on_1x;
  uint8_t registered_ind_valid;
  uint8_t registered_ind;
}qcril_sms_transport_layer_info;

typedef struct
{
  qcril_sms_ack_info_type     sms_ack_info; /* Ack info */
  pthread_mutex_t sms_ack_info_mutex;       /* Mutex to protect access to SMS ACK info */
  uint32                      mt_pending_ack_expry_tmr;
  qcril_sms_transport_layer_info     transport_layer_info; /* Transport layer info */
  pthread_mutex_t transport_layer_info_mutex;       /* Mutex to protect access to Transport layer info */

} qcril_sms_struct_type;

/*===========================================================================

                                UTILITY DEFINITIONS AND TYPES

===========================================================================*/

enum { QCRIL_SMS_TL_MAX_LEN        = 246 };

typedef enum
{
  QCRIL_SMS_TL_TYPE_POINT_TO_POINT    = 0,
  QCRIL_SMS_TL_TYPE_BROADCAST         = 1,
  QCRIL_SMS_TL_TYPE_ACK               = 2,
  QCRIL_SMS_TL_TYPE_MAX               = 2
} qcril_sms_tl_message_type_e_type;


/* ------------------------ */
/* ---- Teleservice Id ---- */
/* ------------------------ */
typedef enum
{
  QCRIL_SMS_TELESERVICE_IS91_PAGE          = 0x00010000,
  QCRIL_SMS_TELESERVICE_IS91_VOICE_MAIL    = 0x00020000,
  QCRIL_SMS_TELESERVICE_IS91_SHORT_MESSAGE = 0x00030000,
  QCRIL_SMS_TELESERVICE_MWI                = 0x00040000,
  QCRIL_SMS_TELESERVICE_BROADCAST          = 0x00050000,
  QCRIL_SMS_TELESERVICE_UNKNOWN            = 0x0FFFFFFF
} qcril_sms_teleservice_e_type;

/* Transport Layer parameter mask values:
*/
enum{ QCRIL_SMS_MASK_TL_NULL                = 0x00000000 };
enum{ QCRIL_SMS_MASK_TL_TELESERVICE_ID      = 0x00000001 };
enum{ QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY     = 0x00000002 };
enum{ QCRIL_SMS_MASK_TL_ADDRESS             = 0x00000004 };
enum{ QCRIL_SMS_MASK_TL_SUBADDRESS          = 0x00000008 };
enum{ QCRIL_SMS_MASK_TL_BEARER_REPLY_OPTION = 0x00000040 };
enum{ QCRIL_SMS_MASK_TL_CAUSE_CODES         = 0x00000080 };
enum{ QCRIL_SMS_MASK_TL_BEARER_DATA         = 0x00000100 };

/* Transport Layer parameter Ids:
*/
typedef enum
{
  QCRIL_SMS_TL_DUMMY          = -1,  /* dummy */
  QCRIL_SMS_TL_TELESERVICE_ID = 0,  /* Teleservice Identifier     */
  QCRIL_SMS_TL_BC_SRV_CATEGORY,     /* Broadcast Service Category */
  QCRIL_SMS_TL_ORIG_ADDRESS,        /* Originating Address        */
  QCRIL_SMS_TL_ORIG_SUBADDRESS,     /* Originating Subaddress     */
  QCRIL_SMS_TL_DEST_ADDRESS,        /* Destination Address        */
  QCRIL_SMS_TL_DEST_SUBADDRESS,     /* Destination Subaddress     */
  QCRIL_SMS_TL_BEARER_REPLY_OPTION, /* Bearer Reply Option        */
  QCRIL_SMS_TL_CAUSE_CODES,         /* Cause Codes                */
  QCRIL_SMS_TL_BEARER_DATA          /* Bearer Data                */

} qcril_sms_tl_parm_id_e_type;

#define TL_HEADER_SIZE    1
#define TL_PARM_SIZE      2

#define MSG_DUP_PARM QCRIL_LOG_ERROR("Duplicate parm: %d", parm_id, 0,0)

typedef enum
{
    QMI_RIL_SMS_SVC_NOT_INITIALZIED = 0,
    QMI_RIL_SMS_SVC_INIT_PENDING,
    QMI_RIL_SMS_SVC_FULLY_OPERATIONAL
} qmi_ril_sms_svc_status_type;

typedef struct
{
  boolean                      is_mo;
  qcril_sms_tl_message_type_e_type   tl_message_type;
  uint16                       mask;
  uint32                       teleservice;
  RIL_CDMA_SMS_Address         address;
  RIL_CDMA_SMS_Subaddress      subaddress;
  uint8                        bearer_reply_seq_num;
  uint32                       service;
} qcril_sms_tl_message_type;

typedef struct
{
  uint16                      data_len;
  uint8                       data[ RIL_CDMA_SMS_BEARER_DATA_MAX ];
} qcril_sms_OTA_message_type;


/*===========================================================================

                                UTILITY FUNCTIONS

===========================================================================*/

boolean qcril_sms_convert_sms_ril_to_qmi
(
  RIL_CDMA_SMS_Message        *cdma_sms_msg,
  uint8                       *data_buf,
  uint16                       data_buf_len,
  uint16                      *ota_data_len,
  boolean                     sms_on_ims,
  boolean                     is_mo_sms
);

boolean qcril_sms_convert_mt_sms_qmi_to_ril
(
  const uint8                 *data,
  uint16                      length,
  RIL_CDMA_SMS_Message        *cdma_sms_msg
);

void qcril_sms_hex_to_byte(const char * hex_pdu,   // INPUT
                           byte * byte_pdu,        // OUTPUT
                           uint32 num_hex_chars);

void qcril_sms_byte_to_hex(byte * byte_pdu,   // INPUT
                           char * hex_pdu,    // OUTPUT
                           uint32 num_bytes);

boolean qcril_sms_convert_smsc_address_to_qmi_format(const char * input_smsc_address,
                                                     wms_set_smsc_address_req_msg_v01 * qmi_request);

boolean qcril_sms_convert_smsc_address_to_ril_format(const wms_get_smsc_address_resp_msg_v01 * qmi_response,
                                                     char * output_smsc_address_ptr);

wms_message_tag_type_enum_v01 qcril_sms_map_ril_tag_to_qmi_tag(int ril_tag);
boolean qcril_sms_is_tag_mo(int ril_tag);

const char *qcril_sms_lookup_cmd_name(unsigned long qmi_cmd);

const char *qcril_sms_lookup_ind_name(unsigned long qmi_ind);

qmi_client_error_type qcril_sms_perform_initial_configuration( void );

void qmi_ril_set_sms_svc_status(qmi_ril_sms_svc_status_type new_status);
qmi_ril_sms_svc_status_type qmi_ril_get_sms_svc_status(void);

void qcril_sms_post_ready_status_update(void);

void qcril_sms_clearing_cdma_ack();

#ifdef QMI_RIL_UTF
void b_packw(word src, byte dst[], word pos, word len);

void b_packb(byte src, byte dst[], word pos, word len);

void qcril_sms_cdma_encode_address
(
  const RIL_CDMA_SMS_Address  * address_ptr,
  uint8                       * parm_len_ptr,
  uint8                       * data
);

void qcril_sms_cdma_encode_subaddress
(
  const RIL_CDMA_SMS_Subaddress  * address_ptr,
  uint8                          * parm_len_ptr,
  uint8                          * data
);

boolean qcril_sms_convert_tl_to_qmi
(
  qcril_sms_tl_message_type        * tl_msg_ptr,   /* IN */
  uint32                             raw_bd_len,   /* IN */
  uint8                            * raw_bd_ptr,   /* IN */
  qcril_sms_OTA_message_type       * OTA_msg_ptr   /* OUT */
);

void qcril_sms_convert_ril_to_tl
(
  RIL_CDMA_SMS_Message *cdma_sms_msg,
  qcril_sms_tl_message_type * tl_ptr,
  boolean sms_on_ims,
  boolean is_mo_sms
);
#endif
void qcril_qmi_sms_unsolicited_indication_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_sms_report_unsol_ims_state_change();
#endif /* QCRIL_QMI_SMS_H */
