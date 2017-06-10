/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

#include "sensor.h"
#include "chromatix.h"
#include "camera_dbg.h"

static chromatix_parms_type chromatix_mt9e013_parms = {
#include "chromatix_mt9e013_video_hfr.h"
};

/*============================================================================
 * FUNCTION    - load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void load_chromatix(void *ctrl)
{
  sensor_ctrl_t *sctrl = (sensor_ctrl_t *) ctrl;

  CDBG("%s:%s:%d\n", __FILE__, __func__, __LINE__);
  memcpy(&sctrl->chromatixData, &chromatix_mt9e013_parms,
    sizeof(chromatix_parms_type));
  CDBG("%s:%d: chromatix_version=%d\n", __func__, __LINE__,
    sctrl->chromatixData.chromatix_version);
}
