/******************************************************************************
  @file    qcril_qmi_ims.c
  @brief   qcril qmi - ims

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI IMS.

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <errno.h>
#include <cutils/memory.h>
#include <cutils/properties.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "ril.h"
#include "IxErrno.h"
#include "comdef.h"
#include "qcrili.h"
#include "qcril_reqlist.h"
#include "qcril_arb.h"
#include "qcril_qmi_client.h"
#include "qcril_cmi.h"
#include "qcril_qmi_ims.h"

/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/
#define QCRIL_QMI_IMS_ERROR -1

/* Macro to retrieve the result of QMI Service Request */
#define QMI_IMS_OP_RESULT( rc, response )                                              \
  ( ( rc != QMI_NO_ERR ) ? rc :                                                          \
                           ( ( ( response == NULL ) ||                                   \
                               ( response->resp.result == QMI_RESULT_SUCCESS_V01 ) ||    \
                               ( ( response->resp.result == QMI_RESULT_FAILURE_V01 ) &&  \
                                 ( response->resp.error == QMI_ERR_NO_EFFECT_V01 ) )     \
                             ) ? QMI_NO_ERR : response->resp.result ) )

static unsigned int call_id;
/*===========================================================================

                    QMI VOICE GLOBALS

===========================================================================*/
static qcril_qmi_ims_info_type  qcril_qmi_ims_info;

/*===========================================================================

                                FUNCTIONS

===========================================================================*/

void qcril_qmi_ims_init
(
  void
)
{
  imsp_set_event_report_req_v01  indication_req;
  imsp_set_event_report_resp_v01 indication_resp;

  indication_req.event_report_bit_masks= IMSP_ENABLE_PUBLISH_TRIGGER_IND_V01 |
                                           IMSP_ENABLE_ENABLER_STATE_IND_V01 |
                                           IMSP_ENABLE_NOTIFY_IND_V01;

  if ( qcril_qmi_client_send_msg_sync( QCRIL_QMI_CLIENT_IMS_PRESENCE,
                                      QMI_IMSP_SET_EVENT_REPORT_REQ_V01,
                                      &indication_req,
                                      sizeof(indication_req),
                                      &indication_resp,
                                      sizeof(indication_resp)
                                    ) !=E_SUCCESS )
  {
    QCRIL_LOG_INFO("Presence Indication registeration failed!");
  }
  else
  {
    QCRIL_LOG_INFO("Presence Indication register successful");
  }

}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_vt_lookup_command_name

===========================================================================*/
/*!
    @brief
    Lookup state name.

    @return
    A pointer to the state name.
*/
/*=========================================================================*/
static char *qcril_qmi_ims_vt_lookup_command_name
(
  unsigned long msg
)
{
  switch( msg )
  {
    case IMS_VT_DIAL_CALL_RESP_V01:
      return "VT_DIAL_CALL_RESP";

    case IMS_VT_ANSWER_CALL_RESP_V01:
      return "VT_ANSWER_CALL_RESP";

    case IMS_VT_END_CALL_RESP_V01:
      return "VT_END_CALL_RESP";

    default:
      return "Unknown";
  } /* end switch */
} /* qcril_qmi_ims_vt_lookup_command_name */


/*===========================================================================

  FUNCTION:  qcril_qmi_ims_presence_lookup_command_name

===========================================================================*/
/*!
    @brief
    Lookup state name.

    @return
    A pointer to the state name.
*/
/*=========================================================================*/
static char *qcril_qmi_ims_presence_lookup_command_name
(
  unsigned long msg
)
{
  switch( msg )
  {
    case QMI_IMSP_GET_ENABLER_STATE_RESP_V01:
      return "QMI_IMSP_GET_ENABLER_STATE_RESP_V01";

    case QMI_IMSP_SEND_PUBLISH_RESP_V01:
      return "QMI_IMSP_SEND_PUBLISH_RESP_V01";

    case QMI_IMSP_SEND_PUBLISH_XML_REQ_V01:
      return "QMI_IMSP_SEND_PUBLISH_XML_REQ_V01";

    case QMI_IMSP_SEND_UNPUBLISH_RESP_V01:
      return "QMI_IMSP_SEND_UNPUBLISH_RESP_V01";

    case QMI_IMSP_SEND_SUBSCRIBE_RESP_V01:
      return "QMI_IMSP_SEND_SUBSCRIBE_RESP_V01";

    case QMI_IMSP_SEND_SUBSCRIBE_XML_RESP_V01:
      return "QMI_IMSP_SEND_SUBSCRIBE_XML_RESP_V01";

    case QMI_IMSP_SEND_UNSUBSCRIBE_RESP_V01:
      return "QMI_IMSP_SEND_UNSUBSCRIBE_RESP_V01";

    case QMI_IMSP_SET_NOTIFY_FMT_RESP_V01:
      return "QMI_IMSP_SET_NOTIFY_FMT_RESP_V01";

    case QMI_IMSP_GET_NOTIFY_FMT_RESP_V01:
      return "QMI_IMSP_GET_NOTIFY_FMT_RESP_V01";

    case QMI_IMSP_SET_EVENT_REPORT_RESP_V01:
      return "QMI_IMSP_SET_EVENT_REPORT_RESP_V01";

    case QMI_IMSP_GET_EVENT_REPORT_RESP_V01:
      return "QMI_IMSP_GET_EVENT_REPORT_RESP_V01";

    default:
      return "Unknown";
  } /* end switch */
} /* qcril_qmi_ims_presence_lookup_command_name */


/*=========================================================================
  FUNCTION:  qcril_qmi_ims_vt_call_status_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_HOOK_VT_UNSOL_CALL_STATUS_IND.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_vt_call_status_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  ims_vt_call_status_ind_v01 *call_status_ind;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  if( ind_data_ptr != NULL )
  {
    call_status_ind = (ims_vt_call_status_ind_v01*)ind_data_ptr;

    QCRIL_LOG_INFO("%s entered", __FUNCTION__ );

    call_status_ind->call_info.call_type = CALL_TYPE_VT_V01;

    if( call_status_ind->remote_party_name_valid )
    {
      QCRIL_LOG_DEBUG("name = %s", call_status_ind->remote_party_name);
    }

    if( call_status_ind->call_info.direction == CALL_DIRECTION_MO_V01 )
    {
      call_status_ind->call_info.call_id = call_id;
    }

    QCRIL_LOG_DEBUG(" call_id = %d,  state = %d, type = %d, direction = %d, name_valid = %d",
                     call_status_ind->call_info.call_id, call_status_ind->call_info.call_state,
                     call_status_ind->call_info.call_type, call_status_ind->call_info.direction,
                     call_status_ind->remote_party_name_valid);

    qcril_hook_unsol_response( instance_id,  QCRIL_EVT_HOOK_VT_UNSOL_CALL_STATUS_IND, ind_data_ptr, ind_data_len );
  }

} /* qcril_qmi_ims_vt_call_status_ind_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_ims_vt_dial_call_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle VT_DIAL_CALL_RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_vt_dial_call_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  ims_vt_dial_call_resp_v01    *dial_call_resp;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    dial_call_resp = (ims_vt_dial_call_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "VT Dial Call Resp : result = %d, resp_error=%d ",
                   dial_call_resp->resp.result,
                   dial_call_resp->resp.error );

    result = QMI_IMS_OP_RESULT( dial_call_resp->resp.result, dial_call_resp );

    if ( result == QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = dial_call_resp;
      resp.resp_len = sizeof(ims_vt_dial_call_resp_v01);
    }
    else
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, dial_call_resp->resp.error, &resp );
    }

    qcril_send_request_response( &resp );
    call_id = dial_call_resp->call_id;
    QCRIL_LOG_INFO( "Dial Call Resp Complete with result %d for call id %d", result, dial_call_resp->call_id );
  }
  else
  {
      QCRIL_LOG_FATAL("params_ptr is NULL");
  }


} /* qcril_qmi_voice_dial_call_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_ims_vt_answer_call_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle VT_ANSWER_CALL_RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_vt_answer_call_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  ims_vt_answer_call_resp_v01  *ans_call_resp;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    ans_call_resp = (ims_vt_answer_call_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "VT Answwer Call Resp : result = %d, resp_error=%d ",
                   ans_call_resp->resp.result,
                   ans_call_resp->resp.error );

    result = QMI_IMS_OP_RESULT( ans_call_resp->resp.result, ans_call_resp );

    if ( result == QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = ans_call_resp;
      resp.resp_len = sizeof(ims_vt_answer_call_resp_v01);
    }
    else
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ans_call_resp->resp.error, &resp );
    }

    qcril_send_request_response( &resp );
    QCRIL_LOG_INFO( "VT Answer Call Resp Complete with result %d for call id %d", result, ans_call_resp->call_id );
  }
  else
  {
      QCRIL_LOG_FATAL("params_ptr is NULL");
  }

} /* qcril_qmi_voice_answer_call_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_ims_vt_end_call_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle VT_END_CALL_RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_vt_end_call_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  ims_vt_end_call_resp_v01  *end_call_resp;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    end_call_resp = (ims_vt_end_call_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "VT End Call Resp : result = %d, resp_error=%d ",
                   end_call_resp->resp.result,
                   end_call_resp->resp.error );

    result = QMI_IMS_OP_RESULT( end_call_resp->resp.result, end_call_resp );

    if ( result == QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = end_call_resp;
      resp.resp_len = sizeof(ims_vt_end_call_resp_v01);
    }
    else
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, end_call_resp->resp.error, &resp );
    }

    qcril_send_request_response( &resp );
    QCRIL_LOG_INFO( "VT Call End Resp Complete with result %d for call id %d", result, end_call_resp->call_id );
  }
  else
  {
      QCRIL_LOG_FATAL("params_ptr is NULL");
  }

} /* qcril_qmi_ims_vt_end_call_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_ims_vt_command_cb

===========================================================================*/
/*!
    @brief
    Common Callback for all the QMI IMS commands.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_vt_command_cb
(
  qmi_client_type              user_handle,
  unsigned int                 msg_id,
  void                        *resp_c_struct,
  unsigned int                 resp_c_struct_len,
  void                        *resp_cb_data,
  qmi_client_error_type        transp_err
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  uint16 req_id;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;
  qcril_request_params_type req_data;
  /*-----------------------------------------------------------------------*/
  QCRIL_ASSERT( resp_c_struct != NULL );
  QCRIL_NOTUSED(user_handle);

  user_data = ( uint32 )(uintptr_t) resp_cb_data;
  instance_id = QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( user_data );
  modem_id = QCRIL_EXTRACT_MODEM_ID_FROM_USER_DATA( user_data );
  req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( user_data );
  req_data.modem_id = modem_id;
  req_data.instance_id = instance_id;
  req_data.datalen = resp_c_struct_len;
  req_data.data = resp_c_struct;
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("msg_id %d (%s)", msg_id, qcril_qmi_ims_vt_lookup_command_name(msg_id));

  /* Lookup the Token ID */
  if ( qcril_reqlist_query_by_req_id( req_id, &instance_id, &req_info ) == E_SUCCESS )
  {
    req_data.t = req_info.t;
    req_data.event_id = req_info.request;

    if( transp_err != QMI_NO_ERR )
    {
      QCRIL_LOG_INFO("Transp error (%d) recieved from QMI for RIL request %d", transp_err, req_info.request);
      /* Send GENERIC_FAILURE response */
      qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
    }
    else
    {
    switch(msg_id)
    {
      case IMS_VT_DIAL_CALL_RESP_V01:
        qcril_qmi_ims_vt_dial_call_resp_hdlr(&req_data);
        break;

      case IMS_VT_END_CALL_RESP_V01:
        qcril_qmi_ims_vt_end_call_resp_hdlr(&req_data);
        qcril_qmi_ims_info.last_incom_connid = QCRIL_QMI_VOICE_INVALID_CONN_ID;
        break;

      case IMS_VT_ANSWER_CALL_RESP_V01:
        qcril_qmi_ims_vt_answer_call_resp_hdlr(&req_data);
        break;

      default:
        QCRIL_LOG_INFO("Unsupported QMI IMS message %d", msg_id);
        break;
      }
    }
  }
  else
  {
    QCRIL_LOG_ERROR( "Req ID: %d not found\n", req_id);
  }

  /* free the buffer allocated */
  qcril_free( resp_c_struct );
}/* qcril_qmi_ims_vt_command_cb */

