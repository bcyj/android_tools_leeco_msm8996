/* sensor.c
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <cutils/trace.h>
#include <stdio.h>
#include <dlfcn.h>
#include <math.h>
#include <asm-generic/errno-base.h>
#include <poll.h>
#include "sensor.h"
#include "modules.h"
#include "mct_pipeline.h"
//#include "sensor_thread.h"

#define QFACTOR 100
#define BUFF_SIZE_255 255

#define MAX_VFE_CLK 320000000

#undef ENABLE_DIS_MARGIN
//#define ENABLE_DIS_MARGIN

#define NANO_SEC_PER_SEC 1000000000
#define NANO_SEC_TO_MICRO_SEC 1000
#define MAX_FPS_VARIANCE 1.0f
#define ATRACE_TAG ATRACE_TAG_CAMERA

/*===========================================================================
* FUNCTION - LOG_IOCTL -
*
* DESCRIPTION: Wrapper for logging and to trace ioctl calls.
*==========================================================================*/
static int LOG_IOCTL(int d, int request, void* par1)
{
  int ret;
  ATRACE_BEGIN("Camera:sensorIoctl");
  ret = ioctl(d, request, par1);
  ATRACE_END();
  return ret;
}

/*===========================================================================
 * FUNCTION    - sensor_load_library -
 *
 * DESCRIPTION:
 *==========================================================================*/
int32_t sensor_load_library(const char *name, void *data)
{
  char lib_name[BUFF_SIZE_255] = {0};
  char open_lib_str[BUFF_SIZE_255] = {0};
  void *(*sensor_open_lib)(void) = NULL;
  sensor_lib_params_t *sensor_lib_params = (sensor_lib_params_t *)data;
  SLOW("enter");
  snprintf(lib_name, BUFF_SIZE_255, "libmmcamera_%s.so", name);
  SLOW("lib_name %s", lib_name);
  sensor_lib_params->sensor_lib_handle = dlopen(lib_name, RTLD_NOW);
  if (!sensor_lib_params->sensor_lib_handle) {
    return -EINVAL;
  }
  snprintf(open_lib_str, BUFF_SIZE_255, "%s_open_lib", name);
  *(void **)&sensor_open_lib = dlsym(sensor_lib_params->sensor_lib_handle,
    open_lib_str);
  if (!sensor_open_lib) {
    SERR("failed");
    return -EINVAL;
  }
  sensor_lib_params->sensor_lib_ptr = (sensor_lib_t *)sensor_open_lib();
  if (!sensor_lib_params->sensor_lib_ptr) {
    SERR("failed");
    return -EINVAL;
  }
  SLOW("exit");
  return 0;
}

/*===========================================================================
 * FUNCTION    - sensor_unload_library -
 *
 * DESCRIPTION:
 *==========================================================================*/
int32_t sensor_unload_library(sensor_lib_params_t *sensor_lib_params)
{
  if (!sensor_lib_params) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  if (sensor_lib_params->sensor_lib_handle) {
    dlclose(sensor_lib_params->sensor_lib_handle);
    sensor_lib_params->sensor_lib_handle = NULL;
    sensor_lib_params->sensor_lib_ptr = NULL;
  }
  return SENSOR_SUCCESS;
}

/** sensor_probe: probe available sensors
 *
 *  @fd: sensor_init fd
 *  @sensor_name: sensor name
 *
 *  Return: TRUE for success and FALSE for failure
 *
 *  1) Open sensor library
 *
 *  2) Pass slave information, probe sensor
 *
 *  3) If probe succeeds, create video node and sensor subdev is
 *  created in kernel
 *
 **/

int32_t sensor_probe(int32_t fd, const char *sensor_name)
{
  boolean                              ret = TRUE;
  int32_t                              rc = 0;
  sensor_lib_params_t                  sensor_lib_params;
  struct sensor_init_cfg_data          cfg;
  struct msm_camera_sensor_slave_info *slave_info = NULL;
  char *eeprom_name = NULL;
  char *actuator_name = NULL;

  /* Validate input parameters */
  if ((fd < 0) || !sensor_name) {
    SERR("failed: invalid params fd %d sensor_name %s", fd, sensor_name);
    return FALSE;
  }

  /* Load sensor library */
  rc = sensor_load_library(sensor_name, &sensor_lib_params);
  if (rc < 0) {
    SERR("failed: to load %s", sensor_name);
    return FALSE;
  }

  slave_info = sensor_lib_params.sensor_lib_ptr->sensor_slave_info;

  /* Check whether slave information is present */
  if (!slave_info) {
    SERR("failed: sensor_slave_info %p", slave_info);
    ret = FALSE;
    goto ERROR;
  }
  /*enabling flash support to all sensors for backward compatibility*/
  sensor_lib_params.sensor_lib_ptr->sensor_slave_info->is_flash_supported = 1;

  struct msm_sensor_init_params *sensor_init_params;
  sensor_init_params = sensor_lib_params.sensor_lib_ptr->sensor_init_params;
  if (sensor_init_params) {
    slave_info->is_init_params_valid = 1;
    slave_info->sensor_init_params = *sensor_init_params;
  }

  /* Update sensor name in slave info */
  memcpy(slave_info->sensor_name, sensor_name, sizeof(slave_info->sensor_name));

  /* Update eeprom name in slave info */
  eeprom_name = sensor_lib_params.sensor_lib_ptr->eeprom_name;
  if (eeprom_name)
    if (strlen(eeprom_name) <  MAX_SENSOR_NAME)
      strlcpy(slave_info->eeprom_name,
               eeprom_name, strlen(eeprom_name) + 1);

  /* Update actuator name in slave info */
  actuator_name = sensor_lib_params.sensor_lib_ptr->actuator_name;
  if (actuator_name)
    if (strlen(actuator_name) <  MAX_SENSOR_NAME)
      strlcpy(slave_info->actuator_name,
               actuator_name, strlen(actuator_name) + 1);

  /* Pass slave information to kernel and probe */
  cfg.cfgtype = CFG_SINIT_PROBE;
  cfg.cfg.setting = slave_info;
  if (LOG_IOCTL(fd, VIDIOC_MSM_SENSOR_INIT_CFG, &cfg) < 0) {
    SERR("failed");
    ret = FALSE;
    goto ERROR;
  }

ERROR:
  /* Unload sensor library */
  sensor_unload_library(&sensor_lib_params);

  return ret;
}

/*===========================================================================
 * FUNCTION    - sensor_set_slave_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_slave_info(void *sctrl)
{
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = ctrl->lib_params;
  SLOW("enter");

  if (lib->sensor_lib_ptr->sensor_slave_info) {
    cfg.cfgtype = CFG_SET_SLAVE_INFO;
    cfg.cfg.setting = (void *)lib->sensor_lib_ptr->sensor_slave_info;
    if (ioctl(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
      SERR("failed");
      return -EIO;
    }
  }

  SLOW("exit");
  return 0;
}

/*===========================================================================
 * FUNCTION    - sensor_set_stop_stream_settings -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_stop_stream_settings(void *sctrl)
{
  int32_t rc = 0;
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = ctrl->lib_params;
  SLOW("enter");

  cfg.cfgtype = CFG_SET_STOP_STREAM_SETTING;
  cfg.cfg.setting = (void *)lib->sensor_lib_ptr->stop_settings;
  rc = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
  if (rc < 0) {
    SERR("failed rc %d", rc);
    return -EIO;
  }

  SLOW("exit");
  return 0;
}

/*===========================================================================
 * FUNCTION    - sensor_power_up -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_power_up(void *sctrl)
{
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = ctrl->lib_params;
  SLOW("enter");

  cfg.cfgtype = CFG_POWER_UP;
  if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
    SERR("failed");
    return -EIO;
  }

  SLOW("exit");
  return 0;
}

/*===========================================================================
 * FUNCTION    - sensor_write_init_settings -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_write_init_settings(void *sctrl)
{
  int32_t rc = 0;
  uint16_t index = 0;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = ctrl->lib_params;
  struct sensor_lib_reg_settings_array *init_settings = NULL;
  struct sensorb_cfg_data cfg;
  SLOW("enter");

  if (lib->sensor_lib_ptr->sensor_output->output_format != SENSOR_YCBCR) {
    cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
    init_settings = lib->sensor_lib_ptr->init_settings_array;
    for (index = 0; index < init_settings->size; index++) {
      cfg.cfg.setting = &init_settings->reg_settings[index];
      if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
        SERR("failed");
        return -EIO;
      }
    }
  } else {
    cfg.cfgtype = CFG_SET_INIT_SETTING;
    if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
      SERR("failed");
      return -EIO;
    }
  }
  SLOW("exit");
  return rc;
}

/*===========================================================================
 * FUNCTION    - sensor_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_init(void *sctrl)
{
  int32_t rc = 0;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  SLOW("enter ctrl %p", ctrl);

  if (ctrl->s_data->fd < 0) {
    SERR("failed");
    return -EINVAL;
  }

  ctrl->s_data->cur_stream_mask = 1 << CAM_STREAM_TYPE_DEFAULT;
  ctrl->s_data->current_fps_div = Q10;

  if (lib->sensor_lib_ptr->sensor_output->output_format != SENSOR_YCBCR) {
    rc = sensor_set_stop_stream_settings(sctrl);
    if (rc < 0) {
      SERR("failed");
      return rc;
    }
  }

  rc = sensor_power_up(sctrl);
  if (rc < 0) {
    SERR("failed");
    return rc;
  }

  rc = sensor_write_init_settings(sctrl);
  if (rc < 0) {
    SERR("failed");
    return rc;
  }
  ctrl->s_data->cur_res = MSM_SENSOR_INVALID_RES;
  ctrl->s_data->hfr_mode = CAM_HFR_MODE_OFF;
  ctrl->s_data->video_hdr_enable = 0;
  ctrl->s_data->snapshot_hdr_enable = 0;
  SLOW("exit");
  return 0;
}

/*+++++======================================================================
 * FUNCTION    - sensor_get_cur_fps -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_get_cur_fps(void *sctrl, void *fps_data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  uint32_t *fps = (uint32_t *) fps_data;
  struct sensor_lib_out_info_t *out_info = NULL;
  enum msm_sensor_resolution_t res = ctrl->s_data->cur_res;
  uint32_t frame_length_lines = 0;
  float cur_fps = 0;

  SLOW("enter");
  if (!fps_data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  if (res >= lib->sensor_lib_ptr->out_info_array->size) {
    SERR("error - res %d >= max res size %d",
      res, lib->sensor_lib_ptr->out_info_array->size);
    return -EINVAL;
  }

  out_info = &lib->sensor_lib_ptr->out_info_array->out_info[res];

  frame_length_lines = out_info->frame_length_lines;
  cur_fps = out_info->max_fps;

  if (ctrl->s_data->current_linecount > frame_length_lines)
    cur_fps = (cur_fps * frame_length_lines) / ctrl->s_data->current_linecount;
  *fps = cur_fps * Q8;
  SLOW("exit");
  return rc;
}

/*===========================================================================
 * FUNCTION    - sensor_set_hfr_mode -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_set_hfr_mode(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("Failed");
    return SENSOR_FAILURE;
  }
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  cam_hfr_mode_t hfr_mode = *((cam_hfr_mode_t*)data);
  SLOW("hfr_mode = %d", hfr_mode);
  ctrl->s_data->hfr_mode = hfr_mode;
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_hdr_ae_bracket -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_set_hdr_ae_bracket(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  cam_exp_bracketing_t *ae_bracket_config = (cam_exp_bracketing_t*)data;
  SLOW("ae_bracket_mode=%d, str=%s",
         ae_bracket_config->mode, ae_bracket_config->values);
  /* copy the ae_bracket config in local data structure */
  memcpy(&(ctrl->s_data->ae_bracket_info.ae_bracket_config),
          ae_bracket_config, sizeof(cam_exp_bracketing_t));
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_aec_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_set_aec_update(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  aec_update_t* aec_update = (aec_update_t*) data;
  sensor_exp_t exposure;

  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  if (!((ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT))
    && (ctrl->s_data->ae_bracket_info.ae_bracket_config.mode ==
      CAM_EXP_BRACKETING_ON))) {

        exposure.luma_hdr = 0;
        exposure.fgain_hdr = 0;
        exposure.real_gain = aec_update->real_gain;
        exposure.linecount = aec_update->linecount;

        if ((ctrl->s_data->video_hdr_enable) ||
            (ctrl->s_data->snapshot_hdr_enable)) {
          exposure.luma_hdr = aec_update->cur_luma;
          exposure.fgain_hdr = aec_update->luma_delta;
          exposure.fgain_hdr |= aec_update->hdr_indoor_detected << 16;
        }

        sensor_set_exposure(sctrl, exposure);
        sensor_apply_exposure(sctrl); /* apply exposure */
  }

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_aec_init_settings -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_set_aec_init_settings(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  aec_get_t* aec_data = (aec_get_t*) data;
  int valid_entries = aec_data->valid_entries;
  SLOW("aec_data.valid_entries=%d", valid_entries);
  if (valid_entries <= 0) {
    SERR("no valid entries in aec_get");
    return SENSOR_FAILURE;
  }
  if ((ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT)) &&
     (ctrl->s_data->ae_bracket_info.ae_bracket_config.mode ==
     CAM_EXP_BRACKETING_ON)) {
    ctrl->s_data->ae_bracket_info.valid_entries = valid_entries;
    int i;
    SLOW("valid entries %d", valid_entries);
    for (i=0; i < valid_entries; i++) {
      SLOW("g%d=%f, lc%d=%d", i,
        aec_data->real_gain[i], i, aec_data->linecount[i]);
      ctrl->s_data->ae_bracket_info.real_gain[i] = aec_data->real_gain[i];
      ctrl->s_data->ae_bracket_info.linecount[i] = aec_data->linecount[i];
    }
    /* to apply entries starting from 1 from next SOF
        (entry 0 will be set in sensor_set_aec_init_settings) */
    ctrl->s_data->ae_bracket_info.apply_index = 0;
    ctrl->s_data->ae_bracket_info.sof_counter = aec_data->valid_entries;
    ctrl->s_data->sensor_skip_counter = 0;
    ctrl->s_data->ae_bracket_info.sync_last_gain_needed = 0;
    ctrl->s_data->ae_bracket_info.post_meta_bus = 0;
    ctrl->s_data->ae_bracket_info.skip_frame = 0;

  } else {
    /* update exposure, first valid entry */
    sensor_exp_t exposure;
    memset(&exposure, 0, sizeof(exposure));
    exposure.real_gain = aec_data->real_gain[0];
    exposure.linecount = aec_data->linecount[0];
    sensor_set_exposure(sctrl, exposure);
    ctrl->s_data->apply_without_sync = TRUE;
    sensor_apply_exposure(sctrl);
  }
  return SENSOR_SUCCESS;
}
/*===========================================================================
 * FUNCTION    - sensor_set_hdr_zsl_mode -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_set_hdr_zsl_mode(void *sctrl, void *data)
{
  int8_t ret = SENSOR_SUCCESS;

  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  sensor_set_hdr_ae_t *hdr_info = (sensor_set_hdr_ae_t *)data;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = ctrl->lib_params;
  uint32_t sensor_max_pipeline_frame_delay;

  sensor_max_pipeline_frame_delay =
    lib->sensor_lib_ptr->sensor_max_pipeline_frame_delay;

  ctrl->s_data->hdr_zsl_mode = hdr_info->hdr_zsl_mode;
  ctrl->s_data->isp_frame_skip = hdr_info->isp_frame_skip;
  ctrl->s_data->next_valid_frame = sensor_max_pipeline_frame_delay;
  return ret;
}

/*===========================================================================
 * FUNCTION    - sensor_set_manual_exposure_mode -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_set_manual_exposure_mode(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("Failed");
    return SENSOR_FAILURE;
  }
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  uint8_t manual_exposure_mode = *((uint8_t*)data);
  SLOW("manual_exposure_mode = %d", manual_exposure_mode);
  ctrl->s_data->manual_exposure_mode = manual_exposure_mode;
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_post_hdr_meta -
 *
 * DESCRIPTION:
 *==========================================================================*/

