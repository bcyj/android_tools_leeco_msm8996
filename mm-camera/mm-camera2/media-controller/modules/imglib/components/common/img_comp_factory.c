/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#include "img_comp_registry.h"

/**
 * Function: img_core_get_comp
 *
 * Description: Get the component of particular role from component
 *              factory
 *
 * Input parameters:
 *   role - role of the component
 *   name - name of the component
 *   p_ops - pointer to the core ops
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_NOT_FOUND
 *
 * Notes: none
 **/
int img_core_get_comp(img_comp_role_t role, char *name,
  img_core_ops_t *p_ops)
{
  uint32_t i = 0;

  if (name == NULL) {
    IDBG_ERROR("%s:%d] null parameter", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_HIGH("%s:%d] role %d name %s", __func__, __LINE__,
    role, name);
  for (i = 0; i < IMG_COMP_REG_SIZE; i++) {
    IDBG_HIGH("%s:%d] comp_index[%d] role %d name %s",
      __func__, __LINE__,
      i, g_img_registry[i].role,
      g_img_registry[i].name);
    if (!strncmp(g_img_registry[i].name, name,
      strlen(g_img_registry[i].name)+1) &&
      (role == g_img_registry[i].role)) {
      IDBG_HIGH("%s:%d] find component role %d name %s",
        __func__, __LINE__, role, name);
      *p_ops = g_img_registry[i].ops;
      return IMG_SUCCESS;
    }
  }
  return IMG_ERR_NOT_FOUND;
}
