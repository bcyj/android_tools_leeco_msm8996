/******************************************************************************

                       Q M I _ C S V T _ S R V C . H

******************************************************************************/

/******************************************************************************

  @file    qmi_csvt_srvc.h
  @brief   Header file for QMI CSVT Service based on QCCI

  DESCRIPTION
  Header file for QMI CSVT service client API definitions.

******************************************************************************/
/*===========================================================================
  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
11/11/11   sg         Initial version

******************************************************************************/

#ifndef __QMI_CSVT_SRVC_H__
#define __QMI_CSVT_SRVC_H__

#include "comdef.h"
#include "qmi.h"
#include "circuit_switched_video_telephony_v01.h"
#include "qmi_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
                     TYPE DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define  QMI_CSVT_INVALID_INST_ID  (0xFFFFFFFF)
#define  QMI_CSVT_INVALID_HNDL     (NULL)
#define  QMI_CSVT_MAX_STR_SIZE     100
#define  QMI_CSVT_MAX_RLP_PARAMS   3


/* Common type definitions */
typedef const void  *qmi_csvt_clnt_hndl;
typedef uint32  qmi_csvt_instance_id_type;

/* QMI CSVT async callback related defines */
typedef enum
{
  QMI_CSVT_ASYNC_RESP_ID_ORIGINATE_CALL
} qmi_csvt_async_resp_id_type;

typedef struct
{
  qmi_csvt_instance_id_type  instance_id;
} qmi_csvt_originate_call_resp_data_type;

/* Union of asynchronous reponses to send to the clients */
typedef union
{
  qmi_csvt_originate_call_resp_data_type  orig_call;
} qmi_csvt_async_resp_data_type;

/* QMI CSVT client registered async callback function */
typedef  void (*qmi_csvt_async_cb_type)
(
  int                            qmi_err_code,
  qmi_csvt_async_resp_id_type    resp_id,
  qmi_csvt_async_resp_data_type  *resp_data,
  void                           *user_data
);

/* QMI CSVT indications */
typedef enum
{
  QMI_CSVT_IND_TYPE_INVALID = 0,
  QMI_CSVT_IND_TYPE_CALL_CONFIRM,
  QMI_CSVT_IND_TYPE_CALL_PROGRESS,
  QMI_CSVT_IND_TYPE_CALL_CONNECT,
  QMI_CSVT_IND_TYPE_CALL_SETUP,
  QMI_CSVT_IND_TYPE_INCOMING_CALL,
  QMI_CSVT_IND_TYPE_CALL_END,
  QMI_CSVT_IND_TYPE_CALL_MODIFY
} qmi_csvt_ind_type;

/* QMI CSVT indication data */
#define  QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_TYPE       0x00000001
#define  QMI_CSVT_CALL_INFO_PARAM_MASK_IS_SYNC_CALL    0x00000002
#define  QMI_CSVT_CALL_INFO_PARAM_MASK_IS_TRANS_CALL   0x00000004
#define  QMI_CSVT_CALL_INFO_PARAM_MASK_NETWORK_TYPE    0x00000008
#define  QMI_CSVT_CALL_INFO_PARAM_MASK_NETWORK_SPEED   0x00000010
#define  QMI_CSVT_CALL_INFO_PARAM_MASK_MAX_FRAME_SIZE  0x00000020
#define  QMI_CSVT_CALL_INFO_PARAM_MASK_INCOMING_NUM    0x00000040
#define  QMI_CSVT_CALL_INFO_PARAM_MASK_UUS_ID          0x00000080
#define  QMI_CSVT_CALL_INFO_PARAM_MASK_MODIFY_ALLOWED  0x00000100
#define  QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_END_CAUSE  0x00000200
#define  QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_PORT_DATA  0x00000400

#define  QMI_CSVT_IND_MASK_CALL_PORT_DATA  0x00000400

/* Call type masks */
#define  QMI_CSVT_CALL_TYPE_MASK_ASYNC_CSVT_CALL       0x01
#define  QMI_CSVT_CALL_TYPE_MASK_SYNC_CSVT_CALL        0x02
#define  QMI_CSVT_CALL_TYPE_MASK_VIDEO_TELEPHONY_CALL  0x08