static int8_t sensor_post_hdr_meta(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  if (!ctrl || !ctrl->s_data){
    SERR("Failed");
    return SENSOR_FAILURE;
  }
  if (!data)
    return SENSOR_SUCCESS; /*Null hdr meta pointer is not fatal*/
  sensor_hdr_meta_t *hdr_meta = (sensor_hdr_meta_t *)data;
  /*Init default value to post hdr meta*/
  hdr_meta->post_meta_bus = 0;
  if (ctrl->s_data->ae_bracket_info.ae_bracket_config.mode !=
    CAM_EXP_BRACKETING_ON)
    return SENSOR_SUCCESS;

  /*Conditions to determine whether to post hdr meta to bus:
   * - Hdr information needs to be posted to bus only for Zsl case
   *
   * - Current implementation requires meta data to be posted only for
   * first HDR frame from hdr counter. We match the hdr counter against
   * the current sof counter also accounting the isp frame skip.
   */
  if (ctrl->s_data->hdr_zsl_mode) {
      if (!ctrl->s_data->ae_bracket_info.post_meta_bus) {
        hdr_meta->post_meta_bus = 1;
        ctrl->s_data->ae_bracket_info.post_meta_bus = 1;
        hdr_meta->isp_frame_skip = ctrl->s_data->isp_frame_skip;
        hdr_meta->next_valid_frame = ctrl->s_data->next_valid_frame;
     }
  }
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_aec_zsl_settings -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_set_aec_zsl_settings(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  aec_get_t* aec_data = (aec_get_t*) data;
  int valid_entries = aec_data->valid_entries;
  SLOW("zsl aec_data.valid_entries=%d", valid_entries);
  if (valid_entries <= 0) {
    SERR("no valid entries in aec_get");
    return SENSOR_FAILURE;
  }
  if (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT)) {
    if (ctrl->s_data->ae_bracket_info.ae_bracket_config.mode ==
          CAM_EXP_BRACKETING_ON) {
      ctrl->s_data->ae_bracket_info.valid_entries = valid_entries;
      int i;
      for (i=0; i < valid_entries; i++) {
        SERR("g%d=%f, lc%d=%d", i,
             aec_data->real_gain[i], i, aec_data->linecount[i]);
        ctrl->s_data->ae_bracket_info.real_gain[i] = aec_data->real_gain[i];
        ctrl->s_data->ae_bracket_info.linecount[i] = aec_data->linecount[i];
      }
      ctrl->s_data->sensor_skip_counter = 0;
      ctrl->s_data->ae_bracket_info.apply_index = 0;
      ctrl->s_data->ae_bracket_info.sof_counter = aec_data->valid_entries;
      ctrl->s_data->ae_bracket_info.sync_last_gain_needed = 0;
      ctrl->s_data->ae_bracket_info.post_meta_bus = 0;
      ctrl->s_data->ae_bracket_info.skip_frame = 1;
      ctrl->s_data->apply_without_sync = FALSE;
    }
  }

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_vfe_sof -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_vfe_sof(void *sctrl, void *data)
{

  int32_t valid_entries;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = NULL;
  if (!ctrl || !ctrl->s_data  || !ctrl->lib_params) {
    return SENSOR_FAILURE;
  }
  lib = (sensor_lib_params_t *)ctrl->lib_params;
  valid_entries = ctrl->s_data->ae_bracket_info.valid_entries;
  /* apply exposures in AE bracketing mode */
  if (ctrl->s_data->ae_bracket_info.ae_bracket_config.mode ==
       CAM_EXP_BRACKETING_ON && valid_entries > 1) {

    if (ctrl->s_data->ae_bracket_info.skip_frame > 0) {
      ctrl->s_data->ae_bracket_info.skip_frame--;
      SLOW("AE Bracketing frame skip");
      return SENSOR_SUCCESS;
    }

    /* Disable bracketing once all exposures have been applied */
    if ( ctrl->s_data->ae_bracket_info.apply_index == valid_entries){
      ctrl->s_data->ae_bracket_info.apply_index = 0;
      ctrl->s_data->ae_bracket_info.ae_bracket_config.mode = CAM_EXP_BRACKETING_OFF;
    }

    if ((ctrl->s_data->ae_bracket_info.sof_counter > 0) &&
      (ctrl->s_data->sensor_skip_counter <= 0)) {

      sensor_exp_t exposure;
      memset(&exposure, 0, sizeof(exposure));
      int32_t idx = ctrl->s_data->ae_bracket_info.apply_index % valid_entries;

      exposure.real_gain = ctrl->s_data->ae_bracket_info.real_gain[idx];
      exposure.linecount = ctrl->s_data->ae_bracket_info.linecount[idx];
      SERR("ae-bracket:idx[%d] of %d real_gain=%f, linecount=%d",
        idx, valid_entries,
               exposure.real_gain, exposure.linecount);
      SLOW("ae-bracket: sof_counter=%d, applying exposure, idx=%d",
        ctrl->s_data->ae_bracket_info.sof_counter, idx);
      sensor_set_exposure(sctrl, exposure);
      sensor_apply_exposure(sctrl);
      sensor_post_hdr_meta(sctrl, data);
      ctrl->s_data->ae_bracket_info.sof_counter--;
      ctrl->s_data->ae_bracket_info.apply_index++;
      ctrl->s_data->sensor_skip_counter = ctrl->s_data->isp_frame_skip;
      if(ctrl->s_data->ae_bracket_info.sof_counter == 0 &&
        ctrl->s_data->stored_gain != ctrl->s_data->prev_gain &&
        ctrl->s_data->sensor_skip_counter == 0 &&
        lib->sensor_lib_ptr->sync_exp_gain == 1){
        ctrl->s_data->ae_bracket_info.sync_last_gain_needed = 1;
      } else if(ctrl->s_data->stored_gain != ctrl->s_data->prev_gain &&
        ctrl->s_data->sensor_skip_counter > 0 &&
        lib->sensor_lib_ptr->sync_exp_gain == 1){
        ctrl->s_data->ae_bracket_info.next_gain_needed = 1;
      }
     } else {
      if(ctrl->s_data->ae_bracket_info.sync_last_gain_needed){
        sensor_apply_exposure(sctrl);
        ctrl->s_data->ae_bracket_info.sync_last_gain_needed = 0;
      }
      if(ctrl->s_data->ae_bracket_info.next_gain_needed) {
        sensor_apply_exposure(sctrl);
        ctrl->s_data->ae_bracket_info.next_gain_needed = 0;
      }
      SERR("AE bracket exposure not applied:"
        "sof counter=%d, sensor frame skip = %d",
        ctrl->s_data->ae_bracket_info.sof_counter,
        ctrl->s_data->sensor_skip_counter);
      if (ctrl->s_data->sensor_skip_counter > 0) {
        ctrl->s_data->sensor_skip_counter--;
      }
    }
  } else {
    SLOW("AE Bracketing not enabled");
  }
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_frame_rate -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_set_frame_rate(void *sctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  cam_fps_range_t *fps = (cam_fps_range_t *)data;
  enum msm_sensor_resolution_t cur_res = MSM_SENSOR_INVALID_RES;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  if (!fps) {
    SERR("data NULL");
    return SENSOR_FAILURE;
  }
  SLOW("max fps %f min fps %f", fps->max_fps,
    fps->min_fps);
  ctrl->s_data->max_fps = ctrl->s_data->cur_fps = fps->video_max_fps;
  /* Update fps divider if cur stream type is preview or video */
  if ((ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW) ||
    ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)) &&
    (ctrl->s_data->cur_res != MSM_SENSOR_INVALID_RES)) {
    cur_res = ctrl->s_data->cur_res;
    out_info_array = ctrl->lib_params->sensor_lib_ptr->out_info_array;
    if (ctrl->s_data->cur_fps > out_info_array->out_info[cur_res].max_fps) {
      SLOW("set_fps=%f > max_fps=%f, capping to max",
        ctrl->s_data->cur_fps, out_info_array->out_info[cur_res].max_fps);
      ctrl->s_data->cur_fps = out_info_array->out_info[cur_res].max_fps;
    }
    ctrl->s_data->current_fps_div =
      (out_info_array->out_info[cur_res].max_fps * Q10) / ctrl->s_data->cur_fps;
    ctrl->s_data->cur_fps =
      (out_info_array->out_info[cur_res].max_fps * Q10) / ctrl->s_data->current_fps_div;
    ctrl->s_data->prev_gain = 0;
    ctrl->s_data->prev_linecount = 0;
    sensor_set_vfe_sof(sctrl, NULL);
  }
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_apply_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_apply_exposure(void *sctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  uint32_t fl_lines;
  uint8_t offset;
  struct sensorb_cfg_data cfg;
  struct msm_camera_i2c_reg_setting exp_gain;
  uint32_t gain_apply = 0;

  if (ctrl->s_data->prev_gain == ctrl->s_data->current_gain &&
      ctrl->s_data->prev_gain == ctrl->s_data->stored_gain &&
      ctrl->s_data->prev_linecount == ctrl->s_data->current_linecount)
    return 0;

  fl_lines = ctrl->s_data->cur_frame_length_lines;
  if (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW) ||
    ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_VIDEO))
    fl_lines = (fl_lines * ctrl->s_data->current_fps_div) / Q10;

  offset = lib->sensor_lib_ptr->exp_gain_info->vert_offset;
  if (ctrl->s_data->current_linecount > (fl_lines - offset))
    fl_lines = ctrl->s_data->current_linecount + offset;

  memset(&exp_gain, 0, sizeof(exp_gain));
  exp_gain.reg_setting = malloc(lib->sensor_lib_ptr->exposure_table_size *
    sizeof(struct msm_camera_i2c_reg_array));
  if (!exp_gain.reg_setting) {
    SERR("failed");
    return -ENOMEM;
  }

  memset(exp_gain.reg_setting, 0, lib->sensor_lib_ptr->exposure_table_size *
    sizeof(struct msm_camera_i2c_reg_array));

  if(lib->sensor_lib_ptr->sync_exp_gain == 1)
    gain_apply = ctrl->s_data->stored_gain;
  else
    gain_apply = ctrl->s_data->current_gain;

  if(ctrl->s_data->apply_without_sync == TRUE){
    gain_apply = ctrl->s_data->current_gain;
    ctrl->s_data->apply_without_sync = FALSE;
  }

  rc = lib->sensor_lib_ptr->exposure_func_table->
    sensor_fill_exposure_array(gain_apply,
      ctrl->s_data->current_linecount, fl_lines,
      ctrl->s_data->current_luma_hdr,
      ctrl->s_data->current_fgain_hdr, &exp_gain);
  if ((rc < 0) || (exp_gain.size == 0)) {
    SERR("failed");
  } else {
    SLOW("cur_gain=%d, applied gain=%d, cur_linecount=%d, fl_lines=%d",
      ctrl->s_data->current_gain, gain_apply,
      ctrl->s_data->current_linecount, fl_lines);
    cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
    cfg.cfg.setting = &exp_gain;
    if (ioctl(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
        SERR("failed");
        free(exp_gain.reg_setting);
        return -EIO;
    }
  }
  free(exp_gain.reg_setting);

  ctrl->s_data->stored_gain = ctrl->s_data->current_gain;
  ctrl->s_data->prev_gain = gain_apply;
  ctrl->s_data->prev_linecount = ctrl->s_data->current_linecount;

  return rc;
}

