/*
** Copyright (c) 2007-2014 Qualcomm Technologies, Inc. All Rights Reserved.
** Qualcomm Technologies Proprietary and Confidential.
*/
#ifndef QMI_QMUX_IF_H
#define QMI_QMUX_IF_H

#include "qmi_i.h"

#define QMI_CTL_SERVICE 0

#define QMI_QMUX_IF_INVALID_HNDL  (NULL)
typedef void * qmi_qmux_if_hndl_t;

/* QMUX client communication mode */
typedef enum
{
  QMI_QMUX_IF_CLNT_MODE_NORMAL,
  QMI_QMUX_IF_CLNT_MODE_RAW
} qmi_qmux_if_clnt_mode_t;


/* This cmd/rsp data will be sent between client and QMUX for CTL
** and other control related messages
*/

typedef struct {
    unsigned char   qmi_svc_type;   /*  QMI service type */
    unsigned short  major_ver;  /*  Major version number */
    unsigned short  minor_ver;  /*  Minor version number */
}qmi_version_type;


typedef struct
{
    int               qmi_service_version_len;
    qmi_version_type  qmi_service_version[QMI_MAX_SERVICE_VERSIONS];
    unsigned char     addendum_label_len;
    unsigned char     addendum_label[QMI_MAX_ADDENDUM_LABEL];
    int               qmi_addendum_instances;
    qmi_version_type  addendum_version_list[QMI_MAX_SERVICE_VERSIONS];
}qmi_service_version_list_type;


typedef union
{
  struct
  {
    qmi_data_format_qos_hdr_state_type  qos_hdr_state;
    qmi_link_layer_protocol_type        link_protocol;
  } qmi_qmux_if_set_data_format_req;

  struct
  {
    qmi_link_layer_protocol_type        link_protocol;
  } qmi_qmux_if_set_data_format_rsp;

  struct
  {
    qmi_service_id_type service_id;
  } qmi_qmux_if_alloc_client_req;

  struct
  {
    qmi_client_id_type   new_client_id;
    qmi_service_id_type  service_id;
  } qmi_qmux_if_alloc_client_rsp;

  struct
  {
    qmi_service_id_type  delete_service_id;
    qmi_client_id_type   delete_client_id;
  } qmi_qmux_if_release_client_req;

  struct
  {
    qmi_qmux_clnt_id_t       qmux_client_id;
    qmi_qmux_if_clnt_mode_t  qmux_client_mode;
  } qmi_qmux_add_delete_qmux_client_req_rsp;

  struct
  {
    qmi_pwr_report_type  report_state;
  } qmi_qmux_if_reg_pwr_save_mode_req;

  struct
  {
    int curr_pwr_state_hndl;
    int prev_pwr_state_hndl;
  } qmi_qmux_if_pwr_state_ind;

  struct
  {
    int                   pwr_state_hndl;
    qmi_service_id_type   service_id;
    int                   num_indication_ids;
    unsigned short        indication_ids[QMI_MAX_PWR_INDICATIONS];
  } qmi_qmux_if_config_pwr_save_settings_req;

  struct
  {
    unsigned long new_pwr_state;
  } qmi_qmux_if_set_pwr_save_mode_req;

  struct
  {
    unsigned long curr_pwr_state;
  } qmi_qmux_if_get_pwr_save_mode_rsp;

  struct
  {
    qmi_connection_id_type conn_id;
  } qmi_qmux_if_sync_ind;
/*
  struct
  {
    qmi_connection_id_type conn_id;
  } qmi_qmux_if_modem_service_ind;
*/
  qmi_service_version_list_type qmi_qmux_if_get_version_info_rsp;

  struct
  {
    int                 is_valid;
    qmi_qmux_clnt_id_t  qmux_client_id;
    qmi_connection_id_type conn_id;
  } qmi_qmux_if_sub_sys_restart_ind;

  struct
  {
    qmi_service_id_type service_id;
  } qmi_qmux_if_reg_srvc_req;

  struct
  {
    qmi_connection_id_type conn_id;
    int write_err_code;
  }qmi_qmux_if_port_write_failed_ind;

} qmi_qmux_if_cmd_rsp_type;

