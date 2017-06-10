/*==========================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include "camera_dbg.h"
#include <media/msm_camera.h>
#include "af_tuning.h"
#include "actuator.h"

static actuator_ctrl_t actuators[] = {
#include "af_main_cam_0.h"
#include "af_main_cam_1.h"
#include "af_main_cam_2.h"
#include "af_main_cam_3.h"
#include "af_main_cam_4.h"
#include "af_main_cam_5.h"
};

/*==========================================================
* FUNCTION	  - af_actuator_move_focus -
*
* DESCRIPTION: This function passes the command to move the focus.
*
*==========================================================*/
int af_actuator_move_focus(void *ptr, int32_t direction, int32_t num_steps)
{
  int rc = 0;
  actuator_t *af_actuator_ptr = (actuator_t *)ptr;
  struct msm_actuator_cfg_data cfg;
  af_tune_parms_t *af_tune_ptr = &(af_actuator_ptr->ctrl->af_tune);
  uint16_t scenario_size = 0;
  uint16_t index = 0;
  uint16_t curr_scene = 0;
  int16_t dest_step_pos = 0;
  int8_t sign_dir = 0;

  if (af_actuator_ptr->fd <= 0)
    return -EINVAL;

  if (direction == MOVE_NEAR)
    sign_dir = 1;
  else if (direction == MOVE_FAR)
    sign_dir = -1;

  dest_step_pos = af_actuator_ptr->curr_step_pos +
    (sign_dir * num_steps);

  if (dest_step_pos < 0)
    dest_step_pos = 0;
  else if (dest_step_pos > af_actuator_ptr->total_steps)
    dest_step_pos = af_actuator_ptr->total_steps;

  cfg.cfgtype = CFG_MOVE_FOCUS;
  cfg.cfg.move.dir = direction;
  cfg.cfg.move.sign_dir = sign_dir;
  cfg.cfg.move.num_steps = num_steps;
  cfg.cfg.move.dest_step_pos = dest_step_pos;
  curr_scene = 0;
  /* Determine scenario */
  scenario_size = af_tune_ptr->actuator_tuned_params.scenario_size[direction];
  for (index = 0; index < scenario_size; index++) {
    if (num_steps <=
      af_tune_ptr->actuator_tuned_params.ringing_scenario[direction][index]) {
      curr_scene = index;
      break;
    }
  }
  cfg.cfg.move.ringing_params =
    &(af_tune_ptr->actuator_tuned_params.
    damping[direction][curr_scene].ringing_params[0]);

  CDBG("%s: dir:%d, steps:%d\n", __func__, cfg.cfg.move.dir,
    cfg.cfg.move.num_steps);

  /* Invoke the IOCTL to move the focus */
  rc = ioctl(af_actuator_ptr->fd, MSM_CAM_IOCTL_ACTUATOR_IO_CFG, &cfg);
  if (rc < 0) {
    CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
  }
  af_actuator_ptr->curr_step_pos = dest_step_pos;

  return rc;
}

/*==========================================================
* FUNCTION    - af_actuator_restore_focus -
*
* DESCRIPTION: This function passes the command to set the default focus.
*
*==========================================================*/
int af_actuator_restore_focus(void *ptr, int32_t direction)
{
  int rc = 0;
  int16_t new_restore_pos = 0;
  actuator_t *af_actuator_ptr = (actuator_t *)ptr;
  uint8_t af_restore =
    af_actuator_ptr->ctrl->af_tune.actuator_params.af_restore_pos;
  if (af_restore) {
    if (direction == MOVE_NEAR) {
      new_restore_pos = af_actuator_ptr->cur_restore_pos;
    } else if (direction == MOVE_FAR) {
      new_restore_pos = af_actuator_ptr->curr_step_pos;
      af_actuator_ptr->cur_restore_pos = af_actuator_ptr->curr_step_pos;
    }
    CDBG("%s:dir:%d,steps:%d\n", __func__, direction, new_restore_pos);
    rc = af_actuator_move_focus(ptr, direction, new_restore_pos);
  }
  return rc;
}

