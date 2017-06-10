/******************************************************************************

                       Q M I _ C S V T _ S R V C . C

******************************************************************************/

/******************************************************************************
  @file    qmi_csvt_srvc.c
  @brief   QMI CSVT service definitions

  DESCRIPTION
  Implementation of QMI CSVT service client API

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

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <pthread.h>
#include "qmi_i.h"
#include "qmi_platform.h"
#include "qmi_csvt_srvc.h"


/*===========================================================================
                          LOCAL DEFINITIONS
===========================================================================*/

/* Timeout for a QMI synchronous call in millisec */
#define QMI_CSVT_MAX_SYNC_MSG_TIMEOUT  30000
#define QMI_CSVT_SYNC_MSG_TIMEOUT      10000

#define QMI_CSVT_MUTEX_LOCK(mutex)                                        \
  do                                                                      \
  {                                                                       \
    if (0 == pthread_mutex_lock(&mutex))                                  \
    {                                                                     \
      QMI_DEBUG_MSG_1(">>>>>> LOCK QMI CSVT MUTEX %p SUCCESS", &mutex);   \
    }                                                                     \
    else                                                                  \
    {                                                                     \
      QMI_ERR_MSG_1(">>>>>> LOCK QMI CSVT MUTEX %p FAILURE", &mutex);     \
    }                                                                     \
  }                                                                       \
  while (0)

#define QMI_CSVT_MUTEX_UNLOCK(mutex)                                      \
  do                                                                      \
  {                                                                       \
    if (0 == pthread_mutex_unlock(&mutex))                                \
    {                                                                     \
      QMI_DEBUG_MSG_1("<<<<<< UNLOCK QMI CSVT MUTEX %p SUCCESS", &mutex); \
    }                                                                     \
    else                                                                  \
    {                                                                     \
      QMI_ERR_MSG_1("<<<<<< UNLOCK QMI CSVT MUTEX %p FAILURE", &mutex);   \
    }                                                                     \
  }                                                                       \
  while (0)


typedef enum
{
  QMI_CSVT_REQUEST_STATUS_INVALID = -1,
  QMI_CSVT_REQUEST_STATUS_SUCCESS,
  QMI_CSVT_REQUEST_STATUS_FAILED
} qmi_csvt_req_status_t;

typedef struct
{
  qmi_csvt_req_status_t  status;
  boolean                is_ce_reason_valid;
  int                    ce_reason;
} qmi_csvt_req_status_info_t;

typedef struct
{
  qmi_idl_service_object_type  csvt_srvc_obj;
  qmi_client_type              qmi_clnt_hndl;

  qmi_csvt_instance_id_type    inst_id;

  /* Client indication callback */
  qmi_csvt_ind_hdlr_type       ind_cb;
  void                         *ind_cb_data;

  /* Wait for and signal incoming indications */
  pthread_mutex_t              req_ind_mutex;
  pthread_cond_t               req_ind_condition;

  qmi_csvt_req_status_info_t   req_status;

} qmi_csvt_client_info_type;


/*===========================================================================
                          LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qmi_csvt_translate_ind
===========================================================================*/
/*!
@brief
  Translates the received QMI indication to CSVT client indication

@param
  None

@return
  QMI_NO_ERR       - when there's no error
  QMI_INTERNAL_ERR - otherwise

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
static int
qmi_csvt_translate_ind
(
  csvt_event_report_ind_msg_v01  *qmi_ind,
  qmi_csvt_ind_data_type         *csvt_ind
)
{
  int rc = QMI_INTERNAL_ERR;


  if (!qmi_ind || !csvt_ind)
  {
    QMI_ERR_MSG_0("qmi_csvt_translate_ind: bad param(s)");
    goto bail;
  }

  if (qmi_ind->event_type < CSVT_EVENT_TYPE_CONFIRM_V01 ||
      qmi_ind->event_type > CSVT_EVENT_TYPE_MODIFY_V01)
  {
    QMI_ERR_MSG_1("qmi_csvt_translate_ind: unknown ind=%d received", qmi_ind->event_type);
    goto bail;
  }

  /* Fill the mandatory parameters */
  /* Indication Type */
  csvt_ind->ind_type = (qmi_csvt_ind_type) qmi_ind->event_type;

  /* Instance ID */
  csvt_ind->instance_id = qmi_ind->instance_id;

  /* Initialize the optional parameter mask to 0 */
  csvt_ind->opt.param_mask = 0;

  /* Fill the optional parameters */
  /* Call Type */
  if (qmi_ind->call_type_valid)
  {
    csvt_ind->opt.param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_TYPE;
    csvt_ind->opt.call_type = (qmi_csvt_call_type) qmi_ind->call_type;
  }

  /* Modify Allowed */
  if (qmi_ind->modify_allowed_valid)
  {
    csvt_ind->opt.param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_MODIFY_ALLOWED;
    csvt_ind->opt.is_modify_allowed = (boolean) qmi_ind->modify_allowed;
  }

  /* Network Type */
  if (qmi_ind->network_type_valid)
  {
    csvt_ind->opt.param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_NETWORK_TYPE;
    csvt_ind->opt.network_type = (uint8) qmi_ind->network_type;
  }

  /* Network Speed */
  if (qmi_ind->network_speed_valid)
  {
    csvt_ind->opt.param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_NETWORK_SPEED;
    csvt_ind->opt.network_speed = (uint16) qmi_ind->network_speed;
  }

  /* Max Frame Size */
  if (qmi_ind->max_frame_size_valid)
  {
    csvt_ind->opt.param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_MAX_FRAME_SIZE;
    csvt_ind->opt.max_frame_size = (uint8) qmi_ind->max_frame_size;
  }

  /* Incoming Number */
  if (qmi_ind->incoming_number_valid)
  {
    csvt_ind->opt.param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_INCOMING_NUM;
    strlcpy(csvt_ind->opt.incoming_num,
            qmi_ind->incoming_number,
            sizeof(csvt_ind->opt.incoming_num));
  }

  if (qmi_ind->uus_id_valid)
  {
    csvt_ind->opt.param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_UUS_ID;

    /* UUS ID Type */
    csvt_ind->opt.uus_id.uus_id_type = (uint8) qmi_ind->uus_id.uus_id_type;

    /* UUS ID */
    strlcpy(csvt_ind->opt.uus_id.uus_id,
            qmi_ind->uus_id.uus_id,
            sizeof(csvt_ind->opt.uus_id.uus_id));
  }

  /* Modify Allowed */
  if (qmi_ind->modify_allowed_valid)
  {
    csvt_ind->opt.param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_MODIFY_ALLOWED;
    csvt_ind->opt.is_modify_allowed = (boolean) qmi_ind->modify_allowed;
  }

  /* Call End Cause*/
  if (qmi_ind->call_end_cause_valid)
  {
    csvt_ind->opt.param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_END_CAUSE;
    csvt_ind->opt.call_end_cause = (uint8) qmi_ind->call_end_cause;
  }

  /* Port Data */
  if (qmi_ind->port_data_valid)
  {
    csvt_ind->opt.param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_PORT_DATA;
    csvt_ind->opt.port_data.port_type = (qmi_csvt_port_type) qmi_ind->port_data.port_family;
    csvt_ind->opt.port_data.port_num  = qmi_ind->port_data.port_number;
  }

  rc = QMI_NO_ERR;

