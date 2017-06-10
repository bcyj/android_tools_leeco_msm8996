/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "csi.h"
#include "csid.h"
#include "csiphy.h"

#ifdef CSI_DEBUG
#undef CDBG
#define CDBG LOGE
#endif

/*==========================================================
 * FUNCTION    - csi_util_process_init -
 *
 * DESCRIPTION:
 *==========================================================*/
int csi_util_process_init(csi_t *csi_obj)
{
  int rc = 0;
  uint32_t csid_version = 0;

  CDBG("%s called\n", __func__);

  rc = csiphy_process_init(csi_obj);
  if(rc < 0) {
    CDBG_ERROR("%s line = %d, ERROR = %d\n", __func__, __LINE__, rc);
    goto ERROR;
  }

  rc = csid_process_init(csi_obj, &csid_version);
  if(rc < 0) {
    CDBG_ERROR("%s line = %d, ERROR = %d\n", __func__, __LINE__, rc);
    goto ERROR;
  }

  csi_obj->csid_version = csid_version;
  CDBG("%s csid_version = %x\n", __func__, csid_version);

ERROR:
  return rc;
}

/*==========================================================
 * FUNCTION    - csi_util_set_cfg -
 *
 * DESCRIPTION:
 *==========================================================*/
int csi_util_set_cfg(csi_t *csi_obj, csi_set_data_t *csi_set)
{
  int rc = 0;
  CDBG("%s curr csi2 params = %p, curr res = %d\n", __func__,
    csi_obj->curr_csi2_params,
    csi_set->res);

  if(!csi_obj->csi_params->csi2_params) {
    CDBG_ERROR("%s csi2_params NULL\n", __func__);
    rc = -EINVAL;
    goto ERROR;
  }

  if (csi_obj->csi_params->csi2_params[csi_set->res] ==
       csi_obj->curr_csi2_params)
    return 0;

  rc = csid_set_cfg(csi_obj, csi_set->res);
  if(rc < 0) {
    CDBG_ERROR("%s line = %d, ERROR = %d\n", __func__, __LINE__, rc);
    goto ERROR;
  }

  rc = csiphy_set_cfg(csi_obj, csi_set->res);
  if(rc < 0) {
    CDBG_ERROR("%s line = %d, ERROR = %d\n", __func__, __LINE__, rc);
    goto ERROR;
  }

  csi_obj->curr_csi2_params = csi_obj->csi_params->csi2_params[csi_set->res];

ERROR:
  return rc;
}
