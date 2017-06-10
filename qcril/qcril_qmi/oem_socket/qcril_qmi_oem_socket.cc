/******************************************************************************
  @file    qcril_qmi_oem_socket.c
  @brief   qcril qmi - oem socket

  DESCRIPTION
    Handles oem socket related functions

  ---------------------------------------------------------------------------

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#define __STDC_FORMAT_MACROS 1
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "qcril_qmi_generic_socket.h"
#include "qcril_qmi_oem_socket.h"

#ifdef QMI_RIL_UTF
extern "C" uint32 qcril_get_time_milliseconds();
#include <unistd.h>
#endif

extern "C" {

    #include "qcril_log.h"
    #include "qcrili.h"
    #include "qcril_reqlist.h"
    #include "qcrilhook_oem.h"

    RIL_Token qcril_qmi_oem_convert_oem_token_to_ril_token(uint32_t oem_token);
    uint32_t qcril_qmi_oem_free_and_convert_ril_token_to_oem_token(RIL_Token ril_token);
}

#if defined(FEATURE_TARGET_GLIBC_x86) || defined(QMI_RIL_UTF)
   extern "C" size_t strlcat(char *, const char *, size_t);
   extern "C" size_t strlcpy(char *, const char *, size_t);
#endif

/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

/* Prefix for OEM socket path */
#ifndef QMI_RIL_UTF
#define QCRIL_QMI_OEM_SOCKET_PATH               "/dev/socket/qmux_radio/rild_oem"
#else
#define QCRIL_QMI_OEM_SOCKET_PATH               "./oem_connect_socket"
#endif

/* Maximum socket connections possible */
#define QCRIL_QMI_OEM_SOCKET_MAX_PENDING_CONNS  5

/* Length of the integer values used in OEM messages */
#define QCRIL_QMI_OEM_INT_VAL_LEN               4

/* Number of integer values in OEM Request messages */
#define QCRIL_QMI_OEM_NUM_INT_VAL_IN_REQ        4

pthread_mutex_t      oem_lock_mutex;

/* OEM socket agent class */
class qcril_qmi_oem_socket_agent : public qcril_qmi_generic_socket_agent
{
public:

    static  qcril_qmi_oem_socket_agent* get_oem_socket_agent();
    boolean send_message(int token, Oem_MsgType type, int message_id, RIL_Errno error, const void* msg, int msg_len);
    boolean send_message_unsol(Oem_MsgType type, const void* msg, int msg_len);

private:

    qcril_qmi_oem_socket_agent();

    boolean      process_incoming_message();

private: // data member

    static qcril_qmi_oem_socket_agent* instance;

};

/* Initialization of static instance variable */
qcril_qmi_oem_socket_agent* qcril_qmi_oem_socket_agent::instance = NULL;


/*===========================================================================

                     FUNCTION DEFINITIONS

===========================================================================*/

/*===========================================================================
  FUNCTION  qcril_qmi_oem_socket_agent::qcril_qmi_oem_socket_agent
===========================================================================*/
/*!
    @brief constructor for qcril_qmi_oem_socket_agent class
*/
/*=========================================================================*/
qcril_qmi_oem_socket_agent::qcril_qmi_oem_socket_agent() :
    qcril_qmi_generic_socket_agent(QCRIL_QMI_OEM_SOCKET_MAX_PENDING_CONNS)
{
    sprintf(socket_path, "%s%d", QCRIL_QMI_OEM_SOCKET_PATH, qmi_ril_get_process_instance_id());
}

/*===========================================================================
  FUNCTION  qcril_qmi_oem_socket_agent::get_oem_socket_agent
===========================================================================*/
/*!
    @brief get oem socket agent instance
*/
/*=========================================================================*/
qcril_qmi_oem_socket_agent* qcril_qmi_oem_socket_agent::get_oem_socket_agent
(
  void
)
{
  QCRIL_LOG_FUNC_ENTRY();
  pthread_mutex_lock (&oem_lock_mutex);
  if (!instance)
  {
    instance = new qcril_qmi_oem_socket_agent();
    QCRIL_LOG_INFO("instance initialized. ");
  }

  pthread_mutex_unlock (&oem_lock_mutex);

  QCRIL_LOG_FUNC_RETURN();
  return instance;

} // qcril_qmi_oem_socket_agent::get_oem_socket_agent

