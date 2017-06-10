/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "csid.h"
#ifdef CSI_DEBUG
#undef CDBG
#define CDBG LOGE
#endif

/*==========================================================
 * FUNCTION    - csid_process_init -
 *
 * DESCRIPTION:
 *==========================================================*/
int csid_process_init(csi_t *csi_obj, uint32_t *csid_version)
{
  int rc = 0;
  struct csid_cfg_data cfg;

  cfg.cfgtype = CSID_INIT;
  rc = ioctl(csi_obj->fd, MSM_CAM_IOCTL_CSID_IO_CFG, &cfg);
  if (rc < 0) {
    CDBG_ERROR("%s MSM_CAM_IOCTL_CSID_IO_CFG failed\n", __func__);
  }

  CDBG("%s csid_version = %x\n", __func__, cfg.cfg.csid_version);
  *csid_version = cfg.cfg.csid_version;

  return rc;
}

/*==========================================================
 * FUNCTION    - csid_set_cfg -
 *
 * DESCRIPTION:
 *==========================================================*/
int csid_set_cfg(csi_t *csi_obj, enum msm_sensor_resolution_t res)
{
  int rc = 0;
  struct csid_cfg_data cfg;

  csi_obj->csi_params->csi2_params[res]->csid_params.lane_assign =
    csi_obj->csi_params->csi_lane_params.csi_lane_assign;
  csi_obj->csi_params->csi2_params[res]->csid_params.phy_sel =
    csi_obj->csi_params->csi_lane_params.csi_phy_sel;

  cfg.cfgtype = CSID_CFG;
  cfg.cfg.csid_params =
    &csi_obj->csi_params->csi2_params[res]->csid_params;
  rc = ioctl(csi_obj->fd, MSM_CAM_IOCTL_CSID_IO_CFG, &cfg);
  if (rc < 0) {
    CDBG_ERROR("%s MSM_CAM_IOCTL_CSID_IO_CFG failed\n", __func__);
  }

  return rc;
}
