/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_common.h"
#include "sensor_dbg.h"

static chromatix_VFE_common_type chromatix_skuf_ov12830_p12v01c_parms = {
#include "chromatix_skuf_ov12830_p12v01c_common.h"
};

/*============================================================================
 * FUNCTION    - load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *load_chromatix(void)
{
  SLOW("chromatix ptr %p", &chromatix_skuf_ov12830_p12v01c_parms);
  return &chromatix_skuf_ov12830_p12v01c_parms;
}
