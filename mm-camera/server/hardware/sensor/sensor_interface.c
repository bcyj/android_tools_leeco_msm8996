/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "camera_dbg.h"
#include "sensor_interface.h"
#include "sensor.h"

static sensor_comp_root_t sensor_controller;

static uint32_t sensor_gen_handle(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[4bits obj idx]|[4 bits client idx] */
  uint32_t handle = ((++sensor_controller.sensor_handle_cnt) << 8) +
    (0xff & client_idx);
  return handle;
}

static int sensor_parse_handle(uint32_t handle, uint8_t *client_idx)
{
  int tmp = (handle & 0xff);
  *client_idx = tmp & 0xff;
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, handle, *client_idx);

  return 0;
}

/*===========================================================================
 * FUNCTION    - get_sensor_client_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static sensor_client_t *get_sensor_client_info(uint32_t handle)
{
  uint8_t client_idx;

  sensor_parse_handle(handle, &client_idx);

  if(sensor_controller.client[client_idx].handle != handle ||
    client_idx >= SENSOR_MAX_CLIENT_NUM) {
    CDBG_ERROR("%s: client_idx = %d, local_handle = 0x%x, input_handle = 0x%x",
      __func__, client_idx, sensor_controller.client[client_idx].handle,
      handle);
    return NULL;
  } else {
    return &(sensor_controller.client[client_idx]);
  }
} /* get_sensor_client_info */

