/*========================================================================


*//** @file jpeg_writer.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-11 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
01/20/11   staceyw Added support for App2 header and Removed common
                   writer unitlity funtions into new file.
09/27/10   mingy   Removed emitting duplicated exif tags.
07/30/10   mingy   Fixed for-loop boundaries in EXIF writing functions.
07/22/10   sigu    Fixed compiler warnings
07/06/10   staceyw Fixed the warning in jpegw_finish_ifd.
05/20/10   staceyw Update upsampler_cfg to scale_cfg for upscale or downscale.
04/20/10   leim    Added mobicat data support implementation
04/15/10   staceyw Fixed the bug of Pixel X and Y Dimension when write EXIF
                   Ifd. Change the dimension to be scaled dimension when
                   scale is enabled.
02/16/10   staceyw Change to return error codes during emit header,
                   and delete jpegw_emit_thumbnail, and added the pass of
                   thumbnail buffer in jpegw_emit_header from encoder
                   intead of allocating one in jpeg writer. Then engine uses
                   this buffer for output during encoding thumbnail.
02/08/10   staceyw Added support for bitstream input.
10/13/09   vma     Changed to pass output to upper layer (encoder) instead
                   of outputting through destination callback directly.
05/12/09   vma     Fixed bug where EXIF tags with ASCII or UNDEFINED type
                   with less than 4 bytes data is not padded to 4 byte
                   boundary.
04/23/09   vma     Added reporting of the event where thumbnail is
                   too big and is dropped.
10/15/08   vma     Fixed improper resetting when jpeg_writer is
                   used after the first time. (Thumbnail buffer not
                   resetted)
09/29/08   vma     Fixed issue with rotation with H2V1/H1V2 images.
09/12/08   vma     Added upsampler support.
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_writer.h"
#include "writer_utility.h"   //TBD
#include "jpeg_buffer_private.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "exif_private.h"
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
static const int jpeg_natural_order [64+16] = {
    0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63,
    63, 63, 63, 63, 63, 63, 63, 63, /* extra entries for safety */
    63, 63, 63, 63, 63, 63, 63, 63
};

extern exif_tag_entry_ex_t default_tag_interopindexstr;
extern exif_tag_entry_ex_t default_tag_r98_version;
static const char mobicat_magic_string[] = "Qualcomm Camera Attributes v2";
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
#define ALLOCATETAGSIZE 2

#define EXIF_BYTE      1
#define EXIF_ASCII     2
#define EXIF_SHORT     3
#define EXIF_LONG      4
#define EXIF_RATIONAL  5
#define EXIF_UNDEFINED 7
#define EXIF_SLONG     9
#define EXIF_SRATIONAL 10
// color space type
#define COLOR_SPACE_SRGB    1
// ycbcr positioning type
#define YCBCR_CENTER        1

// The starting APP segment number
#define MOBICAT_APP_MARKER           0xE5  // APP5
// The mobicat magic string length
#define MOBICAT_MAGIC_STRLEN         (sizeof(mobicat_magic_string)-1)
// Maximum length of mobicat data in each APP segment
#define MAX_MOBICAT_LENGTH_PER_APP   (0xFFFF - 3 - MOBICAT_MAGIC_STRLEN)
// Maximum length of the mobicat data supported
#define MAX_MOBICAT_LENGTH           ((0xEF - MOBICAT_APP_MARKER + 1) * MAX_MOBICAT_LENGTH_PER_APP)

#define CHK(rc)              {if (JPEG_FAILED (rc)) return rc;}
#define WRITE_BYTE_SCRATCH(b) \
    jpegw_emit_byte((uint8_t)(b), p_writer->scratchBuf->ptr, &(p_writer->scratchBuf->offset), \
    p_writer->scratchBuf->size, &(p_writer->overflow_flag))
#define WRITE_SHORT_SCRATCH(s) \
    jpegw_emit_short((uint16_t)(s), p_writer->scratchBuf->ptr, &(p_writer->scratchBuf->offset), \
    p_writer->scratchBuf->size, &(p_writer->overflow_flag))
#define WRITE_LONG_SCRATCH(l) \
    jpegw_emit_long((int)(l), p_writer->scratchBuf->ptr, &(p_writer->scratchBuf->offset), \
    p_writer->scratchBuf->size, &(p_writer->overflow_flag))
#define WRITE_NBYTES_SCRATCH(b, c) \
    jpegw_emit_nbytes((uint8_t *)(b), c, p_writer->scratchBuf->ptr, &(p_writer->scratchBuf->offset), \
    p_writer->scratchBuf->size, &(p_writer->overflow_flag))
#define WRITE_BYTE_AHEAD(b) \
    jpegw_emit_byte((uint8_t)(b), p_writer->aheadBuf->ptr, &(p_writer->aheadBuf->offset), \
    p_writer->aheadBuf->size, &(p_writer->overflow_flag))
#define WRITE_SHORT_AHEAD(s) \
    jpegw_emit_short((uint16_t)(s), p_writer->aheadBuf->ptr, &(p_writer->aheadBuf->offset), \
    p_writer->aheadBuf->size, &(p_writer->overflow_flag))
#define WRITE_LONG_AHEAD(l) \
    jpegw_emit_long((int)(l), p_writer->aheadBuf->ptr, &(p_writer->aheadBuf->offset), \
    p_writer->aheadBuf->size, &(p_writer->overflow_flag))
#define WRITE_NBYTES_AHEAD(b, c) \
    jpegw_emit_nbytes((uint8_t *)(b), c, p_writer->aheadBuf->ptr, &(p_writer->aheadBuf->offset), \
    p_writer->aheadBuf->size, &(p_writer->overflow_flag))

/* =======================================================================
**                          Function Definitions
** ======================================================================= */
int jpegw_init(jpeg_writer_t          *p_writer,
               jpege_obj_t             encoder,
               jpegw_output_handler_t  p_handler)
{
    int rc = JPEGERR_SUCCESS;

    // Validate inputs
    if (!p_writer)
        return JPEGERR_ENULLPTR;

    // Zero all fields in the structure
    STD_MEMSET(p_writer, 0, sizeof(jpeg_writer_t));
    p_writer->encoder   = encoder;
    p_writer->p_handler = p_handler;

    // Initialize and allocate scratch, ahead and thumbnail buffer
    if (JPEG_SUCCEEDED(rc))
    {
        rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_writer->scratchBuf));
    }
    if (JPEG_SUCCEEDED(rc))
    {
        rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_writer->aheadBuf));
    }
