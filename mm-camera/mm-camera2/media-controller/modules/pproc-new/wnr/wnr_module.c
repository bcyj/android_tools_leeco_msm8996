/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "wnr_module.h"

/** wnr_module_init:
 *  Args:
 *    @name: module name
 *  Return:
 *    - mct_module_t pointer corresponding to wnr on SUCCESS
 *    - NULL in case of FAILURE or if WNR hardware does not
 *      exist
 **/
mct_module_t *wnr_module_init(const char *name)
{
  return NULL;
}

/** wnr_module_deinit:
 *
 *  Args:
 *    @module: pointer to wnr mct module
 *  Return:
 *    void
 **/
void wnr_module_deinit(mct_module_t *module)
{
}