bail:
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_csvt_common_ind_hdlr
===========================================================================*/
/*!
@brief
  Common indication handler for all CSVT indications

@param
  user_handle - QMI client handle
  msg_id      - Message ID
  ind_buf     - Raw indication data buffer
  ind_buf_len - Indication data buffer length
  ind_cb_data - Callback data registered during init

@return
  None

@note
  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
static void
qmi_csvt_common_ind_hdlr
(
  qmi_client_type  user_handle,
  unsigned long    msg_id,
  unsigned char    *ind_buf,
  int              ind_buf_len,
  void             *ind_cb_data
)
{
  qmi_csvt_client_info_type       *info = (qmi_csvt_client_info_type *)ind_cb_data;
  qmi_csvt_ind_data_type          csvt_ind_data;
  csvt_event_report_ind_msg_v01   recv_ind_msg;
  csvt_originate_call_ind_msg_v01 req_status_ind_msg;
  int                             rc = QMI_INTERNAL_ERR;


  if (!user_handle || !ind_buf || !info)
  {
    QMI_ERR_MSG_0("qmi_csvt_common_ind_hdlr: bad param(s)");
    goto bail;
  }

  QMI_DEBUG_MSG_1("qmi_csvt_common_ind_hdlr: recvd ind=%lu", msg_id);

  if (NULL == info->ind_cb)
  {
    QMI_ERR_MSG_0("qmi_csvt_common_ind_hdlr: NULL indication cb");
    goto bail;
  }

  memset(&recv_ind_msg, 0, sizeof(recv_ind_msg));
  memset(&req_status_ind_msg, 0, sizeof(req_status_ind_msg));

  switch (msg_id)
  {
    case QMI_CSVT_ORIGINATE_CALL_IND_V01:
    case QMI_CSVT_CONFIRM_CALL_IND_V01:
    case QMI_CSVT_ANSWER_CALL_IND_V01:
    case QMI_CSVT_END_CALL_IND_V01:
    case QMI_CSVT_MODIFY_CALL_IND_V01:
    case QMI_CSVT_ACK_CALL_MODIFY_IND_V01:
      /* Decode the QMI indication message to its corresponding C structure */
      rc = qmi_client_message_decode(user_handle,
                                     QMI_IDL_INDICATION,
                                     msg_id,
                                     ind_buf,
                                     ind_buf_len,
                                     &req_status_ind_msg,
                                     sizeof(req_status_ind_msg));

      QMI_CSVT_MUTEX_LOCK(info->req_ind_mutex);
      info->req_status.status = req_status_ind_msg.request_status;

      if (req_status_ind_msg.call_end_cause_valid)
      {
        info->req_status.is_ce_reason_valid = TRUE;
        info->req_status.ce_reason = req_status_ind_msg.call_end_cause;
      }
      QMI_CSVT_MUTEX_UNLOCK(info->req_ind_mutex);

      QMI_DEBUG_MSG_1("qmi_csvt_common_ind_hdlr: recvd status=%d, signalling",
                      info->req_status);

      /* Inform the main thread to resume */
      pthread_cond_signal(&info->req_ind_condition);
      break;

    case QMI_CSVT_EVENT_REPORT_IND_V01:
      /* Decode the QMI indication message to its corresponding C structure */
      rc = qmi_client_message_decode(user_handle,
                                     QMI_IDL_INDICATION,
                                     msg_id,
                                     ind_buf,
                                     ind_buf_len,
                                     &recv_ind_msg,
                                     sizeof(recv_ind_msg));
      if (QMI_NO_ERR != rc)
      {
        QMI_ERR_MSG_1("qmi_csvt_common_ind_hdlr: indication decode failed err=%d", rc);
        goto bail;
      }

      memset(&csvt_ind_data, 0, sizeof(csvt_ind_data));

      rc = qmi_csvt_translate_ind(&recv_ind_msg, &csvt_ind_data);

      if (QMI_NO_ERR != rc)
      {
        QMI_ERR_MSG_1("qmi_csvt_common_ind_hdlr: failed to translate ind err=%d", rc);
        goto bail;
      }

      /* Call the client callback */
      if (info->ind_cb)
      {
        info->ind_cb(&csvt_ind_data, info->ind_cb_data);
      }
      else
      {
        QMI_ERR_MSG_1("qmi_csvt_common_ind_hdlr: invalid client ind_cb");
      }
      break;

    default:
      QMI_ERR_MSG_1("qmi_csvt_common_ind_hdlr: ignoring unknown indication=%d",
                    msg_id);
      break;
  }

bail:
  return;
}


