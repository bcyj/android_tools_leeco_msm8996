/**
 *Copyright (C) 2011 Qualcomm Technologies, Inc.
 *All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
**/
/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
03/10/11   mingy   Created file.

========================================================================== */
#ifndef _MPO_READER_H
#define _MPO_READER_H

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "mpo_header.h"
#include "jpeg_reader.h"
#include "writer_utility.h"
/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

typedef struct
{
    // Pointer to the JPEG reader structure.
    // The actual JPEG reader structure is declared as a member in jpeg_decoder_t.
    // Here only a handle (pointer to jpeg_reader structure) is defined
    // to avoid duplicate jpeg_reader structure.
    jpeg_reader_t      *p_jpeg_reader;
    // MPO header
    mpo_header_t        mpo_header;
    // The MP endian
    exif_endianness_t   mp_endian;
    // Individual image Start of Offset
    uint32_t            start_offset;
    // Offset of Next IFD
    uint32_t            next_ifd_offset;

} mpo_reader_t;


/* =======================================================================
**                          Function Declarations
** ======================================================================= */
// Function to read and parse the header from the first individual image
mpo_header_t *mpor_read_first_header(mpo_reader_t *p_mpo_reader);
// Function to read and parse the header from individual images
// other than the first individual image
mpo_header_t *mpor_read_header(mpo_reader_t *p_mpo_reader);

#endif /* _MPO_READER_H */
