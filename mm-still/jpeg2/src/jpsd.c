/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpsd.h"
#include "jpegerr.h"
#include "jpegd_private.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
/* =======================================================================
**                          Function Definitions
** ======================================================================= */

/******************************************************************************
 * Function: jpsd_get_config
 * Description: Obtain the jps config information.
 * Input parameters:
 *   obj            - The Jpeg Decoder object.
 *   p_jps_config   - The pointer to the jps config structure.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpsd_get_config(
    jpegd_obj_t    obj,
    jps_cfg_3d_t  *p_jps_config)
{
    jpeg_decoder_t *p_decoder = (jpeg_decoder_t *)obj;
    if (!p_decoder)
        return JPEGERR_EUNINITIALIZED;

    // Validate input arguments
    if (!p_jps_config)
        return JPEGERR_EBADPARM;

    // Reject request if decoder is actively decoding or is cleaning up
    if (p_decoder->state != JPEGD_IDLE)
        return JPEGERR_EBADSTATE;

    // Parse the Jpeg header if not done yet
    if (!p_decoder->p_full_hdr)
    {
        p_decoder->p_full_hdr = jpegr_read_header(&(p_decoder->reader));
    }
    
    // If full header is present, derive short header from it
    if (p_decoder->p_full_hdr)
    {
        *p_jps_config = p_decoder->p_full_hdr->jps_info;
    }
    else
    {
        return JPEGERR_EFAILED;
    }

    return JPEGERR_SUCCESS;
}

