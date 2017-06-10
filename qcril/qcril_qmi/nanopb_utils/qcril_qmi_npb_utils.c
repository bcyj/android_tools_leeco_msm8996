/******************************************************************************
  @file    qcril_qmi_npb_utils.c
  @brief   qcril qmi - nanopb utilities

  DESCRIPTION
    Handles ims message nanopb utility functions.

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "qcril_qmi_npb_utils.h"

//===========================================================================
// qcril_qmi_npb_field_init
//===========================================================================
boolean qcril_qmi_npb_field_init(qcril_qmi_npb_field_iter_type *iter, const pb_field_t *fields, void *dest_struct)
{
    if (NULL == iter || NULL == fields)
    {
        QCRIL_LOG_ERROR("NULL == iter || NULL == fields");
        return FALSE;
    }

    memset(iter, 0, sizeof(*iter));

    iter->pos = fields;
    if (dest_struct)
    {
        iter->cur_data_ptr = (char *)dest_struct + iter->pos->data_offset;
        iter->dest_struct = dest_struct;
    }
    return TRUE;
} // qcril_qmi_npb_field_init

//===========================================================================
// qcril_qmi_npb_field_next
//===========================================================================
boolean qcril_qmi_npb_field_next(qcril_qmi_npb_field_iter_type *iter)
{
    if (iter->pos->tag == 0)
    {
        return FALSE;
    }

    size_t prev_size = iter->pos->data_size;

    if (PB_ATYPE(iter->pos->type) == PB_ATYPE_STATIC &&
        PB_HTYPE(iter->pos->type) == PB_HTYPE_REPEATED)
    {
        prev_size *= iter->pos->array_size;
    }
    else if (PB_ATYPE(iter->pos->type) == PB_ATYPE_POINTER)
    {
        prev_size = sizeof(void*);
    }

    iter->pos++;;

    if (iter->cur_data_ptr)
    {
        iter->cur_data_ptr = (char *)iter->cur_data_ptr + prev_size + iter->pos->data_offset;
    }
    return TRUE;
} // qcril_qmi_npb_field_next

//===========================================================================
// qcril_qmi_npb_get_data_size
//===========================================================================
size_t qcril_qmi_npb_get_data_size(const pb_field_t fields[])
{
    size_t ret = 0;

    qcril_qmi_npb_field_iter_type iter;
    qcril_qmi_npb_field_init(&iter, fields, NULL);

    do
    {
        size_t new_field_size = iter.pos->data_size;

        if (iter.pos->tag == 0)
        {
            new_field_size = 0;
        }
        else
        {
            if (PB_ATYPE(iter.pos->type) == PB_ATYPE_STATIC &&
                PB_HTYPE(iter.pos->type) == PB_HTYPE_REPEATED)
            {
                new_field_size *= iter.pos->array_size;
            }
            else if (PB_ATYPE(iter.pos->type) == PB_ATYPE_POINTER)
            {
                new_field_size = sizeof(void*);
            }
        }

        ret += new_field_size - iter.pos->size_offset;
    } while (qcril_qmi_npb_field_next(&iter));

    return ret;
} // qcril_qmi_npb_get_data_size

//===========================================================================
// qcril_qmi_npb_release_binary_data
//===========================================================================
static void qcril_qmi_npb_release_binary_data(qcril_binary_data_type *bin_data)
{
    if (bin_data)
    {
        if (bin_data->data)
        {
            qcril_free(bin_data->data);
        }
        else
        {
            QCRIL_LOG_DEBUG("bin_data->data is NULL");
        }
        qcril_free(bin_data);
    }
    else
    {
        QCRIL_LOG_DEBUG("bin_data is NULL");
    }
} // qcril_qmi_npb_release_binary_data

//===========================================================================
// qcril_qmi_npb_release
//===========================================================================
void qcril_qmi_npb_release(const pb_field_t fields[], void *dest_struct)
{
    if (NULL == fields || NULL == dest_struct)
    {
        QCRIL_LOG_DEBUG("NULL == fields || NULL == dest_struct");
        return;
    }

    qcril_qmi_npb_field_iter_type iter;
    qcril_qmi_npb_field_init(&iter, fields, dest_struct);

    do
    {
        pb_type_t type;
        type = iter.pos->type;

        if (iter.pos->tag == 0)
        {
            continue;
        }

        if (PB_HTYPE(type) == PB_HTYPE_REPEATED)
        {
            if ( PB_LTYPE(type) == PB_LTYPE_STRING ||
                 PB_LTYPE(type) == PB_LTYPE_FIXED32 ||
                 PB_LTYPE(type) == PB_LTYPE_FIXED64 ||
                 PB_LTYPE(type) == PB_LTYPE_SVARINT ||
                 PB_LTYPE(type) == PB_LTYPE_VARINT ||
                 PB_LTYPE(type) == PB_LTYPE_UVARINT )
            {
                void **data_ptr = (void**)((pb_callback_t *)(iter.cur_data_ptr))->arg;
                if (data_ptr)
                {
                    while (*data_ptr)
                    {
                        qcril_free(*data_ptr);
                        data_ptr++;
                    }
                    qcril_free(((pb_callback_t *)(iter.cur_data_ptr))->arg);
                }
            }
            else if (PB_LTYPE(type) == PB_LTYPE_BYTES)
            {
                qcril_binary_data_type **data_ptr = (qcril_binary_data_type**)((pb_callback_t *)(iter.cur_data_ptr))->arg;
                if (data_ptr)
                {
                    while (*data_ptr)
                    {
                        qcril_qmi_npb_release_binary_data(*data_ptr);
                        data_ptr++;
                    }
                    qcril_free(((pb_callback_t *)(iter.cur_data_ptr))->arg);
                }
            }
            else if (PB_LTYPE(type) == PB_LTYPE_SUBMESSAGE)
            {
                void **data_ptr = (void**)((pb_callback_t *)(iter.cur_data_ptr))->arg;
                if (data_ptr)
                {
                    while (*data_ptr)
                    {
                        qcril_qmi_npb_release((const pb_field_t*)iter.pos->ptr, *data_ptr);
                        data_ptr++;
                    }
                    qcril_free(((pb_callback_t *)(iter.cur_data_ptr))->arg);
                }
            }
            else
            {
                QCRIL_LOG_DEBUG("Unexpected repeated field type: field tag %d", iter.pos->tag);
            }
        }
        else if (PB_LTYPE(type) == PB_LTYPE_STRING)
        {
            qcril_free(((pb_callback_t*) (iter.cur_data_ptr))->arg);
        }
        else if (PB_LTYPE(type) == PB_LTYPE_BYTES)
        {
            qcril_qmi_npb_release_binary_data( ((pb_callback_t*)(iter.cur_data_ptr))->arg );
        }
        else if (PB_LTYPE(type) == PB_LTYPE_SUBMESSAGE)
        {
            qcril_qmi_npb_release((const pb_field_t*)iter.pos->ptr, iter.cur_data_ptr);
        }
    } while (qcril_qmi_npb_field_next(&iter));
} // qcril_qmi_npb_release
