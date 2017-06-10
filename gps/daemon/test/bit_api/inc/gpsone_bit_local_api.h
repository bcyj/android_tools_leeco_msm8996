#ifndef GPSONE_BIT_LOCAL_API_H
#define GPSONE_BIT_LOCAL_API_H

/*===========================================================================

      GPSONE BIT (BEARER-INDEPENDENT-TRANSPORT) LOCAL API HEADER FILE

DESCRIPTION
  This header file contains the BIT local APIs on modem processor
  (no RPC call is involved, only local calls on modem)

  This file should have no dependency on PDCOMM API/Task header files

Copyright (c) 2012 by Qualcomm Technologies, Inc. All Rights Reserved.

===========================================================================*/

/* <EJECT> */
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

 $Header: //source/qcom/qct/modem/gps/gnss/main/latest/pd_comms/inc/gpsone_bit_local_api.h#2 $$DateTime: 2010/06/25 10:06:13 $$Author: ruih $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/30/10    rh     Initial version

===========================================================================*/

#include "comdef.h"    /* Definition for basic types and macros */
#include "msg.h"
#include "gpsone_bit_api.h"

/************************************************************************************/
/*                                                                                  */
/*               Internal BIT constant/data structure definitions                   */
/*                                                                                  */
/************************************************************************************/

#define GPSONE_BIT_MAX_NUM_OF_TRANSPORT  1   /* we only support one AGPS BIT daemon now */

#define BIT_MSG_FUNC( str, a, b, c )      MSG_MED( "=BIT API= " str, a, b, c )
#define BIT_MSG_LOW( str, a, b, c )       MSG_LOW( "=BIT API= " str, a, b, c )
#define BIT_MSG_MED( str, a, b, c )       MSG_MED( "=BIT API= " str, a, b, c )
#define BIT_MSG_ERROR( str, a, b, c )     MSG_ERROR( "=BIT API= " str, a, b, c )
#define BIT_MSG_HIGH( str, a, b, c )      MSG_HIGH( "=BIT API= " str, a, b, c )
#define BIT_ERR_FATAL( str, a, b, c )     ERR_FATAL( "=BIT API= " str, a, b, c )

// lower 16 bits of transaction_id is q_handle_id, higher 16 bits is reserved for ioctl code
#define GPSONE_BIT_TRANSID_HANDLE(trans_id)      ( (trans_id) & 0xFFFF )
#define GPSONE_BIT_TRANSID_OPCODE(trans_id)      ( ((trans_id) & 0xFFFF) >> 16 )
#define GPSONE_BIT_MAKE_TRANSID(handle, opcode)  ( ((handle) & 0xFFFF) | (((opcode) & 0xFFFF) << 16) )

/* dummy transaction id */
#define GPSONE_BIT_DUMMY_TRANSID  (GPSONE_BIT_MAKE_TRANSID(0xFFFF, 0xFFFF))

/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_STATE_TYPE

DESCRIPTION
  enum which defines the BIT transport's global state (not per connection)
---------------------------------------------------------------------------*/
typedef enum
{
  GPSONE_BIT_STATE_UNINITIALIZED = 0,    /* uninitialized */
  GPSONE_BIT_STATE_UNREGISTERED,         /* initialized but unregistered */
  GPSONE_BIT_STATE_OPENING,              /* registered, open in progress */
  GPSONE_BIT_STATE_OPENED,               /* registered, open succeeded */
  GPSONE_BIT_STATE_CLOSING,              /* registered, close in progress */
  GPSONE_BIT_STATE_CLOSED,               /* registered, close succeeded or never opened */

  GPSONE_BIT_STATE_UNCHANGED,            /* a special enum for keeping old state */
}gpsone_bit_state_e_type;


