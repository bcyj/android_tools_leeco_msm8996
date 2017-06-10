/******************************************************************************
  @file    qmi_client.h
  @brief   The QMI common client Header file.

  DESCRIPTION
  QMI common client routines.  All client will be build on top of these
  routines for initializing, sending messages and receiving responses/
  indications.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_client_init() needs to be called before sending or receiving of any
  service specific messages

  ---------------------------------------------------------------------------
  Copyright (c) 2007-2010,2012 Qualcomm Technologies, Inc. All Rights Reserved. 
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
#ifndef QMI_CLIENT_H
#define QMI_CLIENT_H


#include "qmi_idl_lib.h"


typedef struct         qmi_client_struct *qmi_client_type;
typedef int            qmi_client_error_type;
typedef unsigned long  qmi_txn_handle;


/* Asynchronous callback function prototype.  Individual services will
** register functions of this prototype when they send an asynchronous
** message
*/

/* This callback is associated with qmi_client_send_raw_msg_async */
typedef void (*qmi_client_recv_raw_msg_async_cb)
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_buf,
  int                            resp_buf_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
);

/* This callback is associated with qmi_client_send_msg_async */
typedef void (*qmi_client_recv_msg_async_cb)
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
);

/* Service indication callback function prototype.  Functions of this
** prototype will be registered with the common services rountines at
** initialization time.  The registered function will be called each
** time an indication message is recieved
*/
typedef void (*qmi_client_ind_cb)
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  unsigned char                  *ind_buf,
  int                            ind_buf_len,
  void                           *ind_cb_data
);


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
*/
/*=========================================================================*/
extern  qmi_client_error_type
qmi_client_init
(
  const char                                *conn_id,
  qmi_idl_service_object_type               service_obj,/* Defined in the  generated header file */
  qmi_client_ind_cb                         ind_cb,
  void                                      *ind_cb_data,
  qmi_client_type                           *user_handle
);



/*===========================================================================
  FUNCTION  qmi_client_send_raw_msg_async
===========================================================================*/
/*!
@brief
  Sends an asynchronous QMI service message on the specified connection.
  The caller is expected to encode the message before sending it through
  this function.

@return
  QMI_NO_ERR and sets the transaction handle if function is successful,
  error code otherwise.

@note

  - Dependencies
    - None

  - Side Effects
*/
/*=========================================================================*/
extern qmi_client_error_type
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
);


/*===========================================================================
  FUNCTION  qmi_client_send_msg_async
===========================================================================*/
/*!
@brief
  Sends an asynchronous QMI service message on the specified connection.
  The callback functi is expected to encode the message before sending it through
  this function.

@return
  QMI_NO_ERR and sets the transaction handle if function is successful,
  error code otherwise.

@note

  - Dependencies
    - None

  - Side Effects
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
);


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
extern qmi_client_error_type
qmi_client_delete_async_txn
(
  qmi_client_type  user_handle,
  qmi_txn_handle   async_txn_handle
);


/*===========================================================================
  FUNCTION  qmi_client_send_raw_msg_sync
===========================================================================*/
/*!
@brief
  Sends a synchronous QMI service message on the specified connection.
  This function expects the user to encode the message before sending
  and decode the message after receiving.

@return
  QMI_NO_ERR and sets resp_buf_recv_len if function is successful,
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
);


/*===========================================================================
  FUNCTION  qmi_client_send_msg_sync
===========================================================================*/
/*!
@brief
  Sends a synchronous QMI service message on the specified connection.
  This function provides the encoding/decoding functionality and
  user gets the decoded data in the response structure provided.

@return
  QMI_NO_ERR if function is successful,
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
qmi_client_send_msg_sync
(
    qmi_client_type    user_handle,
    int                msg_id,
    void               *req_c_struct,
    int                req_c_struct_len,
    void               *resp_c_struct,
    int                resp_c_struct_len,
    int                timeout_msecs
);
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
extern qmi_client_error_type
qmi_client_release
(
  qmi_client_type     user_handle
);



/*===========================================================================
  FUNCTION  qmi_client_message_encode
===========================================================================*/
/*!
@brief
  Encodes the body (TLV's) of a QMI message from the C data structure to the
  wire format.
@param[in]  user_handle   opaque handle
                          object accessor function from service header file.
@param[in]  message_type  The type of message: request, response, or indication.
@param[in]  message_id    Message ID from IDL.
@param[in]  p_src         Pointer to C structure containing message data.
@param[in]  src_len       Length (size) of p_src C structure in bytes.
@param[out] p_dst         Pointer to beginning of first TLV in message.
@param[in]  dst_len       Length of p_dst buffer in bytes.
@param[out] dst_decoded_len Pointer to the return value, the length of
                          encoded message (to be filled in as the length
                          field in the QMI header).

@retval    QMI_NO_ERR     Success
@retval    QMI_IDL_...    Error, see error codes defined in qmi.h*/
/*=========================================================================*/
extern qmi_client_error_type
qmi_client_message_encode
(
    qmi_client_type                      user_handle,
    qmi_idl_type_of_message_type         req_resp_ind,
    int                                  message_id,
    const void                           *p_src,
    int                                  src_len,
    void                                 *p_dst,
    int                                  dst_len,
    int                                  *dst_encoded_len
);

