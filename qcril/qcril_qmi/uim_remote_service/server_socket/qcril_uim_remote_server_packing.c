/******************************************************************************
  @file    qcril_uim_remote_server_packing.c
  @brief   qcril  - uim remote server message packing

  DESCRIPTION
    Handles uim remote server related message packing/unpacking functions.

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <pb_decode.h>
#include <pb_encode.h>
#include "qcril_uim_remote_server_packing.h"
#include "qcril_log.h"
#include "qcril_uim_remote_server_msg_meta.h"
#include "qcril_qmi_npb_encode.h"
#include "qcril_qmi_npb_decode.h"

#define UIM_REMOTE_SERVER_MAX_TAG_SIZE 7
//===========================================================================
// qcril_uim_remote_server_pack_msg_tag
//===========================================================================
size_t qcril_uim_remote_server_pack_msg_tag(const void *msg, int msg_len,
                                        uint32_t token,
                                        MsgType type,
                                        MsgId message_id,
                                        Error error,
                                        uint8_t *buf, size_t buf_size)
{
    QCRIL_LOG_FUNC_ENTRY();
    if (NULL == buf)
    {
        QCRIL_LOG_ERROR("buf is NULL");
        return 0;
    }

    size_t packed_size = 0;
    if(qcril_uim_remote_server_is_msg_valid(message_id, type))
    {
        QCRIL_LOG_ERROR("encoding the MsgHeader");
        MsgHeader msg_tag;
        memset(&msg_tag, sizeof(msg_tag), 0);

        msg_tag.token = token;
        msg_tag.type = type;
        msg_tag.id = message_id;
        msg_tag.error = error;
        pb_ostream_t ostream;
        qcril_binary_data_type bin_data;
        if(msg == NULL)
        {
            QCRIL_LOG_INFO("Message payload is NULL, assign NULL to payload");
            msg_tag.payload.arg = NULL;
        }
        else
        {
            QCRIL_LOG_INFO("Message payload exists, assign the message(encoded)");
            bin_data.data = msg;
            bin_data.len = msg_len;
            QCRIL_LOG_INFO("bin_data_len: %d", (int)bin_data.len);
            msg_tag.payload.arg = (void*)&bin_data;
        }

        QCRIL_LOG_INFO("Filled in the tag, encoding the tag");
        ostream = pb_ostream_from_buffer(buf, buf_size);
        if (qcril_qmi_encode_npb(&ostream, MsgHeader_fields, &msg_tag))
        {
            packed_size = ostream.bytes_written;
            QCRIL_LOG_INFO("encoded the tag");
        }
        else
        {
            QCRIL_LOG_ERROR("pb_encode failed");
        }
    }
    else
    {
        QCRIL_LOG_ERROR("Message id and type doesnt co-exist. Cannot encode the tag");
    }
    QCRIL_LOG_FUNC_RETURN_WITH_RET((int) packed_size);
    return packed_size;
} // qcril_uim_remote_server_pack_msg_tag

//===========================================================================
// qcril_uim_remote_server_unpack_msg_tag
//===========================================================================
MsgHeader* qcril_uim_remote_server_unpack_msg_tag(const uint8_t *buf, size_t buf_size)
{
    QCRIL_LOG_FUNC_ENTRY();

    MsgHeader* msg_tag_ptr = NULL;

    if (NULL == buf)
    {
        QCRIL_LOG_ERROR("buf is NULL");
    }
    else
    {
        msg_tag_ptr = qcril_malloc(sizeof(*msg_tag_ptr));
        if (msg_tag_ptr)
        {
            pb_istream_t istream;
            istream = pb_istream_from_buffer(buf, buf_size);
            if(!qcril_qmi_decode_npb(&istream, MsgHeader_fields, msg_tag_ptr))
            {
                QCRIL_LOG_ERROR("pb_decode failed");
                qcril_free(msg_tag_ptr);
                return NULL;
            }
        }
        else
        {
            QCRIL_LOG_ERROR("malloc failed");
        }
    }

    QCRIL_LOG_FUNC_RETURN();
    return msg_tag_ptr;
} // qcril_uim_remote_server_unpack_msg_tag
