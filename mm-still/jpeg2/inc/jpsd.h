/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef _JPSD_H
#define _JPSD_H

#include "jps.h"
#include "jpegd.h"

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
    jps_cfg_3d_t  *p_jps_config);

#endif // #ifndef _JPSD_H

