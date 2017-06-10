/*!
  @file
  qcril_uim_remote_client_socket.h

  @brief
  defines UIM remote client socket APIs
    1. start the socket
    2. send messages(resp/ind) to the service on the app layer
    3. send unsol resp to the service on the app layer

*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef QCRIL_UIM_REMOTE_CLIENT_SOCKET_H
#define QCRIL_UIM_REMOTE_CLIENT_SOCKET_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "qcrili.h"
#include "uim_remote_client.pb.h"


#define QCRIL_UIM_REMOTE_CLIENT_SOCKET_MAX_BUF_SIZE (1024*8)

void qcril_uim_remote_client_socket_init();

void qcril_uim_remote_client_socket_send(bool has_token, RIL_Token token,
                                  com_qualcomm_uimremoteclient_MessageType type,
                                  com_qualcomm_uimremoteclient_MessageId message_id,
                                  bool has_error,
                                  com_qualcomm_uimremoteclient_Error error,
                                  const void* msg, int msg_len);

void qcril_uim_remote_client_socket_send_empty_payload_unsol_resp(com_qualcomm_uimremoteclient_MessageId msg_id);

#ifdef  __cplusplus
}
#endif

#endif /* QCRIL_UIM_REMOTE_CLIENT_SOCKET_H */