/* _NOTSURE_ */
#if 0
    if (JPEG_SUCCEEDED(rc))
    {
        rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_writer->thumbnailBuf));
    }
#endif
    // Allocate scratch and ahead buffer
    if (JPEG_SUCCEEDED(rc))
    {
        rc = jpeg_buffer_allocate((jpeg_buffer_t)p_writer->scratchBuf,
                                  EXIF_MAX_APP1_LENGTH, false);
    }
    if (JPEG_SUCCEEDED(rc))
    {
        rc = jpeg_buffer_allocate((jpeg_buffer_t)p_writer->aheadBuf,
                                  EXIF_MAX_APP1_LENGTH, false);
    }
    if (JPEG_FAILED(rc))
    {
        jpegw_destroy(p_writer);
    }
    return rc;
}

void jpegw_configure(jpeg_writer_t    *p_writer,
                     jpege_src_t      *p_source,
                     jpege_dst_t      *p_dest,
                     jpege_cfg_t      *p_encode_cfg,
                     jpege_mbc_t      *p_mobicat_data,
                     exif_info_obj_t  *p_exif_info_obj)
{
    jpegw_reset (p_writer);

    // Store p_dest, encoder, p_source and p_encode_cfg
    p_writer->p_dest          = p_dest;
    p_writer->p_source        = p_source;
    p_writer->p_encode_cfg    = p_encode_cfg;
    JPEG_DBG_MED("%s,%d\n",__func__,__LINE__);

    // check if App2 header exist, if so set the length
    if (p_encode_cfg->app2_header_length)
    {
        p_writer->app2_present = true;

        // Set App2 header length
        p_writer->app2_header_length = p_encode_cfg->app2_header_length;
    }
    JPEG_DBG_MED("%s,%d, p_mobicat_data->length: %d\n",__func__,__LINE__,
      p_mobicat_data->length);
    // check if mobicat data exist, if so set it
    if(p_mobicat_data->data != NULL)
    {
        JPEG_DBG_MED("%s,%d] mobicat_data.length: %d Addr is %d\n",
          __func__, __LINE__,
        p_mobicat_data->length,( uint8_t *)p_mobicat_data->data);
    }
    if (p_mobicat_data->length)
    {
        p_writer->p_mobicat_data = p_mobicat_data;
    }
    // set exif info
    if (p_exif_info_obj)
    {
        if (p_writer->p_exif_info && p_writer->is_exif_info_owned)
        {
            exif_destroy((exif_info_obj_t *)(&(p_writer->p_exif_info)));
        }
        p_writer->p_exif_info = (exif_info_t *)(*p_exif_info_obj);
    }
}

void jpegw_destroy(jpeg_writer_t *p_writer)
{
    JPEG_DBG_LOW("jpegw_destroy\n");
    // Free scratch buffer if allocated
    jpeg_buffer_destroy((jpeg_buffer_t *)&(p_writer->scratchBuf));
    // Free ahead buffer if allocated
    jpeg_buffer_destroy((jpeg_buffer_t *)&(p_writer->aheadBuf));
/* _NOTSURE_ */
    // Free thumbnail buffer if allocated

#if 0
    jpeg_buffer_destroy((jpeg_buffer_t *)&(p_writer->thumbnailBuf));
#endif
    // Destroy exif info if owned by the writer
    if (p_writer->is_exif_info_owned)
    {
        exif_destroy((exif_info_obj_t *)(&(p_writer->p_exif_info)));
    }
}

void jpegw_reset(jpeg_writer_t *p_writer)
{
    p_writer->nTiffHeaderOffset = 0;
    p_writer->nApp1LengthOffset = 0;
    p_writer->nThumbnailOffset = 0;
    p_writer->nThumbnailStreamOffset = 0;
    p_writer->nGpsIfdPointerOffset = 0;
    p_writer->nThumbnailIfdPointerOffset = 0;
    p_writer->nFieldCount = 0;
    p_writer->nFieldCountOffset = 0;
    p_writer->nJpegInterchangeLOffset = 0;
    p_writer->fHeaderWritten = 0;
    p_writer->fApp1Present = false;
    p_writer->app2_present = false;
    p_writer->app2_start_offset = 0;
    p_writer->overflow_flag = false;

    p_writer->aheadBuf->offset = 0;
    p_writer->scratchBuf->offset = 0;

    // Clean up thumbnail buffer
    p_writer->thumbnailBuf = NULL;
}

static int jpegw_flush_scratch_buffer(jpeg_writer_t *p_writer)
{
    int  rc = JPEGERR_SUCCESS;
    // Nothing to flush
    if (!(p_writer->scratchBuf->offset))
        return JPEGERR_SUCCESS;

    // Scratch buffer overflew
    if (p_writer->overflow_flag)
    {
        JPEG_DBG_HIGH("jpegw_flush_scratch_buffer: scratch buffer overflew earlier\n");
        return JPEGERR_EFAILED;
    }
    // Invoke output handler
    rc = p_writer->p_handler(p_writer->encoder, p_writer->scratchBuf);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_MED("jpegw_flush_scratch_buffer: failed to output\n");
        return rc;
    }

    // Reset the offset of the scratch buffer
    p_writer->scratchBuf->offset = 0;
    return rc;
}

static int jpegw_flush_thumbnail_buffer(jpeg_writer_t *p_writer)
{
    int  rc = JPEGERR_SUCCESS;
    if (p_writer->thumbnailBuf->offset)
    {
        // Invoke output handler
        rc = p_writer->p_handler(p_writer->encoder, p_writer->thumbnailBuf);
    }
    return rc;
}

