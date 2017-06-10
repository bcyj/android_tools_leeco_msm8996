#ifndef QMI_CSI_H
#define QMI_CSI_H
/******************************************************************************
  @file    qmi_csi.h
  @brief   The QMI Common Service Interface (CSI) Header file.

  DESCRIPTION
  QMI common server routines.  All server will be build on top of these
  routines for initializing, sending responses and indications.

  ---------------------------------------------------------------------------
  Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
#include "qmi_csi_target_ext.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
  QMI_CSI_NO_ERR = 0,
  QMI_CSI_CONN_REFUSED,
  QMI_CSI_CONN_BUSY,
  QMI_CSI_INVALID_HANDLE,
  QMI_CSI_INVALID_ARGS,
  QMI_CSI_ENCODE_ERR,
  QMI_CSI_DECODE_ERR,
  QMI_CSI_NO_MEM,
  QMI_CSI_INTERNAL_ERR
}qmi_csi_error;

typedef enum
{
  QMI_CSI_CB_NO_ERR = 0,
  QMI_CSI_CB_CONN_REFUSED,
  QMI_CSI_CB_NO_MEM,
  QMI_CSI_CB_INTERNAL_ERR,
  QMI_CSI_CB_UNSUPPORTED_ERR,
  QMI_CSI_CB_REQ_HANDLED,
}qmi_csi_cb_error;

/* Describe handles including theory of operation */
/* Private opaque handles */
typedef struct         qmi_client_handle_struct      *qmi_client_handle;
typedef struct         qmi_req_handle_struct         *qmi_req_handle;

/* Service handle */
typedef struct         qmi_csi_service_struct        *qmi_csi_service_handle;

typedef struct qmi_csi_options_struct qmi_csi_options;

/*=============================================================================
  INTERNAL DEFINES
=============================================================================*/
#define QMI_CSI_OPTIONS_INSTANCE_ID_VALID (1)
#define QMI_CSI_OPTIONS_MAX_OUTSTANDING_INDS_VALID (2)
#define QMI_CSI_OPTIONS_RAW_REQUEST_VALID (8)
#define QMI_CSI_OPTIONS_PRE_REQUEST_VALID (16)
#define QMI_CSI_OPTIONS_RESUME_VALID (32)

/*=============================================================================
  MACRO  QMI_CSI_OPTIONS_INIT
=============================================================================*/
/*!
@brief
  Initialize the QMI CSI Options object. Always call this before
  setting any other options field.

@param[in]  opt                 Service options object
*/
/*=========================================================================*/
#define QMI_CSI_OPTIONS_INIT(opt) (opt).options_set = 0

/*=============================================================================
  MACRO  QMI_CSI_OPTIONS_SET_INSTANCE_ID
=============================================================================*/
/*!
@brief
  Set the instance ID of the service. Default: 0

@param[in]  opt                 Service options object
@param[in]  inst                Instance ID of the service
*/
/*=========================================================================*/
#define QMI_CSI_OPTIONS_SET_INSTANCE_ID(opt, inst) \
  do { \
    (opt).instance_id = inst; \
    (opt).options_set |= QMI_CSI_OPTIONS_INSTANCE_ID_VALID; \
  } while(0)

/*=============================================================================
  MACRO  QMI_CSI_OPTIONS_SET_MAX_OUTSTANDING_INDS
=============================================================================*/
/*!
@brief
  Set the Max number of indications which are allowed to be in flight
  (Outstanding) Default: Implementation defined

@param[in]  opt                   Service options object
@param[in]  _max_outstanding_inds Maximum number of outstanding indications
                                  allowed
*/
/*=========================================================================*/
#define QMI_CSI_OPTIONS_SET_MAX_OUTSTANDING_INDS(opt, _max_outstanding_inds) \
  do {  \
    (opt).max_outstanding_inds = _max_outstanding_inds;  \
    (opt).options_set |= QMI_CSI_OPTIONS_MAX_OUTSTANDING_INDS_VALID;  \
  } while(0)