typedef enum
{
  QMI_CSVT_CALL_TYPE_INVALID         = 0x00,
  QMI_CSVT_CALL_TYPE_ASYNC           = 0x01,
  QMI_CSVT_CALL_TYPE_SYNC            = 0x02,
  QMI_CSVT_CALL_TYPE_VIDEO_TELEPHONY = 0x08,
} qmi_csvt_call_type;

typedef enum
{
  QMI_CSVT_NETWORK_TYPE_GSM   = 0x03,
  QMI_CSVT_NETWORK_TYPE_WCDMA = 0x05,
  QMI_CSVT_NETWORK_TYPE_TDS   = 0x0B
} qmi_csvt_network_type;

typedef enum
{
  QMI_CSVT_UUS_ID_TYPE_EMAIL = 0x01,
  QMI_CSVT_UUS_ID_TYPE_URL   = 0x02,
  QMI_CSVT_UUS_ID_TYPE_H323  = 0x03
} qmi_csvt_uus_id_type;

typedef enum
{
  QMI_CSVT_PORT_TYPE_SMD = 0x04,
  QMI_CSVT_PORT_TYPE_A2  = 0x0E,
  QMI_CSVT_PORT_TYPE_MUX = 0x0F
} qmi_csvt_port_type;

typedef struct
{
  qmi_csvt_port_type  port_type;
  uint8               port_num;
} qmi_csvt_port_data_type;

typedef struct
{
  /* Valid if QMI_CSVT_IND_MASK_UUS_ID_TYPE is set */
  qmi_csvt_uus_id_type       uus_id_type;

  /* Valid if QMI_CSVT_IND_MASK_UUS_ID is set */
  char                       uus_id[QMI_CSVT_MAX_STR_SIZE];
} qmi_csvt_uus_id_info_type;

typedef struct
{
  /* Optional parameters */
  uint32                     param_mask;

  /* Valid if QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_TYPE is set */
  qmi_csvt_call_type         call_type;

  /* Valid if QMI_CSVT_CALL_INFO_PARAM_MASK_IS_SYNC_CALL is set */
  boolean                    is_call_synchronous;

  /* Valid if QMI_CSVT_CALL_INFO_PARAM_MASK_IS_TRANS_CALL is set */
  boolean                    is_call_transparent;

  /* Valid if QMI_CSVT_CALL_INFO_PARAM_MASK_NETWORK_TYPE is set */
  qmi_csvt_network_type      network_type;

  /* Valid if QMI_CSVT_CALL_INFO_PARAM_MASK_NETWORK_SPEED is set */
  uint16                     network_speed;

  /* Valid if QMI_CSVT_CALL_INFO_PARAM_MASK_MAX_FRAME_SIZE is set */
  uint8                      max_frame_size;

  /* Valid if QMI_CSVT_CALL_INFO_PARAM_MASK_INCOMING_NUM is set */
  char                       incoming_num[QMI_CSVT_MAX_STR_SIZE];

  /* Valid if QMI_CSVT_CALL_INFO_PARAM_MASK_UUS_ID is set */
  qmi_csvt_uus_id_info_type  uus_id;

  /* Valid if QMI_CSVT_CALL_INFO_PARAM_MASK_MODIFY_ALLOWED is set */
  boolean                    is_modify_allowed;

  /* Valid if QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_END_CAUSE is set */
  uint8                      call_end_cause;

  /* Valid if QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_PORT_DATA is set */
  qmi_csvt_port_data_type    port_data;

} qmi_csvt_call_info_type;

typedef struct
{
  /* Mandatory fields */
  qmi_csvt_ind_type                 ind_type;

  qmi_csvt_instance_id_type         instance_id;

  /* Optional call info */
  qmi_csvt_call_info_type           opt;

} qmi_csvt_ind_data_type;

/* QMI CSVT client indication handler type */
typedef void (*qmi_csvt_ind_hdlr_type)(qmi_csvt_ind_data_type  *ind_data,
                                       void                    *user_data);