/*===========================================================================
  FUNCTION  qcril_qmi_oem_socket_agent::send_message_unsol
===========================================================================*/
/*!
    @brief Send unsolicited response
*/
/*=========================================================================*/
boolean qcril_qmi_oem_socket_agent::send_message_unsol
(
  Oem_MsgType type,
  const void* msg,
  int         msg_len
)
{
    boolean        ret            = 0;
    int            final_len            = (3 * QCRIL_QMI_OEM_INT_VAL_LEN) + msg_len;
    int            unsol_hook_raw = RIL_UNSOL_OEM_HOOK_RAW;
    unsigned char* tmp_msg;
    unsigned char* send_buffer;
    int            tmp_length;
    int            pyload_length;


    QCRIL_LOG_FUNC_ENTRY();

    send_buffer = (unsigned char *)qcril_malloc(final_len + QCRIL_QMI_OEM_INT_VAL_LEN);

    tmp_msg        = send_buffer;
    if (send_buffer)
    {
      if (is_socket_connected())
      {
        /* Unsolcited response message has following message format
           [length (4)] [message type(4)( Unsol)] [RIL_UNSOL_OEM_HOOK_RAW (4)] [OEM_NAME(8)]
           [Message Id (4) ] [Payload Length (4) ] [Payload] */
        tmp_length = htonl(final_len);
        memcpy( tmp_msg, &tmp_length, QCRIL_QMI_OEM_INT_VAL_LEN);
        tmp_msg+=QCRIL_QMI_OEM_INT_VAL_LEN;
        final_len+=QCRIL_QMI_OEM_INT_VAL_LEN;

        memcpy( tmp_msg, &type, QCRIL_QMI_OEM_INT_VAL_LEN);
        tmp_msg+=QCRIL_QMI_OEM_INT_VAL_LEN;

        memcpy( tmp_msg, &unsol_hook_raw, QCRIL_QMI_OEM_INT_VAL_LEN);
        tmp_msg+=QCRIL_QMI_OEM_INT_VAL_LEN;

        if ((msg_len == 0) || !msg)
        {
          pyload_length = -1;
        }
        else
        {
          pyload_length = msg_len;
        }

        memcpy( tmp_msg, &pyload_length, QCRIL_QMI_OEM_INT_VAL_LEN);
        tmp_msg+=QCRIL_QMI_OEM_INT_VAL_LEN;

        if (msg_len && msg)
        {
          memcpy( tmp_msg, msg, msg_len);
        }

        qcril_qmi_print_hex(send_buffer, final_len);
        if (send(conn_sid, send_buffer, final_len, 0) < 0)
        {
            QCRIL_LOG_ERROR("failed to send data");
        }
      }
      else
      {
        QCRIL_LOG_INFO("socket is not connected");
        ret = 1;
      }
      qcril_free(send_buffer);
    }
    else
    {
      QCRIL_LOG_INFO("could not allocate memory");
      ret = 1;
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET((int) ret);
    return ret;

} // qcril_qmi_oem_socket_agent::send_message_unsol

/*===========================================================================
  FUNCTION  qcril_qmi_oem_socket_agent::send_message
===========================================================================*/
/*!
    @brief Send response message
*/
/*=========================================================================*/
boolean qcril_qmi_oem_socket_agent::send_message
(
  int         token,
  Oem_MsgType type,
  int         message_id,
  RIL_Errno   error,
  const void* msg,
  int         msg_len
)
{
    boolean ret = 0;
    int     final_len = (4 * QCRIL_QMI_OEM_INT_VAL_LEN) + msg_len;
    int     tmp_length;
    int     pyload_length;

    unsigned char* tmp_msg;
    unsigned char* send_buffer;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_LOG_INFO("send message .....final len %d, type: %d, error: %d msg_id: %d", final_len, type, error, message_id);

    send_buffer = (unsigned char *)qcril_malloc(final_len + QCRIL_QMI_OEM_INT_VAL_LEN);
    if (send_buffer)
    {
      tmp_msg = send_buffer;
      if (is_socket_connected())
      {
        /* Response message has following message format
           [Length (4) ] [message type(4)( Resp)] [token (4)] [Error(4)]
           [Message id(4) ] [Payload Length (4) ] [Payload] */
        tmp_length = htonl(final_len);
        memcpy( tmp_msg, &tmp_length, QCRIL_QMI_OEM_INT_VAL_LEN);
        tmp_msg+=QCRIL_QMI_OEM_INT_VAL_LEN;
        final_len+=QCRIL_QMI_OEM_INT_VAL_LEN;

        memcpy( tmp_msg, &type, QCRIL_QMI_OEM_INT_VAL_LEN);
        tmp_msg+=QCRIL_QMI_OEM_INT_VAL_LEN;

        memcpy( tmp_msg, &token, QCRIL_QMI_OEM_INT_VAL_LEN);
        tmp_msg+=QCRIL_QMI_OEM_INT_VAL_LEN;

        memcpy( tmp_msg, &error, QCRIL_QMI_OEM_INT_VAL_LEN);
        tmp_msg+=QCRIL_QMI_OEM_INT_VAL_LEN;

        if ((msg_len == 0) || !msg)
        {
          pyload_length = -1;
        }
        else
        {
          pyload_length = msg_len;
        }

        memcpy( tmp_msg, &pyload_length, QCRIL_QMI_OEM_INT_VAL_LEN);
        tmp_msg+=QCRIL_QMI_OEM_INT_VAL_LEN;

        if (msg_len && msg)
        {
          memcpy( tmp_msg, msg, msg_len);
        }

        qcril_qmi_print_hex(send_buffer, final_len);
        if (send(conn_sid, send_buffer,  final_len, 0) < 0)
        {
            QCRIL_LOG_ERROR("failed to send data");
        }
      }
      else
      {
        QCRIL_LOG_INFO("socket is not connected");
        ret = 1;
      }
      qcril_free(send_buffer);
    }
    else
    {
      QCRIL_LOG_INFO("could not allocate memory");
      ret = 1;
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET((int) ret);
    return ret;

} // qcril_qmi_oem_socket_agent::send_message

/*===========================================================================
  FUNCTION  qcril_qmi_oem_socket_agent::process_incoming_message
===========================================================================*/
/*!
    @brief Process OEM request message
*/
/*=========================================================================*/
boolean qcril_qmi_oem_socket_agent::process_incoming_message
(
)
{
    QCRIL_LOG_FUNC_ENTRY();
    qcril_request_params_type param;
    qmi_ril_oem_hook_request_details_type    oem_hook_req_details;
    qcril_request_resp_params_type           resp;
    RIL_Token                                token;
    qcril_dispatch_table_entry_type *        entry_ptr = NULL;
    RIL_Errno                                audit_result = RIL_E_GENERIC_FAILURE;
    unsigned int                             req_token;
    int                                      index = 0;
    int                                      message_type = 0;
    int                                      req_len = 0;
    unsigned char                           *data = recv_buffer;

    QCRIL_LOG_INFO(" .....received %d bytes", recv_byte_num);

    memset(&param, 0, sizeof(param));
    param.instance_id = qmi_ril_get_process_instance_id();
#ifdef QMI_RIL_UTF
  if (shutdown_request != 0)
  {
    close(listen_sid);
    close(conn_sid);
    delete instance;
    instance = NULL;
    listen_sid = 0;
    conn_sid = 0;
    recv_byte_num = 0;
    shutdown_request = 0;
    pthread_exit(NULL);
  }
#endif

    do {

      if (recv_byte_num < (QCRIL_OTHER_OEM_NAME_LENGTH +
                       (QCRIL_QMI_OEM_NUM_INT_VAL_IN_REQ * QCRIL_QMI_OEM_INT_VAL_LEN)))
      {
        // The request is not supported
        audit_result = RIL_E_REQUEST_NOT_SUPPORTED;
        break;
      }

      qcril_qmi_print_hex(data, recv_byte_num);

      // decode the raw string to find out message type, data[0 - 3], 4 bytes
      memcpy( &message_type, &data[index], QCRIL_QMI_OEM_INT_VAL_LEN);
      data = data + QCRIL_QMI_OEM_INT_VAL_LEN;

      // decode the raw string to find out token, data[4 - 7], 4 bytes
      memcpy( &req_token, &data[index], QCRIL_QMI_OEM_INT_VAL_LEN);
      data = data + QCRIL_QMI_OEM_INT_VAL_LEN;

      // decode the raw string to find out length, data[8 - 11], 4 bytes
      memcpy( &req_len, &data[index], QCRIL_QMI_OEM_INT_VAL_LEN);
      data = data + QCRIL_QMI_OEM_INT_VAL_LEN;

      token = qcril_qmi_oem_convert_oem_token_to_ril_token(req_token);
      QCRIL_LOG_INFO("..... message_type %d token %d", message_type, req_token);

      param.t           = (void *)token;
      param.modem_id    = QCRIL_DEFAULT_MODEM_ID;

      if (!qmi_ril_get_req_details_from_oem_req(&oem_hook_req_details,
                                               &audit_result,
                                                data,
                                               &param,
                                                recv_byte_num))
      {
        break;
      }

      // oem hook qmi idl tunneling
      if ( oem_hook_req_details.is_qmi_tunneling )
      {
        if (!qmi_ril_parse_oem_req_tunnelled_message(&oem_hook_req_details,
                                                     &audit_result,
                                                     &param))
        {
          break;
        }
      }
    } while (0);

    if ( RIL_E_SUCCESS == audit_result )
    {
      if ( qcril_hash_table_lookup( (uint32) param.event_id, &entry_ptr ) != E_SUCCESS || NULL == entry_ptr )
      {
        // The request is not supported
        audit_result = RIL_E_REQUEST_NOT_SUPPORTED;
      }
      else
      {
          if ( qcril_dispatch_event( entry_ptr, &param ) == E_NOT_ALLOWED )
          {
            audit_result = RIL_E_RADIO_NOT_AVAILABLE;
          }
      }
    }

    if ( RIL_E_SUCCESS != audit_result )
    {
      QCRIL_LOG_ERROR( "Send error response" );
      qcril_default_request_resp_params( param.instance_id, param.t, param.event_id, audit_result, &resp );
      resp.android_request_id = RIL_REQUEST_OEM_HOOK_RAW;
      qcril_send_request_response( &resp );
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;

} // qcril_qmi_oem_socket_agent::process_incoming_message

extern "C" {

/*===========================================================================
  FUNCTION  qcril_qmi_oem_socket_init
===========================================================================*/
/*!
    @brief Initialize and start listening on oem socket
*/
/*=========================================================================*/
void qcril_qmi_oem_socket_init
(
  void
)
{
    char threadName[100] = {0};
    QCRIL_LOG_FUNC_ENTRY();
    sprintf(threadName, "%s%d", QMI_RIL_OEM_SOCKET_THREAD_NAME, qmi_ril_get_process_instance_id());
    pthread_mutex_init( &oem_lock_mutex, NULL );
    qcril_qmi_oem_socket_agent::get_oem_socket_agent()->start_socket_server(threadName);
    QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_oem_socket_init

/*===========================================================================
  FUNCTION  qcril_qmi_oem_socket_send_unsol
===========================================================================*/
/*!
    @brief Send unsolicited response message
*/
/*=========================================================================*/
void qcril_qmi_oem_socket_send_unsol
(
  const void *msg,
  int         msg_len
)
{
    QCRIL_LOG_FUNC_ENTRY();
    qcril_qmi_oem_socket_agent::get_oem_socket_agent()->send_message_unsol(OEM__MSG_TYPE__UNSOL_RESPONSE,
                                                                                         msg,
                                                                                         msg_len);
    QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_oem_socket_send_unsol

/*===========================================================================
  FUNCTION  qcril_qmi_oem_socket_send
===========================================================================*/
/*!
    @brief Send response message
*/
/*=========================================================================*/
void qcril_qmi_oem_socket_send
(
  qcril_instance_id_e_type oem_socket_instance_id,
  RIL_Token   token,
  int         message_id,
  RIL_Errno   error,
  const void *msg,
  int         msg_len
)
{
    int req_token;
    QCRIL_LOG_FUNC_ENTRY();
    qcril_reqlist_free( oem_socket_instance_id, token );
    req_token = qcril_qmi_oem_free_and_convert_ril_token_to_oem_token(token);
    qcril_qmi_oem_socket_agent::get_oem_socket_agent()->send_message(req_token,OEM__MSG_TYPE__RESPONSE,message_id,error,msg,msg_len);
    QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_oem_socket_send

/*===========================================================================
  FUNCTION  qcril_qmi_oem_convert_oem_token_to_ril_token
===========================================================================*/
/*!
    @brief Convert oem token to RIL token
*/
/*=========================================================================*/
RIL_Token qcril_qmi_oem_convert_oem_token_to_ril_token(uint32_t oem_token)
{
  RIL_Token ret = qcril_malloc(sizeof(uint32_t));
  if (NULL != ret)
  {
    uint32_t *tmp = (uint32_t*) ret;
    *tmp = oem_token ^ 0xc0000000;
  }
  return ret;
} // qcril_qmi_oem_convert_oem_token_to_ril_token

/*===========================================================================
  FUNCTION  qcril_qmi_oem_free_and_convert_ril_token_to_oem_token
===========================================================================*/
/*!
    @brief get oem token from RIL token and free RIL token
*/
/*=========================================================================*/
uint32_t qcril_qmi_oem_free_and_convert_ril_token_to_oem_token(RIL_Token ril_token)
{
  uint32_t ret = 0xFFFFFFFF;
  if (ril_token)
  {
      ret = (*((uint32_t *) ril_token)) ^ 0xc0000000;
      QCRIL_LOG_INFO("oem token: %d", ret);
      qcril_free((void*) ril_token);
  }
  else
  {
      QCRIL_LOG_ERROR("ril_token is NULL");
  }

  return ret;
} // qcril_qmi_oem_free_and_convert_ril_token_to_oem_token

/*===========================================================================
  FUNCTION qmi_ril_parse_oem_req_tunnelled_message
===========================================================================*/
/*!
    @brief Process OEM request message
*/
/*=========================================================================*/
boolean qmi_ril_parse_oem_req_tunnelled_message
(
  qmi_ril_oem_hook_request_details_type  *oem_hook_req_details,
  RIL_Errno                              *audit_result,
  qcril_request_params_type              *param
)
{
  uint16_t                                *uint16_param;
  char                                    *evt_name;
  RIL_Errno                                info_fetch_result;
  qmi_idl_service_object_type              qmi_idl_tunneling_service_object;
  qmi_client_error_type                    idl_err;
  uint32_t                                 substituted_data_len;
  void                                    *substituted_data;
  qmi_ril_oem_hook_response_context_type*  oem_hook_qmi_idl_resp_track;

  if (!oem_hook_req_details || !audit_result || !param)
  {
    return FALSE;
  }

  *audit_result = RIL_E_GENERIC_FAILURE;

  //ignore the modem id size
  param->data =  (char * )param->data + OEM_HOOK_QMI_TUNNELING_REQ_MODEM_ID_SIZE;

  //extract the service -id
  uint16_param = (uint16_t *) param->data;
  oem_hook_req_details->ril_idl_service_id = *uint16_param;
  param->data =  (char * )param->data + OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE;

  //extract the message -id
  uint16_param = (uint16_t *) param->data;
  oem_hook_req_details->ril_idl_message_id = *uint16_param;
  param->data =  (char * )param->data + OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE;

  param->datalen = param->datalen - OEM_HOOK_QMI_TUNNELING_REQ_MODEM_ID_SIZE
                                                 - OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE
                                                 - OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE;

  evt_name = NULL;
  info_fetch_result = qmi_ril_oem_hook_get_request_id( oem_hook_req_details->ril_idl_service_id,
                                                       oem_hook_req_details->ril_idl_message_id,
                                                       (uint32*)&oem_hook_req_details->hook_req,
                                                       &evt_name );
  QCRIL_LOG_DEBUG( "qmi_ril_oem_hook_get_request_id() returned %d", info_fetch_result );
  if ( RIL_E_SUCCESS != info_fetch_result )
  {
    *audit_result = info_fetch_result;
    return FALSE;
  }
  QCRIL_LOG_DEBUG( " sevice-id = %d, message-id = %d, request_id = %d",
                  oem_hook_req_details->ril_idl_service_id,
                  oem_hook_req_details->ril_idl_message_id,
                  oem_hook_req_details->hook_req );
  if ( NULL != evt_name )
  {
    QCRIL_LOG_DEBUG( "qmi_idl_tunneling: inclined to invoke evt-name = %s", evt_name );
  }
  else
  {
    QCRIL_LOG_DEBUG( "qmi_idl_tunneling: inclined to onvoke evt-name unknown");
  }
  param->event_id = oem_hook_req_details->hook_req;

  // print the recieved TLV byte stream
  QCRIL_LOG_DEBUG("oemhook tlv only byte stream");
  qcril_qmi_print_hex((unsigned char *)param->data,  param->datalen);

  // convert payload if any to readable form
  qmi_idl_tunneling_service_object = qmi_ril_oem_hook_qmi_idl_tunneling_get_service_object( (qmi_ril_oem_hook_qmi_tunneling_service_id_type)oem_hook_req_details->ril_idl_service_id );

  if ( NULL != qmi_idl_tunneling_service_object )
  {
    idl_err = qmi_idl_get_message_c_struct_len( qmi_idl_tunneling_service_object, QMI_IDL_REQUEST, oem_hook_req_details->ril_idl_message_id, &substituted_data_len  );

    QCRIL_LOG_DEBUG("decoded msg len ret = %d, input length = %d", substituted_data_len, param->datalen);
    QCRIL_LOG_DEBUG("srvc_id = %d, msg_id = %d", oem_hook_req_details->ril_idl_service_id, oem_hook_req_details->ril_idl_message_id);

    if ( oem_hook_req_details->ril_idl_service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_VT && oem_hook_req_details->ril_idl_message_id == IMS_VT_GET_CALL_INFO_REQ_V01 )
    {
       if ( substituted_data_len != 4 )
       {
          QCRIL_LOG_DEBUG("change max length to 4 due to  idl issue");
          substituted_data_len = 4;
       }
    }

    if ( QMI_NO_ERR == idl_err )
    {
      if (substituted_data_len > 0 )
      {
        // expected non empty payload
        substituted_data = qcril_malloc( substituted_data_len );

        if ( NULL != substituted_data )
        {
            idl_err = qmi_idl_message_decode( qmi_idl_tunneling_service_object,
                                    QMI_IDL_REQUEST,
                                    oem_hook_req_details->ril_idl_message_id,
                                    param->data,
                                    param->datalen,
                                    substituted_data,
                                    substituted_data_len );

            QCRIL_LOG_DEBUG("substituated msg len  = %d, input length = %d", substituted_data_len, param->datalen);

            if ( QMI_NO_ERR == idl_err )
            {
              param->data      = substituted_data;
              param->datalen   = substituted_data_len;
            }
            else
            {
              QCRIL_LOG_ERROR( "QMI IDL - request decode error %d", (int) idl_err  );
              qcril_free(substituted_data);
              return FALSE;
            }
        }
        else
        {
          QCRIL_LOG_ERROR( "QMI IDL - substituted data decoded request allocation failed for len %d", (int) substituted_data_len  );
          return FALSE;
        }
      }
      else
      { // empty payload
        param->data      = NULL;
        param->datalen   = QMI_RIL_ZERO;
      }
    }
    else
    {
      QCRIL_LOG_ERROR( "QMI IDL - request decode error len %d", (int) idl_err );
      return FALSE;
    }
  }
  else if ( NULL == qmi_idl_tunneling_service_object  )
  {
    QCRIL_LOG_ERROR( "QMI IDL - NULL service object" );
    return FALSE;
  }

  // store token, service, message -d,  as they are required while sending the response.
  oem_hook_qmi_idl_resp_track = (qmi_ril_oem_hook_response_context_type*)qcril_malloc( sizeof( *oem_hook_qmi_idl_resp_track )  );
  if ( NULL == oem_hook_qmi_idl_resp_track )
  {
    QCRIL_LOG_ERROR( "QMI IDL - tracker allocation failure" );
    return FALSE;
  }
  memset( oem_hook_qmi_idl_resp_track, 0, sizeof( *oem_hook_qmi_idl_resp_track ) );
  oem_hook_qmi_idl_resp_track->original_token     = param->t;
  oem_hook_qmi_idl_resp_track->ril_idl_service_id = oem_hook_req_details->ril_idl_service_id;
  oem_hook_qmi_idl_resp_track->ril_idl_message_id = oem_hook_req_details->ril_idl_message_id;
  oem_hook_qmi_idl_resp_track->ril_request_id     = oem_hook_req_details->hook_req;
  *audit_result = RIL_E_SUCCESS;

  pthread_mutex_lock( &qmi_ril_oem_hook_overview.overview_lock_mutex );
  oem_hook_qmi_idl_resp_track->next   = qmi_ril_oem_hook_overview.qmi_ril_oem_hook_qmi_tunneling_reqs_root;
  qmi_ril_oem_hook_overview.qmi_ril_oem_hook_qmi_tunneling_reqs_root  = oem_hook_qmi_idl_resp_track;
  pthread_mutex_unlock( &qmi_ril_oem_hook_overview.overview_lock_mutex );

  return TRUE;
}

/*===========================================================================
  FUNCTION  qmi_ril_get_req_details_from_oem_req
===========================================================================*/
/*!
    @brief Process OEM request message
*/
/*=========================================================================*/
boolean qmi_ril_get_req_details_from_oem_req
(
  qmi_ril_oem_hook_request_details_type  *oem_hook_req_details,
  RIL_Errno                              *audit_result,
  unsigned char                          *data,
  qcril_request_params_type              *param,
  int                                     recv_byte_num
)
{
  if (!oem_hook_req_details || !audit_result || !data || !param)
  {
    return FALSE;
  }

  // parse the OEM hook request to distinguish between internal or customer specific requests
  if ( ( RIL_E_SUCCESS == qmi_ril_parse_oem_hook_header( data, oem_hook_req_details ) ) && oem_hook_req_details->is_oem_hook )
  {
    if ( ( ( oem_hook_req_details->hook_req > QCRIL_EVT_HOOK_BASE ) && ( oem_hook_req_details->hook_req < QCRIL_EVT_HOOK_MAX ) ) ||
         ( ( oem_hook_req_details->hook_req > QCRIL_EVT_OEM_BASE ) && ( oem_hook_req_details->hook_req < QCRIL_EVT_OEM_MAX ) ) )
    {
      // This is an OEM_HOOK request, Convert it to look like a internal RIL REQUEST
      // Move data pointer past the QCRILHook header and re-adjusting the length
      param->event_id = oem_hook_req_details->hook_req;
      param->data = (char *)data+ QCRIL_HOOK_HEADER_SIZE;
      param->datalen = oem_hook_req_details->hook_req_len;
      if ( ( oem_hook_req_details->hook_req > QCRIL_EVT_OEM_BASE ) && ( oem_hook_req_details->hook_req < QCRIL_EVT_OEM_MAX ) )
      {
        // externally provided by an OEM, otherwise will be dispatched via qcril_dispatch_event()
        QCRIL_LOG_DEBUG("diverting inbound OEM HOOK request to external handler");
        qcrilhook_oem( param->instance_id, param->event_id, (char *)data, recv_byte_num, param->t);
        *audit_result = RIL_E_SUCCESS;
        return FALSE;
      }
      *audit_result = RIL_E_SUCCESS;
    }
    else
    {
      // The request is not supported
      *audit_result = RIL_E_REQUEST_NOT_SUPPORTED;
      return FALSE;
    }
  }
  else
  {
    // The request is not supported
    *audit_result = RIL_E_REQUEST_NOT_SUPPORTED;
    return FALSE;
  }

  return TRUE;
}
} // end of extern "C"