/*=============================================================================
  MACRO  QMI_CSI_OPTIONS_SET_RAW_REQUEST_CB
=============================================================================*/
/*!
@brief
  Sets a raw request handler. If the received request message ID is not
  defined in the IDL, this handler will be called with the pre-decoded
  (raw) request buffer. Note the handler has the same prototype as
  qmi_csi_process_req (See down) except the raw buffer is passed in the
  `req_c_struct` field and the buffer length is passed in the
  `req_c_struct_len` field. Returning anything other than QMI_CSI_CB_NO_ERR
  will cause the framework to send an error response on the behalf of the
  service. Default: messages not defined in the IDL will incur a auto-error
  response to be sent back to the client on service's behalf.

@param[in]  opt                 Service options object
@param[in]  _raw_req            Raw request handler
*/
/*=========================================================================*/
#define QMI_CSI_OPTIONS_SET_RAW_REQUEST_CB(opt, _raw_req) \
  do {  \
    (opt).raw_request_cb = _raw_req;  \
    (opt).options_set |= QMI_CSI_OPTIONS_RAW_REQUEST_VALID; \
  } while(0)

/*=============================================================================
  MACRO  QMI_CSI_OPTIONS_SET_PRE_REQUEST_CB
=============================================================================*/
/*!
@brief
  Sets the pre-request handler. If provided, the framework shall call this
  function before decoding the request message. The service then can decide
  if it wants to handle this message raw (Returns QMI_CSI_CB_REQ_HANDLED),
  request the framework to go ahead and decode the message and call the
  request cb (Returns QMI_CSI_CB_NO_ERR) or refuse the request
  message (Returns an error code other than QMI_CSI_CB_NO_ERR or
  QMI_CSI_CB_REQ_HANDLED). The callback just like the raw request callback
  also uses the qmi_csi_process_req prototype with the raw message provided
  in the field `req_c_struct` and the length of the buffer in `req_c_struct_len`.

@param[in]  opt                 Service options object
@param[in]  _pre_req            Pre-request handler
*/
/*=========================================================================*/
#define QMI_CSI_OPTIONS_SET_PRE_REQUEST_CB(opt, _pre_req) \
  do {  \
    (opt).pre_request_cb = _pre_req;  \
    (opt).options_set |= QMI_CSI_OPTIONS_PRE_REQUEST_VALID; \
  } while(0)

/*=============================================================================
  MACRO  QMI_CSI_OPTIONS_SET_RESUME_IND_CB
=============================================================================*/
/*!
@brief
  Sets a TX resume handler which will be called by the framework
  when a previously busy client is now accepting indications.
  Note that the callback will be called only when a call to
  qmi_csi_send_ind*() returns QMI_CSI_CONN_BUSY

@param[in]  opt                 Service options object
@param[in]  _resume_cb          Resume TX callback function
                                (See prototype: qmi_csi_resume)
*/
/*=========================================================================*/
#define QMI_CSI_OPTIONS_SET_RESUME_IND_CB(opt, _resume_cb_) \
  do {  \
    (opt).resume_ind_cb = _resume_cb_;  \
    (opt).options_set |= QMI_CSI_OPTIONS_RESUME_VALID; \
  } while(0)


/*============================================================================
                            CALLBACK FUNCTIONS
============================================================================*/


/*=============================================================================
  CALLBACK FUNCTION qmi_csi_connect
=============================================================================*/
/*!
@brief
  This callback function is called by the QCSI infrastructure when
  infrastructure receives a request from each client(user of the service).

@param[in]   client_handle       Handle used by the infrastructure to
                                 identify different services.
@param[in]   service_cookie      Service specific data. Service cookie is
                                 registered with the infrastructure during
                                 service registration(qmi_csi_register).
@param[out]  connection_handle   Services return this handle as a token to
                                 represent this client connection
                                 to the service.

@retval    QMI_CSI_CB_NO_ERR     Success
@retval    QMI_CSI_CB.....       Look into the enumeration qmi_csi_error for
                                 the error values.
*/
/*=========================================================================*/
typedef qmi_csi_cb_error (*qmi_csi_connect)
(
 qmi_client_handle         client_handle,
 void                      *service_cookie,
 void                      **connection_handle
);