/* QMI_CSVT_SET_EVENT_REPORT related defines */
#define  QMI_CSVT_EVT_REPORT_CALL_EVENTS_PARAM_MASK     0x0001
#define  QMI_CSVT_EVT_REPORT_CALL_TYPE_MASK_PARAM_MASK  0x0002

typedef struct
{
  uint16   param_mask;

  /* Valid if QMI_CSVT_EVT_REPORT_CALL_EVENTS_PARAM_MASK is set */
  boolean  report_call_events;

  /* Valid if QMI_CSVT_EVT_REPORT_CALL_TYPE_MASK_PARAM_MASK is set */
  uint32   call_type_mask;

} qmi_csvt_event_report_params_type;


/* QMI_CSVT_ORIGINATE_CALL related defines */
#define  QMI_CSVT_CALL_PARAM_MASK_NETWORK_DATARATE    0x00000001
#define  QMI_CSVT_CALL_PARAM_MASK_AIR_IFACE_DATARATE  0x00000002
#define  QMI_CSVT_CALL_PARAM_MASK_SYNCHRONOUS_CALL    0x00000004
#define  QMI_CSVT_CALL_PARAM_MASK_TRANSPARENT_CALL    0x00000008
#define  QMI_CSVT_CALL_PARAM_MASK_CLI_ENABLED         0x00000010
#define  QMI_CSVT_CALL_PARAM_MASK_CUG_ENABLED         0x00000020
#define  QMI_CSVT_CALL_PARAM_MASK_CUG_INDEX           0x00000040
#define  QMI_CSVT_CALL_PARAM_MASK_SUPRESS_PREF_CUG    0x00000080
#define  QMI_CSVT_CALL_PARAM_MASK_SUPRESS_OUT_ACCESS  0x00000100
#define  QMI_CSVT_CALL_PARAM_MASK_UUS_ID              0x00000200

typedef enum
{
  QMI_CSVT_CALL_MODE_DATA_ONLY  = 0x01,
  QMI_CSVT_CALL_MODE_VOICE_DATA = 0x02,
  QMI_CSVT_CALL_MODE_DATA_VOICE = 0x04
} qmi_csvt_call_mode_type;

typedef struct
{
  /* Mandatory parameters */
  qmi_csvt_instance_id_type  inst_id;

  qmi_csvt_call_mode_type    call_mode;

  char                       dial_string[QMI_CSVT_MAX_STR_SIZE];

  /* Optional parameters */
  uint32                     param_mask;

  /* Valid if QMI_CSVT_CALL_PARAM_MASK_NETWORK_DATARATE is set in param_mask */
  uint8                      network_data_rate;

  /* Valid if QMI_CSVT_CALL_PARAM_MASK_AIR_IFACE_DATARATE is set in param_mask */
  uint8                      air_iface_data_rate;

  /* Valid if QMI_CSVT_CALL_PARAM_MASK_SYNCHRONOUS_CALL is set in param_mask */
  boolean                    is_call_synchronous;

  /* Valid if QMI_CSVT_CALL_PARAM_MASK_TRANSPARENT_CALL is set in param_mask */
  boolean                    is_call_transparent;

  /* Valid if QMI_CSVT_CALL_PARAM_MASK_CLI_ENABLED is set in param_mask */
  boolean                    is_cli_enabled;

  /* Valid if QMI_CSVT_CALL_PARAM_MASK_CUG_ENABLED is set in param_mask */
  boolean                    is_cug_enabled;

  /* Valid if QMI_CSVT_CALL_PARAM_MASK_CUG_ENABLED is set in param_mask */
  uint8                      cug_index;

  /* Valid if QMI_CSVT_CALL_PARAM_MASK_SUPRESS_PREF_CUG is set in param_mask */
  boolean                    supress_pref_cug;

  /* Valid if QMI_CSVT_CALL_PARAM_MASK_SUPRESS_OUT_ACCESS is set in param_mask */
  boolean                    supress_out_access;

  /* Valid if QMI_CSVT_CALL_PARAM_UUS_ID is set in param_mask */
  qmi_csvt_uus_id_info_type  uus_id;

} qmi_csvt_call_params_type;

