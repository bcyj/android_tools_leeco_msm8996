/*
** Copyright (c) 2007-2012,2014 Qualcomm Technologies, Inc. All Rights Reserved.
** Qualcomm Technologies Proprietary and Confidential.
*/
#ifndef QMI_SERVICE_H
#define QMI_SERVICE_H

#include "qmi_i.h"
#include "qmi_client.h"
#include "qmi_util.h"
#include "qmi_qmux_if.h"


#define QMI_SRVC_CLIENT_HANDLE_TO_CONN_ID(client_handle) \
  (qmi_connection_id_type) ((client_handle >> 24) & 0x7F)

#define QMI_SRVC_CLIENT_HANDLE_TO_CLIENT_ID(client_handle) \
  (qmi_client_id_type) ((client_handle >> 16) & 0xFF)

#define QMI_SRVC_CLIENT_HANDLE_TO_SERVICE_ID(client_handle) \
  (qmi_service_id_type)((client_handle >> 8) & 0xFF)


#define QMI_QCCI_APIS 1
#define QMI_OLD_APIS 0

typedef enum
{
  QMI_SERVICE_REQUEST_MSG,
  QMI_SERVICE_RESPONSE_MSG,
  QMI_SERVICE_INDICATION_MSG,
  QMI_SERVICE_ERROR_MSG
} qmi_service_msg_type;

typedef void (*qmi_client_decode_msg_async_cb)
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

/* Asynchronous callback function prototype.  Individual services will
** register functions of this prototype when they send an asynchronous
** message
*/
typedef void (*srvc_async_cb_fn_type)
(
  int                     user_handle,
  qmi_service_id_type     service_id,
  unsigned long           msg_id,
  int                     ret_code,
  int                     qmi_error_code,
  unsigned char           *reply_msg_data,
  int                     reply_msg_data_size,
  void                    *srvc_async_cb_fn_data,
  void                    *user_async_cb_fn,
  void                    *user_async_cb_data
);

/* Service indication callback function prototype.  Functions of this
** prototype will be registered with the common services rountines at
** initialization time.  The registered function will be called each
** time an indication message is recieved
*/
typedef void (*qmi_service_ind_rx_hdlr)
(
  int                     user_handle,
  qmi_service_id_type     service_id,
  unsigned long           msg_id,
  void                                *user_ind_msg_hdlr,
  void                                *user_ind_msg_hdlr_user_data,
  unsigned char           *rx_msg_buf,
  int                     rx_msg_len
);
typedef struct  qmi_service_txn_info_type *qmi_service_txn_info_type_ptr;


/* Asynchronous transaction information.  Pointer to a
** service-specific callback that will be called, as well as
** client callback functions that will be called by the service-specific
** callback after decoding the reply message
*/
typedef struct qmi_async_client_txn_info_type
{
  srvc_async_cb_fn_type   service_async_cb_fn;
  void                    *service_async_cb_data;
  void                    *user_async_cb_fn;
  void                    *user_async_cb_data;
  qmi_client_recv_raw_msg_async_cb  user_rsp_raw_cb;
  qmi_client_decode_msg_async_cb    user_decode_cb;
  void                              *user_decode_handle;
  void                              *user_buf;
  int                               user_buf_len;
} qmi_async_client_txn_info_type;


/* Synchronous transaction information.  The information needed
** is a pointer to a service-specific reply buffer,
** a pointer to indicate the size of the reply buffer, and a
** pointer to the returned message ID
*/
typedef struct
{
  unsigned char                   *user_reply_buf;
  int                             user_reply_buf_size;
  int                             rsp_rc;
  int                             qmi_err_code;
  QMI_PLATFORM_SIGNAL_DATA_TYPE   signal_data;
} qmi_sync_client_txn_info_type;

/* Whether or not a transaction should be synchronous
** or asynchronous
*/
typedef enum
{
  QMI_TXN_ASYNC,
  QMI_TXN_SYNC
} qmi_service_txn_type;


typedef struct
{
  qmi_client_id_type    client_id;
  unsigned long         txn_id;
} qmi_service_txn_cmp_type;

/* Transaction information.  This information will be filled out by
** service-specific functions and passed into the common service
** send message routine.  The service specific functions will determine
** whether the transaction is synchronous or asynchronous and fill out
** the relevant information
*/
typedef struct
{
  qmi_service_txn_type txn_type;
  union {
    qmi_sync_client_txn_info_type  sync;
    qmi_async_client_txn_info_type async;
  } sync_async;

} qmi_srvc_txn_info_type;