/*=========================================================================
  FUNCTION:  qcril_qmi_ims_vt_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Callback for QMI IMS indications

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_vt_unsol_ind_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                          *ind_buf,
  unsigned int                   ind_buf_len,
  void                          *ind_cb_data
)
{
  uint32_t decoded_payload_len = 0;
  qmi_client_error_type qmi_err = QCRIL_QMI_IMS_ERROR;
  void* decoded_payload = NULL;

  user_handle = user_handle;
  ind_cb_data = ind_cb_data;

  qmi_err = qmi_idl_get_message_c_struct_len(qcril_qmi_client_get_service_object(QCRIL_QMI_CLIENT_IMS_VT),
                            QMI_IDL_INDICATION,
                            msg_id,
                            &decoded_payload_len);

  QCRIL_LOG_DEBUG("msg_id = %d, qmi_err = %d, max payload len = %d", msg_id, qmi_err, decoded_payload_len);

  if(decoded_payload_len)
  {
     decoded_payload = qcril_malloc(decoded_payload_len);
  }

  if ( ( decoded_payload != NULL ) && ( decoded_payload_len > 0 ) )
  {
     qmi_err = qmi_client_message_decode(qcril_qmi_client_get_user_handle(QCRIL_QMI_CLIENT_IMS_VT),
                                    QMI_IDL_INDICATION,
                                    msg_id,
                                    ind_buf,
                                    ind_buf_len,
                                    decoded_payload,
                                    decoded_payload_len);

     if( qmi_err == QMI_NO_ERR )
     {
        switch( msg_id )
        {
            case IMS_VT_CALL_STATUS_IND_V01:
              qcril_qmi_ims_vt_call_status_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            default:
              QCRIL_LOG_INFO("Unknown QMI IMS indication %d", msg_id);
              break;
        }
     }
     else
     {
       QCRIL_LOG_DEBUG("%s: error in decoding IMS VT indication message", __FUNCTION__);
     }
  }
  else
  {
     QCRIL_LOG_DEBUG("%s: malloc failed", __FUNCTION__);
  }

  if( decoded_payload_len )
  {
     qcril_free(decoded_payload);
  }

}/* qcril_qmi_ims_vt_unsol_ind_cb */


/*===========================================================================

  FUNCTION:  qcril_qmi_ims_vt_dial_call

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_VT_DIAL_CALL.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_vt_dial_call
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_reqlist_public_type reqlist_entry;
  ims_vt_dial_call_req_v01  *dial_call_req = NULL;
  ims_vt_dial_call_resp_v01 *dial_call_resp = NULL;
  int err = QMI_NO_ERR;
  uint32 user_data;
  qcril_request_resp_params_type resp;
  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO("%s entered", __FUNCTION__ );
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );
  /*-----------------------------------------------------------------------*/

  modem_id = QCRIL_DEFAULT_MODEM_ID;

  if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
  {
     err = QMI_SERVICE_ERR_MALFORMED_MSG;
  }

  if ( err == QMI_NO_ERR )
  {
     dial_call_req = (ims_vt_dial_call_req_v01 *) params_ptr->data;

     QCRIL_LOG_DEBUG("VT call type valid = %d, call type = %d, number = %s", dial_call_req->call_type_valid, dial_call_req->call_type, dial_call_req->calling_number);
  }

  if ( err == QMI_NO_ERR )
  {
      /* Add entry to ReqList */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );

      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
        /* Fail to add entry to ReqList */
          err = QMI_SERVICE_ERR_NO_MEMORY;
      }

      dial_call_resp = qcril_malloc( sizeof( ims_vt_dial_call_resp_v01 ) );

      if ( dial_call_resp )
      {
         user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

         /* Send QMI IMS VT VOICE DIAL CALL REQ */
         if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_VT,
                                           IMS_VT_DIAL_CALL_REQ_V01,
                                           dial_call_req,
                                           sizeof(ims_vt_dial_call_req_v01),
                                           dial_call_resp,
                                           sizeof(ims_vt_dial_call_resp_v01),
                                           (void*)(uintptr_t)user_data) != E_SUCCESS )
         {
           err = QMI_SERVICE_ERR_GENERAL;
         }
      }
      else
      {
         err = QMI_SERVICE_ERR_NO_MEMORY;
      }
  }

  if ( err != QMI_NO_ERR )
  {
    /* free the allocated buffer in case of faliure */
    if( dial_call_resp )
    {
       qcril_free( dial_call_resp );
    }

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_INFO("%s completed with result = %d", __FUNCTION__, err );
} /* qcril_qmi_ims_vt_dial_call() */


/*===========================================================================

  FUNCTION:   qcril_qmi_ims_vt_answer_call

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_VT_ANSWER_CALL.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_vt_answer_call
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  ims_vt_answer_call_req_v01  *answer_call_req = NULL;
  ims_vt_answer_call_resp_v01 *answer_call_resp = NULL;
  int err = QMI_NO_ERR;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO("%s entered",__FUNCTION__);

  modem_id = QCRIL_DEFAULT_MODEM_ID; /* Assigning default modem id till fusion is supported for qmi */

  if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
  {
     err = QMI_SERVICE_ERR_MALFORMED_MSG;
  }

  if ( err == QMI_NO_ERR )
  {
     answer_call_req = (ims_vt_answer_call_req_v01 *) params_ptr->data;

     QCRIL_LOG_DEBUG("call id = %d, accept = %d", answer_call_req->call_id, answer_call_req->answer);
  }

  if ( err == QMI_NO_ERR )
  {
     /* Add entry to ReqList */
     qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                                  QCRIL_EVT_NONE, NULL, &reqlist_entry );

     reqlist_entry.valid_sub_id = TRUE;
     reqlist_entry.sub_id = answer_call_req->call_id;

     if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
     {
       /* Fail to add entry to ReqList */
         err = QMI_SERVICE_ERR_NO_MEMORY;
     }

     if( err == QMI_NO_ERR )
     {
        answer_call_resp = qcril_malloc( sizeof(ims_vt_answer_call_resp_v01) );

        if( answer_call_resp == NULL )
        {
          qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, QMI_SERVICE_ERR_NO_MEMORY, &resp );
          qcril_send_request_response( &resp );
          return;
        }

        user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

        /* Send QMI VOICE ANSWER CALL REQ */
        if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_VT,
                                          IMS_VT_ANSWER_CALL_REQ_V01,
                                          answer_call_req,
                                          sizeof(ims_vt_answer_call_req_v01),
                                          answer_call_resp,
                                          sizeof(ims_vt_answer_call_resp_v01),
                                          (void*)(uintptr_t)user_data) != E_SUCCESS )
        {
           err = QMI_SERVICE_ERR_GENERAL;
        }
     }
  }

  if ( err != QMI_NO_ERR )
  {
     /* free the allocated buffer */
     if( answer_call_resp != NULL )
     {
        qcril_free( answer_call_resp );
     }

     /* send the response */
     qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
     qcril_send_request_response( &resp );
  }
} /* qcril_qmi_ims_vt_answer_call() */