/*===========================================================================
 * FUNCTION    - sensor_set_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_set_exposure(void *sctrl, sensor_exp_t exposure)
{
  if (!sctrl ) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  sensor_exposure_info_t exp_info;
  SLOW("real_gain=%f line_count=%d",
    exposure.real_gain, exposure.linecount);
  lib->sensor_lib_ptr->exposure_func_table->sensor_calculate_exposure(
     exposure.real_gain, exposure.linecount, &exp_info);

  ctrl->s_data->current_gain = exp_info.reg_gain;
  ctrl->s_data->sensor_real_gain = exp_info.sensor_real_gain;
  ctrl->s_data->sensor_digital_gain = exp_info.sensor_digital_gain;
  ctrl->s_data->current_linecount = exposure.linecount;
  ctrl->s_data->digital_gain = exp_info.digital_gain;
  ctrl->s_data->current_luma_hdr = exposure.luma_hdr;
  ctrl->s_data->current_fgain_hdr = exposure.fgain_hdr;

  SLOW("reg_gain=%d, line_count=%d, dig_gain=%f",
        exp_info.reg_gain, exp_info.line_count, exp_info.digital_gain);
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_awb_video_hdr_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_set_awb_video_hdr_update(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  awb_update_t* awb_hdr_update = (awb_update_t*) data;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  struct sensorb_cfg_data cfg;
  struct msm_camera_i2c_seq_reg_setting awb_hdr;

  if (!lib || !lib->sensor_lib_ptr) {
    SERR("failed: invalid sensor lib ptr");
    return SENSOR_FAILURE;
  }

  if (!lib->sensor_lib_ptr->video_hdr_awb_lsc_func_table) {
    return SENSOR_SUCCESS;
  }

  /*convert AWB gains to sensor register format (u5.8 fixed point)*/
  uint16_t awb_gain_r_reg = (uint16_t)(awb_hdr_update->gain.r_gain * 256.0);
  uint16_t awb_gain_b_reg = (uint16_t)(awb_hdr_update->gain.b_gain * 256.0);
  SLOW("HDR--awb_gain_r_reg: %d, awb_gain_b_reg: %d",
    awb_gain_r_reg, awb_gain_b_reg);
  awb_hdr.reg_setting = malloc(lib->sensor_lib_ptr->
    video_hdr_awb_lsc_func_table->awb_table_size *
    sizeof(struct msm_camera_i2c_seq_reg_array));
  if (!awb_hdr.reg_setting) {
    SERR("failed");
    return -ENOMEM;
  }

  rc = lib->sensor_lib_ptr->video_hdr_awb_lsc_func_table->
    sensor_fill_awb_array(awb_gain_r_reg,
    awb_gain_b_reg, &awb_hdr);

  if (rc < 0) {
    SERR("failed");
  } else {
    cfg.cfgtype = CFG_WRITE_I2C_SEQ_ARRAY;
    cfg.cfg.setting = &awb_hdr;
    if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
        SERR("failed");
        free(awb_hdr.reg_setting);
        return -EIO;
    }
  }
  free(awb_hdr.reg_setting);
  /*group hold off */

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_start_stream -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_set_start_stream(void *sctrl)
{
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  SLOW("enter");

  if (ctrl->s_data->fd < 0)
    return FALSE;

  if (lib->sensor_lib_ptr->sensor_output->output_format != SENSOR_YCBCR) {
    cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
    cfg.cfg.setting = ctrl->lib_params->sensor_lib_ptr->start_settings;
  } else {
    cfg.cfgtype = CFG_SET_START_STREAM;
  }
  if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
    SLOW("failed");
    return 0;
  }
  SLOW("exit");
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - sensor_set_stop_stream -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_set_stop_stream(void *sctrl)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  struct sensorb_cfg_data cfg;
  SLOW("enter");

  if (ctrl->s_data->fd < 0)
    return FALSE;

  if (lib->sensor_lib_ptr->sensor_output->output_format != SENSOR_YCBCR) {
    cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
    cfg.cfg.setting = ctrl->lib_params->sensor_lib_ptr->stop_settings;
  } else {
    cfg.cfgtype = CFG_SET_STOP_STREAM;
  }
  if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
    SLOW("failed");
    return 0;
  }
  SLOW("exit");
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - sensor_get_csi_lane_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_get_csi_lane_params(void *sctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  sensor_get_t *sensor_get = (sensor_get_t *)data;
  SLOW("enter");
  if (!data) {
    SERR("invalid params %p or %p", ctrl, data);
    return SENSOR_FAILURE;
  }
  sensor_get->csi_lane_params = lib->sensor_lib_ptr->csi_lane_params;
  SLOW("csi lane params lane assign %x mask %x if %d csid %d phy %d",
    sensor_get->csi_lane_params->csi_lane_assign,
    sensor_get->csi_lane_params->csi_lane_mask,
    sensor_get->csi_lane_params->csi_if,
    sensor_get->csi_lane_params->csid_core[0],
    sensor_get->csi_lane_params->csi_phy_sel);
  SLOW("exit");
  return rc;
}



static int8_t sensor_set_wait_frames(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  uint16_t *wait_frame_count = (uint16_t *)data;
  SLOW("enter");
  if (!ctrl || !wait_frame_count) {
    SERR("Invalid params %p or %p", ctrl, wait_frame_count);
    return FALSE;
  }
  ctrl->s_data->wait_frame_count = *wait_frame_count;
  SLOW("digital gain %d", *wait_frame_count);
  SLOW("exit");
  return TRUE;
}

static int8_t sensor_get_wait_frames(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  uint16_t *wait_frame_count = (uint16_t *)data;
  SLOW("enter");
  if (!ctrl || !wait_frame_count) {
    SERR("Invalid params %p or %p", ctrl, wait_frame_count);
    return FALSE;
  }
  *wait_frame_count = ctrl->s_data->wait_frame_count;
  SLOW("digital gain %d", *wait_frame_count);
  SLOW("exit");
  return TRUE;
}

static int8_t sensor_get_actuator_name(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  char **name = (char**) data;
  SLOW("enter");
  if (!ctrl || !data) {
    SERR("failed, %p, %p", ctrl, data);
    return FALSE;
  } else if (!ctrl->lib_params ) {
     SERR("failed, %p", ctrl->lib_params);
     return FALSE;
  } else if (!ctrl->lib_params->sensor_lib_ptr) {
     SERR("failed, %p", ctrl->lib_params->sensor_lib_ptr);
     return FALSE;
  }
  *name = ctrl->lib_params->sensor_lib_ptr->actuator_name;
  SLOW("exit");
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - sensor_get_digital_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t sensor_get_digital_gain(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  /*sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;*/
  float *digital_gain= (float *)data;
  SLOW("enter");
  if (!ctrl || !digital_gain) {
    SERR("Invalid params %p or %p", ctrl, digital_gain);
    return FALSE;
  }
  *digital_gain = ctrl->s_data->digital_gain;
  SLOW("digital gain %f", *digital_gain);
  SLOW("exit");
  return TRUE;
}

/** sensor_get_sensor_format: Get sensor format
 *
 *  @sctrl: sensor control structure
 *  @data: pointer to
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function returns sensor format **/

static int8_t sensor_get_sensor_format(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = NULL;
  sensor_output_format_t *output_format = (sensor_output_format_t *)data;
  SLOW("enter");
  if (!ctrl || !output_format) {
    SERR("Invalid params %p or %p", ctrl, output_format);
    return FALSE;
  }
  lib = (sensor_lib_params_t *)ctrl->lib_params;
  if (!lib || !lib->sensor_lib_ptr || !lib->sensor_lib_ptr->sensor_output) {
    SERR("Invalid params : %p = ctrl->lib_params", lib);
    return FALSE;
  }

  *output_format = lib->sensor_lib_ptr->sensor_output->output_format;
  SLOW("output_format %d", *output_format);
  SLOW("exit");
  return TRUE;
}

/** sensor_get_lens_info: Get actuator information
 *
 *  @sctrl: sensor control structure
 *  @data: pointer to sensor_lens_info_t
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function returns lens info **/

static int8_t sensor_get_lens_info(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = NULL;
  sensor_lens_info_t *lens_info = (sensor_lens_info_t *)data;
  SLOW("enter");
  if (!ctrl || !lens_info) {
    SERR("Invalid params %p or %p", ctrl, lens_info);
    return FALSE;
  }
  lib = (sensor_lib_params_t *)ctrl->lib_params;
  if (!lib || !lib->sensor_lib_ptr || !lib->sensor_lib_ptr->default_lens_info) {
    SERR("Invalid params");
    return FALSE;
  }

  *lens_info = *lib->sensor_lib_ptr->default_lens_info;
  SLOW("aperture %f", lens_info->f_number);
  SLOW("exit");
  return TRUE;
}

/*==========================================================
 * FUNCTION    - sensor_get_cur_csiphy_cfg -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_cur_csiphy_cfg(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_get_t  *sensor_get = (sensor_get_t *)data;
  if (!data) {
    SERR("data NULL");
    return SENSOR_FAILURE;
  }
  sensor_get->csiphy_params =
    &ctrl->lib_params->sensor_lib_ptr->
    csi_params_array->csi2_params[ctrl->s_data->cur_res]->csiphy_params;
  SLOW("csiphy params lane cnt %d settle cnt %x lane mask %d combo %d",
    sensor_get->csiphy_params->lane_cnt,
    sensor_get->csiphy_params->settle_cnt, sensor_get->csiphy_params->lane_mask,
    sensor_get->csiphy_params->combo_mode);
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_get_cur_csid_cfg -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_cur_csid_cfg(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_get_t  *sensor_get = (sensor_get_t *)data;
  uint32_t i = 0;
  if (!data) {
    SERR("data NULL");
    return SENSOR_FAILURE;
  }
  sensor_get->csid_params =
    &ctrl->lib_params->sensor_lib_ptr->
    csi_params_array->csi2_params[ctrl->s_data->cur_res]->csid_params;

  SLOW("csid params lane cnt %d lane assign %x phy sel %d",
    sensor_get->csid_params->lane_cnt,
    sensor_get->csid_params->lane_assign, sensor_get->csid_params->phy_sel);
  for (i = 0; i < sensor_get->csid_params->lut_params.num_cid; i++) {
    SLOW("lut[%d] cid %d dt %x decode format %x", i,
      sensor_get->csid_params->lut_params.vc_cfg[i]->cid,
      sensor_get->csid_params->lut_params.vc_cfg[i]->dt,
      sensor_get->csid_params->lut_params.vc_cfg[i]->decode_format);
  }
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_get_csi_clk_scale_cfg -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_csi_clk_scale_cfg(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct msm_camera_csid_params   *csid_params;
  uint32_t  *csi_clk = (uint32_t *) data;
  uint32_t  temp_csi_clk;
  uint32_t  op_pixel_clk;
  uint32_t  bits_per_pixel;
  if (!sctrl) {
    SERR("sctrl NULL");
    return SENSOR_FAILURE;
  }

  if (!data) {
    SERR("data NULL");
    return SENSOR_FAILURE;
  }
  csid_params = &ctrl->lib_params->sensor_lib_ptr->
    csi_params_array->csi2_params[ctrl->s_data->cur_res]->csid_params;

  switch(csid_params->lut_params.vc_cfg[0]->dt){
    case CSI_RAW10:
      bits_per_pixel = 10;
      break;
    case CSI_RAW8:
      bits_per_pixel = 8;
      break;
    case CSI_RAW12:
      bits_per_pixel = 12;
      break;
    case CSI_YUV422_8:
      bits_per_pixel = 8;
      break;
    default :
      bits_per_pixel = 10;
      SHIGH("default bits per pixel = %u", bits_per_pixel);
  }
  op_pixel_clk = ctrl->lib_params->sensor_lib_ptr->
    out_info_array->out_info[ctrl->s_data->cur_res].op_pixel_clk;

  temp_csi_clk = ((op_pixel_clk / (8 * csid_params->lane_cnt))
    * bits_per_pixel);

  /*adding 10% buffer*/
  *csi_clk = temp_csi_clk + temp_csi_clk/10;

  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_open -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_open(void **sctrl, const char *subdev_name)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = NULL;
  char subdev_string[32];

  if (!sctrl || !subdev_name) {
    SERR("failed sctrl %p subdev name %p",
      sctrl, subdev_name);
    return SENSOR_ERROR_INVAL;
  }
  ctrl = malloc(sizeof(sensor_ctrl_t));
  if (!ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  memset(ctrl, 0, sizeof(sensor_ctrl_t));

  ctrl->s_data = malloc(sizeof(sensor_data_t));
  if (!ctrl->s_data) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR2;
  }
  memset(ctrl->s_data, 0, sizeof(sensor_data_t));

  /* Set default fps value */
  ctrl->s_data->cur_fps = 30.0;

  /* Initialize mutex */
  pthread_mutex_init(&ctrl->s_data->mutex, NULL);

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s", subdev_name);
  /* Open subdev */
  ctrl->s_data->fd = open(subdev_string, O_RDWR);
  if (ctrl->s_data->fd < 0) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR2;
  }
   ctrl->session_count = 0;
  *sctrl = (void *)ctrl;
  SLOW("ctrl %p", ctrl);
  return rc;

ERROR1:
  free(ctrl->s_data);
ERROR2:
  free(ctrl);
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_lib_params -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_lib_params(void *sctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  if (!data) {
    SERR("data NULL");
    return SENSOR_FAILURE;
  }
  ctrl->lib_params = (sensor_lib_params_t *)data;
  return rc;
}

