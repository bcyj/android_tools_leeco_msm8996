/* Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */
#include <stdint.h>

#if defined (FEATURE_QMI) || defined (DEBUG_X86)
//Use the following for QMI Target & general off-target compilation
#include "comdef.h"
#else
#include "oncrpc.h"
#endif

#include "gpsone_bit_api.h"

int gpsone_bit_forward_register(void);
int gpsone_bit_forward_deregister(void);
int gpsone_bit_forward_notify(gpsone_bit_session_handle_type session_handle, uint32_t transaction_id, gpsone_bit_event_payload_type * event_payload);
#ifdef FEATURE_QMI
int gpsone_bit_forward_qmi_init(void);
int gpsone_bit_forward_qmi_destroy(void);
#endif


gpsone_bit_status_e_type gpsone_bit_forward_open
(
  gpsone_bit_transport_handle_type    transport_handle,
  const gpsone_bit_open_params_type       *open_param
);

gpsone_bit_status_e_type gpsone_bit_forward_close
(
  gpsone_bit_transport_handle_type    transport_handle,
  const gpsone_bit_close_params_type       *close_param
);

gpsone_bit_status_e_type gpsone_bit_forward_connect
(
  gpsone_bit_transport_handle_type    transport_handle,
  uint32                            transaction_id,
  const gpsone_bit_connect_params_type    *connect_param
);


gpsone_bit_status_e_type gpsone_bit_forward_disconnect
(
  gpsone_bit_transport_handle_type    transport_handle,
  gpsone_bit_session_handle_type    session_handle,
  uint32                            transaction_id,
  const gpsone_bit_disconnect_params_type *disconnect_param
);

gpsone_bit_status_e_type gpsone_bit_forward_send
(
  gpsone_bit_transport_handle_type    transport_handle,
  gpsone_bit_session_handle_type    session_handle,
  uint32                            transaction_id,
  uint8                             *send_buf,
  uint32                            length
);

gpsone_bit_status_e_type gpsone_bit_forward_recv
(
  gpsone_bit_transport_handle_type    transport_handle,
  gpsone_bit_session_handle_type     session_handle,
  uint8                              *recv_buf,
  uint32                             max_buf_size,
  uint32                             *bytes_returned,
  uint32                             *bytes_leftover
);

gpsone_bit_status_e_type gpsone_bit_forward_ioctl
(
  gpsone_bit_transport_handle_type    transport_handle,
 gpsone_bit_session_handle_type     session_handle,
 uint32                             transaction_id,
 gpsone_bit_ioctl_e_type            ioctl_request,
 const gpsone_bit_ioctl_params_type       *ioctl_param
);
