#ifndef WMSTS_H
#define WMSTS_H

/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*


                    W I R E L E S S   M E S S A G I N G   S E R V I C E S
                   -- Translation Services

GENERAL DESCRIPTION
  The WMS module which implements the User API for SMS. This source file
  defines the internal translation functions.

Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007 by Qualcomm Technologies, Inc.
  All Rights Reserved.

Export of this technology or software is regulated by the U.S. Government.
Diversion contrary to U.S. law prohibited.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/


/* ^L<EJECT> */
/*===========================================================================
$Header:

===========================================================================*/


/*===========================================================================

                         DATA TYPE DECLARATIONS

===========================================================================*/

enum { WMS_TL_MAX_LEN        = 246 };

/* Transport Layer parameter mask values:
*/
enum{ WMS_MASK_TL_NULL                = 0x00000000 };
enum{ WMS_MASK_TL_TELESERVICE_ID      = 0x00000001 };
enum{ WMS_MASK_TL_BC_SRV_CATEGORY     = 0x00000002 };
enum{ WMS_MASK_TL_ADDRESS             = 0x00000004 };
enum{ WMS_MASK_TL_SUBADDRESS          = 0x00000008 };
enum{ WMS_MASK_TL_BEARER_REPLY_OPTION = 0x00000040 };
enum{ WMS_MASK_TL_CAUSE_CODES         = 0x00000080 };
enum{ WMS_MASK_TL_BEARER_DATA         = 0x00000100 };

/* Transport Layer parameter Ids:
*/
typedef enum
{
  WMS_TL_DUMMY          = -1,  /* dummy */
  WMS_TL_TELESERVICE_ID = 0,  /* Teleservice Identifier     */
  WMS_TL_BC_SRV_CATEGORY,     /* Broadcast Service Category */
  WMS_TL_ORIG_ADDRESS,        /* Originating Address        */
  WMS_TL_ORIG_SUBADDRESS,     /* Originating Subaddress     */
  WMS_TL_DEST_ADDRESS,        /* Destination Address        */
  WMS_TL_DEST_SUBADDRESS,     /* Destination Subaddress     */
  WMS_TL_BEARER_REPLY_OPTION, /* Bearer Reply Option        */
  WMS_TL_CAUSE_CODES,         /* Cause Codes                */
  WMS_TL_BEARER_DATA          /* Bearer Data                */

} wms_tl_parm_id_e_type;

typedef enum
{
  WMS_TL_TYPE_MIN               = 0,

  WMS_TL_TYPE_POINT_TO_POINT    = 0,
  WMS_TL_TYPE_BROADCAST         = 1,
  WMS_TL_TYPE_ACK               = 2,

  WMS_TL_TYPE_MAX               = 2

} wms_tl_message_type_e_type;

/* Cause code
*/
typedef struct
{
  uint8                     reply_seq_num;
  wms_error_class_e_type    error_class;

  /* If error_class is NoError, status should be ignored.
     Only the first half of the enums are used in this structure.
  */
  wms_cdma_tl_status_e_type tl_status;

} wms_cause_code_type;

/* TL fields without bearer data
*/
typedef struct
{
  boolean                      is_mo;

  wms_tl_message_type_e_type   tl_message_type;

  /* the mask indicates which fields are present in this message */
  uint16                       mask;

  wms_teleservice_e_type       teleservice;
  wms_address_s_type           address;
  wms_subaddress_s_type        subaddress;
  uint8                        bearer_reply_seq_num; /* 0..63 */
  wms_service_e_type           service;
  wms_cause_code_type          cause_code;

  /* decoded bearer data */
  wms_client_bd_s_type         cl_bd;

} wms_tl_message_type;

