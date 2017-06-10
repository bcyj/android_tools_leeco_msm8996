/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_common.h"
#include "camera_dbg.h"

static chromatix_VFE_common_type chromatix_ov9724_parms = {
#include "chromatix_ov9724_common.h"
};

/*============================================================================
 * FUNCTION    - load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *load_chromatix(void)
{
  CDBG("%s:%d%d\n", __func__, __LINE__);
  return &chromatix_ov9724_parms;
}