/** sensor_set_mount_angle: sensor set mount angle
 *
 *  @sctrl: handle to sensor control structure
 *  @data: handle to sensor mount angle
 *
 *  This function sets mount angle in sensor control structure
 *
 *  Return: 0 for success and negative error for failure
 **/
static int32_t sensor_set_mount_angle(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  uint32_t      *mount_angle = (uint32_t *)data;

  if (!ctrl || !mount_angle) {
    SERR("failed: invalid params %p %p", ctrl, mount_angle);
    return SENSOR_FAILURE;
  }

  if (!ctrl->lib_params || !ctrl->lib_params->sensor_lib_ptr ||
      !ctrl->lib_params->sensor_lib_ptr->sensor_init_params) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  if( ctrl->lib_params->sensor_lib_ptr->sensor_init_params->sensor_mount_angle
    >= SENSOR_MOUNTANGLE_360 ) {
  SHIGH("UserSpace mount angle invalid using DT mountangle");
  ctrl->lib_params->sensor_lib_ptr->sensor_init_params->sensor_mount_angle =
    *mount_angle;
  }

  return SENSOR_SUCCESS;
}

/** sensor_set_position: sensor set position
 *
 *  @sctrl: handle to sensor control structure
 *  @data: handle to sensor position
 *
 *  This function sets mount angle in sensor control structure
 *
 *  Return: 0 for success and negative error for failure
 **/
static int32_t sensor_set_position(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  uint32_t      *position = (uint32_t *)data;

  if (!ctrl || !position) {
    SERR("failed: invalid params %p %p", ctrl, position);
    return SENSOR_FAILURE;
  }

  if (!ctrl->lib_params || !ctrl->lib_params->sensor_lib_ptr ||
      !ctrl->lib_params->sensor_lib_ptr->sensor_init_params) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  ctrl->lib_params->sensor_lib_ptr->sensor_init_params->position =
    *position;
  SHIGH("Position ID: %d",*position);
  return SENSOR_SUCCESS;
}

/** sensor_set_mode: sensor set mode
 *
 *  @sctrl: handle to sensor control structure
 *  @data: handle to sensor mode
 *
 *  This function sets sensor mode in sensor control structure
 *
 *  Return: 0 for success and negative error for failure
 **/
static int32_t sensor_set_mode(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  uint32_t      *sensor_mode = (uint32_t *)data;

  if (!ctrl || !sensor_mode) {
    SERR("failed: invalid params %p %p", ctrl, sensor_mode);
    return SENSOR_FAILURE;
  }

  if (!ctrl->lib_params || !ctrl->lib_params->sensor_lib_ptr ||
      !ctrl->lib_params->sensor_lib_ptr->sensor_init_params) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  ctrl->lib_params->sensor_lib_ptr->sensor_init_params->modes_supported =
    *sensor_mode;
  SHIGH("Sensor Mode ID: %d",*sensor_mode);
  return SENSOR_SUCCESS;
}

/** sensor_pick_resolution: get raw dimension for sensor
 *
 *  @sctrl: handle to sensor control structure
 *  @res_cfg: handle to sensor_set_res_cfg_t
 *  @pick_res: handle to enum msm_sensor_resolution_t
 *
 *  This function stores picks proper resolution based on mode
 *  and requested dimension
 *
 *  Return: SENSOR_SUCCESS for success
 *          Negative error for failure
 **/
static int32_t sensor_pick_resolution(void *sctrl,
  sensor_set_res_cfg_t *res_cfg, enum msm_sensor_resolution_t *pick_res)
{
  int32_t                           rc = SENSOR_SUCCESS, i = 0;
  sensor_ctrl_t                    *ctrl = (sensor_ctrl_t *)sctrl;
  uint32_t                          vfe_pixel_clk_max = 0;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  uint16_t                          width = 0, height = 0;
  uint32_t                          asp_ratio_4_3 = (4 * QFACTOR) / 3;
  uint32_t                          asp_ratio_16_9 = (16 * QFACTOR) / 9;
  uint32_t                          req_asp_ratio = 0.0f;
  uint32_t                          asp_ratio = 0.0f;
  uint32_t                          asp_ratio_last = 1000.0f;
  uint32_t                          sup_asp_ratio = 0.0f;

  SHIGH("Enter");
  if (!sctrl || !res_cfg || !pick_res) {
    SERR("failed: invalid params %p %p %p", sctrl, res_cfg, pick_res);
    return SENSOR_FAILURE;
  }

  if (!ctrl->s_data) {
    SERR("failed: invalid params ctrl->s_data %p", ctrl->s_data);
    return SENSOR_FAILURE;
  }

  if (!ctrl->lib_params || !ctrl->lib_params->sensor_lib_ptr) {
    SERR("falied: invalid lib params");
    return SENSOR_FAILURE;
  }

  width = res_cfg->width;
  height = res_cfg->height;

  SHIGH("Height %d Width %d", height, width);

  PTHREAD_MUTEX_LOCK(&ctrl->s_data->mutex);

  vfe_pixel_clk_max = ctrl->s_data->isp_pixel_clk_max;

  PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);

  if (res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_RAW)) {
    /* Restrict VFE pixel clk to max of one VFE */
    vfe_pixel_clk_max = MAX_VFE_CLK;
  }

  /* Calculate requested aspect ratio */
  if (height > 0) {
    req_asp_ratio = (width * QFACTOR) / height;
  }
  SHIGH("req_asp_ratio %d stream type %x", req_asp_ratio, res_cfg->stream_mask);

  out_info_array = ctrl->lib_params->sensor_lib_ptr->out_info_array;

  if ((ctrl->s_data->hfr_mode != CAM_HFR_MODE_OFF) &&
      (res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO))) {

    /* In case of HFR mode */
    for (i = 0; i < out_info_array->size; i++) {

      float req_fps = sensor_get_hfr_mode_fps(ctrl->s_data->hfr_mode);
      SLOW("hfr i %d x_output %d y_output %d max  fps %f req fps %f", i,
        out_info_array->out_info[i].x_output,
        out_info_array->out_info[i].y_output,
        out_info_array->out_info[i].max_fps,
        req_fps);

      if (vfe_pixel_clk_max == 0) {
        if (out_info_array->out_info[i].y_output >= height &&
          out_info_array->out_info[i].max_fps >= req_fps) {
          *pick_res = i;
          ctrl->s_data->cur_fps = out_info_array->out_info[i].max_fps;
          SLOW("HFR mode: %d, resolution picked :%d",
                          ctrl->s_data->hfr_mode, *pick_res);
          break;
        }
      } else {
        if (out_info_array->out_info[i].y_output >= height &&
          out_info_array->out_info[i].max_fps >= req_fps &&
          out_info_array->out_info[i].op_pixel_clk <= vfe_pixel_clk_max) {
          *pick_res = i;
          ctrl->s_data->cur_fps = out_info_array->out_info[i].max_fps;
          SLOW("HFR mode: %d, resolution picked :%d",
                          ctrl->s_data->hfr_mode, *pick_res);
          break;
        }
      }
    }

  } else if ((ctrl->s_data->video_hdr_enable) &&
      (res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW) ||
      res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO))) {

    /* In case of HDR mode */
    for (i = 0; i < out_info_array->size; i++) {

      SLOW("hdr i %d x_output %d y_output %d max fps %f", i,
        out_info_array->out_info[i].x_output,
        out_info_array->out_info[i].y_output,
        out_info_array->out_info[i].max_fps);

        if (out_info_array->out_info[i].mode == SENSOR_HDR_MODE &&
          out_info_array->out_info[i].max_fps + MAX_FPS_VARIANCE >=
          ctrl->s_data->cur_fps &&
          out_info_array->out_info[i].x_output >= width) {
          *pick_res = i;
          ctrl->s_data->cur_fps = out_info_array->out_info[i].max_fps;

          break;
        }

      if ((*pick_res >= out_info_array->size) ||
          (*pick_res == MSM_SENSOR_INVALID_RES)) {
        *pick_res = out_info_array->size - 1;
      }
    }
  } else if ((ctrl->s_data->snapshot_hdr_enable) &&
          (res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW) ||
           res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT))) {
        /* In case of snapshot HDR mode */
        for (i = 0; i < out_info_array->size; i++) {

          SLOW("snapshot hdr i %d x_output %d y_output %d max fps %f", i,
            out_info_array->out_info[i].x_output,
            out_info_array->out_info[i].y_output,
            out_info_array->out_info[i].max_fps);

            if (out_info_array->out_info[i].mode == SENSOR_HDR_MODE &&
              out_info_array->out_info[i].max_fps + MAX_FPS_VARIANCE  >=
              ctrl->s_data->cur_fps &&
              out_info_array->out_info[i].x_output >= width) {
              *pick_res = i;
              ctrl->s_data->cur_fps = out_info_array->out_info[i].max_fps;
              break;
            }

      if ((*pick_res >= out_info_array->size) ||
          (*pick_res == MSM_SENSOR_INVALID_RES)) {
        *pick_res = out_info_array->size - 1;
      }
    }
  } else {

    /* In case of normal preview/video mode */
    for (i = 0; i < out_info_array->size; i++) {

      SLOW("preview i %d x_output %d y_output %d max fps %f", i,
        out_info_array->out_info[i].x_output,
        out_info_array->out_info[i].y_output,
        out_info_array->out_info[i].max_fps);

      if (out_info_array->out_info[i].x_output >= width &&
          out_info_array->out_info[i].y_output >= height &&
          SENSOR_DEFAULT_MODE == out_info_array->out_info[i].mode) {

        if (out_info_array->out_info[i].y_output > 0) {

          sup_asp_ratio = (out_info_array->out_info[i].x_output * QFACTOR) /
            out_info_array->out_info[i].y_output;

        } else {
          sup_asp_ratio = 0;
        }

        SHIGH("req_asp_ratio %d sup_asp_ratio %d", req_asp_ratio,
          sup_asp_ratio);

        asp_ratio = fabs((int32_t)req_asp_ratio - (int32_t)sup_asp_ratio);

        SHIGH("fabs %d", asp_ratio);

        if (vfe_pixel_clk_max == 0) {
          if ((req_asp_ratio > 0) && (sup_asp_ratio > 0)) {
            if ((out_info_array->out_info[i].max_fps + MAX_FPS_VARIANCE >=
              ctrl->s_data->cur_fps) && (asp_ratio <= asp_ratio_last)) {
              *pick_res = i;
              SHIGH("pick res %d", *pick_res);
              asp_ratio_last = asp_ratio;
              if (!asp_ratio)
                break;
            }
          } else {
            if ((out_info_array->out_info[i].max_fps + MAX_FPS_VARIANCE >=
              ctrl->s_data->cur_fps)) {
              *pick_res = i;
              SHIGH("pick res %d", *pick_res);
              break;
            }
          }
        } else {

          if ((req_asp_ratio > 0) && (sup_asp_ratio > 0)) {
            if ((out_info_array->out_info[i].max_fps + MAX_FPS_VARIANCE >=
              ctrl->s_data->cur_fps) && (asp_ratio <= asp_ratio_last) &&
              (out_info_array->out_info[i].op_pixel_clk <= vfe_pixel_clk_max)) {
              *pick_res = i;
              SHIGH("pick res %d", *pick_res);
              asp_ratio_last = asp_ratio;
              if (!asp_ratio)
                break;
            }
          } else {
            if ((out_info_array->out_info[i].max_fps + MAX_FPS_VARIANCE >=
              ctrl->s_data->cur_fps) &&
              (out_info_array->out_info[i].op_pixel_clk <= vfe_pixel_clk_max)) {
              *pick_res = i;
              SHIGH("pick res %d", *pick_res);
              break;
            }
          }
        }
      }
    }
  }

  SHIGH("Exit");
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_set_resolution -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_resolution(void *sctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS, i = 0;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_set_res_cfg_t *res_cfg = (sensor_set_res_cfg_t *)data;
  uint16_t width = 0, height = 0;
  float fps = 0;
  enum msm_sensor_resolution_t res = MSM_SENSOR_INVALID_RES;
  enum msm_camera_stream_type_t stream_type = MSM_CAMERA_STREAM_INVALID;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  struct msm_camera_i2c_reg_setting *reg_settings = NULL;
  struct msm_camera_i2c_reg_array *regs = NULL;
  struct sensor_lib_out_info_t *dimension = NULL;
  struct sensorb_cfg_data cfg;
  SHIGH("Enter");
  if (!res_cfg) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  width = res_cfg->width;
  height = res_cfg->height;
  ctrl->s_data->cur_fps = ctrl->s_data->max_fps;
  out_info_array = ctrl->lib_params->sensor_lib_ptr->out_info_array;

  if (!width || !height) {
    SERR("failed: width %d height %d", width, height);
    return SENSOR_FAILURE;
  }

#ifdef ENABLE_DIS_MARGIN
  if ((res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)) &&
      (ctrl->s_data->dis_enable != 0)) {
    /* Add 10% for DIS */
    width = (width * 11) / 10;
    height = (height * 11) / 10;
  }