/* Over-the-air (raw) message structure
*/
typedef PACKED struct wms_OTA_message_struct
{
  wms_format_e_type           format;
  uint16                      data_len;
  uint8                       data[ WMS_MAX_LEN ];
} wms_OTA_message_type;
/* GW template masks as in TS 31.102 (SMS parameters)
*/
enum
{
  WMS_GW_TEMPLATE_MASK_DEST_ADDR = 0x01,
  WMS_GW_TEMPLATE_MASK_SC_ADDR   = 0x02,
  WMS_GW_TEMPLATE_MASK_PID       = 0x04,
  WMS_GW_TEMPLATE_MASK_DCS       = 0x08,
  WMS_GW_TEMPLATE_MASK_VALIDITY  = 0x10
};

enum { WMS_SMS_UDL_MAX_7_BIT  = 160 }; /* as in the spec */
enum { WMS_SMS_UDL_MAX_8_BIT  = 140 }; /* as in the spec */

enum { WMS_GW_CB_PAGE_HEADER_LEN = 6 };
enum { WMS_GW_CB_MAX_PAGE_USER_DATA_LEN = 93 };
enum { WMS_GW_CB_PAGE_SIZE    = 88 };

/* Bearer Data subparameter Ids:
*/
typedef enum
{
  WMS_BD_DUMMY  = -1,   /* dummy */
  WMS_BD_MSG_ID = 0,    /* Message Identifier                */
  WMS_BD_USER_DATA,     /* User Data                         */
  WMS_BD_USER_RESP,     /* User Response Code                */
  WMS_BD_MC_TIME,       /* Message Center Time Stamp         */
  WMS_BD_VALID_ABS,     /* Validity Period - Absolute        */
  WMS_BD_VALID_REL,     /* Validity Period - Relative        */
  WMS_BD_DEFER_ABS,     /* Deferred Delivery Time - Absolute */
  WMS_BD_DEFER_REL,     /* Deferred Delivery Time - Relative */
  WMS_BD_PRIORITY,      /* Priority Indicator                */
  WMS_BD_PRIVACY,       /* Privacy Indicator                 */
  WMS_BD_REPLY_OPTION,  /* Reply Option                      */
  WMS_BD_NUM_OF_MSGS,   /* Number of Messages                */
  WMS_BD_ALERT,         /* Alert on Message Delivery         */
  WMS_BD_LANGUAGE,      /* Language Indicator                */
  WMS_BD_CALLBACK,      /* Call Back Number                  */
  WMS_BD_DISPLAY_MODE,  /* Display Mode */
  WMS_BD_MULTIPLE_USER_DATA, /* Multiple Encoding User Data  */
  WMS_BD_DEPOSIT_INDEX, /* Message Deposit Index             */
  WMS_BD_SCPT_DATA,     /* Service Category Program Data       */
  WMS_BD_SCPT_RESULT,   /* Service Category Program Result     */
  WMS_BD_DELIVERY_STATUS /* Message Status */

  , WMS_BD_IP_ADDRESS     = 85   /* IP Address               */
  , WMS_BD_RSN_NO_NOTIFY  = 86   /* Reason for Not Notified  */
  , WMS_BD_ESN            = 87   /* ESN/UIM-ID */

} wms_bd_sub_parm_id_e_type;

enum { WMS_GW_ADDRESS_MAX         = 20  };
enum { WMS_GW_COMMAND_DATA_MAX  = 157 };

