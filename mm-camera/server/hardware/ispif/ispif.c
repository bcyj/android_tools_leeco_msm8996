/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "ispif.h"

#if ISPIF_DEBUG
#undef CDBG
#define CDBG LOGE
#endif

/*==========================================================
 * FUNCTION    - ispif_process_init -
 *
 * DESCRIPTION:
 *==========================================================*/
int ispif_process_init(ispif_client_t *ispif_client,
  ispif_t *ispif_obj)
{
  int rc = 0;
  struct ispif_cfg_data cfg;
  CDBG("%s called %d\n", __func__, ispif_obj->ref_count);

  if (ispif_obj->ref_count)
    return rc;

  CDBG("%s csid_version = %x\n", __func__, ispif_obj->csid_version);
  cfg.cfgtype = ISPIF_INIT;
  cfg.cfg.csid_version = ispif_obj->csid_version;

  rc = ioctl(ispif_client->ops->fd, MSM_CAM_IOCTL_ISPIF_IO_CFG, &cfg);
  if (rc < 0)
    CDBG_ERROR("%s MSM_CAM_IOCTL_ISPIF_IO_CFG failed\n", __func__);
  return rc;
}

/*==========================================================
 * FUNCTION    - ispif_on_frame_boundary -
 *
 * DESCRIPTION:
 *==========================================================*/
int ispif_process_start_on_frame_boundary(ispif_client_t *ispif_client,
  ispif_t *ispif_obj)
{
  int rc = 0;
  struct ispif_cfg_data cfg;
  CDBG("%s called\n", __func__);
  cfg.cfgtype = ISPIF_SET_ON_FRAME_BOUNDARY;
  cfg.cfg.cmd = ISPIF_STREAM(ispif_client->stream_mask,
    ISPIF_ON_FRAME_BOUNDARY, VFE0);

  rc = ioctl(ispif_client->ops->fd, MSM_CAM_IOCTL_ISPIF_IO_CFG, &cfg);
  if (rc < 0)
    CDBG_ERROR("%s MSM_CAM_IOCTL_ISPIF_IO_CFG failed\n", __func__);

  return rc;
}

/*==========================================================
 * FUNCTION    - ispif_off_frame_boundary -
 *
 * DESCRIPTION:
 *==========================================================*/
int ispif_process_stop_on_frame_boundary(ispif_client_t *ispif_client,
  ispif_t *ispif_obj)
{
  int rc = 0;
  struct ispif_cfg_data cfg;
  CDBG("%s called\n", __func__);
  cfg.cfgtype = ISPIF_SET_OFF_FRAME_BOUNDARY;
  cfg.cfg.cmd = ISPIF_STREAM(ispif_client->stream_mask,
    ISPIF_OFF_FRAME_BOUNDARY, VFE0);

  rc = ioctl(ispif_client->ops->fd, MSM_CAM_IOCTL_ISPIF_IO_CFG, &cfg);
  if (rc < 0)
    CDBG_ERROR("%s MSM_CAM_IOCTL_ISPIF_IO_CFG failed\n", __func__);

  return rc;
}


/*==========================================================
 * FUNCTION    - ispif_off_immediately -
 *
 * DESCRIPTION:
 *==========================================================*/
int ispif_process_stop_immediately(ispif_client_t *ispif_client,
  ispif_t *ispif_obj)
{
  int rc = 0;
  struct ispif_cfg_data cfg;
  CDBG("%s called\n", __func__);
  cfg.cfgtype = ISPIF_SET_OFF_IMMEDIATELY;
  cfg.cfg.cmd = ISPIF_STREAM(ispif_client->stream_mask, ISPIF_OFF_IMMEDIATELY,
     VFE0);

  rc = ioctl(ispif_client->ops->fd, MSM_CAM_IOCTL_ISPIF_IO_CFG, &cfg);
  if (rc < 0)
    CDBG_ERROR("%s MSM_CAM_IOCTL_ISPIF_IO_CFG failed\n", __func__);

  return rc;
}

/*==========================================================
 * FUNCTION    - ispif_set_cfg -
 *
 * DESCRIPTION:
 *==========================================================*/
int ispif_process_cfg(ispif_client_t *ispif_client,
  ispif_t *ispif_obj)
{
  int rc = 0, i = 0, index = 0;
  struct ispif_cfg_data cfg;

  CDBG("%s called\n", __func__);

  cfg.cfgtype = ISPIF_SET_CFG;
  for (i = 0; i < INTF_MAX; i++) {
    if (!ispif_obj->ispif_ctrl[i].pending)
      continue;
    cfg.cfg.ispif_params.params[index++] =
      ispif_obj->ispif_ctrl[i].ispif_params;
    ispif_obj->ispif_ctrl[i].pending = 0;
  }
  if (!index)
    return rc;

  cfg.cfg.ispif_params.len = index;
  rc = ioctl(ispif_client->ops->fd, MSM_CAM_IOCTL_ISPIF_IO_CFG, &cfg);
  if (rc < 0)
    CDBG_ERROR("%s MSM_CAM_IOCTL_ISPIF_IO_CFG failed\n", __func__);

  return rc;
}

/*==========================================================
 * FUNCTION    - ispif_process_release -
 *
 * DESCRIPTION:
 *==========================================================*/
int ispif_process_release(ispif_client_t *ispif_client, ispif_t *ispif_obj)
{
  int rc = 0;
  struct ispif_cfg_data cfg;
  CDBG("%s called %d\n", __func__,ispif_obj->ref_count);
  cfg.cfgtype = ISPIF_RELEASE;
  if (ispif_obj->ref_count)
    return rc;

  rc = ioctl(ispif_client->ops->fd, MSM_CAM_IOCTL_ISPIF_IO_CFG, &cfg);
  if (rc < 0)
    CDBG_ERROR("%s MSM_CAM_IOCTL_ISPIF_IO_CFG failed\n", __func__);

  return rc;
}