/*==========================================================
* FUNCTION    - af_actuator_set_default_focus -
*
* DESCRIPTION: This function passes the command to set the default focus.
*
*==========================================================*/
int af_actuator_set_default_focus(void *ptr, int32_t af_step)
{
  int rc = 0;
  actuator_t *af_actuator_ptr = (actuator_t *)ptr;
  struct msm_actuator_cfg_data cfg;
  af_tune_parms_t *af_tune_ptr = &(af_actuator_ptr->ctrl->af_tune);
  uint16_t curr_scene = 0;
  uint16_t scenario_size = 0;
  uint16_t index = 0;

  if (af_actuator_ptr->fd <= 0)
    return -EINVAL;

  cfg.cfgtype = CFG_SET_DEFAULT_FOCUS;
  cfg.cfg.move.dir = MOVE_FAR;
  cfg.cfg.move.sign_dir = -1;
  cfg.cfg.move.num_steps = af_actuator_ptr->curr_step_pos;
  cfg.cfg.move.dest_step_pos = 0;
  curr_scene = 0;
  /* Determine scenario */
  scenario_size = af_tune_ptr->actuator_tuned_params.
    scenario_size[MOVE_FAR];
  for (index = 0; index < scenario_size; index++) {
    if (af_actuator_ptr->curr_step_pos <=
      af_tune_ptr->actuator_tuned_params.
      ringing_scenario[MOVE_FAR][index]) {
      curr_scene = index;
      break;
    }
  }
  cfg.cfg.move.ringing_params =
    &(af_tune_ptr->actuator_tuned_params.
    damping[MOVE_FAR][curr_scene].ringing_params[0]);

  CDBG("%s: dir:%d, steps:%d\n", __func__, cfg.cfg.move.dir,
    cfg.cfg.move.num_steps);

  /* Invoke the IOCTL to set the default focus */
  rc = ioctl(af_actuator_ptr->fd, MSM_CAM_IOCTL_ACTUATOR_IO_CFG, &cfg);
  if (rc < 0) {
    CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
  }

  af_actuator_ptr->curr_step_pos = 0;
  return rc;
}

/*==========================================================
* FUNCTION	- af_actuator_get_info -
*
* DESCRIPTION: This function gets the af parameters to the kernel driver.
*
*==========================================================*/
int af_actuator_get_info(void *ptr, actuator_get_data_t *actuator_get)
{
  int rc = 0;
  actuator_t *af_actuator_ptr = (actuator_t *)ptr;
  af_tune_parms_t *af_tune_ptr = &(af_actuator_ptr->ctrl->af_tune);
  actuator_get->af_support = af_actuator_ptr->is_af_supported;
  return rc;
}

/*==========================================================
* FUNCTION    - af_actuator_init -
*
* DESCRIPTION: This function intializes the actuator object with function pointers.
*
*==========================================================*/
int af_actuator_init(void *ptr)
{
  int rc = 0;
  actuator_t *af_actuator_ptr = (actuator_t *)ptr;
  struct msm_actuator_cfg_data cfg;
  uint8_t cnt = 0;
  uint16_t total_steps = 0;
  actuator_tuned_params_t *actuator_tuned_params = NULL;
  actuator_params_t *actuator_params = NULL;
  if (af_actuator_ptr == NULL) {
    CDBG_ERROR("%s: Invalid Argument - af_actuator_ptr", __func__);
    return -EINVAL;
  }
  af_actuator_ptr->ctrl = NULL;
  af_actuator_ptr->is_af_supported = 0;
  af_actuator_ptr->curr_step_pos = 0;
  af_actuator_ptr->cur_restore_pos = 0;

  rc = ioctl(af_actuator_ptr->fd, MSM_CAM_IOCTL_GET_ACTUATOR_INFO, &cfg);
  if (rc < 0) {
    CDBG_ERROR("MSM_CAM_IOCTL_GET_ACTUATOR_INFO(%d) failed!\n",
      af_actuator_ptr->fd);
    return rc;
  }

  if (cfg.is_af_supported) {
    af_actuator_ptr->is_af_supported = cfg.is_af_supported;
    CDBG("kernel returned %d\n", cfg.cfg.cam_name);
    for (cnt = 0; cnt < (sizeof(actuators) / sizeof(actuators[0])); cnt++) {
      if (cfg.cfg.cam_name == actuators[cnt].af_tune.
        af_header_info.cam_name) {
        af_actuator_ptr->ctrl = &actuators[cnt];
        af_actuator_load_params(ptr, &cfg);
        break;
      }
    }
  }

  return rc;
}

