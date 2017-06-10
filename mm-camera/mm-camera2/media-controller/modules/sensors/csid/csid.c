/* csid.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "csid.h"
#include "sensor_common.h"
#include "server_debug.h"

/*==========================================================
 * FUNCTION    - csid_get_version -
 *
 * DESCRIPTION:
 *==========================================================*/
static int csid_get_version(void *csid_ctrl, void *data)
{
  int rc = SENSOR_SUCCESS;
  sensor_csid_data_t *ctrl = (sensor_csid_data_t *)csid_ctrl;
  uint32_t *csid_version = (uint32_t *)data;

  if (!csid_version) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  *csid_version = ctrl->csid_version;
  return rc;
}
/*==========================================================
 * FUNCTION    - csid_set_lane_params -
 *
 * DESCRIPTION:
 *==========================================================*/
static int csid_set_lane_params(void *csid_ctrl, void *data)
{
  int rc = SENSOR_SUCCESS;
  sensor_csid_data_t *ctrl = (sensor_csid_data_t *)csid_ctrl;
  struct csi_lane_params_t *csi_lane_params = (struct csi_lane_params_t *)data;

  if (!data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  ctrl->csi_lane_params = csi_lane_params;
  return rc;
}

/*==========================================================
 * FUNCTION    - csid_set_cfg -
 *
 * DESCRIPTION:
 *==========================================================*/
static int csid_set_cfg(void *csid_ctrl, void *data)
{
  int rc = SENSOR_SUCCESS;
  sensor_csid_data_t *ctrl = (sensor_csid_data_t *)csid_ctrl;
  struct msm_camera_csid_params *csid_params =
    (struct msm_camera_csid_params *)data;
  struct csid_cfg_data cfg;

  if (!data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  if (csid_params->csi_clk == ctrl->cur_csid_params.csi_clk) {
    SLOW("same csid params");
    return rc;
  }
  csid_params->lane_assign = ctrl->csi_lane_params->csi_lane_assign;
  csid_params->phy_sel = ctrl->csi_lane_params->csi_phy_sel;

  cfg.cfgtype = CSID_CFG;
  cfg.cfg.csid_params = csid_params;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSID_IO_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSID_IO_CFG failed");
  }

  memcpy((void *)&(ctrl->cur_csid_params), (void *)csid_params,
    sizeof(struct msm_camera_csid_params));
  return rc;

}

/*==========================================================
 * FUNCTION    - csid_open -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t csid_open(void **csid_ctrl, const char *subdev_name)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_csid_data_t *ctrl = NULL;
  struct csid_cfg_data cfg;
  char subdev_string[32];

  SLOW("Enter");
  if (!csid_ctrl || !subdev_name) {
    SERR("failed sctrl %p subdev name %p",
      csid_ctrl, subdev_name);
    return SENSOR_ERROR_INVAL;
  }
  ctrl = malloc(sizeof(sensor_csid_data_t));
  if (!ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  memset(ctrl, 0, sizeof(sensor_csid_data_t));

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s", subdev_name);
  SLOW("sd name %s", subdev_string);
  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if ((ctrl->fd) >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    ctrl->fd = -1;
    goto ERROR;
  }
  if (ctrl->fd < 0) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR;
  }

  cfg.cfgtype = CSID_INIT;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSID_IO_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSID_IO_CFG failed %s", strerror(errno));
    goto ERROR;
  }
  ctrl->csid_version = cfg.cfg.csid_version;

  *csid_ctrl = (void *)ctrl;
  SLOW("Exit");
  return rc;

ERROR:
  free(ctrl);
  return rc;
}

/*==========================================================
 * FUNCTION    - csid_process -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t csid_process(void *csid_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  if (!csid_ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  switch (event) {
  case CSID_GET_VERSION:
    csid_get_version(csid_ctrl, data);
    break;
  case CSID_SET_LANE_PARAMS:
    csid_set_lane_params(csid_ctrl, data);
    break;
  case CSID_SET_CFG:
    csid_set_cfg(csid_ctrl, data);
    break;
  default:
    SERR("invalid event %d", event);
    rc = SENSOR_FAILURE;
   break;
  }
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - csid_close -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t csid_close(void *csid_ctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_csid_data_t *ctrl = (sensor_csid_data_t *)csid_ctrl;
  struct csid_cfg_data cfg;

  if (!ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  SLOW("Enter");
  cfg.cfgtype = CSID_RELEASE;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSID_IO_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSID_IO_CFG failed");
  }

  /* close subdev */
  close(ctrl->fd);

  free(ctrl);
  SLOW("Exit");
  return rc;
}

/*==========================================================
 * FUNCTION    - csid_sub_module_init -
 *
 * DESCRIPTION:
 *==========================================================*/
int32_t csid_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  if (!func_tbl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  func_tbl->open = csid_open;
  func_tbl->process = csid_process;
  func_tbl->close = csid_close;
  return SENSOR_SUCCESS;
}