/*===========================================================================

  FUNCTION:  qcril_qmi_ims_vt_end_call

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_VT_END_CALL.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_vt_end_call
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  ims_vt_end_call_req_v01  *end_call_req = NULL;
  ims_vt_end_call_resp_v01 *end_call_resp = NULL;
  int err = QMI_NO_ERR;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  /* Request service on the same modem as the voice call is active */
  modem_id = QCRIL_DEFAULT_MODEM_ID;

  QCRIL_LOG_INFO("%s entered", __FUNCTION__);

  if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
  {
     err = QMI_SERVICE_ERR_MALFORMED_MSG;
  }

  if ( err == QMI_NO_ERR )
  {
     end_call_req = (ims_vt_end_call_req_v01 *) params_ptr->data;

     QCRIL_LOG_DEBUG("call id = %d", end_call_req->call_id);
  }

  if ( err == QMI_NO_ERR )
  {
     /* A call Id associated with the specified Connectin Index is found, add entry to ReqList */
     qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                               QCRIL_EVT_NONE, NULL, &reqlist_entry );

     reqlist_entry.valid_sub_id = TRUE;
     reqlist_entry.sub_id = end_call_req->call_id;

     if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
     {
       /* Fail to add entry to ReqList */
         err = QMI_SERVICE_ERR_NO_MEMORY;
     }

     QCRIL_LOG_INFO("Call with conn id %d is being disconnected", end_call_req->call_id);

     end_call_resp = qcril_malloc( sizeof(voice_end_call_resp_msg_v02) );

     if( end_call_resp == NULL )
     {
        err = QMI_SERVICE_ERR_NO_MEMORY;
     }

     if( err == QMI_NO_ERR )
     {
        user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

        /* Send QMI IMS VT END CALL REQ */
        if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_VT,
                                          IMS_VT_END_CALL_REQ_V01,
                                          end_call_req,
                                          sizeof(ims_vt_end_call_req_v01),
                                          end_call_resp,
                                          sizeof(ims_vt_end_call_resp_v01),
                                          (void*)(uintptr_t)user_data) != E_SUCCESS )

        {
           err = QMI_SERVICE_ERR_GENERAL;
        }
     }
  }

  if ( err != QMI_NO_ERR )
  {
     if( end_call_resp != NULL )
     {
        qcril_free( end_call_resp );
     }

     /* send the response */
     qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
     qcril_send_request_response( &resp );
  }
} /* qcril_qmi_ims_vt_end_call() */

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_vt_get_call_info
    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_vt_get_call_info
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;
  ims_vt_get_call_info_req_v01 *get_call_info_req = NULL;
  ims_vt_get_call_info_resp_v01 *get_call_info_res = NULL;
  int err  = QMI_NO_ERR;
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_INFO("%s entered", __FUNCTION__ );

  if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
  {
     err = QMI_SERVICE_ERR_MALFORMED_MSG;
  }

  if ( err == QMI_NO_ERR )
  {
     get_call_info_req = (ims_vt_get_call_info_req_v01 *) params_ptr->data;

     QCRIL_LOG_DEBUG("call id = %d", get_call_info_req->call_id);
  }

  if ( err == QMI_NO_ERR )
  {
     get_call_info_res = (ims_vt_get_call_info_resp_v01 *)qcril_malloc(sizeof(ims_vt_get_call_info_resp_v01));

     if( get_call_info_res == NULL )
     {
        err = QMI_SERVICE_ERR_NO_MEMORY;
     }

     if ( err == QMI_NO_ERR )
     {
        if( qcril_qmi_client_send_msg_sync( QCRIL_QMI_CLIENT_IMS_VT,
                                           IMS_VT_GET_CALL_INFO_REQ_V01,
                                           get_call_info_req,
                                           sizeof( ims_vt_get_call_info_req_v01 ),
                                           get_call_info_res,
                                           sizeof(ims_vt_get_call_info_resp_v01)
                                         ) !=E_SUCCESS )
        {
          err = QMI_SERVICE_ERR_GENERAL;
        }
        else if( get_call_info_res->resp.result != QMI_RESULT_SUCCESS_V01 )
        {
           err = QMI_SERVICE_ERR_GENERAL;
        }
     }
  }

  if( err == QMI_NO_ERR )
  {
    /* send the response */
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    resp.resp_pkt = (void *) get_call_info_res;
    resp.resp_len = sizeof( ims_vt_get_call_info_resp_v01 ) ;
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
  }

  qcril_send_request_response( &resp );

  /* free the allocated buffer */
  if( get_call_info_res )
  {
     qcril_free( get_call_info_res );
  }
} /* qcril_qmi_ims_vt_get_call_info */



/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_state_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle IMSP_ENABLER_STATE_RESP_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_enabler_state_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  imsp_get_enabler_state_resp_v01  *enabler_state_resp;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type req_info;
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {

     enabler_state_resp = (imsp_get_enabler_state_resp_v01 *) params_ptr->data;

     QCRIL_LOG_INFO( "enabler state result = %d, error = %d", enabler_state_resp->resp.result, enabler_state_resp->resp.error);

     if( enabler_state_resp->enabler_state_valid )
     {
        //Ignore the enabler state information in the response and wait for indication.
        QCRIL_LOG_INFO( "enabler state = %d", enabler_state_resp->enabler_state);
     }

    if ( qcril_reqlist_query_by_request( instance_id, QCRIL_EVT_HOOK_IMS_ENABLER_STATE_REQ, &req_info ) == E_SUCCESS )
    {
       qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );
       resp.resp_pkt = (void *) enabler_state_resp;
       resp.resp_len = sizeof( enabler_state_resp );
       qcril_send_request_response( &resp );
     }
  }
} /* qcril_qmi_ims_presence_enabler_state_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_send_publish_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SEND_PUBLISH_RESP_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_send_publish_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  imsp_send_publish_resp_v01  *publish_resp = NULL;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    publish_resp = (imsp_send_publish_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "publish Resp : result = %d, resp_error=%d ",
                   publish_resp->resp.result,
                   publish_resp->resp.error );

    result = QMI_IMS_OP_RESULT( QMI_NO_ERR, publish_resp );

    if ( result != QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, publish_resp->resp.error, &resp );
      qcril_send_request_response( &resp );
    }

    if( publish_resp->imsp_publish_callid_valid )
    {
       qcril_reqlist_update_sub_id( params_ptr->instance_id, params_ptr->t, publish_resp->imsp_publish_callid );
    }
  }
  else
  {
    QCRIL_LOG_DEBUG("%s","invalid data received for simple publish resp");
  }
} /* qcril_qmi_ims_presence_send_publish_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_send_publish_xml_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SEND_PUBLISH_XML_RESP_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_send_publish_xml_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  imsp_send_publish_xml_resp_v01  *publish_resp = NULL;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    publish_resp = (imsp_send_publish_xml_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "publish Resp : result = %d, resp_error=%d ",
                   publish_resp->resp.result,
                   publish_resp->resp.error );

    result = QMI_IMS_OP_RESULT( QMI_NO_ERR, publish_resp );

    if ( result != QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, publish_resp->resp.error, &resp );
      qcril_send_request_response( &resp );
    }

    /* update the call_id for matching against the indication */
    if( publish_resp->imsp_publish_callid_valid )
    {
       qcril_reqlist_update_sub_id( params_ptr->instance_id, params_ptr->t, publish_resp->imsp_publish_callid );
    }
  }
  else
  {
    QCRIL_LOG_DEBUG("%s","invalid data received for simple publish resp");
  }
} /* qcril_qmi_ims_presence_send_publish_xml_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_ims_send_unpublish_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SEND_UNPUBLISH_RESP_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_send_unpublish_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  imsp_send_unpublish_resp_v01  *unpublish_resp = NULL;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    unpublish_resp = (imsp_send_unpublish_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "Unpublish Resp : result = %d, resp_error=%d ",
                   unpublish_resp->resp.result,
                   unpublish_resp->resp.error );

    result = QMI_IMS_OP_RESULT( unpublish_resp->resp.result, unpublish_resp );

    if ( result != QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, unpublish_resp->resp.error, &resp );
      qcril_send_request_response( &resp );
    }

    /* update the call_id for matching against the indication */
    if( unpublish_resp->imsp_unpublish_callid_valid )
    {
       qcril_reqlist_update_sub_id( params_ptr->instance_id, params_ptr->t, unpublish_resp->imsp_unpublish_callid);
    }
  }
  else
  {
     QCRIL_LOG_ERROR("%s","Invalid data recieved for unpublish resp");
  }
} /* qcril_qmi_ims_send_unpublish_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_send_subscribe_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SEND_SUBSCRIBE_RESP_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_presence_send_subscribe_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  imsp_send_subscribe_resp_v01  *subscribe_resp;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    subscribe_resp = (imsp_send_subscribe_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "Suscribe Resp : result = %d, resp_error=%d ",
                   subscribe_resp->resp.result,
                   subscribe_resp->resp.error );

    result = QMI_IMS_OP_RESULT( subscribe_resp->resp.result, subscribe_resp );

    if ( result != QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, subscribe_resp->resp.error, &resp );
      qcril_send_request_response( &resp );
    }

    /* update the call_id for matching against the indication */
    if( subscribe_resp->imsp_subscribe_callid_valid )
    {
       qcril_reqlist_update_sub_id( params_ptr->instance_id, params_ptr->t, subscribe_resp->imsp_subscribe_callid );
    }
  }
  else
  {
     QCRIL_LOG_ERROR("%s","Invalid data recieved for subscribe resp");
  }

} /* qcril_qmi_send_subscribe_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_presence_send_subscribe_xml_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SEND_SUBSCRIBE_XML_RESP_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_presence_send_subscribe_xml_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  imsp_send_subscribe_xml_resp_v01  *subscribe_xml_resp;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    subscribe_xml_resp = (imsp_send_subscribe_xml_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "Suscribe Resp : result = %d, resp_error=%d ",
                   subscribe_xml_resp->resp.result,
                   subscribe_xml_resp->resp.error );

    result = QMI_IMS_OP_RESULT( subscribe_xml_resp->resp.result, subscribe_xml_resp );

    if ( result != QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, subscribe_xml_resp->resp.error, &resp );
      qcril_send_request_response( &resp );
    }

    /* update the call_id for matching against the indication */
    if( subscribe_xml_resp->imsp_subscribe_callid_valid )
    {
       qcril_reqlist_update_sub_id( params_ptr->instance_id, params_ptr->t, subscribe_xml_resp->imsp_subscribe_callid );
    }
  }
  else
  {
     QCRIL_LOG_ERROR("%s","Invalid data recieved for subscribe resp");
  }

} /* qcril_qmi_presence_send_subscribe_xml_resp_hdlr */



/*=========================================================================
  FUNCTION:  qcril_qmi_send_unsubscribe_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SEND_UNSUBSCRIBE_RESP_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_presence_send_unsubscribe_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  imsp_send_unsubscribe_resp_v01  *unsubscribe_resp;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;
  char *oem_hook_resp = NULL;
  int oem_hook_resp_len = 0;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    unsubscribe_resp = (imsp_send_unsubscribe_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "Unsubscribe Resp : result = %d, resp_error=%d ",
                   unsubscribe_resp->resp.result,
                   unsubscribe_resp->resp.error );

    result = QMI_IMS_OP_RESULT( unsubscribe_resp->resp.result, unsubscribe_resp );

    if ( result != QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, unsubscribe_resp->resp.error, &resp );
      qcril_send_request_response( &resp );
    }

    /* update the call_id for matching against the indication */
    if( unsubscribe_resp->imsp_unsubscribe_callid_valid )
    {
       qcril_reqlist_update_sub_id( params_ptr->instance_id, params_ptr->t, unsubscribe_resp->imsp_unsubscribe_callid );
    }
  }
  else
  {
     QCRIL_LOG_ERROR("%s","Invalid data recieved for subscribe resp");
  }
} /* qcril_qmi_send_unsubscribe_resp_hdlr */