/* QMI_CSVT_MODIFY_CALL related defines */
typedef struct
{
  qmi_csvt_instance_id_type  instance_id;

  qmi_csvt_call_type         new_call_type;

} qmi_csvt_modify_call_params_type;

/* QMI_CSVT_ACK_CALL_MODIFY related defines */
typedef struct
{
  qmi_csvt_instance_id_type  instance_id;

  boolean                    accept_req;

} qmi_csvt_ack_call_modify_params_type;

/* QMI_CSVT_GET_RLP_PARAMS related defines */
/* RLP Parameters */
typedef struct
{
  uint32  version;
  uint32  tx_window_size;
  uint32  rx_window_size;
  uint32  ack_timer;
  uint32  retrans_attempts;
  uint32  reseq_timer;
} qmi_csvt_rlp_params_type;

/* V42 parameters related defines */
/* Direction of compression */
typedef enum
{
  QMI_CSVT_V42_COMPRESS_ORIGINATED_CALLS = 1,
  QMI_CSVT_V42_COMPRESS_ANSWERED_CALLS
} qmi_csvt_v42_direction_type;

/* Compression preference */
typedef enum
{
  QMI_CSVT_V42_COMPRESS_WHEN_POSSIBLE = 0,
  QMI_CSVT_V42_COMPRESS_ALWAYS_ENABLED,
} qmi_csvt_v42_negotiation_type;

/* V42 Parameters */
typedef struct
{
  qmi_csvt_v42_direction_type    direction;
  qmi_csvt_v42_negotiation_type  negotiation_pref;
  uint32                         max_dict_size;
  uint32                         max_str_size;
} qmi_csvt_v42_params_type;

#define  QMI_CSVT_RLP_PARAMS_VALID_PARAM_MASK     (0x0001)
#define  QMI_CSVT_RLP_PARAMS_V1_VALID_PARAM_MASK  QMI_CSVT_RLP_PARAMS_VALID_PARAM_MASK   
#define  QMI_CSVT_RLP_PARAMS_V2_VALID_PARAM_MASK  (0x0002)
#define  QMI_CSVT_RLP_PARAMS_V3_VALID_PARAM_MASK  (0x0004)
#define  QMI_CSVT_V42_PARAMS_VALID_PARAM_MASK     (0x0008)

typedef struct
{
  uint16                    param_mask;

  /* Valid if QMI_CSVT_RLP_PARAMS_V1_PARAM_MASK is set */ 
  qmi_csvt_rlp_params_type  rlp_params_v1;

  /* Valid if QMI_CSVT_RLP_PARAMS_V2_PARAM_MASK is set */ 
  qmi_csvt_rlp_params_type  rlp_params_v2;

  /* Valid if QMI_CSVT_RLP_PARAMS_V3_PARAM_MASK is set */ 
  qmi_csvt_rlp_params_type  rlp_params_v3;

  /* Valid if QMI_CSVT_V42_PARAMS_PARAM_MASK is set */ 
  qmi_csvt_v42_params_type  v42_params;

} qmi_csvt_get_rlp_params_type;

typedef struct
{
  uint16                    param_mask;

  /* Valid if QMI_CSVT_RLP_PARAMS_PARAM_MASK is set */ 
  qmi_csvt_rlp_params_type  rlp_params;

  /* Valid if QMI_CSVT_V42_PARAMS_PARAM_MASK is set */ 
  qmi_csvt_v42_params_type  v42_params;

} qmi_csvt_set_rlp_params_type;

/* QMI_CSVT_GET_CALL_STATS related defines */
#define  QMI_CSVT_CALL_STATS_CALL_ACTIVE_PARAM_MASK  0x0001
#define  QMI_CSVT_CALL_STATS_TX_COUNTER_PARAM_MASK   0x0002
#define  QMI_CSVT_CALL_STATS_RX_COUNTER_PARAM_MASK   0x0004

typedef struct
{
  uint16   param_mask;

  /* Valid if QMI_CSVT_CALL_STATS_CALL_ACTIVE_PARAM_MASK */
  boolean  is_call_active;

  /* Valid if QMI_CSVT_CALL_STATS_TX_COUNTER_PARAM_MASK */
  uint32   tx_counter;

  /* Valid if QMI_CSVT_CALL_STATS_RX_COUNTER_PARAM_MASK */
  uint32   rx_counter;

} qmi_csvt_call_stats_type;


