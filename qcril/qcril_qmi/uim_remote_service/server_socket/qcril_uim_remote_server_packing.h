/*!
  @file
  qcril_uim_remote_server_packing.h

  @brief
   APIs to pack the uim remote server messages
*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#ifndef QCRIL_UIM_REMOTE_SERVER_PACKING_H
#define QCRIL_UIM_REMOTE_SERVER_PACKING_H

#include "sap-api.pb.h"
#include "qcrili.h"

size_t qcril_uim_remote_server_pack_msg_tag(const void *msg, int msg_len,
                                        uint32_t token,
                                        MsgType type,
                                        MsgId message_id,
                                        Error error,
                                        uint8_t *buf, size_t buf_size);

MsgHeader* qcril_uim_remote_server_unpack_msg_tag(const uint8_t *buf, size_t buf_size);

#endif /* QCRIL_UIM_REMOTE_SERVER_PACKING_H */