/*=========================================================================
  FUNCTION:  qcril_qmi_presence_send_set_notify_fmt_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SET_NOTIFY_FMT_RESP_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_presence_send_set_notify_fmt_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  imsp_set_notify_fmt_resp_v01  *set_notify_resp;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;
  char *oem_hook_resp = NULL;
  int oem_hook_resp_len = 0;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    set_notify_resp = (imsp_set_notify_fmt_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "Set notify fmt resp : result = %d, resp_error=%d ",
                   set_notify_resp->resp.result,
                   set_notify_resp->resp.error );

    result = QMI_IMS_OP_RESULT( set_notify_resp->resp.result, set_notify_resp );

    if ( result != QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, set_notify_resp->resp.error, &resp );
      qcril_send_request_response( &resp );
    }
    else
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      qcril_send_request_response( &resp );
    }
  }
  else
  {
     QCRIL_LOG_ERROR("%s","Invalid data recieved for subscribe resp");
  }
} /* qcril_qmi_presence_send_set_notify_fmt_resp_hdlr */



/*=========================================================================

  FUNCTION:  qcril_qmi_presence_send_get_notify_fmt_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_GET_NOTIFY_FMT_RESP_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_presence_send_get_notify_fmt_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  imsp_get_notify_fmt_resp_v01  *get_notify_resp;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;
  char *oem_hook_resp = NULL;
  int oem_hook_resp_len = 0;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    get_notify_resp = (imsp_get_notify_fmt_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "Set notify fmt resp : result = %d, resp_error=%d ",
                   get_notify_resp->resp.result,
                   get_notify_resp->resp.error );

    result = QMI_IMS_OP_RESULT( get_notify_resp->resp.result, get_notify_resp );

    if ( result != QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, get_notify_resp->resp.error, &resp );
      qcril_send_request_response( &resp );
    }
    else
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = get_notify_resp;
      resp.resp_len = sizeof( *get_notify_resp );
      qcril_send_request_response( &resp );
    }
  }
  else
  {
     QCRIL_LOG_ERROR("%s","Invalid data recieved for subscribe resp");
  }
} /* qcril_qmi_presence_send_get_notify_fmt_resp_hdlr */



/*=========================================================================

  FUNCTION:  qcril_qmi_presence_send_set_event_report_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SET_EVENT_REPORT_RESP_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_presence_send_set_event_report_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  imsp_set_event_report_resp_v01  *set_event_resp;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;
  char *oem_hook_resp = NULL;
  int oem_hook_resp_len = 0;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    set_event_resp = (imsp_set_event_report_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "Set event report resp : result = %d, resp_error=%d ",
                   set_event_resp->resp.result,
                   set_event_resp->resp.error );

    result = QMI_IMS_OP_RESULT( set_event_resp->resp.result, set_event_resp );

    if ( result != QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, set_event_resp->resp.error, &resp );
      qcril_send_request_response( &resp );
    }
    else
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = set_event_resp;
      resp.resp_len = sizeof( *set_event_resp );
      qcril_send_request_response( &resp );
    }
  }
  else
  {
     QCRIL_LOG_ERROR("%s","Invalid data recieved for Set event  resp");
  }
} /* qcril_qmi_presence_send_set_event_report_resp_hdlr */



/*=========================================================================

  FUNCTION:  qcril_qmi_presence_send_get_event_report_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_GET_EVENT_REPORT_RESP_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_presence_send_get_event_report_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  imsp_get_event_report_resp_v01  *get_event_resp;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type  modem_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  int result  = QCRIL_QMI_IMS_ERROR;
  char *oem_hook_resp = NULL;
  int oem_hook_resp_len = 0;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  modem_id = params_ptr->modem_id;


  if( params_ptr->data != NULL )
  {
    get_event_resp = (imsp_get_event_report_resp_v01 *) params_ptr->data;

    QCRIL_LOG_INFO( "Get event report resp : result = %d, resp_error=%d ",
                   get_event_resp->resp.result,
                   get_event_resp->resp.error );

    result = QMI_IMS_OP_RESULT( get_event_resp->resp.result, get_event_resp );

    if ( result != QMI_NO_ERR )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, get_event_resp->resp.error, &resp );
      qcril_send_request_response( &resp );
    }
    else
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = get_event_resp;
      resp.resp_len = sizeof( *get_event_resp );
      qcril_send_request_response( &resp );
    }
  }
  else
  {
     QCRIL_LOG_ERROR("%s","Invalid data recieved for Get event report resp");
  }
} /* qcril_qmi_presence_send_set_event_report_resp_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_publish_trigger_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_PUBLISH_TRIGGER_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_publish_trigger_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  imsp_publish_trigger_ind_v01 *publish_trigger_ind;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  uint32 req_id = QMI_RIL_ZERO;
  boolean result = FALSE;
  qcril_request_resp_params_type resp;

  if( ind_data_ptr != NULL )
  {
    publish_trigger_ind = (imsp_publish_trigger_ind_v01*)ind_data_ptr;
    qcril_hook_unsol_response( instance_id,  QCRIL_EVT_HOOK_IMS_PUBLISH_TRIGGER_IND_V01, ind_data_ptr, ind_data_len );
  }

  QCRIL_LOG_FUNC_RETURN();
}/* qcril_qmi_ims_presence_publish_trigger_ind_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_notify_xml_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_NOTIFY_XML_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_notify_xml_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  imsp_notify_xml_ind_v01 *notify_xml_ind;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_reqlist_public_type req_info;
  uint32 req_id = QMI_RIL_ZERO;
  boolean result = FALSE;
  qcril_request_resp_params_type resp;

  if( ind_data_ptr != NULL )
  {
    notify_xml_ind = (imsp_notify_xml_ind_v01*)ind_data_ptr;

    QCRIL_LOG_DEBUG(" callid valid = %d, value = %d", notify_xml_ind->imsp_subscribe_callid_valid, notify_xml_ind->imsp_subscribe_callid);

    qcril_hook_unsol_response( instance_id,  QCRIL_EVT_HOOK_IMS_NOTIFY_XML_IND_V01, ind_data_ptr, ind_data_len );
  }

  QCRIL_LOG_FUNC_RETURN();
}/* qcril_qmi_ims_presence_notify_xml_ind_hdlr */



/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_notify_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_NOTIFY_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_notify_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  imsp_notify_ind_v01 *notify_ind;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_reqlist_public_type req_info;
  uint32 req_id = QMI_RIL_ZERO;
  boolean result = FALSE;
  qcril_request_resp_params_type resp;

  if( ind_data_ptr != NULL )
  {
    notify_ind = (imsp_notify_ind_v01*)ind_data_ptr;

    QCRIL_LOG_DEBUG(" call-id valid = %d, value = %d", notify_ind->imsp_subscribe_callid_valid, notify_ind->imsp_subscribe_callid);

    qcril_hook_unsol_response( instance_id,  QCRIL_EVT_HOOK_IMS_NOTIFY_IND_V01, ind_data_ptr, ind_data_len );
  }

  QCRIL_LOG_FUNC_RETURN();
}/* qcril_qmi_ims_presence_notify_ind_hdlr */



