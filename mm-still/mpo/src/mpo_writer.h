#ifndef _MPO_WRITER_H
#define _MPO_WRITER_H
/*========================================================================

                 C o m m o n   D e f i n i t i o n s

*//** @file mpo_writer.h

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
#include "mpoe.h"
#include "mpo_private.h"
#include "jpeg_buffer_private.h"


/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
// Maximum APP2 length
#define MPO_MAX_APP2_LENGTH     0xFFF

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef int (*mpow_output_handler_t)(mpoe_obj_t encoder, jpeg_buf_t *p_buf);

typedef struct
{
    // Output handler
    mpow_output_handler_t  p_handler;

    // Header buffer for each individual images
    jpeg_buf_t *header_buf[MAX_IMAGES];

    // Value buffer for each individual images
    jpeg_buf_t *value_buf[MAX_IMAGES];

    // Scratch buffer: name shared by both mpo and jpeg writer
    jpeg_buf_t *scratchBuf;

    // Write ahead buffer: name shared by both mpo and jpeg writer
    jpeg_buf_t *aheadBuf;

    // The App2 header length
    uint32_t   app2_header_length;

    // The App2 header start offset:
    // pass from jpeg encoder, and update it inside mpo encoder writer
    uint32_t   app2_start_offset[MAX_IMAGES];

    // The App2 header MP entry value offset
    uint32_t   app2_mp_entry_value_offset;

    // Total Image size
    uint32_t   total_images_size;

    // For piece-wise output, each individual image's last piece could
    // be partial. Next individual image first piece could start from
    // some middle point of output piece buffer
    uint32_t   last_image_offset[MAX_IMAGES];

    // Individual Image size
    uint32_t   image_size[MAX_IMAGES];

    // Save the App2 image size
    uint32_t   app2_image_size_offset;

    // Save the App2 data offset
    uint32_t   app2_data_offset_offset;

    // Start of Offset of MP: Base address from which all of
    // the offset addresses in the MP Extensions are calculated
    // in the address of the MP Endian field in MP header
    uint32_t   app2_start_of_offset;

    // Flag indicating whether this source is large thumbnail
    uint8_t    large_thumbnail_flag[MAX_IMAGES];

    // The image source object
    mpoe_src_t  source[MAX_IMAGES];

    // The source count
    uint32_t    source_cnt;

    // The destination object
    mpoe_dst_t *p_dest;

    // The encode configuration
    mpo_info_t *p_mpo_info;

    // The Jpeg Encoder object
    mpoe_obj_t  encoder;

    // Flag indicating whether any overflow has occurred
    uint8_t    overflow_flag;

    // Keep count of how many tags are written
    uint32_t   field_count;

    // Save the location to write the IFD count
    uint32_t   field_count_offset;

} mpo_writer_t;

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Declarations
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
int mpow_init(
    mpo_writer_t        *p_writer);

/******************************************************************************
 * Function: mpow_configure_first
 * Description: Configure for the first individual image.
 * Input parameters:
 *   p_writer  - The Mpo Writer object.
 *   p_source  - The pointer to the sources.
 *   p_dest    - The pointer to destination.
 *   p_mpo_info - The pointer to mpo info.
 *   source_cnt - The source count.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpow_configure_first(
    mpo_writer_t        *p_writer,
    mpoe_src_t          *p_source,
    mpoe_dst_t          *p_dest,
    mpo_info_obj_t      *p_mpo_info,
    uint32_t             source_cnt);

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
int mpow_configure(
    mpo_writer_t        *p_writer,
    uint32_t             src_id);

/******************************************************************************
 * Function: mpow_set_app2_info
 * Description: Sets App2 info into the Mpo Writer object.
 * Input parameters:
 *   p_writer      - The pointer to the Mpo Writer object
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
    mpo_writer_t        *p_writer,
    uint32_t             buf_offset,
    uint32_t             app2_start_offset,
    uint32_t             src_id);

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
    mpo_writer_t        *p_writer,
    uint32_t             buf_offset);

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
    uint32_t             src_id);

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
    uint32_t             src_id);

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
int mpow_destroy (
    mpo_writer_t        *p_writer);

#endif /* _MPO_WRITER_H */