/* Bearer data subparameter mask values:
*/
enum{ WMS_MASK_BD_NULL             =   0x00000000 };
enum{ WMS_MASK_BD_MSG_ID           =   0x00000001 };
enum{ WMS_MASK_BD_USER_DATA        =   0x00000002 };
enum{ WMS_MASK_BD_USER_RESP        =   0x00000004 };
enum{ WMS_MASK_BD_MC_TIME          =   0x00000008 };
enum{ WMS_MASK_BD_VALID_ABS        =   0x00000010 };
enum{ WMS_MASK_BD_VALID_REL        =   0x00000020 };
enum{ WMS_MASK_BD_DEFER_ABS        =   0x00000040 };
enum{ WMS_MASK_BD_DEFER_REL        =   0x00000080 };
enum{ WMS_MASK_BD_PRIORITY         =   0x00000100 };
enum{ WMS_MASK_BD_PRIVACY          =   0x00000200 };
enum{ WMS_MASK_BD_REPLY_OPTION     =   0x00000400 };
enum{ WMS_MASK_BD_NUM_OF_MSGS      =   0x00000800 };
enum{ WMS_MASK_BD_ALERT            =   0x00001000 };
enum{ WMS_MASK_BD_LANGUAGE         =   0x00002000 };
enum{ WMS_MASK_BD_CALLBACK         =   0x00004000 };
enum{ WMS_MASK_BD_DISPLAY_MODE     =   0x00008000 };
enum{ WMS_MASK_BD_SCPT_DATA        =   0x00010000 };
enum{ WMS_MASK_BD_SCPT_RESULT      =   0x00020000 };
enum{ WMS_MASK_BD_DEPOSIT_INDEX    =   0x00040000 };
enum{ WMS_MASK_BD_DELIVERY_STATUS  =   0x00080000 };
enum{ WMS_MASK_BD_IP_ADDRESS       =   0x10000000 };
enum{ WMS_MASK_BD_RSN_NO_NOTIFY    =   0x20000000 };
enum{ WMS_MASK_BD_OTHER            =   0x40000000 };

/* GW Command types - ref. 3GPP TS 23.040 section 9.2.3.19
*/
typedef enum
{
  WMS_GW_COMMAND_ENQUIRY                = 0x00,
  WMS_GW_COMMAND_CANCEL_STATUS_REPORT   = 0x01,
  WMS_GW_COMMAND_DELETE_SM              = 0x02,
  WMS_GW_COMMAND_ENABLE_STATUS_REPORT   = 0x03,
  WMS_GW_COMMAND_MAX32 = 0x10000000   /* pas to 32 bit int */
  /* reserved: 0x04 - 0x1f */
  /* specific to each SC: 0xe0 - 0xff */
} wms_gw_command_e_type;

/* GW Validity Formats
*/
typedef enum
{
  WMS_GW_VALIDITY_NONE = 0,
  WMS_GW_VALIDITY_RELATIVE = 2,
  WMS_GW_VALIDITY_ABSOLUTE = 3,
  WMS_GW_VALIDITY_ENHANCED = 1,
  WMS_GW_VALIDITY_MAX32 = 0x10000000   /* pas to 32 bit int */
} wms_gw_validity_format_e_type;