/*=============================================================================
  CALLBACK FUNCTION qmi_csi_disconnect
=============================================================================*/
/*!
@brief
  This callback function is called by the QCSI infrastructure when the each
  client(user of service) deregisters with the  QCSI infrastructure.

@param[in]  connection_handle      Service handle as given by the service in
                                   qmi_csi_connect for the client
                                   disconnecting.
@param[in]  service_cookie         Service specific data.Service cookie is
                                   registered with the infrastructure during
                                   service registration(qmi_csi_register).
@retval    QMI_CSI_CB_NO_ERR       Success
@retval    QMI_CSI_CB.....         Look into the enumeration qmi_csi_error for
                                   the error values.
*/
/*=========================================================================*/
typedef void (*qmi_csi_disconnect)
(
  void                     *connection_handle,
  void                     *service_cookie
);

/*=============================================================================
  CALLBACK FUNCTION qmi_csi_process_req
=============================================================================*/
/*!
@brief
   This callback is invoked when the infrastructure receives an
   incoming message. The infrastructure decodes the data and gives it to
   the services.

@param[in]  connection_handle      Service handle as given by the service in
                                   qmi_csi_connect.
@param[in]  req_handle             Handle provided by the infrastructure
                                   to specify this particular transaction and
                                   message.
@param[in]  msg_id                 Message Id pertaining to this particular
                                   message.
@param[in]  req_c_struct           C struct with the decoded data.
@param[in]  req_c_struct_len       Length of the c struct.
@param[in]  service_cookie         Service specific data.Service cookie is
                                   registered with the infrastructure during
                                   service registration(qmi_csi_register).


@retval    QMI_CSI_CB_NO_ERR       Success
@retval    QMI_CSI_CB.....         Look into the enumeration qmi_csi_error for
                                   the error values.
*/
/*=========================================================================*/
typedef qmi_csi_cb_error (*qmi_csi_process_req)
(
  void                     *connection_handle,
  qmi_req_handle           req_handle,
 unsigned int             msg_id,
  void                     *req_c_struct,
 unsigned int             req_c_struct_len,
  void                     *service_cookie
);

/*=============================================================================
  CALLBACK FUNCTION qmi_csi_resume_ind
=============================================================================*/
/*!
@brief
  This callback function (if provided) is called by the framework
  when a previously busy client becomes available for more indications.
  (See QMI_CSI_OPTIONS_SET_RESUME_CB)

@param[in]   client_handle       Handle used by the infrastructure to
                                 identify different services.
@param[in]  connection_handle    Service handle as given by the service in
                                 qmi_csi_connect.
@param[in]   service_cookie      Service specific data. Service cookie is
                                 registered with the infrastructure during
                                 service registration(qmi_csi_register).
                                 represent this client connection
                                 to the service.

@retval    None
*/
/*=========================================================================*/
typedef void (*qmi_csi_resume_ind)
(
 qmi_client_handle         client_handle,
 void                      *connection_handle,
 void                      *service_cookie
 );


/*===========================================================================
                        FUNCTIONS
============================================================================*/


/*=============================================================================
  FUNCTION  qmi_csi_register
=============================================================================*/
/*!
@brief
  Register a service with the QCSI infrastructure.

@param[in]  service_obj         Object containing meta information to
                                encode/decode the messages.
@param[in]  service_connect     Callback to register each client with the
                                service.
@param[in]  service_disconnect  Callback to unregister each client from
                                service.
@param[in]  service_process_req Callback that handles the incoming requests.
@param[in]  service_cookie      User data.
@param[out] service_provider    Handle that infra provides to represent this
                                service connection.
@retval    QMI_CSI_NO_ERR       Success
@retval    QMI_CSI_.....        Look into the enumeration qmi_csi_error for
                                the error values.
*/
/*=========================================================================*/

qmi_csi_error
qmi_csi_register
(
   qmi_idl_service_object_type               service_obj,
   qmi_csi_connect                           service_connect,
   qmi_csi_disconnect                        service_disconnect,
   qmi_csi_process_req                       service_process_req,
   void                                      *service_cookie,
   qmi_csi_os_params                         *os_params,
   qmi_csi_service_handle                    *service_provider
);