/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_enabler_state_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle IMS_ENABLER_STATE_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_enabler_state_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  imsp_enabler_state_ind_v01 *enabler_state_ind;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  uint32 req_id = QMI_RIL_ZERO;
  boolean result = FALSE;
  qcril_request_resp_params_type resp;

  if( ind_data_ptr != NULL )
  {
    enabler_state_ind = (imsp_enabler_state_ind_v01*)ind_data_ptr;

    QCRIL_LOG_DEBUG(" enabler_state = %d", enabler_state_ind->enabler_state);

    qcril_hook_unsol_response( instance_id,  QCRIL_EVT_HOOK_IMS_ENABLER_STATUS_IND, ind_data_ptr, ind_data_len );
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_ims_presence_enabler_state_ind_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_publish_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SEND_PUBLISH_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_publish_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  imsp_send_publish_ind_v01 *publish_ind;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_reqlist_public_type req_info;
  imsp_send_publish_resp_v01  publish_resp;
  qcril_request_resp_params_type resp;
  int err_code = QMI_SERVICE_ERR_GENERAL;

  memset( &publish_resp, 0, sizeof(publish_resp) );

  QCRIL_NOTUSED(ind_data_len);
  if( ind_data_ptr != NULL )
  {
     publish_ind = (imsp_send_publish_ind_v01*)ind_data_ptr;

     QCRIL_LOG_DEBUG(" publish status resp = %d", publish_ind->status_response);

     if ( qcril_reqlist_query_by_request( instance_id, QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_REQ, &req_info ) != E_SUCCESS )
     {
        QCRIL_LOG_DEBUG("ignoring publish ind, as there is no request pending for indication");
        return;
     }

     if( ( publish_ind->imsp_publish_callid == req_info.sub_id ) && (req_info.valid_sub_id ) )
     {
        publish_resp.imsp_publish_callid_valid = TRUE;
        publish_resp.imsp_publish_callid = publish_ind->imsp_publish_callid;

        if( publish_ind->status_response )
        {
           qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );
        }
        else
        {
           publish_resp.resp.result = QMI_RESULT_FAILURE_V01;
           publish_resp.resp.error  = QMI_ERR_INTERNAL_V01;
           if( publish_ind->imsp_sip_resp_code_valid )
           {
                publish_resp.resp.error = publish_ind->imsp_sip_resp_code;
                err_code = publish_ind->imsp_sip_resp_code;
           }
           else
           {
               QCRIL_LOG_ERROR("did not receive sip code");
               err_code = QMI_SERVICE_ERR_GENERAL;
           }
           qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, err_code, &resp );
        }
        resp.resp_pkt = (void *) &publish_resp;
        resp.resp_len = sizeof( publish_resp );
        qcril_send_request_response( &resp );
     }
     else
     {
         QCRIL_LOG_DEBUG("call_id between publish response(%d) and indication(%d) does not match, ignoring publish indication",
                         req_info.sub_id, publish_ind->imsp_publish_callid );
     }

  }
  else
  {
     QCRIL_LOG_DEBUG("NULL data received in publish indication, Ignoring");
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_ims_presence_publish_ind_hdlr */



/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_publish_xml_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SEND_PUBLISH_XML_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_publish_xml_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  imsp_send_publish_xml_ind_v01 *publish_xml_ind;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_reqlist_public_type req_info;
  imsp_send_publish_xml_resp_v01  publish_xml_resp;
  qcril_request_resp_params_type resp;
  int err_code = QMI_SERVICE_ERR_GENERAL;

  memset( &publish_xml_resp, 0, sizeof(publish_xml_resp) );

  QCRIL_NOTUSED(ind_data_len);
  if( ind_data_ptr != NULL )
  {
     publish_xml_ind = (imsp_send_publish_xml_ind_v01*)ind_data_ptr;

     QCRIL_LOG_DEBUG(" publish status resp = %d", publish_xml_ind->status_response);

     if ( qcril_reqlist_query_by_request( instance_id, QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_XML_REQ, &req_info ) != E_SUCCESS )
     {
        QCRIL_LOG_DEBUG("ignoring publish ind, as there is no request pending for indication");
        return;
     }

     if( ( publish_xml_ind->imsp_publish_callid == req_info.sub_id ) && (req_info.valid_sub_id ) )
     {
         publish_xml_resp.imsp_publish_callid_valid = TRUE;
         publish_xml_resp.imsp_publish_callid = publish_xml_ind->imsp_publish_callid;

         if( publish_xml_ind->status_response )
         {
            qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );

         }
         else
         {
            publish_xml_resp.resp.result = QMI_RESULT_FAILURE_V01;
            publish_xml_resp.resp.error  = QMI_ERR_INTERNAL_V01;
            if( publish_xml_ind->imsp_sip_resp_code_valid )
            {
                publish_xml_resp.resp.error = publish_xml_ind->imsp_sip_resp_code;
                err_code = publish_xml_ind->imsp_sip_resp_code;
           }
           else
           {
               QCRIL_LOG_ERROR("did not receive sip code");
               err_code = QMI_SERVICE_ERR_GENERAL;
            }
            qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, err_code, &resp );
         }
         resp.resp_pkt = (void *) &publish_xml_resp;
         resp.resp_len = sizeof( publish_xml_resp );
         qcril_send_request_response( &resp );
     }
     else
     {
         QCRIL_LOG_DEBUG("call_id between publish_xml response(%d) and indication(%d) does not match, ignoring publish_xml indication",
                         req_info.sub_id, publish_xml_ind->imsp_publish_callid );
     }
  }
  else
  {
     QCRIL_LOG_DEBUG("NULL data received in publish indication, Ignoring");
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_ims_presence_publish_xml_ind_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_unpublish_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SEND_UNPUBLISH_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_unpublish_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  imsp_send_unpublish_ind_v01 *unpublish_ind;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_reqlist_public_type req_info;
  imsp_send_unpublish_resp_v01  unpublish_resp;
  qcril_request_resp_params_type resp;
  int err_code = QMI_SERVICE_ERR_GENERAL;

  memset( &unpublish_resp, 0, sizeof(unpublish_resp) );

  QCRIL_NOTUSED(ind_data_len);
  if( ind_data_ptr != NULL )
  {
     unpublish_ind = (imsp_send_unpublish_ind_v01*)ind_data_ptr;

     QCRIL_LOG_DEBUG(" unpublish status resp = %d", unpublish_ind->status_response);

     if ( qcril_reqlist_query_by_request( instance_id, QCRIL_EVT_HOOK_IMS_SEND_UNPUBLISH_REQ, &req_info ) != E_SUCCESS )
     {
        QCRIL_LOG_DEBUG("ignoring unpublish ind, as there is no request pending for indication");
        return;
     }

     if( ( unpublish_ind->imsp_unpublish_callid == req_info.sub_id ) && (req_info.valid_sub_id ) )
     {
        unpublish_resp.imsp_unpublish_callid_valid = TRUE;
        unpublish_resp.imsp_unpublish_callid = unpublish_ind->imsp_unpublish_callid;

        if( unpublish_ind->status_response )
        {
           qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );
        }
        else
        {
           unpublish_resp.resp.result = QMI_RESULT_FAILURE_V01;
           unpublish_resp.resp.error  = QMI_ERR_INTERNAL_V01;
           if( unpublish_ind->imsp_sip_resp_code_valid )
           {
              unpublish_resp.resp.error = unpublish_ind->imsp_sip_resp_code;
              err_code = unpublish_ind->imsp_sip_resp_code;
           }
           else
           {
               QCRIL_LOG_ERROR("did not receive sip code");
               err_code = QMI_SERVICE_ERR_GENERAL;
           }
           qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );
        }
        resp.resp_pkt = (void *) &unpublish_resp;
        resp.resp_len = sizeof( unpublish_resp );
        qcril_send_request_response( &resp );
     }
     else
     {
        QCRIL_LOG_DEBUG("call_id between unpublish response(%d) and indication(%d) does not match, ignoring unpublish indication",
                        req_info.sub_id, unpublish_ind->imsp_unpublish_callid );
     }
  }
  else
  {
     QCRIL_LOG_DEBUG("NULL data received in unpublish indication, Ignoring");
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_ims_presence_unpublish_ind_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_subscribe_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SEND_SUBSCRIBE_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_subscribe_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  imsp_send_subscribe_ind_v01 *subscribe_ind;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_reqlist_public_type req_info;
  imsp_send_subscribe_resp_v01  subscribe_resp;
  qcril_request_resp_params_type resp;
  int err_code = QMI_SERVICE_ERR_GENERAL;

  memset( &subscribe_resp, 0, sizeof(subscribe_resp) );

  QCRIL_NOTUSED(ind_data_len);
  if( ind_data_ptr != NULL )
  {
     subscribe_ind = (imsp_send_subscribe_ind_v01*)ind_data_ptr;

     QCRIL_LOG_DEBUG(" subscribe status resp = %d", subscribe_ind->status_response);

     if ( qcril_reqlist_query_by_request( instance_id, QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_REQ, &req_info ) != E_SUCCESS )
     {
        QCRIL_LOG_DEBUG("ignoring subscribe ind, as there is no request pending for indication");
        return;
     }

     if( ( subscribe_ind->imsp_subscribe_callid == req_info.sub_id ) && (req_info.valid_sub_id ) )
     {
         subscribe_resp.imsp_subscribe_callid_valid = TRUE;
         subscribe_resp.imsp_subscribe_callid_valid = subscribe_ind->imsp_subscribe_callid;

         if( subscribe_ind->status_response )
         {
            qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );

         }
         else
         {
            subscribe_resp.resp.result = QMI_RESULT_FAILURE_V01;
            subscribe_resp.resp.error  = QMI_ERR_INTERNAL_V01;
            if( subscribe_ind->imsp_sip_resp_code_valid )
            {
                subscribe_resp.resp.error = subscribe_ind->imsp_sip_resp_code;
                err_code = subscribe_ind->imsp_sip_resp_code;
           }
           else
           {
               QCRIL_LOG_ERROR("did not receive sip code");
               err_code = QMI_SERVICE_ERR_GENERAL;
            }
            qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, err_code, &resp );
         }
         resp.resp_pkt = (void *) &subscribe_resp;
         resp.resp_len = sizeof( subscribe_resp );
         qcril_send_request_response( &resp );
     }
     else
     {
         QCRIL_LOG_DEBUG("call_id between subscribe response(%d) and indication(%d) does not match, ignoring subscribe indication",
                         req_info.sub_id, subscribe_ind->imsp_subscribe_callid );
     }
  }
  else
  {
     QCRIL_LOG_DEBUG("NULL data received in unpublish indication, Ignoring");
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_ims_presence_subscribe_ind_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_subscribe_xml_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SEND_SUBSCRIBE_XML_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_subscribe_xml_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  imsp_send_subscribe_xml_ind_v01 *subscribe_xml_ind;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_reqlist_public_type req_info;
  imsp_send_subscribe_xml_resp_v01  subscribe_xml_resp;
  qcril_request_resp_params_type resp;
  int err_code = QMI_SERVICE_ERR_GENERAL;

  memset( &subscribe_xml_resp, 0, sizeof(subscribe_xml_resp) );

  QCRIL_NOTUSED(ind_data_len);
  if( ind_data_ptr != NULL )
  {
     subscribe_xml_ind = (imsp_send_subscribe_xml_ind_v01*)ind_data_ptr;

     QCRIL_LOG_DEBUG(" subscribe status resp = %d", subscribe_xml_ind->status_response);

     if ( qcril_reqlist_query_by_request( instance_id, QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_XML_REQ, &req_info ) != E_SUCCESS )
     {
        QCRIL_LOG_DEBUG("ignoring subscribe ind, as there is no request pending for indication");
        return;
     }

     if( ( subscribe_xml_ind->imsp_subscribe_callid == req_info.sub_id ) && (req_info.valid_sub_id ) )
     {
         subscribe_xml_resp.imsp_subscribe_callid_valid = TRUE;
         subscribe_xml_resp.imsp_subscribe_callid = subscribe_xml_ind->imsp_subscribe_callid;

         if( subscribe_xml_ind->status_response )
         {
            qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );

         }
         else
         {
            subscribe_xml_resp.resp.result = QMI_RESULT_FAILURE_V01;
            subscribe_xml_resp.resp.error  = QMI_ERR_INTERNAL_V01;
            if( subscribe_xml_ind->imsp_sip_resp_code_valid )
            {
                subscribe_xml_resp.resp.error = subscribe_xml_ind->imsp_sip_resp_code;
                err_code = subscribe_xml_ind->imsp_sip_resp_code;
            }
            else
            {
                QCRIL_LOG_ERROR("did not receive sip code");
                err_code = QMI_SERVICE_ERR_GENERAL;
            }
            qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, err_code, &resp );
         }
         resp.resp_pkt = (void *) &subscribe_xml_resp;
         resp.resp_len = sizeof( subscribe_xml_resp );
         qcril_send_request_response( &resp );
     }
     else
     {
         QCRIL_LOG_DEBUG("call_id between subscribe_xml response(%d) and indication(%d) does not match, ignoring subscribe_xml indication",
                         req_info.sub_id, subscribe_xml_ind->imsp_subscribe_callid );
     }
  }
  else
  {
     QCRIL_LOG_DEBUG("NULL data received in subscibe_xml indication, Ignoring");
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_ims_presence_subscribe_xml_ind_hdlr */



/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_unsubscribe_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_IMSP_SEND_UNSUBSCRIBE_IND_V01.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_unsubscribe_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  imsp_send_unsubscribe_ind_v01 *unsubscribe_ind;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_reqlist_public_type req_info;
  imsp_send_unsubscribe_resp_v01  unsubscribe_resp;
  qcril_request_resp_params_type resp;
  int err_code = QMI_SERVICE_ERR_GENERAL;

  memset( &unsubscribe_resp, 0, sizeof(unsubscribe_resp) );

  QCRIL_NOTUSED(ind_data_len);
  if( ind_data_ptr != NULL )
  {
     unsubscribe_ind = (imsp_send_unsubscribe_ind_v01*)ind_data_ptr;

     QCRIL_LOG_DEBUG(" subscribe status resp = %d", unsubscribe_ind->status_response);

     if ( qcril_reqlist_query_by_request( instance_id, QCRIL_EVT_HOOK_IMS_SEND_UNSUBSCRIBE_REQ, &req_info ) != E_SUCCESS )
     {
        QCRIL_LOG_DEBUG("ignoring unsubscribe ind, as there is no request pending for indication");
        return;
     }

     if( ( unsubscribe_ind->imsp_unsubscribe_callid == req_info.sub_id ) && (req_info.valid_sub_id ) )
     {
        unsubscribe_resp.imsp_unsubscribe_callid_valid = TRUE;
        unsubscribe_resp.imsp_unsubscribe_callid = unsubscribe_ind->imsp_unsubscribe_callid;
        if( unsubscribe_ind->status_response )
        {
           qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_SUCCESS, &resp );

        }
        else
        {
           unsubscribe_resp.resp.result = QMI_RESULT_FAILURE_V01;
           unsubscribe_resp.resp.error  = QMI_ERR_INTERNAL_V01;
           if ( unsubscribe_ind->imsp_sip_resp_code_valid )
           {
              unsubscribe_resp.resp.error = unsubscribe_ind->imsp_sip_resp_code;
              err_code = unsubscribe_ind->imsp_sip_resp_code;
           }
           else
           {
              QCRIL_LOG_ERROR("did not receive sip code");
              err_code = QMI_SERVICE_ERR_GENERAL;
           }
           qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, err_code, &resp );
        }
        resp.resp_pkt = (void *) &unsubscribe_resp;
        resp.resp_len = sizeof( unsubscribe_resp );
        qcril_send_request_response( &resp );
     }
     else
     {
        QCRIL_LOG_DEBUG("call_id between unsubscribe response(%d) and indication(%d) does not match, ignoring unsubscribe indication",
                        req_info.sub_id, unsubscribe_ind->imsp_unsubscribe_callid );
     }
  }
  else
  {
     QCRIL_LOG_DEBUG("NULL data received in unsubscribe indication, Ignoring");
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_ims_presence_unsubscribe_ind_hdlr */



/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_command_cb

===========================================================================*/
/*!
    @brief
    Common Callback for all the QMI IMS commands.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_command_cb
(
  qmi_client_type              user_handle,
  unsigned int                 msg_id,
  void                        *resp_c_struct,
  unsigned int                 resp_c_struct_len,
  void                        *resp_cb_data,
  qmi_client_error_type        transp_err
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  uint16 req_id;
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;
  qcril_request_params_type req_data;
  /*-----------------------------------------------------------------------*/
  user_handle = user_handle;

  QCRIL_ASSERT( resp_c_struct != NULL );
  user_data = ( uint32 )(uintptr_t) resp_cb_data;
  instance_id = QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( user_data );
  modem_id = QCRIL_EXTRACT_MODEM_ID_FROM_USER_DATA( user_data );
  req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( user_data );
  req_data.modem_id = modem_id;
  req_data.instance_id = instance_id;
  req_data.datalen = resp_c_struct_len;
  req_data.data = resp_c_struct;
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("%s for msg_id %d (%s)", __FUNCTION__, msg_id, qcril_qmi_ims_presence_lookup_command_name(msg_id));


  /* Lookup the Token ID */
  if ( qcril_reqlist_query_by_req_id( req_id, &instance_id, &req_info ) == E_SUCCESS )
  {
     req_data.t = req_info.t;
     req_data.event_id = req_info.request;

     QCRIL_LOG_DEBUG("request-id = %d", req_info.request );

     if( transp_err != QMI_NO_ERR )
     {
       QCRIL_LOG_INFO("Transp error (%d) recieved from QMI for RIL request %d", transp_err, req_info.request);
       /* Send GENERIC_FAILURE response */
       qcril_default_request_resp_params( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE, &resp );
       qcril_send_request_response( &resp );
     }
     else
     {
        switch(msg_id)
        {
          case QMI_IMSP_GET_ENABLER_STATE_RESP_V01:
            qcril_qmi_ims_presence_enabler_state_resp_hdlr(&req_data);
            break;

          case QMI_IMSP_SEND_PUBLISH_RESP_V01:
            qcril_qmi_ims_presence_send_publish_resp_hdlr(&req_data);
            break;

          case QMI_IMSP_SEND_PUBLISH_XML_RESP_V01:
            qcril_qmi_ims_presence_send_publish_xml_resp_hdlr(&req_data);
            break;

          case QMI_IMSP_SEND_UNPUBLISH_RESP_V01:
            qcril_qmi_ims_presence_send_unpublish_resp_hdlr(&req_data);
            break;

          case QMI_IMSP_SEND_SUBSCRIBE_RESP_V01:
            qcril_qmi_presence_send_subscribe_resp_hdlr(&req_data);
            break;

          case QMI_IMSP_SEND_SUBSCRIBE_XML_RESP_V01:
            qcril_qmi_presence_send_subscribe_xml_resp_hdlr(&req_data);
            break;

          case QMI_IMSP_SEND_UNSUBSCRIBE_RESP_V01:
            qcril_qmi_presence_send_unsubscribe_resp_hdlr(&req_data);
            break;

          case QMI_IMSP_SET_NOTIFY_FMT_RESP_V01:
            qcril_qmi_presence_send_set_notify_fmt_resp_hdlr(&req_data);
            break;

          case QMI_IMSP_GET_NOTIFY_FMT_RESP_V01:
            qcril_qmi_presence_send_get_notify_fmt_resp_hdlr(&req_data);
            break;

          case QMI_IMSP_SET_EVENT_REPORT_RESP_V01:
            qcril_qmi_presence_send_set_event_report_resp_hdlr(&req_data);
            break;

          case QMI_IMSP_GET_EVENT_REPORT_RESP_V01:
            qcril_qmi_presence_send_get_event_report_resp_hdlr(&req_data);
            break;

          default:
            QCRIL_LOG_INFO("Unsupported QMI IMS message %d", msg_id);
            break;
        }
     }
  }
  else
  {
    QCRIL_LOG_ERROR( "Req ID: %d not found for %s\n", req_id, __FUNCTION__ );
  }

  /* free the buffer allocated */
  qcril_free( resp_c_struct );
}/* qcril_qmi_ims_vt_command_cb */



