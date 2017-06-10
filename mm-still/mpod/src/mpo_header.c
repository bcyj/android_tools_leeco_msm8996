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

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_header.h"
#include "mpo_header.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Declarations
** ======================================================================= */

void mpo_header_destroy(mpo_header_t *p_mpo_header)
{
    if (p_mpo_header)
    {
        // Cast index and attribute into object type to be fed into the destroy functions
        mpo_index_obj_t     mpo_index_obj     = (mpo_index_obj_t)p_mpo_header->p_index_ifd;
        mpo_attribute_obj_t mpo_attribute_obj = (mpo_attribute_obj_t)p_mpo_header->p_attribute_ifd;

        // Destroy index and attribute objects
        mpo_index_destroy(&mpo_index_obj);
        mpo_attribute_destroy(&mpo_attribute_obj);

        STD_MEMSET(p_mpo_header, 0, sizeof(mpo_header_t));
    }
}