/*---------------------------------------------------------------------------
TYPEDEF GPSONE_BIT_TRANSPORT_INFO_TYPE

DESCRIPTION
  data structure to define a particular BIT transport
---------------------------------------------------------------------------*/
typedef struct
{
  gpsone_bit_register_params_type                  reg_param;

  gpsone_bit_open_f_type                           *open_fp;
  gpsone_bit_close_f_type                          *close_fp;
  gpsone_bit_connect_f_type                        *connect_fp;
  gpsone_bit_disconnect_f_type                     *disconnect_fp;
  gpsone_bit_send_f_type                           *send_fp;
  gpsone_bit_recv_f_type                           *recv_fp;
  gpsone_bit_ioctl_f_type                          *ioctl_fp;
} gpsone_bit_transport_info_type;


/************************************************************************************/
/*                                                                                  */
/*                   Function prototypes called by PDCOMM TCP Task                  */
/*                                                                                  */
/************************************************************************************/

/*===========================================================================

FUNCTION: gpsone_bit_init

DESCRIPTION:
  This function is used by PDSM during startup to initialize BIT (mutex
  setup, etc)
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_init
(
 void
);

/*===========================================================================

FUNCTION: gpsone_bit_open

DESCRIPTION:
  This function is used to inform BIT daemon that PDSM will start using its
  service. If for some reason, BIT daemon fails to initialize, it should return
  GPSONE_BIT_STATUS_FAIL or other error code. Otherwise, it should return
  GPSONE_BIT_STATUS_WAIT to inform PDSM that BIT daemon is in the process of being
  brought up and open result will be sent asynchronously via gpsone_bit_notify
  with OPEN_RESULT event.

  If BIT daemon can finish the open operation immediately, it can return
  GPSONE_BIT_STATUS_SUCCESS immediately without further event notification.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_open
(
  gpsone_bit_open_params_type       *open_param
);


/*===========================================================================

FUNCTION: gpsone_bit_close

DESCRIPTION:
  This function is used to inform BIT daemon that PDSM will stop using its
  service. If for some reason, BIT daemon fails to close, it should return
  GPSONE_BIT_STATUS_FAIL or other error code. Otherwise, it should return
  GPSONE_BIT_STATUS_WAIT to inform PDSM that BIT daemon is in the process of
  being shut down and close result will be sent asynchronously via
  gpsone_bit_notify with CLOSE_RESULT event.

  If BIT daemon can finish the close operation immediately, it can return
  GPSONE_BIT_STATUS_SUCCESS immediately without further event notification.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_close
(
  gpsone_bit_close_params_type       *close_param
);


/*===========================================================================

FUNCTION: gpsone_bit_connect

DESCRIPTION:
  This function is used to establish end-to-end connection to a network server.
  If for some reason, end-to-end connection can not be established, it should
  return GPSONE_BIT_STATUS_FAIL or other error code. therwise, it should return
  GPSONE_BIT_STATUS_WAIT to inform PDSM that end-to-end connection is in the
  process of being established. Connect result should be sent asynchronously
  via gpsone_bit_notify with CONNECT_RESULT event.

  If BIT daemon can finish the connect operation immediately, it can return
  GPSONE_BIT_STATUS_SUCCESS immediately without further event notification.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_connect
(
  uint32                            transaction_id,
  gpsone_bit_connect_params_type    *connect_param
);


/*===========================================================================

FUNCTION: gpsone_bit_disconnect

DESCRIPTION:
  This function is used to tear down end-to-end connection to a network server.
  If for some reason, end-to-end connection can not be torn down, it should
  return GPSONE_BIT_STATUS_FAIL. Otherwise, it should return GPSONE_BIT_STATUS_WAIT
  to inform PDSM that end-to-end connection is in the process of being torn down.
  Disconnect result should be sent asynchronously via gpsone_bit_notify with
  CONNECT_RESULT event.

  If BIT daemon can finish the disconnect operation immediately, it can return
  GPSONE_BIT_STATUS_SUCCESS immediately without further event notification.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_disconnect
(
  gpsone_bit_session_handle_type    session_handle,
  uint32                            transaction_id,
  gpsone_bit_disconnect_params_type *disconnect_param
);


/*===========================================================================

FUNCTION: gpsone_bit_send

DESCRIPTION:
  This function is used by PDSM to send data to the network. It passes a pointer
  to a data buffer (*send_buf) and also specifies the size (in bytes) of the data buffer.
  If for some reason, BIT daemon is not able to process the send request,
  it should return GPSONE_BIT_STATUS_FAIL or other error code to inform PDSM that the
  send request has failed. Otherwise, it should return GPSONE_BIT_STATUS_WAIT and return
  result asynchronously via gpsone_bit_notify() with SEND_RESULT event.

  If BIT daemon can finish the send operation immediately, it can return
  GPSONE_BIT_STATUS_SUCCESS immediately without further event notification.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_send
(
  gpsone_bit_session_handle_type    session_handle,
  uint32                            transaction_id,
  uint8                             *send_buf,
  uint32                            length
);


/*===========================================================================

FUNCTION: gpsone_bit_recv

DESCRIPTION:
  This function is used by PDSM to receive data from BIT daemon, when it gets
  DATA_READY event from BIT daemon. Upon getting recv request, BIT daemon should
  copy incoming data to the data buffer pointed by *recv_buf. If incoming data
  size is larger than max_buff_size, it should only copy max_buf_size to the
  data buffer pointed by *recv_buf.

  BIT daemon should return the number of bytes copied to the data buffer and the
  bytes left in its data buffer. BIT daemon should return GPSONE_BIT_STATUS_SUCCESS
  when some bytes are succesfully returned, otherwise an error code should be returned.
  If no data available now, BIT daemon should return GPSONE_BIT_STATUS_NO_MORE_DATA.

  Please note that this is a synchronous call, there is no async event for recv(),
  BIT daemon should handle the recv operation in a non-blocking way.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_recv
(
  gpsone_bit_session_handle_type     session_handle,
  uint8                              *recv_buf,
  uint32                             max_buf_size,
  uint32                             *bytes_returned,
  uint32                             *bytes_leftover
);


/*===========================================================================

FUNCTION: pgpsone_bit_ioctl

DESCRIPTION:
  This function is used by PDSM to request ioctl operation on BIT daemon.
  If for some reason, BIT daemon can't do this ioctl operation, it should return
  GPSONE_BIT_STATUS_FAIL or other error code. Otherwise, it should return
  GPSONE_BIT_STATUS_WAIT to inform PDSM that the ioctl operation is in progress.
  Result will be sent back asynchronously via gpsone_bit_notify() later with
  IOCTL_RESULT event.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_ioctl
(
 gpsone_bit_session_handle_type     session_handle,
 uint32                             transaction_id,
 gpsone_bit_ioctl_e_type            ioctl_request,
 gpsone_bit_ioctl_params_type       *ioctl_param
);

/*===========================================================================
FUNCTION gpsone_bit_process_event

DESCRIPTION
  This function is called by PDCOMM TCP Task if some events need processing
  by BIT API layer within PDCOMM TCP Task context (mainly for OPEN_RESULT &
  CLOSE_RESULT)
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_process_event
(
  gpsone_bit_event_payload_type    *event_payload
);


/*===========================================================================
FUNCTION gpsone_bit_post_event

DESCRIPTION
  This function is called from the context of BIT daemon when a BIT
  event occurs(inside gpsone_bit_notify). PDCOMM TCP task needs to provide
  this function. The function simply creates an IPC message and sends it to
  PDCOMM APP so that it can process the BIT event.

  This is the only function calling from BIT daemon context into PDCOMM
  TCP task context
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_post_event
(
  gpsone_bit_session_handle_type         session_handle,
  uint32                                 transaction_id,
  const gpsone_bit_event_payload_type    *p_event_payload
);


#endif /* GPSONE_BIT_LOCAL_API_H */