/*===========================================================================
 * FUNCTION    - sensor_interface_init -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
static int sensor_client_init(uint32_t handle, mctl_ops_t *ops,
  void *p_init_data)
{
  int status = 0;
  sensor_init_data_t *init_data = p_init_data;
  uint32_t fd = init_data->fd;
  sensor_client_t *sensor_client = get_sensor_client_info(handle);

  if(!sensor_client) {
    CDBG_ERROR("%s: failed %d\n", __func__, __LINE__);
    return -EINVAL;
  }

  sensor_client->ops = ops;
  sensor_ctrl_t *sctrl = sensor_client->sensorCtrl;
  CDBG("%s: sfd = %d\n", __func__, fd);
  sctrl->sfd = fd;
  status = sensor_init(sctrl);

  return status;
}

/*===========================================================================
 * FUNCTION    - sensor_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int  sensor_client_set_params(uint32_t handle, int type, void *parm_in, void *parm_out)
{
  int status = 0;
  sensor_set_t *sensor_set = parm_in;
  sensor_client_t *sensor_client = get_sensor_client_info(handle);
  if(!sensor_client) {
    CDBG_ERROR("%s: failed %d\n", __func__, __LINE__);
    return -EINVAL;
  }

  sensor_ctrl_t *sctrl = sensor_client->sensorCtrl;

  switch(type) {
  case SENSOR_SET_MODE:
    if (sctrl->fn_table->sensor_set_op_mode) {
      status = sctrl->fn_table->sensor_set_op_mode(sctrl,
                 sensor_set->data.mode);
    }
    break;
  case SENSOR_SET_FPS:
    if (sctrl->sensor.op_mode == SENSOR_MODE_INVALID ) {
      sctrl->sensor.pending_fps = sensor_set->data.aec_data.fps;
    } else {
      if (sctrl->fn_table->sensor_set_frame_rate) {
        status = sctrl->fn_table->sensor_set_frame_rate(sctrl,
                   sensor_set->data.aec_data.fps);
      }
      sctrl->sensor.pending_fps = 0;
    }
    break;
  case SENSOR_SET_EXPOSURE: {
    /* get exposure settings from stats proc */
    sensor_set_aec_data_t aec_data;
    if (NULL == sensor_set) {
      stats_proc_get_t stats_proc_get;
      stats_proc_get.d.get_aec.type = AEC_EXPOSURE_PARAMS;
      stats_proc_get.d.get_aec.d.exp_params.is_snapshot =
        (sctrl->sensor.op_mode == SENSOR_MODE_SNAPSHOT
        || sctrl->sensor.op_mode == SENSOR_MODE_RAW_SNAPSHOT);
      sensor_client->ops->fetch_params(sensor_client->my_comp_id,
         sensor_client->ops->parent,
        ((MCTL_COMPID_STATSPROC << 24) | STATS_PROC_AEC_TYPE),
        &stats_proc_get, sizeof(stats_proc_get));
      aec_data.current_luma =
        stats_proc_get.d.get_aec.d.exp_params.current_luma;
      aec_data.gain =
        stats_proc_get.d.get_aec.d.exp_params.gain;
      aec_data.linecount =
        stats_proc_get.d.get_aec.d.exp_params.linecount;
      aec_data.luma_target =
        stats_proc_get.d.get_aec.d.exp_params.luma_target;
      CDBG("%s:%d] SENSOR_SET_EXPOSURE snap %d lum %d gain %f linecnt %u"
        " luma target %d ", __func__, __LINE__,
        stats_proc_get.d.get_aec.d.exp_params.is_snapshot,
        aec_data.current_luma,
        aec_data.gain,
        aec_data.linecount,
        aec_data.luma_target);
    } else {
      aec_data = sensor_set->data.aec_data;
    }

    if (sctrl->sensor.op_mode == SENSOR_MODE_SNAPSHOT ||
      sctrl->sensor.op_mode == SENSOR_MODE_RAW_SNAPSHOT) {
      if(sctrl->fn_table->sensor_set_snapshot_exposure_gain) {
        status = sctrl->fn_table->sensor_set_snapshot_exposure_gain(
          sctrl, &aec_data);
      }
    } else {
      if(sctrl->fn_table->sensor_set_exposure_gain) {
        status = sctrl->fn_table->sensor_set_exposure_gain(sctrl, &aec_data);
      }
    }
    break;
  }
  case SENSOR_SET_CHROMATIX_TYPE:
    status = sensor_re_load_chromatix(sctrl, sensor_set->data.chromatix_type);
    break;
  case SENSOR_SET_SATURATION:
    if (sctrl->fn_table->sensor_set_saturation) {
      sctrl->fn_table->sensor_set_saturation(sctrl,
        sensor_set->data.saturation);
    }
    break;
  case SENSOR_SET_CONTRAST:
    if (sctrl->fn_table->sensor_set_contrast) {
      sctrl->fn_table->sensor_set_contrast(sctrl,
        sensor_set->data.contrast);
    }
    break;
  case SENSOR_SET_SHARPNESS:
    if (sctrl->fn_table->sensor_set_sharpness) {
      sctrl->fn_table->sensor_set_sharpness(sctrl,
        sensor_set->data.sharpness);
    }
    break;
  case SENSOR_SET_EXPOSURE_COMPENSATION:
    if (sctrl->fn_table->sensor_set_exposure_compensation) {
      sctrl->fn_table->sensor_set_exposure_compensation(sctrl,
        sensor_set->data.exposure);
    }
    break;
  case SENSOR_SET_ISO:
    if (sctrl->fn_table->sensor_set_iso)
      sctrl->fn_table->sensor_set_iso(sctrl, sensor_set->data.iso);
    break;
  case SENSOR_SET_SPECIAL_EFFECT:
    if (sctrl->fn_table->sensor_set_special_effect) {
      sctrl->fn_table->sensor_set_special_effect(sctrl,
        sensor_set->data.effect);
    }
    break;
  case SENSOR_SET_WHITEBALANCE:
    if (sctrl->fn_table->sensor_set_wb_oem) {
      sctrl->fn_table->sensor_set_wb_oem(sctrl,
        sensor_set->data.whitebalance);
    }
    break;
  case SENSOR_SET_START_STREAM:
    if (sctrl->fn_table->sensor_set_start_stream)
      sctrl->fn_table->sensor_set_start_stream(sctrl);
    break;
  case SENSOR_SET_STOP_STREAM:
    if (sctrl->fn_table->sensor_set_stop_stream)
      sctrl->fn_table->sensor_set_stop_stream(sctrl);
    break;
  case SENSOR_SET_OEM:
    if (sctrl->fn_table->sensor_set_config_setting)
      sctrl->fn_table->sensor_set_config_setting(sctrl,
        sensor_set->data.oem_setting);
    break;
  }
  if (status < 0) {
    CDBG_ERROR("%s: failed %d\n", __func__, __LINE__);
  }
  return status;
}
/*===========================================================================
 * FUNCTION    - sensor_get_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int sensor_client_get_params(uint32_t handle, int type,
                                    void *parm, int parm_len)
{
  sensor_get_t *sensor_get = parm;
  sensor_client_t *sensor_client = get_sensor_client_info(handle);
  if(!sensor_client) {
    CDBG_ERROR("%s: failed %d\n", __func__, __LINE__);
    return -EINVAL;
  }

  sensor_ctrl_t *sctrl = sensor_client->sensorCtrl;

  switch(type) {
  case SENSOR_GET_DIM_INFO:
    if (sctrl->fn_table->sensor_get_dim_info) {
      sctrl->fn_table->sensor_get_dim_info(sctrl, &sensor_get->data.sensor_dim);
    }
    break;
  case SENSOR_GET_CHROMATIX_PTR:
    sensor_get->data.chromatix_ptr = sctrl->sensor.out_data.chromatix_ptr;
    break;
  case SENSOR_GET_CAMIF_CFG:
    if (sctrl->fn_table->sensor_get_camif_cfg) {
      sctrl->fn_table->sensor_get_camif_cfg(sctrl,
        &sensor_get->data.camif_setting);
    }
    break;
  case SENSOR_GET_OUTPUT_CFG:
    if (sctrl->fn_table->sensor_get_output_cfg) {
      sctrl->fn_table->sensor_get_output_cfg(sctrl, &sensor_get->data);
    }
    break;
  case SENSOR_GET_SENSOR_MODE_AEC_INFO:
    if (sctrl->fn_table->sensor_get_mode_aec_info) {
      sctrl->fn_table->sensor_get_mode_aec_info(sctrl,
        &sensor_get->data.aec_info);
    }
    break;
  case SENSOR_GET_DIGITAL_GAIN:
    if (sctrl->fn_table->sensor_get_digital_gain) {
      sctrl->fn_table->sensor_get_digital_gain(sctrl,
        &sensor_get->data.aec_info.digital_gain);
    }
    break;
  case SENSOR_GET_SENSOR_MAX_AEC_INFO:
    sensor_get->data.aec_info.max_gain =
      sctrl->sensor.out_data.aec_info.max_gain;
    sensor_get->data.aec_info.max_linecount =
      sctrl->sensor.out_data.aec_info.max_linecount;
    break;
  case SENSOR_GET_PREVIEW_FPS_RANGE:
    if (!sctrl->fn_table->sensor_get_preview_fps_range)
      return -1;
    sctrl->fn_table->sensor_get_preview_fps_range
      (sctrl, &sensor_get->data.fps_range);
    break;
  case SENSOR_GET_PENDING_FPS:
    if (sctrl->sensor.op_mode != SENSOR_MODE_INVALID) {
      sensor_get->data.get_pending_fps = sctrl->sensor.pending_fps;
    } else {
      sensor_get->data.get_pending_fps = 0;
    }
    break;
  case SENSOR_GET_CHROMATIX_TYPE:
    sensor_get->data.chromatix_type = sctrl->chromatixType;
    break;
  case SENSOR_GET_MAX_SUPPORTED_HFR_MODE:
    if (sctrl->fn_table->sensor_get_max_supported_hfr_mode) {
      sctrl->fn_table->sensor_get_max_supported_hfr_mode
        (sctrl, &sensor_get->data.max_supported_hfr_mode);
    } else {
      sensor_get->data.max_supported_hfr_mode = CAMERA_HFR_MODE_OFF;
    }
    break;
  case SENSOR_GET_CSI_PARAMS:
    if(sctrl->fn_table->sensor_get_csi_params)
      sctrl->fn_table->sensor_get_csi_params(sctrl,
        &sensor_get->data.sensor_csi_params);
    break;
  case SENSOR_GET_LENS_INFO:
    if (!sctrl->fn_table->sensor_get_lens_info)
      return -1;
    sctrl->fn_table->sensor_get_lens_info(sctrl, &sensor_get->data.lens_info);
    break;
  case SENSOR_GET_CUR_FPS:
    if (!sctrl->fn_table->sensor_get_cur_fps)
      return -EINVAL;
    sctrl->fn_table->sensor_get_cur_fps(sctrl, &sensor_get->data.fps);
    break;
  case SENSOR_GET_CUR_RES:
    if(sctrl->fn_table->sensor_get_cur_res) {
      sctrl->fn_table->sensor_get_cur_res(sctrl, sensor_get->data.op_mode,
        &sensor_get->data.cur_res);
    }
    break;
  case SENSOR_GET_SENSOR_INFO:
    sensor_get->sinfo = sctrl->sinfo;
    break;
  }
  return 0;
}

/*===========================================================================
 * FUNCTION    - sensor_interface_destroy -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int sensor_client_destroy(uint32_t handle)
{
  sensor_client_t *sensor_client = get_sensor_client_info(handle);

  pthread_mutex_lock(&sensor_controller.mutex);
  if(!sensor_client) {
    pthread_mutex_unlock(&sensor_controller.mutex);
    return 0;
  }

  free(sensor_client->sensorCtrl);
  memset(sensor_client, 0, sizeof(sensor_client_t));
  pthread_mutex_unlock(&sensor_controller.mutex);
  return 0;
}

/*============================================================================
 * FUNCTION    - STATSPROC_interface_create -
 *
 * DESCRIPTION:
 *===========================================================================*/