/* TP-Status - ref. 3GPP TS 23.040 section 9.2.3.15
*/
typedef enum
{
  /* Short message transaction completed:
  */
  WMS_TP_STATUS_RECEIVED_OK                   = 0x00,
  WMS_TP_STATUS_UNABLE_TO_CONFIRM_DELIVERY    = 0x01,
  WMS_TP_STATUS_REPLACED                      = 0x02,
  /* reserved: 0x03 - 0x0f */
  /* specific to each SC: 0x10 - 0x1f */

  /* Temporary error, SC still trying to transfer SM:
  */
  WMS_TP_STATUS_TRYING_CONGESTION             = 0x20,
  WMS_TP_STATUS_TRYING_SME_BUSY               = 0x21,
  WMS_TP_STATUS_TRYING_NO_RESPONSE_FROM_SME   = 0x22,
  WMS_TP_STATUS_TRYING_SERVICE_REJECTED       = 0x23,
  WMS_TP_STATUS_TRYING_QOS_NOT_AVAILABLE      = 0x24,
  WMS_TP_STATUS_TRYING_SME_ERROR              = 0x25,
  /* reserved: 0x26 - 0x2f */
  /* specific to each SC: 0x30 - 0x3f */

  /* Permanent error, SC is not making any more attempts:
  */
  WMS_TP_STATUS_PERM_REMOTE_PROCEDURE_ERROR   = 0x40,
  WMS_TP_STATUS_PERM_INCOMPATIBLE_DEST        = 0x41,
  WMS_TP_STATUS_PERM_REJECTED_BY_SME          = 0x42,
  WMS_TP_STATUS_PERM_NOT_OBTAINABLE           = 0x43,
  WMS_TP_STATUS_PERM_QOS_NOT_AVAILABLE        = 0x44,
  WMS_TP_STATUS_PERM_NO_INTERWORKING          = 0x45,
  WMS_TP_STATUS_PERM_VP_EXPIRED               = 0x46,
  WMS_TP_STATUS_PERM_DELETED_BY_ORIG_SME      = 0x47,
  WMS_TP_STATUS_PERM_DELETED_BY_SC_ADMIN      = 0x48,
  WMS_TP_STATUS_PERM_SM_NO_EXISTING           = 0x49,
  /* reserved: 0x4a - 0x4f */
  /* specific to each SC: 0x50 - 0x5f */

  /* Temporary error, SC is not making any more attempts:
  */
  WMS_TP_STATUS_TEMP_CONGESTION               = 0x60,
  WMS_TP_STATUS_TEMP_SME_BUSY                 = 0x61,
  WMS_TP_STATUS_TEMP_NO_RESPONSE_FROM_SME     = 0x62,
  WMS_TP_STATUS_TEMP_SERVICE_REJECTED         = 0x63,
  WMS_TP_STATUS_TEMP_QOS_NOT_AVAILABLE        = 0x64,
  WMS_TP_STATUS_TEMP_SME_ERROR                = 0x65,
  /* reserved: 0x66 - 0x6f */
  /* specific to each SC: 0x70 - 0x7f */

  /* reserved: 0x80 - 0xff */

  WMS_TP_STATUS_LAST = 0xFF,
  WMS_TP_STATUS_MAX32 = 0x10000000   /* pas to 32 bit int */
} wms_tp_status_e_type;

/* TPDU parameter bit masks
*/
enum
{
  WMS_TPDU_MASK_PID         = 0x0001,
  WMS_TPDU_MASK_DCS         = 0x0002,
  WMS_TPDU_MASK_USER_DATA   = 0x0004
};

/* Deliver TPDU
*/
typedef struct wms_gw_deliver_s
{
  boolean                          more;                     /* TP-MMS */
  boolean                          reply_path_present;       /* TP-RP */
  boolean                          user_data_header_present; /* TP-UDHI */
  boolean                          status_report_enabled;    /* TP-SRI */
  wms_address_s_type               address;                  /* TP-OA */
  wms_pid_e_type                   pid;                      /* TP-PID */
  wms_gw_dcs_s_type                dcs;                      /* TP-DCS */
  wms_timestamp_s_type             timestamp;                /* TP-SCTS */
  wms_gw_user_data_s_type          user_data;                /* TP-UD */
} wms_gw_deliver_s_type;

/* GW Validity info
*/
typedef struct wms_gw_validity_s
{
  wms_gw_validity_format_e_type  format;
  union wms_gw_validity_u
  {
    wms_timestamp_s_type    time;
    /*~ IF (_DISC_ == WMS_GW_VALIDITY_RELATIVE || _DISC_ == WMS_GW_VALIDITY_ABSOLUTE) wms_gw_validity_u.time */
      /* used by both absolute and relative formats */
    // TBD: enhanced format

    /*~ DEFAULT wms_gw_validity_u.void */
  } u;
  /*~ FIELD wms_gw_validity_s.u DISC _OBJ_.format */
} wms_gw_validity_s_type;

/* Submit TPDU
*/
typedef struct wms_gw_submit_s
{
  boolean                           reject_duplicates;        /* TP-RD */
  boolean                           reply_path_present;       /* TP-RP */
  boolean                           user_data_header_present; /* TP-UDHI */
  boolean                           status_report_enabled;    /* TP-SRR */
  wms_message_number_type           message_reference;        /* TP-MR */
  wms_address_s_type                address;                  /* TP-DA */
  wms_pid_e_type                    pid;                      /* TP-PID */
  wms_gw_dcs_s_type                 dcs;                      /* TP-DCS */
  wms_gw_validity_s_type            validity;                 /* TP-VPF & TP-VP */
  wms_gw_user_data_s_type           user_data;                /* TP-UD */
} wms_gw_submit_s_type;