#endif

  SHIGH("Requested width %d, height %d", width, height);
  SHIGH("Requested stream mask %x hfr mode %d Cur fps %f", res_cfg->stream_mask,
    ctrl->s_data->hfr_mode, ctrl->s_data->cur_fps);

  rc = sensor_pick_resolution(sctrl, res_cfg, &res);
  if (rc < 0) {
    SERR("failed: sensor_pick_resolution rc %d", rc);
    return rc;
  }

  SHIGH("Curr Res ID %d New Res ID %d New FPS %f", ctrl->s_data->cur_res, res,
    ctrl->s_data->cur_fps);
  if ((res >= out_info_array->size) || (res == MSM_SENSOR_INVALID_RES)) {
    if (out_info_array->out_info[MSM_SENSOR_RES_FULL].x_output >= width &&
        out_info_array->out_info[MSM_SENSOR_RES_FULL].y_output >= height) {
      res = MSM_SENSOR_RES_FULL;
      SHIGH("Hardcode res %d", res);
    } else {
      SERR("Error: failed to find resolution requested w*h %d*%d fps %f",
        width, height, ctrl->s_data->cur_fps);
      return SENSOR_FAILURE;
    }
  }

  if (lib->sensor_lib_ptr->sensor_output->output_format == SENSOR_YCBCR) {
    if (res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)) {
        stream_type = MSM_CAMERA_STREAM_VIDEO;
    } else if (res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW)) {
        stream_type = MSM_CAMERA_STREAM_PREVIEW;
    } else if (res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT)) {
        stream_type = MSM_CAMERA_STREAM_SNAPSHOT;
    }
    SHIGH("stream_type set = %d", stream_type);
    cfg.cfgtype = CFG_SET_STREAM_TYPE;
    cfg.cfg.setting = &stream_type;
    if (ioctl(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
      /* Ignore return value for backward compatibility */
      SHIGH("Set stream_type to sensor is either unsupported or unsuccessful");
    }
  }

  ctrl->s_data->prev_gain = 0;
  ctrl->s_data->prev_linecount = 0;
  ctrl->s_data->current_gain = 0;
  ctrl->s_data->current_linecount = 0;
  ctrl->s_data->sensor_skip_counter = 0;

  if (ctrl->s_data->cur_res == res) {
    ctrl->s_data->cur_stream_mask = res_cfg->stream_mask;
    if (ctrl->s_data->cur_fps > out_info_array->out_info[res].max_fps) {
      SERR("set_fps=%f > max_fps=%f, capping to max",
        ctrl->s_data->cur_fps, out_info_array->out_info[res].max_fps);
      ctrl->s_data->cur_fps = out_info_array->out_info[res].max_fps;
      /* not returning failure here */
    }
    SLOW("same resolution, returning..");
    return SENSOR_SUCCESS;
  }
  if (lib->sensor_lib_ptr->sensor_output->output_format != SENSOR_YCBCR) {
    if (res > lib->sensor_lib_ptr->res_settings_array->size) {
      SERR("failed res %d max size %d",
        res, lib->sensor_lib_ptr->res_settings_array->size);
      return SENSOR_FAILURE;
    }
    reg_settings = &lib->sensor_lib_ptr->res_settings_array->reg_settings[res];
    regs = reg_settings->reg_setting;
    ctrl->s_data->cur_frame_length_lines =
      out_info_array->out_info[res].frame_length_lines;
    ctrl->s_data->cur_line_length_pclk =
      out_info_array->out_info[res].line_length_pclk;
    /* Update fps divider if cur stream type is preview or video */
    if ((res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW) ||
      res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)) &&
     (lib->sensor_lib_ptr->sensor_output->output_format != SENSOR_YCBCR) &&
      (ctrl->s_data->cur_fps > 0)) {
      if (ctrl->s_data->cur_fps > out_info_array->out_info[res].max_fps) {
        SERR("set_fps=%f > max_fps=%f, capping to max",
                    ctrl->s_data->cur_fps,
                    out_info_array->out_info[res].max_fps);
        ctrl->s_data->cur_fps = out_info_array->out_info[res].max_fps;
        /* not returning failure here */
      }
      if (ctrl->s_data->hfr_mode != CAM_HFR_MODE_OFF) {
        ctrl->s_data->current_fps_div = Q10;
      } else {
        ctrl->s_data->current_fps_div =
          (out_info_array->out_info[res].max_fps * Q10) / ctrl->s_data->cur_fps;
      }
    } else {
      ctrl->s_data->current_fps_div = Q10;
    }
    SLOW("reg array size %d", reg_settings->size);
    /*write single array for that mode*/
    for (i = 0; i < reg_settings->size; i++) {
      SLOW("addr %x data %x",
        regs[i].reg_addr, regs[i].reg_data);
    }

    cfg.cfg.setting = reg_settings;
    cfg.cfgtype = CFG_WRITE_I2C_ARRAY;

    rc = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
    if (rc < 0) {
      SERR("failed");
      return rc;
     }

    if (res > lib->sensor_lib_ptr->out_info_array->size) {
      SERR("failed res %d max size %d",
        res, lib->sensor_lib_ptr->out_info_array->size);
      return SENSOR_FAILURE;
    }
    dimension = &lib->sensor_lib_ptr->out_info_array->out_info[res];
    SLOW("res x %d y %d llpclk %d fl %d",
      dimension->x_output, dimension->y_output, dimension->line_length_pclk,
      dimension->frame_length_lines);
    struct msm_camera_i2c_reg_array reg_array[] = {
      {lib->sensor_lib_ptr->output_reg_addr->x_output,
        dimension->x_output},
      {lib->sensor_lib_ptr->output_reg_addr->y_output,
        dimension->y_output},
      {lib->sensor_lib_ptr->output_reg_addr->line_length_pclk,
        dimension->line_length_pclk},
      {lib->sensor_lib_ptr->output_reg_addr->frame_length_lines,
        ((dimension->frame_length_lines * ctrl->s_data->current_fps_div)
        / Q10)},
    };

    struct msm_camera_i2c_reg_setting out_settings = {
      reg_array,
      4,
      MSM_CAMERA_I2C_WORD_ADDR,
      MSM_CAMERA_I2C_WORD_DATA,
      0
    };

    cfg.cfg.setting = &out_settings;
    rc = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
    if (rc < 0) {
      SERR("failed");
      return rc;
    }
  } else {
    cfg.cfgtype = CFG_SET_RESOLUTION;
    cfg.cfg.setting = &res;
    rc = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
    if (rc < 0) {
      SERR("failed");
      return rc;
    }
  }
  SLOW("Done mode change");
  ctrl->s_data->prev_res = ctrl->s_data->cur_res;
  ctrl->s_data->cur_res = res;
  ctrl->s_data->cur_stream_mask = res_cfg->stream_mask;
  SHIGH("Exit");
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_saturation -
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_saturation(void * sctrl , void *data)
{
   int32_t ret = 0;
   //struct msm_camera_i2c_reg_array sat_config;
   struct sensorb_cfg_data cfg;
   sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
   int32_t *saturation_level = (int32_t*)data;
   SHIGH("%s: SATURATION VALUE %d ",__func__,*saturation_level);
   cfg.cfg.setting = saturation_level;
   cfg.cfgtype = CFG_SET_SATURATION;

   ret = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
    if (ret < 0) {
      SERR("failed");
      return ret;
     }
   return ret;
}