/*===========================================================================
  FUNCTION  qmi_client_message_decode
===========================================================================*/
/*!
@brief
  Decodes the body (TLV's) of a QMI message body from the wire format to the
  C structure.

@param[in]  user_handle   opaque  handle
                          object accessor function from service header file.
@param[in]  message_type  The type of message: request, response, or indication.
@param[in]  message_id    Message ID from IDL.
@param[in]  p_src         Pointer to beginning of first TLV in message.
@param[in]  src_len       Length of p_src buffer in bytes.
@param[out] p_dst         Pointer to C structure for decoded data
@param[in]  dst_len       Length (size) of p_dst C structure in bytes.

@retval    QMI_NO_ERR     Success
@retval    QMI_IDL_...    Error, see error codes defined in qmi.h
*/
/*=========================================================================*/
extern qmi_client_error_type
qmi_client_message_decode
(
    qmi_client_type                         user_handle,
    qmi_idl_type_of_message_type            req_resp_ind,
    int                                     message_id,
    const void                              *p_src,
    int                                     src_len,
    void                                    *p_dst,
    int                                     dst_len
);

/*===========================================================================
  FUNCTION  qmi_client_get_service_version
===========================================================================*/
/*!
@brief 
   Provides Major/Minor version information pertaining to a service object
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
    const char                   *conn_id,
    qmi_idl_service_object_type   service_obj,
    qmi_service_version_info     *service_version_info
);


/* type for callback to monitor outbound QMI request traffic in QMI byte stream format */
typedef void (*_qmi_service_hook_request_passthrough_type ) ( qmi_idl_service_object_type service_object, 
                                                         int message_id, 
                                                         void* encoded_qmi_bytestream, 
                                                         int encoded_qmi_bytestream_len );

/* type for callback to monitor imbound QMI response traffic in QMI byte stream format */
typedef void (*_qmi_service_hook_response_passthrough_type ) ( qmi_idl_service_object_type service_object, 
                                                             int message_id, 
                                                             void* encoded_qmi_bytestream, 
                                                             int encoded_qmi_bytestream_len );


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
                                        _qmi_service_hook_response_passthrough_type response_passthrough );

/* type for callback to monitor imbound QMI indication traffic in QMI byte stream format */
typedef void (*_qmi_service_hook_indication_passthrough_type ) ( int service_id, 
                                                             int message_id, 
                                                             void* encoded_qmi_bytestream, 
                                                             int encoded_qmi_bytestream_len );

/*=========================================================================
 FUNCTION _qmi_client_set_passthrough_hook_indication
 =========================================================================*/
/*!
@brief
  
  installs internal hooks for QMI indication bytestream traffic
@return
  none

@note

  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
void _qmi_client_set_passthrough_hook_indication( _qmi_service_hook_indication_passthrough_type indication_passthrough );


#endif /* QMI_CLIENT_H  */
