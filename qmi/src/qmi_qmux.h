#ifndef QMI_QMUX_H
#define QMI_QMUX_H

/******************************************************************************
  @file    qmi_qmux.c
  @brief   The QMI QMUX layer

  DESCRIPTION
  Interface definition for QMI QMUX layer

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

  ---------------------------------------------------------------------------
  Copyright (c) 2007-2012, 2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_i.h"

/* QMUX receive callback function used by qmux client */
typedef void (*qmi_qmux_if_rx_msg_cb_type) (int                       qmux_client_id,
                                              qmi_connection_id_type    conn_id,
                                              qmi_service_id_type       service_id,
                                              qmi_client_id_type        client_id,
                                              unsigned char             control_flags,
                                              unsigned char             *msg,
                                              int                       msg_len);


/* Typedef used by platform specific I/O layer to report incoming data */
typedef void (*qmi_qmux_io_platform_rx_cb_ptr)
(
  qmi_connection_id_type  conn_id,
  unsigned char           *rx_buf_ptr,
  int                     rx_buf_len
);


typedef enum
{
  QMI_QMUX_IO_PORT_READ_ERR_MODEM_RESET_EVT, /* Read error due to modem reset */
  QMI_QMUX_IO_PORT_WRITE_FAILED_EVT,
  QMI_QMUX_IO_PORT_READ_ERR_UNKNOWN_EVT,     /* Read error, unknown cause */
  QMI_QMUX_IO_PORT_READ_ERR_CLEARED_EVT,     /* Read error cleared */
} qmi_qmux_io_platform_event_type;

typedef union
{
  struct
  {
    qmi_connection_id_type conn_id;
    int write_err_code;
  }qmi_qmux_io_platform_write_failed_err;
} qmi_qmux_io_platform_event_info_type;

/* Typedef used by platform specific I/O layer to report events */
typedef void (*qmi_qmux_io_platform_event_cb_ptr)
(
  qmi_connection_id_type                conn_id,
  qmi_qmux_io_platform_event_type       event,
  qmi_qmux_io_platform_event_info_type  *event_info
);


/*===========================================================================
  FUNCTION  qmi_qmux_tx_msg
===========================================================================*/
/*!
@brief
  Function to send a QMUX PDU via the SMD control port.  This function
  will add on the appropriate QMUX header to the PDU.  It is assumed
  that space has been pre-allocated in the PDU for the header.

@return
  0 if function is successful, negative value if not.

@note

  - Connection is assumed to be opened with valid data before this
  function starts to execute

  - Side Effects
    -

*/
/*=========================================================================*/
extern int
qmi_qmux_tx_msg
(
  qmi_qmux_clnt_id_t  qmux_client_id,
  unsigned char       *msg,
  int                 msg_len
);


/*===========================================================================
  FUNCTION  qmi_qmux_open_connection
===========================================================================*/
/*!
@brief
  Function to open a QMI QMUX control port connection.  Function takes
  two parameters:  The connection ID of the connection to open, and a callback
  pointer to a function that will be called when messages are received
  (responses and indications)

@return
  0 if function is successful, negative value if not.

@note

  -

  - Side Effects
    -

*/
/*=========================================================================*/
int qmi_qmux_open_connection
(
  qmi_connection_id_type   conn_id,
  qmi_qmux_open_mode_type  mode
);

/*===========================================================================
  FUNCTION  qmi_qmux_close_connection
===========================================================================*/
/*!
@brief
  Function to close a QMI QMUX control port connection.  Function takes
  two parameters:  The connection ID of the connection to close

@return
  0 if function is successful, negative value if not.

@note

  -

  - Side Effects
    -

*/
/*=========================================================================*/
int qmi_qmux_close_connection
(
  qmi_connection_id_type  conn_id
);

/*===========================================================================
  FUNCTION  qmi_qmux_is_connection_active
===========================================================================*/
/*!
@brief
  Routine to query whether or not a specific connection is active

@return
  TRUE if connection is ACTIVE, FALSE otherwise

@note

  - Side Effects
    - None

*/
/*=========================================================================*/
extern int
qmi_qmux_is_connection_active
(
  qmi_connection_id_type  conn_id
);

/*===========================================================================
  FUNCTION  qmi_qmux_delete_qmux_client
===========================================================================*/
/*!
@brief


@return

@note

  - Side Effects
    -

*/
/*=========================================================================*/
extern void
qmi_qmux_delete_qmux_client
(
  int qmux_client_id
);

/*===========================================================================
  FUNCTION  qmi_qmux_pwr_up_init
===========================================================================*/
/*!
@brief
  Function to initialize the QMI QMUX layer.  Should only be called once.

@return
  0 if function is successful, negative value if not.

@note
  - Side Effects
    Initializes QMI QMUX subsystem
*/
/*=========================================================================*/
extern int
qmi_qmux_pwr_up_init (void);

/*===========================================================================
  FUNCTION  qmi_qmux_disable_port
===========================================================================*/
/*!
@brief
  Function to disable/enable a port from being used.  If called with TRUE
  argument, port will be disabled an not available for any use.  If called
  with FALSE argument, port will be re-enabled and available for normal
  operations.  NOTE:  Ports will be enabled by default.  This function must
  only be used if you wish to disable a port or (possibly) re-enable later.
  Also, re-enabling a disabled port will not open the port....
  qmux_open_connection would still need to be called.  The intent of this
  function is to be called at startup to permanently disable ports that are
  not valid in a particular configuration

@return
  0 if function is successful, negative value if not.

@note
  - Side Effects
    May permanently disable a port from being used.
*/
/*=========================================================================*/
extern int
qmi_qmux_disable_port
(
  qmi_connection_id_type  conn_id,
  char                   *conn_id_str,
  int                     disable
);

#endif /* QMI_QMUX_H */