int jpegw_write_mobicat_data(jpeg_writer_t *p_writer)
{
    int rc = JPEGERR_SUCCESS;
    jpeg_marker_t app_marker = MOBICAT_APP_MARKER;
    uint32_t bytes_to_write = 0;
    uint32_t bytes_written = 0;
    uint32_t bytes_per_write = 0;
    uint32_t mobicat_total_length;
    if (!p_writer->p_mobicat_data)
        return JPEGERR_SUCCESS;
    if (!p_writer->p_mobicat_data->length)
        return JPEGERR_SUCCESS;
     mobicat_total_length = p_writer->p_mobicat_data->length;

    // Truncate mobicat data if the length is over the limit
    if (mobicat_total_length > MAX_MOBICAT_LENGTH)
        mobicat_total_length = MAX_MOBICAT_LENGTH;
    JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
    while (bytes_written < mobicat_total_length)
    {
        JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
        bytes_to_write = STD_MIN((mobicat_total_length - bytes_written),
                                 (0xFFFF - 3 - MOBICAT_MAGIC_STRLEN));
        // Write apps marker
        WRITE_SHORT_SCRATCH(0xFF00 | app_marker);

        // write segment size (2 bytes)
        WRITE_SHORT_SCRATCH((uint16_t)(bytes_to_write + 3 + MOBICAT_MAGIC_STRLEN));

        // write the length of magic string (1 byte)
        WRITE_BYTE_SCRATCH(MOBICAT_MAGIC_STRLEN);

        // write mobicat magic string length
        WRITE_NBYTES_SCRATCH((uint8_t *)mobicat_magic_string, MOBICAT_MAGIC_STRLEN);

        // write mobicat data
        // do it in a loop to make sure there is no overflow problem
        while (bytes_to_write)
        {
            // check how many byte scratch buffer can take
            bytes_per_write = STD_MIN(bytes_to_write,
                                      (uint32_t)(p_writer->scratchBuf->size - 1 - p_writer->scratchBuf->offset));
            WRITE_NBYTES_SCRATCH(p_writer->p_mobicat_data->data + bytes_written, bytes_per_write);
            bytes_written  += bytes_per_write;
            bytes_to_write -= bytes_per_write;
            rc = jpegw_flush_scratch_buffer(p_writer);
            CHK(rc);
        }
        // increase apps marker
        app_marker++;
    }
    // delete  mobicat data
    p_writer->p_mobicat_data->length = 0;
    JPEG_FREE(p_writer->p_mobicat_data->data);
    return JPEGERR_SUCCESS;
}

int jpegw_flush_file_header(jpeg_writer_t *p_writer)
{
    int rc;
    int thumbnail_dropped = 0;
    JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
    if ((p_writer->thumbnailBuf) &&
        (p_writer->thumbnailBuf->ptr))
    {
        // drop the thumbnail if it is larger than the allowable size or the allocated
        // buffer has overflown
        if ( (p_writer->fApp1Present && p_writer->thumbnailBuf->offset + p_writer->scratchBuf->offset
              - p_writer->nApp1LengthOffset > EXIF_MAX_APP1_LENGTH) ||
             (p_writer->thumbnailBuf->offset == p_writer->thumbnailBuf->size))
        {
            // Update App1 length field
            jpegw_overwrite_short ((uint16_t)(p_writer->nThumbnailOffset - p_writer->nApp1LengthOffset),
                                   p_writer->scratchBuf->ptr, p_writer->nApp1LengthOffset,
                                   p_writer->scratchBuf->size, &(p_writer->overflow_flag));

            // set thumbnail IFD pointer to NULL
            jpegw_overwrite_long (0, p_writer->scratchBuf->ptr,
                                  p_writer->nThumbnailIfdPointerOffset,
                                  p_writer->scratchBuf->size,
                                  &(p_writer->overflow_flag));

            // reset the scratch buffer offset to prior to writing the thumbnail IFD
            p_writer->scratchBuf->offset = p_writer->nThumbnailOffset;

            // clean up the thumbnail buffer
            p_writer->thumbnailBuf->offset = 0;

            thumbnail_dropped = 1;
        }
        else
        {
            if (p_writer->fApp1Present)
            {
                // Update Jpeg Interchange Format Length
                jpegw_overwrite_long ((p_writer->scratchBuf->offset - p_writer->nThumbnailStreamOffset) +
                                      p_writer->thumbnailBuf->offset,
                                      p_writer->scratchBuf->ptr,
                                      p_writer->nJpegInterchangeLOffset,
                                      p_writer->scratchBuf->size,
                                      &(p_writer->overflow_flag));
            }

            // Update App1 length field
            jpegw_overwrite_short ((uint16_t) ((p_writer->scratchBuf->offset - p_writer->nApp1LengthOffset)+ p_writer->thumbnailBuf->offset),
                                   p_writer->scratchBuf->ptr,
                                   p_writer->nApp1LengthOffset,
                                   p_writer->scratchBuf->size,
                                   &(p_writer->overflow_flag));
        }
    }
    // Flush out whatever is in the scratch buffer for the header

    if (p_writer->app2_present)
    {
        // Update APP2 start offset
        p_writer->app2_start_offset += p_writer->scratchBuf->offset;
    }

    // Flush out whatever is in the scratch buffer for the header
    rc = jpegw_flush_scratch_buffer(p_writer);
    CHK(rc);
    // Flush the thumbnail
    if (p_writer->p_source->p_thumbnail)
    {
        if (p_writer->thumbnailBuf)
        {
            rc = jpegw_flush_thumbnail_buffer(p_writer);
            if (JPEG_FAILED(rc))
            {
                JPEG_DBG_MED("jpegw_flush_file_header: failed to output\n");
                return rc;
            }
        }
        else
        {
            JPEG_DBG_MED("jpegw_flush_file_header: failed to output thumbnail\n");
            return JPEGERR_EBADSTATE;
        }
    }
    if (p_writer->app2_present)
    {
        // Update APP2 start offset
        p_writer->app2_start_offset += p_writer->thumbnailBuf->offset;
    }

    // Reset scratch buffer
    p_writer->scratchBuf->offset = 0;
    if (p_writer->app2_present)
    {
        // Write APP2 Marker
        WRITE_SHORT_SCRATCH(0xFF00 | M_APP2);
        // Update APP2 start offset
        p_writer->app2_start_offset += p_writer->scratchBuf->offset;

        // Reserve number of bytes
        p_writer->scratchBuf->offset += p_writer->app2_header_length;
    }

    if (p_writer->p_mobicat_data)
    {
        rc = jpegw_write_mobicat_data(p_writer);
        JPEG_DBG_LOW("%s,%d:rc= %d\n",__func__,__LINE__,rc);
        CHK(rc);
    }

    jpegw_emit_frame_header(p_writer, &(p_writer->p_encode_cfg->main_cfg), p_writer->p_source->p_main);

    jpegw_emit_scan_header(p_writer, &(p_writer->p_encode_cfg->main_cfg));

    rc = jpegw_flush_scratch_buffer(p_writer);

    if (JPEG_SUCCEEDED(rc) && thumbnail_dropped)
        rc = JPEGERR_THUMBNAIL_DROPPED;

    return rc;
}

void jpegw_start_ifd(jpeg_writer_t *p_writer)
{
    p_writer->nFieldCount = 0;
    p_writer->nFieldCountOffset = p_writer->scratchBuf->offset;
    p_writer->scratchBuf->offset += 2;
}