/* Status report TPDU
*/
typedef struct wms_gw_status_report_s
{
  boolean                       user_data_header_present; /* TP-UDHI */
  boolean                       more;                     /* TP-MMS */
  boolean                       status_report_qualifier;  /* TP-SRQ */
  wms_message_number_type       message_reference;        /* TP-MR */
  wms_address_s_type            address;                  /* TP-RA */
  wms_timestamp_s_type          timestamp;                /* TP-SCTS */
  wms_timestamp_s_type          discharge_time;           /* TP-DT */
  wms_tp_status_e_type          tp_status;                /* TP-ST */

  /* the mask indicates which of the optional fields are present
  */
  uint32                        mask;                     /* TP-PI */
  wms_pid_e_type                pid;                      /* TP-PID */
  wms_gw_dcs_s_type             dcs;                      /* TP-DCS */
  wms_gw_user_data_s_type       user_data;                /* TP-UD */
} wms_gw_status_report_s_type;

/* Command TPDU
*/
typedef struct wms_gw_command_s
{
  boolean                      user_data_header_present; /* TP-UDHI */
  boolean                      status_report_enabled;    /* TP-SRR */
  wms_message_number_type      message_reference;        /* TP-MR */
  wms_pid_e_type               pid;                      /* TP-PID */
  wms_gw_command_e_type        command;                  /* TP-CT */
  wms_message_number_type      message_number;           /* TP-MN */
  wms_address_s_type           address;                  /* TP-DA */
  uint8                        command_data_len;         /* TP-CDL */
  uint8                        command_data[WMS_GW_COMMAND_DATA_MAX]; /* TP-CD */
} wms_gw_command_s_type;

/* GW Point-to-Point TS data decoded from the raw TS data
*/
typedef struct wms_gw_pp_ts_data_s
{
  wms_gw_tpdu_type_e_type                tpdu_type;
    /* Note: this is not equivalent to TP-MTI */
  union wms_gw_pp_ts_data_u
  {
    wms_gw_deliver_s_type                deliver;
    /*~ CASE WMS_TPDU_DELIVER wms_gw_pp_ts_data_u.deliver */
    wms_gw_deliver_report_ack_s_type     deliver_report_ack;
    /*~ CASE WMS_TPDU_DELIVER_REPORT_ACK wms_gw_pp_ts_data_u.deliver_report_ack */
    wms_gw_deliver_report_error_s_type   deliver_report_error;
    /*~ CASE WMS_TPDU_DELIVER_REPORT_ERROR wms_gw_pp_ts_data_u.deliver_report_error */
    wms_gw_submit_s_type                 submit;
    /*~ CASE WMS_TPDU_SUBMIT wms_gw_pp_ts_data_u.submit */
    wms_gw_submit_report_ack_s_type      submit_report_ack;
    /*~ CASE WMS_TPDU_SUBMIT_REPORT_ACK wms_gw_pp_ts_data_u.submit_report_ack */
    wms_gw_submit_report_error_s_type    submit_report_error;
    /*~ CASE WMS_TPDU_SUBMIT_REPORT_ERROR wms_gw_pp_ts_data_u.submit_report_error */
    wms_gw_status_report_s_type          status_report;
    /*~ CASE WMS_TPDU_STATUS_REPORT wms_gw_pp_ts_data_u.status_report*/
    wms_gw_command_s_type                command;
    /*~ CASE WMS_TPDU_COMMAND wms_gw_pp_ts_data_u.command */

    /*~ DEFAULT wms_gw_pp_ts_data_u.void */

  } u; /*~ FIELD wms_gw_pp_ts_data_s.u DISC _OBJ_.tpdu_type */

} wms_gw_pp_ts_data_s_type;


