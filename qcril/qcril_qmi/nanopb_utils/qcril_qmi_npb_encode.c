/******************************************************************************
  @file    qcril_qmi_npb_encode.c
  @brief   qcril qmi - nanopb encode

  DESCRIPTION
    Handles ims message nanopb encode related functions.

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <pb_encode.h>
#include "qcril_qmi_npb_encode.h"
#include "qcril_qmi_npb_utils.h"
#include "imsIF.pb.h"

static boolean qcril_qmi_npb_encode_preparation(const pb_field_t fields[], void *dest_struct);

//===========================================================================
// qcril_qmi_npb_encode_string
//===========================================================================
static boolean qcril_qmi_npb_encode_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    const char *str = (const char *)*arg;

    if (!str)
    {
        return TRUE;
    }

    if (!pb_encode_tag_for_field(stream, field))
    {
        return FALSE;
    }
    return pb_encode_string(stream, (const uint8_t *)str, strlen(str));
} // qcril_qmi_npb_encode_string

//===========================================================================
// qcril_qmi_npb_encode_repeated_string
//===========================================================================
static boolean qcril_qmi_npb_encode_repeated_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    char* const *str = (char * const *)*arg;

    while (str && *str)
    {
        if (!qcril_qmi_npb_encode_string(stream, field, (void* const *)str))
        {
            return FALSE;
        }
        str++;
    };
    return TRUE;
} // qcril_qmi_npb_encode_repeated_string

//===========================================================================
// qcril_qmi_npb_encode_byte_string
//===========================================================================
static boolean qcril_qmi_npb_encode_byte_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    const qcril_binary_data_type *bin_data = (const qcril_binary_data_type *)*arg;

    if (!bin_data)
    {
        return true;
    }

    if (!pb_encode_tag_for_field(stream, field))
    {
        return FALSE;
    }

    return pb_encode_string(stream, bin_data->data, bin_data->len);
} // qcril_qmi_npb_encode_byte_string

//===========================================================================
// qcril_qmi_npb_encode_repeated_byte_string
//===========================================================================
static boolean qcril_qmi_npb_encode_repeated_byte_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    qcril_binary_data_type* const *bin_data = (qcril_binary_data_type * const *)*arg;

    while (bin_data && *bin_data)
    {
        if (!qcril_qmi_npb_encode_byte_string(stream, field, (void* const *)bin_data))
        {
            return FALSE;
        }
        bin_data++;
    };
    return TRUE;
} // qcril_qmi_npb_encode_repeated_byte_string

//===========================================================================
// qcril_qmi_npb_encode_repeated_fixed32
//===========================================================================
static boolean qcril_qmi_npb_encode_repeated_fixed32(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    int32_t* const *vals = (int32_t * const *)*arg;

    while (vals && *vals)
    {
        if (!pb_encode_tag_for_field(stream, field))
        {
            return FALSE;
        }

        if (!pb_encode_fixed32(stream, (const void *)*vals))
        {
            return FALSE;
        }
        vals++;
    };
    return TRUE;
} // qcril_qmi_npb_encode_repeated_fixed32

//===========================================================================
// qcril_qmi_npb_encode_repeated_fixed64
//===========================================================================
static boolean qcril_qmi_npb_encode_repeated_fixed64(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    int64_t* const *vals = (int64_t * const *)*arg;

    while (vals && *vals)
    {
        if (!pb_encode_tag_for_field(stream, field))
        {
            return FALSE;
        }
        if (!pb_encode_fixed64(stream, (const void *)*vals))
        {
            return FALSE;
        }
        vals++;
    };
    return TRUE;
} // qcril_qmi_npb_encode_repeated_fixed64

//===========================================================================
// qcril_qmi_npb_encode_repeated_varint
//===========================================================================
static boolean qcril_qmi_npb_encode_repeated_varint(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    uint64_t* const *vals = (uint64_t * const *)*arg;

    while (vals && *vals)
    {
        if (!pb_encode_tag_for_field(stream, field))
        {
            return FALSE;
        }
        if (!pb_encode_varint(stream, **vals))
        {
            return FALSE;
        }
        vals++;
    };
    return TRUE;
} // qcril_qmi_npb_encode_repeated_varint

//===========================================================================
// qcril_qmi_npb_encode_repeated_svarint
//===========================================================================
static boolean qcril_qmi_npb_encode_repeated_svarint(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    uint64_t* const *vals = (uint64_t * const *)*arg;

    while (vals && *vals)
    {
        if (!pb_encode_tag_for_field(stream, field))
        {
            return FALSE;
        }
        if (!pb_encode_svarint(stream, **vals))
        {
            return FALSE;
        }
        vals++;
    };
    return TRUE;
} // qcril_qmi_npb_encode_repeated_svarint

//===========================================================================
// qcril_qmi_npb_encode_repeated_submsg
//===========================================================================
static boolean qcril_qmi_npb_encode_repeated_submsg(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    void** vals = (void **)*arg;
    while (vals && *vals)
    {
        qcril_qmi_npb_encode_preparation((const pb_field_t *)field->ptr, *vals);

        if (!pb_encode_tag_for_field(stream, field))
        {
            return FALSE;
        }
        if (!pb_encode_submessage(stream, (const pb_field_t *)field->ptr, *vals))
        {
            return FALSE;
        }

        vals++;
    };
    return TRUE;
} // qcril_qmi_npb_encode_repeated_submsg

//===========================================================================
// qcril_qmi_npb_encode_preparation
//===========================================================================
static boolean qcril_qmi_npb_encode_preparation(const pb_field_t fields[], void *dest_struct)
{
    if (NULL == dest_struct || NULL == fields)
    {
        QCRIL_LOG_ERROR("NULL == dest_struct || NULL == fields");
        return FALSE;
    }

    qcril_qmi_npb_field_iter_type iter;
    qcril_qmi_npb_field_init(&iter, fields, dest_struct);

    do
    {
        if (iter.pos->tag == 0)
        {
            continue;
        }

        pb_type_t type = iter.pos->type;

        if (PB_HTYPE(type) == PB_HTYPE_REPEATED)
        {
            if (PB_LTYPE(type) == PB_LTYPE_STRING)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.encode = qcril_qmi_npb_encode_repeated_string;
            }
            else if (PB_LTYPE(type) == PB_LTYPE_BYTES)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.encode = qcril_qmi_npb_encode_repeated_byte_string;
            }
            else if (PB_LTYPE(type) == PB_LTYPE_SUBMESSAGE)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.encode = qcril_qmi_npb_encode_repeated_submsg;
            }
            else if (PB_LTYPE(type) == PB_LTYPE_FIXED32)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.encode = qcril_qmi_npb_encode_repeated_fixed32;
            }
            else if (PB_LTYPE(type) == PB_LTYPE_FIXED64)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.encode = qcril_qmi_npb_encode_repeated_fixed64;
            }
            else if (PB_LTYPE(type) == PB_LTYPE_SVARINT)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.encode = qcril_qmi_npb_encode_repeated_svarint;
            }
            else if (PB_LTYPE(type) == PB_LTYPE_VARINT || PB_LTYPE(type) == PB_LTYPE_UVARINT)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.encode = qcril_qmi_npb_encode_repeated_varint;
            }
            else
            {
                QCRIL_LOG_DEBUG("Unexpected repeated field type: field tag %d", iter.pos->tag);
            }
        }
        else if (PB_LTYPE(type) == PB_LTYPE_STRING)
        {
            ((pb_callback_t*) (iter.cur_data_ptr))->funcs.encode = qcril_qmi_npb_encode_string;
        }
        else if (PB_LTYPE(type) == PB_LTYPE_BYTES)
        {
            ((pb_callback_t*) (iter.cur_data_ptr))->funcs.encode = qcril_qmi_npb_encode_byte_string;
        }
        else if (PB_LTYPE(type) == PB_LTYPE_SUBMESSAGE)
        {
            qcril_qmi_npb_encode_preparation((const pb_field_t*)iter.pos->ptr, iter.cur_data_ptr);
        }
    } while (qcril_qmi_npb_field_next(&iter));

    return TRUE;
} // qcril_qmi_npb_encode_preparation

//===========================================================================
// qcril_qmi_encode_npb
//===========================================================================
boolean qcril_qmi_encode_npb(pb_ostream_t *os, const pb_field_t pb_fields[], void *msg)
{
    boolean status = qcril_qmi_npb_encode_preparation(pb_fields, msg);

    if (status)
    {
        status = pb_encode(os, pb_fields, msg);
        if (!status)
        {
            QCRIL_LOG_DEBUG("Encoding failed: %s", PB_GET_ERROR(os));
        }
    }
    return status;
} // qcril_qmi_encode_npb