void jpegw_finish_ifd(jpeg_writer_t *p_writer)
{
    uint32_t i;
    // the offset in scratch buffer where ahead buffer should be copied to
    int nAheadBufDestination;

    // Emit Next Ifd pointer
    WRITE_LONG_SCRATCH(0);

    // Update the number of fields written and flush the ahead buffer to scratch buffer
    jpegw_emit_short((int16_t)(p_writer->nFieldCount), p_writer->scratchBuf->ptr,
                     &(p_writer->nFieldCountOffset), p_writer->scratchBuf->size,
                     &(p_writer->overflow_flag));

    nAheadBufDestination = p_writer->nFieldCountOffset + p_writer->nFieldCount * 12 + 4; // extra 4 bytes for NextIfdOffset
    for (i = 0; i < p_writer->nFieldCount; i++)
    {
        uint32_t nAheadOffset;
        uint32_t nWriteOffset = p_writer->nFieldCountOffset + i * 12 + 8;

        // update offset if dataLen is greater than 4
        const uint32_t type = jpegw_read_short(p_writer->scratchBuf->ptr, nWriteOffset - 6);
        const uint32_t count = jpegw_read_long(p_writer->scratchBuf->ptr, nWriteOffset - 4);

        // tag_type_sizes is with size 11
        // valid type is 0 to 10
        if (type >= 11)
        {
            JPEG_DBG_ERROR("jpegw_finish_ifd: invalid tag type %d in field %d\n", type, i);
        }
        else if (count * tag_type_sizes[type] > 4)
        {
            nAheadOffset = jpegw_read_long(p_writer->scratchBuf->ptr, nWriteOffset);

            // adjust offset by adding
            jpegw_emit_long(nAheadOffset + nAheadBufDestination - p_writer->nTiffHeaderOffset,
                            p_writer->scratchBuf->ptr, &nWriteOffset, p_writer->scratchBuf->size,
                            &(p_writer->overflow_flag));
        }
    }
    // copy ahead buffer to scratch buffer
    (void)STD_MEMMOVE(p_writer->scratchBuf->ptr + nAheadBufDestination,
                      p_writer->aheadBuf->ptr, p_writer->aheadBuf->offset);
    p_writer->scratchBuf->offset = nAheadBufDestination + p_writer->aheadBuf->offset;
    p_writer->aheadBuf->offset = 0;
    p_writer->nFieldCount = 0;
}

void jpegw_emit_DQT(jpeg_writer_t *p_writer, const uint16_t *qtbl)
{
    int i;
    unsigned int qval;

    for (i = 0; i < 64; i++)
    {
        /* The table entries must be emitted in zigzag order. */
        qval = qtbl[jpeg_natural_order[i]];
        WRITE_BYTE_SCRATCH((uint8_t) (qval & 0xFF));
    }
}

void jpegw_emit_DHT(jpeg_writer_t *p_writer, const jpeg_huff_table_t *htbl, int index)
{
    uint16_t length = 0, i;

    for (i = 1; i <= 16; i++)
        length = length + (uint16_t)(htbl->bits[i]);

    WRITE_BYTE_SCRATCH((uint8_t)(index & 0xFF));

    for (i = 1; i <= 16; i++)
        WRITE_BYTE_SCRATCH((uint8_t)(htbl->bits[i]));

    for (i = 0; i < length; i++)
        WRITE_BYTE_SCRATCH((uint8_t)(htbl->values[i]));
}

void jpegw_emit_DRI(jpeg_writer_t *p_writer, uint16_t nRestartInterval)
{
    WRITE_SHORT_SCRATCH(0xFF00 | M_DRI);

    WRITE_SHORT_SCRATCH(4); /* fixed length */

    WRITE_SHORT_SCRATCH((int16_t) (nRestartInterval));
}

void jpegw_emit_SOF(jpeg_writer_t *p_writer, jpeg_marker_t code,
                    jpege_img_cfg_t *p_config,
                    jpege_img_data_t *p_src)
{
    uint8_t nNumComponents = 3;
    const uint8_t nPrecision = 8;
    uint8_t i;
    uint32_t output_width, output_height;

    //If Monochrome set number of components to 1
    if (p_src->p_fragments->num_of_planes ==1)
    {
      nNumComponents =1;
    }
    WRITE_SHORT_SCRATCH(0xFF00 | code);

    WRITE_SHORT_SCRATCH(3 * (uint16_t) nNumComponents + 2 + 5 + 1); /* length */

    WRITE_BYTE_SCRATCH(nPrecision); /* data precision */

    if (p_config->scale_cfg.enable)
    {
        output_width  = p_config->scale_cfg.output_width;
        output_height = p_config->scale_cfg.output_height;
    }
    else
    {
        output_width  = p_src->width;
        output_height = p_src->height;
    }
    if (p_config->rotation_degree_clk % 180)
    {
        WRITE_SHORT_SCRATCH((uint16_t)output_width);
        WRITE_SHORT_SCRATCH((uint16_t)output_height);
    }
    else
    {
        WRITE_SHORT_SCRATCH((uint16_t)output_height);
        WRITE_SHORT_SCRATCH((uint16_t)output_width);
    }

    WRITE_BYTE_SCRATCH(nNumComponents);

    for (i = 0; i < nNumComponents; i++)
    {
        // Write component ID
        WRITE_BYTE_SCRATCH(i + 1);

        // Luma
        if (i == 0)
        {
            // Write sampling factors
            switch (((uint8_t)p_src->color_format) / 2)
            {
            case JPEG_H2V2:
	    case JPEG_BS_H2V2:
                WRITE_BYTE_SCRATCH(0x22);
                break;
            case JPEG_H2V1:
	    case JPEG_BS_H2V1:
                if (p_config->rotation_degree_clk % 180)
                {
                    WRITE_BYTE_SCRATCH(0x12);
                }
                else
                {
                    WRITE_BYTE_SCRATCH(0x21);
                }
                break;
            case JPEG_H1V2:
	    case JPEG_BS_H1V2:
                if (p_config->rotation_degree_clk % 180)
                {
                    WRITE_BYTE_SCRATCH(0x21);
                }
                else
                {
                    WRITE_BYTE_SCRATCH(0x12);
                }
                break;
            case JPEG_H1V1:
	    case JPEG_BS_H1V1:
                WRITE_BYTE_SCRATCH(0x11);
                break;
            default:
                 WRITE_BYTE_SCRATCH(0x11);
                 break;
            }
            // Write quantization table selector
            WRITE_BYTE_SCRATCH(0);
        }
        // Chroma
        else
        {
            // Write sampling factors
            WRITE_BYTE_SCRATCH(0x11);

            // Write quantization table selector
            WRITE_BYTE_SCRATCH(1);
        }
    }
}

