/******************************************************************************
  @file    qmi_client.c
  @brief   The QMI common client layer.

  DESCRIPTION
  QMI common client routines.  All client will be build on top of these
  routines for initializing, sending messages and receiving responses/
  indications.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_client_init() needs to be called before sending or receiving of any
  service specific messages

  ---------------------------------------------------------------------------
  Copyright (c) 2007-2010,2012,2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_i.h"
#include "qmi.h"
#include "qmi_service.h"
#include "qmi_qmux_if.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_util.h"
#include "qmi_platform.h"


_qmi_service_hook_request_passthrough_type  _qmi_service_hook_request_passthrough;
_qmi_service_hook_response_passthrough_type _qmi_service_hook_response_passthrough;

struct qmi_client_struct {
    int service_user_handle;
    qmi_idl_service_object_type p_service;
}qmi_client_struct_type;



/* Forward declaration of the function */
void
qmi_client_decode_msg_async
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_buf,
  int                            resp_buf_len,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  qmi_client_recv_msg_async_cb   resp_cb,
  void                           *resp_cb_data
);


void print_qmi_encoded_decoded_msg (
  unsigned char           *msg,
  int                     msg_len
)
{
  #define MAX_BUFFER_BYTES_PER_LINE 16
  #define MAX_OUTPUT_BUF_SIZE ((MAX_BUFFER_BYTES_PER_LINE * 3) + 2)
  char output_buf [MAX_OUTPUT_BUF_SIZE];

  int i;
  char *p;
  while (msg_len > 0)
  {
    p = output_buf;

    /* Collect MAX_BUFFER_BYTES_PER_LINE bytes of buffer for display */
    for (i=0; (i<MAX_BUFFER_BYTES_PER_LINE) && (msg_len > 0); i++)
    {
      unsigned char val;

      /* First digit */
      val = (*msg >> 4) & 0x0F;
      if (val <= 9)
      {
        *p++ = val + '0';
      }
      else
      {
        *p++ = (val - 10) + 'A';
      }

      /* Second digit... ugly copied code */
      val = *msg & 0x0F;
      if (val <= 9)
      {
        *p++ = val + '0';
      }
      else
      {
        *p++ = (val - 10) + 'A';
      }

      /* Add a space, and increment msg pointer */
      *p++ = ' ';
      msg++;
      msg_len--;
    }

    /* Add \n and NULL terminator and print out */
    *p++ = '\n';
    *p = '\0';
    QMI_DEBUG_MSG_1 ("%s",output_buf);
  }

}

/*===========================================================================
  FUNCTION  qmi_client_init
===========================================================================*/
/*!
@brief

  This function is used for initializing a connection to a service.

@return

  Sets the user_handle and returns QMI_NO_ERR if successful,
  error code if not successful

@note

  - Dependencies
    - None

  - Side Effects
    - QMI connection is opened
    - Thread may block waiting for client ID to be returned by lower layers
*/
/*=========================================================================*/

extern  qmi_client_error_type
qmi_client_init
(
  const char                                *dev_id,
  qmi_idl_service_object_type               service_obj,/* Defined in the  Library header file */
  qmi_client_ind_cb                         ind_cb,
  void                                      *ind_cb_data,
  qmi_client_type                           *user_handle
)
{
    int qmi_err_code = 0;
    int err_code_connection_init;
    qmi_client_error_type rc;
    int ind_rc,conn_rc,service_id_rc;
    uint32_t service_id;
    qmi_connection_id_type conn_id;
    int handle;


    if (!user_handle)
    {
        return QMI_SERVICE_ERR_INVALID_ARG;
    }

    /*  Construct the user handle */
     *user_handle =  malloc(sizeof (qmi_client_struct_type));
     if (*user_handle == NULL ) {
         return QMI_CLIENT_ALLOC_FAILURE;
     }

     /* Extract service id */
     service_id_rc =  qmi_idl_get_service_id(service_obj,&service_id);
     if (service_id_rc !=  QMI_NO_ERR) {
         free(*user_handle);
         *user_handle = NULL;
         return service_id_rc;
     }

     if ((conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID(dev_id)) == QMI_CONN_ID_INVALID)
     {
         free(*user_handle);
         *user_handle = NULL;
         return QMI_INTERNAL_ERR;
     }

     conn_rc = qmi_service_connection_init(conn_id,
                                          &err_code_connection_init);
     if (conn_rc == QMI_NO_ERR ) {

        handle = qmi_service_init(conn_id,
                                  service_id,
                                  (void *)ind_cb,
                                  ind_cb_data,
                                  &qmi_err_code
                                 );
        if (handle < 0) {
            free(*user_handle);
            *user_handle = NULL;
            if (qmi_err_code != 0 ) {
                return qmi_err_code;
            }
            return handle;
        }
        else
        {
           /* Function call to initialize the indication call back to null as now we have only one level
           of callback */
           ind_rc = qmi_service_set_srvc_functions(service_id,NULL);
           if (ind_rc == QMI_NO_ERR ) {
                /* Save the service object and constructed service user handle */
               (*user_handle)->service_user_handle = handle;
               (*user_handle)->p_service = service_obj;

               /* Add the user_handle to the transaction information */
               rc = qmi_service_add_decode_handle(handle,*user_handle);
               if (rc != QMI_NO_ERR ) {
                   return rc;
               }
               return QMI_NO_ERR;
           }
           else {
               free(*user_handle);
               *user_handle = NULL;
               return (qmi_client_error_type)ind_rc;
           }
        }
     }
     else
     {
         free(*user_handle);
         *user_handle = NULL;
         return conn_rc;
     }
}