/* GW CB Decoded Page Data:
*/
typedef struct wms_gw_cb_ts_data_s
{
  wms_gw_cb_page_header_s_type     cb_page_hdr;
  wms_gw_user_data_s_type          user_data;
} wms_gw_cb_ts_data_s_type;

/* Client TS data decoded from the raw TS data
*/
typedef struct wms_client_ts_data_s
{
  wms_format_e_type             format;

  union wms_client_ts_data_u
  {
    wms_client_bd_s_type        cdma;
      /* for both Point-to-Point and Broadcast */
    /*~ CASE WMS_FORMAT_CDMA wms_client_ts_data_u.cdma */

    wms_gw_pp_ts_data_s_type    gw_pp;
      /* Point-to-Point */
    /*~ CASE WMS_FORMAT_GW_PP wms_client_ts_data_u.gw_pp */

    wms_gw_cb_ts_data_s_type    gw_cb;
      /* Cell Broadcast */
    /*~ CASE WMS_FORMAT_GW_CB wms_client_ts_data_u.gw_cb */

    /*~ DEFAULT wms_client_ts_data_u.void */

  } u; /*~ FIELD wms_client_ts_data_s.u DISC _OBJ_.format */

} wms_client_ts_data_s_type;

/*=========================================================================
FUNCTION
  wms_ts_int_to_bcd

DESCRIPTION
  Convert an Integer to a BCD.
  Valid Integer values are from 0 to 99 and byte arrangement is <MSB, ... ,LSB)

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_int_to_bcd
(
  const uint8 i
);

/*=========================================================================
FUNCTION
  wms_ts_encode_deliver_report_ack

DESCRIPTION
  Encode Deliver-Report-Ack structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of encoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_encode_deliver_report_ack
(
  const wms_gw_deliver_report_ack_s_type    *deliver_report_ack,
  wms_raw_ts_data_s_type                    *raw_ts_data_ptr
);

/*=========================================================================
FUNCTION
  wms_ts_encode_deliver_report_error

DESCRIPTION
  Encode Deliver-Report-Error structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of encoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_encode_deliver_report_error
(
  const wms_gw_deliver_report_error_s_type    *deliver_report_error,
  wms_raw_ts_data_s_type                      *raw_ts_data_ptr
);


/*=========================================================================
FUNCTION
  wms_ts_decode_deliver

DESCRIPTION
  Decode Deliver TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_deliver
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr,
  wms_gw_deliver_s_type                   * deliver
);

/*=========================================================================
FUNCTION
  wms_ts_decode_submit_report_ack

DESCRIPTION
  Decode Submit-Report-Ack TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_submit_report_ack
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr,
  wms_gw_submit_report_ack_s_type         * submit_report_ack
);

/*=========================================================================
FUNCTION
  wms_ts_decode_submit_report_error

DESCRIPTION
  Decode Submit-Report-Error TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_submit_report_error
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr,
  wms_gw_submit_report_error_s_type       * submit_report_error
);

/*=========================================================================
FUNCTION
  wms_ts_decode_deliver_report_ack

DESCRIPTION
  Decode Deliver-Report-Ack TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_deliver_report_ack
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr,
  wms_gw_deliver_report_ack_s_type        * deliver_report_ack
);


/*=========================================================================
FUNCTION
  wms_ts_decode_deliver_report_error

DESCRIPTION
  Decode Deliver-Report-Error TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_deliver_report_error
(
  const wms_raw_ts_data_s_type              * raw_ts_data_ptr,
  wms_gw_deliver_report_error_s_type        * deliver_report_error
);

/*=========================================================================
FUNCTION
  wms_ts_encode_address

DESCRIPTION
  Encode address structure into format for SIM storage and for interfacing
  with lower layers.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes encoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_encode_address
(
  const wms_address_s_type    * addr,
  uint8                         * data
);

/*=========================================================================
FUNCTION
  wms_ts_encode

DESCRIPTION
  Allow the client to encode a Transport Service message.

DEPENDENCIES
  None

RETURN VALUE
  Encoding status

SIDE EFFECTS
  None

COMMENTS
  If referenced in the union, Set client_ts_data_ptr->u.cdma.other.input_other_len
                              to a Valid Value or 0.
  If referenced in the union, Set client_ts_data_ptr->u.cdma.other.other_data
                              to a Valid Value or NULL.

=========================================================================*/
wms_status_e_type wms_ts_encode
(
  const wms_client_ts_data_s_type         * client_ts_data_ptr,
  wms_raw_ts_data_s_type                  * raw_ts_data_ptr /* IN/OUTPUT */
);