void jpegw_emit_SOS(jpeg_writer_t *p_writer)
{
    uint16_t nComponents = 3;

    //If Monochrome set number of components to 1
    if (p_writer->p_source->p_main->p_fragments->num_of_planes==1)
    {
      nComponents =1;
    }

    WRITE_SHORT_SCRATCH(0xFF00 | M_SOS);

    WRITE_SHORT_SCRATCH(2 * nComponents + 2 + 1 + 3); /* length */

    /* number of components in scan */
    WRITE_BYTE_SCRATCH( (uint8_t) nComponents);

    /* Y                 Comp ID   Table Indices  */
    /*                                   DC       AC  */
    WRITE_SHORT_SCRATCH( (1 << 8)   |  (0 << 4) | 0); /* (0 << 4) + 0 DC and AC table index */

    if (nComponents != 1){
    /* Cb */
      WRITE_SHORT_SCRATCH( (2 << 8)   |  (1 << 4) | 1); /* DC and AC table index */

    /* Cr */
      WRITE_SHORT_SCRATCH( (3 << 8)   |  (1 << 4) | 1); /* DC and AC table index */
    }
    /*                    Ss        Se  */
    WRITE_SHORT_SCRATCH( (0 << 8)  |  63);  /* Se */

    WRITE_BYTE_SCRATCH(0); /* (0 << 4) + 0) Ah and Al */
}

void jpegw_emit_app0(jpeg_writer_t *p_writer)
{
    /*
     * Length of APP0 block	(2 bytes)
     * Block ID			(4 bytes - ASCII "JFIF")
     * Zero byte			(1 byte to terminate the ID string)
     * Version Major, Minor	(2 bytes - major first)
     * Units			(1 byte - 0x00 = none, 0x01 = inch, 0x02 = cm)
     * Xdpu			(2 bytes - dots per unit horizontal)
     * Ydpu			(2 bytes - dots per unit vertical)
     * Thumbnail X size		(1 byte)
     * Thumbnail Y size		(1 byte)
     */
    WRITE_SHORT_SCRATCH(0xFF00 | M_APP0);

    // no thumbnail case
    if (!p_writer->thumbnailBuf->ptr)
    {
        WRITE_SHORT_SCRATCH(2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1); /* length */

        // Write "JFIF" app string
        WRITE_LONG_SCRATCH(0x4A464946);

        /* 0 | major version | minor version | pixel size (density unit) */
        WRITE_LONG_SCRATCH((0<<24) | (1<<16) | (1<<8) | 0);

        /* XDensity | YDensity */
        WRITE_LONG_SCRATCH((1<<16) | (1));

        /* No thumbnail | No thumbnail */
        WRITE_SHORT_SCRATCH(0);
    }
    else
    {
        // length will be determined later, by pass for now
        p_writer->nApp1LengthOffset = p_writer->scratchBuf->offset;
        p_writer->scratchBuf->offset += 2;

        // Write "JFXX" app string
        WRITE_LONG_SCRATCH(0x4A465858);
        WRITE_BYTE_SCRATCH(0);

        // Extension code for Thumbnail coded using JPEG
        WRITE_BYTE_SCRATCH(0x10);

        // Extension data
        // Save Thumbnail Stream Offset
        p_writer->nThumbnailStreamOffset = p_writer->scratchBuf->offset;

        // Emit SOI
        WRITE_SHORT_SCRATCH(0xFF00 | M_SOI);

        // Emit Frame Header
        jpegw_emit_frame_header(p_writer, &(p_writer->p_encode_cfg->thumbnail_cfg), p_writer->p_source->p_thumbnail);

        // Emit Scan Header
        jpegw_emit_scan_header(p_writer, &(p_writer->p_encode_cfg->thumbnail_cfg));
    }
}