/*===========================================================================
  FUNCTION  qmi_csvt_populate_originate_call_req
===========================================================================*/
/*!
@brief
  Populate the QMI call origination request

@param
  qmi_orig_req     - QMI call origination request
  csvt_orig_params - Call origination parameters provided by the client

@return
  None

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
static void
qmi_csvt_populate_originate_call_req
(
  csvt_originate_call_req_msg_v01  *qmi_orig_req,
  const qmi_csvt_call_params_type  *call_params
)
{
  if (!qmi_orig_req || !call_params)
  {
    QMI_ERR_MSG_0("qmi_csvt_populate_originate_call_req: bad param(s)");
    return;
  }

  /* Update the mandatory parameters */

  /* Instance ID */
  qmi_orig_req->instance_id = call_params->inst_id;

  /* Call mode */
  qmi_orig_req->call_mode = (csvt_call_mode_type_enum_v01) call_params->call_mode;

  /* Dial string*/
  strlcpy(qmi_orig_req->dial_string,
          call_params->dial_string,
          sizeof(qmi_orig_req->dial_string));

  /* Copy the optional parameters */

  /* Network Data Rate */
  if (call_params->param_mask & QMI_CSVT_CALL_PARAM_MASK_NETWORK_DATARATE)
  {
    qmi_orig_req->network_datarate_valid = TRUE;
    qmi_orig_req->network_datarate = (uint8_t) call_params->network_data_rate;
  }

  /* Air Interface Data Rate */
  if (call_params->param_mask & QMI_CSVT_CALL_PARAM_MASK_AIR_IFACE_DATARATE)
  {
    qmi_orig_req->air_interface_datarate_valid = TRUE;
    qmi_orig_req->air_interface_datarate = (uint8_t) call_params->air_iface_data_rate;
  }

  /* Synchronous Call */
  if (call_params->param_mask & QMI_CSVT_CALL_PARAM_MASK_SYNCHRONOUS_CALL)
  {
    qmi_orig_req->synchronous_call_valid = TRUE;
    qmi_orig_req->synchronous_call = (uint8_t) call_params->is_call_synchronous;
  }

  /* Transparent Call */
  if (call_params->param_mask & QMI_CSVT_CALL_PARAM_MASK_TRANSPARENT_CALL)
  {
    qmi_orig_req->transparent_call_valid = TRUE;
    qmi_orig_req->transparent_call = (uint8_t) call_params->is_call_transparent;
  }

  /* CLI Enabled */
  if (call_params->param_mask & QMI_CSVT_CALL_PARAM_MASK_CLI_ENABLED)
  {
    qmi_orig_req->cli_enabled_valid = TRUE;
    qmi_orig_req->cli_enabled = (uint8_t) call_params->is_cli_enabled;
  }

  /* CUG Enabled */
  if (call_params->param_mask & QMI_CSVT_CALL_PARAM_MASK_CUG_ENABLED)
  {
    qmi_orig_req->cug_enabled_valid = TRUE;
    qmi_orig_req->cug_enabled = (uint8_t) call_params->is_cug_enabled;
  }

  /* CUG Index */
  if (call_params->param_mask & QMI_CSVT_CALL_PARAM_MASK_CUG_INDEX)
  {
    qmi_orig_req->cug_index_valid = TRUE;
    qmi_orig_req->cug_index = (uint8_t) call_params->cug_index;
  }

  /* Supress Preferred CUG */
  if (call_params->param_mask & QMI_CSVT_CALL_PARAM_MASK_SUPRESS_PREF_CUG)
  {
    qmi_orig_req->supress_preferred_cug_valid = TRUE;
    qmi_orig_req->supress_preferred_cug = (uint8_t) call_params->supress_pref_cug;
  }

  /* Supress Preferred CUG */
  if (call_params->param_mask & QMI_CSVT_CALL_PARAM_MASK_SUPRESS_OUT_ACCESS)
  {
    qmi_orig_req->supress_outgoing_access_valid = TRUE;
    qmi_orig_req->supress_outgoing_access = (uint8_t) call_params->supress_out_access;
  }

  /* UUS ID Type Valid */
  if (call_params->param_mask & QMI_CSVT_CALL_PARAM_MASK_UUS_ID)
  {
    qmi_orig_req->uus_id_valid = TRUE;

    qmi_orig_req->uus_id.uus_id_type = (csvt_uus_id_type_enum_v01) call_params->uus_id.uus_id_type;

    strlcpy(qmi_orig_req->uus_id.uus_id,
            call_params->uus_id.uus_id,
            sizeof(qmi_orig_req->uus_id.uus_id));
  }
}


/*===========================================================================
  FUNCTION  qmi_csvt_translate_rlp_params_qmi_to_csvt
===========================================================================*/
/*!
@brief
  Translate RLP parameters received via QMI to the client's format

@param
  qmi_rlp  - RLP parameters from QMI
  csvt_rlp - RLP parameters provided to client

@return
  None

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
static void
qmi_csvt_translate_rlp_params_qmi_to_csvt
(
  const csvt_get_rlp_params_resp_msg_v01  *qmi_rlp,
  qmi_csvt_get_rlp_params_type            *csvt_rlp
)
{
  if (!qmi_rlp || !csvt_rlp)
  {
    QMI_ERR_MSG_0("qmi_csvt_translate_rlp_params_qmi_to_csvt: bad param(s)");
  }
  else
  {
    int i;

    /* Convert the RLP validity paramters into an array for easy access */
    uint8  qmi_rlp_params_valid[QMI_CSVT_MAX_RLP_PARAMS] =
    {
      qmi_rlp->rlp1_parameters_valid,
      qmi_rlp->rlp2_parameters_valid,
      qmi_rlp->rlp3_parameters_valid
    };

    /* Convert the RLP paramters into an array for easy access */
    const rlp_params_v01  *qmi_rlp_params[QMI_CSVT_MAX_RLP_PARAMS] =
    {
      &qmi_rlp->rlp1_parameters,
      &qmi_rlp->rlp2_parameters,
      &qmi_rlp->rlp3_parameters
    };

    uint16  csvt_param_mask[QMI_CSVT_MAX_RLP_PARAMS] =
    {
      QMI_CSVT_RLP_PARAMS_V1_VALID_PARAM_MASK,
      QMI_CSVT_RLP_PARAMS_V2_VALID_PARAM_MASK,
      QMI_CSVT_RLP_PARAMS_V3_VALID_PARAM_MASK
    };

    qmi_csvt_rlp_params_type  *csvt_params[QMI_CSVT_MAX_RLP_PARAMS] =
    {
      &csvt_rlp->rlp_params_v1,
      &csvt_rlp->rlp_params_v2,
      &csvt_rlp->rlp_params_v3
    };

    /* Store the RLP parameters */
    for (i = 0; i < QMI_CSVT_MAX_RLP_PARAMS; ++i)
    {
      if (qmi_rlp_params_valid[i])
      {
        csvt_rlp->param_mask |= csvt_param_mask[i];

        /* Version */
        csvt_params[i]->version          = qmi_rlp_params[i]->rlp_version;

        /* TX Window Size */
        csvt_params[i]->tx_window_size   = qmi_rlp_params[i]->rlp_tx_window_size;

        /* RX Window Size */
        csvt_params[i]->rx_window_size   = qmi_rlp_params[i]->rlp_rx_window_size;

        /* Ack Timer */
        csvt_params[i]->ack_timer        = qmi_rlp_params[i]->rlp_ack_timer;

        /* Retransmission Attempts */
        csvt_params[i]->retrans_attempts = qmi_rlp_params[i]->rlp_retrans_attempts;

        /* Resequencing Timer */
        csvt_params[i]->reseq_timer      = qmi_rlp_params[i]->rlp_reseq_timer;
      }
    }
  }
}


