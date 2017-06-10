/*!
  @file
  qcril_uim_remote_client_packing.h

  @brief
   APIs to pack the uim remote client messages
*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#ifndef QCRIL_UIM_REMOTE_CLIENT_PACKING_H
#define QCRIL_UIM_REMOTE_CLIENT_PACKING_H

#include "uim_remote_client.pb.h"
#include "qcrili.h"

size_t qcril_uim_remote_client_pack_msg_and_tag(const void *msg, bool has_token, uint32_t token,
                                                com_qualcomm_uimremoteclient_MessageType type,
                                                com_qualcomm_uimremoteclient_MessageId message_id,
                                                bool has_error, com_qualcomm_uimremoteclient_Error error,
                                                uint8_t *buf, size_t buf_size);

com_qualcomm_uimremoteclient_MessageTag* qcril_uim_remote_client_unpack_msg_tag(const uint8_t *buf, size_t buf_size);

void qcril_uim_remote_client_parse_packed_msg(com_qualcomm_uimremoteclient_MessageType msg_type,
                                              com_qualcomm_uimremoteclient_MessageId msg_id,
                                              const pb_callback_t packed_msg, size_t msg_size,
                                              void** unpacked_msg, size_t*unpacked_msg_size_ptr,
                                              qcril_evt_e_type* event_ptr);

#endif /* QCRIL_UIM_REMOTE_CLIENT_PACKING_H */