void jpegw_emit_exif_tag(jpeg_writer_t *p_writer, exif_tag_entry_ex_t *p_entry)
{
    uint16_t tag_id;
    uint32_t to_write_len, i;
    uint32_t bytes_written = 0;

    if (!p_entry)
        return;

    tag_id = p_entry->tag_id & 0xFFFF;

    // Write Tag ID
    WRITE_SHORT_SCRATCH(tag_id);

    // Write Tag Type
    WRITE_SHORT_SCRATCH((uint16_t)p_entry->entry.type);

    // Write Tag count
    WRITE_LONG_SCRATCH(p_entry->entry.count);

    // Compute the length that needs to be written
    to_write_len = tag_type_sizes[p_entry->entry.type] * p_entry->entry.count;

    /*
     * If to_write_len <= 4, tag value written along-side taglength and
     * other parameters in the Scratch Buffer. Else, a pointer to the tag value
     * is written along-side taglength etc in the Scratch Buffer, and the actual
     * value written in the ahead buffer. Refer to EXIF std for details.
     */
    if (to_write_len <= 4)
    {
        if (p_entry->entry.type == EXIF_ASCII ||
            p_entry->entry.type == EXIF_UNDEFINED)
        {
            WRITE_NBYTES_SCRATCH(p_entry->entry.data._ascii, to_write_len);
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
                        WRITE_BYTE_SCRATCH(p_entry->entry.data._bytes[i]);
                        bytes_written ++;
                        break;
                    case EXIF_SHORT:
                        WRITE_SHORT_SCRATCH(p_entry->entry.data._shorts[i]);
                        bytes_written += 2;
                        break;
                    default:
                        JPEG_DBG_ERROR("jpegw_emit_exif_tag: impossible case! p_entry->entry.type = %d\n",
                                       p_entry->entry.type);
                        return;
                    }
                }
            }
            else
            {
                switch (p_entry->entry.type)
                {
                case EXIF_BYTE:
                    WRITE_BYTE_SCRATCH((uint32_t)p_entry->entry.data._byte);
                    bytes_written = 1;
                    break;
                case EXIF_SHORT:
                    WRITE_SHORT_SCRATCH((uint32_t)p_entry->entry.data._short);
                    bytes_written = 2;
                    break;
                case EXIF_LONG:
                    WRITE_LONG_SCRATCH((uint32_t)p_entry->entry.data._long);
                    bytes_written = 4;
                    break;
                case EXIF_SLONG:
                    WRITE_LONG_SCRATCH((uint32_t)p_entry->entry.data._slong);
                    bytes_written = 4;
                    break;
                default:
                    JPEG_DBG_ERROR("jpegw_emit_exif_tag: impossible case! p_entry->entry.type = %d\n",
                                   p_entry->entry.type);
                    return;
                }
            }
        }
        // Fill up 0's till there are totally 4 bytes written
        for (i = bytes_written; i < 4; i++)
            WRITE_BYTE_SCRATCH(0);
    }
    else // if (to_write_len <= 4)
    {
        if (p_writer->aheadBuf->offset & 1) p_writer->aheadBuf->offset++;

        // Write the temporary offset (to be updated later)
        WRITE_LONG_SCRATCH(p_writer->aheadBuf->offset);

        if (p_entry->entry.type == EXIF_ASCII || p_entry->entry.type == EXIF_UNDEFINED)
        {
            WRITE_NBYTES_AHEAD(p_entry->entry.data._ascii, p_entry->entry.count);
        }
        else if (p_entry->entry.count > 1)
        {
            // Multiple data to write
            for (i = 0; i < p_entry->entry.count; i++)
            {
                switch (p_entry->entry.type)
                {
                case EXIF_BYTE:
                    WRITE_BYTE_AHEAD(p_entry->entry.data._bytes[i]);
                    break;
                case EXIF_SHORT:
                    WRITE_SHORT_AHEAD(p_entry->entry.data._shorts[i]);
                    break;
                case EXIF_LONG:
                    WRITE_LONG_AHEAD(p_entry->entry.data._longs[i]);
                    break;
                case EXIF_SLONG:
                    WRITE_LONG_AHEAD(p_entry->entry.data._slongs[i]);
                    break;
                case EXIF_RATIONAL:
                    WRITE_LONG_AHEAD(p_entry->entry.data._rats[i].num);
                    WRITE_LONG_AHEAD(p_entry->entry.data._rats[i].denom);
                    break;
                case EXIF_SRATIONAL:
                    WRITE_LONG_AHEAD(p_entry->entry.data._srats[i].num);
                    WRITE_LONG_AHEAD(p_entry->entry.data._srats[i].denom);
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
                WRITE_BYTE_AHEAD(p_entry->entry.data._byte);
                break;
            case EXIF_SHORT:
                WRITE_SHORT_AHEAD(p_entry->entry.data._short);
                break;
            case EXIF_LONG:
                WRITE_LONG_AHEAD(p_entry->entry.data._long);
                break;
            case EXIF_SLONG:
                WRITE_LONG_AHEAD(p_entry->entry.data._slong);
                break;
            case EXIF_RATIONAL:
                WRITE_LONG_AHEAD(p_entry->entry.data._rat.num);
                WRITE_LONG_AHEAD(p_entry->entry.data._rat.denom);
                break;
            case EXIF_SRATIONAL:
                WRITE_LONG_AHEAD(p_entry->entry.data._srat.num);
                WRITE_LONG_AHEAD(p_entry->entry.data._srat.denom);
                break;
            default:
                break;
            }
        }
    }
    // update p_writer->nFieldCount
    p_writer->nFieldCount++;
}

void jpegw_emit_0th_ifd(jpeg_writer_t *p_writer, uint32_t *nExifIfdPointerOffset, uint32_t *nGpsIfdPointerOffset)
{
    int i;
    exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)(p_writer->p_exif_info);
    exif_tag_entry_ex_t dummy_entry;

    jpegw_start_ifd(p_writer);

    for (i = (int)NEW_SUBFILE_TYPE; i <= (int)GPS_IFD; i++)
    {
        jpegw_emit_exif_tag(p_writer, pp_entries[i]);
    }

    // Save location for later update and emit Exif IFD pointer
    *nExifIfdPointerOffset = p_writer->scratchBuf->offset;

    // Craft Exif IFD pointer tag
    dummy_entry.entry.count = 1;
    dummy_entry.entry.type = (exif_tag_type_t)EXIFTAGTYPE_EXIF_IFD_PTR;
    dummy_entry.entry.data._long = 0;
    dummy_entry.tag_id = EXIFTAGID_EXIF_IFD_PTR;

    jpegw_emit_exif_tag(p_writer, &dummy_entry);

    // Save location for later use and emit GPD Ifd pointer
    *nGpsIfdPointerOffset = p_writer->scratchBuf->offset;

    dummy_entry.tag_id = EXIFTAGID_GPS_IFD_PTR;
    jpegw_emit_exif_tag(p_writer, &dummy_entry);

    // Save location for thumbnail pointer IFD
    p_writer->nThumbnailIfdPointerOffset = p_writer->scratchBuf->offset;

    jpegw_finish_ifd(p_writer);
}

void jpegw_emit_exif_ifd(jpeg_writer_t *p_writer, uint32_t *nInteropIfdPointerOffset)
{
    int i;
    exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)(p_writer->p_exif_info);
    exif_tag_entry_ex_t dummy_entry;
    uint32_t output_width, output_height;

    jpegw_start_ifd(p_writer);

    // Emit EXIF IFD
    for (i = (int)EXPOSURE_TIME; i <= (int)EXIF_COLOR_SPACE; i++)
    {
        jpegw_emit_exif_tag(p_writer, pp_entries[i]);
    }

    if (p_writer->p_encode_cfg->main_cfg.scale_cfg.enable)
    {
        output_width  = p_writer->p_encode_cfg->main_cfg.scale_cfg.output_width;
        output_height = p_writer->p_encode_cfg->main_cfg.scale_cfg.output_height;
    }
    else
    {
        output_width  = p_writer->p_source->p_main->width;
        output_height = p_writer->p_source->p_main->height;
    }

    // Emit Pixel X and Y Dimension
    dummy_entry.entry.count = 1;
    dummy_entry.entry.type = (exif_tag_type_t)EXIF_LONG;
    if (p_writer->p_encode_cfg->main_cfg.rotation_degree_clk % 180)
    {
        dummy_entry.entry.data._long = output_height;
    }
    else
    {
        dummy_entry.entry.data._long = output_width;
    }
    dummy_entry.tag_id = EXIFTAGID_EXIF_PIXEL_X_DIMENSION;
    jpegw_emit_exif_tag(p_writer, &dummy_entry);

    if (p_writer->p_encode_cfg->main_cfg.rotation_degree_clk % 180)
    {
        dummy_entry.entry.data._long = output_width;
    }
    else
    {
        dummy_entry.entry.data._long = output_height;
    }
    dummy_entry.tag_id = EXIFTAGID_EXIF_PIXEL_Y_DIMENSION;
    jpegw_emit_exif_tag(p_writer, &dummy_entry);

    // Save offset and Emit Interoperability Ifd Pointer
    *nInteropIfdPointerOffset = p_writer->scratchBuf->offset;
    dummy_entry.entry.data._long = 0;
    dummy_entry.tag_id = EXIFTAGID_INTEROP_IFD_PTR;

    jpegw_emit_exif_tag(p_writer, &dummy_entry);

    // Continue with the rest of the IFD
    for (i = (int)RELATED_SOUND_FILE; i <= (int)PIM; i++)
    {
        jpegw_emit_exif_tag(p_writer, pp_entries[i]);
    }

    jpegw_finish_ifd(p_writer);
}