/*===========================================================================
  FUNCTION  qmi_csvt_translate_v42_params_qmi_to_csvt
===========================================================================*/
/*!
@brief
  Translate RLP parameters received via QMI to the client's format

@param
  qmi_rlp  - RLP parameters from QMI
  csvt_rlp - RLP parameters provided to client

@return
  None

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
static void
qmi_csvt_translate_v42_params_qmi_to_csvt
(
  const csvt_get_rlp_params_resp_msg_v01  *qmi_rlp,
  qmi_csvt_get_rlp_params_type            *csvt_rlp
)
{
  if (!qmi_rlp || !csvt_rlp)
  {
    QMI_ERR_MSG_0("qmi_csvt_translate_v42_params_qmi_to_csvt: bad param(s)");
    return;
  }

  /* V42 Parameters */
  if (qmi_rlp->v42_parameters_valid)
  {
    csvt_rlp->param_mask |= QMI_CSVT_V42_PARAMS_VALID_PARAM_MASK;

    /* Direction */
    csvt_rlp->v42_params.direction        = qmi_rlp->v42_parameters.v42_direction;

    /* Negotiation Preference */
    csvt_rlp->v42_params.negotiation_pref = qmi_rlp->v42_parameters.v42_negotiation;

    /* Max Dictionary Size */
    csvt_rlp->v42_params.max_dict_size    = qmi_rlp->v42_parameters.v42_max_dict;

    /* Max String Size */
    csvt_rlp->v42_params.max_str_size     = qmi_rlp->v42_parameters.v42_max_string;
  }
}


/*===========================================================================
  FUNCTION  qmi_csvt_populate_set_rlp_params_req
===========================================================================*/
/*!
@brief
  Populate the QMI_CSVT_SET_RLP_PARAMETERS request message parameters

@param
  qmi_req        - QMI request message
  set_rlp_params - Client provided RLP & V42 parameters

@return
  None

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
static void
qmi_csvt_populate_set_rlp_params_req
(
  csvt_set_rlp_params_req_msg_v01     *qmi_req,
  const qmi_csvt_set_rlp_params_type  *set_rlp_params
)
{
  if (!qmi_req || !set_rlp_params)
  {
    QMI_ERR_MSG_0("qmi_csvt_populate_set_rlp_params_req: bad param(s)");
    return;
  }

  /* Valid RLP parameters */
  if (set_rlp_params->param_mask & QMI_CSVT_RLP_PARAMS_VALID_PARAM_MASK)
  {
    qmi_req->rlp_parameters_valid = TRUE;

    /* Version */
    qmi_req->rlp_parameters.rlp_version          = set_rlp_params->rlp_params.version;

    /* TX Window Size */
    qmi_req->rlp_parameters.rlp_tx_window_size   = set_rlp_params->rlp_params.tx_window_size;

    /* RX Window Size */
    qmi_req->rlp_parameters.rlp_rx_window_size   = set_rlp_params->rlp_params.rx_window_size;

    /* Ack Timer */
    qmi_req->rlp_parameters.rlp_ack_timer        = set_rlp_params->rlp_params.ack_timer;

    /* Retransmission Attempts */
    qmi_req->rlp_parameters.rlp_retrans_attempts = set_rlp_params->rlp_params.retrans_attempts;

    /* Resequencing Timer */
    qmi_req->rlp_parameters.rlp_reseq_timer      = set_rlp_params->rlp_params.reseq_timer;
  }

  /* Valid V42 parameters */
  if (set_rlp_params->param_mask & QMI_CSVT_V42_PARAMS_VALID_PARAM_MASK)
  {
    qmi_req->v42_parameters_valid = TRUE;

    /* Compression Direction */
    qmi_req->v42_parameters.v42_direction   = (uint32_t) set_rlp_params->v42_params.direction;

    /* Compression Negotiation Preference */
    qmi_req->v42_parameters.v42_negotiation = (uint32_t) set_rlp_params->v42_params.negotiation_pref;

    /* Max Dictionary Size */
    qmi_req->v42_parameters.v42_max_dict    = set_rlp_params->v42_params.max_dict_size;

    /* Max String Size */
    qmi_req->v42_parameters.v42_max_string  = set_rlp_params->v42_params.max_str_size;
  }
}