uint32_t sensor_client_open(module_ops_t *ops)
{
  uint32_t handle = 0;
  int idx;
  sensor_client_t *sensor_client = NULL;

  if(!ops) {
    CDBG_ERROR("%s: failed %d\n", __func__, __LINE__);
    return -EINVAL;
  }
  memset(ops, 0, sizeof(module_ops_t));

  pthread_mutex_lock(&sensor_controller.mutex);
  for(idx = 0; idx < SENSOR_MAX_CLIENT_NUM; idx++) {
    if(sensor_controller.client[idx].handle == 0) {
      sensor_client = &sensor_controller.client[idx];
      break;
    }
  }

  /* if not found return null */
  if (!sensor_client) {
    pthread_mutex_unlock(&sensor_controller.mutex);
    return (uint32_t)NULL;
  } else {
    memset(sensor_client, 0, sizeof(sensor_client_t));
    sensor_client->obj_idx_mask = 0;
    sensor_client->client_idx = idx;
    sensor_client->my_comp_id = MCTL_COMPID_SENSOR;
    sensor_client->handle = sensor_gen_handle((uint8_t)idx);
    sensor_client->sensorCtrl = (sensor_ctrl_t *) malloc(sizeof(sensor_ctrl_t));

    if(NULL == sensor_client->sensorCtrl) {
      pthread_mutex_unlock(&sensor_controller.mutex);
      return (uint32_t)NULL;
    }

    memset(sensor_client->sensorCtrl, 0, sizeof(sensor_ctrl_t));
    sensor_client->sensorCtrl->sensor.op_mode = SENSOR_MODE_INVALID;

    ops->handle = (uint32_t)sensor_client->handle;
    ops->init = sensor_client_init;
    ops->set_params = sensor_client_set_params;
    ops->get_params = sensor_client_get_params;
    ops->process = NULL;
    ops->abort = NULL;
    ops->destroy= sensor_client_destroy;
  }
  pthread_mutex_unlock(&sensor_controller.mutex);

  CDBG("%s: client_idx = %d, handle = 0x%x", __func__, idx,
    sensor_client->handle);

  return sensor_client->handle;
}/*stats_interface_create*/

/*===========================================================================
 * FUNCTION    - CAMIF_comp_create -
 *
 * DESCRIPTION: initialize AXI component.
 *==========================================================================*/
int sensor_comp_create()
{
  memset(&sensor_controller, 0, sizeof(sensor_controller));
  pthread_mutex_init(&sensor_controller.mutex, NULL);
  return 0;
}
/*===========================================================================
 * FUNCTION    - CAMIF_comp_destroy -
 *
 * DESCRIPTION: destroy the component
 *==========================================================================*/
int sensor_comp_destroy()
{
  pthread_mutex_destroy(&sensor_controller.mutex);
  memset(&sensor_controller, 0, sizeof(sensor_controller));
  return 0;
}