/*=========================================================================
FUNCTION
  wms_ts_encode_submit_report_ack

DESCRIPTION
  Encode Submit-Report-Ack structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of encoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_encode_submit_report_ack
(
  const wms_gw_submit_report_ack_s_type     *submit_report_ack,
  wms_raw_ts_data_s_type                    *raw_ts_data_ptr
);

/*=========================================================================
FUNCTION
  wms_ts_encode_user_data_header

DESCRIPTION
  Encode User Data structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes encoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_encode_user_data_header
(
  uint8                           num_headers, /* IN */
  const wms_udh_s_type            * headers,   /* IN */
  uint8                           *data        /* OUT */
);

/*=========================================================================
FUNCTION
  wms_ts_decode_user_data_header

DESCRIPTION
  Decode User Data from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes decoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_decode_user_data_header
(
  const uint8               len, /* user_data_length*/
  const uint8               *data,
  uint8                     * num_headers_ptr, /* OUT */
  wms_udh_s_type            * udh_ptr          /* OUT */
);

/*=========================================================================
FUNCTION
  wms_ts_decode_address

DESCRIPTION
  Decode address data into a structure.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes decoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_decode_address
(
  const uint8               * data,
  wms_address_s_type        * addr
);

/*=========================================================================
FUNCTION
  wms_ts_decode_user_status_report

DESCRIPTION
  Decode Status Report from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Decoding status

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_status_report
(
  const wms_raw_ts_data_s_type              * raw_ts_data_ptr,
  wms_gw_status_report_s_type               * status_report
);

/*=========================================================================
FUNCTION
  wms_ts_decode_gw_cb_page_header

DESCRIPTION
  Decode GSM Cell Broadcast Page Header from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes decoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_decode_gw_cb_page_header
(
  const uint8                   * data,
  wms_gw_cb_page_header_s_type  * page_header_ptr
);

/*=========================================================================
FUNCTION
  wms_ts_decode_gw_cb_dcs

DESCRIPTION
  Decode GSM Cell Broadcast Data Coding Scheme.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes decoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_decode_gw_cb_dcs
(
  const uint8                  data,            /* INPUT */
  const uint8                  * user_data_ptr, /* INPUT */
  wms_gw_cb_dcs_s_type         * dcs_ptr        /* OUTPUT */
);

/*=========================================================================
FUNCTION
  wms_ts_decode

DESCRIPTION
  Allow the client to decode a Transport Service message.

  Note: If the CDMA bearer data has 'other' parameters, upon return of this
  API function, the 'mask' will have WMS_MASK_BD_OTHER set, and
  'desired_other_len' will indicate how many bytes are needed to properly
  decode the 'other' parameters by calling wms_ts_decode_cdma_bd_with_other().

DEPENDENCIES
  None

RETURN VALUE
  Decoding status

SIDE EFFECTS
  None

COMMENTS
  If referenced in the union, Set client_ts_data_ptr->u.cdma.other.input_other_len
                              to a Valid Value or 0.
  If referenced in the union, Set client_ts_data_ptr->u.cdma.other.other_data
                              to a Valid Value or NULL.
=========================================================================*/
wms_status_e_type wms_ts_decode
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr,
  wms_client_ts_data_s_type               * client_ts_data_ptr /* OUTPUT */
);