/*=========================================================================
  FUNCTION:  qcril_qmi_ims_presence_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Callback for QMI PRESENCE IMS indications

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_unsol_ind_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                          *ind_buf,
  unsigned int                   ind_buf_len,
  void                          *ind_cb_data
)
{
  uint32_t decoded_payload_len = 0;
  qmi_client_error_type qmi_err = QCRIL_QMI_IMS_ERROR;
  void* decoded_payload = NULL;

  user_handle = user_handle;
  ind_cb_data = ind_cb_data;

  qmi_err = qmi_idl_get_message_c_struct_len(qcril_qmi_client_get_service_object(QCRIL_QMI_CLIENT_IMS_PRESENCE),
                            QMI_IDL_INDICATION,
                            msg_id,
                            &decoded_payload_len);
  QCRIL_LOG_DEBUG("msg_id = %d, qmi_err = %d, decoded payload len = %d", msg_id, qmi_err, decoded_payload_len);

  if(decoded_payload_len)
  {
     decoded_payload = qcril_malloc(decoded_payload_len);
  }

  if ( ( decoded_payload != NULL ) && ( decoded_payload_len != 0) )
  {
     qmi_err = qmi_client_message_decode(qcril_qmi_client_get_user_handle(QCRIL_QMI_CLIENT_IMS_PRESENCE),
                                    QMI_IDL_INDICATION,
                                    msg_id,
                                    ind_buf,
                                    ind_buf_len,
                                    decoded_payload,
                                    decoded_payload_len);

     if( qmi_err == QMI_NO_ERR )
     {
        switch( msg_id )
        {
            case QMI_IMSP_PUBLISH_TRIGGER_IND_V01:
              qcril_qmi_ims_presence_publish_trigger_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            case QMI_IMSP_NOTIFY_XML_IND_V01:
              qcril_qmi_ims_presence_notify_xml_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            case QMI_IMSP_NOTIFY_IND_V01:
              qcril_qmi_ims_presence_notify_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            case QMI_IMSP_ENABLER_STATE_IND_V01:
              qcril_qmi_ims_presence_enabler_state_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            case QMI_IMSP_SEND_PUBLISH_IND_V01:
              qcril_qmi_ims_presence_publish_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            case QMI_IMSP_SEND_PUBLISH_XML_IND_V01:
              qcril_qmi_ims_presence_publish_xml_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            case QMI_IMSP_SEND_UNPUBLISH_IND_V01:
              qcril_qmi_ims_presence_unpublish_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            case QMI_IMSP_SEND_SUBSCRIBE_IND_V01:
              qcril_qmi_ims_presence_subscribe_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            case QMI_IMSP_SEND_SUBSCRIBE_XML_IND_V01:
              qcril_qmi_ims_presence_subscribe_xml_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            case QMI_IMSP_SEND_UNSUBSCRIBE_IND_V01:
              qcril_qmi_ims_presence_unsubscribe_ind_hdlr(decoded_payload, decoded_payload_len);
              break;

            default:
              QCRIL_LOG_INFO("Unknown QMI IMS presence Indication %d", msg_id);
              break;
        }
     }
     else
     {
       QCRIL_LOG_DEBUG("error while decodin the indication, err = %d", qmi_err);
     }
  }
  else
  {
    QCRIL_LOG_ERROR("%s", "memory exhausted");
  }

  if( decoded_payload != NULL )
  {
     qcril_free(decoded_payload);
  }

}/* qcril_qmi_ims_presence_unsol_ind_cb */



/*===========================================================================

  FUNCTION:   qcril_qmi_ims_presence_enabler_state_req

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_IMS_ENABLER_STATE_REQ.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_enabler_state_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  imsp_get_enabler_state_resp_v01 *enabler_state_resp = NULL;
  int err = QMI_NO_ERR;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO(": entered");

  modem_id = QCRIL_DEFAULT_MODEM_ID; /* Assigning default modem id till fusion is supported for qmi */

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                              QCRIL_EVT_NONE, NULL, &reqlist_entry );


  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
     /* Fail to add entry to ReqList */
     err = QMI_SERVICE_ERR_NO_MEMORY;
  }

  if( err == QMI_NO_ERR )
  {
     enabler_state_resp = qcril_malloc( sizeof(*enabler_state_resp) );

     if( enabler_state_resp == NULL )
     {
       qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, QMI_SERVICE_ERR_NO_MEMORY, &resp );
       qcril_send_request_response( &resp );
       return;
     }

     user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

     /* Send QMI_IMSP_GET_ENABLER_STATE_REQ_V01 */
     if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_PRESENCE,
                                       QMI_IMSP_GET_ENABLER_STATE_REQ_V01,
                                       NULL,
                                       0,
                                       enabler_state_resp,
                                       sizeof(*enabler_state_resp),
                                       (void*)(uintptr_t)user_data) != E_SUCCESS )
     {
        err = QMI_SERVICE_ERR_GENERAL;
     }
  }


  if ( err != QMI_NO_ERR )
  {
     /* free the allocated buffer */
     if( enabler_state_resp != NULL )
     {
        qcril_free( enabler_state_resp );
     }

     /* send the response */
     qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
     qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) err );
} /* qcril_qmi_ims_presence_enabler_state_req() */


