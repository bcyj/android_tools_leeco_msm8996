/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "cac_module.h"

/** cac_module_init:
 *  Args:
 *    @name: module name
 *  Return:
 *    - mct_module_t pointer corresponding to cac on SUCCESS
 *    - NULL in case of FAILURE or if CAC hardware does not
 *      exist
 **/
mct_module_t *cac_module_init(const char *name)
{
  return NULL;
}

/** cac_module_deinit:
 *
 *  Args:
 *    @module: pointer to cac mct module
 *  Return:
 *    void
 **/
void cac_module_deinit(mct_module_t *module)
{
}