/*===========================================================================
  FUNCTION  qmi_csvt_wait_for_request_status
===========================================================================*/
/*!
@brief
  Populate the QMI_CSVT_SET_RLP_PARAMETERS request message parameters

@param
  info         - QMI request message
  qmi_err_code - Client provided RLP & V42 parameters

@return
  QMI_NO_ERR
  QMI_SERVICE_ERR

@note

  - Dependencies
    - info->req_ind_mutex must be locked by the caller.

  - Side Effects
    - None.
*/
/*=========================================================================*/
static int
qmi_csvt_wait_for_request_status
(
  qmi_csvt_client_info_type  *info,
  int                        *qmi_err_code
)
{
  int              rc = QMI_SERVICE_ERR;
  int              ret;
  struct timeval   curr_time;
  struct timespec  timeout;


  if (!info || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_wait_for_req_status: bad param(s)");
    return rc;
  }

  QMI_DEBUG_MSG_0("qmi_csvt_wait_for_req_status: waiting for status indication");

  /* Set wait time seconds to current + the number of seconds needed for timeout */
  gettimeofday(&curr_time, NULL);
  timeout.tv_sec  = curr_time.tv_sec + QMI_CSVT_MAX_SYNC_MSG_TIMEOUT;
  timeout.tv_nsec = curr_time.tv_usec * 1000;

  /* Wait for the originate call indication to arrive */
  while (QMI_CSVT_REQUEST_STATUS_INVALID == info->req_status.status)
  {
    ret = pthread_cond_timedwait(&info->req_ind_condition,
                                 &info->req_ind_mutex,
                                 &timeout);

    /* In case of failure return error */
    if (ret < 0)
    {
      QMI_ERR_MSG_2("qmi_csvt_wait_for_req_status: pthread_cond_timedwait failed errno=[%d:%s]",
                    errno,
                    strerror(errno));

      info->req_status.status = QMI_CSVT_REQUEST_STATUS_FAILED;
    }
  }

  if (QMI_CSVT_REQUEST_STATUS_SUCCESS == info->req_status.status)
  {
    rc = QMI_NO_ERR;
  }
  else
  {
    rc = QMI_SERVICE_ERR;

    if (info->req_status.is_ce_reason_valid)
    {
      QMI_DEBUG_MSG_1("qmi_csvt_wait_for_req_status: returning qmi_err_code=%d",
                      *qmi_err_code);
      *qmi_err_code = info->req_status.ce_reason;;
    }
    else
    {
      *qmi_err_code = QMI_SERVICE_ERR_INTERNAL;
    }
  }

  QMI_DEBUG_MSG_1("qmi_csvt_wait_for_req_status: returning rc=%d", rc);

  return rc;
}


/*===========================================================================
                          GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qmi_csvt_init_client
===========================================================================*/
/*!
@brief
  Initialize a CSVT client. This function should be called before calling
  other API functions.

@param
  conn_id     - Connection ID
  ind_cb      - Callback function to be called when an indication is received
  ind_cb_data - User data to be provided during callback

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
)
{
  int                        rc = QMI_NO_ERR;
  qmi_csvt_client_info_type  *info = NULL;


  if (!conn_id || !ind_cb)
  {
    QMI_ERR_MSG_0("qmi_csvt_init_client: bad param(s)");
    goto bail;
  }

  if (NULL == (info = malloc(sizeof(qmi_csvt_client_info_type))))
  {
    QMI_ERR_MSG_0("qmi_csvt_init_client: failed to alloc memory");
    goto bail;
  }

  /* Initialize the client info data */
  memset(info, 0, sizeof(qmi_csvt_client_info_type));

  info->csvt_srvc_obj = csvt_get_service_object_v01();

  rc = qmi_client_init(conn_id,
                       info->csvt_srvc_obj,
                       qmi_csvt_common_ind_hdlr,
                       (void *)info,
                       &info->qmi_clnt_hndl);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_init_client: qmi_client_init failed=%d", rc);
    goto bail;
  }

  /* Initialize instance_id to invalid */
  info->inst_id = QMI_CSVT_INVALID_INST_ID;

  /* Save the client indication callback info */
  info->ind_cb      = ind_cb;
  info->ind_cb_data = ind_cb_data;

  /* Initialize the indication condition variable and mutex */
  pthread_mutex_init(&info->req_ind_mutex, NULL);
  pthread_cond_init(&info->req_ind_condition, NULL);

  memset(&info->req_status, 0, sizeof(info->req_status));

  info->req_status.status = QMI_CSVT_REQUEST_STATUS_INVALID;

  return info;

bail:
  free(info);
  return QMI_CSVT_INVALID_HNDL;
}


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
)
{
  int                        rc = QMI_INTERNAL_ERR;
  qmi_csvt_client_info_type  *info = (qmi_csvt_client_info_type *) csvt_hndl;


  if (!info)
  {
    QMI_ERR_MSG_0("qmi_csvt_release_client: bad param(s)");
    goto bail;
  }

  rc = qmi_client_release(info->qmi_clnt_hndl);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_release_client: qmi_client_release failed err=%d", rc);
  }

  pthread_mutex_destroy(&info->req_ind_mutex);
  pthread_cond_destroy(&info->req_ind_condition);

  /* Free the client info structure previously allocated */
  free(info);