/*===========================================================================

  FUNCTION:   qcril_qmi_ims_presence_send_publish_req

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_REQ.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_send_publish_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  imsp_send_publish_req_v01 *send_publish_req = NULL;
  imsp_send_publish_resp_v01 *send_publish_resp = NULL;
  int err = QMI_NO_ERR;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO("%s entered",__FUNCTION__);

  modem_id = QCRIL_DEFAULT_MODEM_ID;

  if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
  {
     err = QMI_SERVICE_ERR_MALFORMED_MSG;
  }

  if ( err == QMI_NO_ERR )
  {
     send_publish_req = (imsp_send_publish_req_v01 *) params_ptr->data;

     QCRIL_LOG_DEBUG("publish_status  = %d", send_publish_req->publish_status);

     if( send_publish_req->presence_info_valid )
     {
        QCRIL_LOG_DEBUG("publish_service_capabilities = %d, publish_service_descriptions  = %d", send_publish_req->presence_info.service_capabilities, send_publish_req->presence_info.service_descriptions);
     }
  }

  if ( err == QMI_NO_ERR )
  {
      /* Add entry to ReqList */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );

      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
        /* Fail to add entry to ReqList */
          err = QMI_SERVICE_ERR_NO_MEMORY;
      }

      send_publish_resp = qcril_malloc( sizeof( *send_publish_resp ) );

      if ( send_publish_resp != NULL )
      {
         user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

         /* Send QMI IMS PRESENCE SEND PUBLISH REQ */
         if (qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_PRESENCE,
                                           QMI_IMSP_SEND_PUBLISH_REQ_V01,
                                           send_publish_req,
                                           sizeof( *send_publish_req ),
                                           send_publish_resp,
                                           sizeof( *send_publish_resp ),
                                           (void*)(uintptr_t)user_data) != E_SUCCESS )
         {
            err = QMI_SERVICE_ERR_GENERAL;
         }
      }
      else
      {
         err = QMI_SERVICE_ERR_NO_MEMORY;
      }
  }

  if ( err != QMI_NO_ERR )
  {
    /* free the allocated buffer in case of faliure */

    if( send_publish_resp != NULL )
    {
       qcril_free( send_publish_resp );
    }

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) err );
} /* qcril_qmi_ims_presence_send_publish_req() */


/*===========================================================================

  FUNCTION:   qcril_qmi_ims_presence_send_publish_xml_req

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_XML_REQ.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_send_publish_xml_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  imsp_send_publish_xml_req_v01 *send_publish_req = NULL;
  imsp_send_publish_xml_resp_v01 *send_publish_resp = NULL;
  int err = QMI_NO_ERR;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO(":entered");

  modem_id = QCRIL_DEFAULT_MODEM_ID;

  if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
  {
     err = QMI_SERVICE_ERR_MALFORMED_MSG;
  }

  if ( err == QMI_NO_ERR )
  {
     send_publish_req = (imsp_send_publish_xml_req_v01 *) params_ptr->data;

     QCRIL_LOG_DEBUG("publish_xml  = %s", send_publish_req->imsp_rich_publish_xml);
  }

  if ( err == QMI_NO_ERR )
  {
      /* Add entry to ReqList */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );

      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
        /* Fail to add entry to ReqList */
          err = QMI_SERVICE_ERR_NO_MEMORY;
      }

      send_publish_resp = qcril_malloc( sizeof( *send_publish_resp ) );

      if ( send_publish_resp != NULL )
      {
         user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

         /* Send QMI_IMSP_SEND_PUBLISH_XML_REQ */
         if (qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_PRESENCE,
                                           QMI_IMSP_SEND_PUBLISH_XML_REQ_V01,
                                           send_publish_req,
                                           sizeof( *send_publish_req ),
                                           send_publish_resp,
                                           sizeof( *send_publish_resp ),
                                           (void*)(uintptr_t)user_data) != E_SUCCESS )
         {
            err = QMI_SERVICE_ERR_GENERAL;
         }
      }
      else
      {
         err = QMI_SERVICE_ERR_NO_MEMORY;
      }
  }

  if ( err != QMI_NO_ERR )
  {
    /* free the allocated buffer in case of faliure */

    if( send_publish_resp != NULL )
    {
       qcril_free( send_publish_resp );
    }

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) err );
} /* qcril_qmi_ims_presence_send_publish_xml_req() */

/*===========================================================================

  FUNCTION:   qcril_qmi_ims_presence_send_unpublish_req

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_IMS_SEND_UNPUBLISH_REQ.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_send_unpublish_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  imsp_send_unpublish_req_v01 *unpublish_req = NULL;
  imsp_send_unpublish_resp_v01 *unpublish_resp = NULL;
  int err = QMI_NO_ERR;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO("%s entered",__FUNCTION__);

  modem_id = QCRIL_DEFAULT_MODEM_ID; /* Assigning default modem id till fusion is supported for qmi */

  if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
  {
     err = QMI_SERVICE_ERR_MALFORMED_MSG;
  }

  if ( err == QMI_NO_ERR )
  {
     unpublish_req = (imsp_send_unpublish_req_v01 *) params_ptr->data;
     QCRIL_LOG_DEBUG("unpublish call_id_valid  = %d, call_id = %d", unpublish_req->imsp_publish_callid_valid, unpublish_req->imsp_publish_callid);
  }

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                              QCRIL_EVT_NONE, NULL, &reqlist_entry );


  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
     /* Fail to add entry to ReqList */
     err = QMI_SERVICE_ERR_NO_MEMORY;
  }

  if( err == QMI_NO_ERR )
  {
     unpublish_resp = qcril_malloc( sizeof( *unpublish_resp ) );

     if( unpublish_resp == NULL )
     {
       qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, QMI_SERVICE_ERR_NO_MEMORY, &resp );
       qcril_send_request_response( &resp );
       return;
     }

     user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

     /* Send QMI_IMSP_SEND_UNPUBLISH_REQ_V01 */
     if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_PRESENCE,
                                       QMI_IMSP_SEND_UNPUBLISH_REQ_V01,
                                       unpublish_req,
                                       sizeof( *unpublish_req ),
                                       unpublish_resp,
                                       sizeof( *unpublish_resp ),
                                       (void*)(uintptr_t)user_data) != E_SUCCESS )
     {
        err = QMI_SERVICE_ERR_GENERAL;
     }
  }


  if ( err != QMI_NO_ERR )
  {
     /* free the allocated buffer */
     if( unpublish_resp != NULL )
     {
        qcril_free( unpublish_resp );
     }

     /* send the response */
     qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
     qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) err );
} /* qcril_qmi_ims_presence_send_unpublish_req() */


/*===========================================================================

  FUNCTION:   qcril_qmi_ims_presence_send_subscribe_req

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_REQ.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_send_subscribe_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  imsp_send_subscribe_req_v01 *send_subscribe_req = NULL;
  imsp_send_subscribe_resp_v01 *send_subscribe_resp = NULL;
  int err = QMI_NO_ERR;
  uint32_t i;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO("%s entered",__FUNCTION__);

  modem_id = QCRIL_DEFAULT_MODEM_ID;

  if ( params_ptr->datalen == 0  || params_ptr->data == NULL ||
       (params_ptr->datalen > sizeof(imsp_send_subscribe_req_v01)))
  {
     err = QMI_SERVICE_ERR_MALFORMED_MSG;
  }

  if ( err == QMI_NO_ERR )
  {
     send_subscribe_req = (imsp_send_subscribe_req_v01 *) params_ptr->data;

     if (send_subscribe_req->subscribe_user_list_len > IMSP_MAX_SUBSCRIBE_USER_COUNT_V01)
     {
       err = QMI_SERVICE_ERR_MALFORMED_MSG;
     }
     else
     {
       QCRIL_LOG_DEBUG("subscription type  = %d", send_subscribe_req->subscription_type);

       for( i = 0; i < send_subscribe_req->subscribe_user_list_len; i++ )
       {
          QCRIL_LOG_DEBUG("peerURI  = %s", send_subscribe_req->subscribe_user_list[i].imsp_user_uri);
       }
     }
  }

  if ( err == QMI_NO_ERR )
  {
      /* Add entry to ReqList */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );

      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
        /* Fail to add entry to ReqList */
          err = QMI_SERVICE_ERR_NO_MEMORY;
      }

      send_subscribe_resp = qcril_malloc( sizeof( *send_subscribe_resp ) );

      if ( send_subscribe_resp != NULL )
      {
         user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

         /* Send QMI_IMSP_SEND_SUBSCRIBE_REQ_V01 */
        if( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_PRESENCE,
                                           QMI_IMSP_SEND_SUBSCRIBE_REQ_V01,
                                           send_subscribe_req,
                                           sizeof( *send_subscribe_req ),
                                           send_subscribe_resp,
                                           sizeof( *send_subscribe_resp ),
                                           (void*)(uintptr_t)user_data) != E_SUCCESS )
        {
           err = QMI_SERVICE_ERR_GENERAL;
        }
      }
      else
      {
         err = QMI_SERVICE_ERR_NO_MEMORY;
      }
  }

  if ( err != QMI_NO_ERR )
  {
    /* free the allocated buffer in case of faliure */

    if( send_subscribe_resp )
    {
       qcril_free( send_subscribe_resp );
    }

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) err );
} /* qcril_qmi_ims_presence_send_subscribe_req() */



