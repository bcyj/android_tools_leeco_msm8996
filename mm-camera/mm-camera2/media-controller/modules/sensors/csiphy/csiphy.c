/* csiphy.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "csiphy.h"
#include "sensor_common.h"
#include "server_debug.h"

/*==========================================================
 * FUNCTION    - csiphy_set_lane_params -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t csiphy_set_lane_params(void *csiphy_ctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_csiphy_data_t *ctrl = (sensor_csiphy_data_t *)csiphy_ctrl;
  struct csi_lane_params_t *csi_lane_params = (struct csi_lane_params_t *)data;

  if (!data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  ctrl->csi_lane_params = csi_lane_params;
  return rc;
}

/*==========================================================
 * FUNCTION    - csiphy_set_cfg -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t csiphy_set_cfg(void *csiphy_ctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_csiphy_data_t *ctrl = (sensor_csiphy_data_t *)csiphy_ctrl;
  struct msm_camera_csiphy_params *csiphy_params =
    (struct msm_camera_csiphy_params *)data;
  struct csiphy_cfg_data cfg;

  if (!data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  if (csiphy_params->csiphy_clk == ctrl->cur_csiphy_params.csiphy_clk) {
    SLOW("same csiphy params");
    return rc;
  }
  csiphy_params->lane_mask = ctrl->csi_lane_params->csi_lane_mask;

  cfg.cfgtype = CSIPHY_CFG;
  cfg.cfg.csiphy_params = csiphy_params;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSIPHY_IO_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSIPHY_IO_CFG failed");
  }

  memcpy((void *)&(ctrl->cur_csiphy_params), (void *)csiphy_params,
    sizeof(struct msm_camera_csiphy_params));
  return rc;
}

/*==========================================================
 * FUNCTION    - csiphy_open -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t csiphy_open(void **csiphy_ctrl, const char *subdev_name)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_csiphy_data_t *ctrl = NULL;
  struct csiphy_cfg_data cfg;
  char subdev_string[32];

  if (!csiphy_ctrl || !subdev_name) {
    SERR("failed sctrl %p subdev name %p",
      csiphy_ctrl, subdev_name);
    return SENSOR_ERROR_INVAL;
  }
  ctrl = malloc(sizeof(sensor_csiphy_data_t));
  if (!ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  memset(ctrl, 0, sizeof(sensor_csiphy_data_t));

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s", subdev_name);
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

  cfg.cfgtype = CSIPHY_INIT;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSIPHY_IO_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSIPHY_IO_CFG failed");
    goto ERROR;
  }

  *csiphy_ctrl = (void *)ctrl;
  return rc;

ERROR:
  free(ctrl);
  return rc;
}

/*==========================================================
 * FUNCTION    - csiphy_process -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t csiphy_process(void *csiphy_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  if (!csiphy_ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  switch (event) {
  case CSIPHY_SET_LANE_PARAMS:
    csiphy_set_lane_params(csiphy_ctrl, data);
    break;
  case CSIPHY_SET_CFG:
    csiphy_set_cfg(csiphy_ctrl, data);
    break;
  default:
    SERR("invalid event %d", event);
    rc = SENSOR_FAILURE;
    break;
  }
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - csiphy_close -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t csiphy_close(void *csiphy_ctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_csiphy_data_t *ctrl = (sensor_csiphy_data_t *)csiphy_ctrl;
  struct csiphy_cfg_data cfg;
  struct msm_camera_csi_lane_params csi_lane_params;

  if (!ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  cfg.cfgtype = CSIPHY_RELEASE;
  memset(&csi_lane_params, 0, sizeof(csi_lane_params));
  if (ctrl->csi_lane_params) {
    csi_lane_params.csi_lane_assign = ctrl->csi_lane_params->csi_lane_assign;
    csi_lane_params.csi_lane_mask = ctrl->csi_lane_params->csi_lane_mask;
  }
  cfg.cfg.csi_lane_params = &csi_lane_params;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_CSIPHY_IO_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_CSIPHY_IO_CFG failed");
  }

  /* close subdev */
  close(ctrl->fd);

  free(ctrl);
  return rc;
}

/*==========================================================
 * FUNCTION    - csiphy_sub_module_init -
 *
 * DESCRIPTION:
 *==========================================================*/
int32_t csiphy_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  if (!func_tbl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  func_tbl->open = csiphy_open;
  func_tbl->process = csiphy_process;
  func_tbl->close = csiphy_close;
  return SENSOR_SUCCESS;
}