/*=============================================================================
  FUNCTION  qmi_csi_register_with_options
=============================================================================*/
/*!
@brief
  Register a service with the QCSI infrastructure.

@param[in]  service_obj         Object containing meta information to
                                encode/decode the messages.
@param[in]  service_connect     Callback to register each client with the
                                service.
@param[in]  service_disconnect  Callback to unregister each client from
                                service.
@param[in]  service_process_req Callback that handles the incoming requests.
@param[in]  service_cookie      User data.
@param[in]  options             Options as defined by qmi_csi_options
@param[out] service_provider    Handle that infra provides to represent this
                                service connection.
@retval    QMI_CSI_NO_ERR       Success
@retval    QMI_CSI_.....        Look into the enumeration qmi_csi_error for
                                the error values.
*/
/*=========================================================================*/

qmi_csi_error
qmi_csi_register_with_options
(
 qmi_idl_service_object_type               service_obj,
 qmi_csi_connect                           service_connect,
 qmi_csi_disconnect                        service_disconnect,
 qmi_csi_process_req                       service_process_req,
 void                                      *service_cookie,
 qmi_csi_os_params                         *os_params,
 qmi_csi_options                           *options,
 qmi_csi_service_handle                    *service_provider
 );

/*=============================================================================
  FUNCTION  qmi_csi_handle_event
=============================================================================*/
/*!
@brief
  Handle event after the server thread receives an event notification.
  Callbacks from qmi_csi_register may be invoked in the server's context.

@param[in] service_provider    Opaque handle that defines the service.
@param[in] os_params           OS-defined parameters such as file handle.

@retval    QMI_CSI_NO_ERR       Success
@retval    Other error codes    Failure
*/
/*=========================================================================*/

qmi_csi_error
qmi_csi_handle_event
(
   qmi_csi_service_handle                    service_provider,
   qmi_csi_os_params                         *os_params
);

/*=============================================================================
  FUNCTION  qmi_csi_send_resp
=============================================================================*/
/*!
@brief
  Sends a response to the client.

@param[in]  req_handle            Handle used by QCSI infrastructure to
                                  identify the transaction and the destination
                                  client.
@param[in]  msg_id                Message ID for this particular message.
@param[in]  resp_c_struct         C data structure for this response.
@param[in]  resp_c_struct_len     Size of the response c struct.

@retval  QMI_CSI_NO_ERR           Success.
@retval  QMI_CSI_.....            Look into the enumeration qmi_csi_error for
                                  the error values.
*/
/*=========================================================================*/
qmi_csi_error
qmi_csi_send_resp
(
    qmi_req_handle     req_handle,
 unsigned int       msg_id,
    void               *resp_c_struct,
 unsigned int       resp_c_struct_len
);

/*=============================================================================
  FUNCTION  qmi_csi_send_resp_raw
=============================================================================*/
/*!
@brief
  Sends a response to the client without encoding.

@param[in]  req_handle            Handle used by QCSI infrastructure to
                                  identify the transaction and the destination
                                  client.
@param[in]  msg_id                Message ID for this particular message.
@param[in]  resp_buf              Response buffer
@param[in]  resp_buf_len          Size of the response buffer

@retval  QMI_CSI_NO_ERR           Success.
@retval  QMI_CSI_.....            Look into the enumeration qmi_csi_error for
                                  the error values.
*/
/*=========================================================================*/
qmi_csi_error
qmi_csi_send_resp_raw
(
 qmi_req_handle     req_handle,
 unsigned int       msg_id,
 void               *resp_buf,
 unsigned int       resp_buf_len
 );

/*=============================================================================
  FUNCTION  qmi_csi_send_ind
=============================================================================*/
/*!
@brief
  Sends an indication to the client.

@param[in]  client_handle            Handle used by QCSI infrastructure
                                     to identify the destination client.
@param[in]  msg_id                   Message ID for this particular message.
@param[in]  ind_c_struct             C data strcuture for this indication.
@param[in]  ind_c_struct_len         Size of the indication c struct

@retval    QMI_CSI_NO_ERR            Success.
@retval    QMI_CSI_.....             Look into the enumeration qmi_csi_error for
                                     the error values.
*/
/*=========================================================================*/
qmi_csi_error
qmi_csi_send_ind
(
    qmi_client_handle  client_handle,
 unsigned int       msg_id,
    void               *ind_c_struct,
 unsigned int       ind_c_struct_len
);