/*==========================================================
 * FUNCTION    - sensor_set_sharpness -
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_sharpness(void * sctrl , void *data)
{
   int32_t ret = 0;
   //struct msm_camera_i2c_reg_array sat_config;
   struct sensorb_cfg_data cfg;
   sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
   int32_t *sharpness_level = (int32_t*)data;
   SHIGH("%s: SHARPNESS VALUE %d ",__func__,*sharpness_level);
   cfg.cfg.setting = sharpness_level;
   cfg.cfgtype = CFG_SET_SHARPNESS;

   ret = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
    if (ret < 0) {
      SERR("failed");
      return ret;
     }
   return ret;
}

/*==========================================================
 * FUNCTION    - sensor_set_contrast
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_contrast(void * sctrl , void *data)
{
  int32_t ret = 0;
  //struct msm_camera_i2c_reg_array sat_config;
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *contrast_level = (int32_t*)data;
  SHIGH("%s: CONTRAST VALUE %d ",__func__,*contrast_level);
  cfg.cfg.setting = contrast_level;
  cfg.cfgtype = CFG_SET_CONTRAST;

  ret = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
  if (ret < 0) {
    SERR("failed");
    return ret;
  }
  return ret;
}

/*==========================================================
 * FUNCTION    - sensor_set_autofocus
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_autofocus(void * sctrl , void *data)
{
  int32_t ret = 0;
  // sensor_af_thread_t af_status_thread;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
   sensor_info_t * sensor_info = (sensor_info_t*)data;
   module_sensor_ctrl_t        *module_ctrl = NULL;
   module_ctrl = (module_sensor_ctrl_t *)sensor_info->module->module_private;
  if (!module_ctrl) {
    SERR("failed");
    return FALSE;
  }
   //struct msm_camera_i2c_reg_array sat_config;
   struct sensorb_cfg_data cfg;
   //af_status_thread.is_thread_started = FALSE;
   cfg.cfgtype = CFG_SET_AUTOFOCUS;
   ret = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
    if (ret < 0) {
      SERR("failed");
      //return ret;
   }
   sensor_thread_msg_t msg;
   msg.msgtype = SET_AUTOFOCUS;
   msg.stop_thread = FALSE;
   msg.fd = ctrl->s_data->fd;
   msg.module = sensor_info->module;
   msg.sessionid = sensor_info->session_id;
   int nwrite = 0;
   ALOGE("%s write fd %d", __func__, module_ctrl->pfd[1]);
   nwrite = write(module_ctrl->pfd[1], &msg, sizeof(sensor_thread_msg_t));
   if(nwrite < 0)
   {
     SERR("%s: Writing into fd failed",__func__);
   }

  return ret;
}
/*==========================================================
 * FUNCTION    - sensor_cancel_autofocus
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_cancel_autofocus(void * sctrl , void *data)
{
   int32_t ret = 0;
   sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
   sensor_info_t * sensor_info = (sensor_info_t*)data;
   struct sensorb_cfg_data cfg;
   cfg.cfgtype = CFG_CANCEL_AUTOFOCUS;
   ret = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
    if (ret < 0) {
      SERR("failed");
   }
   return ret;
}

/*==========================================================
 * FUNCTION    - sensor_set_iso
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_iso(void * sctrl , void *data)
{
  int32_t ret = 0;
  //struct msm_camera_i2c_reg_array sat_config;
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *iso_level = (int32_t*)data;
  SHIGH("%s: ISO VALUE %d ",__func__,*iso_level);
  cfg.cfg.setting = iso_level;
  cfg.cfgtype = CFG_SET_ISO;

  ret = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
  if (ret < 0) {
    SERR("failed");
    return ret;
  }
  return ret;
}

/*==========================================================
 * FUNCTION    - sensor_set_exposure_compensation
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_exposure_compensation(void * sctrl , void *data)
{
  int32_t ret = 0;
  //struct msm_camera_i2c_reg_array sat_config;
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *exposure_comp_level = (int32_t*)data;
  SHIGH("%s: exposure compensation VALUE %d ",__func__,*exposure_comp_level);
  cfg.cfg.setting = exposure_comp_level;
  cfg.cfgtype = CFG_SET_EXPOSURE_COMPENSATION;

  ret = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
  if (ret < 0) {
    SERR("failed");
    return ret;
  }
  return ret;
}

/*==========================================================
 * FUNCTION    - sensor_set_antibanding
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_antibanding(void * sctrl , void *data)
{
  int32_t ret = 0;
  //struct msm_camera_i2c_reg_array sat_config;
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *antibanding_level = (int32_t*)data;
  SHIGH("%s: antibanding VALUE %d ",__func__,*antibanding_level);
  cfg.cfg.setting = antibanding_level;
  cfg.cfgtype = CFG_SET_ANTIBANDING;

  ret = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
  if (ret < 0) {
    SERR("failed");
    return ret;
  }
  return ret;
}

/*==========================================================
 * FUNCTION    - sensor_set_bestshot_mode
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_bestshot_mode(void * sctrl , void *data)
{
  int32_t ret = 0;
  //struct msm_camera_i2c_reg_array sat_config;
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *bestshot_mode = (int32_t*)data;
  SHIGH("%s: bestshot_mode VALUE %d ",__func__,*bestshot_mode);
  cfg.cfg.setting = bestshot_mode;
  cfg.cfgtype = CFG_SET_BESTSHOT_MODE;

  ret = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
  if (ret < 0) {
    SERR("failed");
    return ret;
  }
  return ret;
}

/*==========================================================
 * FUNCTION    - sensor_set_effect
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_effect(void * sctrl , void *data)
{
  int32_t ret = 0;
  //struct msm_camera_i2c_reg_array sat_config;
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *effect_mode = (int32_t*)data;
  SHIGH("%s: effect mode %d ",__func__,*effect_mode);
  cfg.cfg.setting = effect_mode;
  cfg.cfgtype = CFG_SET_EFFECT;

  ret = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
  if (ret < 0) {
    SERR("failed");
    return ret;
  }
  return ret;
}

/*==========================================================
 * FUNCTION    - sensor_set_white_balance
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_white_balance(void * sctrl , void *data)
{
  int32_t ret = 0;
  //struct msm_camera_i2c_reg_array sat_config;
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *wb_mode = (int32_t*)data;
  SHIGH("%s: white balance mode %d ",__func__,*wb_mode);
  cfg.cfg.setting = wb_mode;
  cfg.cfgtype = CFG_SET_WHITE_BALANCE;

  ret = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
  if (ret < 0) {
    SERR("failed");
    return ret;
  }
  return ret;
}


/*==========================================================
 * FUNCTION    - sensor_get_res_cfg_table -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_res_cfg_table(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct sensor_res_cfg_table_t **res_cfg =
    (struct sensor_res_cfg_table_t **)data;
  if (!res_cfg) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  *res_cfg = ctrl->lib_params->sensor_lib_ptr->sensor_res_cfg_table;
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_get_cur_chromatix_name -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_cur_chromatix_name(void *sctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_get_t *sensor_get = (sensor_get_t *)data;
  struct sensor_lib_chromatix_array *chromatix_array = NULL;
  enum msm_sensor_resolution_t cur_res = MSM_SENSOR_INVALID_RES;
  if (!sensor_get) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  chromatix_array = ctrl->lib_params->sensor_lib_ptr->chromatix_array;
  cur_res = ctrl->s_data->cur_res;
  if (ctrl->lib_params->sensor_lib_ptr->chromatix_array) {
    if (cur_res > chromatix_array->size) {
      SERR("failed cur res %d max size %d",
        cur_res, chromatix_array->size);
      return SENSOR_ERROR_INVAL;
    }

    sensor_get->chromatix_name.common_chromatix =
      chromatix_array->sensor_lib_chromatix[cur_res].common_chromatix;

    /* Fill snapshot chroamtix name of FULL SIZE resolution */
    sensor_get->chromatix_name.snapshot_chromatix = chromatix_array->
      sensor_lib_chromatix[MSM_SENSOR_RES_FULL].camera_snapshot_chromatix;

    if (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)) {
      sensor_get->chromatix_name.chromatix =
        chromatix_array->sensor_lib_chromatix[cur_res].camcorder_chromatix;
      sensor_get->chromatix_name.liveshot_chromatix =
        chromatix_array->sensor_lib_chromatix[cur_res].liveshot_chromatix;
      SLOW("res %d mask %x camcorder chromatix %s",
        cur_res, ctrl->s_data->cur_stream_mask,
        sensor_get->chromatix_name.chromatix);
    } else if (
      (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT)) &&
      (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW))) {
      sensor_get->chromatix_name.chromatix =
        chromatix_array->sensor_lib_chromatix[cur_res].camera_preview_chromatix;
      SLOW("res %d mask %x preview chromatix %s",
        cur_res, ctrl->s_data->cur_stream_mask,
        sensor_get->chromatix_name.chromatix);
    } else if ((ctrl->s_data->cur_stream_mask &
              (1 << CAM_STREAM_TYPE_SNAPSHOT)) ||
              (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_RAW))) {
      sensor_get->chromatix_name.chromatix =
        chromatix_array->sensor_lib_chromatix[cur_res].
        camera_snapshot_chromatix;
      SLOW("res %d mask %x snapshot chromatix %s",
        cur_res, ctrl->s_data->cur_stream_mask,
        sensor_get->chromatix_name.chromatix);
    } else if (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW)) {
      sensor_get->chromatix_name.chromatix =
        chromatix_array->sensor_lib_chromatix[cur_res].camera_preview_chromatix;
      SLOW("res %d mask %x preview chromatix %s",
        cur_res, ctrl->s_data->cur_stream_mask,
        sensor_get->chromatix_name.chromatix);
    }
  } else {
    sensor_get->chromatix_name.common_chromatix = NULL;
    sensor_get->chromatix_name.chromatix = NULL;
  }
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_get_capabilities -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_capabilities(void *slib, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)slib;
  mct_pipeline_sensor_cap_t *sensor_cap = (mct_pipeline_sensor_cap_t *)data;
  uint32_t i = 0, size = 0;
  struct sensor_lib_out_info_t *out_info = NULL;
  //msm_sensor_scale_size_t *scale_params = NULL;
  msm_sensor_dimension_t scale_tbl[MAX_SCALE_SIZES_CNT];
  struct sensor_crop_parms_t *crop_info;

  if (!sensor_cap || !lib || !lib->sensor_lib_ptr ||
      !lib->sensor_lib_ptr->default_lens_info) {
    SERR("failed");
    return SENSOR_ERROR_INVAL;
  }
  /* Fill lens info */
  sensor_cap->focal_length =
    lib->sensor_lib_ptr->default_lens_info->focal_length;
  sensor_cap->hor_view_angle =
    lib->sensor_lib_ptr->default_lens_info->hor_view_angle;
  sensor_cap->ver_view_angle =
    lib->sensor_lib_ptr->default_lens_info->ver_view_angle;
  sensor_cap->near_end_distance =
    lib->sensor_lib_ptr->default_lens_info->near_end_distance;
  if (lib->sensor_lib_ptr->video_hdr_awb_lsc_func_table) {
    sensor_cap->feature_mask =
      lib->sensor_lib_ptr->video_hdr_awb_lsc_func_table->video_hdr_capability;
  } else {
    sensor_cap->feature_mask = 0;
  }

  if (lib->sensor_lib_ptr->sensor_output->output_format == SENSOR_YCBCR) {
    sensor_cap->ae_lock_supported = FALSE;
    sensor_cap->wb_lock_supported = FALSE;
    sensor_cap->scene_mode_supported = FALSE;
    /* scene mode for YUV sensor */
    if (lib->sensor_lib_ptr->sensor_supported_scene_mode) {
      sensor_cap->sensor_supported_scene_modes = *(lib->sensor_lib_ptr->sensor_supported_scene_mode);
    }
    /* effect mode for YUV sensor */
    if (lib->sensor_lib_ptr->sensor_supported_effect_mode) {
      sensor_cap->sensor_supported_effect_modes = *(lib->sensor_lib_ptr->sensor_supported_effect_mode);
    }
  } else {
    sensor_cap->ae_lock_supported = TRUE;
    sensor_cap->wb_lock_supported = TRUE;
    sensor_cap->scene_mode_supported = TRUE;
  }

  if (lib->sensor_lib_ptr->out_info_array->size > SENSOR_MAX_RESOLUTION) {
    SERR("out info array %d > MAX", lib->sensor_lib_ptr->out_info_array->size);
    size = SENSOR_MAX_RESOLUTION;
    return rc;
  } else {
    size = lib->sensor_lib_ptr->out_info_array->size;
  }
  out_info = lib->sensor_lib_ptr->out_info_array->out_info;
  crop_info = lib->sensor_lib_ptr->crop_params_array->crop_params;
  sensor_cap->dim_fps_table_count = 0;
  for (i = 0; i < size; i++) {
    sensor_cap->dim_fps_table[i].dim.width = out_info[i].x_output -
      crop_info[i].left_crop - crop_info[i].right_crop;
    if (lib->sensor_lib_ptr->sensor_output->output_format == SENSOR_YCBCR)
      sensor_cap->dim_fps_table[i].dim.width /= 2;
    sensor_cap->dim_fps_table[i].dim.height = out_info[i].y_output -
      crop_info[i].top_crop - crop_info[i].bottom_crop;

    /* Use 1052x760 as 720p*/
    if( out_info[i].y_output == 760 )
    {
      sensor_cap->dim_fps_table[i].dim.width = 1280;
      sensor_cap->dim_fps_table[i].dim.height = 720;
    }
    sensor_cap->dim_fps_table[i].fps.min_fps = out_info[i].min_fps;
    sensor_cap->dim_fps_table[i].fps.max_fps = out_info[i].max_fps;
    sensor_cap->dim_fps_table[i].fps.video_min_fps =
      sensor_cap->dim_fps_table[i].fps.min_fps;
    sensor_cap->dim_fps_table[i].fps.video_max_fps =
      sensor_cap->dim_fps_table[i].fps.max_fps;
    sensor_cap->dim_fps_table_count++;
    sensor_cap->dim_fps_table[i].mode  = out_info[i].mode;
  }

  /* Fill scale size table*/
  if(lib->sensor_lib_ptr->get_scale_tbl &&
      lib->sensor_lib_ptr->scale_tbl_cnt > 0){
    if(lib->sensor_lib_ptr->scale_tbl_cnt > MAX_SCALE_SIZES_CNT){
      lib->sensor_lib_ptr->scale_tbl_cnt = MAX_SCALE_SIZES_CNT;
    }
    rc = lib->sensor_lib_ptr->get_scale_tbl(scale_tbl);
    if(rc){
      return rc;
    }

    sensor_cap->scale_picture_sizes_cnt = lib->sensor_lib_ptr->scale_tbl_cnt;
    for(i = 0; i < lib->sensor_lib_ptr->scale_tbl_cnt; i++){
      sensor_cap->scale_picture_sizes[i].width = scale_tbl[i].width;
      sensor_cap->scale_picture_sizes[i].height= scale_tbl[i].height;
      SLOW("scale size width=%d, height=%d.",
        sensor_cap->scale_picture_sizes[i].width, sensor_cap->scale_picture_sizes[i].height);
    }
  }


  /*raw fmts*/
  sensor_cap->supported_raw_fmts_cnt = 0;
  sensor_stream_info_t *sensor_stream_info =
      &(lib->sensor_lib_ptr->sensor_stream_info_array->sensor_stream_info[0]);
  uint16_t num_cid_ch = sensor_stream_info->vc_cfg_size;
  for (i = 0; i < num_cid_ch; i++) {
    switch (sensor_stream_info->pix_fmt_fourcc[i].fourcc) {
      case V4L2_PIX_FMT_SBGGR8: {
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
      }
        break;
      case V4L2_PIX_FMT_SGBRG8: {
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
      }
        break;
      case V4L2_PIX_FMT_SGRBG8: {
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
      }
        break;
      case V4L2_PIX_FMT_SRGGB8: {
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
      }
        break;
      case V4L2_PIX_FMT_SBGGR10: {
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
      }
        break;
      case V4L2_PIX_FMT_SGBRG10: {
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
      }
        break;
      case V4L2_PIX_FMT_SGRBG10: {
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
      }
        break;
      case V4L2_PIX_FMT_SRGGB10: {
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
      }
        break;
      case V4L2_PIX_FMT_SBGGR12: {
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
      }
        break;
      case V4L2_PIX_FMT_SGBRG12: {
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
      }
        break;
      case V4L2_PIX_FMT_SGRBG12: {
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
      }
        break;
      case V4L2_PIX_FMT_SRGGB12: {
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
      }
        break;
      case V4L2_PIX_FMT_NV12:
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_420_NV12;
        break;
      case V4L2_PIX_FMT_NV16:
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        break;
      case V4L2_PIX_FMT_NV61:
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV61;
        break;
      case V4L2_MBUS_FMT_YUYV8_2X8:
      case V4L2_MBUS_FMT_YVYU8_2X8:
      case V4L2_MBUS_FMT_UYVY8_2X8:
      case V4L2_MBUS_FMT_VYUY8_2X8:
        sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
            CAM_FORMAT_YUV_422_NV16;
        break;
      }
    }

  /* Fill sensor pipeline delay */
  sensor_cap->max_pipeline_frame_delay =
      lib->sensor_lib_ptr->sensor_max_pipeline_frame_delay;

  /* Fill sensor manual exposure info */
  if(lib->sensor_lib_ptr->manual_exp_info){
    sensor_cap->min_exposure_time =
      lib->sensor_lib_ptr->manual_exp_info->min_exposure_time;
    sensor_cap->max_exposure_time =
      lib->sensor_lib_ptr->manual_exp_info->max_exposure_time;
    sensor_cap->min_iso = lib->sensor_lib_ptr->manual_exp_info->min_iso;
    sensor_cap->max_iso = lib->sensor_lib_ptr->manual_exp_info->max_iso;
  } else {
    sensor_cap->min_exposure_time = 0;
    sensor_cap->max_exposure_time = 0;
    sensor_cap->min_iso = 0;
    sensor_cap->max_iso = 0;
  }

  /* Fill sensor init params */
  sensor_cap->modes_supported =
    lib->sensor_lib_ptr->sensor_init_params->modes_supported;
  sensor_cap->position = lib->sensor_lib_ptr->sensor_init_params->position;
  sensor_cap->sensor_mount_angle =
    lib->sensor_lib_ptr->sensor_init_params->sensor_mount_angle;
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_metadata_hdr -
 *
 * DESCRIPTION:
 *==========================================================*/
static void sensor_set_metadata_hdr(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  sensor_out_info_t *sensor_out_info = (void *)data;
  uint32_t i;

  for (i = 0; i < sensor_out_info->meta_cfg.num_meta; i++) {
    if(!lib->sensor_lib_ptr->meta_data_out_info_array->meta_data_out_info[i].ebd) {
      sensor_out_info->meta_cfg.sensor_meta_info[i].is_valid = TRUE;
      sensor_out_info->meta_cfg.sensor_meta_info[i].dump_to_fs = FALSE;
      sensor_out_info->meta_cfg.sensor_meta_info[i].dim.width =
      lib->sensor_lib_ptr->meta_data_out_info_array->meta_data_out_info[i].width;
      sensor_out_info->meta_cfg.sensor_meta_info[i].dim.height =
      lib->sensor_lib_ptr->meta_data_out_info_array->meta_data_out_info[i].height;
      sensor_out_info->meta_cfg.sensor_meta_info[i].fmt = CAM_FORMAT_META_RAW_8BIT;
      sensor_out_info->meta_cfg.sensor_meta_info[i].stats_type =
        lib->sensor_lib_ptr->meta_data_out_info_array->meta_data_out_info[i].stats_type;
      if (lib->sensor_lib_ptr->meta_data_out_info_array->meta_data_out_info[i].fmt == 10) {
        sensor_out_info->meta_cfg.sensor_meta_info[i].fmt = CAM_FORMAT_META_RAW_10BIT;
      }
    }
  }
}


/*==========================================================
 * FUNCTION    - sensor_set_metadata_ebd -
 *
 * DESCRIPTION:
 *==========================================================*/