/*===========================================================================
                     GLOBAL FUNCTION DECLARATIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qmi_csvt_init_client
===========================================================================*/
/*!
@brief
  Initialize a CSVT client. This function should be called before calling
  other API functions.

@param
  conn_id     - QMI Connection ID to use
  ind_cb      - Callback function for receiving indications
  ind_cb_data - User data associated with the callback

@return
  CSVT client handle used in other API functions on SUCCESS
  NULL on FAILURE

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
qmi_csvt_clnt_hndl
qmi_csvt_init_client
(
  const char              *conn_id,
  qmi_csvt_ind_hdlr_type  ind_cb,
  void                    *ind_cb_data
);


/*===========================================================================
  FUNCTION  qmi_csvt_release_client
===========================================================================*/
/*!
@brief
  Release a CSVT client.

@param
  csvt_hndl    - CSVT client handle

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_release_client
(
  qmi_csvt_clnt_hndl  csvt_hndl
);


/*===========================================================================
  FUNCTION  qmi_csvt_reset
===========================================================================*/
/*!
@brief
  Reset the CSVT service state variables of the requesting control point.

@param
  clnt_hndl    - CSVT client handle
  qmi_err_code - QMI error received if return value is not QMI_NO_ERR

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_reset
(
  qmi_csvt_clnt_hndl  clnt_hndl,
  int                 *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_csvt_set_event_report
===========================================================================*/
/*!
@brief
  Set the event report preference of the requesting control point.

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@param
  clnt_hndl    - QMI CSVT client handle
  event_params - Event report parameters
  qmi_err_code - QMI error received if return value is QMI_SERVICE_ERR

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_set_event_report
(
  qmi_csvt_clnt_hndl                       clnt_hndl,
  const qmi_csvt_event_report_params_type  *event_params,
  int                                      *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_csvt_originate_call
===========================================================================*/
/*!
@brief
  Allows the controlling control point to originate a new CSVT call

@param
  clnt_hndl          - QMI CSVT client handle
  call_params        - Call parameters
  qmi_err_code [out] - QMI error received if return value is not QMI_NO_ERR

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

  In case of asynchronous mode and return value > 0, the value would
  indicate the transaction ID that can be used to abort the call later

@note

  - This function executes synchronously if user_cb is NULL and asynchronously
    if one is provideed.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_originate_call
(
  qmi_csvt_clnt_hndl               clnt_hndl,
  const qmi_csvt_call_params_type  *call_params,
  int                              *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_csvt_confirm_call
===========================================================================*/
/*!
@brief
  This message allows the controlling control point to confirm a CSVT
  call that it had originated or answered earlier

@param
  clnt_hndl    - QMI CSVT client handle
  inst_id      - Instance ID of the call to confirm
  confirm_call - Confirm or Reject the call
  qmi_err_code - QMI error received if return value is QMI_SERVICE_ERR

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_confirm_call
(
  qmi_csvt_clnt_hndl         clnt_hndl,
  qmi_csvt_instance_id_type  inst_id,
  boolean                    confirm_call,
  int                        *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_csvt_answer_call
===========================================================================*/
/*!
@brief
  This message allows the controlling control point to answer an incoming
  CSVT call

@param
  clnt_hndl    - QMI CSVT client handle
  inst_id      - Instance ID of the call to answer
  answer_call  - Answer or Reject the call
  qmi_err_code - QMI error received if return value is QMI_SERVICE_ERR

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_answer_call
(
  qmi_csvt_clnt_hndl         clnt_hndl,
  qmi_csvt_instance_id_type  inst_id,
  boolean                    answer_call,
  int                        *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_csvt_end_call
===========================================================================*/
/*!
@brief
  This message ends an ongoing CSVT call.

@param
  clnt_hndl    - QMI CSVT client handle
  inst_id      - Instance ID of the call to end
  qmi_err_code - QMI error received if return value is QMI_SERVICE_ERR

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_end_call
(
  qmi_csvt_clnt_hndl         clnt_hndl,
  qmi_csvt_instance_id_type  inst_id,
  int                        *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_csvt_modify_call
===========================================================================*/
/*!
@brief
  This message allows the control point to modify an existing call or to accept
  or reject a modification initiated by the network.

@param
  clnt_hndl     - QMI CSVT client handle
  modify_params - Call parameters to modify
  qmi_err_code  - QMI error received if return value is QMI_SERVICE_ERR

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_modify_call
(
  qmi_csvt_clnt_hndl                      clnt_hndl,
  const qmi_csvt_modify_call_params_type  *modify_params,
  int                                     *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_csvt_ack_call_modify
===========================================================================*/
/*!
@brief 
  This message allows the control point to accept a network-initiated
  call modification.

@param
  clnt_hndl         - QMI CSVT client handle
  ack_modify_params - Acknowledge parameters
  qmi_err_code      - QMI error received if return value is QMI_SERVICE_ERR

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_ack_call_modify
(
  qmi_csvt_clnt_hndl                          clnt_hndl,
  const qmi_csvt_ack_call_modify_params_type  *ack_modify_params,
  int                                         *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_csvt_get_rlp_params
===========================================================================*/
/*!
@brief
  This message queries the current active settings for the radio link
  protocol (RLP) and V42.bis

@param
  clnt_hndl    - QMI CSVT client handle
  rlp_params   - RLP & V42 parameters
  qmi_err_code - QMI error received if return value is QMI_SERVICE_ERR

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_get_rlp_params
(
  qmi_csvt_clnt_hndl             clnt_hndl,
  qmi_csvt_get_rlp_params_type   *rlp_params,
  int                            *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_csvt_set_rlp_params
===========================================================================*/
/*!
@brief
  This message allows the control point to set the non-transparent parameters
  in the radio link protocol (RLP) and V42.bis settings.

@param
  clnt_hndl    - QMI CSVT client handle
  rlp_params   - RLP & V42 parameters
  qmi_err_code - QMI error received if return value is QMI_SERVICE_ERR

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_set_rlp_params
(
  qmi_csvt_clnt_hndl                   clnt_hndl,
  const qmi_csvt_set_rlp_params_type   *rlp_params,
  int                                  *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_csvt_get_active_call_list
===========================================================================*/
/*!
@brief 
  This message queries the list of the current active CSVT calls.

@param
  clnt_hndl              - QMI CSVT client handle
  active_call_list [out] - instance IDs of active calls
  list_len [in/out]      - memory allocated for active_call_list on input and
                           the number of active calls found on return
  qmi_err_code           - QMI error received if return value is QMI_SERVICE_ERR

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_get_active_call_list
(
  qmi_csvt_clnt_hndl         clnt_hndl,
  qmi_csvt_instance_id_type  *active_call_list,
  uint32                     *active_call_list_len,
  int                        *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_csvt_get_call_stats
===========================================================================*/
/*!
@brief
  This message queries the call information for the given call instance.

@param
  clnt_hndl    - QMI CSVT client handle
  inst_id      - Instance ID of the call
  call_info    - Call information
  qmi_err_code - QMI error received if return value is QMI_SERVICE_ERR

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_get_call_info
(
  qmi_csvt_clnt_hndl         clnt_hndl,
  qmi_csvt_instance_id_type  inst_id,
  qmi_csvt_call_info_type    *call_info,
  int                        *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_csvt_get_call_stats
===========================================================================*/
/*!
@brief
  This message queries the call statistics for the given call instance.

@param
  clnt_hndl    - QMI CSVT client handle
  inst_id      - Instance ID of the call
  call_stats   - Statisics associated with the given inst_id
  qmi_err_code - QMI error received if return value is QMI_SERVICE_ERR

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_csvt_get_call_stats
(
  qmi_csvt_clnt_hndl         clnt_hndl,
  qmi_csvt_instance_id_type  inst_id,
  qmi_csvt_call_stats_type   *call_stats,
  int                        *qmi_err_code
);

#ifdef __cplusplus
}
#endif

#endif /* __QMI_CSVT_SRVC_H__ */