/*=============================================================================
  FUNCTION  qmi_csi_send_ind_raw
=============================================================================*/
/*!
@brief
  Sends an indication to the client without encoding

@param[in]  client_handle            Handle used by QCSI infrastructure
                                     to identify the destination client.
@param[in]  msg_id                   Message ID for this particular message.
@param[in]  ind_buf                  Indication buffer.
@param[in]  ind_buf_len              Size of the indication buffer.

@retval    QMI_CSI_NO_ERR            Success.
@retval    QMI_CSI_.....             Look into the enumeration qmi_csi_error for
                                     the error values.
*/
/*=========================================================================*/
qmi_csi_error
qmi_csi_send_ind_raw
(
 qmi_client_handle  client_handle,
 unsigned int       msg_id,
 void               *ind_buf,
 unsigned int       ind_buf_len
 );

/*=============================================================================
  FUNCTION  qmi_csi_send_broadcast_ind
=============================================================================*/
/*!
@brief
  Sends a broadcast indication to all registered clients.

@param[in]  service_provider         Handle used by QCSI infrastructure
                                     to identify the service that intends to
                                     send a broadcast message.
@param[in]  msg_id                   Message ID for this particular message.
@param[in]  ind_c_struct             C data structure for this broadcast
                                     indication.
@param[in]  ind_c_struct_len         Size of the broadcast indication

@retval    QMI_CSI_NO_ERR            Success
@retval    QMI_CSI_.....             Look into the enumeration qmi_csi_error for
                                     the error values.
*/
/*=========================================================================*/

qmi_csi_error
qmi_csi_send_broadcast_ind
(
    qmi_csi_service_handle   service_provider,
 unsigned int             msg_id,
    void                     *ind_c_struct,
 unsigned int             ind_c_struct_len
);

/*=============================================================================
  FUNCTION  qmi_csi_send_broadcast_ind_raw
=============================================================================*/
/*!
@brief
  Sends a raw broadcast indication to all registered clients

@param[in]  service_provider         Handle used by QCSI infrastructure
                                     to identify the service that intends to
                                     send a broadcast message.
@param[in]  msg_id                   Message ID for this particular message.
@param[in]  ind_buf                  broadcast indication buffer
@param[in]  ind_buf_len              Size of the broadcast indication

@retval    QMI_CSI_NO_ERR            Success
@retval    QMI_CSI_.....             Look into the enumeration qmi_csi_error for
                                     the error values.
*/
/*=========================================================================*/

qmi_csi_error
qmi_csi_send_broadcast_ind_raw
(
 qmi_csi_service_handle   service_provider,
 unsigned int             msg_id,
 void                     *ind_buf,
 unsigned int             ind_buf_len
 );

/*=============================================================================
  FUNCTION  qmi_csi_unregister
=============================================================================*/
/*!
@brief
  Unregisters a server.

@param[in]  service_provider         Handle given in the qmi_csi_register by
                                     the service.
@retval     QMI_CSI_NO_ERR           Success
@retval     QMI_CSI_.....            Look into the enumeration qmi_csi_error for
                                     the error values.
*/
/*=========================================================================*/
qmi_csi_error
qmi_csi_unregister
(
    qmi_csi_service_handle service_provider
);

/*=============================================================================
  PRIVATE qmi_csi_options_struct
=============================================================================*/
/*!
@brief
  Provide storage class for the options structure. This structure should not
  be directly manipulated. Please use the QMI_CSI_OPTIONS_* macros.
*/
/*=========================================================================*/
struct qmi_csi_options_struct
{
  unsigned int        options_set;
  unsigned int        instance_id;
  unsigned int        max_outstanding_inds;
  uint64_t            scope;
  qmi_csi_process_req raw_request_cb;
  qmi_csi_process_req pre_request_cb;
  qmi_csi_resume_ind  resume_ind_cb;
};

#ifdef __cplusplus
}
#endif
#endif /* QMI_CSI_H */