bail:
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_csvt_reset
===========================================================================*/
/*!
@brief
  Reset the CSVT service state variables of the requesting control point.

@param
  clnt_hndl    - CSVT client handle
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
qmi_csvt_reset
(
  qmi_csvt_clnt_hndl  clnt_hndl,
  int                 *qmi_err_code
)
{
  int                        rc = QMI_INTERNAL_ERR;
  csvt_reset_resp_msg_v01    resp_msg;
  qmi_csvt_client_info_type  *info = (qmi_csvt_client_info_type *) clnt_hndl;


  if (!info || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_reset: bad param(s)");
    goto bail;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&resp_msg, 0, sizeof(resp_msg));

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_RESET_REQ_V01,
                                NULL,
                                0,
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);


  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_reset: failed send_msg_sync err=%d",
                  rc);
    goto bail;
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_reset: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
    goto bail;
  }

bail:
  return rc;
}


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
)
{
  csvt_set_event_report_req_msg_v01   req_msg;
  csvt_set_event_report_resp_msg_v01  resp_msg;
  qmi_csvt_client_info_type  *info = (qmi_csvt_client_info_type *) clnt_hndl;
  int                        rc = QMI_INTERNAL_ERR;


  if (!info || !event_params || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_set_event_report: bad param(s)");
    goto bail;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&req_msg, 0, sizeof(req_msg));
  memset(&resp_msg, 0, sizeof(resp_msg));

  if (event_params->param_mask & QMI_CSVT_EVT_REPORT_CALL_EVENTS_PARAM_MASK)
  {
    req_msg.report_call_events_valid = TRUE;
    req_msg.report_call_events = (uint8_t) event_params->report_call_events;
  }

  if (event_params->param_mask & QMI_CSVT_EVT_REPORT_CALL_TYPE_MASK_PARAM_MASK)
  {
    req_msg.call_types_valid = TRUE;
    req_msg.call_types       = (csvt_data_call_type_mask_v01) event_params->call_type_mask;
  }

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_SET_EVENT_REPORT_REQ_V01,
                                &req_msg,
                                sizeof(req_msg),
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_set_event_report: failed send_msg_sync err=%d",
                  rc);
    goto bail;
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_set_event_report: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
    goto bail;
  }

bail:
  return rc;
}


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
)
{
  int                               rc = QMI_INTERNAL_ERR;
  csvt_originate_call_req_msg_v01   req_msg;
  csvt_originate_call_resp_msg_v01  resp_msg;
  qmi_csvt_client_info_type         *info = (qmi_csvt_client_info_type *) clnt_hndl;
  qmi_txn_handle                    txn_hndl;


  if (!info || !call_params || QMI_CSVT_INVALID_INST_ID == call_params->inst_id || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_originate_call: bad param(s)");
    return rc;
  }

  /* Set output params to default values */
  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&req_msg, 0, sizeof(req_msg));
  memset(&resp_msg, 0, sizeof(resp_msg));

  qmi_csvt_populate_originate_call_req(&req_msg, call_params);

  QMI_CSVT_MUTEX_LOCK(info->req_ind_mutex);

  memset(&info->req_status, 0, sizeof(info->req_status));

  info->req_status.status = QMI_CSVT_REQUEST_STATUS_INVALID;

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_ORIGINATE_CALL_REQ_V01,
                                &req_msg,
                                sizeof(req_msg),
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_originate_call: failed send_msg_sync err=%d",
                  rc);
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_originate_call: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
  }
  else
  {
    rc = qmi_csvt_wait_for_request_status(info, qmi_err_code);
  }

  QMI_CSVT_MUTEX_UNLOCK(info->req_ind_mutex);

  return rc;
}


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
  confirm_call - Confirm or Reject call
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
)
{
  int                             rc = QMI_INTERNAL_ERR;
  csvt_confirm_call_req_msg_v01   req_msg;
  csvt_confirm_call_resp_msg_v01  resp_msg;
  qmi_csvt_client_info_type       *info = (qmi_csvt_client_info_type *) clnt_hndl;


  if (!info || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_confirm_call: bad param(s)");
    return rc;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&req_msg, 0, sizeof(req_msg));
  memset(&resp_msg, 0, sizeof(resp_msg));

  req_msg.instance_id  = inst_id;
  req_msg.confirm_call = (uint8_t) confirm_call;

  QMI_CSVT_MUTEX_LOCK(info->req_ind_mutex);

  memset(&info->req_status, 0, sizeof(info->req_status));

  info->req_status.status = QMI_CSVT_REQUEST_STATUS_INVALID;

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_CONFIRM_CALL_REQ_V01,
                                &req_msg,
                                sizeof(req_msg),
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_confirm_call: failed send_msg_sync err=%d",
                  rc);
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_confirm_call: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
  }
  else
  {
    rc = qmi_csvt_wait_for_request_status(info, qmi_err_code);
  }

  QMI_CSVT_MUTEX_UNLOCK(info->req_ind_mutex);

  return rc;
}


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
  answer_call  - Answer or Reject call
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
)
{
  int                            rc = QMI_INTERNAL_ERR;
  csvt_answer_call_req_msg_v01   req_msg;
  csvt_answer_call_resp_msg_v01  resp_msg;
  qmi_csvt_client_info_type      *info = (qmi_csvt_client_info_type *) clnt_hndl;


  if (!info || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_answer_call: bad param(s)");
    return rc;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&req_msg, 0, sizeof(req_msg));
  memset(&resp_msg, 0, sizeof(resp_msg));

  req_msg.instance_id = inst_id;
  req_msg.answer_call = (uint8_t) answer_call;

  QMI_CSVT_MUTEX_LOCK(info->req_ind_mutex);

  memset(&info->req_status, 0, sizeof(info->req_status));

  info->req_status.status = QMI_CSVT_REQUEST_STATUS_INVALID;

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_ANSWER_CALL_REQ_V01,
                                &req_msg,
                                sizeof(req_msg),
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_answer_call: failed send_msg_sync err=%d",
                  rc);
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_answer_call: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
  }
  else
  {
    rc = qmi_csvt_wait_for_request_status(info, qmi_err_code);
  }

  QMI_CSVT_MUTEX_UNLOCK(info->req_ind_mutex);

  return rc;
}