/*===========================================================================
  FUNCTION  qmi_client_send_raw_msg_async
===========================================================================*/
/*!
@brief
  Sends raw asynchronous QMI service message on the specified connection.
  It is the expected that the user will encode the data before sending
  and decode the data in the callback.

@return
  QMI_NO_ERR and sets the transaction handle if function is successful,
  error code otherwise.

@note

  - Dependencies
     N/A
  - Side Effects
     N/A
*/
/*=========================================================================*/
qmi_client_error_type
qmi_client_send_raw_msg_async
(
  qmi_client_type                   user_handle,
  unsigned long                     msg_id,
  void                              *req_buf,
  int                               req_buf_len,
  void                              *resp_buf,
  int                               resp_buf_len,
  qmi_client_recv_raw_msg_async_cb  resp_cb,
  void                              *resp_cb_data,
  qmi_txn_handle                    *txn_handle
)
{
    qmi_service_txn_info_type_ptr            txn;
    qmi_connection_id_type                   conn_id;
    qmi_client_id_type                       client_id;
    uint32_t                                 service_id;
    int                                      rc;


    if (!user_handle)
    {
        return QMI_SERVICE_ERR_INVALID_HANDLE;
    }

    conn_id = QMI_SRVC_CLIENT_HANDLE_TO_CONN_ID (user_handle->service_user_handle);
    client_id = QMI_SRVC_CLIENT_HANDLE_TO_CLIENT_ID (user_handle->service_user_handle);

    rc = qmi_idl_get_service_id(user_handle->p_service,&service_id);
    if (rc != QMI_NO_ERR ) {
        return rc;
    }

    if ( (rc = qmi_service_setup_txn(user_handle->service_user_handle,/* This is service layer user_handle  */
                                    service_id,
                                    msg_id,
                                    NULL,
                                    NULL,
                                    NULL,
                                    resp_cb_data,
                                    resp_cb,
                                    NULL,
                                    user_handle,
                                    resp_buf,
                                    resp_buf_len,
                                    QMI_QCCI_APIS,
                                    &txn)) < 0 )
   {
       return rc;
   }

   rc = qmi_service_send_msg (conn_id,
                              service_id,
                              client_id,
                              msg_id,
                              req_buf,
                              req_buf_len,
                              txn);
   qmi_service_release_txn(user_handle->service_user_handle,
                           txn,
                           txn_handle,
                           rc);
   return rc;
}


/*===========================================================================
  FUNCTION  qmi_client_delete_async_txn
===========================================================================*/
/*!
@brief
  Deletes an asynchronous transaction so that it will free resources
  associated with transaction

@return
   QMI_NO_ERR if successful, error code otherwise
@note

  - Dependencies
    - None

  - Side Effects
    - Async response will not be delivered

  Users should be aware of the potential race condition where an
  asynchronous response may be in the process of being handled
  by the "users_rsp_cb" callback up until this routine returns.
*/
/*=========================================================================*/
qmi_client_error_type
qmi_client_delete_async_txn
(
  qmi_client_type user_handle,
  qmi_txn_handle async_txn_handle
)
{   qmi_client_error_type rc;

    if (!user_handle)
    {
        return QMI_SERVICE_ERR_INVALID_HANDLE;
    }

    rc =  qmi_service_delete_async_txn(user_handle->service_user_handle,async_txn_handle);
    if (rc != QMI_NO_ERR ) {
        return QMI_INVALID_TXN;
    }
    return rc;
}