static void  sensor_set_metadata_ebd(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  sensor_out_info_t *sensor_out_info = (void *)data;
  uint32_t i;
  for (i = 0; i < sensor_out_info->meta_cfg.num_meta; i++) {
    if(lib->sensor_lib_ptr->meta_data_out_info_array->meta_data_out_info[i].ebd) {
      sensor_out_info->meta_cfg.sensor_meta_info[i].is_valid = TRUE;
      sensor_out_info->meta_cfg.sensor_meta_info[i].dump_to_fs = FALSE;
      if (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_RAW)) {
        sensor_out_info->meta_cfg.sensor_meta_info[i].dump_to_fs = TRUE;
      }
      sensor_out_info->meta_cfg.sensor_meta_info[i].dim.width =
      lib->sensor_lib_ptr->meta_data_out_info_array->meta_data_out_info[i].width;
      sensor_out_info->meta_cfg.sensor_meta_info[i].dim.height =
      lib->sensor_lib_ptr->meta_data_out_info_array->meta_data_out_info[i].height;
      sensor_out_info->meta_cfg.sensor_meta_info[i].fmt = CAM_FORMAT_META_RAW_8BIT;
      if (lib->sensor_lib_ptr->meta_data_out_info_array->meta_data_out_info[i].fmt == 10) {
        sensor_out_info->meta_cfg.sensor_meta_info[i].fmt = CAM_FORMAT_META_RAW_10BIT;
      }
    }
  }
}

/*==========================================================
 * FUNCTION    - sensor_get_resolution_info -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_resolution_info(void *sctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  sensor_out_info_t *sensor_out_info = (void *)data;
  enum msm_sensor_resolution_t cur_res;
  uint16_t i = 0;
  sensor_stream_info_array_t *sensor_stream_info_array = NULL;
  if ((!sensor_out_info) ||
  (ctrl->s_data->cur_res >= MSM_SENSOR_INVALID_RES)) {
    SERR("failed");
    return SENSOR_ERROR_INVAL;
  }
  cur_res = ctrl->s_data->cur_res;
  sensor_out_info->csi_clk_scale_enable =
    lib->sensor_lib_ptr->csi_params_array->csi2_params[cur_res]->csi_clk_scale_enable;
  /* Fill isp params */
  sensor_out_info->mode =
    lib->sensor_lib_ptr->sensor_init_params->modes_supported;
  sensor_stream_info_array = lib->sensor_lib_ptr->sensor_stream_info_array;
  sensor_out_info->dim_output.width =
    lib->sensor_lib_ptr->out_info_array->out_info[cur_res].x_output;
  sensor_out_info->dim_output.height =
    lib->sensor_lib_ptr->out_info_array->out_info[cur_res].y_output;
  sensor_out_info->request_crop.first_pixel =
    lib->sensor_lib_ptr->crop_params_array->crop_params[cur_res].left_crop;
  sensor_out_info->request_crop.last_pixel =
    lib->sensor_lib_ptr->out_info_array->out_info[cur_res].x_output -
    lib->sensor_lib_ptr->crop_params_array->crop_params[cur_res].right_crop - 1;
  sensor_out_info->request_crop.first_line =
    lib->sensor_lib_ptr->crop_params_array->crop_params[cur_res].top_crop;
  sensor_out_info->request_crop.last_line =
    lib->sensor_lib_ptr->out_info_array->out_info[cur_res].y_output -
    lib->sensor_lib_ptr->crop_params_array->crop_params[cur_res].bottom_crop -
    1;
  sensor_out_info->op_pixel_clk =
    lib->sensor_lib_ptr->out_info_array->out_info[cur_res].op_pixel_clk;
  if (ctrl->s_data->ae_bracket_info.ae_bracket_config.mode ==
     CAM_EXP_BRACKETING_ON) {
    sensor_out_info->num_frames_skip =
      lib->sensor_lib_ptr->sensor_num_HDR_frame_skip;
  } else if(ctrl->s_data->manual_exposure_mode == 1){
    sensor_out_info->num_frames_skip = 0;
  } else{
    sensor_out_info->num_frames_skip =
      lib->sensor_lib_ptr->sensor_num_frame_skip;
  }
  if (lib->sensor_lib_ptr->sensor_output->output_format != SENSOR_YCBCR) {
    /* Fill 3A params */
    sensor_out_info->max_gain =
      lib->sensor_lib_ptr->aec_info->max_gain;
    sensor_out_info->max_linecount =
      lib->sensor_lib_ptr->aec_info->max_linecount;
    if (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW) ||
      ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)) {
      sensor_out_info->max_fps = ctrl->s_data->cur_fps;
    } else {
      sensor_out_info->max_fps =
        lib->sensor_lib_ptr->out_info_array->out_info[cur_res].max_fps;
    }
  } else {
    sensor_out_info->max_fps =
      lib->sensor_lib_ptr->out_info_array->out_info[cur_res].max_fps;
  }
  sensor_out_info->vt_pixel_clk =
    lib->sensor_lib_ptr->out_info_array->out_info[cur_res].vt_pixel_clk;
  sensor_out_info->ll_pck =
    lib->sensor_lib_ptr->out_info_array->out_info[cur_res].line_length_pclk;
  sensor_out_info->fl_lines =
    lib->sensor_lib_ptr->out_info_array->out_info[cur_res].frame_length_lines;
  sensor_out_info->pixel_sum_factor =
    lib->sensor_lib_ptr->out_info_array->out_info[cur_res].binning_factor;
  /* Lens related information for AF */
  sensor_out_info->af_lens_info.f_number =
     lib->sensor_lib_ptr->default_lens_info->f_number;
  sensor_out_info->af_lens_info.focal_length =
     lib->sensor_lib_ptr->default_lens_info->focal_length;
  sensor_out_info->af_lens_info.hor_view_angle =
     lib->sensor_lib_ptr->default_lens_info->hor_view_angle;
  sensor_out_info->af_lens_info.ver_view_angle =
     lib->sensor_lib_ptr->default_lens_info->ver_view_angle;
  sensor_out_info->af_lens_info.pix_size =
     lib->sensor_lib_ptr->default_lens_info->pix_size;
  sensor_out_info->af_lens_info.total_f_dist =
     lib->sensor_lib_ptr->default_lens_info->total_f_dist;
  sensor_out_info->af_lens_info.um_per_dac =
     lib->sensor_lib_ptr->default_lens_info->um_per_dac;
  sensor_out_info->af_lens_info.dac_offset =
     lib->sensor_lib_ptr->default_lens_info->dac_offset;

  /* Fill sensor pipeline delay */
  sensor_out_info->sensor_max_pipeline_frame_delay =
      lib->sensor_lib_ptr->sensor_max_pipeline_frame_delay;

  sensor_out_info->meta_cfg.num_meta = 0;
  memset(sensor_out_info->meta_cfg.sensor_meta_info, 0,
      sizeof(sensor_out_info->meta_cfg.sensor_meta_info));

  if (lib->sensor_lib_ptr->meta_data_out_info_array) {
      sensor_out_info->meta_cfg.num_meta =
      lib->sensor_lib_ptr->meta_data_out_info_array->size;
  }
  if (sensor_out_info->meta_cfg.num_meta) {
    if (ctrl->s_data->video_hdr_enable || ctrl->s_data->snapshot_hdr_enable) {
        sensor_set_metadata_hdr(sctrl, data);
    }
    else if ((ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_RAW)) || \
      (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW))) {
      sensor_set_metadata_ebd(sctrl, data);
    }
    else {
      for (i = 0; i < sensor_out_info->meta_cfg.num_meta; i++) {
        sensor_out_info->meta_cfg.sensor_meta_info[i].is_valid = FALSE;
      }
    }
  }else {
  for (i = 0; i < sensor_out_info->meta_cfg.num_meta; i++) {
      sensor_out_info->meta_cfg.sensor_meta_info[i].is_valid = FALSE;
    }
  }
  /* Fill sensor mount angle */
  sensor_out_info->sensor_mount_angle =
    lib->sensor_lib_ptr->sensor_init_params->sensor_mount_angle;

  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_get_sensor_port_info -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_sensor_port_info(void *slib, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)slib;
  sensor_stream_info_array_t **sensor_port_info_array =
    (sensor_stream_info_array_t **)data;
  if (!sensor_port_info_array) {
    SERR("failed");
    return SENSOR_ERROR_INVAL;
  }
  *sensor_port_info_array = lib->sensor_lib_ptr->sensor_stream_info_array;
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_video_hdr_enable -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_video_hdr_enable(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *video_hdr_enable = (int32_t *)data;
  if (!video_hdr_enable) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  SLOW("video_hdr enable %d", *video_hdr_enable);
  if(ctrl->s_data->video_hdr_enable != *video_hdr_enable)
    ctrl->s_data->video_hdr_enable = *video_hdr_enable;
  else
    /* not an error condition, just indicate that we are already in this mode */
    return SENSOR_ERROR_INVAL;
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_set_snapshot_hdr_enable -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_snapshot_hdr_enable(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *snapshot_hdr_enable = (int32_t *)data;
  if (!snapshot_hdr_enable) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  SLOW("snapshot_hdr enable %d", *snapshot_hdr_enable);
  if(ctrl->s_data->snapshot_hdr_enable != *snapshot_hdr_enable)
    ctrl->s_data->snapshot_hdr_enable = *snapshot_hdr_enable;
  else
    /* not an error condition, just indicate that we are already in this mode */
    return SENSOR_ERROR_INVAL;
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_set_dis_enable -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_dis_enable(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *dis_enable = (int32_t *)data;
  if (!dis_enable) {
    SERR("failed");
    return -EINVAL;
  }
  SLOW("dis enable %d", *dis_enable);
  ctrl->s_data->dis_enable = *dis_enable;
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_set_op_pixel_clk_change -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_op_pixel_clk_change(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  uint32_t *op_pixel_clk = (uint32_t *)data;
  if (!ctrl || !ctrl->s_data || !op_pixel_clk) {
    SERR("failed");
    return -EINVAL;
  }

  PTHREAD_MUTEX_LOCK(&ctrl->s_data->mutex);

  SLOW("dis enable %d", *op_pixel_clk);
  ctrl->s_data->isp_pixel_clk_max = *op_pixel_clk;

  PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);

  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_set_calibration_data -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_calibration_data(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct sensorb_cfg_data cfg;
  struct msm_camera_i2c_reg_setting* setting =
    (struct msm_camera_i2c_reg_setting*)data;

  if (!ctrl || !ctrl->s_data) {
    SERR("failed");
    return -EINVAL;
  }

  if(setting->size == 0){
    SERR("%s:Golden module or OTP data is null", __func__);
    return SENSOR_SUCCESS;
  }

  PTHREAD_MUTEX_LOCK(&ctrl->s_data->mutex);

  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  cfg.cfg.setting = data;
  if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
    SERR("failed");
    PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);
    return -EIO;
  }

  PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);

  return SENSOR_SUCCESS;
}


/** sensor_set_max_raw_dimension: set max dimension for sensor
 *
 *  @sctrl: handle to sensor control structure
 *  @data: handle to cam_dimension_t
 *
 *  This function stores max raw dimension passed by HAL
 *
 *  Return: SENSOR_SUCCESS for success
 *          Negative error for failure
 **/
static int32_t sensor_set_max_dimension(void *sctrl, void *data)
{
  sensor_ctrl_t   *ctrl = (sensor_ctrl_t *)sctrl;
  cam_dimension_t *max_dim = (cam_dimension_t *)data;

  if (!ctrl || !ctrl->s_data || !max_dim) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  if ((max_dim->width <= 0) || (max_dim->height <= 0)) {
    SERR("failed: invalid params raw dim w %d h %d", max_dim->width,
      max_dim->height);
    return SENSOR_FAILURE;
  }

  SERR("raw w %d h %d", max_dim->width, max_dim->height);
  ctrl->s_data->max_dim = *max_dim;

  return SENSOR_SUCCESS;
}

/** sensor_get_raw_dimension: get raw dimension for sensor
 *
 *  @sctrl: handle to sensor control structure
 *  @data: handle to cam_dimension_t
 *
 *  This function stores retrieves raw dimension for the
 *  resolution passed by HAL
 *
 *  Return: SENSOR_SUCCESS for success
 *          Negative error for failure
 **/
