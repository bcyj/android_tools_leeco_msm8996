/*========================================================================


*//** @file mpo_writer.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
01/10/11   staceyw Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "mpo_writer.h"
#include "writer_utility.h"
#include "exif_private.h"
#include "jpeg_buffer_private.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include <stdlib.h>

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
// 4 bytes ASCII value of "0100"
#define MP_DEFAULT_FORMAT_VERSION 0x30313030

// 4 bytes identifier should always be “MPF ”
#define MP_FORMAT 0x4D504600

#define MP_BIG_ENDIAN 0x4D4D002A

#define MP_LITTLE_ENDIAN 0x49492A00

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
// define length of MP header field
#define MP_APP2_FIELD_LENGTH_BYTES 2
#define MP_FORMAT_IDENTIFIER_BYTES 4
#define MP_ENDIAN_BYTES 4
#define MP_HEADER_OFFSET_TO_FIRST_IFD_BYTES 4
#define MP_INDEX_COUNT_BYTES 2
#define MP_INDEX_VERSION_BYTES 12
#define MP_INDEX_NUMBER_OF_IMAGES_BYTES 12
#define MP_INDEX_ENTRY_BYTES 12
#define MP_INDEX_OFFSET_OF_NEXT_IFD_BYTES 4
#define MP_INDEX_ENTRY_VALUE_BYTES 16
#define MP_ATTRIBUTE_COUNT_BYTES 2
#define MP_ATTRIBUTE_OFFSET_OF_NEXT_IFD_BYTES 4
#define MP_TAG_BYTES 12
#define MP_INDIVIDUAL_IMAGE_ID_BYTES 33
#define MP_INDEX_IFD_START 2

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
#define MPO_WRITE_BYTE_SCRATCH(b) WRITE_BYTE_SCRATCH(b)

#define MPO_WRITE_SHORT_SCRATCH(s) WRITE_SHORT_SCRATCH_LITTLE(s)

#define MPO_WRITE_LONG_SCRATCH(l) WRITE_LONG_SCRATCH_LITTLE(l)

#define MPO_WRITE_NBYTES_SCRATCH(b, c) WRITE_NBYTES_SCRATCH_LITTLE(b, c)

#define MPO_WRITE_BYTE_VALUE(b) WRITE_BYTE_AHEAD(b)

#define MPO_WRITE_SHORT_VALUE(s) WRITE_SHORT_AHEAD_LITTLE(s)

#define MPO_WRITE_LONG_VALUE(l) WRITE_LONG_AHEAD_LITTLE(l)

#define MPO_WRITE_NBYTES_VALUE(b, c) WRITE_NBYTES_AHEAD_LITTLE(b, c)

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
int mpow_emit_app2_tag(mpo_writer_t *p_writer, exif_tag_entry_ex_t *p_entry);

int mpow_emit_index_ifd(mpo_writer_t *p_writer);

int mpow_emit_attribute_ifd(mpo_writer_t *p_writer, uint32_t src_id);

/* =======================================================================
**                          Function Definitions
** ======================================================================= */
/******************************************************************************
 * Function: mpoe_init
 * Description: Initializes the Mpo Encoder object. Dynamic allocations take
 *              place during the call. One should always call mpoe_destroy to
 *              clean up the Mpo Encoder object after use.
 * Input parameters:
 *   p_writer     - The pointer to the Mpo Writer object to be initialized.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpow_init(mpo_writer_t          *p_writer)
{
    int rc = JPEGERR_SUCCESS;
    uint32_t i;

    // Validate inputs
    if (!p_writer)
        return JPEGERR_ENULLPTR;

    // Zero all fields in the structure
    STD_MEMSET(p_writer, 0, sizeof(mpo_writer_t));

    // Initialize scratch buffer
    for (i = 0; i < MAX_IMAGES; i++)
    {
        if (JPEG_SUCCEEDED(rc))
        {
            rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_writer->header_buf[i]));
            rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_writer->value_buf[i]));
        }
    }

    if (JPEG_FAILED(rc))
    {
        mpow_destroy(p_writer);
    }
    return rc;
}

/******************************************************************************
 * Function: mpow_configure_first
 * Description: Configure for the first individual image.
 * Input parameters:
 *   p_writer  - The Mpo Writer object.
 *   p_source  - The pointer to the sources.
 *   p_dest    - The pointer to destination.
 *   p_mpo_info_obj - The pointer to mpo info.
 *   source_cnt - The source count.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpow_configure_first(
mpo_writer_t    *p_writer,
mpoe_src_t      *p_source,
mpoe_dst_t      *p_dest,
mpo_info_obj_t  *p_mpo_info_obj,
uint32_t         source_cnt)
{
    uint32_t i, tag_size;
    exif_tag_entry_ex_t **pp_entries;
    int rc = JPEGERR_SUCCESS;

    // Validate inputs
    if (!p_writer ||
        !p_source ||
        !p_dest ||
        !p_mpo_info_obj)
    {
        return JPEGERR_ENULLPTR;
    }

    // Store p_dest, encoder, p_source and p_encode_cfg
    p_writer->p_dest     = p_dest;
    p_writer->source_cnt = source_cnt;

    // Allocate header and value buffers
    for (i = 0; i < source_cnt; i++)
    {
        if (JPEG_SUCCEEDED(rc))
        {
           rc = jpeg_buffer_allocate((jpeg_buffer_t)p_writer->header_buf[i],
                                      MPO_MAX_APP2_LENGTH, false);
        }
        if (JPEG_SUCCEEDED(rc))
        {
           rc = jpeg_buffer_allocate((jpeg_buffer_t)p_writer->value_buf[i],
                                      MPO_MAX_APP2_LENGTH, false);
        }
    }

    for (i = 0; i < source_cnt; i++)
    {
        p_writer->image_size[i] = 0;
        p_writer->source[i] = *(p_source + i);

        // Validate mpo type
        if (p_writer->source[i].image_attribute.type != MULTI_VIEW_DISPARITY)
        {
            JPEG_DBG_ERROR("mpoe_set_source: MPO type not supported: %d\n",
                           p_writer->source[i].image_attribute.type);
            return JPEGERR_EUNSUPPORTED;
        }

        if (p_writer->source[i].image_attribute.type == LARGE_TN_CLASS_1 ||
            p_writer->source[i].image_attribute.type == LARGE_TN_CLASS_2)
        {
            p_writer->large_thumbnail_flag[i] = true;
        }
    }

    // set mpo info
    if (p_mpo_info_obj)
    {
        p_writer->p_mpo_info = (mpo_info_t *)(*p_mpo_info_obj);
    }

    // Calculate App2 header length
    p_writer->app2_header_length = MP_APP2_FIELD_LENGTH_BYTES
                                 + MP_FORMAT_IDENTIFIER_BYTES
                                 + MP_ENDIAN_BYTES
                                 + MP_HEADER_OFFSET_TO_FIRST_IFD_BYTES
                                 + MP_INDEX_COUNT_BYTES + MP_INDEX_VERSION_BYTES
                                 + MP_INDEX_OFFSET_OF_NEXT_IFD_BYTES;

    pp_entries = (exif_tag_entry_ex_t **)(&(p_writer->p_mpo_info->index_ifd));
    // Calculate tags in index IFD
    for (i = 0; i < MPO_INDEX_IFD_SIZE; i++)
    {
       if (pp_entries[i])
       {
           p_writer->app2_header_length += MP_TAG_BYTES;
           tag_size = tag_type_sizes[pp_entries[i]->entry.type] * pp_entries[i]->entry.count;
           if (tag_size > 4)
           {
               p_writer->app2_header_length += tag_size;
           }
       }
    }

    if (!p_writer->large_thumbnail_flag[0])
    {
        // Update header length with attribute IFD
        p_writer->app2_header_length += MP_ATTRIBUTE_COUNT_BYTES + MP_ATTRIBUTE_OFFSET_OF_NEXT_IFD_BYTES;

        pp_entries = (exif_tag_entry_ex_t **)(&p_writer->p_mpo_info->attribute_ifds[0]);
        // Calculate tags in attribute IFD
        for (i = 0; i < MPO_ATTRIBUTE_IFD_SIZE; i++)
        {
            if (pp_entries[i])
            {
                p_writer->app2_header_length += MP_TAG_BYTES;
                tag_size = tag_type_sizes[pp_entries[i]->entry.type] * pp_entries[i]->entry.count;
                if (tag_size > 4)
                {
                    p_writer->app2_header_length += tag_size;
                }
            }
        }

    }
    return JPEGERR_SUCCESS;

}

/******************************************************************************
 * Function: mpow_configure
 * Description: Configure for all other individual images except first.
 *              Typically this is used after the encoding is done and before
 *              encoder is destroyed.
 * Input parameters:
 *   p_writer  - The Mpo Writer object.
 *   src_id    - The source count.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpow_configure(mpo_writer_t    *p_writer,
                   uint32_t         src_id)
{
    uint32_t i, tag_size;
    exif_tag_entry_ex_t **pp_entries;

    // Validate inputs
    if (!p_writer)
        return JPEGERR_ENULLPTR;

    if (src_id >= MAX_IMAGES)
    {
        return JPEGERR_EBADPARM;
    }
    p_writer->app2_header_length = 0;

    // Calculate app2 data length
    if (!p_writer->large_thumbnail_flag[src_id])
    {
        p_writer->app2_header_length = MP_APP2_FIELD_LENGTH_BYTES
                                     + MP_FORMAT_IDENTIFIER_BYTES
                                     + MP_ENDIAN_BYTES + MP_HEADER_OFFSET_TO_FIRST_IFD_BYTES;

        p_writer->app2_header_length += MP_ATTRIBUTE_COUNT_BYTES + MP_ATTRIBUTE_OFFSET_OF_NEXT_IFD_BYTES;

        pp_entries = (exif_tag_entry_ex_t **)(&p_writer->p_mpo_info->attribute_ifds[src_id]);
        // Calculate tags in attribute IFD
        for (i = 0; i < MPO_ATTRIBUTE_IFD_SIZE; i++)
        {
            if (pp_entries[i])
            {
                p_writer->app2_header_length += MP_TAG_BYTES;
                tag_size = tag_type_sizes[pp_entries[i]->entry.type] * pp_entries[i]->entry.count;
                if (tag_size > 4)
                {
                    p_writer->app2_header_length += tag_size;
                }
            }
        }
    }
    return JPEGERR_SUCCESS;

}

/******************************************************************************
 * Function: mpoe_destroy
 * Description: Destroys the Mpo Encoder object, releasing all memory
 *              previously allocated.
 * Input parameters:
 *   p_writer       - The pointer to the Mpo Writer object
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpow_destroy(mpo_writer_t *p_writer)
{
    uint32_t i;

    // Validate inputs
    if (!p_writer)
        return JPEGERR_ENULLPTR;

    JPEG_DBG_LOW("mpow_destroy\n");

    // Free app2 buffer if allocated
    for (i = 0; i < p_writer->source_cnt; i++)
    {
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_writer->header_buf[i]));
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_writer->value_buf[i]));
    }

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpow_set_app2_info
 * Description: Sets App2 info into the Mpo Writer object.
 * Input parameters:
 *   p_writer  - The pointer to the Mpo Writer object
 *   buf_offset    - The output buffer offset
 *   app2_start_offset - The App2 header start offset
 *   src_id        - source id
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpow_set_app2_info(
    mpo_writer_t   *p_writer,
    uint32_t        buf_offset,    // The output buffer offset
    uint32_t        app2_start_offset, // The App2 start offset
    uint32_t        src_id)
{
    uint32_t buf_size;
    // Validate inputs
    if (!p_writer)
        return JPEGERR_ENULLPTR;

    // Update image size
    p_writer->total_images_size += buf_offset;
    jpeg_buffer_get_max_size(*(p_writer->p_dest->p_buffer), &buf_size);
    if (buf_offset < buf_size)
    {
       p_writer->last_image_offset[src_id] = buf_offset;
    }

    // Set App2 start offset
    if (app2_start_offset)
    {
        p_writer->app2_start_offset[src_id] = app2_start_offset;
    }

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpow_emit_app2_tag
 * Description: Emit an tag. Typical use is to call this function multiple times
 *              - to emit all the
 *              desired Exif Tags individually and
 *              then pass the info object to the Mpo writer object so
 *              the inserted tags would be emitted as tags in the App2 header.
 * Input parameters:
 *   p_writer  - The Mpo writer.
 *   p_entry   - The pointer to the tag entry structure which contains the
 *               details of tag. The pointer can be set to NULL to un-do
 *               previous insertion for a certain tag.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpow_emit_app2_tag(mpo_writer_t *p_writer, exif_tag_entry_ex_t *p_entry)
{
    uint16_t tag_id;
    uint32_t to_write_len, i;
    uint32_t bytes_written = 0;

    // Validate inputs
    if (!p_writer ||
        !p_entry)
    {
        return JPEGERR_ENULLPTR;
    }

    tag_id = p_entry->tag_id & 0xFFFF;

    // Write Tag ID
    MPO_WRITE_SHORT_SCRATCH(tag_id);

    // Write Tag Type
    MPO_WRITE_SHORT_SCRATCH((uint16_t)p_entry->entry.type);

    // Write Tag count
    MPO_WRITE_LONG_SCRATCH(p_entry->entry.count);

    // Compute the length that needs to be written
    to_write_len = tag_type_sizes[p_entry->entry.type] * p_entry->entry.count;

    if (to_write_len <= 4)
    {
        if (p_entry->entry.type == EXIF_ASCII ||
            p_entry->entry.type == EXIF_UNDEFINED)
        {
            MPO_WRITE_NBYTES_SCRATCH(p_entry->entry.data._ascii, to_write_len);
            bytes_written = to_write_len;
        }
        else
        {
            if (p_entry->entry.count > 1)
            {
                for (i = 0; i < p_entry->entry.count; i++)
                {
                    switch (p_entry->entry.type)
                    {
                    case EXIF_BYTE:
                        MPO_WRITE_BYTE_SCRATCH(p_entry->entry.data._bytes[i]);
                        bytes_written ++;
                        break;
                    case EXIF_SHORT:
                        MPO_WRITE_SHORT_SCRATCH(p_entry->entry.data._shorts[i]);
                        bytes_written += 2;
                        break;
                    default:
                        JPEG_DBG_ERROR("jpegw_emit_exif_tag: impossible case! p_entry->entry.type = %d\n",
                                       p_entry->entry.type);
                        return JPEGERR_EBADPARM;
                    }
                }
            }
            else
            {
                switch (p_entry->entry.type)
                {
                case EXIF_BYTE:
                    MPO_WRITE_BYTE_SCRATCH((uint32_t)p_entry->entry.data._byte);
                    bytes_written = 1;
                    break;
                case EXIF_SHORT:
                    MPO_WRITE_SHORT_SCRATCH((uint32_t)p_entry->entry.data._short);
                    bytes_written = 2;
                    break;
                case EXIF_LONG:
                    MPO_WRITE_LONG_SCRATCH((uint32_t)p_entry->entry.data._long);
                    bytes_written = 4;
                    break;
                case EXIF_SLONG:
                    MPO_WRITE_LONG_SCRATCH((uint32_t)p_entry->entry.data._slong);
                    bytes_written = 4;
                    break;
                default:
                    JPEG_DBG_ERROR("jpegw_emit_exif_tag: impossible case! p_entry->entry.type = %d\n",
                                   p_entry->entry.type);
                    return JPEGERR_EBADPARM;
                }
            }
        }
        // Fill up 0's till there are totally 4 bytes written
        for (i = bytes_written; i < 4; i++)
            MPO_WRITE_BYTE_SCRATCH(0);
    }
    else // if (to_write_len <= 4)
    {
        if (p_writer->aheadBuf->offset & 1) p_writer->aheadBuf->offset++;

        // Write the temporary offset (to be updated later)
        MPO_WRITE_LONG_SCRATCH(p_writer->aheadBuf->offset);

        if (p_entry->entry.type == EXIF_ASCII || p_entry->entry.type == EXIF_UNDEFINED)
        {
            MPO_WRITE_NBYTES_VALUE(p_entry->entry.data._ascii, p_entry->entry.count);
        }
        else if (p_entry->entry.count > 1)
        {
            // Multiple data to write
            for (i = 0; i < p_entry->entry.count; i++)
            {
                switch (p_entry->entry.type)
                {
                case EXIF_BYTE:
                    MPO_WRITE_BYTE_VALUE(p_entry->entry.data._bytes[i]);
                    break;
                case EXIF_SHORT:
                    MPO_WRITE_SHORT_VALUE(p_entry->entry.data._shorts[i]);
                    break;
                case EXIF_LONG:
                    MPO_WRITE_LONG_VALUE(p_entry->entry.data._longs[i]);
                    break;
                case EXIF_SLONG:
                    MPO_WRITE_LONG_VALUE(p_entry->entry.data._slongs[i]);
                    break;
                case EXIF_RATIONAL:
                    MPO_WRITE_LONG_VALUE(p_entry->entry.data._rats[i].num);
                    MPO_WRITE_LONG_VALUE(p_entry->entry.data._rats[i].denom);
                    break;
                case EXIF_SRATIONAL:
                    MPO_WRITE_LONG_VALUE(p_entry->entry.data._srats[i].num);
                    MPO_WRITE_LONG_VALUE(p_entry->entry.data._srats[i].denom);
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            switch (p_entry->entry.type)
            {
            case EXIF_BYTE:
                MPO_WRITE_BYTE_VALUE(p_entry->entry.data._byte);
                break;
            case EXIF_SHORT:
                MPO_WRITE_SHORT_VALUE(p_entry->entry.data._short);
                break;
            case EXIF_LONG:
                MPO_WRITE_LONG_VALUE(p_entry->entry.data._long);
                break;
            case EXIF_SLONG:
                MPO_WRITE_LONG_VALUE(p_entry->entry.data._slong);
                break;
            case EXIF_RATIONAL:
                MPO_WRITE_LONG_VALUE(p_entry->entry.data._rat.num);
                MPO_WRITE_LONG_VALUE(p_entry->entry.data._rat.denom);
                break;
            case EXIF_SRATIONAL:
                MPO_WRITE_LONG_VALUE(p_entry->entry.data._srat.num);
                MPO_WRITE_LONG_VALUE(p_entry->entry.data._srat.denom);
                break;
            default:
                break;
            }
        }
    }
    // update field count
    p_writer->field_count++;

    return JPEGERR_SUCCESS;
}

void mpow_write_value_ifd(mpo_writer_t *p_writer, uint32_t src_id)
{
  uint32_t i;
  uint32_t ahead_dest;
  // extra 4 bytes for NextIfdOffset
  ahead_dest = p_writer->field_count_offset + p_writer->field_count * 12 + 4;
  for (i = 0; i < p_writer->field_count; i++)
  {
      uint32_t nAheadOffset;
      // extra 2 bytes for count
      uint32_t nWriteOffset = p_writer->field_count_offset + i * 12;
      // 2 bytes tag, 2 bytes type, 4 bytes count
      uint32_t dest_offset = nWriteOffset + 8;

      // update offset if dataLen is greater than 4
      const uint32_t type = writer_read_short_little(p_writer->scratchBuf->ptr,
        nWriteOffset + 2);
      const uint32_t count = writer_read_long_little(p_writer->scratchBuf->ptr,
        nWriteOffset + 4);

      // tag_type_sizes is with size 11
      // valid type is 0 to 10
      if (type >= 11)
      {
        JPEG_DBG_ERROR("%s: invalid tag type %d in field %d\n",
          __func__, type, i);
      }
      else if (count * tag_type_sizes[type] > 4)
      {
        nAheadOffset =
          writer_read_long_little(p_writer->scratchBuf->ptr, nWriteOffset + 8);

          // adjust offset by adding
          writer_emit_long_little(nAheadOffset + ahead_dest +
            p_writer->app2_start_offset[src_id] -
            p_writer->app2_start_of_offset,
            p_writer->scratchBuf->ptr, &dest_offset,
            p_writer->scratchBuf->size,
            &(p_writer->overflow_flag));
      }
  }

  (void)STD_MEMMOVE(p_writer->scratchBuf->ptr + ahead_dest,
                    p_writer->aheadBuf->ptr, p_writer->aheadBuf->offset);
  if (src_id == 0)
  {
      p_writer->app2_mp_entry_value_offset = ahead_dest;
  }
  p_writer->scratchBuf->offset += p_writer->aheadBuf->offset;
  p_writer->aheadBuf->offset = 0;
  p_writer->field_count = 0;
}


/******************************************************************************
 * Function: mpow_emit_index_ifd
 * Description: Emit index ifd.
 * Input parameters:
 *   p_writer  - The Mpo Writer object.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpow_emit_index_ifd(mpo_writer_t *p_writer)
{
    uint32_t i;
    uint32_t format_version = MP_DEFAULT_FORMAT_VERSION;
    uint32_t offset_of_next_ifd = 0, offset_of_next_ifd_value = 0;
    uint32_t count_offset = 0;
    uint16_t count_value = 0;
    exif_tag_entry_ex_t dummy_entry;
    exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)(p_writer->p_mpo_info);

    // Validate inputs
    if (!p_writer)
    {
        return JPEGERR_ENULLPTR;
    }

    count_offset = p_writer->scratchBuf->offset;
    MPO_WRITE_SHORT_SCRATCH(0x0000);

    p_writer->field_count = 0;
    p_writer->field_count_offset = p_writer->scratchBuf->offset;

    // Write MP Format Version: 12 bytes
    dummy_entry.entry.count = 4;
    dummy_entry.entry.type = (exif_tag_type_t)EXIF_UNDEFINED;
    dummy_entry.entry.data._undefined = &format_version;
    dummy_entry.tag_id = _ID_MP_F_VERSION;

    mpow_emit_app2_tag(p_writer, &dummy_entry);
    count_value ++;

    // Write Number of Image: 12 bytes
    if (p_writer->source_cnt > MAX_IMAGES)
    {
        JPEG_DBG_ERROR("mpow_emit_app2: invalid MPO input images number\n");
        return JPEGERR_EBADPARM;
    }
    dummy_entry.entry.count = 1;
    dummy_entry.entry.type = (exif_tag_type_t)EXIF_LONG;
    dummy_entry.entry.data._long = p_writer->source_cnt;
    dummy_entry.tag_id = _ID_NUMBER_OF_IMAGES;

    mpow_emit_app2_tag(p_writer, &dummy_entry);
    count_value ++;

    // Emit tags in index IFD
    for (i = MP_INDEX_IFD_START; i < MPO_INDEX_IFD_SIZE; i++)
    {
        if (pp_entries[i])
        {
           mpow_emit_app2_tag(p_writer, pp_entries[i]);
           count_value ++;
        }
    }

    // Update index ifd count
    writer_overwrite_short_little(count_value,
      p_writer->scratchBuf->ptr, count_offset,
      p_writer->scratchBuf->size, &(p_writer->overflow_flag));
// Write Offset to Next IFD : 4 types
    // Start of Offset is MP Endian
    offset_of_next_ifd = p_writer->scratchBuf->offset;
    MPO_WRITE_LONG_SCRATCH(0x00);

    mpow_write_value_ifd(p_writer, 0);

    // Calcute offset of next ifd
    offset_of_next_ifd_value = p_writer->app2_start_offset[0] +
      p_writer->scratchBuf->offset - p_writer->app2_start_of_offset;

    // Update offset of next ifd
    writer_overwrite_long_little(offset_of_next_ifd_value,
       p_writer->scratchBuf->ptr, offset_of_next_ifd,
       p_writer->scratchBuf->size, &(p_writer->overflow_flag));

    return JPEGERR_SUCCESS;

}

/******************************************************************************
 * Function: mpow_emit_attribute_ifd
 * Description: Emit attribut ifd.
 * Input parameters:
 *   p_writer  - The Mpo Writer object.
 *   src_id    - The source id.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpow_emit_attribute_ifd(mpo_writer_t *p_writer, uint32_t src_id)
{
    int i;
    uint32_t count_offset = 0;
    uint16_t count_value = 0;
    exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)(&p_writer->p_mpo_info->attribute_ifds[src_id]);

    // Validate inputs
    if (!p_writer)
    {
        return JPEGERR_ENULLPTR;
    }

    count_offset = p_writer->scratchBuf->offset;
    MPO_WRITE_SHORT_SCRATCH(0x0000);

    p_writer->field_count = 0;
    p_writer->field_count_offset = p_writer->scratchBuf->offset;

    // Emit tags in attribute IFD
    for (i = 0; i < MPO_ATTRIBUTE_IFD_SIZE; i++)
    {
        if (pp_entries[i])
        {
           mpow_emit_app2_tag(p_writer, pp_entries[i]);
           count_value ++;
        }
    }

    // Update attributes ifd count
    writer_overwrite_short_little(count_value,
      p_writer->scratchBuf->ptr, count_offset,
      p_writer->scratchBuf->size, &(p_writer->overflow_flag));

    // Write Offset to Next IFD : the value shall be 0 (bytes).
    MPO_WRITE_LONG_SCRATCH(0x00);

    // Write value IFD
    mpow_write_value_ifd(p_writer, 1);
    return JPEGERR_SUCCESS;

}

/******************************************************************************
 * Function: mpow_fill_app2_header_first
 * Description: Fill the App2 header for the first individual images.
 * Input parameters:
 *   p_writer   - The Mpo Writer object.
 *   buf_offset - The output buffer offset.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpow_fill_app2_header_first(
    mpo_writer_t      *p_writer,
    uint32_t           buf_offset)    // The buffer offset
{
    int rc = JPEGERR_SUCCESS;
    uint32_t i;
    uint32_t offset = 0;

    // Validate inputs
    if (!p_writer)
    {
        return JPEGERR_ENULLPTR;
    }
    p_writer->scratchBuf = p_writer->header_buf[0];
    p_writer->aheadBuf = p_writer->value_buf[0];
    p_writer->image_size[0] = p_writer->total_images_size;

    WRITE_SHORT_SCRATCH(p_writer->app2_header_length);

    // Write A unique identifier used to distinguish this
    // APP2 marker as a stereoscopic descriptor.
    // These 4 bytes should always be “MPF ” 
    WRITE_LONG_SCRATCH(MP_FORMAT);

    // Start to Write MP Header
    // Writer MP Endian, same as EXIF Endian 
    p_writer->app2_start_of_offset = p_writer->app2_start_offset[0] +
      p_writer->scratchBuf->offset;
    WRITE_LONG_SCRATCH(MP_LITTLE_ENDIAN);

    // Write Offset to First IFD
    // Start of Offset is MP Endian
    // Either "MP Index IFD" or "MP Attributes IFD" is immediately after
    // "Offset to First IFD", the value shall be 8 (bytes).
    MPO_WRITE_LONG_SCRATCH(0x08);

    // Write MP index ifd
    rc = mpow_emit_index_ifd(p_writer);
    if (JPEG_FAILED(rc))
    {
        return rc;
    }

    // Write MP attribute ifd
    if (!p_writer->large_thumbnail_flag[0])
    {
        rc = mpow_emit_attribute_ifd(p_writer, 0);
    }
    if (JPEG_FAILED(rc))
    {
        return rc;
    }

    // Write MP entry value
    offset = p_writer->scratchBuf->offset;
    p_writer->scratchBuf->offset = p_writer->app2_mp_entry_value_offset;

    // Write Entry value:
    // Individual Image Attribute:
    MPO_WRITE_LONG_SCRATCH(ATTRIBUTE(p_writer->source[0].image_attribute));

    // Individual Image Size:
    MPO_WRITE_LONG_SCRATCH(p_writer->image_size[0]);

    // Individual Image Data Offset:
    MPO_WRITE_LONG_SCRATCH(0x00);

    if (p_writer->large_thumbnail_flag[0])
    {
        // Dependent Image 1 Entry Number:
        MPO_WRITE_SHORT_SCRATCH(p_writer->source_cnt);
    }
    else
    {
        // Dependent Image 1 Entry Number:
        MPO_WRITE_SHORT_SCRATCH(0x0000);
    }
    // Dependent Image 2 Entry Number:
    MPO_WRITE_SHORT_SCRATCH(0x0000);

    for (i = 1; i < p_writer->source_cnt; i++)
    {
        // Write Entry value:
        MPO_WRITE_LONG_SCRATCH(ATTRIBUTE(p_writer->source[i].image_attribute));
        // Individual Image Size:
        p_writer->app2_image_size_offset = p_writer->scratchBuf->offset;
        MPO_WRITE_LONG_SCRATCH(0x00);
        // Individual Image data offset: 4 bytes
        p_writer->app2_data_offset_offset = p_writer->scratchBuf->offset;
        MPO_WRITE_LONG_SCRATCH(0x00);

        // Dependent Image 1 Entry Number: 2 bytes
        MPO_WRITE_SHORT_SCRATCH(0x0000);

        // Dependent Image 2 Entry Number: 2 bytes
        MPO_WRITE_SHORT_SCRATCH(0x0000);
    }
    p_writer->scratchBuf->offset = offset;

    return rc;
}

/******************************************************************************
 * Function: mpow_fill_app2_header
 * Description: Fill the App2 header for the other individual images.
 * Input parameters:
 *   p_writer  - The Mpo Writer object.
 *   src_id    - The source id.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpow_fill_app2_header(
    mpo_writer_t        *p_writer,
    uint32_t             src_id)
{
    int rc = JPEGERR_SUCCESS;
    uint32_t i, images_size = 0;

    // Validate inputs
    if (!p_writer)
    {
        return JPEGERR_ENULLPTR;
    }

    // When MP file contains large thumbnail,
    // The App2 shall not be recorded
    if (p_writer->large_thumbnail_flag[src_id])
    {
        JPEG_DBG_ERROR("mpow_fill_app2_header: large thumbnail\n");
        return JPEGERR_SUCCESS;
    }

    if (src_id >= MAX_IMAGES)
    {
        return JPEGERR_EBADPARM;
    }
    p_writer->scratchBuf = p_writer->header_buf[src_id];
    p_writer->aheadBuf = p_writer->value_buf[src_id];

    p_writer->field_count = 0;
    p_writer->field_count_offset = p_writer->scratchBuf->offset;

    // Update App2 start offset inside individual image to from first image
    for (i = 0; i < src_id; i++)
    {
       p_writer->app2_start_offset[src_id] += p_writer->image_size[i];
       images_size += p_writer->image_size[i];
    }
    p_writer->image_size[src_id] = p_writer->total_images_size - images_size -
      p_writer->last_image_offset[src_id - 1];

    // Update first App2 header image size field
    writer_overwrite_long_little ((uint32_t)(p_writer->image_size[src_id]),
      p_writer->header_buf[0]->ptr, p_writer->app2_image_size_offset,
      p_writer->header_buf[0]->size, &(p_writer->overflow_flag));

    // Update first App2 header image data offset field
    writer_overwrite_long_little ((uint32_t)(images_size -
      p_writer->app2_start_of_offset),
      p_writer->header_buf[0]->ptr, p_writer->app2_data_offset_offset,
      p_writer->header_buf[0]->size, &(p_writer->overflow_flag));

    WRITE_SHORT_SCRATCH(p_writer->app2_header_length);

    // Write A unique identifier used to distinguish this
    // APP2 marker as a stereoscopic descriptor.
    // These 4 bytes should always be “MPF ” 
    WRITE_LONG_SCRATCH(MP_FORMAT);

    // Write MP Header
    // Writer MP Endian:  
    WRITE_LONG_SCRATCH(MP_LITTLE_ENDIAN);

    // Write Offset to First IFD
    // Start of Offset is MP Endian
    // Either "MP Index IFD" or "MP Attributes IFD" is immediately after
    // "Offset to First IFD", the value shall be 8 (bytes).
    MPO_WRITE_LONG_SCRATCH(0x08);

    // Write MP attribute ifd
    rc = mpow_emit_attribute_ifd(p_writer, src_id);
    return rc;
}

/******************************************************************************
 * Function: mpow_get_app2_header
 * Description: Get the app2 data for individual image.
 * Input parameters:
 *   p_writer    - The Mpo Writer object.
 *   p_app2_data - The App2 header data.
 *   src_id      - The source count.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpow_get_app2_header(
    mpo_writer_t        *p_writer,
    mpoe_app2_data_t    *p_app2_data,
    uint32_t             src_id)
{
    // Validate inputs
    if (!p_writer)
    {
        return JPEGERR_ENULLPTR;
    }

    // Validate source id
    if (src_id > p_writer->source_cnt)
    {
        return JPEGERR_EBADPARM;
    }

    // If App2 header written
    if (p_writer->header_buf[src_id]->offset)
    {
        // Copy App2 start offset
        p_app2_data->start_offset = p_writer->app2_start_offset[src_id];

        // Copy App2 header data
        STD_MEMMOVE(p_app2_data->data_buf->ptr, p_writer->header_buf[src_id]->ptr, p_writer->header_buf[src_id]->offset);

        p_app2_data->data_buf->offset += p_writer->header_buf[src_id]->offset;
    }
    return JPEGERR_SUCCESS;
}