/**************************************************************************/
/* QMI service transaction structure */
typedef struct  qmi_service_txn_info_type
{
  qmi_txn_hdr_type         hdr;
  qmi_connection_id_type   conn_id;
  qmi_service_id_type      service_id;
  qmi_client_id_type       client_id;
  unsigned long            msg_id;
  unsigned long            txn_id;
  int                      api_flag;
  qmi_srvc_txn_info_type   srvc_txn_info;
} qmi_service_txn_info_type;

int qmi_service_add_decode_handle
(
    int  user_handle,
    void *user_decode_handle
);

int  qmi_service_setup_txn
(
  int                                user_handle,
  qmi_service_id_type                service_id,
  unsigned long                      msg_id,
  srvc_async_cb_fn_type              srvc_cb,
  void                               *srvc_cb_data,
  void                               *user_cb,
  void                               *user_cb_data,
  qmi_client_recv_raw_msg_async_cb   user_rsp_raw_cb,
  qmi_client_decode_msg_async_cb     user_decode_cb,
  void                               *user_decode_handle,
  void                               *user_buf,
  int                                user_buf_len,
  int                                api_flag,
  qmi_service_txn_info_type          **txn
);

void qmi_service_release_txn
(
    int                            user_handle,
    qmi_service_txn_info_type      *txn,
    unsigned long                  *tran_id,
    int                            err_code
);

int qmi_service_send_msg (
  qmi_connection_id_type    conn_id,
  qmi_service_id_type       service_id,
  qmi_client_id_type        client_id,
  unsigned long             msg_id,
  unsigned char             *msg_buf,
  int                       msg_buf_size,
  qmi_service_txn_info_type         *txn
);

