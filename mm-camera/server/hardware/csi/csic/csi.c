/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "csi.h"

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
  struct csic_cfg_data cfg;

  CDBG("%s called\n", __func__);

  cfg.cfgtype = CSIC_INIT;
  rc = ioctl(csi_obj->fd, MSM_CAM_IOCTL_CSIC_IO_CFG, &cfg);
  if (rc < 0) {
    CDBG_ERROR("%s MSM_CAM_IOCTL_CSIC_IO_CFG failed\n", __func__);
  }

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
  struct csic_cfg_data cfg;

  CDBG("%s curr csic params = %p, curr res = %d\n", __func__,
    csi_obj->curr_csic_params,
    csi_set->res);

  if(!csi_obj->csi_params->csic_params) {
    CDBG_ERROR("%s csic_params NULL\n", __func__);
    rc = -EINVAL;
    goto ERROR;
  }

  if (csi_obj->csi_params->csic_params[csi_set->res] ==
       csi_obj->curr_csic_params)
    return 0;

  cfg.cfgtype = CSIC_CFG;
  cfg.csic_params = csi_obj->csi_params->csic_params[csi_set->res];
  rc = ioctl(csi_obj->fd, MSM_CAM_IOCTL_CSIC_IO_CFG, &cfg);
  if (rc < 0) {
    CDBG_ERROR("%s MSM_CAM_IOCTL_CSIC_IO_CFG failed\n", __func__);
  }

  csi_obj->curr_csic_params = csi_obj->csi_params->csic_params[csi_set->res];

ERROR:
  return rc;
}
