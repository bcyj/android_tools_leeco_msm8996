/******************************************************************************
  @file    qcril_qmi_ims_if_pb.c
  @brief   qcril qmi - ims proto buf interface wrapper

  DESCRIPTION
    Creates  wrapper functions over ims protobuf interfce apis.

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qcrili.h"

void   qcril_qmi_ims__msg_tag__init
(
    Ims__MsgTag *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__msg_tag__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__msg_tag__get_packed_size
(
    const Ims__MsgTag *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__msg_tag__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__msg_tag__pack
(
    const Ims__MsgTag *message,
    uint8_t           *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__msg_tag__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__msg_tag__pack_to_buffer
(
    const Ims__MsgTag *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__msg_tag__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__MsgTag * qcril_qmi_ims__msg_tag__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__msg_tag__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void qcril_qmi_ims__msg_tag__free_unpacked
(
    Ims__MsgTag        *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__msg_tag__free_unpacked(message, allocator);
#else
  return;
#endif
}

/* Ims__Info methods */
void   qcril_qmi_ims__info__init
                     (Ims__Info         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__info__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__info__get_packed_size
                     (const Ims__Info   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__info__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__info__pack
                     (const Ims__Info   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__info__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__info__pack_to_buffer
                     (const Ims__Info   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__info__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__Info *
       qcril_qmi_ims__info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__info__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__info__free_unpacked
                     (Ims__Info *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__info__free_unpacked(message, allocator);
#else
  return;
#endif
}

/* Ims__SrvStatusList methods */
void   qcril_qmi_ims__srv_status_list__init
                     (Ims__SrvStatusList         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__srv_status_list__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__srv_status_list__get_packed_size
                     (const Ims__SrvStatusList   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__srv_status_list__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__srv_status_list__pack
                     (const Ims__SrvStatusList   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__srv_status_list__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__srv_status_list__pack_to_buffer
                     (const Ims__SrvStatusList   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__srv_status_list__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__SrvStatusList *
       qcril_qmi_ims__srv_status_list__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__srv_status_list__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__srv_status_list__free_unpacked
                     (Ims__SrvStatusList *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__srv_status_list__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__call_details__init
(
    Ims__CallDetails   *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_details__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__call_details__get_packed_size
(
    const Ims__CallDetails *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_details__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__call_details__pack
(
    const Ims__CallDetails *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_details__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__call_details__pack_to_buffer
(
    const Ims__CallDetails *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_details__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__CallDetails * qcril_qmi_ims__call_details__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_details__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__call_details__free_unpacked
(
    Ims__CallDetails *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_details__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__call_modify__init
(
    Ims__CallModify         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_modify__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__call_modify__get_packed_size
(
    const Ims__CallModify *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_modify__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__call_modify__pack
(
    const Ims__CallModify *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_modify__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__call_modify__pack_to_buffer
(
    const Ims__CallModify *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_modify__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__CallModify *qcril_qmi_ims__call_modify__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_modify__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__call_modify__free_unpacked
(
    Ims__CallModify *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_modify__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__call_list__call__init
(
    Ims__CallList__Call         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_list__call__init(message);
#else
  return;
#endif
}

void   qcril_qmi_ims__call_list__init
(
    Ims__CallList         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_list__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__call_list__get_packed_size
(
    const Ims__CallList *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_list__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__call_list__pack
(
    const Ims__CallList *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_list__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__call_list__pack_to_buffer
(
    const Ims__CallList *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_list__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__CallList *qcril_qmi_ims__call_list__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_list__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__call_list__free_unpacked
(
    Ims__CallList *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_list__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__dial__init
(
    Ims__Dial *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__dial__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__dial__get_packed_size
(
    const Ims__Dial *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__dial__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__dial__pack
(
    const Ims__Dial *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__dial__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__dial__pack_to_buffer
(
    const Ims__Dial *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__dial__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__Dial *qcril_qmi_ims__dial__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__dial__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__dial__free_unpacked
(
    Ims__Dial *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__dial__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__hangup__init
(
    Ims__Hangup         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__hangup__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__hangup__get_packed_size
(
    const Ims__Hangup *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__hangup__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__hangup__pack
(
    const Ims__Hangup *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__hangup__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__hangup__pack_to_buffer
(
    const Ims__Hangup *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__hangup__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__Hangup *qcril_qmi_ims__hangup__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__hangup__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__hangup__free_unpacked
(
    Ims__Hangup *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__hangup__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__deflect_call__init
(
    Ims__DeflectCall         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__deflect_call__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__deflect_call__get_packed_size
(
    const Ims__DeflectCall *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__deflect_call__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__deflect_call__pack
(
    const Ims__DeflectCall *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__deflect_call__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__deflect_call__pack_to_buffer
(
    const Ims__DeflectCall *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__deflect_call__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__DeflectCall *qcril_qmi_ims__deflect_call__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__deflect_call__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__deflect_call__free_unpacked
(
    Ims__DeflectCall *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__deflect_call__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__clir__init
(
    Ims__Clir         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__clir__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__clir__get_packed_size
(
    const Ims__Clir *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__clir__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__clir__pack
(
    const Ims__Clir *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__clir__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__clir__pack_to_buffer
(
    const Ims__Clir *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__clir__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__Clir *qcril_qmi_ims__clir__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__clir__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__clir__free_unpacked
(
    Ims__Clir *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__clir__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__answer__init
(
    Ims__Answer         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__answer__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__answer__get_packed_size
(
    const Ims__Answer *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__answer__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__answer__pack
(
    const Ims__Answer *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__answer__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__answer__pack_to_buffer
(
    const Ims__Answer *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__answer__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__Answer *qcril_qmi_ims__answer__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__answer__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__answer__free_unpacked
(
    Ims__Answer *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__answer__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__switch_waiting_or_holding_and_active__init
(
    Ims__SwitchWaitingOrHoldingAndActive         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__switch_waiting_or_holding_and_active__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__switch_waiting_or_holding_and_active__get_packed_size
(
    const Ims__SwitchWaitingOrHoldingAndActive *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__switch_waiting_or_holding_and_active__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__switch_waiting_or_holding_and_active__pack
(
    const Ims__SwitchWaitingOrHoldingAndActive *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__switch_waiting_or_holding_and_active__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__switch_waiting_or_holding_and_active__pack_to_buffer
(
    const Ims__SwitchWaitingOrHoldingAndActive *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__switch_waiting_or_holding_and_active__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__SwitchWaitingOrHoldingAndActive *qcril_qmi_ims__switch_waiting_or_holding_and_active__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__switch_waiting_or_holding_and_active__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__switch_waiting_or_holding_and_active__free_unpacked
(
    Ims__SwitchWaitingOrHoldingAndActive *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__switch_waiting_or_holding_and_active__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__mute__init
(
    Ims__Mute         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mute__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__mute__get_packed_size
(
    const Ims__Mute *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mute__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__mute__pack
(
    const Ims__Mute *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mute__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__mute__pack_to_buffer
(
    const Ims__Mute *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mute__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__Mute *qcril_qmi_ims__mute__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mute__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__mute__free_unpacked
(
    Ims__Mute *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mute__free_unpacked(message, allocator);
#else
  return;
#endif
}
void   qcril_qmi_ims__dtmf__init
(
    Ims__Dtmf         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__dtmf__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__dtmf__get_packed_size
(
    const Ims__Dtmf *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__dtmf__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__dtmf__pack
(
    const Ims__Dtmf *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__dtmf__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__dtmf__pack_to_buffer
(
    const Ims__Dtmf *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__dtmf__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__Dtmf *qcril_qmi_ims__dtmf__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__dtmf__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__dtmf__free_unpacked
(
    Ims__Dtmf *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__dtmf__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__registration__init
(
    Ims__Registration *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__registration__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__registration__get_packed_size
(
    const Ims__Registration *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__registration__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__registration__pack
(
    const Ims__Registration *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__registration__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__registration__pack_to_buffer
(
    const Ims__Registration *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__registration__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__Registration *qcril_qmi_ims__registration__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__registration__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__registration__free_unpacked
(
    Ims__Registration *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__registration__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__ring_back_tone__init
(
    Ims__RingBackTone         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__ring_back_tone__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__ring_back_tone__get_packed_size
(
    const Ims__RingBackTone *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__ring_back_tone__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__ring_back_tone__pack
(
    const Ims__RingBackTone *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__ring_back_tone__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__ring_back_tone__pack_to_buffer
(
    const Ims__RingBackTone *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__ring_back_tone__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__RingBackTone *qcril_qmi_ims__ring_back_tone__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__ring_back_tone__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void qcril_qmi_ims__ring_back_tone__free_unpacked
(
    Ims__RingBackTone *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__ring_back_tone__free_unpacked(message, allocator);
#else
  return;
#endif
}
void   qcril_qmi_ims__ifconnected__init
(
    Ims__IFConnected         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__ifconnected__init(message);
#else
  return;
#endif
}
size_t qcril_qmi_ims__ifconnected__get_packed_size
(
    const Ims__IFConnected *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__ifconnected__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__ifconnected__pack
(
    const Ims__IFConnected *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__ifconnected__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__ifconnected__pack_to_buffer
(
    const Ims__IFConnected *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__ifconnected__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__IFConnected *qcril_qmi_ims__ifconnected__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__ifconnected__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__ifconnected__free_unpacked
(
    Ims__IFConnected *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__ifconnected__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__call_fail_cause_response__init
(
    Ims__CallFailCauseResponse         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_fail_cause_response__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__call_fail_cause_response__get_packed_size
(
    const Ims__CallFailCauseResponse *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_fail_cause_response__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__call_fail_cause_response__pack
(
    const Ims__CallFailCauseResponse *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_fail_cause_response__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__call_fail_cause_response__pack_to_buffer
(
    const Ims__CallFailCauseResponse *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_fail_cause_response__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__CallFailCauseResponse *qcril_qmi_ims__call_fail_cause_response__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_fail_cause_response__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__call_fail_cause_response__free_unpacked
(
    Ims__CallFailCauseResponse *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_fail_cause_response__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__last_fail_cause__init
(
    Ims__LastFailCause         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__last_fail_cause__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__last_fail_cause__get_packed_size
(
    const Ims__LastFailCause *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__last_fail_cause__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__last_fail_cause__pack
(
    const Ims__LastFailCause *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__last_fail_cause__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__last_fail_cause__pack_to_buffer
(
    const Ims__LastFailCause *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__last_fail_cause__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__LastFailCause *qcril_qmi_ims__last_fail_cause__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__last_fail_cause__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__last_fail_cause__free_unpacked
(
    Ims__LastFailCause *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__last_fail_cause__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__extra__init
(
    Ims__Extra         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__extra__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__extra__get_packed_size
(
    const Ims__Extra *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__extra__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__extra__pack
(
    const Ims__Extra *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__extra__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__extra__pack_to_buffer
(
    const Ims__Extra *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__extra__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__Extra *qcril_qmi_ims__extra__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__extra__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__extra__free_unpacked
(
    Ims__Extra *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__extra__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__status_for_access_tech__init
(
    Ims__StatusForAccessTech         *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__status_for_access_tech__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__status_for_access_tech__get_packed_size
(
    const Ims__StatusForAccessTech *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__status_for_access_tech__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__status_for_access_tech__pack
(
    const Ims__StatusForAccessTech *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__status_for_access_tech__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__status_for_access_tech__pack_to_buffer
(
    const Ims__StatusForAccessTech *message,
    ProtobufCBuffer *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__status_for_access_tech__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__StatusForAccessTech *qcril_qmi_ims__status_for_access_tech__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__status_for_access_tech__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__status_for_access_tech__free_unpacked
(
    Ims__StatusForAccessTech *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__status_for_access_tech__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__handover__init
(
    Ims__Handover *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__handover__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__handover__get_packed_size
(
    const Ims__Handover *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__handover__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__handover__pack
(
    const Ims__Handover *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__handover__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__handover__pack_to_buffer
(
    const Ims__Handover *message,
    ProtobufCBuffer     *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__handover__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__Handover *qcril_qmi_ims__handover__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__handover__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__handover__free_unpacked
(
    Ims__Handover      *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__handover__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__tty_notify__init
(
    Ims__TtyNotify *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__tty_notify__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__tty_notify__get_packed_size
(
    const Ims__TtyNotify *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__tty_notify__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__tty_notify__pack
(
    const Ims__TtyNotify *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__tty_notify__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__tty_notify__pack_to_buffer
(
    const Ims__TtyNotify *message,
    ProtobufCBuffer     *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__tty_notify__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__TtyNotify *qcril_qmi_ims__tty_notify__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__tty_notify__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__tty_notify__free_unpacked
(
    Ims__TtyNotify      *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__tty_notify__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__radio_state_changed__init
(
    Ims__RadioStateChanged *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__radio_state_changed__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__radio_state_changed__get_packed_size
(
    const Ims__RadioStateChanged *message
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__radio_state_changed__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__radio_state_changed__pack
(
    const Ims__RadioStateChanged *message,
    uint8_t       *out
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__radio_state_changed__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__radio_state_changed__pack_to_buffer
(
    const Ims__RadioStateChanged *message,
    ProtobufCBuffer     *buffer
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__radio_state_changed__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__RadioStateChanged *qcril_qmi_ims__radio_state_changed__unpack
(
    ProtobufCAllocator  *allocator,
    size_t               len,
    const uint8_t       *data
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__radio_state_changed__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__radio_state_changed__free_unpacked
(
    Ims__RadioStateChanged      *message,
    ProtobufCAllocator *allocator
)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__radio_state_changed__free_unpacked(message, allocator);
#else
  return;
#endif
}

/* Ims__ServiceClass methods */
void   qcril_qmi_ims__service_class__init
                     (Ims__ServiceClass         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__service_class__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__service_class__get_packed_size
                     (const Ims__ServiceClass   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__service_class__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__service_class__pack
                     (const Ims__ServiceClass   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__service_class__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__service_class__pack_to_buffer
                     (const Ims__ServiceClass   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__service_class__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__ServiceClass *
       qcril_qmi_ims__service_class__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__service_class__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__service_class__free_unpacked
                     (Ims__ServiceClass *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__service_class__free_unpacked(message, allocator);
#else
  return;
#endif
}

/* Ims__CbNumList methods */
void   qcril_qmi_ims__cb_num_list__init
                     (Ims__CbNumList         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__cb_num_list__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__cb_num_list__get_packed_size
                     (const Ims__CbNumList   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__cb_num_list__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__cb_num_list__pack
                     (const Ims__CbNumList   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__cb_num_list__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__cb_num_list__pack_to_buffer
                     (const Ims__CbNumList   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__cb_num_list__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__CbNumList *
       qcril_qmi_ims__cb_num_list__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__cb_num_list__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__cb_num_list__free_unpacked
                     (Ims__CbNumList *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__cb_num_list__free_unpacked(message, allocator);
#else
  return;
#endif
}

/* Ims__CbNumListType methods */
void   qcril_qmi_ims__cb_num_list_type__init
                     (Ims__CbNumListType         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__cb_num_list_type__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__cb_num_list_type__get_packed_size
                     (const Ims__CbNumListType   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__cb_num_list_type__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__cb_num_list_type__pack
                     (const Ims__CbNumListType   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__cb_num_list_type__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__cb_num_list_type__pack_to_buffer
                     (const Ims__CbNumListType   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__cb_num_list_type__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__CbNumListType *
       qcril_qmi_ims__cb_num_list_type__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__cb_num_list_type__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__cb_num_list_type__free_unpacked
                     (Ims__CbNumListType *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__cb_num_list_type__free_unpacked(message, allocator);
#else
  return;
#endif
}

/* Ims__CallWaitingInfo methods */
void   qcril_qmi_ims__call_waiting_info__init
                     (Ims__CallWaitingInfo         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_waiting_info__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__call_waiting_info__get_packed_size
                     (const Ims__CallWaitingInfo   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_waiting_info__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__call_waiting_info__pack
                     (const Ims__CallWaitingInfo   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_waiting_info__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__call_waiting_info__pack_to_buffer
                     (const Ims__CallWaitingInfo   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_waiting_info__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__CallWaitingInfo *
       qcril_qmi_ims__call_waiting_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_waiting_info__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__call_waiting_info__free_unpacked
                     (Ims__CallWaitingInfo *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_waiting_info__free_unpacked(message, allocator);
#else
  return;
#endif
}

/* Ims__CallForwardInfoList__CallForwardInfo methods */
void   qcril_qmi_ims__call_forward_info_list__call_forward_info__init
                     (Ims__CallForwardInfoList__CallForwardInfo         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_forward_info_list__call_forward_info__init(message);
#else
  return;
#endif
}

/* Ims__CallForwardInfoList methods */
void   qcril_qmi_ims__call_forward_info_list__init
                     (Ims__CallForwardInfoList         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_forward_info_list__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__call_forward_info_list__get_packed_size
                     (const Ims__CallForwardInfoList   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_forward_info_list__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__call_forward_info_list__pack
                     (const Ims__CallForwardInfoList   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_forward_info_list__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__call_forward_info_list__pack_to_buffer
                     (const Ims__CallForwardInfoList   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_forward_info_list__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__CallForwardInfoList *
       qcril_qmi_ims__call_forward_info_list__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_forward_info_list__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__call_forward_info_list__free_unpacked
                     (Ims__CallForwardInfoList *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_forward_info_list__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__call_fwd_timer_info__init
                     (Ims__CallFwdTimerInfo         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_fwd_timer_info__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__call_fwd_timer_info__get_packed_size
                     (const Ims__CallFwdTimerInfo   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_fwd_timer_info__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__call_fwd_timer_info__pack
                     (const Ims__CallFwdTimerInfo   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_fwd_timer_info__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__call_fwd_timer_info__pack_to_buffer
                     (const Ims__CallFwdTimerInfo   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_fwd_timer_info__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__CallFwdTimerInfo *
       qcril_qmi_ims__call_fwd_timer_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_fwd_timer_info__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__call_fwd_timer_info__free_unpacked
                     (Ims__CallFwdTimerInfo *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__call_fwd_timer_info__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__conf_info__init
                     (Ims__ConfInfo         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__conf_info__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__conf_info__get_packed_size
                     (const Ims__ConfInfo   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__conf_info__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__conf_info__pack
                     (const Ims__ConfInfo   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__conf_info__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__conf_info__pack_to_buffer
                     (const Ims__ConfInfo   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__conf_info__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__ConfInfo *
       qcril_qmi_ims__conf_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__conf_info__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__conf_info__free_unpacked
                     (Ims__ConfInfo *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__conf_info__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__supp_svc_notification__init
                     (Ims__SuppSvcNotification         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_notification__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__supp_svc_notification__get_packed_size
                     (const Ims__SuppSvcNotification   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_notification__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__supp_svc_notification__pack
                     (const Ims__SuppSvcNotification   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_notification__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__supp_svc_notification__pack_to_buffer
                     (const Ims__SuppSvcNotification   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_notification__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__SuppSvcNotification *
       qcril_qmi_ims__supp_svc_notification__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_notification__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__supp_svc_notification__free_unpacked
                     (Ims__SuppSvcNotification *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_notification__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__supp_svc_status__init
                     (Ims__SuppSvcStatus         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_status__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__supp_svc_status__get_packed_size
                     (const Ims__SuppSvcStatus   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_status__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__supp_svc_status__pack
                     (const Ims__SuppSvcStatus   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_status__pack(message, out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__supp_svc_status__pack_to_buffer
                     (const Ims__SuppSvcStatus   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_status__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}

Ims__SuppSvcStatus *
       qcril_qmi_ims__supp_svc_status__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_status__unpack(allocator, len, data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__supp_svc_status__free_unpacked
                     (Ims__SuppSvcStatus *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_status__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__supp_svc_request__init
                     (Ims__SuppSvcRequest         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_request__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__supp_svc_request__get_packed_size
                     (const Ims__SuppSvcRequest   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_request__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__supp_svc_request__pack
                     (const Ims__SuppSvcRequest   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_request__pack(message,
                                     out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__supp_svc_request__pack_to_buffer
                     (const Ims__SuppSvcRequest   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_request__pack_to_buffer(message,
                                               buffer);
#else
  return 0;
#endif
}

Ims__SuppSvcRequest *
       qcril_qmi_ims__supp_svc_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_request__unpack(allocator,
                                       len,
                                       data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__supp_svc_request__free_unpacked
                     (Ims__SuppSvcRequest *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_request__free_unpacked(message,
                                              allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__supp_svc_response__init
                     (Ims__SuppSvcResponse         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_response__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__supp_svc_response__get_packed_size
                     (const Ims__SuppSvcResponse   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_response__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__supp_svc_response__pack
                     (const Ims__SuppSvcResponse   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_response__pack(message,
                                      out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__supp_svc_response__pack_to_buffer
                     (const Ims__SuppSvcResponse   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_response__pack_to_buffer(message,
                                                buffer);
#else
  return 0;
#endif
}

Ims__SuppSvcResponse *
       qcril_qmi_ims__supp_svc_response__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_response__unpack(allocator,
                                        len,
                                        data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__supp_svc_response__free_unpacked
                     (Ims__SuppSvcResponse *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__supp_svc_response__free_unpacked(message,
                                               allocator);
#else
  return;
#endif
}

/* Ims__ClipProvisionStatus methods */
void   qcril_qmi_ims__clip_provision_status__init
                     (Ims__ClipProvisionStatus         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__clip_provision_status__init(message);
#else
  return;
#endif
}

size_t qcril_qmi_ims__clip_provision_status__get_packed_size
                     (const Ims__ClipProvisionStatus   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__clip_provision_status__get_packed_size(message);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__clip_provision_status__pack
                     (const Ims__ClipProvisionStatus   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__clip_provision_status__pack(message,
                                      out);
#else
  return 0;
#endif
}

size_t qcril_qmi_ims__clip_provision_status__pack_to_buffer
                     (const Ims__ClipProvisionStatus   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__clip_provision_status__pack_to_buffer(message,
                                                buffer);
#else
  return 0;
#endif
}

Ims__ClipProvisionStatus *
       qcril_qmi_ims__clip_provision_status__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__clip_provision_status__unpack(allocator,
                                        len,
                                        data);
#else
  return NULL;
#endif
}

void   qcril_qmi_ims__clip_provision_status__free_unpacked
                     (Ims__ClipProvisionStatus *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__clip_provision_status__free_unpacked(message,
                                               allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__video_call_quality__init
                     (Ims__VideoCallQuality         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__video_call_quality__init(message);
#else
  return;
#endif
}
size_t qcril_qmi_ims__video_call_quality__get_packed_size
                     (const Ims__VideoCallQuality *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__video_call_quality__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__video_call_quality__pack
                     (const Ims__VideoCallQuality *message,
                      uint8_t       *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__video_call_quality__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__video_call_quality__pack_to_buffer
                     (const Ims__VideoCallQuality *message,
                      ProtobufCBuffer *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__video_call_quality__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}
Ims__VideoCallQuality *
       qcril_qmi_ims__video_call_quality__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__video_call_quality__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__video_call_quality__free_unpacked
                     (Ims__VideoCallQuality *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__video_call_quality__free_unpacked(message, allocator);
#else
  return;
#endif
}

void   qcril_qmi_ims__colr__init
                     (Ims__Colr         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__colr__init(message);
#else
  return;
#endif
}
size_t qcril_qmi_ims__colr__get_packed_size
                     (const Ims__Colr *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__colr__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__colr__pack
                     (const Ims__Colr *message,
                      uint8_t       *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__colr__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__colr__pack_to_buffer
                     (const Ims__Colr *message,
                      ProtobufCBuffer *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__colr__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}
Ims__Colr *
       qcril_qmi_ims__colr__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__colr__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__colr__free_unpacked
                     (Ims__Colr *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__colr__free_unpacked(message, allocator);
#else
  return;
#endif
}

/* Ims__MwiMessageSummary methods */
void   qcril_qmi_ims__mwi_message_summary__init
                     (Ims__MwiMessageSummary         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi_message_summary__init(message);
#else
  return;
#endif
}
size_t qcril_qmi_ims__mwi_message_summary__get_packed_size
                     (const Ims__MwiMessageSummary   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi_message_summary__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__mwi_message_summary__pack
                     (const Ims__MwiMessageSummary   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi_message_summary__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__mwi_message_summary__pack_to_buffer
                     (const Ims__MwiMessageSummary   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi_message_summary__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}
Ims__MwiMessageSummary *
       qcril_qmi_ims__mwi_message_summary__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi_message_summary__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__mwi_message_summary__free_unpacked
                     (Ims__MwiMessageSummary *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi_message_summary__free_unpacked(message, allocator);
#else
  return;
#endif
}
/* Ims__MwiMessageDetails methods */
void   qcril_qmi_ims__mwi_message_details__init
                     (Ims__MwiMessageDetails         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi_message_details__init(message);
#else
  return;
#endif
}
size_t qcril_qmi_ims__mwi_message_details__get_packed_size
                     (const Ims__MwiMessageDetails   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi_message_details__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__mwi_message_details__pack
                     (const Ims__MwiMessageDetails   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi_message_details__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__mwi_message_details__pack_to_buffer
                     (const Ims__MwiMessageDetails   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi_message_details__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}
Ims__MwiMessageDetails *
       qcril_qmi_ims__mwi_message_details__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi_message_details__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__mwi_message_details__free_unpacked
                     (Ims__MwiMessageDetails *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi_message_details__free_unpacked(message, allocator);
#else
  return;
#endif
}
/* Ims__Mwi methods */
void   qcril_qmi_ims__mwi__init
                     (Ims__Mwi         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi__init(message);
#else
  return;
#endif
}
size_t qcril_qmi_ims__mwi__get_packed_size
                     (const Ims__Mwi   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__mwi__pack
                     (const Ims__Mwi   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__mwi__pack_to_buffer
                     (const Ims__Mwi   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}
Ims__Mwi *
       qcril_qmi_ims__mwi__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__mwi__free_unpacked
                     (Ims__Mwi *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__mwi__free_unpacked(message, allocator);
#else
  return;
#endif
}
void   qcril_qmi_ims__hold__init
                     (Ims__Hold         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__hold__init(message);
#else
  return;
#endif
}
size_t qcril_qmi_ims__hold__get_packed_size
                     (const Ims__Hold *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__hold__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__hold__pack
                     (const Ims__Hold *message,
                      uint8_t       *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__hold__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__hold__pack_to_buffer
                     (const Ims__Hold *message,
                      ProtobufCBuffer *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__hold__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}
Ims__Hold *
       qcril_qmi_ims__hold__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__hold__unpack(allocator, len, data);
#else
  return NULL;
#endif

}
void   qcril_qmi_ims__hold__free_unpacked
                     (Ims__Hold *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__hold__free_unpacked(message, allocator);
#else
  return;
#endif
}
void   qcril_qmi_ims__resume__init
                     (Ims__Resume         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__resume__init(message);
#else
  return;
#endif
}
size_t qcril_qmi_ims__resume__get_packed_size
                     (const Ims__Resume *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__resume__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__resume__pack
                     (const Ims__Resume *message,
                      uint8_t       *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__resume__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__resume__pack_to_buffer
                     (const Ims__Resume *message,
                      ProtobufCBuffer *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__resume__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}
Ims__Resume *
       qcril_qmi_ims__resume__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__resume__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__resume__free_unpacked
                     (Ims__Resume *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__resume__free_unpacked(message, allocator);
#else
  return;
#endif
}
void   qcril_qmi_ims__rtp_statistics_data__init
                     (Ims__RtpStatisticsData         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__rtp_statistics_data__init(message);
#else
  return;
#endif
}
size_t qcril_qmi_ims__rtp_statistics_data__get_packed_size
                     (const Ims__RtpStatisticsData *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__rtp_statistics_data__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__rtp_statistics_data__pack
                     (const Ims__RtpStatisticsData *message,
                      uint8_t       *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__rtp_statistics_data__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__rtp_statistics_data__pack_to_buffer
                     (const Ims__RtpStatisticsData *message,
                      ProtobufCBuffer *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__rtp_statistics_data__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}
Ims__RtpStatisticsData *
       qcril_qmi_ims__rtp_statistics_data__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__rtp_statistics_data__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__rtp_statistics_data__free_unpacked
                     (Ims__RtpStatisticsData *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__rtp_statistics_data__free_unpacked(message, allocator);
#else
  return;
#endif
}
/* Ims__WifiCallingInfo methods */
void   qcril_qmi_ims__wifi_calling_info__init
                     (Ims__WifiCallingInfo         *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__wifi_calling_info__init(message);
#else
  return;
#endif
}
size_t qcril_qmi_ims__wifi_calling_info__get_packed_size
                     (const Ims__WifiCallingInfo   *message)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__wifi_calling_info__get_packed_size(message);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__wifi_calling_info__pack
                     (const Ims__WifiCallingInfo   *message,
                      uint8_t             *out)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__wifi_calling_info__pack(message, out);
#else
  return 0;
#endif
}
size_t qcril_qmi_ims__wifi_calling_info__pack_to_buffer
                     (const Ims__WifiCallingInfo   *message,
                      ProtobufCBuffer     *buffer)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__wifi_calling_info__pack_to_buffer(message, buffer);
#else
  return 0;
#endif
}
Ims__WifiCallingInfo *
       qcril_qmi_ims__wifi_calling_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__wifi_calling_info__unpack(allocator, len, data);
#else
  return NULL;
#endif
}
void   qcril_qmi_ims__wifi_calling_info__free_unpacked
                     (Ims__WifiCallingInfo *message,
                      ProtobufCAllocator *allocator)
{
#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
  return ims__wifi_calling_info__free_unpacked(message, allocator);
#else
  return;
#endif
}