/* Local function prototypes */
int qmi_service_send_msg_sync_millisec (
  int                       user_handle,
  qmi_service_id_type       service_id,
  unsigned long             msg_id,
  unsigned char             *msg_buf,
  int                       num_bytes_in_msg_buf,
  unsigned char             *reply_buf,
  int                       *reply_buf_size_ptr,
  int                       reply_buf_size,
  int                       timeout_milli_secs,
  int                       api_flag,
  int                       *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_service_pwr_down_release
===========================================================================*/
/*!
@brief
  Should be called once when process using the QMI library is started.
  No QMI functions can be sucessfully called until this function has been
  called.

@return
  0 if successful, negative number if not successful

@note

  - Dependencies
    - None

  - Side Effects
    - QMI connection is opened
*/
/*=========================================================================*/
extern int
qmi_service_pwr_up_init
(
  qmi_sys_event_rx_hdlr   event_rx_hdlr,
  void                    *event_user_data
);


/*===========================================================================
  FUNCTION  qmi_service_pwr_down_release
===========================================================================*/
/*!
@brief
  Should be called once when process using the QMI library is exiting

@return
  0 if successful, negative number if not successful

@note

  - Dependencies
    - None

  - Side Effects
    - QMI connection is opened
*/
/*=========================================================================*/
extern int
qmi_service_pwr_down_release
(
  void
);


/*===========================================================================
  FUNCTION  qmi_service_init
===========================================================================*/
/*!
@brief
  Initializes a QMI connection

@return
  0 if successful, negative number if not successful

@note

  - Dependencies
    - None

  - Side Effects
    - QMI connection is opened
*/
/*=========================================================================*/
extern int
qmi_service_connection_init
(
  qmi_connection_id_type  conn_id,
  int                     *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_service_set_srvc_functions
===========================================================================*/
/*!
@brief
  Routine that should be called once by each service to set the calback
  called to recieve indications, and service-specific write/read transaction
  header routines if they exist

@return
  QMI_NO_ERR if success, negative value if not

@note

  - Dependencies
    - None

  - Side Effects
    -

*/
/*=========================================================================*/
extern int
qmi_service_set_srvc_functions
(
  qmi_service_id_type                 service_id,
  qmi_service_ind_rx_hdlr             srvc_rx_ind_msg_hdlr
);

/*===========================================================================
  FUNCTION  qmi_service_init
===========================================================================*/
/*!
@brief
  Initializes a specified service on a specified connection.  Upon
  successful return, the service may be used on the connection.

@return
  0 if function is successful, negative value if not.

@note

  - Dependencies
    - None

  - Side Effects
    - Calls into CTL service to obtain a client ID.  This is a blocking
    call which will suspend the calling thread.
*/
/*=========================================================================*/
extern int
qmi_service_init
(
  qmi_connection_id_type             conn_id,
  qmi_service_id_type                service_id,
  void                               *user_ind_msg_hdlr,
  void                               *user_ind_msg_hdlr_user_data,
  int                                *qmi_err_code
);



/*===========================================================================
  FUNCTION  qmi_service_release
===========================================================================*/
/*!
@brief
  Sends a synchronous QMI service message on the specified connection.

@return
  0 if function is successful, negative value if not.

@note

  - Dependencies
    - None

  - Side Effects
    - If the transaction information passed with message indicates that
    it should be a synchronous transaction, the calling thread will block
    until response is received.
*/
/*=========================================================================*/
extern int
qmi_service_release
(
  int                   user_handle,
  int                   *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_service_send_msg_sync
===========================================================================*/
/*!
@brief
  Sends a synchronous QMI service message on the specified connection.

@return
  0 if function is successful, negative value if not.

@note

  - Dependencies
    - None

  - Side Effects
    - If the transaction information passed with message indicates that
    it should be a synchronous transaction, the calling thread will block
    until response is received.
*/
/*=========================================================================*/
extern int
qmi_service_send_msg_sync
(
  int                       user_handle,
  qmi_service_id_type       service_id,
  unsigned long             msg_id,
  unsigned char             *msg_buf,
  int                       num_bytes_in_msg_buf,
  unsigned char             *reply_buf,
  int                       *reply_buf_size_ptr,
  int                       reply_buf_size,
  int                       timeout_secs,
  int                       *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_service_send_msg_async
===========================================================================*/
/*!
@brief
  Sends an asynchronous QMI service message on the specified connection.

@return
  0 if function is successful, negative value if not.

@note

  - Dependencies
    - None

  - Side Effects
    - If the transaction information passed with message indicates that
    it should be a synchronous transaction, the calling thread will block
    until response is received.
*/
/*=========================================================================*/
extern int
qmi_service_send_msg_async
(
  int                       user_handle,
  qmi_service_id_type       service_id,
  unsigned long             msg_id,
  unsigned char             *msg_buf,
  int                       msg_buf_size,
  srvc_async_cb_fn_type     srvc_cb,
  void                      *srvc_cb_data,
  void                      *user_cb,
  void                      *user_cb_data
);



/*===========================================================================
  FUNCTION  qmi_service_validate_client_handle
===========================================================================*/
/*!
@brief
  Validates that a user handle is valid and belongs to the passed
  in service type

@return
  TRUE if handle is valid, FALSE if not

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
extern int
qmi_service_validate_client_handle
(
  int                  user_handle,
  qmi_service_id_type  service_id
);

/*===========================================================================
  FUNCTION  qmi_service_delete_async_txn
===========================================================================*/
/*!
@brief
  Deletes an asynchronous transaction so that it will free resources
  associated with transaction

@return
   QMI_NO_ERR if successful, negative otherwise
@note

  - Dependencies
    - None

  - Side Effects
    - Async response will not be delivered
*/
/*=========================================================================*/
extern int
qmi_service_delete_async_txn
(
  int user_handle,
  int async_txn_handle
);

/*===========================================================================
  FUNCTION  qmi_service_set_data_format
===========================================================================*/
/*!
@brief
    Sets the data format to the specified value for the specified
    connection ID.

@return
   QMI_NO_ERR if successful, negative otherwise
@note

  - Dependencies
    - None

  - Side Effects
    - Async response will not be delivered
*/
/*=========================================================================*/
extern int
qmi_service_set_data_format
(
  qmi_connection_id_type                conn_id,
  qmi_data_format_qos_hdr_state_type    qos_hdr_state,
  qmi_link_layer_protocol_type          *link_protocol,
  int                                   *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_service_reg_pwr_save_mode
===========================================================================*/
/*!
@brief
  This function is used to register/de-register for power state change
  events.  Calls relevant QMI_QMUX function


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
extern int
qmi_service_reg_pwr_save_mode
(
  qmi_pwr_report_type   report_state,
  int                   *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_service_config_pwr_save_settings
===========================================================================*/
/*!
@brief
  Configures the power state indication filter for each connection.
  Calls relevant QMI QMUX function.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
extern int
qmi_service_config_pwr_save_settings
(
  int                  pwr_state_hndl,
  qmi_service_id_type  service_id,
  int                  num_indication_ids,
  unsigned short       indication_ids[],
  int                  *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_service_set_pwr_state
===========================================================================*/
/*!
@brief
  Sets power state.  Calls relevant QMI QMUX function to do so


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    - Talks to modem processor
    - Modem will not send filtered indications until later power state change.
*/
/*=========================================================================*/
extern int
qmi_service_set_pwr_state
(
  unsigned long        pwr_state,
  int                  *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_service_get_pwr_state
===========================================================================*/
/*!
@brief
  Gets power state.  Calls relevant QMI QMUX function to do so.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
extern int
qmi_service_get_pwr_state
(
  const char       *dev_id,
  unsigned long    *pwr_state,
  int              *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_service_get_qmux_if_client_handle
===========================================================================*/
/*!
@brief
  Returns the QMUX IF client handle

@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
extern qmi_qmux_if_hndl_t
qmi_service_get_qmux_if_handle(void);

#endif /* QMI_SERVICE_H  */