void jpegw_emit_interop_ifd(jpeg_writer_t *p_writer)
{
    jpegw_start_ifd(p_writer);

    // Emit Interoperability Index
    jpegw_emit_exif_tag(p_writer, &default_tag_interopindexstr);

    // Emit Exif R98 Version
    jpegw_emit_exif_tag(p_writer, &default_tag_r98_version);

    jpegw_finish_ifd(p_writer);
}

void jpegw_emit_gps_ifd(jpeg_writer_t *p_writer)
{
    int i;
    exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)(p_writer->p_exif_info);
    jpegw_start_ifd(p_writer);

    // Emit tags in GPS IFD
    for (i = (int)GPS_VERSION_ID; i <= (int)GPS_DIFFERENTIAL; i++)
    {
        jpegw_emit_exif_tag(p_writer, pp_entries[i]);
    }

    jpegw_finish_ifd(p_writer);
}

void jpegw_emit_thumbnail_ifd(jpeg_writer_t *p_writer)
{
    exif_tag_entry_ex_t dummy_entry;
    int i, nJpegInterchangeOffset = 0;
    exif_tag_entry_ex_t **pp_entries = (exif_tag_entry_ex_t **)(p_writer->p_exif_info);

    // Update Thumbnail_IFD pointer
    jpegw_overwrite_long(p_writer->scratchBuf->offset - p_writer->nTiffHeaderOffset,
                         p_writer->scratchBuf->ptr, p_writer->nThumbnailIfdPointerOffset,
                         p_writer->scratchBuf->size, &(p_writer->overflow_flag));

    jpegw_start_ifd(p_writer);

    // Emit Pixel X and Y Dimension
    dummy_entry.entry.count = 1;
    dummy_entry.entry.type = (exif_tag_type_t)EXIF_LONG;
    dummy_entry.entry.data._long = 0;

    // Emit tags in TIFF IFD for thumbnail
    for (i = (int)TN_IMAGE_WIDTH; i <= (int)TN_COPYRIGHT; i++)
    {
        // Save offset to 'Offset to JPEG SOI'
        if (i == (int)TN_JPEGINTERCHANGE_FORMAT)
        {
            nJpegInterchangeOffset = p_writer->scratchBuf->offset + 8;
            dummy_entry.tag_id = EXIFTAGID_TN_JPEGINTERCHANGE_FORMAT;
            jpegw_emit_exif_tag(p_writer, &dummy_entry);
        }
        else if (i == (int)TN_JPEGINTERCHANGE_FORMAT_L)
        {
            p_writer->nJpegInterchangeLOffset = p_writer->scratchBuf->offset + 8;
            dummy_entry.tag_id = EXIFTAGID_TN_JPEGINTERCHANGE_FORMAT_L;
            jpegw_emit_exif_tag(p_writer, &dummy_entry);
        }
        else
        {
            jpegw_emit_exif_tag(p_writer, pp_entries[i]);
        }
    }

    jpegw_finish_ifd(p_writer);

    // Save Thumbnail Stream Offset
    p_writer->nThumbnailStreamOffset = p_writer->scratchBuf->offset;

    // Emit SOI
    WRITE_SHORT_SCRATCH(0xFF00 | M_SOI);

    // Emit Frame Header
    jpegw_emit_frame_header(p_writer, &(p_writer->p_encode_cfg->thumbnail_cfg), p_writer->p_source->p_thumbnail);

    // Emit Scan Header
    jpegw_emit_scan_header(p_writer, &(p_writer->p_encode_cfg->thumbnail_cfg));

    // Update Jpeg Interchange Format
    jpegw_overwrite_long(p_writer->nThumbnailStreamOffset - p_writer->nTiffHeaderOffset,
                         p_writer->scratchBuf->ptr,
                         nJpegInterchangeOffset,
                         p_writer->scratchBuf->size,
                         &(p_writer->overflow_flag));
}