/*===========================================================================
  FUNCTION  qmi_client_send_msg_async
===========================================================================*/
/*!
@brief
  Sends a asynchronous QMI service message on the specified connection.
  The infrastructure encodes/decodes the data for the user.

@return
  QMI_NO_ERR and sets reply_buf_received_len if function is successful,
  error code otherwise.

@note
  - timeout_msecs is time out in milliseconds.
  - Dependencies
    - Users are advised to validate the user_handle using
      qmi_client_validate_client_handle function before passing the user_handle

  - Side Effects
    - None
*/
/*=========================================================================*/

qmi_client_error_type
qmi_client_send_msg_async
(
    qmi_client_type                 user_handle,
    unsigned long                   msg_id,
    void                            *req_c_struct,
    int                             req_c_struct_len,
    void                            *resp_c_struct,
    int                             resp_c_struct_len,
    qmi_client_recv_msg_async_cb    resp_cb,
    void                            *resp_cb_data,
    qmi_txn_handle                  *txn_handle
)
{

  int                           msg_size;
  unsigned char                 *tmp_msg_ptr;
  qmi_client_error_type         rc;
  qmi_service_txn_info_type_ptr txn;
  qmi_connection_id_type        conn_id;
  qmi_client_id_type            client_id;
  uint32_t                      max_msg_len;
  unsigned char                 *msg;
  uint32_t                      service_id;
  int encode_ret_val = 0;

  if (!user_handle)
  {
      return QMI_SERVICE_ERR_INVALID_HANDLE;
  }

  /*Extract the max message size from the service object */
  QMI_DEBUG_MSG_0(" Getting maximum message length\n");
  rc = qmi_idl_get_max_message_len(user_handle->p_service,
                                   QMI_IDL_REQUEST,
                                   msg_id,
                                   &max_msg_len);

  if (rc != QMI_NO_ERR ) {
      return rc;
  }

  QMI_DEBUG_MSG_1(" The maximum message length is : %d\n",max_msg_len);

  QMI_DEBUG_MSG_0(" Extracting serivce id \n");
  /*Extract the service id */
  rc = qmi_idl_get_service_id(user_handle->p_service,&service_id);

  if (rc != QMI_NO_ERR ) {
      return rc;
  }

  QMI_DEBUG_MSG_1(" Service id: %d\n",service_id);
  /* Allocate buffer */
  msg_size = max_msg_len + QMI_MAX_HDR_SIZE;
  msg = malloc(msg_size);

  if (msg == NULL ) {
      return QMI_CLIENT_ALLOC_FAILURE;
  }

  conn_id = QMI_SRVC_CLIENT_HANDLE_TO_CONN_ID (user_handle->service_user_handle);
  client_id = QMI_SRVC_CLIENT_HANDLE_TO_CLIENT_ID (user_handle->service_user_handle);


  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_CLIENT_TLV_PTR(msg);

  if (req_c_struct_len > 0 ) {
  /* Encode the buffer  */
  QMI_DEBUG_MSG_0("Encoding buffer \n");
  rc = qmi_client_message_encode(user_handle,
                                 QMI_IDL_REQUEST,
                                 msg_id,
                                 req_c_struct,
                                 req_c_struct_len,
                                 tmp_msg_ptr,
                                 msg_size,
                                 &encode_ret_val);

  print_qmi_encoded_decoded_msg(msg,msg_size);

  if ( rc != QMI_NO_ERR ) {
      QMI_DEBUG_MSG_0(" Encode failed \n");
      free(msg);
      return rc;
  }

  if (NULL != _qmi_service_hook_request_passthrough ) {

      _qmi_service_hook_request_passthrough( user_handle->p_service, msg_id, tmp_msg_ptr, encode_ret_val );
  }

   QMI_DEBUG_MSG_1(" %d bytes encoded \n",encode_ret_val);
  }

  /* set up transaction  */
   QMI_DEBUG_MSG_0(" Setting up transaction \n");

   if ( (rc = qmi_service_setup_txn(user_handle->service_user_handle,/* This is service layer user_handle  */
                                    service_id,
                                    msg_id,
                                    NULL,
                                    NULL,
                                    resp_cb,
                                    resp_cb_data,
                                    NULL,
                                    &qmi_client_decode_msg_async,
                                    user_handle,
                                    resp_c_struct,
                                    resp_c_struct_len,
                                    QMI_QCCI_APIS,
                                    &txn)) < 0 )
   {
       QMI_DEBUG_MSG_0("Transaction set up failed \n");
       free(msg);
       return rc;
   }
    QMI_DEBUG_MSG_0(" Sending message \n");
    /* Send message accross to modem processor */
    rc = qmi_service_send_msg (conn_id,
                               service_id,
                               client_id,
                               msg_id,
                               QMI_CLIENT_TLV_PTR(msg),
                               encode_ret_val,
                               txn);

    QMI_DEBUG_MSG_0(" Releasing Transaction \n");

    qmi_service_release_txn(user_handle->service_user_handle,
                            txn,
                            txn_handle,
                            rc);


  free(msg);
  return rc;

}

