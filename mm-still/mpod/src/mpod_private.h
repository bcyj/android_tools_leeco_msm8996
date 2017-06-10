/**
 *Copyright (C) 2011 Qualcomm Technologies, Inc.
 *All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
**/
/*========================================================================
                             Edit History


when       who     what, where, why
--------   ---     -------------------------------------------------------
03/10/11   mingy   Created file.

*//*====================================================================== */

#ifndef __MPOD_PRIV_H__
#define __MPOD_PRIV_H__

#include "jpegd_private.h"
#include "mpod.h"
#include "mpo_reader.h"
#include "mpo_header.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

typedef struct mpo_decoder_t
{
    // Pointer to the jpeg decoder
    jpeg_decoder_t  *p_jpeg_decoder;
    // The MPO reader
    mpo_reader_t     mpo_reader;
    // Full MPO header
    mpo_header_t    *p_mpo_full_hdr;

} mpo_decoder_t;

#endif // #ifndef __MPOD_PRIV_H__