/*=========================================================================
FUNCTION
  wms_ts_compute_user_data_header_length

DESCRIPTION
  Computes the User Data Header Length

DEPENDENCIES
  None

RETURN VALUE
  User Data Header Length in bytes

SIDE EFFECTS
  None
=========================================================================*/
uint32 wms_ts_compute_user_data_header_length
(
  const uint8           num_headers,
  const wms_udh_s_type *headers
);

/*=========================================================================
FUNCTION
  wms_ts_compute_gw_user_data_length

DESCRIPTION
  Computes the GW User Data Length

DEPENDENCIES
  None

RETURN VALUE
  GW User Data Length in bytes

SIDE EFFECTS
  None
=========================================================================*/
uint32 wms_ts_compute_gw_user_data_length
(
  const wms_gw_dcs_s_type         *dcs,
  const wms_gw_user_data_s_type   *user_data
);

/*=========================================================================
FUNCTION
  wms_ts_pack_gw_7_bit_chars

DESCRIPTION
  Pack GSM 7-bit characters into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes encoded.

SIDE EFFECTS
  None
=========================================================================*/
uint16 wms_ts_pack_gw_7_bit_chars
(
  const uint8     * in,
  uint16          in_len, //number of chars
  uint16          shift,
  uint16          out_len_max,
  uint8           * out
);

/*=========================================================================
FUNCTION
  wms_ts_unpack_gw_7_bit_chars

DESCRIPTION
  Unpack raw data to GSM 7-bit characters.

DEPENDENCIES
  None

RETURN VALUE
  Number of 7-bit charactes decoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_unpack_gw_7_bit_chars
(
  const uint8       * in,
  uint8             in_len,
  uint8             out_len_max,
  uint16            shift,
  uint8             * out
);

/*=========================================================================
FUNCTION
  wms_ts_decode_relative_time

DESCRIPTION
  Decode the Relative Timestamp raw byte to a WMS Timestamp structure.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
=========================================================================*/
void wms_ts_decode_relative_time
(
  uint8                     v,
  wms_timestamp_s_type    * timestamp
);

/*=========================================================================
FUNCTION
  wms_ts_encode_relative_time

DESCRIPTION
  Encode the Relative Time to a raw byte.

DEPENDENCIES
  None

RETURN VALUE
  Encoded Byte

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_encode_relative_time
(
  const wms_timestamp_s_type  *timestamp
);


/*=========================================================================
FUNCTION
  wmsts_verify

DESCRIPTION
  Verifies RIL_CDMA_SMS_ClientBd does not differ from wms_client_bd_s_type.
  This is important since QCRIL takes a RIL_CDMA_SMS_ClientBd type and
  casts it to wms_client_bd_s_type to pass it into the wmsts library.
  This check is currently limited to detecting changes in ENUM values or
  sizes of any underlying data structures.  Any changes to the RIL_CDMA_SMS
  types in ril.h should be investigated specifically to make sure they 
  won't break the wmsts library.

DEPENDENCIES
  None

RETURN VALUE
  0 == SUCCESS, 1 == FAILURE

SIDE EFFECTS
  None
=========================================================================*/
int wmsts_verify (void);


/*=========================================================================
FUNCTION
  wms_ts_ascii_to_bcd

DESCRIPTION
  Convert an ASCII number string to BCD digits.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
=========================================================================*/
void wms_ts_ascii_to_bcd
(
  const uint8     * ascii,
  uint8           len,
  uint8           * out
);


/*=========================================================================
FUNCTION
  wms_ts_bcd_to_ascii

DESCRIPTION
  Convert BCD digits to an ASCII number string.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
=========================================================================*/
void wms_ts_bcd_to_ascii
(
  const uint8         * addr,
  uint8               len,
  uint8               * out
);

#endif /* WMSTS_H */