/*===========================================================================
  FUNCTION  qmi_client_decode_msg_async
===========================================================================*/
/*!
@brief
  This function will be called back from the QMI serivce layer that will decode the
  async message received from the modem processor.

@return
  N/A
@note

@Dependencies

@Side Effects
*/
/*=========================================================================*/
void
qmi_client_decode_msg_async
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_buf,
  int                            resp_buf_len,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  qmi_client_recv_msg_async_cb   resp_cb,
  void                           *resp_cb_data
)
{
    QMI_DEBUG_MSG_0(" Inside qmi_client_decode_msg_async  Callback \n");
    qmi_client_error_type rc;

    if (!user_handle)
    {
        return;
    }

    if ( NULL != _qmi_service_hook_response_passthrough ) {
        _qmi_service_hook_response_passthrough( user_handle->p_service, msg_id, resp_buf, resp_buf_len );
    }

    rc = qmi_client_message_decode( user_handle,
                                    QMI_IDL_RESPONSE,
                                    msg_id,
                                    resp_buf,
                                    resp_buf_len,
                                    resp_c_struct,
                                    (int)resp_c_struct_len);

    if (rc != QMI_NO_ERR ) {
        /* Log: Decode was not successful */
        QMI_DEBUG_MSG_1("Decode not successful w/%d\n",rc);
    }

    /* Call the user callback */
    if (resp_cb != NULL ) {

        resp_cb(user_handle,
                msg_id,
                resp_c_struct,
                (int)resp_c_struct_len,
                resp_cb_data,
                rc);

    }
    else {
        /* Log: User callback not registered */
        QMI_DEBUG_MSG_0(" User callback not registered \n");
    }

}
/*===========================================================================
  FUNCTION  qmi_client_send_raw_msg_sync
===========================================================================*/
/*!
@brief
  Sends a raw synchronous QMI service message on the specified connection.

@return
  QMI_NO_ERR and sets reply_buf_received_len if function is successful,
  error code otherwise.

@note
  - timeout_msecs is time out in milliseconds.
  - Dependencies
    - None
  - Side Effects
    - None
*/
/*=========================================================================*/
extern qmi_client_error_type
qmi_client_send_raw_msg_sync
(
  qmi_client_type           user_handle,
  unsigned long             msg_id,
  void                      *req_buf,
  int                       req_buf_len,
  void                      *resp_buf,
  int                       resp_buf_len,
  int                       *resp_buf_recv_len,
  int                       timeout_msecs
)
{
     int qmi_error;
     qmi_service_id_type service_id;
     qmi_client_error_type return_code;

     if (!user_handle)
     {
         return QMI_SERVICE_ERR_INVALID_HANDLE;
     }

     service_id = QMI_SRVC_CLIENT_HANDLE_TO_SERVICE_ID(user_handle->service_user_handle);

     int ret_val = qmi_service_send_msg_sync_millisec(user_handle->service_user_handle,
                                                      service_id,
                                                      msg_id,
                                                      QMI_CLIENT_TLV_PTR(req_buf),
                                                      (req_buf_len - QMI_MAX_HDR_SIZE),
                                                      resp_buf,
                                                      resp_buf_recv_len,
                                                      resp_buf_len,
                                                      timeout_msecs,
                                                      QMI_QCCI_APIS,
                                                      &qmi_error);

     /* Return the correct return code  */

     if (ret_val < 0 )
     {
         return_code =  (qmi_error > 0) ? (qmi_error) : ret_val;
     }
     else {
         return_code = QMI_NO_ERR;
     }

    return return_code;
}
/*===========================================================================
  FUNCTION  qmi_client_send_msg_sync
===========================================================================*/
/*!
@brief
  Sends a synchronous QMI service message on the specified connection.

@return
  QMI_NO_ERR and sets reply_buf_received_len if function is successful,
  error code otherwise.

@note
  - timeout_msecs is time out in milliseconds.
  - Dependencies
    N/A
  - Side Effects
    - None
*/
/*=========================================================================*/
extern qmi_client_error_type
qmi_client_send_msg_sync
(
    qmi_client_type    user_handle,
    int                msg_id,
    void               *req_c_struct,
    int                req_c_struct_len,
    void               *resp_c_struct,
    int                resp_c_struct_len,
    int                timeout_msecs
)
{

  unsigned char    *msg;
  unsigned char    *tmp_msg_ptr;
  qmi_client_error_type  rc;
  int reply_buf_received_len;
  int reply_max_buf_len;
  int qmi_error,ret_val;
  int req_max_msg_len;
  int max_msg_len;
  qmi_service_id_type service_id;
  int encode_ret_val = 0;


  if (!user_handle)
  {
      return QMI_SERVICE_ERR_INVALID_HANDLE;
  }

  /*Extract the max message length corresponding to request msg for this msg id */
  rc = qmi_idl_get_max_message_len( user_handle->p_service,
                                    QMI_IDL_REQUEST,
                                    msg_id,
                                   &req_max_msg_len);

  if (rc != QMI_NO_ERR ) {
      return rc;
  }
  /* Extract the max message length corresponding to response msg for this msg id */
  rc = qmi_idl_get_max_message_len(user_handle->p_service,
                                   QMI_IDL_RESPONSE,
                                   msg_id,
                                   &reply_max_buf_len);
  if (rc != QMI_NO_ERR ) {
      return rc;
  }

  max_msg_len = (req_max_msg_len >= reply_max_buf_len )? req_max_msg_len : reply_max_buf_len;

  msg = malloc(max_msg_len + QMI_MAX_HDR_SIZE);

  if (msg == NULL ) {
      return QMI_CLIENT_ALLOC_FAILURE;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_CLIENT_TLV_PTR(msg);


  /* Encode the buffer */
  if (req_c_struct_len > 0 ) {
  rc = qmi_client_message_encode( user_handle,
                                  QMI_IDL_REQUEST,
                                  msg_id,
                                  req_c_struct,
                                  req_c_struct_len,
                                  tmp_msg_ptr,
                                  max_msg_len,
                                  &encode_ret_val);

  if (rc != QMI_NO_ERR ) {
      free(msg);
      return rc;
  }
  }

  if (NULL != _qmi_service_hook_request_passthrough ) {

      (*_qmi_service_hook_request_passthrough)( user_handle->p_service, msg_id, tmp_msg_ptr, encode_ret_val );
  }


  reply_max_buf_len += QMI_MAX_HDR_SIZE;

      service_id = QMI_SRVC_CLIENT_HANDLE_TO_SERVICE_ID(user_handle->service_user_handle);
      /* Synchronously send message to modem processor */
      ret_val = qmi_service_send_msg_sync_millisec(user_handle->service_user_handle,
                                                   service_id,
                                                   msg_id,
                                                   QMI_CLIENT_TLV_PTR(msg),
                                                   encode_ret_val,
                                                   msg,
                                                   &reply_buf_received_len,
                                               reply_max_buf_len,
                                                   timeout_msecs,
                                               QMI_QCCI_APIS,
                                                   &qmi_error);
     if (ret_val < 0 )
     {
         rc =  (qmi_error > 0) ? (qmi_error) : ret_val;
         free(msg);
         return rc;
     }

   /* The return data is expected to be present in the (msg) buffer,
      if the qmi_service_send_msg_sync_millisec  function succeeds */
  /* Set tmp_msg_ptr to return data */
  tmp_msg_ptr = msg;

  /* Decode the data in case of no error */
  if (rc == QMI_NO_ERR )
  {

      if ( NULL != _qmi_service_hook_response_passthrough ) {
          _qmi_service_hook_response_passthrough( user_handle->p_service, msg_id, tmp_msg_ptr, reply_buf_received_len );
      }

        rc = qmi_client_message_decode (user_handle,
                                        QMI_IDL_RESPONSE,
                                        msg_id,
                                        tmp_msg_ptr,
                                        reply_buf_received_len,
                                        resp_c_struct,
                                        resp_c_struct_len);
        if (rc != QMI_NO_ERR ) {
            /* Log a decode error */
            QMI_ERR_MSG_1("Message decoding error ERROR CODE:%d \n",rc);
        }
  }

  free(msg);
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_client_release
===========================================================================*/
/*!
@brief
  Releases the connection to a service.

@return
  QMI_NO_ERR if function is successful, error code otherwise.

@note

  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
qmi_client_error_type
qmi_client_release
(
  qmi_client_type     user_handle
)
{
    qmi_client_error_type rc;
    int qmi_error = 0;

    if (!user_handle)
    {
        return QMI_SERVICE_ERR_INVALID_HANDLE;
    }

    rc = qmi_service_release(user_handle->service_user_handle,
                             &qmi_error);
    free(user_handle);

    if (qmi_error > 0 ) {
        rc =  qmi_error;
    }
    return rc;
}



/*===========================================================================
  FUNCTION  qmi_client_message_encode
===========================================================================*/
/*!
@brief
  Encodes a message

@return
  QMI_NO_ERROR if function is successful, error code otherwise.

@note

  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
qmi_client_error_type
qmi_client_message_encode
(
    qmi_client_type                   user_handle,
    qmi_idl_type_of_message_type      req_resp_ind,
    int                               message_id,
    const void                        *p_src,
    int                               src_len,
    void                              *p_dst,
    int                               dst_len,
    int                               *dst_encoded_len
)
{
    qmi_client_error_type rc;

    if (!user_handle)
    {
        return QMI_SERVICE_ERR_INVALID_HANDLE;
    }

    rc = qmi_idl_message_encode(user_handle->p_service,
                                req_resp_ind,
                                message_id,
                                p_src,
                                src_len,
                                p_dst,
                                dst_len,
                                (uint32_t*)dst_encoded_len);
    return rc;
}

/*===========================================================================
  FUNCTION  qmi_client_message_decode
===========================================================================*/
/*!
@brief
  Decodes a message

@return
  QMI_NO_ERROR if function is successful, error code otherwise.

@note

  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
qmi_client_error_type
qmi_client_message_decode
(
    qmi_client_type                   user_handle,
    qmi_idl_type_of_message_type      req_resp_ind,
    int                               msg_id,
    const void                        *p_src,
    int                               src_len,
    void                              *p_dst,
    int                               dst_len
)
{
    qmi_client_error_type rc;

    if (!user_handle)
    {
        return QMI_SERVICE_ERR_INVALID_HANDLE;
    }

    rc = qmi_idl_message_decode(user_handle->p_service,
                                req_resp_ind,
                                msg_id,
                                p_src,
                                src_len,
                                p_dst,
                                dst_len);
    return rc;
}

/*=========================================================================
 FUNCTION qmi_client_get_service_version
 =========================================================================*/
/*!
@brief


@return
  QMI_NO_ERR if function is successful, error code otherwise.

@note

  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
qmi_client_error_type
qmi_client_get_service_version
(
    const char                    *dev_id,
    qmi_idl_service_object_type    service_obj,
    qmi_service_version_info      *service_version_info
)
{
    qmi_client_error_type           rc;
    uint32_t                        service_id;
    qmi_connection_id_type          conn_id;
    int                             qmi_err_code;
    int                             service_id_rc;
    unsigned short                  major_ver;  /*  Major version number */
    unsigned short                  minor_ver;  /*  Minor version number */

    service_id_rc =  qmi_idl_get_service_id(service_obj,&service_id);
    if (service_id_rc !=  QMI_NO_ERR) {
        return service_id_rc;
    }

    rc = qmi_service_get_version( dev_id,
                                  service_id,
                                  service_version_info,
                                  &qmi_err_code);

   return rc;
}

/*=========================================================================
 FUNCTION _qmi_client_set_passthrough_hooks_request_response
 =========================================================================*/
/*!
@brief

  installs internal hooks for QMI request and response bytestream traffic
@return
  none

@note

  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
void _qmi_client_set_passthrough_hooks_request_response( _qmi_service_hook_request_passthrough_type request_passthough,
                                        _qmi_service_hook_response_passthrough_type response_passthrough )
{
    _qmi_service_hook_request_passthrough  = request_passthough;
    _qmi_service_hook_response_passthrough = response_passthrough;
}