/*===========================================================================
  FUNCTION  qmi_csvt_end_call
===========================================================================*/
/*!
@brief
  This message ends an ongoing CSVT call that had been confirmed
  earlier by this control point.

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
)
{
  int                         rc = QMI_INTERNAL_ERR;
  csvt_end_call_req_msg_v01   req_msg;
  csvt_end_call_resp_msg_v01  resp_msg;
  qmi_csvt_client_info_type   *info = (qmi_csvt_client_info_type *) clnt_hndl;


  if (!info || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_end_call: bad param(s)");
    return rc;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&req_msg, 0, sizeof(req_msg));
  memset(&resp_msg, 0, sizeof(resp_msg));

  req_msg.instance_id = inst_id;

  QMI_CSVT_MUTEX_LOCK(info->req_ind_mutex);

  memset(&info->req_status, 0, sizeof(info->req_status));

  info->req_status.status = QMI_CSVT_REQUEST_STATUS_INVALID;

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_END_CALL_REQ_V01,
                                &req_msg,
                                sizeof(req_msg),
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_end_call: failed send_msg_sync err=%d",
                  rc);
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_end_call: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
  }
  else
  {
    rc = qmi_csvt_wait_for_request_status(info, qmi_err_code);
  }

  QMI_CSVT_MUTEX_UNLOCK(info->req_ind_mutex);

  return rc;
}


/*===========================================================================
  FUNCTION  qmi_csvt_modify_call
===========================================================================*/
/*!
@brief
  This message allows the control point to modify an existing call or to accept
  or reject a modification initiated by the network.

@param
  clnt_hndl    - QMI CSVT client handle
  modify_params - Call parameters to modify
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
qmi_csvt_modify_call
(
  qmi_csvt_clnt_hndl                      clnt_hndl,
  const qmi_csvt_modify_call_params_type  *modify_params,
  int                                     *qmi_err_code
)
{
  int                            rc = QMI_INTERNAL_ERR;
  csvt_modify_call_req_msg_v01   req_msg;
  csvt_modify_call_resp_msg_v01  resp_msg;
  qmi_csvt_client_info_type      *info = (qmi_csvt_client_info_type *) clnt_hndl;


  if (!info || !modify_params || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_modify_call: bad param(s)");
    return rc;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&req_msg, 0, sizeof(req_msg));
  memset(&resp_msg, 0, sizeof(resp_msg));

  /* Update the mandatory parameters */
  req_msg.instance_id = modify_params->instance_id;
  req_msg.new_call_type = (csvt_data_call_type_enum_v01) modify_params->new_call_type;

  QMI_CSVT_MUTEX_LOCK(info->req_ind_mutex);

  memset(&info->req_status, 0, sizeof(info->req_status));

  info->req_status.status = QMI_CSVT_REQUEST_STATUS_INVALID;

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_MODIFY_CALL_REQ_V01,
                                &req_msg,
                                sizeof(req_msg),
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_modify_call: failed send_msg_sync err=%d",
                  rc);
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_modify_call: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
  }
  else
  {
    rc = qmi_csvt_wait_for_request_status(info, qmi_err_code);
  }

  QMI_CSVT_MUTEX_UNLOCK(info->req_ind_mutex);

  return rc;
}


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
)
{
  int                               rc = QMI_INTERNAL_ERR;
  csvt_ack_call_modify_req_msg_v01  req_msg;
  csvt_ack_call_modify_resp_msg_v01 resp_msg;
  qmi_csvt_client_info_type         *info = (qmi_csvt_client_info_type *) clnt_hndl;


  if (!info || !ack_modify_params || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_modify_call: bad param(s)");
    return rc;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&req_msg, 0, sizeof(req_msg));
  memset(&resp_msg, 0, sizeof(resp_msg));

  /* Update the mandatory parameters */
  req_msg.instance_id = ack_modify_params->instance_id;
  req_msg.accept_request = (uint8_t) ack_modify_params->accept_req;

  QMI_CSVT_MUTEX_LOCK(info->req_ind_mutex);

  memset(&info->req_status, 0, sizeof(info->req_status));

  info->req_status.status = QMI_CSVT_REQUEST_STATUS_INVALID;

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_ACK_CALL_MODIFY_REQ_V01,
                                &req_msg,
                                sizeof(req_msg),
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_ack_call_modify: failed send_msg_sync err=%d",
                  rc);
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_ack_call_modify: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
  }
  else
  {
    rc = qmi_csvt_wait_for_request_status(info, qmi_err_code);
  }

  QMI_CSVT_MUTEX_UNLOCK(info->req_ind_mutex);

  return rc;
}


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
)
{
  int                               rc = QMI_INTERNAL_ERR;
  csvt_get_rlp_params_resp_msg_v01  resp_msg;
  qmi_csvt_client_info_type         *info = (qmi_csvt_client_info_type *) clnt_hndl;


  if (!info || !rlp_params || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_get_rlp_params: bad param(s)");
    goto bail;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&resp_msg, 0, sizeof(resp_msg));

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_GET_RLP_PARAMS_REQ_V01,
                                NULL,
                                0,
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_get_rlp_params: failed send_msg_sync err=%d",
                  rc);
    goto bail;
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_get_rlp_params: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
    goto bail;
  }

  /* Initialize the output parameter */
  memset(rlp_params, 0, sizeof(qmi_csvt_get_rlp_params_type));

  /* Translate RLP parameters */
  qmi_csvt_translate_rlp_params_qmi_to_csvt(&resp_msg, rlp_params);

  /* Translate V42 parameters */
  qmi_csvt_translate_v42_params_qmi_to_csvt(&resp_msg, rlp_params);

bail:
  return rc;
}

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
)
{
  int                               rc = QMI_INTERNAL_ERR;
  csvt_set_rlp_params_req_msg_v01   req_msg;
  csvt_set_rlp_params_resp_msg_v01  resp_msg;
  qmi_csvt_client_info_type         *info = (qmi_csvt_client_info_type *) clnt_hndl;


  if (!info || !rlp_params || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_set_rlp_params: bad param(s)");
    goto bail;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&req_msg, 0, sizeof(req_msg));
  memset(&resp_msg, 0, sizeof(resp_msg));

  /* Populate the parameters in the request message */
  qmi_csvt_populate_set_rlp_params_req(&req_msg, rlp_params);

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_SET_RLP_PARAMS_REQ_V01,
                                &req_msg,
                                sizeof(req_msg),
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_set_rlp_params: failed send_msg_sync err=%d",
                  rc);
    goto bail;
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_set_rlp_params: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
    goto bail;
  }

bail:
  return rc;
}


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
)
{
  int                                    rc = QMI_INTERNAL_ERR;
  csvt_get_active_call_list_resp_msg_v01 resp_msg;
  qmi_csvt_client_info_type              *info = (qmi_csvt_client_info_type *) clnt_hndl;
  uint32                                 output_list_len, i;


  if (!active_call_list || !active_call_list_len || 0 == *active_call_list_len || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_get_active_call_list: bad param(s)");
    goto bail;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&resp_msg, 0, sizeof(resp_msg));

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_GET_ACTIVE_CALL_LIST_REQ_V01,
                                NULL,
                                0,
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_get_active_call_list: failed send_msg_sync err=%d",
                  rc);
    goto bail;
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_get_active_call_list: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
    goto bail;
  }

  output_list_len = *active_call_list_len;

  output_list_len = (output_list_len > resp_msg.active_call_inst_id_len) ?
                     resp_msg.active_call_inst_id_len :
                     output_list_len;

  for (i = 0; i < output_list_len; ++i)
  {
    active_call_list[i] = resp_msg.active_call_inst_id[i];
  }

  *active_call_list_len = output_list_len;