static int32_t sensor_get_raw_dimension(void *sctrl, void *data)
{
  int32_t                           rc = SENSOR_SUCCESS;
  sensor_ctrl_t                    *ctrl = (sensor_ctrl_t *)sctrl;
  cam_dimension_t                  *raw_dim = NULL;
  cam_dimension_t                  *max_dim = NULL;
  sensor_set_res_cfg_t              res_cfg;
  enum msm_sensor_resolution_t      pick_res = MSM_SENSOR_INVALID_RES;
  sensor_get_raw_dimension_t       *sensor_get =
    (sensor_get_raw_dimension_t *)data;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  struct sensor_lib_out_info_t     *out_info = NULL;
  struct sensor_crop_parms_t       *crop_params = NULL;

  if (!ctrl || !ctrl->s_data || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  if (!ctrl->lib_params || !ctrl->lib_params->sensor_lib_ptr ||
      !ctrl->lib_params->sensor_lib_ptr->out_info_array ||
      !ctrl->lib_params->sensor_lib_ptr->crop_params_array) {
    SERR("failed: invalid params");
    return SENSOR_FAILURE;
  }

  out_info_array = ctrl->lib_params->sensor_lib_ptr->out_info_array;
  raw_dim = (cam_dimension_t *)sensor_get->raw_dim;
  max_dim = &ctrl->s_data->max_dim;
  if ((max_dim->width <= 0) || (max_dim->height <= 0)) {
    SERR("failed: invalid params maw dim w %d h %d", max_dim->width,
      max_dim->height);
    return SENSOR_FAILURE;
  }

  res_cfg.width = max_dim->width;
  res_cfg.height = max_dim->height;
  res_cfg.stream_mask = (sensor_get->stream_mask | (1 << CAM_STREAM_TYPE_RAW));
  rc = sensor_pick_resolution(sctrl, &res_cfg, &pick_res);
  if (rc < 0) {
    SERR("failed: sensor_pick_resolution rc %d", rc);
    return rc;
  }

  SERR("pick res %d", pick_res);
  if ((pick_res >= out_info_array->size) ||
      (pick_res == MSM_SENSOR_INVALID_RES)) {
    SERR("Error: failed to find resolution requested w*h %d*%d fps %f",
      res_cfg.width, res_cfg.height, ctrl->s_data->cur_fps);
    return SENSOR_FAILURE;
  }

  out_info = &out_info_array->out_info[pick_res];
  crop_params =
    &ctrl->lib_params->sensor_lib_ptr->crop_params_array->crop_params[pick_res];
  raw_dim->width =
    out_info->x_output - crop_params->left_crop - crop_params->right_crop;
  raw_dim->height =
    out_info->y_output - crop_params->top_crop - crop_params->bottom_crop;
  SERR("raw w %d h %d", raw_dim->width, raw_dim->height);

  return SENSOR_SUCCESS;
}

/** sensor_get_exposure_time: Get sensor exposure time
 *
 *  @sctrl: sensor control structure
 *  @data: pointer to
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function returns sensor exposure time in nanosecond **/

static int8_t sensor_get_exposure_time(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  enum msm_sensor_resolution_t cur_res = MSM_SENSOR_INVALID_RES;
  int64_t *exp = (int64_t *)data;

  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  out_info_array = ctrl->lib_params->sensor_lib_ptr->out_info_array;
  cur_res = ctrl->s_data->cur_res;

  SLOW("%s: line_length_pclk = %d, current_linecount = %d, vt_pixel_clk = %d",
    __func__, ctrl->s_data->cur_line_length_pclk,
    ctrl->s_data->current_linecount,
    out_info_array->out_info[cur_res].vt_pixel_clk);
  if (out_info_array->out_info[cur_res].vt_pixel_clk > 0) {
    *exp = NANO_SEC_PER_SEC *
      ((float)ctrl->s_data->cur_line_length_pclk *
      ctrl->s_data->current_linecount /
      out_info_array->out_info[cur_res].vt_pixel_clk);
  }
  ctrl->s_data->cur_exposure_time = *exp / NANO_SEC_TO_MICRO_SEC;
  SLOW("exp %lld" , *exp);

  return SENSOR_SUCCESS;
}

/** sensor_get_frame_metadata
 *
 *  @sctrl: sensor control struct handle
 *  @data: handle to return per frame metadata
 *
 *  Return per frame metadata applied on current frame
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t sensor_get_frame_metadata(void *sctrl, void *data)
{
  int32_t                           rc = 0;
  sensor_ctrl_t                    *ctrl = NULL;
  sensor_per_frame_metadata_t      *metadata = NULL;
  enum msm_sensor_resolution_t      cur_res = MSM_SENSOR_INVALID_RES;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  int64_t                           exp_time = 0;

  if (!sctrl || !data) {
    SERR("failed: sctrl %p data %p", sctrl, data);
    return SENSOR_FAILURE;
  }
  ctrl = (sensor_ctrl_t *)sctrl;
  metadata = (sensor_per_frame_metadata_t *)data;

  if (!ctrl->s_data || !ctrl->lib_params || !ctrl->lib_params->sensor_lib_ptr) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  out_info_array = ctrl->lib_params->sensor_lib_ptr->out_info_array;
  cur_res = ctrl->s_data->cur_res;
  if ((cur_res >= out_info_array->size) ||
      (cur_res >= MSM_SENSOR_INVALID_RES)) {
    SERR("Error: failed cur res %d array size %d max res %d", cur_res,
      out_info_array->size, MSM_SENSOR_INVALID_RES);
    return SENSOR_FAILURE;
  }

  /* Remove this call after integrating HAL 3.0 changes */
  sensor_get_exposure_time(sctrl, &exp_time);

  /* Fill per frame metadata */
  metadata->width = out_info_array->out_info[cur_res].x_output;
  metadata->height = out_info_array->out_info[cur_res].y_output;
  metadata->sensor_real_gain = ctrl->s_data->sensor_real_gain;
  metadata->sensor_digital_gain = ctrl->s_data->digital_gain;
  metadata->total_gain = ctrl->s_data->sensor_real_gain *
                                  ctrl->s_data->digital_gain *
                                  ctrl->s_data->sensor_digital_gain;
  metadata->exposure_time = ctrl->s_data->cur_exposure_time;

  return SENSOR_SUCCESS;
}

/** sensor_get_per_frame_info
 *
 *  @sctrl: sensor control struct handle
 *  @data: handle to return per frame info
 *
 *  Return per frame info
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t sensor_get_per_frame_info(void *sctrl, void *data)
{
  sensor_ctrl_t                    *ctrl = NULL;
  sensor_per_frame_delay_info_t    *delay_info = NULL;

  if (!sctrl || !data) {
    SERR("failed: sctrl %p data %p", sctrl, data);
    return SENSOR_FAILURE;
  }

  ctrl = (sensor_ctrl_t *)sctrl;
  if (!ctrl->lib_params || !ctrl->lib_params->sensor_lib_ptr) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  delay_info = (sensor_per_frame_delay_info_t *)data;
  delay_info->sensor_apply_delay =
    ctrl->lib_params->sensor_lib_ptr->sensor_max_immediate_frame_delay;
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_process -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_process(void *sctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  if (!sctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  SLOW("sctrl %p event %d", sctrl, event);
  switch (event) {
  /* Get enums */
  case SENSOR_GET_CAPABILITIES:
    rc = sensor_get_capabilities(sctrl, data);
    break;
  case SENSOR_GET_CUR_CSIPHY_CFG:
    rc = sensor_get_cur_csiphy_cfg(sctrl, data);
    break;
  case SENSOR_GET_CUR_CSID_CFG:
    rc = sensor_get_cur_csid_cfg(sctrl, data);
    break;
  case SENSOR_GET_CSI_CLK_SCALE_CFG:
    rc = sensor_get_csi_clk_scale_cfg(sctrl, data);
    break;
  case SENSOR_GET_CUR_CHROMATIX_NAME:
    rc = sensor_get_cur_chromatix_name(sctrl, data);
    break;
  case SENSOR_GET_CSI_LANE_PARAMS:
    rc = sensor_get_csi_lane_params(sctrl, data);
    break;
  case SENSOR_GET_CUR_FPS:
    rc = sensor_get_cur_fps(sctrl, data);
    break;
  case SENSOR_GET_RESOLUTION_INFO:
    rc = sensor_get_resolution_info(sctrl, data);
    break;
  case SENSOR_GET_RES_CFG_TABLE:
    rc = sensor_get_res_cfg_table(sctrl, data);
    break;
  case SENSOR_GET_SENSOR_PORT_INFO:
    rc = sensor_get_sensor_port_info(sctrl, data);
    break;
  case SENSOR_GET_DIGITAL_GAIN:
    rc = sensor_get_digital_gain(sctrl, data);
    break;
  case SENSOR_GET_SENSOR_FORMAT:
    rc = sensor_get_sensor_format(sctrl, data);
    break;
  case SENSOR_GET_LENS_INFO:
    rc = sensor_get_lens_info(sctrl, data);
    break;
  case SENSOR_GET_RAW_DIMENSION:
    rc = sensor_get_raw_dimension(sctrl, data);
    break;
  case SENSOR_GET_FRAME_METADATA:
    rc = sensor_get_frame_metadata(sctrl, data);
    break;
  case SENSOR_GET_PER_FRAME_INFO:
    rc = sensor_get_per_frame_info(sctrl, data);
    break;

  /* Set enums */
  case SENSOR_SET_LIB_PARAMS:
    rc = sensor_set_lib_params(sctrl, data);
    break;
  case SENSOR_SET_MOUNT_ANGLE:
    rc = sensor_set_mount_angle(sctrl, data);
    break;
  case SENSOR_SET_POSITION:
    rc = sensor_set_position(sctrl, data);
    break;
  case SENSOR_SET_MODE:
    rc = sensor_set_mode(sctrl, data);
    break;
  case SENSOR_INIT:
    rc = sensor_init(sctrl);
    break;
  case SENSOR_STOP_STREAM:
    rc = sensor_set_stop_stream(sctrl);
    break;
  case SENSOR_START_STREAM:
    rc = sensor_set_start_stream(sctrl);
    break;
  case SENSOR_SET_RESOLUTION:
    rc = sensor_set_resolution(sctrl, data);
    break;
  case SENSOR_SET_AEC_UPDATE:
    rc = sensor_set_aec_update(sctrl, data);
    break;
  case SENSOR_SET_AWB_UPDATE:
    rc = sensor_set_awb_video_hdr_update(sctrl, data);
    break;
  case SENSOR_SET_AEC_INIT_SETTINGS:
    rc = sensor_set_aec_init_settings(sctrl, data);
    break;
  case SENSOR_SET_VFE_SOF:
    rc = sensor_set_vfe_sof(sctrl, data);
    break;
  case SENSOR_SET_FPS:
    rc = sensor_set_frame_rate(sctrl, data);
    break;
  case SENSOR_SET_HFR_MODE:
    rc = sensor_set_hfr_mode(sctrl, data);
    break;
  case SENSOR_GET_WAIT_FRAMES:
    rc = sensor_get_wait_frames(sctrl, data);
    break;
  case SENSOR_GET_ACTUATOR_NAME:
    rc = sensor_get_actuator_name(sctrl, data);
    break;
  case SENSOR_SET_WAIT_FRAMES:
    rc = sensor_set_wait_frames(sctrl, data);
    break;
  case SENSOR_SET_HDR_AE_BRACKET:
    rc = sensor_set_hdr_ae_bracket(sctrl, data);
    break;
  case SENSOR_SET_HDR_ZSL_MODE:
    rc = sensor_set_hdr_zsl_mode(sctrl, data);
    break;
  case SENSOR_SET_MANUAL_EXPOSURE_MODE:
    rc = sensor_set_manual_exposure_mode(sctrl, data);
    break;
  case SENSOR_SET_VIDEO_HDR_ENABLE:
    rc = sensor_set_video_hdr_enable(sctrl, data);
    break;
  case SENSOR_SET_SNAPSHOT_HDR_ENABLE:
    rc = sensor_set_snapshot_hdr_enable(sctrl, data);
    break;
  case SENSOR_SET_DIS_ENABLE:
    rc = sensor_set_dis_enable(sctrl, data);
    break;
  case SENSOR_SET_OP_PIXEL_CLK_CHANGE:
    rc = sensor_set_op_pixel_clk_change(sctrl, data);
    break;
  case SENSOR_SET_CALIBRATION_DATA:
    rc = sensor_set_calibration_data(sctrl, data);
    break;
  case SENSOR_SET_SATURATION:
    rc = sensor_set_saturation(sctrl, data);
    break;
  case SENSOR_SET_CONTRAST:
    rc = sensor_set_contrast(sctrl, data);
    break;
  case SENSOR_SET_SHARPNESS:
    rc = sensor_set_sharpness(sctrl, data);
    break;
  case SENSOR_SET_AUTOFOCUS:
    rc = sensor_set_autofocus(sctrl, data);
    break;
  case SENSOR_CANCEL_AUTOFOCUS:
    //rc = sensor_cancel_autofocus(sctrl, data);
    break;
  case SENSOR_SET_ISO:
    rc = sensor_set_iso(sctrl, data);
    break;
  case SENSOR_SET_EXPOSURE_COMPENSATION:
    rc = sensor_set_exposure_compensation(sctrl, data);
    break;
  case SENSOR_SET_ANTIBANDING:
    rc = sensor_set_antibanding(sctrl, data);
    break;
  case SENSOR_SET_BESTSHOT_MODE:
    rc = sensor_set_bestshot_mode(sctrl, data);
    break;
  case SENSOR_SET_EFFECT:
    rc = sensor_set_effect(sctrl, data);
    break;
  case SENSOR_SET_WHITE_BALANCE:
    rc = sensor_set_white_balance(sctrl, data);
    break;
  case SENSOR_SET_MAX_DIMENSION:
    rc = sensor_set_max_dimension(sctrl, data);
    break;
  case SENSOR_SET_AEC_ZSL_SETTINGS:
    rc = sensor_set_aec_zsl_settings(sctrl, data);
    break;
  default:
    SERR("invalid event %d", event);
    rc = SENSOR_FAILURE;
   break;
  }
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_close -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_close(void *sctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct sensorb_cfg_data cfg;

  cfg.cfgtype = CFG_POWER_DOWN;
  rc = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_SENSOR_CFG failed");
  }

  /* close subdev */
  close(ctrl->s_data->fd);

  free(ctrl->s_data);
  free(ctrl);
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_sub_module_init -
 *
 * DESCRIPTION:
 *==========================================================*/
int32_t sensor_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  if (!func_tbl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  func_tbl->open = sensor_open;
  func_tbl->process = sensor_process;
  func_tbl->close = sensor_close;
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_get_hfr_mode_fps -
 *
 * DESCRIPTION: Convert HFR mode enum to fps value
 *==========================================================*/
float sensor_get_hfr_mode_fps(cam_hfr_mode_t mode) {
  switch (mode) {
  case CAM_HFR_MODE_60FPS: return 60.0f;
  case CAM_HFR_MODE_90FPS: return 90.0f;
  case CAM_HFR_MODE_120FPS: return 120.0f;
  case CAM_HFR_MODE_150FPS: return 150.0f;
  default:
    SERR("Error: Invalid HFR mode");
    return 0.0f;
  }
}
