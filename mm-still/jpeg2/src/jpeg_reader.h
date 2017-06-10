#ifndef _JPEG_READER_H
#define _JPEG_READER_H
/*========================================================================

                 C o m m o n   D e f i n i t i o n s

*//** @file jpeg_reader.h

Copyright (C) 2008-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpegd.h"
#include "jpeg_header.h"
#include "jpeg_writer.h"
#include "exif_private.h"
#include "jpegd_engine_sw.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef int(*jpegr_input_req_handler_t)(jpegd_obj_t   obj,
                                        jpeg_buf_t   *p_buffer,
                                        uint32_t      start_offset,
                                        uint32_t      length);

typedef struct
{
    // The Jpeg Decoder object
    jpegd_obj_t               decoder;

    // The input request handler
    jpegr_input_req_handler_t p_input_req_handler;
    // The source object
    jpegd_src_t              *p_source;

    // The overal offset in the input stream of the byte to be fetched next
    uint32_t                  next_byte_offset;

    // The input data buffer
    jpeg_buf_t               *p_input_buf;

    // The start offset in the overall input bitstream
    // of the first byte of data in the input buffer
    uint32_t                  input_start_offset;

    // The endianess
    exif_endianness_t         endianness;

    // Error occurred
    uint8_t                   error_flag;

    // Jpeg header
    jpeg_header_t             header;

    uint32_t                  tiff_hdr_offset;
    uint32_t                  exif_ifd_offset;
    uint32_t                  gps_ifd_offset;
    uint32_t                  first_ifd_offset;
    uint32_t                  interop_ifd_offset;

} jpeg_reader_t;

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
/* =======================================================================
**                          Function Declarations
** ======================================================================= */

int  jpegr_init                  (jpeg_reader_t             *p_reader,
                                  jpegd_obj_t                decoder,
                                  jpegd_src_t               *p_source,
                                  jpegr_input_req_handler_t  p_handler);
jpeg_header_t *jpegr_read_header (jpeg_reader_t             *p_reader);
void jpegr_destroy               (jpeg_reader_t             *p_reader);

#endif /* _JPEG_READER_H */