bail:
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_csvt_get_call_info
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
)
{
  int                              rc = QMI_INTERNAL_ERR;
  csvt_get_call_info_req_msg_v01   req_msg;
  csvt_get_call_info_resp_msg_v01  resp_msg;
  qmi_csvt_client_info_type        *info = (qmi_csvt_client_info_type *) clnt_hndl;


  if (!call_info || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_get_call_info: bad param(s)");
    goto bail;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&req_msg, 0, sizeof(req_msg));
  memset(&resp_msg, 0, sizeof(resp_msg));

  req_msg.instance_id = inst_id;

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_GET_CALL_INFO_REQ_V01,
                                &req_msg,
                                sizeof(req_msg),
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_get_call_info: failed send_msg_sync err=%d",
                  rc);
    goto bail;
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_get_call_info: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
    goto bail;
  }

  /* Initialize the optional parameter mask to 0 */
  call_info->param_mask = 0;

  /* Fill the optional parameters */
  /* Call Type */
  if (resp_msg.call_type_valid)
  {
    call_info->param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_TYPE;
    call_info->call_type = (qmi_csvt_call_type) resp_msg.call_type;
  }

  /* Modify Allowed */
  if (resp_msg.modify_allowed_valid)
  {
    call_info->param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_MODIFY_ALLOWED;
    call_info->is_modify_allowed = (boolean) resp_msg.modify_allowed;
  }

  /* Network Type */
  if (resp_msg.network_type_valid)
  {
    call_info->param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_NETWORK_TYPE;
    call_info->network_type = (uint8) resp_msg.network_type;
  }

  /* Network Speed */
  if (resp_msg.network_speed_valid)
  {
    call_info->param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_NETWORK_SPEED;
    call_info->network_speed = (uint16) resp_msg.network_speed;
  }

  /* Max Frame Size */
  if (resp_msg.max_frame_size_valid)
  {
    call_info->param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_MAX_FRAME_SIZE;
    call_info->max_frame_size = (uint8) resp_msg.max_frame_size;
  }

  /* Incoming Number */
  if (resp_msg.incoming_number_valid)
  {
    call_info->param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_INCOMING_NUM;
    strlcpy(call_info->incoming_num,
            resp_msg.incoming_number,
            sizeof(call_info->incoming_num));
  }

  if (resp_msg.uus_id_valid)
  {
    call_info->param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_UUS_ID;

    /* UUS ID Type */
    call_info->uus_id.uus_id_type = (uint8) resp_msg.uus_id.uus_id_type;

    /* UUS ID */
    strlcpy(call_info->uus_id.uus_id,
            resp_msg.uus_id.uus_id,
            sizeof(call_info->uus_id.uus_id));
  }

  /* Modify Allowed */
  if (resp_msg.modify_allowed_valid)
  {
    call_info->param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_MODIFY_ALLOWED;
    call_info->is_modify_allowed = (boolean) resp_msg.modify_allowed;
  }

  /* Call End Cause*/
  if (resp_msg.call_end_cause_valid)
  {
    call_info->param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_END_CAUSE;
    call_info->call_end_cause = (uint8) resp_msg.call_end_cause;
  }

  /* Port Data */
  if (resp_msg.port_data_valid)
  {
    call_info->param_mask |= QMI_CSVT_CALL_INFO_PARAM_MASK_CALL_PORT_DATA;
    call_info->port_data.port_type = (qmi_csvt_port_type) resp_msg.port_data.port_family;
    call_info->port_data.port_num  = resp_msg.port_data.port_number;
  }

bail:
  return rc;
}


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
)
{
  int                               rc = QMI_INTERNAL_ERR;
  csvt_get_call_stats_req_msg_v01   req_msg;
  csvt_get_call_stats_resp_msg_v01  resp_msg;
  qmi_csvt_client_info_type         *info = (qmi_csvt_client_info_type *) clnt_hndl;


  if (!info || !call_stats || !qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_csvt_get_call_stats: bad param(s)");
    goto bail;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  memset(&req_msg, 0, sizeof(req_msg));
  memset(&resp_msg, 0, sizeof(resp_msg));

  /* Initialize output param to invalid */
  memset(call_stats, 0, sizeof(qmi_csvt_call_stats_type));

  /* Fill the request message params */
  req_msg.instance_id = inst_id;

  rc = qmi_client_send_msg_sync(info->qmi_clnt_hndl,
                                QMI_CSVT_GET_CALL_STATS_REQ_V01,
                                &req_msg,
                                sizeof(req_msg),
                                &resp_msg,
                                sizeof(resp_msg),
                                QMI_CSVT_SYNC_MSG_TIMEOUT);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_csvt_get_call_stats: failed send_msg_sync err=%d",
                  rc);
    goto bail;
  }
  else if (QMI_NO_ERR != resp_msg.resp.result)
  {
    QMI_ERR_MSG_1("qmi_csvt_get_call_stats: failed response err=%d",
                  resp_msg.resp.error);

    rc = QMI_SERVICE_ERR;
    *qmi_err_code = resp_msg.resp.error;
    goto bail;
  }

  /* Update the call stats */

  /* Call Active */
  if (resp_msg.call_active_valid)
  {
    call_stats->param_mask |= QMI_CSVT_CALL_STATS_CALL_ACTIVE_PARAM_MASK;
    call_stats->is_call_active = resp_msg.call_active;
  }

  /* TX Counter */
  if (resp_msg.tx_counter_valid)
  {
    call_stats->param_mask |= QMI_CSVT_CALL_STATS_TX_COUNTER_PARAM_MASK;
    call_stats->tx_counter = resp_msg.tx_counter;
  }

  /* RX Counter */
  if (resp_msg.rx_counter_valid)
  {
    call_stats->param_mask |= QMI_CSVT_CALL_STATS_RX_COUNTER_PARAM_MASK;
    call_stats->rx_counter = resp_msg.rx_counter;
  }

bail:
  return rc;
}

