#ifndef QMI_QMUX_IF_PLATFORM_H
#define QMI_QMUX_IF_PLATFORM_H
/******************************************************************************
  @file    qmi_qmux.c
  @brief   The QMI QMUX layer

  DESCRIPTION
  Interface definition for QMI QMUX layer

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

  ---------------------------------------------------------------------------
  Copyright (c) 2008-2015 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_i.h"
#include <stdint.h>

typedef struct
{
  int total_msg_size;
  int qmux_client_id;
} linux_qmi_qmux_if_platform_hdr_type;


#define QMI_QMUX_IF_PLATFORM_SPECIFIC_HDR_SIZE  sizeof (linux_qmi_qmux_if_platform_hdr_type)

#if defined(FEATURE_QMI_TEST)

#define QMI_QMUX_IF_CONN_SOCKET_PATH              "/tmp/data/qmux_connect_socket"
#define QMI_QMUX_IF_CLIENT_SOCKET_PATH            "/tmp/data/qmux_client_socket"

#define QMI_QMUX_IF_RADIO_CONN_SOCKET_PATH        QMI_QMUX_IF_CONN_SOCKET_PATH
#define QMI_QMUX_IF_RADIO_CLIENT_SOCKET_PATH      QMI_QMUX_IF_CLIENT_SOCKET_PATH

/* Server connection and client binding socket paths for clients belonging to audio group */
#define QMI_QMUX_IF_AUDIO_CONN_SOCKET_PATH        QMI_QMUX_IF_CONN_SOCKET_PATH
#define QMI_QMUX_IF_AUDIO_CLIENT_SOCKET_PATH      QMI_QMUX_IF_CLIENT_SOCKET_PATH

/* Server connection and client binding socket paths for clients belonging to bluetooth group */
#define QMI_QMUX_IF_BLUETOOTH_CONN_SOCKET_PATH    QMI_QMUX_IF_CONN_SOCKET_PATH
#define QMI_QMUX_IF_BLUETOOTH_CLIENT_SOCKET_PATH  QMI_QMUX_IF_CLIENT_SOCKET_PATH

#define QMI_QMUX_IF_GPS_CONN_SOCKET_PATH          QMI_QMUX_IF_CONN_SOCKET_PATH
#define QMI_QMUX_IF_GPS_CLIENT_SOCKET_PATH        QMI_QMUX_IF_CLIENT_SOCKET_PATH

#define QMI_QMUX_IF_NFC_CONN_SOCKET_PATH          QMI_QMUX_IF_CONN_SOCKET_PATH
#define QMI_QMUX_IF_NFC_CLIENT_SOCKET_PATH        QMI_QMUX_IF_CLIENT_SOCKET_PATH

#elif defined(FEATURE_QMI_ANDROID)

/* Server connection and client binding socket paths for clients belonging to audio group */
#define QMI_QMUX_IF_AUDIO_CONN_SOCKET_PATH    "/dev/socket/qmux_audio/qmux_connect_socket"
#define QMI_QMUX_IF_AUDIO_CLIENT_SOCKET_PATH  "/dev/socket/qmux_audio/qmux_client_socket"

/* Server connection and client binding socket paths for clients belonging to bluetooth group */
#define QMI_QMUX_IF_BLUETOOTH_CONN_SOCKET_PATH    "/dev/socket/qmux_bluetooth/qmux_connect_socket"
#define QMI_QMUX_IF_BLUETOOTH_CLIENT_SOCKET_PATH  "/dev/socket/qmux_bluetooth/qmux_client_socket"

/* Server connection and client binding socket paths for clients belonging to radio group */
#define QMI_QMUX_IF_RADIO_CONN_SOCKET_PATH    "/dev/socket/qmux_radio/qmux_connect_socket"
#define QMI_QMUX_IF_RADIO_CLIENT_SOCKET_PATH  "/dev/socket/qmux_radio/qmux_client_socket"