void jpegw_emit_app1(jpeg_writer_t *p_writer)
{
    uint32_t nExifIfdPointerOffset;
    uint32_t nGpsIfdPointerOffset;
    uint32_t nInteropIfdPointerOffset;

    WRITE_SHORT_SCRATCH(0xFF00 | M_APP1);

    // length will be calculated after all operations. bypass for now
    p_writer->nApp1LengthOffset = p_writer->scratchBuf->offset;
    p_writer->scratchBuf->offset += 2;

    // Write "Exif" app string
    WRITE_LONG_SCRATCH(0x45786966);
    WRITE_SHORT_SCRATCH(0);

    // TIFF header
    p_writer->nTiffHeaderOffset = p_writer->scratchBuf->offset;
    WRITE_SHORT_SCRATCH(0x4D4D);
    WRITE_SHORT_SCRATCH(0x002A);
    WRITE_LONG_SCRATCH(0x00000008);

    // Write 0th Ifd
    jpegw_emit_0th_ifd(p_writer, &nExifIfdPointerOffset, &nGpsIfdPointerOffset);

    // Make sure the starting offset is at 2-byte boundary.
    if (p_writer->scratchBuf->offset & 1)
        WRITE_BYTE_SCRATCH(0);

    // Go back and update EXIF_IFD pointer
    nExifIfdPointerOffset += 8;
    jpegw_emit_long(p_writer->scratchBuf->offset - p_writer->nTiffHeaderOffset, p_writer->scratchBuf->ptr,
                    &nExifIfdPointerOffset, p_writer->scratchBuf->size, &(p_writer->overflow_flag));

    // Write Exif Ifd
    jpegw_emit_exif_ifd(p_writer, &nInteropIfdPointerOffset);

    // Go back and update INTEROP_IFD pointer
    nInteropIfdPointerOffset += 8;
    jpegw_emit_long(p_writer->scratchBuf->offset - p_writer->nTiffHeaderOffset,
                    p_writer->scratchBuf->ptr,
                    &nInteropIfdPointerOffset,
                    p_writer->scratchBuf->size,
                    &(p_writer->overflow_flag));

    // Write Interoperability Ifd
    jpegw_emit_interop_ifd(p_writer);

    // Make sure the starting offset is at 2-byte boundary.
    if (p_writer->scratchBuf->offset & 1)
        WRITE_BYTE_SCRATCH(0);

        // Go back and update GPS_IFD pointer
        nGpsIfdPointerOffset += 8;
        jpegw_emit_long(p_writer->scratchBuf->offset - p_writer->nTiffHeaderOffset,
                        p_writer->scratchBuf->ptr,
                        &nGpsIfdPointerOffset,
                        p_writer->scratchBuf->size,
                        &(p_writer->overflow_flag));

        // Write GPS Ifd
        jpegw_emit_gps_ifd(p_writer);

    if ((p_writer->thumbnailBuf) &&
        (p_writer->thumbnailBuf->size))
    {
        // Make sure the starting offset is at 2-byte boundary.
        if (p_writer->scratchBuf->offset & 1)
            WRITE_BYTE_SCRATCH(0);

        // Save the thumbnail starting offset
        p_writer->nThumbnailOffset = p_writer->scratchBuf->offset;

        jpegw_emit_thumbnail_ifd(p_writer);
    }

    // Update App1 length field
    jpegw_overwrite_short ((uint16_t)(p_writer->scratchBuf->offset - p_writer->nApp1LengthOffset),
                           p_writer->scratchBuf->ptr, p_writer->nApp1LengthOffset, p_writer->scratchBuf->size,
                           &(p_writer->overflow_flag));

    p_writer->fApp1Present = true;
}

int jpegw_emit_header_exif(jpeg_writer_t *p_writer)
{
    int rc;

    // Allocate internal exif info object if none is supplied
    if (!p_writer->p_exif_info)
    {
        exif_info_obj_t exif_info_obj;
        rc = exif_init(&exif_info_obj);
        if (JPEG_SUCCEEDED(rc))
        {
            p_writer->p_exif_info = (exif_info_t *)exif_info_obj;
            p_writer->is_exif_info_owned = true;
        }
        else
        {
            // Better to return it immediately, else, later point exif_info_obj
            // or equivalently  p_writer->p_exif_info may get de-refrenced.
            JPEG_DBG_ERROR("jpegw_emit_header_exif : exif_init failed\n");
            return rc;
        }
    }

    WRITE_SHORT_SCRATCH(0xFF00 | M_SOI);

    jpegw_emit_app1(p_writer);
    rc = jpegw_flush_file_header(p_writer);
    if (rc != JPEGERR_THUMBNAIL_DROPPED)
        CHK(rc);

    p_writer->fHeaderWritten = true;
    return rc;
}

int jpegw_emit_header_jfif(jpeg_writer_t *p_writer)
{
    int rc;

    WRITE_SHORT_SCRATCH(0xFF00 | M_SOI);

    jpegw_emit_app0(p_writer);
    rc = jpegw_flush_file_header(p_writer);
    CHK(rc);

    p_writer->fHeaderWritten = true;
    return JPEGERR_SUCCESS;
}

int jpegw_emit_header(jpeg_writer_t *p_writer, jpeg_buf_t *p_thumbnail_buf)
{
    if ((p_writer->p_source->p_thumbnail) &&
        (!p_thumbnail_buf))
    {
        JPEG_DBG_ERROR("jpegw_emit_header: failed with null thumb buffer\n");
        return JPEGERR_EBADSTATE;
    }
    p_writer->thumbnailBuf = p_thumbnail_buf;

    if (p_writer->p_encode_cfg->header_type == OUTPUT_JFIF)
    {
        return jpegw_emit_header_jfif(p_writer);
    }
    else
    {
        return jpegw_emit_header_exif(p_writer);
    }
}

void jpegw_emit_frame_header(jpeg_writer_t *p_writer,
                             jpege_img_cfg_t *p_config,
                             jpege_img_data_t *p_src)
{
    /* Emit DQT for each quantization table.
     * Note that emit_dqt() suppresses any duplicate tables.
     */
    WRITE_SHORT_SCRATCH(0xFF00 | M_DQT);

    WRITE_SHORT_SCRATCH(64*2 + 2 + 2);

    WRITE_BYTE_SCRATCH(0);
    jpegw_emit_DQT(p_writer, p_config->luma_quant_tbl);

   WRITE_BYTE_SCRATCH(1);
   jpegw_emit_DQT(p_writer, p_config->chroma_quant_tbl);

    jpegw_emit_SOF(p_writer, M_SOF0, p_config, p_src);  // SOF code for baseline implementation
}

void jpegw_emit_scan_header(jpeg_writer_t *p_writer, const jpege_img_cfg_t *p_config)
{
    int i;
    uint16_t length;

    // Emit Huffman tables.
    WRITE_SHORT_SCRATCH(0xFF00 | M_DHT);

    // Compute length
    length = 0;

    for (i = 1; i <= 16; i++)
    {
        length = length + (uint16_t) (p_config->luma_dc_huff_tbl.bits[i]);
        length = length + (uint16_t) (p_config->luma_ac_huff_tbl.bits[i]);
        length = length + (uint16_t) (p_config->chroma_dc_huff_tbl.bits[i]);
        length = length + (uint16_t) (p_config->chroma_ac_huff_tbl.bits[i]);
    }

    //lint -e734 length + 17 * 4 + 2 will never overflow 16 bits
    WRITE_SHORT_SCRATCH(2 + length + 17*4); // 4 Huffman tables

    jpegw_emit_DHT(p_writer, &(p_config->luma_dc_huff_tbl), 0);        // luma DC table
    jpegw_emit_DHT(p_writer, &(p_config->luma_ac_huff_tbl), 0 | 0x10); // luma AC table
    jpegw_emit_DHT(p_writer, &(p_config->chroma_dc_huff_tbl), 1);        // chroma DC table
    jpegw_emit_DHT(p_writer, &(p_config->chroma_ac_huff_tbl), 1 | 0x10); // chroma AC table

    /*
     * Emit DRI
     */
    if (p_config->restart_interval > 0)
    {
        jpegw_emit_DRI(p_writer, (uint16_t) p_config->restart_interval);
    }

    jpegw_emit_SOS(p_writer);
}
