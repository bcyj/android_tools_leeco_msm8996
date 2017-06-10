/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "csiphy.h"
#ifdef CSI_DEBUG
#undef CDBG
#define CDBG LOGE
#endif

/*==========================================================
 * FUNCTION    - csiphy_process_init -
 *
 * DESCRIPTION:
 *==========================================================*/
int csiphy_process_init(csi_t *csi_obj)
{
  int rc = 0;
  struct csiphy_cfg_data cfg;

  CDBG("%s called\n", __func__);

  cfg.cfgtype = CSIPHY_INIT;
  rc = ioctl(csi_obj->fd, MSM_CAM_IOCTL_CSIPHY_IO_CFG, &cfg);
  if (rc < 0) {
    CDBG_ERROR("%s MSM_CAM_IOCTL_CSIPHY_IO_CFG failed\n", __func__);
  }

  return rc;
}

/*==========================================================
 * FUNCTION    - csiphy_set_cfg -
 *
 * DESCRIPTION:
 *==========================================================*/
int csiphy_set_cfg(csi_t *csi_obj, enum msm_sensor_resolution_t res)
{
  int rc = 0;
  struct csiphy_cfg_data cfg;

  csi_obj->csi_params->csi2_params[res]->csiphy_params.lane_mask =
    csi_obj->csi_params->csi_lane_params.csi_lane_mask;

  CDBG("%s csi2_params = %p\n", __func__,
    csi_obj->csi_params->csi2_params[res]);

  cfg.cfgtype = CSIPHY_CFG;
  cfg.csiphy_params = &csi_obj->csi_params->csi2_params[res]->csiphy_params;
  rc = ioctl(csi_obj->fd, MSM_CAM_IOCTL_CSIPHY_IO_CFG, &cfg);
  if (rc < 0) {
    CDBG_ERROR("%s MSM_CAM_IOCTL_CSIPHY_IO_CFG failed\n", __func__);
  }

  return rc;
}