typedef void (*qmi_qmux_if_rx_msg_hdlr_type)( qmi_connection_id_type  conn_id,
                                                qmi_service_id_type     service_id,
                                                qmi_client_id_type      client_id,
                                                unsigned char           control_flags,
                                                unsigned char           *rx_msg,
                                                int                     rx_msg_len);

/*===========================================================================
  FUNCTION  qmi_qmux_if_alloc_service_client
===========================================================================*/
/*!
@brief
  Calls the QMUX layer to add a service client

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
extern int
qmi_qmux_if_alloc_service_client
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      *client_id,
  int                     *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qmux_if_release_service_client
===========================================================================*/
/*!
@brief
  Calls the QMUX layer to remove a service client

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
extern int
qmi_qmux_if_release_service_client
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      client_id,
  int                     *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qmux_if_set_data_format
===========================================================================*/
/*!
@brief
  Calls the QMUX layer to set the data format of the port associated
  with the input connection ID.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise.
  If QMI_SERVICE_ERR, then the qmi_err_code will be valid and contain
  the error code returned by QMI on the modem.

@note

  - Side Effects
    -

*/
/*=========================================================================*/
extern int
qmi_qmux_if_set_data_format
(
  qmi_qmux_if_hndl_t                  qmux_if_client_handle,
  qmi_connection_id_type              conn_id,
  qmi_data_format_qos_hdr_state_type  qos_hdr_state,
  qmi_link_layer_protocol_type        *link_protocol,
  int                                 *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qmux_if_is_conn_active
===========================================================================*/
/*!
@brief
  Calls the QMUX layer to query the active state of a connection

@return
  TRUE if connection is active, FALSE otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
extern int
qmi_qmux_if_is_conn_active
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id
);


/*===========================================================================
  FUNCTION  qmi_qmux_if_send_qmi_msg
===========================================================================*/
/*!
@brief
  Sends a QMI message for the connection/service/client specified

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
extern int
qmi_qmux_if_send_qmi_msg
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      client_id,
  unsigned char           *msg_buf,
  int                     msg_buf_size
);




/*===========================================================================
  FUNCTION  qmi_qmux_if_open_connection
===========================================================================*/
/*!
@brief
  Opens up a new connection for the QMUX client

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
extern int
qmi_qmux_if_open_connection
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id
);


/*===========================================================================
  FUNCTION  qmi_qmux_if_pwr_up_init
===========================================================================*/
/*!
@brief
  Starts up a new "QMUX" client.  There can be multiple QMUX clients per PD.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

  Upon successful return, qmi_qmux_handle parameter will contain a handle
  that will need to be retained and passed into the qmi_qmux_if_pwr_down_release
  function.

@note

  - Side Effects
    -

*/
/*=========================================================================*/
extern int
qmi_qmux_if_pwr_up_init
(
  qmi_qmux_if_rx_msg_hdlr_type  rx_msg_hdlr,
  qmi_sys_event_rx_hdlr         sys_event_rx_hdlr,
  void                          *sys_event_user_data,
  qmi_qmux_if_hndl_t            *qmi_qmux_handle
);


/*===========================================================================
  FUNCTION  qmi_qmux_if_pwr_up_init_ex
===========================================================================*/
/*!
@brief
  Starts up a new "QMUX" client.  There can be multiple QMUX clients per PD.
  If the mode parameter is set to QMI_QMUX_IF_CLNT_MODE_RAW then all the CTL
  responses and indications will be received raw (QMUX SDU) via the
  rx_msg_hdlr.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

  Upon successful return, qmi_qmux_handle parameter will contain a handle
  that will need to be retained and passed into the qmi_qmux_if_pwr_down_release
  function.

@note

  - Side Effects
    -

*/
/*=========================================================================*/
extern int
qmi_qmux_if_pwr_up_init_ex
(
  qmi_qmux_if_rx_msg_hdlr_type  rx_msg_hdlr,
  qmi_sys_event_rx_hdlr         sys_event_rx_hdlr,
  void                          *sys_event_user_data,
  qmi_qmux_if_hndl_t            *qmi_qmux_handle,
  qmi_qmux_if_clnt_mode_t       mode
);


/*===========================================================================
  FUNCTION  qmi_qmux_if_pwr_down_release
===========================================================================*/
/*!
@brief
  Called on client shutdown.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
extern int
qmi_qmux_if_pwr_down_release
(
  qmi_qmux_if_hndl_t  qmux_if_client_handle
);

/*===========================================================================
  FUNCTION  qmi_qmux_if_rx_hdlr
===========================================================================*/
/*!
@brief
  QMI QMUX service RX handler.  This is to be called by QMUX layer to
  report incoming message.

@return
  None.

@note

  - Side Effects
    -

*/
/*=========================================================================*/
extern void
qmi_qmux_if_rx_msg
(
  unsigned char *msg,
  int            msg_len
);

/*===========================================================================
  FUNCTION  qmi_qmux_if_reg_pwr_save_mode
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
qmi_qmux_if_reg_pwr_save_mode
(
  qmi_qmux_if_hndl_t   qmux_if_client_handle,
  qmi_pwr_report_type  report_state,
  int                  *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_qmux_if_config_pwr_save_settings
===========================================================================*/
/*!
@brief
  Configures the power state indication filter for each connection.


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
qmi_qmux_if_config_pwr_save_settings
(
  qmi_qmux_if_hndl_t   qmux_if_client_handle,
  int                  pwr_state_hndl,
  qmi_service_id_type  service_id,
  int                  num_indication_ids,
  unsigned short       indication_ids[],
  int                  *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_qmux_if_set_pwr_state
===========================================================================*/
/*!
@brief
  Sets power state for each connection.


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
qmi_qmux_if_set_pwr_state
(
  qmi_qmux_if_hndl_t  qmux_if_client_handle,
  unsigned long       pwr_state,
  int                 *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qmux_if_get_pwr_state
===========================================================================*/
/*!
@brief
  Gets power state for specified connection.


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
qmi_qmux_if_get_pwr_state
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id,
  unsigned long           *pwr_state,
  int                     *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qmux_if_send_if_msg_to_qmux
===========================================================================*/
/*!
    @brief
    Send QMI message to QMUX.  Synchronous timeout units are seconds.

    @return
    None
*/
/*=========================================================================*/
extern int qmi_qmux_if_send_if_msg_to_qmux
(
  qmi_qmux_if_hndl_t        qmux_if_client_handle,
  qmi_qmux_if_msg_id_type   msg_id,
  qmi_connection_id_type    conn_id,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  int                       *qmi_err_code,
  unsigned int              timeout
);


/*===========================================================================
  FUNCTION  qmi_qmux_if_qmi_proxy_send_to_qmux
===========================================================================*/
/*!
    @brief
    Send QMI Proxy message to QMUX.

    @return
    None
*/
/*=========================================================================*/
extern int qmi_qmux_if_qmi_proxy_send_to_qmux
(
  qmi_qmux_if_hndl_t       qmux_if_client_handle,
  qmi_qmux_if_msg_id_type  msg_id,
  unsigned long            qmux_txn_id,
  qmi_connection_id_type   qmi_conn_id,
  qmi_service_id_type      qmi_service_id,
  qmi_client_id_type       qmi_client_id,
  unsigned char            control_flags,
  int                      sys_err_code,
  int                      qmi_err_code,
  unsigned char            *msg,
  int                      msg_len
);

/*===========================================================================
  FUNCTION  qmi_qmux_if_get_version_list
===========================================================================*/
/*!
@brief
  Query modem for QMI service version list, returning specified
  service information.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
extern int
qmi_qmux_if_get_version_list
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  unsigned short          *major_ver,  /*  Major version number */
  unsigned short          *minor_ver,  /*  Minor version number */
  int                     *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qmux_if_reg_service_avail
===========================================================================*/
/*!
@brief
  This function is used to register for service availability indication
  from modem.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    - None
*/
/*=========================================================================*/
int
qmi_qmux_if_reg_srvc_avail
(
  qmi_qmux_if_hndl_t     qmux_if_client_handle,
  qmi_connection_id_type conn_id,
  qmi_service_id_type    service_id,
  int                    *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qmux_if_send_raw_qmi_cntl_msg
===========================================================================*/
/*!
@brief
  Send a raw QMI control message to the modem. A QMUX SDU is expected in
  the raw message.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    - None

*/
/*=========================================================================*/
extern int
qmi_qmux_if_send_raw_qmi_cntl_msg
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id,
  unsigned char           *msg,
  int                     msg_len
);

#endif /* QMI_QMUX_IF_H */
