#ifndef QMI_QMUX_IO_PLATFORM_H
#define QMI_QMUX_IO_PLATFORM_H

/******************************************************************************
  @file    qmi_platform_qmux_io.h
  @brief   The QMI QMUX generic platform layer hearder file

  DESCRIPTION
  Interface definition for QMI QMUX platform layer.  This file will pull in
  the appropriate platform header file(s).

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

  ---------------------------------------------------------------------------
  Copyright (c) 2007,2014-2015 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_i.h"
#include "qmi_qmux.h"
#include "qmi_platform_config.h"
#include <time.h>

#define QMI_INVALID_TIMERID (0)

/* Data kept for each connection ID */
typedef struct
{
  char                  port_id_name[QMI_DEVICE_NAME_SIZE];
  pthread_t             th_id;
  int                   f_desc;
  unsigned char         *rx_buf;
  int                   rx_buf_len;
  unsigned long         flags;
  timer_t               timer_id;
} linux_qmi_qmux_io_conn_info_type;

extern linux_qmi_qmux_io_conn_info_type linux_qmi_qmux_io_conn_info[];


/******************************************************************************
  @file    qmi_qmux.c
  @brief   The QMI QMUX layer

  DESCRIPTION
  Interface definition for QMI QMUX layer

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

  ---------------------------------------------------------------------------
  Copyright (c) 2008 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

/*===========================================================================
  FUNCTION  linux_qmi_qmux_io_send_qmi_msg
===========================================================================*/
/*!
@brief
  Function to send a QMUX PDU on the Linux platform

@return
  0 if function is successful, negative value if not.

@note

  - Connection must have been previously opened.

  - Side Effects
    - Sends a QMUX PDU to modem processor

*/
/*=========================================================================*/
extern int
linux_qmi_qmux_io_send_qmi_msg
(
  qmi_connection_id_type  conn_id,
  unsigned char           *msg_ptr,
  int                     msg_len
);


/*===========================================================================
  FUNCTION  linux_qmi_qmux_io_open_conn
===========================================================================*/
/*!
@brief
  Function used to open a connection.  This function must be called
  prior to sending any messages or receiving any indications

@return
  0 if function is successful, negative value if not.

@note
  - Side Effects
    - Opens up SMD port and spawns a thread for RX handling.

*/
/*=========================================================================*/
extern int
linux_qmi_qmux_io_open_conn
(
  qmi_connection_id_type  conn_id,
  unsigned char           *rx_buf,
  int                     rx_buf_len,
  qmi_qmux_open_mode_type mode
);

/*===========================================================================
  FUNCTION  linux_qmi_qmux_io_close_conn
===========================================================================*/
/*!
@brief
  Function used to close a connection.

@return
  0 if function is successful, negative value if not.

@note
  - Side Effects
    - Close up transport port and stops the RX thread.

*/
/*=========================================================================*/
extern int
linux_qmi_qmux_io_close_conn
(
  qmi_connection_id_type  conn_id
);

/*===========================================================================
  FUNCTION  linux_qmi_qmux_io_device_name
===========================================================================*/
/*!
@brief
  Function used obtain the device name for a given connection ID

@return
  device name if function is successful, NULL if not.

@note
  - Side Effects
    - None

*/
/*=========================================================================*/
extern const char *
linux_qmi_qmux_io_device_name
(
  qmi_connection_id_type  conn_id
);

/*===========================================================================
  FUNCTION  linux_qmi_qmux_io_pwr_up_init
===========================================================================*/
/*!
@brief
  Initialization function to be called once at power-up.  Must be called
  prior to calling the linux_qmi_qmux_io_open_conn()

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
linux_qmi_qmux_io_pwr_up_init
(
  qmi_qmux_io_platform_rx_cb_ptr      rx_cb_ptr,
  qmi_qmux_io_platform_event_cb_ptr   event_cb_ptr
);


/*===========================================================================
  FUNCTION  linux_qmi_qmux_io_start_watchdog_timer
===========================================================================*/
/*!
@brief
  Function used to start the given watchdog timer

@return
  0 if function is successful, negative value if not.

@note
  - Side Effects
    - None

*/
/*=========================================================================*/
int
linux_qmi_qmux_io_start_watchdog_timer
(
  qmi_connection_id_type  conn_id,
  timer_t                 *timer_id
);

/*===========================================================================
  FUNCTION  linux_qmi_qmux_io_stop_watchdog_timer
===========================================================================*/
/*!
@brief
  Function used to stop the given watchdog timer

@return
  0 if function is successful, negative value if not.

@note
  - Side Effects
    - None

*/
/*=========================================================================*/
int
linux_qmi_qmux_io_stop_watchdog_timer
(
  qmi_connection_id_type  conn_id,
  timer_t                 *timer_id
);

/* These macros are used in QMUX */
#define QMI_QMUX_IO_PLATFORM_SEND_QMI_MSG(conn_id,msg_buf,len) \
  linux_qmi_qmux_io_send_qmi_msg (conn_id,msg_buf,len)

#define QMI_QMUX_IO_PLATFORM_OPEN_CONN(conn_id, rx_buf, rx_buf_len,mode) \
  linux_qmi_qmux_io_open_conn (conn_id,rx_buf,rx_buf_len,mode)

#define QMI_QMUX_IO_PLATFORM_CLOSE_CONN(conn_id) \
  linux_qmi_qmux_io_close_conn (conn_id)

#define QMI_QMUX_IO_PLATFORM_DEV_NAME(conn_id) \
  linux_qmi_qmux_io_device_name (conn_id)

#define QMI_QMUX_IO_PLATFORM_PWR_UP_INIT(rx_cb_ptr,event_cb_ptr) \
  linux_qmi_qmux_io_pwr_up_init (rx_cb_ptr,event_cb_ptr)

#define QMI_QMUX_IO_PLATFORM_START_WATCHDOG_TIMER(conn_id,timer_id) \
  linux_qmi_qmux_io_start_watchdog_timer (conn_id,timer_id)

#define QMI_QMUX_IO_PLATFORM_STOP_WATCHDOG_TIMER(conn_id,timer_id) \
  linux_qmi_qmux_io_stop_watchdog_timer (conn_id,timer_id)

#endif /* QMI_QMUX_IO_PLATFORM_H */