/* Server connection and client binding socket paths for clients belonging to gps group */
#define QMI_QMUX_IF_GPS_CONN_SOCKET_PATH      "/dev/socket/qmux_gps/qmux_connect_socket"
#define QMI_QMUX_IF_GPS_CLIENT_SOCKET_PATH    "/dev/socket/qmux_gps/qmux_client_socket"

/* Server connection and client binding socket paths for clients belonging to gps group */
#define QMI_QMUX_IF_NFC_CONN_SOCKET_PATH      "/dev/socket/qmux_nfc/qmux_connect_socket"
#define QMI_QMUX_IF_NFC_CLIENT_SOCKET_PATH    "/dev/socket/qmux_nfc/qmux_client_socket"

/* Default to the radio group */
#define QMI_QMUX_IF_CONN_SOCKET_PATH          QMI_QMUX_IF_RADIO_CONN_SOCKET_PATH
#define QMI_QMUX_IF_CLIENT_SOCKET_PATH        QMI_QMUX_IF_RADIO_CLIENT_SOCKET_PATH

#else

#define QMI_QMUX_IF_CONN_SOCKET_PATH          "/var/qmux_connect_socket"
#define QMI_QMUX_IF_CLIENT_SOCKET_PATH        "/var/qmux_client_socket"

#define QMI_QMUX_IF_RADIO_CONN_SOCKET_PATH    QMI_QMUX_IF_CONN_SOCKET_PATH
#define QMI_QMUX_IF_RADIO_CLIENT_SOCKET_PATH  QMI_QMUX_IF_CLIENT_SOCKET_PATH

#endif

#define QMI_PLATFORM_MAX_RETRIES      (120)
#define QMI_PLATFORM_INFINITE_RETRIES (UINT32_MAX)

extern int
linux_qmi_qmux_if_client_init
(
  qmi_qmux_clnt_id_t  *qmux_client_id,
  unsigned char       *rx_buf,
  int                 rx_buf_size
);

extern int
linux_qmi_qmux_if_client_release
(
  qmi_qmux_clnt_id_t  qmux_client_id
);

extern int
linux_qmi_qmux_if_client_tx_msg
(
  qmi_qmux_clnt_id_t  qmux_client_id,
  unsigned char       *msg,
  int                 msg_len
);


extern int
linux_qmi_qmux_if_server_tx_msg
(
  qmi_qmux_clnt_id_t  qmux_client_id,
  unsigned char       *msg,
  int                 msg_len
);

extern int
linux_qmi_qmux_if_server_validate_client_msg
(
  qmi_qmux_clnt_id_t   qmux_client_id,
  qmi_service_id_type  srvc_id
);

extern void
linux_qmi_qmux_if_reinit_connection
(
  qmi_connection_id_type  conn_id
);

extern void
linux_qmi_qmux_if_server_log_thread_state
(
   void
);

#define QMI_QMUX_IF_PLATFORM_CLIENT_INIT(client,rx_buf,buf_siz) \
     linux_qmi_qmux_if_client_init (client,rx_buf,buf_siz)

#define QMI_QMUX_IF_PLATFORM_CLIENT_RELEASE(client) \
     linux_qmi_qmux_if_client_release (client)

#define QMI_QMUX_IF_PLATFORM_REINIT_CONN(conn_id) \
     linux_qmi_qmux_if_reinit_connection(conn_id)

#define QMI_QMUX_IF_PLATFORM_TX_MSG(client,msg,msg_len) \
     linux_qmi_qmux_if_client_tx_msg (client,msg,msg_len)

#define QMI_QMUX_IF_PLATFORM_RX_MSG(client,msg,msg_len) \
     linux_qmi_qmux_if_server_tx_msg (client,msg,msg_len)

#define QMI_QMUX_IF_PLATFORM_VALIDATE_CLIENT_MSG(client, srvc_id) \
     linux_qmi_qmux_if_server_validate_client_msg(client, srvc_id)

#define QMI_QMUX_IF_PLATFORM_LOG_THREAD_STATE() \
     linux_qmi_qmux_if_server_log_thread_state()

#endif