int af_actuator_load_params(void *ptr, struct msm_actuator_cfg_data *cfg)
{
  int rc = 0;
  actuator_t *af_actuator_ptr = (actuator_t *)ptr;
  uint8_t cnt = 0;
  uint16_t total_steps = 0;
  af_tune_parms_t *af_tune_ptr = &(af_actuator_ptr->ctrl->af_tune);
  actuator_tuned_params_t *actuator_tuned_params = NULL;
  actuator_params_t *actuator_params = NULL;
  if (af_actuator_ptr == NULL) {
    CDBG_ERROR("%s: Invalid Argument - af_actuator_ptr", __func__);
    return -EINVAL;
  }

  if (af_actuator_ptr->is_af_supported) {
    actuator_tuned_params = &af_tune_ptr->actuator_tuned_params;
    actuator_params = &af_tune_ptr->actuator_params;

    cfg->cfgtype = CFG_SET_ACTUATOR_INFO;
    total_steps = af_tune_ptr->position_far_end -
      af_tune_ptr->position_near_end + 1;
    total_steps += af_tune_ptr->undershoot_adjust;

    af_actuator_ptr->total_steps = total_steps;
    cfg->cfg.set_info.af_tuning_params.total_steps = total_steps;
    cfg->cfg.set_info.actuator_params.act_type =
      actuator_params->act_type;
    cfg->cfg.set_info.af_tuning_params.pwd_step =
      actuator_tuned_params->region_params[0].step_bound[1];
    cfg->cfg.set_info.af_tuning_params.initial_code =
      actuator_tuned_params->initial_code;
    cfg->cfg.set_info.actuator_params.reg_tbl_size =
      actuator_params->reg_tbl.reg_tbl_size;
    cfg->cfg.set_info.actuator_params.reg_tbl_params =
      &(actuator_params->reg_tbl.reg_params[0]);
    cfg->cfg.set_info.actuator_params.data_size =
      actuator_params->data_size;
    cfg->cfg.set_info.actuator_params.i2c_addr =
      actuator_params->i2c_addr;
    cfg->cfg.set_info.actuator_params.i2c_addr_type =
      actuator_params->i2c_addr_type;

    cfg->cfg.set_info.af_tuning_params.region_size =
      actuator_tuned_params->region_size;
    cfg->cfg.set_info.af_tuning_params.region_params =
      &(actuator_tuned_params->region_params[0]);
    cfg->cfg.set_info.actuator_params.init_setting_size =
      actuator_params->init_setting_size;
    cfg->cfg.set_info.actuator_params.i2c_data_type =
      actuator_params->i2c_data_type;
    cfg->cfg.set_info.actuator_params.init_settings =
      &(actuator_params->init_settings[0]);

    /* Invoke the IOCTL to set the af parameters to the kernel driver */
    rc = ioctl(af_actuator_ptr->fd, MSM_CAM_IOCTL_ACTUATOR_IO_CFG, cfg);
    if (rc < 0) {
      CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
    }
  }

  return rc;
}

int af_actuator_linear_test(void *ptr, uint8_t stepsize)
{
  int rc = 0;
  actuator_t *af_actuator_ptr = (actuator_t *)ptr;
  actuator_ctrl_t *ctrl = af_actuator_ptr->ctrl;
  uint8_t index;

  rc = af_actuator_set_default_focus(ptr, 0);
  usleep(1000000);

  for (index = 0; index < af_actuator_ptr->total_steps;
    index+=stepsize) {
    rc = af_actuator_move_focus(ptr, MOVE_NEAR, stepsize);
    usleep(1000000);
  }

  for (index = 0; index < af_actuator_ptr->total_steps;
    index+=stepsize) {
    rc = af_actuator_move_focus(ptr, MOVE_FAR, stepsize);
    usleep(1000000);
  }
  return rc;
}

int af_actuator_ring_test(void *ptr, uint8_t stepsize)
{
  int rc = 0;
  actuator_t *af_actuator_ptr = (actuator_t *)ptr;
  actuator_ctrl_t *ctrl = af_actuator_ptr->ctrl;
  uint8_t index;

  rc = af_actuator_set_default_focus(ptr, 0);
  usleep(1000000);

  for (index = 0; index < af_actuator_ptr->total_steps;
    index+=stepsize) {
    rc = af_actuator_move_focus(ptr, MOVE_NEAR, stepsize);
    usleep(60000);
  }

  rc = af_actuator_set_default_focus(ptr, 0);
  usleep(1000000);

  return rc;
}
