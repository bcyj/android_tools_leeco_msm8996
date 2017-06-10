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
#ifndef _MPO_HEADER_H
#define _MPO_HEADER_H

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_header.h"
#include "mpo_private.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

typedef struct
{
    // Pointer to the JPEG header structure.
    // The actual JPEG header structure is declared as a member in jpeg_reader_t.
    // Here only a handle (pointer to jpeg_header structure) is defined
    // to avoid duplicate jpeg_header structure.
    jpeg_header_t        *p_jpeg_header;
    // MPO index IFD
    mpo_index_ifd_t      *p_index_ifd;
    // MPO attribute IFD
    mpo_attribute_ifd_t  *p_attribute_ifd;

} mpo_header_t;

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
/* =======================================================================
**                          Function Declarations
** ======================================================================= */
void mpo_header_destroy(mpo_header_t *p_mpo_header);


#endif /* _MPO_HEADER_H */