/*===========================================================================

  FUNCTION:   qcril_qmi_ims_presence_send_subscribe_xml_req

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_XML_REQ.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_send_subscribe_xml_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  imsp_send_subscribe_xml_req_v01 *send_subscribe_req = NULL;
  imsp_send_subscribe_xml_resp_v01 *send_subscribe_resp = NULL;
  int err = QMI_NO_ERR, i;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO("%s entered",__FUNCTION__);

  modem_id = QCRIL_DEFAULT_MODEM_ID;

  if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
  {
     err = QMI_SERVICE_ERR_MALFORMED_MSG;
  }

  if ( err == QMI_NO_ERR )
  {
     send_subscribe_req = (imsp_send_subscribe_xml_req_v01 *) params_ptr->data;

     QCRIL_LOG_DEBUG("subscription user list xml = %s", send_subscribe_req->imsp_user_list_xml);

  }

  if ( err == QMI_NO_ERR )
  {
      /* Add entry to ReqList */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );

      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
        /* Fail to add entry to ReqList */
          err = QMI_SERVICE_ERR_NO_MEMORY;
      }

      send_subscribe_resp = qcril_malloc( sizeof( *send_subscribe_resp ) );

      if ( send_subscribe_resp != NULL )
      {
         user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

         /* Send QMI_IMSP_SEND_SUBSCRIBE_XML_REQ_V01 */
        if( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_PRESENCE,
                                           QMI_IMSP_SEND_SUBSCRIBE_XML_REQ_V01,
                                           send_subscribe_req,
                                           sizeof( *send_subscribe_req ),
                                           send_subscribe_resp,
                                           sizeof( *send_subscribe_resp ),
                                           (void*)(uintptr_t)user_data) != E_SUCCESS )
        {
           err = QMI_SERVICE_ERR_GENERAL;
        }
      }
      else
      {
         err = QMI_SERVICE_ERR_NO_MEMORY;
      }
  }

  if ( err != QMI_NO_ERR )
  {
    /* free the allocated buffer in case of faliure */

    if( send_subscribe_resp )
    {
       qcril_free( send_subscribe_resp );
    }

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) err );
} /* qcril_qmi_ims_presence_send_subscribe_req() */


/*===========================================================================

  FUNCTION:   qcril_qmi_ims_presence_send_unsubscribe_req

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_IMS_SEND_UNSUBSCRIBE_REQ.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_send_unsubscribe_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  imsp_send_unsubscribe_req_v01 *unsubscribe_req = NULL;
  imsp_send_unsubscribe_resp_v01 *unsubscribe_resp = NULL;
  int err = QMI_NO_ERR;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO("%s entered",__FUNCTION__);

  modem_id = QCRIL_DEFAULT_MODEM_ID;

  if ( params_ptr->datalen < sizeof(imsp_send_unsubscribe_req_v01)  || params_ptr->data == NULL )
  {
     err = QMI_SERVICE_ERR_MALFORMED_MSG;
  }

  if ( err == QMI_NO_ERR )
  {
     unsubscribe_req = (imsp_send_unsubscribe_req_v01 *) params_ptr->data;
     QCRIL_LOG_DEBUG("call_id  = %d", unsubscribe_req->imsp_subscribe_callid);
  }

  if ( err == QMI_NO_ERR )
  {
      /* Add entry to ReqList */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );

      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
        /* Fail to add entry to ReqList */
          err = QMI_SERVICE_ERR_NO_MEMORY;
      }

      unsubscribe_resp = qcril_malloc( sizeof( *unsubscribe_resp ) );

      if ( unsubscribe_resp != NULL )
      {
         user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

         /* Send QMI IMS PRESENCE SEND UNSUBSCRIBE REQ */
         err = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_PRESENCE,
                                           QMI_IMSP_SEND_UNSUBSCRIBE_REQ_V01,
                                           unsubscribe_req,
                                           sizeof( *unsubscribe_req ),
                                           unsubscribe_resp,
                                           sizeof( *unsubscribe_resp ),
                                           (void*)(uintptr_t)user_data);
      }
      else
      {
         err = QMI_SERVICE_ERR_NO_MEMORY;
      }
  }

  if ( err != QMI_NO_ERR )
  {
    /* free the allocated buffer in case of faliure */

    if( unsubscribe_resp )
    {
       qcril_free( unsubscribe_resp );
    }

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
    qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) err );
} /* qcril_qmi_ims_presence_send_unsubscribe_req() */


/*===========================================================================

  FUNCTION:   qcril_qmi_ims_presence_set_notify_fmt_req

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_IMS_SET_NOTIFY_FMT_REQ_V01 .

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_set_notify_fmt_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  imsp_set_notify_fmt_req_v01*notify_fmt_req = NULL;
  imsp_set_notify_fmt_resp_v01 *notify_fmt_resp = NULL;
  int err = QMI_NO_ERR;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO("%s entered",__FUNCTION__);

  modem_id = QCRIL_DEFAULT_MODEM_ID;

  if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
  {
     err = QMI_SERVICE_ERR_MALFORMED_MSG;
  }

  if ( err == QMI_NO_ERR )
  {
     notify_fmt_req = (imsp_set_notify_fmt_req_v01 *) params_ptr->data;
     QCRIL_LOG_DEBUG("update_with_struct_info  = %d", notify_fmt_req->update_with_struct_info);
  }

  if ( err == QMI_NO_ERR )
  {
      /* Add entry to ReqList */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );

      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
        /* Fail to add entry to ReqList */
          err = QMI_SERVICE_ERR_NO_MEMORY;
      }

      notify_fmt_resp = qcril_malloc( sizeof( *notify_fmt_resp ) );

      if ( notify_fmt_resp != NULL )
      {
         user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

         /* Send QMI_IMSP_SET_NOTIFY_FMT_REQ_V01 */
         err = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_PRESENCE,
                                           QMI_IMSP_SET_NOTIFY_FMT_REQ_V01,
                                           notify_fmt_req,
                                           sizeof( *notify_fmt_req ),
                                           notify_fmt_resp,
                                           sizeof( *notify_fmt_resp ),
                                           (void*)(uintptr_t)user_data);
      }
      else
      {
         err = QMI_SERVICE_ERR_NO_MEMORY;
      }
  }

  if ( err != QMI_NO_ERR )
  {
    /* free the allocated buffer in case of faliure */

    if( notify_fmt_resp )
    {
       qcril_free( notify_fmt_resp );
    }

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
    qcril_send_request_response( &resp );
  }

    QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) err );
} /* qcril_qmi_ims_presence_set_notify_fmt_req() */


/*===========================================================================

  FUNCTION:   qcril_qmi_ims_presence_get_notify_fmt_req

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_IMS_GET_NOTIFY_FMT_REQ_V01 .

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_get_notify_fmt_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  imsp_get_notify_fmt_resp_v01 *notify_fmt_resp = NULL;
  int err = QMI_NO_ERR;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO("%s entered",__FUNCTION__);

  modem_id = QCRIL_DEFAULT_MODEM_ID;

  if ( err == QMI_NO_ERR )
  {
      /* Add entry to ReqList */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );

      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
        /* Fail to add entry to ReqList */
          err = QMI_SERVICE_ERR_NO_MEMORY;
      }

      notify_fmt_resp = qcril_malloc( sizeof( *notify_fmt_resp ) );

      if ( notify_fmt_resp != NULL )
      {
         user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

         /* Send QMI_IMSP_SET_NOTIFY_FMT_REQ_V01 */
         err = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_PRESENCE,
                                           QMI_IMSP_GET_NOTIFY_FMT_REQ_V01,
                                           NULL,
                                           0,
                                           notify_fmt_resp,
                                           sizeof( *notify_fmt_resp ),
                                           (void*)(uintptr_t)user_data);
      }
      else
      {
         err = QMI_SERVICE_ERR_NO_MEMORY;
      }
  }

  if ( err != QMI_NO_ERR )
  {
    /* free the allocated buffer in case of faliure */

    if( notify_fmt_resp )
    {
       qcril_free( notify_fmt_resp );
    }

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
    qcril_send_request_response( &resp );
  }

    QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) err );
} /* qcril_qmi_ims_presence_get_notify_fmt_req() */


/*===========================================================================

  FUNCTION:   qcril_qmi_ims_presence_set_event_report_req

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_IMS_SET_EVENT_REPORT_REQ_V01 .

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_set_event_report_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  imsp_set_event_report_req_v01 *event_report_req = NULL;
  imsp_set_event_report_resp_v01 *event_report_resp = NULL;
  int err = QMI_NO_ERR;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO("%s entered",__FUNCTION__);

  modem_id = QCRIL_DEFAULT_MODEM_ID;

  if ( err == QMI_NO_ERR )
  {
     event_report_req = (imsp_set_event_report_req_v01 *) params_ptr->data;

     QCRIL_LOG_DEBUG("imsp_set_event_report bit mask   = %d", event_report_req->event_report_bit_masks);

  }

  if ( err == QMI_NO_ERR )
  {
      /* Add entry to ReqList */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );

      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
        /* Fail to add entry to ReqList */
          err = QMI_SERVICE_ERR_NO_MEMORY;
      }

      event_report_resp = qcril_malloc( sizeof( *event_report_resp ) );

      if ( event_report_resp != NULL )
      {
         user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

         /* Send QMI_IMSP_SET_NOTIFY_FMT_REQ_V01 */
         err = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_PRESENCE,
                                           QMI_IMSP_SET_EVENT_REPORT_REQ_V01,
                                           event_report_req,
                                           sizeof( *event_report_req ),
                                           event_report_resp,
                                           sizeof( *event_report_resp ),
                                           (void*)(uintptr_t)user_data);
      }
      else
      {
         err = QMI_SERVICE_ERR_NO_MEMORY;
      }
  }

  if ( err != QMI_NO_ERR )
  {
    /* free the allocated buffer in case of faliure */

    if( event_report_resp )
    {
       qcril_free( event_report_resp );
    }

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
    qcril_send_request_response( &resp );
  }

    QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) err );
}

/*===========================================================================

  FUNCTION:   qcril_qmi_ims_presence_get_event_report_req

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_IMS_GET_EVENT_REPORT_REQ_V01 .

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_ims_presence_get_event_report_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  imsp_get_event_report_resp_v01 *event_report_resp = NULL;
  int err = QMI_NO_ERR;
  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_INFO("%s entered",__FUNCTION__);

  modem_id = QCRIL_DEFAULT_MODEM_ID;

  if ( err == QMI_NO_ERR )
  {
      /* Add entry to ReqList */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );

      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
        /* Fail to add entry to ReqList */
          err = QMI_SERVICE_ERR_NO_MEMORY;
      }

      event_report_resp = qcril_malloc( sizeof( *event_report_resp ) );

      if ( event_report_resp != NULL )
      {
         user_data = QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, reqlist_entry.req_id );

         /* Send QMI_IMSP_SET_NOTIFY_FMT_REQ_V01 */
         err = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_PRESENCE,
                                           QMI_IMSP_GET_EVENT_REPORT_REQ_V01,
                                           NULL,
                                           0,
                                           event_report_resp,
                                           sizeof( *event_report_resp ),
                                           (void*)(uintptr_t)user_data);
      }
      else
      {
         err = QMI_SERVICE_ERR_NO_MEMORY;
      }
  }

  if ( err != QMI_NO_ERR )
  {
    /* free the allocated buffer in case of faliure */

    if( event_report_resp )
    {
       qcril_free( event_report_resp );
    }

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, err, &resp );
    qcril_send_request_response( &resp );
  }

    QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) err );
}
