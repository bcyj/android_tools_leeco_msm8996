/******************************************************************************
  @file    qcril_qmi_npb_decode.c
  @brief   qcril qmi - nanopb decode

  DESCRIPTION
    Handles ims message nanopb decode related functions.

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <pb_decode.h>
#include "qcril_qmi_npb_decode.h"
#include "qcril_qmi_npb_utils.h"

static bool qcril_qmi_npb_decode_preparation(const pb_field_t fields[], void *dest_struct);

//===========================================================================
// qcril_qmi_npb_decode_add_new_element_to_repeated_fields
//===========================================================================
static bool qcril_qmi_npb_decode_add_new_element_to_repeated_fields(void* new_element, void **arg)
{
    if (NULL == arg || NULL == new_element)
    {
        QCRIL_LOG_ERROR("NULL == arg || NULL == new_element");
        return FALSE;
    }

    int pos = 0;
    if (NULL == *arg)
    {
        *arg = qcril_malloc(sizeof(void*)*2);
        if (NULL == *arg)
        {
            QCRIL_LOG_ERROR("Malloc failed");
            return FALSE;
        }
    }
    else
    {
        void** tmp = *arg;
        void **none_detecting_ptr = (void**) *arg;
        while (*none_detecting_ptr != NULL)
        {
            none_detecting_ptr++;
            pos++;
        }
        *arg = qcril_malloc(sizeof(void*) * (pos+2));
        if (NULL == *arg)
        {
            QCRIL_LOG_ERROR("Malloc failed");
            return FALSE;
        }
        memcpy(*arg, tmp, sizeof(void*)*pos);
        qcril_free(tmp);
    }

    ((void**)(*arg))[pos] = new_element;
    ((void**)(*arg))[pos+1] = NULL;
    return TRUE;
} // qcril_qmi_npb_decode_add_new_element_to_repeated_fields

//===========================================================================
// qcril_qmi_npb_decode_repeated_fixed32
//===========================================================================
static bool qcril_qmi_npb_decode_repeated_fixed32(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    int32_t *val = qcril_malloc(sizeof(*val));

    if (NULL == val)
    {
        QCRIL_LOG_ERROR("Malloc failed");
        return FALSE;
    }
    if (!pb_decode_fixed32(stream, val))
    {
        return FALSE;
    }

    return qcril_qmi_npb_decode_add_new_element_to_repeated_fields((void*)val, arg);
} // qcril_qmi_npb_decode_repeated_fixed32

//===========================================================================
// qcril_qmi_npb_decode_repeated_fixed64
//===========================================================================
static bool qcril_qmi_npb_decode_repeated_fixed64(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    int64_t *val = qcril_malloc(sizeof(*val));

    if (NULL == val)
    {
        QCRIL_LOG_ERROR("Malloc failed");
        return FALSE;
    }
    if (!pb_decode_fixed64(stream, val))
    {
        return FALSE;
    }

    return qcril_qmi_npb_decode_add_new_element_to_repeated_fields((void*)val, arg);
} // qcril_qmi_npb_decode_repeated_fixed64

//===========================================================================
// qcril_qmi_npb_decode_repeated_varint
//===========================================================================
static bool qcril_qmi_npb_decode_repeated_varint(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    uint64_t *val = qcril_malloc(sizeof(*val));

    if (NULL == val)
    {
        QCRIL_LOG_ERROR("Malloc failed");
        return FALSE;
    }
    if (!pb_decode_varint(stream, val))
    {
        return FALSE;
    }

    return qcril_qmi_npb_decode_add_new_element_to_repeated_fields((void*)val, arg);
} // qcril_qmi_npb_decode_repeated_varint

//===========================================================================
// qcril_qmi_npb_decode_repeated_svarint
//===========================================================================
static bool qcril_qmi_npb_decode_repeated_svarint(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    int64_t *val = qcril_malloc(sizeof(*val));

    if (NULL == val)
    {
        QCRIL_LOG_ERROR("Malloc failed");
        return FALSE;
    }
    if (!pb_decode_svarint(stream, val))
    {
        return FALSE;
    }

    return qcril_qmi_npb_decode_add_new_element_to_repeated_fields((void*)val, arg);
} // qcril_qmi_npb_decode_repeated_svarint

//===========================================================================
// qcril_qmi_npb_construct_byte_string
//===========================================================================
static bool qcril_qmi_npb_construct_byte_string(pb_istream_t *stream, qcril_binary_data_type* ret_bin_data)
{
    if (NULL == ret_bin_data || NULL == stream)
    {
        QCRIL_LOG_ERROR("NULL == ret_bin_data || NULL == stream");
        return FALSE;
    }

    ret_bin_data->data = NULL;
    ret_bin_data->len = 0;

    size_t len = stream->bytes_left;

    uint8_t *data = qcril_malloc(len);
    if (NULL == data)
    {
        QCRIL_LOG_ERROR("Malloc failed");
        return FALSE;
    }

    if (!pb_read(stream, data, len))
    {
        return FALSE;
    }

    ret_bin_data->data = data;
    ret_bin_data->len = len;

    return TRUE;
} // qcril_qmi_npb_construct_byte_string

//===========================================================================
// qcril_qmi_npb_decode_byte_string
//===========================================================================
static bool qcril_qmi_npb_decode_byte_string(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    *arg = qcril_malloc(sizeof(qcril_binary_data_type));
    if (NULL == *arg)
    {
        QCRIL_LOG_ERROR("Malloc failed");
        return FALSE;
    }
    return qcril_qmi_npb_construct_byte_string(stream, (qcril_binary_data_type *)*arg);
} // qcril_qmi_npb_decode_byte_string

//===========================================================================
// qcril_qmi_npb_decode_repeated_byte_string
//===========================================================================
static bool qcril_qmi_npb_decode_repeated_byte_string(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    qcril_binary_data_type *tmp = qcril_malloc(sizeof(qcril_binary_data_type));
    if (NULL == tmp)
    {
        QCRIL_LOG_ERROR("Malloc failed");
        return FALSE;
    }
    if (!qcril_qmi_npb_construct_byte_string(stream, tmp))
    {
        return FALSE;
    }

    return qcril_qmi_npb_decode_add_new_element_to_repeated_fields(tmp, arg);
} // qcril_qmi_npb_decode_repeated_byte_string

//===========================================================================
// qcril_qmi_npb_construct_string
//===========================================================================
static bool qcril_qmi_npb_construct_string(pb_istream_t *stream, char **ret_str)
{
    if (NULL == ret_str || NULL == stream)
    {
        QCRIL_LOG_ERROR("NULL == ret_str || NULL == stream");
        return FALSE;
    }

    size_t size = stream->bytes_left;
    *ret_str = qcril_malloc(size+1);

    if (NULL == *ret_str)
    {
        QCRIL_LOG_ERROR("Malloc failed");
        return FALSE;
    }
    (*ret_str)[size] = 0;

    if (!pb_read(stream, (uint8_t*)*ret_str, size))
    {
        return FALSE;
    }

    return TRUE;
} // qcril_qmi_npb_construct_string

//===========================================================================
// qcril_qmi_npb_decode_string
//===========================================================================
static bool qcril_qmi_npb_decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    return qcril_qmi_npb_construct_string(stream, (char**)arg);
} // qcril_qmi_npb_decode_string

//===========================================================================
// qcril_qmi_npb_decode_repeated_string
//===========================================================================
bool qcril_qmi_npb_decode_repeated_string(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    char* tmp;

    if (!qcril_qmi_npb_construct_string(stream, &tmp))
    {
        return FALSE;
    }

    return qcril_qmi_npb_decode_add_new_element_to_repeated_fields(tmp, arg);
} // qcril_qmi_npb_decode_repeated_string

//===========================================================================
// qcril_qmi_npb_decode_repeated_submsg
//===========================================================================
static bool qcril_qmi_npb_decode_repeated_submsg(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    size_t submsg_size = qcril_qmi_npb_get_data_size((const pb_field_t *)field->ptr);

    void *submsg = qcril_malloc(submsg_size);
    if (NULL == submsg)
    {
        QCRIL_LOG_ERROR("Malloc failed");
        return FALSE;
    }

    if (!qcril_qmi_npb_decode_preparation((const pb_field_t *)field->ptr, (void*)submsg))
    {
        return FALSE;
    }

    if (!pb_decode(stream, (const pb_field_t *)field->ptr, (void*)submsg))
    {
        return FALSE;
    }

    return qcril_qmi_npb_decode_add_new_element_to_repeated_fields(submsg, arg);
} // qcril_qmi_npb_decode_repeated_submsg

//===========================================================================
// qcril_qmi_npb_decode_preparation
//===========================================================================
static bool qcril_qmi_npb_decode_preparation(const pb_field_t fields[], void *dest_struct)
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
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.decode = qcril_qmi_npb_decode_repeated_string;
            }
            else if (PB_LTYPE(type) == PB_LTYPE_BYTES)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.decode = qcril_qmi_npb_decode_repeated_byte_string;
            }
            else if (PB_LTYPE(type) == PB_LTYPE_SUBMESSAGE)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.decode = qcril_qmi_npb_decode_repeated_submsg;
            }
            else if (PB_LTYPE(type) == PB_LTYPE_FIXED32)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.decode = qcril_qmi_npb_decode_repeated_fixed32;
            }
            else if (PB_LTYPE(type) == PB_LTYPE_FIXED64)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.decode = qcril_qmi_npb_decode_repeated_fixed64;
            }
            else if (PB_LTYPE(type) == PB_LTYPE_SVARINT)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.decode = qcril_qmi_npb_decode_repeated_svarint;
            }
            else if (PB_LTYPE(type) == PB_LTYPE_VARINT || PB_LTYPE(type) == PB_LTYPE_UVARINT)
            {
                ((pb_callback_t*) (iter.cur_data_ptr))->funcs.decode = qcril_qmi_npb_decode_repeated_varint;
            }
            else
            {
                QCRIL_LOG_DEBUG("Unexpected repeated field type: field tag %d", iter.pos->tag);
            }
        }
        else if (PB_LTYPE(type) == PB_LTYPE_STRING)
        {
            ((pb_callback_t*) (iter.cur_data_ptr))->funcs.decode = qcril_qmi_npb_decode_string;
        }
        else if (PB_LTYPE(type) == PB_LTYPE_BYTES)
        {
            ((pb_callback_t*) (iter.cur_data_ptr))->funcs.decode = qcril_qmi_npb_decode_byte_string;
        }
        else if (PB_LTYPE(type) == PB_LTYPE_SUBMESSAGE)
        {
            qcril_qmi_npb_decode_preparation((const pb_field_t*)iter.pos->ptr, iter.cur_data_ptr);
        }
    } while (qcril_qmi_npb_field_next(&iter));

    return TRUE;
} // qcril_qmi_npb_decode_preparation

//===========================================================================
// qcril_qmi_decode_npb
//===========================================================================
boolean qcril_qmi_decode_npb(pb_istream_t *is, const pb_field_t pb_fields[], void* msg)
{
    if (NULL == is || NULL == pb_fields || NULL == msg)
    {
        QCRIL_LOG_ERROR("NULL == is || NULL == pb_fields || NULL == msg");
        return FALSE;
    }

    if (!qcril_qmi_npb_decode_preparation(pb_fields, msg))
    {
        QCRIL_LOG_ERROR("qcril_npb_decode_preparation failed");
        return FALSE;
    }

    if (!pb_decode(is, pb_fields, msg))
    {
        QCRIL_LOG_ERROR("Decoding failed: %s", PB_GET_ERROR(is));
        return FALSE;
    }
    return TRUE;
} // qcril_qmi_decode_npb
