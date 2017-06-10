/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix.h"
#include "sensor_dbg.h"

static chromatix_parms_type chromatix_s5k4e1_13p1ba_parms = {
#include "chromatix_s5k4e1_13p1ba_liveshot.h"
};

/*============================================================================
 * FUNCTION    - load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *load_chromatix(void)
{
  ALOGE("chromatix ptr %p", &chromatix_s5k4e1_13p1ba_parms);
  return &chromatix_s5k4e1_13p1ba_parms;
}
