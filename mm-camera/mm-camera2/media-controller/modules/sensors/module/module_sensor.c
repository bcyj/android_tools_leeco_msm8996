/* module_sensor.c
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <linux/media.h>
#include "mct_module.h"
#include "module_sensor.h"
#include "modules.h"
#include "mct_stream.h"
#include "mct_pipeline.h"
#include "media_controller.h"
#include "mct_event_stats.h"
#include "actuator_driver.h"
#include "af_algo_tuning.h"
#include "port_sensor.h"
#include "sensor_util.h"
#include <poll.h>
#include "sensor_init.h"
#include <../stats/q3a/q3a_stats_hw.h>
#include "led_flash/led_flash.h"
#include "server_debug.h"

/** Initialization table **/
static int32_t (*sub_module_init[])(sensor_func_tbl_t *) = {
  [SUB_MODULE_SENSOR]       = sensor_sub_module_init,
  [SUB_MODULE_CHROMATIX]    = chromatix_sub_module_init,
  [SUB_MODULE_ACTUATOR]     = actuator_sub_module_init,
  [SUB_MODULE_EEPROM]       = eeprom_sub_module_init,
  [SUB_MODULE_LED_FLASH]    = led_flash_sub_module_init,
  [SUB_MODULE_STROBE_FLASH] = strobe_flash_sub_module_init,
  [SUB_MODULE_CSIPHY]       = csiphy_sub_module_init,
  [SUB_MODULE_CSIPHY_3D]    = csiphy_sub_module_init,
  [SUB_MODULE_CSID]         = csid_sub_module_init,
  [SUB_MODULE_CSID_3D]      = csid_sub_module_init,
  [SUB_MODULE_OIS]          = NULL,
};


static boolean module_sensor_pass_op_clk_change(void *data1, void *data2)
{
  int32_t                     rc = 0;
  module_sensor_bundle_info_t *s_bundle = (module_sensor_bundle_info_t *)data1;
  module_sensor_params_t      *sensor = NULL;

  if (!s_bundle || !data2) {
    SERR("failed: s_bundle %p data2 %p", s_bundle, data2);
    /* Return TRUE here, else mct_list_traverse will terminate */
    return TRUE;
  }

  sensor = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  rc = sensor->func_tbl.process(sensor->sub_module_private,
    SENSOR_SET_OP_PIXEL_CLK_CHANGE, data2);
  if (rc < 0) {
    SERR("failed");
  }

  return TRUE;
}

/** module_sensor_offload_lens_reset: Reset lens
 *
 *  @param1: actuator module params
 *  @param2: not used
 *  @param3: not used
 *  @param4: if not null is pointer to mutex for signaling end of lens movement
 *
 *  This function is used to update reset the lens
 *
 *  NOTE:
 *  It is supposed to be execured in sensor thread,
 *  not to block thread that sends SOF
 *
 *  Return: nothing
 **/
static void module_sensor_offload_lens_reset(void* param1,
  void* param2 __attribute__((unused)),
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)))
{
  af_update_t            af_update;
  module_sensor_params_t *actuator_module_params;
  sensor_af_bracket_t    *bracket = NULL;
  int32_t ret = -1;

  if (param1 && param3) {
    actuator_module_params = param1;
    bracket = (sensor_af_bracket_t*)param3;

    /* Move Lens in Actuator */
    af_update.reset_lens = TRUE;

    if (actuator_module_params->func_tbl.process != NULL) {
      ret = actuator_module_params->func_tbl.process(
        actuator_module_params->sub_module_private,
        ACTUATOR_MOVE_FOCUS, &af_update);
    }

    if (ret < 0) {
      SERR("Fail to move lens");
    }

    pthread_mutex_lock(&bracket->lens_move_done_sig);
    bracket->lens_move_progress = FALSE;
    pthread_mutex_unlock(&bracket->lens_move_done_sig);
  } else {
    SERR("Null pointer detected");
  }
}

/** module_sensor_offload_lens_move: Move lens
 *
 *  @param1: actuator module params
 *  @param2: direction
 *  @param3: number of steps
 *  @param4: if not null is pointer to mutex for signaling end of lens movement
 *
 *  This function is used to update move the lens in specified direction
 *  for specified number of steps
 *
 *  NOTE:
 *  It is supposed to be execured in sensor thread,
 *  not to block thread that sends SOF
 *
 *  Return: nothing
 **/
static void module_sensor_offload_lens_move(void* param1,
  void* param2,
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)))
{
  af_update_t               af_update;
  module_sensor_params_t    *actuator_module_params;
  sensor_af_bracket_t       *bracket;
  sensor_frame_order_ctrl_t *ctrl;
  int32_t ret = -1;

  if (param1 && param3) {
    actuator_module_params = param1;
    bracket = (sensor_af_bracket_t*)param3;

    /* Move Lens in Actuator */
    af_update.move_lens    = TRUE;
    af_update.direction    = (int)param2;
    af_update.reset_lens   = FALSE;
    af_update.reset_pos    = FALSE;
    af_update.stop_af      = FALSE;

    if (param4) {
       ctrl = (sensor_frame_order_ctrl_t*)param4;
       af_update.num_of_steps =
          (int32_t)bracket->abs_steps_to_move[ctrl->captured_count-1];
    } else {
       af_update.num_of_steps = (int32_t)bracket->num_steps;
    }
    if (actuator_module_params->func_tbl.process != NULL) {
      ret = actuator_module_params->func_tbl.process(
        actuator_module_params->sub_module_private,
        ACTUATOR_MOVE_FOCUS, &af_update);
    }

    if (ret < 0) {
      SERR("Fail to move lens");
    }

    pthread_mutex_lock(&bracket->lens_move_done_sig);
    bracket->lens_move_progress = FALSE;
    pthread_mutex_unlock(&bracket->lens_move_done_sig);
  } else {
    SERR("Null pointer detected");
  }
}

/** module_sensor_handle_pixel_clk_change: handle pixel clk
 *  change event sent by ISP
 *
 *  @module: sensor module
 *  @data: event control data
 *
 *  This function handles stores op pixel clk value in module
 *  private
 *
 *  Return: TRUE for success and FALSE for failure
 *  **/

boolean module_sensor_handle_pixel_clk_change(mct_module_t *module,
  uint32_t identity, void *data)
{
  int32_t                     rc = 0;
  boolean                     ret = TRUE;
  module_sensor_ctrl_t        *module_ctrl = NULL;
  uint32_t                    i = 0;
  module_sensor_params_t      *sensor = NULL;
  sensor_bundle_info_t         bundle_info;

  if (!module || !data) {
    SERR("failed: module %p data %p", module, data);
    return FALSE;
  }

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  if (!module_ctrl) {
    SERR("failed: module_ctrl %p", module_ctrl);
    return FALSE;
  }

  mct_list_traverse(module_ctrl->sensor_bundle,
    module_sensor_pass_op_clk_change, data);

  return ret;
}

/** module_sensors_subinit: sensor module init function
 *
 *  @data: sensor bundle data for first sensor
 *  @user_data: NULL
 *
 *  Return: 0 for success and negative error for failure
 *
 *  This function allocates memory to hold module_sensor_prams_t struct for each
 *  sub module and calls init to initialize function table **/

static boolean module_sensors_subinit(void *data, void *user_data)
{
  int32_t rc = SENSOR_SUCCESS, i = 0;
  module_sensor_bundle_info_t *s_bundle = (module_sensor_bundle_info_t *)data;
  if (!s_bundle) {
    SERR("failed");
    return FALSE;
  }
  s_bundle->sensor_lib_params = malloc(sizeof(sensor_lib_params_t));
  if (!s_bundle->sensor_lib_params) {
    SERR("failed");
    return FALSE;
  }
  memset(s_bundle->sensor_lib_params, 0, sizeof(sensor_lib_params_t));

  for (i = 0; i < SUB_MODULE_MAX; i++) {
    s_bundle->module_sensor_params[i] = malloc(sizeof(module_sensor_params_t));
    if (!s_bundle->module_sensor_params[i]) {
      SERR("failed");
      goto ERROR;
    }
    memset(s_bundle->module_sensor_params[i], 0,
      sizeof(module_sensor_params_t));
    if (s_bundle->sensor_info->subdev_id[i] != -1) {
      SLOW("i %d subdev name %s strlen %d", i, s_bundle->sensor_sd_name[i],
        strlen(s_bundle->sensor_sd_name[i]));
      if (!strlen(s_bundle->sensor_sd_name[i]) &&
         ((i == SUB_MODULE_ACTUATOR) || (i == SUB_MODULE_EEPROM) ||
          (i == SUB_MODULE_LED_FLASH) || (i == SUB_MODULE_STROBE_FLASH) ||
          (i == SUB_MODULE_OIS))) {
        SERR("session %d subdev %d not present, reset to -1",
          s_bundle->sensor_info->session_id, i);
        s_bundle->sensor_info->subdev_id[i] = -1;
      } else {
        if (!sub_module_init[i])
          continue;
        rc = sub_module_init[i](&s_bundle->module_sensor_params[i]->func_tbl);
        if (rc < 0 || !s_bundle->module_sensor_params[i]->func_tbl.open ||
            !s_bundle->module_sensor_params[i]->func_tbl.process ||
            !s_bundle->module_sensor_params[i]->func_tbl.close) {
          SERR("failed");
          goto ERROR;
        }
      }
    }
  }
  return TRUE;

ERROR:
  for (i--; i >= 0; i--)
    free(s_bundle->module_sensor_params[i]);
  SERR("failed");
  return FALSE;
}

/** module_sensor_frame_ctrl_release: Release the queued
 *  frame control data
 *
 *  @data: frame control data pointer
 *  @user_data: NULL
 *
 *  Return: TRUE for success and FALSE for failure
 *
 *  This function is called during deinit. It called for each node in
 *  the queue and frees all the memory associated with frame control
 *  parameters**/
static boolean module_sensor_frame_ctrl_release(void *data, void *user_data)
{
  sensor_frame_ctrl_data_t *sensor_frame_ctrl_data =
    (sensor_frame_ctrl_data_t *)data;
  if (sensor_frame_ctrl_data) {
    free(sensor_frame_ctrl_data);
    sensor_frame_ctrl_data = NULL;
  }
  return TRUE;
}

static boolean module_sensor_actuator_init_calibrate(
    module_sensor_bundle_info_t *s_bundle)
{
    int32_t                  rc = 0;
    char                     *a_name = NULL;
    sensor_get_af_algo_ptr_t af_algo;
    eeprom_set_chroma_af_t   eeprom_set;
    af_algo_tune_parms_t     *af_algo_cam_ptr = NULL;
    actuator_driver_params_t *af_driver_ptr = NULL;

    /* Get actuator name from sensor */
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_GET_ACTUATOR_NAME, &a_name, rc);
    if (rc < 0) {
        SERR("sensor_failure : SENSOR_GET_ACTUATOR_NAME failed");
        return FALSE;
    }

    /* Initialize the actuator */
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
        ACTUATOR_INIT, a_name, rc);
    if (rc < 0) {
        SERR("sensor_failure : ACTUATOR_INIT failed");
        return FALSE;
    }

    /* Get diver param from actuator */
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
        ACTUATOR_GET_AF_DRIVER_PARAM_PTR, &af_driver_ptr, rc);
    if (rc < 0 || af_driver_ptr == NULL) {
        SERR("sensor_failure : ACTUATOR_GET_AF_DRIVER_PARAM_PTR failed");
        return FALSE;
    }

    /* Set driver param to eeprom */
    eeprom_set.af_driver_ptr = af_driver_ptr;

    /* Get camera af_algo param */
    af_algo.cam_mode = ACTUATOR_CAM_MODE_CAMERA;
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
        ACTUATOR_GET_AF_ALGO_PARAM_PTR, &af_algo, rc);
    if (rc < 0 || af_algo.af_tune_ptr == NULL) {
        SERR("sensor_failure : ACTUATOR_GET_AF_ALGO_PARAM_PTR failed");
        return FALSE;
    }

    /* Set camera af algo to eeprom */
    eeprom_set.af_algo_ptr[0] = af_algo.af_tune_ptr;

    /* Get camcorder af_algo param */
    af_algo.cam_mode = ACTUATOR_CAM_MODE_CAMCORDER;
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
        ACTUATOR_GET_AF_ALGO_PARAM_PTR, &af_algo, rc);
    if (rc < 0 || af_algo.af_tune_ptr == NULL) {
        SERR("sensor_failure : ACTUATOR_GET_AF_DRIVER_PARAM_PTR failed");
        return FALSE;
    }

    /* Set camera af algo to eeprom */
    eeprom_set.af_algo_ptr[1] = af_algo.af_tune_ptr;
    /* Perform calibration if eeprom is present */
    if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] != -1)
    {
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
            EEPROM_CALIBRATE_FOCUS_DATA, &eeprom_set, rc);
        if (rc < 0) {
            SERR("sensor_failure : EEPROM_SET_CALIBRATE_FOCUS failed");
            return FALSE;
        }
    }

    return TRUE;
}

/** module_sensor_init_session: init session function for sensor
 *
 *  @s_bundle: sensor bundle pointer pointing to the sensor
 *             for which stream is added
 *
 *  Return: 0 for success and negative error for failure
 *
 *  When called first time, this function
 *  1) opens all sub modules to open subdev node
 *  2) loads sensor library
 *  3) calls init on sensor, csiphy and csid. Has ref count to
 *  ensure that actual add stream sequence is executed only
 *  once **/

static boolean module_sensor_init_session(module_sensor_bundle_info_t *s_bundle)
{
  int32_t                 rc = SENSOR_SUCCESS, i = 0, j = 0;
  module_sensor_params_t  *module_sensor_params = NULL;
  module_sensor_params_t  *actuator_module_params = NULL;
  module_sensor_params_t  *csiphy_module_params = NULL;
  module_sensor_params_t  *csid_module_params = NULL;
  module_sensor_params_t  *eeprom_module_params = NULL;
  module_sensor_params_t  *flash_module_params = NULL;
  module_sensor_params_t  *ois_module_params = NULL;
  sensor_get_t             sensor_get;
  sensor_lens_info_t       lens_info;
  enum sensor_sub_module_t s_module;
  af_algo_tune_parms_t     *af_algo_cam_ptr = NULL;
  af_algo_tune_parms_t     *af_algo_camcorder_ptr = NULL;
  actuator_driver_params_t *af_driver_ptr = NULL;
  boolean                   status = TRUE;
  if (s_bundle->ref_count++) {
    SLOW("ref_count %d", s_bundle->ref_count);
    return TRUE;
  }

  /* Initialize max width, height and stream on count */
  s_bundle->max_width = 0;
  s_bundle->max_height = 0;
  s_bundle->stream_on_count = 0;
  s_bundle->stream_mask = 0;
  s_bundle->last_idx = 0;
  s_bundle->num_skip = 4;
  s_bundle->state = 0;
  s_bundle->regular_led_trigger = 0;
  s_bundle->regular_led_af = 0;
  s_bundle->torch_on = 0;
  s_bundle->longshot = 0;
  s_bundle->stream_thread_wait_time = 2.5;

  /* Initialize frame control queues*/
  for (j = 0; j < FRAME_CTRL_SIZE; j++) {
    s_bundle->frame_ctrl.frame_ctrl_q[j] =
      (mct_queue_t*)calloc(1, sizeof(mct_queue_t));
    if(!s_bundle->frame_ctrl.frame_ctrl_q[j]) {
      SERR("%s:%d, calloc failed", __func__, __LINE__);
      goto ERROR0;
    }
    mct_queue_init(s_bundle->frame_ctrl.frame_ctrl_q[j]);
    /* Initialize frame control mutex */
    pthread_mutex_init(&s_bundle->frame_ctrl.frame_ctrl_mutex[j], NULL);
  }

  /* Initialize the mutex*/
  pthread_mutex_init(&s_bundle->mutex, NULL);

  for (i = 0; i < SUB_MODULE_MAX; i++) {
    SLOW("sensor_sd_name=%s", s_bundle->sensor_sd_name[i]);
    if (s_bundle->module_sensor_params[i]->func_tbl.open) {
      rc = s_bundle->module_sensor_params[i]->func_tbl.open(
        &s_bundle->module_sensor_params[i]->sub_module_private,
        s_bundle->sensor_sd_name[i]);
      if (rc < 0) {
        SERR("failed rc %d", rc);
        goto ERROR1;
      }
    }
  }

  /* Load sensor library*/
  rc = sensor_load_library(s_bundle->sensor_info->sensor_name,
    s_bundle->sensor_lib_params);
  if (rc < 0) {
    SERR("failed rc %d", rc);
    goto ERROR1;
  }

  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  eeprom_module_params = s_bundle->module_sensor_params[SUB_MODULE_EEPROM];
  /* set sensor_lib_params to sensor sub module */
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_SET_LIB_PARAMS, s_bundle->sensor_lib_params);
  if (rc < 0) {
    SERR("failed rc %d", rc);
    goto ERROR2;
  }

  /* set sensor_info to sensor sub module */
  if (s_bundle->sensor_info->is_mount_angle_valid == 1) {
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_MOUNT_ANGLE, &s_bundle->sensor_info->sensor_mount_angle);
    if (rc < 0) {
      SERR("failed rc %d", rc);
      goto ERROR2;
    }
  }

  /* set sensor_info to sensor sub module */
  if (s_bundle->sensor_info->position != INVALID_CAMERA_B) {
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_POSITION, &s_bundle->sensor_info->position);
    if (rc < 0) {
      SERR("failed rc %d", rc);
      goto ERROR2;
    }
  }

  /* set sensor_info to sensor sub module */
  if (s_bundle->sensor_info->modes_supported != CAMERA_MODE_INVALID) {
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_MODE, &s_bundle->sensor_info->modes_supported);
    if (rc < 0) {
      SERR("failed rc %d", rc);
      goto ERROR2;
    }
  }

  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_GET_PER_FRAME_INFO, &s_bundle->frame_ctrl.delay_info);
  if (rc < 0) {
    SERR("failed rc %d", rc);
    goto ERROR2;
  }

  s_bundle->sensor_params.aperture_value = 0;
  memset(&lens_info, 0, sizeof(lens_info));
  /* get lens info to sensor sub module */
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_GET_LENS_INFO, &lens_info);
  if (rc < 0) {
    SERR("failed rc %d", rc);
  } else {
    /* Fill aperture */
    s_bundle->sensor_params.aperture_value = lens_info.f_number;
    s_bundle->sensor_params.f_number       = lens_info.f_number;
    s_bundle->sensor_params.focal_length   = lens_info.focal_length;
    s_bundle->sensor_params.sensing_method = lens_info.sensing_method;
    s_bundle->sensor_params.crop_factor    = lens_info.crop_factor;
    SLOW("aperture %f", s_bundle->sensor_params.aperture_value);
  }

   /* set eeeprom data */
/* need to check whether to set the data loaded from kernel
   to be user space should be used or eeprom gets data during open close??
   also */

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] != -1) {
    rc = eeprom_module_params->func_tbl.process(
      eeprom_module_params->sub_module_private,
      EEPROM_SET_BYTESTREAM, &(s_bundle->eeprom_data->eeprom_params));
      if (rc < 0) {
        SERR("failed rc %d", rc);
      }
    rc = eeprom_module_params->func_tbl.process(
      eeprom_module_params->sub_module_private,
      EEPROM_SET_FORMAT_DATA, NULL);
    if (rc < 0) {
        SERR("failed rc %d", rc);
    }
  }

  /* get eeeprom data */
  /* af_actuator_init */
  /* get actuator header */
  /* sensor init */
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_INIT, NULL);
  if (rc < 0) {
    SERR("failed rc %d", rc);
    goto ERROR2;
  }

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
    status = module_sensor_actuator_init_calibrate(s_bundle);
    if (status != TRUE) {
        SERR("sensor_failure : module_sensor_actuator_init_calibrate failed");
        goto ERROR2;
    }
  }

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] != -1) {
    struct msm_camera_i2c_reg_setting cal_setting;
    int is_insensor;
    /* EEPROM IN-SENSOR CALIBRATION*/

    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
      EEPROM_GET_ISINSENSOR_CALIB, &is_insensor, rc);
    if (rc < 0) {
      SERR("failed");
      goto ERROR2;
    }
    if (is_insensor) {
      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
        EEPROM_GET_RAW_DATA, &cal_setting, rc);
      if (rc < 0) {
        SERR("Get raw data failed");
        goto ERROR2;
      }
      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_SET_CALIBRATION_DATA, &cal_setting, rc);
      if (rc < 0) {
        SERR("Set calibration data failed");
        goto ERROR2;
      }
    }
  }

  /* get actuator header */

  /* Get CSI lane params from sensor */
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_GET_CSI_LANE_PARAMS, &sensor_get);
  if (rc < 0) {
    SERR("failed rc %d", rc);
    goto ERROR2;
  }

  csiphy_module_params = s_bundle->module_sensor_params[SUB_MODULE_CSIPHY];
  /* Set csi lane params on csiphy */
  rc = csiphy_module_params->func_tbl.process(
    csiphy_module_params->sub_module_private,
    CSIPHY_SET_LANE_PARAMS, sensor_get.csi_lane_params);
  if (rc < 0) {
    SERR("failed rc %d", rc);
    goto ERROR2;
  }

  csid_module_params = s_bundle->module_sensor_params[SUB_MODULE_CSID];
  /* Set csi lane params on csid */
  rc = csid_module_params->func_tbl.process(
    csid_module_params->sub_module_private,
    CSID_SET_LANE_PARAMS, sensor_get.csi_lane_params);
  if (rc < 0) {
    SERR("failed rc %d", rc);
    goto ERROR2;
  }
  SHIGH("Success");
  return TRUE;

ERROR2:
  sensor_unload_library(s_bundle->sensor_lib_params);
ERROR1:
  for (i--; i >= 0; i--) {
    if (s_bundle->module_sensor_params[i]->func_tbl.close) {
      s_bundle->module_sensor_params[i]->func_tbl.close(
        s_bundle->module_sensor_params[i]->sub_module_private);
    }
  }
ERROR0:
  s_bundle->ref_count--;
  for (j--; j >= 0; j--) {
    /* Free the frame control queue*/
    mct_queue_free(s_bundle->frame_ctrl.frame_ctrl_q[j]);
    s_bundle->frame_ctrl.frame_ctrl_q[j] = NULL;
    /* Destroy frame control mutex */
    pthread_mutex_destroy(&s_bundle->frame_ctrl.frame_ctrl_mutex[j]);
  }
  SERR("failed");
  return FALSE;
}

/** module_sensor_deinit_session: deinit session function for
 *  sensor
 *
 *  @s_bundle: sensor bundle pointer pointing to the sensor
 *             for which stream is added
 *  @port: port
 *
 *  Return: 0 for success and negative error for failure
 *
 *  This function calls close on all sub modules to close
 *  subdev node. Also, unloads sensor library. Has ref count
 *  to ensure that actual close happens only when last stream
 *  is removed */

static boolean module_sensor_deinit_session(
  module_sensor_bundle_info_t *s_bundle)
{
  int16_t i = 0;
  sensor_frame_ctrl_data_t *sensor_frame_ctrl_data = NULL;

  if (!s_bundle->ref_count) {
    SERR("ref count 0");
    return FALSE;
  }

  if (--s_bundle->ref_count) {
    SLOW("ref_count %d", s_bundle->ref_count);
    return TRUE;
  }

   for (i = SUB_MODULE_MAX-1; i >= 0; i--) {
    if (s_bundle->module_sensor_params[i]->func_tbl.close) {
      s_bundle->module_sensor_params[i]->func_tbl.close(
        s_bundle->module_sensor_params[i]->sub_module_private);
    }
    s_bundle->module_sensor_params[i]->sub_module_private = NULL;
  }
  sensor_unload_library(s_bundle->sensor_lib_params);

  /* Destroy the mutex */
  pthread_mutex_destroy(&s_bundle->mutex);

  for (i = 0; i < FRAME_CTRL_SIZE; i++) {
    pthread_mutex_lock(&s_bundle->frame_ctrl.frame_ctrl_mutex[i]);
    /* Flush Frame Control queue*/
    mct_queue_flush(s_bundle->frame_ctrl.frame_ctrl_q[i],
      module_sensor_frame_ctrl_release);
    pthread_mutex_unlock(&s_bundle->frame_ctrl.frame_ctrl_mutex[i]);
    /* Free frame control queue */
    mct_queue_free(s_bundle->frame_ctrl.frame_ctrl_q[i]);
    s_bundle->frame_ctrl.frame_ctrl_q[i] = NULL;
    /* Destroy frame control mutex */
    pthread_mutex_destroy(&s_bundle->frame_ctrl.frame_ctrl_mutex[i]);
  }

  return TRUE;
}

/** module_sensor_start_session:
 *
 *  @module: sensor module
 *  @sessionid: session id
 *
 *  Return: TRUE for success and FALSE for failure
 **/
static boolean module_sensor_start_session(
  mct_module_t *module, unsigned int sessionid)
{
  module_sensor_ctrl_t        *module_ctrl = NULL;
  mct_list_t                  *s_list = NULL;
  module_sensor_bundle_info_t *s_bundle = NULL;
  boolean                     ret = TRUE;

  SHIGH("session %d", sessionid);
  if (!module) {
    SERR("failed");
    return FALSE;
  }

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  if (!module_ctrl) {
    SERR("failed");
    return FALSE;
  }

  /* get the s_bundle from session id */
  s_list = mct_list_find_custom(module_ctrl->sensor_bundle, &sessionid,
    sensor_util_find_bundle);
  if (!s_list) {
    SERR("failed");
    return FALSE;
  }
  s_bundle = (module_sensor_bundle_info_t *)s_list->data;
  if (!s_bundle) {
    SERR("failed");
    return FALSE;
  }

  /* initialize the "torch on" flag to 0 */
  s_bundle->torch_on = 0;
  s_bundle->longshot = 0;

  /* this init session includes
     power up sensor, config init setting */
  ret = module_sensor_init_session(s_bundle);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR;
  }

  /* Create a Sensor thread */
  ret = sensor_thread_create(module);

  if(FALSE != ret) {
     SERR("failed");
     goto ERROR;
  }

  return TRUE;
ERROR:
  SERR("failed");
  return FALSE;
}

/** module_sensor_stop_session:
 *
 *  @module: sensor module
 *  @sessionid: session id
 *
 *  Return: 0 for success and negative error on failure
 **/
static boolean module_sensor_stop_session(
  mct_module_t *module, unsigned int sessionid)
{
  module_sensor_ctrl_t        *module_ctrl = NULL;
  mct_list_t                  *s_list = NULL;
  module_sensor_bundle_info_t *s_bundle = NULL;
  boolean                     ret = TRUE;

  SHIGH("session %d", sessionid);
  if (!module) {
    SERR("failed");
    return FALSE;
  }
  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  if (!module_ctrl) {
    SERR("failed");
    return FALSE;
  }

  /* get the s_bundle from session id */
  s_list = mct_list_find_custom(module_ctrl->sensor_bundle, &sessionid,
    sensor_util_find_bundle);
  if (!s_list) {
    SERR("failed");
    return FALSE;
  }
  s_bundle = (module_sensor_bundle_info_t *)s_list->data;
  if (!s_bundle) {
    SERR("failed");
    return FALSE;
  }

  /* this deinit session includes
     power off sensor */
  ret = module_sensor_deinit_session(s_bundle);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR;
  }
 /* Terminate sensor thread */
  sensor_thread_msg_t msg;
    msg.stop_thread = TRUE;
   int32_t nwrite = 0;
   nwrite = write(module_ctrl->pfd[1], &msg, sizeof(sensor_thread_msg_t));
   if(nwrite < 0)
   {
     SERR("%s: Writing into fd failed",__func__);
   }

  return TRUE;
ERROR:
  SERR("failed");
  return FALSE;
}

static int32_t module_sensor_get_stats_data(mct_module_t *module,
  uint32_t identity, stats_get_data_t* stats_get)
{
  boolean rc;
  memset(stats_get, 0x00, sizeof(stats_get_data_t));
  mct_event_t new_event;
  new_event.type = MCT_EVENT_MODULE_EVENT;
  new_event.identity = identity;
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.module_event.type = MCT_EVENT_MODULE_STATS_GET_DATA;
  new_event.u.module_event.module_event_data = (void *)stats_get;
  rc = sensor_util_post_event_on_src_port(module, &new_event);
  if (rc == FALSE){
    SERR("failed");
    return -EFAULT;
  }
  return 0;
}

static boolean module_sensor_is_ready_for_stream_on(mct_port_t *port,
  mct_event_t *event, sensor_set_res_cfg_t *res_cfg,
  module_sensor_bundle_info_t *s_bundle, int32_t bundle_id)
{
  boolean            is_bundle_started = TRUE;
  mct_stream_info_t* stream_info = (mct_stream_info_t*)
                              event->u.ctrl_event.control_event_data;
  uint32_t identity = event->identity;
  uint32_t session_id, stream_id;
  sensor_util_unpack_identity(identity, &session_id, &stream_id);

  SHIGH("session_id=%d, stream_id=%d", session_id, stream_id);

  /* find whether there is any bundle for this session that is already
     streamed ON  */
  is_bundle_started = sensor_util_find_is_any_bundle_started(port);
  SLOW("any bundle started %d", is_bundle_started);

  /* Find whether this stream belongs to bundle */
  if (bundle_id == -1) {
    /* This stream does not belong to any bundle */

    res_cfg->width = s_bundle->max_width;
    res_cfg->height = s_bundle->max_height;
    res_cfg->stream_mask = s_bundle->stream_mask;

    SLOW("s_bundle->stream_on_count %d", s_bundle->stream_on_count);
    SLOW("is_bundle_started %d", is_bundle_started);
    /* Start sensor streaming if this is the first stream ON and
       no other bundle has started */
    if ((s_bundle->stream_on_count == 0) && (is_bundle_started == FALSE)) {
      SLOW("non-bundled stream, count %d dim=%dx%d mask=0x%x",
        s_bundle->stream_on_count, res_cfg->width, res_cfg->height,
        res_cfg->stream_mask);
      s_bundle->stream_on_count++;
      return TRUE;
    } else {
      s_bundle->stream_on_count++;
      return FALSE;
    }
  }
  module_sensor_port_bundle_info_t *bundle_info;
  bundle_info = sensor_util_find_bundle_by_id(port, bundle_id);
  if (bundle_info == NULL) {
    SERR("can't find bundle with id=%d", bundle_id);
    return FALSE;
  }
  bundle_info->stream_on_count++;

  /* update the res_cfg with bundle dim and mask */
  res_cfg->width = s_bundle->max_width;
  res_cfg->height = s_bundle->max_height;
  res_cfg->stream_mask = s_bundle->stream_mask;

  SLOW("bundle_info->stream_on_count %d", bundle_info->stream_on_count);
  SLOW("bundle_info->bundle_config.num_of_streams %d",
    bundle_info->bundle_config.num_of_streams);
  SLOW("s_bundle->stream_on_count %d", s_bundle->stream_on_count);
  SLOW("is_bundle_started %d", is_bundle_started);
  /* If all streams in this bundle are on, we are ready to stream ON
     provided no other streams which is NOT part of bundle is STREAMED ON */
  if ((bundle_info->stream_on_count ==
       bundle_info->bundle_config.num_of_streams) &&
      (s_bundle->stream_on_count == 0) && (is_bundle_started == FALSE)) {
    SHIGH("stream_on_count=%d, w=%d, h=%d, stream_mask=%x",
      bundle_info->stream_on_count, res_cfg->width, res_cfg->height,
      res_cfg->stream_mask);
    return TRUE;
  }
  return FALSE;
}

static boolean module_sensor_is_ready_for_stream_off(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle)
{
  boolean            is_bundle_started = TRUE;
  mct_stream_info_t* stream_info = (mct_stream_info_t*)
                              event->u.ctrl_event.control_event_data;
  uint32_t identity = event->identity;
  int32_t bundle_id = -1;
  uint32_t session_id, stream_id;
  sensor_util_unpack_identity(identity, &session_id, &stream_id);
  SLOW("session_id=%d, stream_id=%d",
         session_id, stream_id);
  mct_port_t *port = sensor_util_find_src_port_with_identity(
                          module, identity);
  if (!port) {
    SERR("cannot find matching port with identity=0x%x", identity);
    return FALSE;
  }
  bundle_id = sensor_util_find_bundle_id_for_stream(port, identity);
  /* Find whether this stream is part of bundle */
  if (bundle_id == -1) {
    /* This stream does NOT belong to any bundle */
    s_bundle->stream_on_count--;

    /* find whether there is any bundle for this session that is already
       streamed ON  */
    is_bundle_started = sensor_util_find_is_any_bundle_started(port);
    SHIGH("any bundle started %d", is_bundle_started);

    /* Call sensor stream OFF only when all non bundle streams are streamed
       off AND no bundle is streaming */
    if ((s_bundle->stream_on_count == 0) && (is_bundle_started == FALSE)) {
      SLOW("non-bundled stream, stream count %d",
        s_bundle->stream_on_count);
      return TRUE;
    } else {
      SLOW("non-bundled stream, stream count %d",
        s_bundle->stream_on_count);
      return FALSE;
    }
  }
  module_sensor_port_bundle_info_t* bundle_info;
  bundle_info = sensor_util_find_bundle_by_id(port, bundle_id);
  if (bundle_info == NULL) {
    SERR("can't find bundle with id=%d",
                bundle_id);
    return FALSE;
  }
  /* decrement the counter */
  bundle_info->stream_on_count--;

  /* find whether there is any other bundle for this session that is already
     streamed ON  */
  is_bundle_started = sensor_util_find_is_any_bundle_started(port);
  SHIGH("any bundle started %d", is_bundle_started);

  /* If this stream is the last stream in the bundle to get stream_off,
     do sensor stream off provided no non bundle streams are streaming */
  if ((bundle_info->stream_on_count == 0) && (s_bundle->stream_on_count == 0) &&
      (is_bundle_started == FALSE)) {
    return TRUE;
  }
  SLOW("not needed, count=%d", bundle_info->stream_on_count);
  return FALSE;
}

static boolean module_sensor_config_meta(
  mct_stream_info_t* stream_info,
  sensor_set_res_cfg_t *stream_on_cfg)
{
  boolean   config_meta = FALSE;
  if (!stream_info || !stream_on_cfg) {
    SERR("failed");
    return FALSE;
  }

 /*For zsl or snapshot*/
  if ((stream_on_cfg->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT))
    &&  stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT)
    config_meta = TRUE;
  /*For non-zsl preview*/
  else if (!(stream_on_cfg->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT))
    && stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW)
    config_meta = TRUE;
  /*For video*/
  else if((stream_on_cfg->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO))
    && stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW)
    config_meta = TRUE;

  SLOW("config_meta = %d, stream_type = %x, mask = %x",
    config_meta, stream_info->stream_type, stream_on_cfg->stream_mask);
  return config_meta;
}

static boolean modules_sensor_set_new_resolution(mct_module_t *module,
  mct_event_t *event,
  module_sensor_bundle_info_t *s_bundle,
  module_sensor_params_t *module_sensor_params,
  sensor_set_res_cfg_t *stream_on_cfg,
  boolean *is_retry,
  mct_stream_info_t *stream_info)
{
  int32_t                rc = 0;
  boolean                ret = TRUE;
  sensor_out_info_t      sensor_out_info;
  mct_event_t            new_event;
  uint8_t                i = 0;
  mct_port_t            *port = NULL;
  sensor_src_port_cap_t *port_cap = NULL;

  SHIGH("SENSOR_SET_RESOLUTION %d*%d mask %x", stream_on_cfg->width,
    stream_on_cfg->height, stream_on_cfg->stream_mask);
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_SET_RESOLUTION, stream_on_cfg);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }
  memset(&sensor_out_info, 0, sizeof(sensor_out_info));
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_GET_RESOLUTION_INFO, &sensor_out_info);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }

  sensor_out_info.af_lens_info.af_supported =
    (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) ?
                                                 TRUE : FALSE;

  if (sensor_out_info.meta_cfg.num_meta &&
       module_sensor_config_meta(stream_info, stream_on_cfg)) {
    new_event.type = MCT_EVENT_MODULE_EVENT;
    new_event.identity = event->identity;
    new_event.direction = MCT_EVENT_DOWNSTREAM;
    new_event.u.module_event.type = MCT_EVENT_MODULE_SENSOR_META_CONFIG;
    new_event.u.module_event.module_event_data =
      (void *)&(sensor_out_info.meta_cfg);
    ret = sensor_util_post_event_on_src_port(module, &new_event);
    if (ret == FALSE) {
      SERR("failed");
      return FALSE;
    }
  }

  sensor_out_info.is_retry = FALSE;
  /* Fill some default format */
  sensor_out_info.fmt = s_bundle->frame_ctrl.fmt =
    CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG;
  s_bundle->frame_ctrl.fmt = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG;
  /* Fill format from source port */
  port = sensor_util_find_src_port_with_identity(module, event->identity);
  if (port && port->caps.u.data) {
    port_cap = (sensor_src_port_cap_t *)port->caps.u.data;
    if (port_cap->num_cid_ch == 1) {
      sensor_out_info.fmt = s_bundle->frame_ctrl.fmt =
        port_cap->sensor_cid_ch[0].fmt;
      SLOW("fmt %d", sensor_out_info.fmt);
    } else if (port_cap->num_cid_ch > 1) {
      for (i = 0; i < port_cap->num_cid_ch; i++) {
        if (port_cap->sensor_cid_ch[i].fmt != CAM_FORMAT_META_RAW_8BIT ||
          port_cap->sensor_cid_ch[i].fmt != CAM_FORMAT_META_RAW_10BIT) {
          sensor_out_info.fmt = s_bundle->frame_ctrl.fmt =
          port_cap->sensor_cid_ch[i].fmt;
          SLOW("fmt %d", sensor_out_info.fmt);
          break;
        }
      }
    }
  }
  if(s_bundle->regular_led_trigger == 1)
    sensor_out_info.prep_flash_on = TRUE;
  else
    sensor_out_info.prep_flash_on = FALSE;

  new_event.type = MCT_EVENT_MODULE_EVENT;
  new_event.identity = event->identity;
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.module_event.type = MCT_EVENT_MODULE_SET_STREAM_CONFIG;
  new_event.u.module_event.module_event_data = (void *)&sensor_out_info;
  ret = sensor_util_post_event_on_src_port(module, &new_event);

  *is_retry = sensor_out_info.is_retry;

  return ret;
}

/** module_sensor_start_af_bracketing: start focus bracketing
 *
 *  @module: mct module handle
 *  @s_bundle: sensor bundle handle
 *  @event: mct event
 *
 *  This function is used to start focus bracketing
 *
 *  Return: TRUE for success and FALSE on failure
 **/
boolean module_sensor_start_af_bracketing(mct_module_t *module,
  module_sensor_bundle_info_t *s_bundle, mct_event_t *event)
{
  module_sensor_ctrl_t      *module_ctrl =
      (module_sensor_ctrl_t *)module->module_private;
  module_sensor_params_t    *actuator_module_params = NULL;
  sensor_af_bracket_t       *bracket = &(s_bundle->af_bracket_params);
  sensor_frame_order_ctrl_t *ctrl = &(s_bundle->af_bracket_params.ctrl);
  int32_t rc = 0;

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
    actuator_module_params =
        s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];
  } else {
    SHIGH("No Actuator Module!");
    return TRUE;
  }

  SHIGH("AF_bracketing set for zsl snapshot");

  /* Lock mutex to wait properly for reseting lens */
  pthread_mutex_lock(&bracket->lens_move_done_sig);
  bracket->lens_move_progress = TRUE;
  pthread_mutex_unlock(&bracket->lens_move_done_sig);

  rc = sensor_util_set_frame_skip_to_isp(module,
         event->identity, SKIP_ALL);
  if (rc < 0) {
    SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
  }

  bracket->lens_reset = FALSE;
  bracket->wait_frame = ACTUATOR_WAIT_FRAME;

  sensor_thread_msg_t msg;
  msg.msgtype = OFFLOAD_FUNC;
  msg.offload_func = module_sensor_offload_lens_reset;
  msg.param1 = actuator_module_params;
  msg.param2 = NULL;
  msg.param3 = (void *)bracket;
  msg.param4 = NULL;
  msg.stop_thread = FALSE;
  int32_t nwrite = 0;
  nwrite = write(module_ctrl->pfd[1], &msg, sizeof(sensor_thread_msg_t));
  if(nwrite < 0) {
    SERR("%s: Writing into fd failed",__func__);
    pthread_mutex_lock(&bracket->lens_move_done_sig);
    bracket->lens_move_progress = FALSE;
    pthread_mutex_unlock(&bracket->lens_move_done_sig);
  }

  ctrl->captured_count = 0;
  ctrl->enable = TRUE;

  return TRUE;
}

/** module_sensor_start_mtf_bracketing: start focus bracketing
 *
 *  @module: mct module handle
 *  @s_bundle: sensor bundle handle
 *  @event: mct event
 *
 *  This function is used to start focus bracketing
 *
 *  Return: TRUE for success and FALSE on failure
 **/
boolean module_sensor_start_mtf_bracketing(mct_module_t *module,
  module_sensor_bundle_info_t *s_bundle, mct_event_t *event)
{
  module_sensor_ctrl_t      *module_ctrl =
      (module_sensor_ctrl_t *)module->module_private;
  module_sensor_params_t    *actuator_module_params = NULL;
  sensor_af_bracket_t       *bracket = &(s_bundle->mtf_bracket_params);
  sensor_frame_order_ctrl_t *ctrl = &(s_bundle->mtf_bracket_params.ctrl);
  int32_t rc = 0;

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
    actuator_module_params =
        s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];
  } else {
    SHIGH("No Actuator Module!");
    return TRUE;
  }

  SHIGH("MTF_bracketing set for zsl snapshot");

  /* Lock mutex to wait properly for reseting lens */
  pthread_mutex_lock(&bracket->lens_move_done_sig);
  bracket->lens_move_progress = TRUE;
  pthread_mutex_unlock(&bracket->lens_move_done_sig);

  rc = sensor_util_set_frame_skip_to_isp(module,
         event->identity, SKIP_ALL);
  if (rc < 0) {
    SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
  }

  bracket->lens_reset = FALSE;
  bracket->wait_frame = ACTUATOR_WAIT_FRAME;

  sensor_thread_msg_t msg;
  msg.msgtype = OFFLOAD_FUNC;
  msg.offload_func = module_sensor_offload_lens_reset;
  msg.param1 = actuator_module_params;
  msg.param2 = NULL;
  msg.param3 = (void *)bracket;
  msg.param4 = NULL;
  msg.stop_thread = FALSE;
  int32_t nwrite = 0;
  nwrite = write(module_ctrl->pfd[1], &msg, sizeof(sensor_thread_msg_t));
  if(nwrite < 0) {
    SERR("%s: Writing into fd failed",__func__);
    pthread_mutex_lock(&bracket->lens_move_done_sig);
    bracket->lens_move_progress = FALSE;
    pthread_mutex_unlock(&bracket->lens_move_done_sig);
  }

  ctrl->captured_count = 0;
  ctrl->enable = TRUE;

  return TRUE;
}

/** module_sensor_update_af_bracket_entry: update af bracket
 *  entry
 *
 *  @module: mct module handle
 *  @s_bundle: sensor bundle handle
 *  @event: received event
 *
 *  This function is used to update selected af bracket entry to
 *  move lens in actuator upon sof under af bracketing mode.
 *  It Also updates the MCT bus msg to notify good frame id.
 *
 *  NOTE:
 *  For ZSL Snapshot, it will first reset lens at
 *  MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT.
 *  For Non-ZSL Snapshot, it will first reset lens at
 *  MCT_EVENT_CONTROL_STREAMON It will be always be the first
 *  lens position
 *
 *  Return: TRUE for success and FALSE on failure**/
boolean module_sensor_update_af_bracket_entry(mct_module_t *module,
 void *s_bundle, mct_event_t *event)
{
  int32_t                      rc = 0;
  uint8_t                      num_of_burst;
  module_sensor_bundle_info_t  *bun = NULL;
  module_sensor_params_t       *actuator_module_params = NULL;
  sensor_af_bracket_t          *bracket = NULL;
  sensor_frame_order_ctrl_t    *ctrl = NULL;
  module_sensor_ctrl_t         *module_ctrl = NULL;
  pthread_mutex_t              *lens_move_done_sig = NULL;

  if (!s_bundle) {
    SERR("Invalid s_bundle_ptr");
    return FALSE;
  }

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  bun = (module_sensor_bundle_info_t *) s_bundle;

  if (bun->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
    actuator_module_params = bun->module_sensor_params[SUB_MODULE_ACTUATOR];
  } else {
    SHIGH("Error: No Actuator Module present!");
    return TRUE;
  }
  actuator_module_params = bun->module_sensor_params[SUB_MODULE_ACTUATOR];
  lens_move_done_sig = &bun->af_bracket_params.lens_move_done_sig;

  ctrl = &(bun->af_bracket_params.ctrl);
  bracket = &(bun->af_bracket_params);
  num_of_burst = bracket->burst_count;

  SHIGH("AF_bracketing : Enter, sof LOGIC fid[%d]", bun->last_idx);
  if (ctrl->captured_count < num_of_burst) {
    // Check if lens movement is completed
    pthread_mutex_lock(&bracket->lens_move_done_sig);
    if (!bracket->lens_move_progress) {

       if (bracket->wait_frame == ACTUATOR_WAIT_FRAME) {
            mct_bus_msg_t bus_msg;
            cam_frame_idx_range_t range;
            bus_msg.sessionid = bun->sensor_info->session_id;
            bus_msg.type = MCT_BUS_MSG_ZSL_TAKE_PICT_DONE;
            range.min_frame_idx = bun->last_idx + ACTUATOR_WAIT_FRAME;
            range.max_frame_idx = range.min_frame_idx + ACTUATOR_MAX_WAIT_FRAME;

            bus_msg.msg = &range;
            if(mct_module_post_bus_msg(module,&bus_msg)!=TRUE)
              SERR("error posting led frame range msg");
       }
       if (bracket->wait_frame-- > 1) {
           /*In case actuator need more than
           * one frame time to settle after reaching
           * its position*/
           pthread_mutex_unlock(&bracket->lens_move_done_sig);
           return TRUE;
       }
       rc = sensor_util_set_frame_skip_to_isp(module,
                event->identity, EVERY_32FRAME);
       if (!rc) {
          SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
        }
        bracket->lens_reset = TRUE;
        bracket->wait_frame = ACTUATOR_WAIT_FRAME;
        bracket->lens_move_progress = TRUE;
    } else if (bracket->lens_reset &&
      sensor_util_is_previous_frame_sent(module, event)) {
      /* Move the lens only when previous frame is sent and lens is reset,
      * in order to avoid false potive check for the first frame
      */
      rc = sensor_util_set_frame_skip_to_isp(module,
             event->identity, SKIP_ALL);
      if (!rc) {
        SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
      }
      ctrl->captured_count++;
      /* Don't do extra lens move after the last frame is captured */
      if (ctrl->captured_count < num_of_burst) {
        SHIGH("moving lens, steps = %d", bracket->num_steps);
        /* Move Lens in Actuator */
        sensor_thread_msg_t msg;
        msg.msgtype = OFFLOAD_FUNC;
        msg.offload_func = module_sensor_offload_lens_move;
        msg.param1 = actuator_module_params;
        msg.param2 = (void*)MOVE_NEAR;
        msg.param3 = (void*)bracket;
        msg.param4 = NULL;
        msg.stop_thread = FALSE;
        ssize_t nwrite = 0;
        nwrite = write(module_ctrl->pfd[1],
          &msg, sizeof(sensor_thread_msg_t));
        if(nwrite < 0) {
          SERR("%s: Writing into fd failed",__func__);
          bracket->lens_move_progress = FALSE;
        }
      } else {
        bracket->lens_move_progress = FALSE;
      }
    }
    pthread_mutex_unlock(&bracket->lens_move_done_sig);
  } else {
    SHIGH("AF_bracketing : captured passed busrt, disabling");
    ctrl->enable = FALSE;
    ctrl->captured_count = 0;

    /*Restore HAL skip pattern*/
    rc = sensor_util_set_frame_skip_to_isp(module,
           event->identity, bun->hal_frame_skip_pattern);
    if (!rc) {
      SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
    }
  }
  SHIGH("AF_bracketing : sof fid[%d], captured_count %d, X",
    bun->last_idx, ctrl->captured_count);

  return TRUE;
}

/** module_sensor_update_mtf_bracket_entry: update mtf bracket
 *  entry
 *
 *  @module: mct module handle
 *  @s_bundle: sensor bundle handle
 *  @event: received event
 *
 *  This function is used to update multi-touch focus bracket
 *  entry to move lens in actuator upon sof under af bracketing
 *  mode. It Also updates the MCT bus msg to notify good frame
 *  id.
 *
 *  NOTE:
 *  For ZSL Snapshot, it will first reset lens at
 *  MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT. For Non-ZSL Snapshot,
 *  it will first reset lens at MCT_EVENT_CONTROL_STREAMON It
 *  will be always be the first lens position. this reset is done by
 *  module_sensor_start_mtf_bracketing()
 *
 *  Return: TRUE for success and FALSE on failure**/
boolean module_sensor_update_mtf_bracket_entry(mct_module_t *module,
 void *s_bundle, mct_event_t *event)
{
  int32_t                      rc = 0;
  uint8_t                      num_of_burst;
  module_sensor_bundle_info_t  *bun = NULL;
  module_sensor_params_t       *actuator_module_params = NULL;
  sensor_af_bracket_t          *bracket = NULL;
  sensor_frame_order_ctrl_t    *ctrl = NULL;
  module_sensor_ctrl_t         *module_ctrl = NULL;
  pthread_mutex_t              *lens_move_done_sig = NULL;

  if (!s_bundle) {
    SERR("Invalid s_bundle_ptr");
    return FALSE;
  }

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  bun = (module_sensor_bundle_info_t *) s_bundle;

  if (bun->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
    actuator_module_params = bun->module_sensor_params[SUB_MODULE_ACTUATOR];
  } else {
    SHIGH("Error: No Actuator Module present!");
    return TRUE;
  }
  actuator_module_params = bun->module_sensor_params[SUB_MODULE_ACTUATOR];
  lens_move_done_sig = &bun->mtf_bracket_params.lens_move_done_sig;

  ctrl = &(bun->mtf_bracket_params.ctrl);
  bracket = &(bun->mtf_bracket_params);
  num_of_burst = bracket->burst_count;

  SHIGH("MTF_bracketing : Enter, sof LOGIC fid[%d]", bun->last_idx);
  if (ctrl->captured_count < num_of_burst) {
    // Check if lens movement is completed
    pthread_mutex_lock(&bracket->lens_move_done_sig);
    if (!bracket->lens_move_progress) {
       if (bracket->wait_frame == ACTUATOR_WAIT_FRAME) {
         if (ctrl->captured_count != 0) {
            mct_bus_msg_t bus_msg;
            cam_frame_idx_range_t range;
            bus_msg.sessionid = bun->sensor_info->session_id;
            bus_msg.type = MCT_BUS_MSG_ZSL_TAKE_PICT_DONE;
            range.min_frame_idx = bun->last_idx + ACTUATOR_WAIT_FRAME;
            range.max_frame_idx = range.min_frame_idx + ACTUATOR_MAX_WAIT_FRAME;
            bus_msg.msg = &range;
            if(mct_module_post_bus_msg(module,&bus_msg)!=TRUE)
              SERR("error posting frame range msg");
         }
       }
       if (bracket->wait_frame-- > 1) {
           /*In case actuator need more than
           * one frame time to settle after reaching
           * its position*/
           pthread_mutex_unlock(lens_move_done_sig);
           return TRUE;
       }
       rc = sensor_util_set_frame_skip_to_isp(module,
                event->identity, EVERY_32FRAME);
       if (!rc) {
          SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
        }
        bracket->lens_reset = TRUE;
        bracket->wait_frame = ACTUATOR_WAIT_FRAME;
        bracket->lens_move_progress = TRUE;
    } else if (bracket->lens_reset &&
      sensor_util_is_previous_frame_sent(module, event)) {
      /* Move the lens only when previous frame is sent and lens is reset,
      * in order to avoid false potive check for the first frame
      */
      rc = sensor_util_set_frame_skip_to_isp(module,
             event->identity, SKIP_ALL);
      if (!rc) {
        SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
      }
      ctrl->captured_count++;
      /* Don't do extra lens move after the last frame is captured */
      if (ctrl->captured_count < num_of_burst + 1) {
        SHIGH("moving lens, captured count = %d, steps = %d",
              ctrl->captured_count,
              bracket->abs_steps_to_move[ctrl->captured_count-1]);
        /* Move Lens in Actuator */
        sensor_thread_msg_t msg;
        msg.msgtype = OFFLOAD_FUNC;
        msg.offload_func = module_sensor_offload_lens_move;
        msg.param1 = actuator_module_params;
        msg.param2 = (void*)MOVE_NEAR;
        msg.param3 = (void*)bracket;
        msg.param4 = (void*)ctrl;
        msg.stop_thread = FALSE;
        ssize_t nwrite = 0;
        nwrite = write(module_ctrl->pfd[1],
          &msg, sizeof(sensor_thread_msg_t));
        if(nwrite < 0) {
          SERR("%s: Writing into fd failed",__func__);
          bracket->lens_move_progress = FALSE;
        }
      } else {
        bracket->lens_move_progress = FALSE;
      }
    }
    pthread_mutex_unlock(&bracket->lens_move_done_sig);
  } else {
    SHIGH("MTF_bracketing : captured passed busrt, disabling");
    ctrl->enable = FALSE;
    ctrl->captured_count = 0;

    /*Restore HAL skip pattern*/
    rc = sensor_util_set_frame_skip_to_isp(module,
           event->identity, bun->hal_frame_skip_pattern);
    if (!rc) {
      SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
    }
  }
  SHIGH("MTF_bracketing : sof fid[%d], captured_count %d, X",
    bun->last_idx, ctrl->captured_count);

  return TRUE;
}

/** module_sensor_update_flash_bracket_entry:
 *
 *  @module: mct module handle
 *  @s_bundle: sensor bundle handle
 *  @event: received event
 *
 *  This function is used to update Flash bracket entry.
 *  Currently only two frame is needed, so it only turns off LED
 *  when LED on frame is reached.
 *
 *  Return: TRUE for success and FALSE on failure**/
boolean module_sensor_update_flash_bracket_entry(mct_module_t *module,
 void *s_bundle, mct_event_t *event)
{
  int32_t rc = 0;
  uint8_t num_of_burst;
  module_sensor_params_t    *led_module_params = NULL;
  sensor_flash_bracket_t              *bracket = NULL;
  sensor_bracket_ctrl_t                  *ctrl = NULL;
  module_sensor_params_t *module_sensor_params = NULL;

  if (!s_bundle) {
    SERR("Invalid s_bundle_ptr");
    return FALSE;
  }

  module_sensor_bundle_info_t * bun = (module_sensor_bundle_info_t *) s_bundle;
  led_module_params = bun->module_sensor_params[SUB_MODULE_LED_FLASH];
  module_sensor_params = bun->module_sensor_params[SUB_MODULE_SENSOR];
  bracket = &(bun->flash_bracket_params.flash_bracket);
  ctrl = &(bun->flash_bracket_params.ctrl);
  num_of_burst = bracket->burst_count;

  SLOW("Flash_bracketing : Enter");

  /* Check internal flag */
  if (ctrl->enable == TRUE) {
    if (bun->last_idx < ctrl->max_frame_idx) {
      SHIGH("Flash_bracketing : sof LOGIC fid[%d], captured_cpunt %d",
        bun->last_idx, ctrl->captured_count);

      // Request one isp frame for led on and led off frame
      if ((bun->last_idx == ctrl->max_frame_idx - 1) ||
        (bun->last_idx == ctrl->min_frame_idx - 1)) {

        rc = sensor_util_set_frame_skip_to_isp(module,
          bracket->preview_identity, EVERY_32FRAME);
        if (!rc < 0) {
          SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
        }
        rc = sensor_util_set_frame_skip_to_isp(module,
          bracket->snapshot_identity, EVERY_32FRAME);
        if (!rc) {
          SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
        }
      }

      /* check for matched LED frame */
      if(bun->last_idx < ctrl->min_frame_idx) {
        SHIGH("waiting led frame, cur sof [%d), expect frame [%d], skipping",
          bun->last_idx, ctrl->min_frame_idx);
      } else if (bun->last_idx == ctrl->min_frame_idx) {

        rc = sensor_util_set_frame_skip_to_isp(module,
          bracket->preview_identity, SKIP_ALL);
        if (!rc < 0) {
          SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
        }
        rc = sensor_util_set_frame_skip_to_isp(module,
          bracket->snapshot_identity, SKIP_ALL);
        if (!rc) {
          SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
        }

      } else if (bun->last_idx == ctrl->min_frame_idx + 1) {
        /* disable LED for max frame */
        SHIGH("LED frame match, turn off LED");
        rc = led_module_params->func_tbl.process(
          led_module_params->sub_module_private,
          LED_FLASH_SET_OFF , NULL);

        if ( rc < 0 ) {
          SERR("failed: LED_FLASH_SET_OFF");
          return FALSE;
        } else {
          /*recover gain and linecount from LED off*/
          aec_update_t aec_update;
          aec_update.real_gain = bun->led_off_gain;
          aec_update.linecount = bun->led_off_linecount;
          SERR("led_off stats gain %f lnct %d", aec_update.real_gain,
              aec_update.linecount);
          rc = module_sensor_params->func_tbl.process(
              module_sensor_params->sub_module_private,
              SENSOR_SET_AEC_UPDATE, &aec_update);
          if (rc < 0) {
            SERR("failed");
            return FALSE;
          }
          bun->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
          sensor_util_post_led_state_msg(module, s_bundle,
            event->identity);

          mct_bracket_ctrl_t bracket_ctrl;
          bracket_ctrl.state = MCT_BRACKET_CTRL_RESTORE_LOCK_3A;
          rc = sensor_util_post_downstream_event(module, event->identity,
            MCT_EVENT_MODULE_SENSOR_BRACKET_CTRL, &bracket_ctrl);
        }
      } else {
        SHIGH("waiting for LED off frame");
      }
    } else {
      SHIGH("max_frame_idx reached, disabling, cur sof : [%d]", bun->last_idx);
      ctrl->enable = FALSE;
      bracket->enable = FALSE;

      rc = sensor_util_set_frame_skip_to_isp(module, bracket->preview_identity,
        bun->hal_frame_skip_pattern);
      if (!rc) {
        SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
      }
      rc = sensor_util_set_frame_skip_to_isp(module, bracket->snapshot_identity,
        bun->hal_frame_skip_pattern);
      if (!rc) {
        SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
      }

      /* Post downstream event to off bracketting */
      mct_bracket_ctrl_t bracket_ctrl;
      bracket_ctrl.state = MCT_BRACKET_CTRL_OFF;
      rc = sensor_util_post_downstream_event(module, event->identity,
        MCT_EVENT_MODULE_SENSOR_BRACKET_CTRL, &bracket_ctrl);
    }
  } else {
    SHIGH("Flash_bracketing not enabled");
  }
  return TRUE;
}

/** module_sensor_set_param_af_bracket:
 *
 *  @module: mct module handle
 *  @event: event associated with stream
 *  @s_bundle: sensor bundle handle
 *
 *  This function is passes HAL AF bracketing params to sensor
 *  and setup bracketing info. for UbiFocus the af is evenly
 *  spaced between the total steps.
 *
 *  Return: TRUE for success and FALSE on failure**/
static boolean module_sensor_set_param_af_bracket(
  module_sensor_bundle_info_t * s_bundle, mct_event_control_parm_t *event_parm)
{
  boolean                   ret = TRUE;
  uint16_t                  total_steps;
  sensor_get_af_algo_ptr_t  af_algo_ptr;
  af_algo_tune_parms_t      *af_tune = NULL;
  af_tuning_algo_t          *af_algo = NULL;
  module_sensor_params_t    *actuator_module_params = NULL;
  cam_af_bracketing_t       *cam_af_bracket = NULL;
  sensor_af_bracket_t       *bracket = NULL;
  sensor_bracket_ctrl_t     *ctrl = NULL;

  SLOW("AF_bracketing : E");

  if (!event_parm->parm_data) {
    SERR("Invalid event ptr");
    return FALSE;
  }

  bracket = &(s_bundle->af_bracket_params);
  ctrl = &(s_bundle->af_bracket_params.ctrl);

  /* Copy structure from HAL */
  cam_af_bracket = (cam_af_bracketing_t*) event_parm->parm_data;
  bracket->enable = cam_af_bracket->enable;
  bracket->burst_count = cam_af_bracket->burst_count;

  actuator_module_params = s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];

  if (!bracket->enable) {
    af_update_t af_update;
    SHIGH("Disable focus bracketing");
    af_update.reset_lens = TRUE;

    if (actuator_module_params->func_tbl.process != NULL) {
      ret = actuator_module_params->func_tbl.process(
        actuator_module_params->sub_module_private,
        ACTUATOR_MOVE_FOCUS, &af_update);
    }

    return TRUE;
  }

  if (bracket->burst_count > ACTUATOR_BRACKET_MAX ||
    bracket->burst_count < ACTUATOR_BRACKET_MIN) {
    SERR("Invalid AF Bracket burst num : %d", bracket->burst_count);
    return FALSE;
  }

  /* Get AF Pointer */
  af_algo_ptr.cam_mode = ACTUATOR_CAM_MODE_CAMERA;
  ret = actuator_module_params->func_tbl.process(
    actuator_module_params->sub_module_private,
    ACTUATOR_GET_AF_ALGO_PARAM_PTR, &af_algo_ptr);
  if ((ret < 0) || (!af_algo_ptr.af_tune_ptr)) {
    SERR("failed to get af_tune rc %d", ret);
    return FALSE;
  }

  /* Calculate bracket */
  af_algo = &af_algo_ptr.af_tune_ptr->af_algo;
  total_steps =
    (uint16_t)(af_algo->position_far_end - af_algo->position_near_end + 1);
  total_steps = (uint16_t)(total_steps + af_algo->undershoot_adjust);

  if (total_steps < bracket->burst_count) {
    SERR("Invalid params: total steps %d, burst %d",
      total_steps, bracket->burst_count);
    return FALSE;
  }
  bracket->num_steps = total_steps / (bracket->burst_count - 1);
  SHIGH("total_steps = %d, burst %d, num_steps = %d", total_steps,
    bracket->burst_count, bracket->num_steps);

  /* Initialize control */
  ctrl->captured_count = 0;
  ctrl->enable         = FALSE;
  ctrl->is_post_msg    = FALSE;

  return TRUE;
}

/** module_sensor_set_param_mtf_bracket:
 *
 *  @module: mct module handle
 *  @event: event associated with stream
 *  @s_bundle: sensor bundle handle
 *
 *  This function is passes HAL MTF bracketing params to sensor
 *  and setup bracketing info. for Multi-touch focus, steps to
 *  move are different.
 *
 *  Return: TRUE for success and FALSE on failure**/
static boolean module_sensor_set_param_mtf_bracket(
  module_sensor_bundle_info_t * s_bundle, mct_event_control_parm_t *event_parm)
{
  boolean                   ret = TRUE;
  uint8_t                   i = 0;
  uint16_t                  total_steps;
  uint16_t                  lens_far_end;
  int32_t                   native_steps[MAX_AF_BRACKETING_VALUES];
  sensor_get_af_algo_ptr_t  af_algo_ptr;
  af_tuning_algo_t          *af_algo = NULL;
  module_sensor_params_t    *actuator_module_params = NULL;
  cam_af_bracketing_t       *cam_af_bracket = NULL;
  sensor_af_bracket_t       *bracket = NULL;
  sensor_frame_order_ctrl_t *ctrl = NULL;

  SLOW("MTF_bracketing set HAL parameters: E");

  if (!event_parm->parm_data) {
    SERR("Invalid event ptr");
    return FALSE;
  }

  bracket = &(s_bundle->mtf_bracket_params);
  ctrl = &(s_bundle->mtf_bracket_params.ctrl);

  /* Copy structure from HAL */
  cam_af_bracket = (cam_af_bracketing_t*) event_parm->parm_data;
  bracket->enable = cam_af_bracket->enable;
  bracket->burst_count = cam_af_bracket->burst_count;
  for (i = 0; i < bracket->burst_count; i++) {
    native_steps[i] = cam_af_bracket->focus_steps[i];
    SHIGH("%s: native step from HAL idx %d is %d", __func__,
          i, cam_af_bracket->focus_steps[i]);
  }

  actuator_module_params = s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];

  if (!bracket->enable) {
    af_update_t af_update;
    SHIGH("Disable focus bracketing");
    af_update.reset_lens = TRUE;

    if (actuator_module_params->func_tbl.process != NULL) {
      ret = actuator_module_params->func_tbl.process(
        actuator_module_params->sub_module_private,
        ACTUATOR_MOVE_FOCUS, &af_update);
    }

    return TRUE;
  }

  if (bracket->burst_count > ACTUATOR_BRACKET_MAX ||
    bracket->burst_count < ACTUATOR_BRACKET_MIN) {
    SERR("Invalid AF Bracket burst num : %d", bracket->burst_count);
    return FALSE;
  }

  /* Get AF Pointer */
  ret = actuator_module_params->func_tbl.process(
    actuator_module_params->sub_module_private,
    ACTUATOR_GET_AF_ALGO_PARAM_PTR, &af_algo_ptr);
  if ((ret < 0) || (!af_algo_ptr.af_tune_ptr)) {
    SERR("failed to get af_tune rc %d", ret);
    return FALSE;
  }

  /* Calculate bracket */
  af_algo = &af_algo_ptr.af_tune_ptr->af_algo;
  lens_far_end = af_algo->position_far_end;
  SHIGH("lens far end = %d, lens near end = %d", lens_far_end, af_algo->position_near_end);
  bracket->abs_steps_to_move[0] = lens_far_end - native_steps[0];
  for (i = 1; i < bracket->burst_count; i++) {
     bracket->abs_steps_to_move[i] = native_steps[i-1] - native_steps[i];
  }

  /* check focus step validity*/
  for (i = 0; i < bracket->burst_count; i++) {
     SHIGH("%s: steps to move: %d", __func__,
          bracket->abs_steps_to_move[i]);
     if (bracket->abs_steps_to_move[i] < 0) {
        SERR("%s: invalid step %d.", __func__, i);
        return FALSE;
     }
  }

  /* Initialize control */
  ctrl->captured_count = 0;
  ctrl->enable         = FALSE;
  ctrl->is_post_msg    = FALSE;

  return TRUE;
}

/** module_sensor_set_hdr_zsl_mode: sensor hdr mode
 *
 *  @module: mct module handle
 *  @event: event associated with stream
 *  @s_bundle: sensor bundle handle
 *
 *  This function is used to set the hdr zsl mode
 *  and fetch the required information from stats
 *
 *  Return: TRUE for success and FALSE on failure**/
static boolean module_sensor_set_hdr_zsl_mode(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle)
{
  module_sensor_params_t  *module_sensor_params =
    s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  int32_t rc = 0;
  boolean ret = TRUE;
  sensor_set_hdr_ae_t hdr_info;
  memset(&hdr_info, 0, sizeof(sensor_set_hdr_ae_t));

  rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_GET_SENSOR_FORMAT, &hdr_info.output_format);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }
  if ((s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT)) &&
    (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW))) {
    SLOW("HDR Zsl Mode");
    hdr_info.hdr_zsl_mode = 1;
    /* TODO: The skip should
       * come from ISP*/
     hdr_info.isp_frame_skip = 1;
  } else {
    SLOW("HDR Non Zsl Mode");
    hdr_info.hdr_zsl_mode = 0;
    hdr_info.isp_frame_skip = 0;
  }
  if (hdr_info.hdr_zsl_mode  && (hdr_info.output_format == SENSOR_BAYER)) {
    /* get initial gain/linecount from AEC */
    rc = module_sensor_get_stats_data(module, event->identity, &hdr_info.stats_get);
    if (rc < 0) {
      SERR("Failed to get stats info");
      return FALSE;
    }
  }
  rc = module_sensor_params->func_tbl.process(
         module_sensor_params->sub_module_private,
         SENSOR_SET_HDR_ZSL_MODE, &hdr_info);
  if (rc < 0) {
    SERR("Set HDR Zsl Mode failed");
    return FALSE;
  }
  return ret;
}

/** module_sensor_stream_on: sensor stream on
 *
 *  @module: mct module handle
 *  @event: event associated with stream on
 *  @s_bundle: sensor bundle handle
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function executes stream on sequence based on the
 *  order provided in sensor library. The possible sequences
 *  are listed in sensor_res_cfg_type_t enum **/

static boolean module_sensor_stream_on(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle)
{
  boolean                        ret = TRUE;
  int32_t                        rc = SENSOR_SUCCESS;
  int32_t                        i = 0;
  sensor_lib_params_t           *lib = s_bundle->sensor_lib_params;
  uint32_t                       bits_per_pixel = 0;
  int32_t                        csi_clk = 0;
  module_sensor_params_t        *module_sensor_params = NULL;
  module_sensor_params_t        *csiphy_module_params = NULL;
  module_sensor_params_t        *csid_module_params = NULL;
  module_sensor_params_t        *chromatix_module_params = NULL;
  module_sensor_params_t        *actuator_module_params = NULL;
  sensor_get_t                   sensor_get;
  sensor_out_info_t              sensor_out_info;
  struct sensor_res_cfg_table_t *res_cfg_table = NULL;
  mct_stream_info_t* stream_info =
    (mct_stream_info_t*) event->u.ctrl_event.control_event_data;
  sensor_set_res_cfg_t stream_on_cfg;
  int32_t bundle_id = -1;

  SHIGH("ide %x SENSOR_START_STREAM", event->identity);
  mct_port_t *port = sensor_util_find_src_port_with_identity(
                          module, event->identity);
  if (!port) {
    SERR("cannot find matching port with identity=0x%x",
      event->identity);
    return FALSE;
  }
  sensor_util_dump_bundle_and_stream_lists(port, __func__, __LINE__);
  bundle_id = sensor_util_find_bundle_id_for_stream(port, event->identity);
  boolean stream_on_flag = module_sensor_is_ready_for_stream_on(port, event,
    &stream_on_cfg, s_bundle, bundle_id);
  if (!stream_on_flag) {
    SLOW("NO STREAM_ON, dummy excersice");
  } else {
    SLOW("REAL STREAM_ON");
  }

  SLOW("config: dim=%dx%d, mask=0x%x stream type: %d", stream_on_cfg.width,
    stream_on_cfg.height, stream_on_cfg.stream_mask, stream_info->stream_type);

  /* Check whether this is live snapshot stream ON */
  if ((lib->sensor_lib_ptr->sensor_output->output_format != SENSOR_YCBCR) &&
      (stream_on_cfg.stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)) &&
      (stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT)) {
    /* Load live snapshot chromatix */
    ret = sensor_util_load_liveshot_chromatix(module, port, event, s_bundle);
    if (ret == FALSE) {
      SERR("failed");
    }
  }

  if (1 == s_bundle->torch_on) {
    module_sensor_params_t *led_module_params =
       s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
    if (led_module_params->func_tbl.process != NULL) {
      rc = led_module_params->func_tbl.process(
        led_module_params->sub_module_private, LED_FLASH_SET_TORCH, NULL);
      if (rc < 0) {
        SERR("failed: LED_FLASH_SET_TORCH");
      } else {
        s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_TORCH;
        s_bundle->torch_on = 1;
        sensor_util_post_led_state_msg(module, s_bundle, event->identity);
      }
    }
  }

  if (bundle_id == -1 && stream_on_flag == FALSE) {
    SLOW("propogate stream on event");
    /* Call send_event to propogate event to next module*/
    ret = sensor_util_post_event_on_src_port(module, event);
    if (ret == FALSE) {
      SERR("failed");
      return FALSE;
    }
    return TRUE;
  }

  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  csiphy_module_params = s_bundle->module_sensor_params[SUB_MODULE_CSIPHY];
  csid_module_params = s_bundle->module_sensor_params[SUB_MODULE_CSID];
  chromatix_module_params =
    s_bundle->module_sensor_params[SUB_MODULE_CHROMATIX];
  if (!module_sensor_params || !csiphy_module_params || !csid_module_params ||
    !chromatix_module_params) {
    SERR("failed");
    return FALSE;
  }
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_GET_RES_CFG_TABLE, &res_cfg_table);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }
  for (i = 0; i < res_cfg_table->size; i++) {
    SLOW("cfg type %d",
      res_cfg_table->res_cfg_type[i]);
    switch (res_cfg_table->res_cfg_type[i]) {
    case SENSOR_SET_STOP_STREAM: {
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_STOP_STREAM, NULL);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }
      break;
    }
    case SENSOR_SET_START_STREAM: {
      mct_event_t new_event;
      float digital_gain=0.0;
      sensor_output_format_t output_format;
      sensor_chromatix_params_t chromatix_params;

      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_GET_SENSOR_FORMAT, &output_format);
      if (rc < 0) {
          SERR("failed");
      } else if (output_format == SENSOR_BAYER) {
        stats_get_data_t stats_get;
        stats_get_data_t *dest_stats_get;
        memset(&stats_get, 0, sizeof(stats_get_data_t));
        /* get initial gain/linecount from AEC */
        rc = module_sensor_get_stats_data(module, event->identity, &stats_get);
        if (rc < 0) {
          SERR("failed");
          return FALSE;
        }
        if ((stats_get.flag & STATS_UPDATE_AEC) == 0x00) {
          /* non-fatal error */
          SERR("Invalid: No AEC update in stats_get");
        } else {
          mct_bus_msg_t bus_msg;
          int32_t bus_index;
          SLOW("bus msg");
          chromatix_params.stream_mask = s_bundle->stream_mask;
          rc = chromatix_module_params->func_tbl.process(
            chromatix_module_params->sub_module_private,
            CHROMATIX_GET_PTR, &chromatix_params);
          if (rc < 0) {
            SERR("failed");
            return FALSE;
          }
          if (!chromatix_params.common_chromatix_ptr ||
              !chromatix_params.chromatix_ptr) {
            SERR("failed common %s chromatix %s",
              sensor_get.chromatix_name.common_chromatix,
              sensor_get.chromatix_name.chromatix);
          } else {
            /* Send bus msg for passing chromatix pointers */
            s_bundle->chromatix_metadata.chromatix_ptr =
              (void *)chromatix_params.snapchromatix_ptr;
            s_bundle->chromatix_metadata.common_chromatix_ptr =
              (void *)chromatix_params.common_chromatix_ptr;
            bus_msg.sessionid = s_bundle->sensor_info->session_id;
            bus_msg.type = MCT_BUS_MSG_SET_SENSOR_INFO;
            bus_msg.msg = (void *)&s_bundle->chromatix_metadata;
            if(mct_module_post_bus_msg(module, &bus_msg) == FALSE)
              SERR("failed");
            /* Print chromatix pointers */
            SLOW("c %p com c %p", s_bundle->chromatix_metadata.chromatix_ptr,
              s_bundle->chromatix_metadata.common_chromatix_ptr);
            /* Send bus msg for passing AEC trigger update */
            if (sizeof(stats_get_data_t) >
                sizeof(s_bundle->aec_metadata.private_data)) {
              SERR("failed");
            } else {
              memcpy(s_bundle->aec_metadata.private_data, &stats_get,
                sizeof(stats_get_data_t));
              bus_msg.sessionid = s_bundle->sensor_info->session_id;
              bus_msg.type = MCT_BUS_MSG_SET_STATS_AEC_INFO;
              bus_msg.size = sizeof(stats_get_data_t);
              SLOW("bus msg size %d", bus_msg.size);
              bus_msg.msg = (void *)&s_bundle->aec_metadata;
              if(mct_module_post_bus_msg(module, &bus_msg) == FALSE)
                SERR("failed");
              /* Print source stats get data */
              SLOW("source valid entries %d", stats_get.aec_get.valid_entries);
              for (bus_index = 0; bus_index < stats_get.aec_get.valid_entries;
                   bus_index++) {
                SLOW("source g %f lux idx %f",
                  stats_get.aec_get.real_gain[bus_index],
                  stats_get.aec_get.lux_idx);
              }
              /* Print destination stats get data */
              dest_stats_get =
                (stats_get_data_t *)s_bundle->aec_metadata.private_data;
              SLOW("dest valid entries %d",
                dest_stats_get->aec_get.valid_entries);
              for (bus_index = 0;
                   bus_index < dest_stats_get->aec_get.valid_entries;
                   bus_index++) {
                SLOW("dest g %f lux idx %f",
                  dest_stats_get->aec_get.real_gain[bus_index],
                  dest_stats_get->aec_get.lux_idx);
              }
            }
          }
        }

        sensor_bracket_params_t *flash_bracket_params = NULL;
        sensor_bracket_ctrl_t * flash_ctrl = NULL;
        flash_bracket_params = &(s_bundle->flash_bracket_params);
        flash_ctrl = &(flash_bracket_params->ctrl);

        /* save postview identity as preview one */
        if (CAM_STREAM_TYPE_POSTVIEW == stream_info->stream_type) {
          if (flash_bracket_params->flash_bracket.enable == TRUE) {
            flash_bracket_params->flash_bracket.preview_identity =
              event->identity;
          }
        }
        /* save snapshot identity */
        if (CAM_STREAM_TYPE_SNAPSHOT == stream_info->stream_type) {
          if (flash_bracket_params->flash_bracket.enable == TRUE) {
            flash_bracket_params->flash_bracket.snapshot_identity =
              event->identity;
          }
        }

        if (s_bundle->regular_led_trigger == 1)  {
          module_sensor_params_t        *led_module_params = NULL;
          led_module_params =
            s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
          if (led_module_params != NULL &&
              led_module_params->func_tbl.process != NULL) {
            int flash_mode;
            if(s_bundle->longshot)
              flash_mode = LED_FLASH_SET_TORCH;
            else
              flash_mode  = LED_FLASH_SET_MAIN_FLASH;
            rc = led_module_params->func_tbl.process(
              led_module_params->sub_module_private,
              flash_mode, NULL);
            if (rc < 0) {
              s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
              SERR("failed: LED_FLASH_SET_MAIN_FLASH");
            } else {
              s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_ON;
              SHIGH("%s, post-flash: ON", __func__);
            }
          }
          sensor_util_post_led_state_msg(module, s_bundle, event->identity);

          SHIGH("%s, post-flash: ON", __func__);
          s_bundle->regular_led_trigger = 0;
        }

        /* enable Flash bracketing upon streamon for both streams */
        if (flash_bracket_params->flash_bracket.snapshot_identity &&
            flash_bracket_params->flash_bracket.preview_identity) {
          SLOW("current frame %d", s_bundle->last_idx);
          /* @num_skip_exp_linecount :
           *   fixed delay for valid exp and linecount and led off
           * @num_skip_led_up :
           *   fixed delay for LED to take effect
           */
          const uint8_t num_skip_exp_linecount = 3;
          const uint8_t num_skip_led_up = 2;

          rc = sensor_util_set_frame_skip_to_isp(module,
            flash_bracket_params->flash_bracket.preview_identity, SKIP_ALL);
          if (!rc < 0) {
            SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
          }
          rc = sensor_util_set_frame_skip_to_isp(module,
            flash_bracket_params->flash_bracket.snapshot_identity, SKIP_ALL);
          if (!rc) {
            SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
          }

          /* Fill metadta msg */
          cam_frame_idx_range_t range;
          range.min_frame_idx = num_skip_led_up;
          range.max_frame_idx = range.min_frame_idx +
            num_skip_exp_linecount;
          mct_bus_msg_t bus_msg;
          bus_msg.sessionid = s_bundle->sensor_info->session_id;
          bus_msg.type = MCT_BUS_MSG_ZSL_TAKE_PICT_DONE;
          bus_msg.msg = &range;

          /* save frame info to local structure */
          flash_ctrl->min_frame_idx = range.min_frame_idx;
          flash_ctrl->max_frame_idx = range.max_frame_idx;

          SLOW(" Posting flash_bracket msg to bus, sof %d, min:%d max:%d",
            s_bundle->last_idx, range.min_frame_idx, range.max_frame_idx);
          if(mct_module_post_bus_msg(module, &bus_msg) != TRUE) {
            SERR("Failed to send flash_bracket msg");
          }
          /* enable internally */
          SERR("Marking Flash Bracket Enable!!");
          flash_bracket_params->ctrl.enable = TRUE;
        }

        /* set initial exposure settings, before stream_on */
        rc = module_sensor_params->func_tbl.process(
                module_sensor_params->sub_module_private,
                SENSOR_SET_AEC_INIT_SETTINGS, (void*)(&(stats_get.aec_get)));
        if (rc < 0) {
          SERR("failed");
          return FALSE;
        }

        /* enable focus bracketing for non-zsl snapshot */
        sensor_af_bracket_t *af_bracket_params =
           &(s_bundle->af_bracket_params);
        if (af_bracket_params->enable == TRUE) {
           rc = module_sensor_start_af_bracketing(module, s_bundle, event);
           if (!rc) {
              SERR("can't start af bracketing");
              return FALSE;
           }
        } else {
           SLOW("AF_bracketing not enabled");
        }
        /* enable multi-touch focus bracketing for non-zsl snapshot */
        sensor_af_bracket_t *mtf_bracket_params =
           &(s_bundle->mtf_bracket_params);
        if (mtf_bracket_params->enable == TRUE) {
           rc = module_sensor_start_mtf_bracketing(module, s_bundle, event);
           if (!rc) {
              SERR("can't start mtf bracketing");
              return FALSE;
           }
        } else {
           SLOW("MTF_bracketing not enabled");
        }

        ret = sensor_util_set_digital_gain_to_isp(module, s_bundle,
          event->identity);
        if (ret == FALSE)
          SERR("can't set digital gain");
      } /* if bayer */
      if (stream_on_flag == TRUE) {
        SHIGH("ide %x SENSOR_START_STREAM", event->identity);
        rc = module_sensor_params->func_tbl.process(
          module_sensor_params->sub_module_private, SENSOR_START_STREAM, NULL);
          if (rc < 0) {
            SERR("failed");
            return FALSE;
          }
          mct_bus_msg_t bus_msg;
          bus_msg.sessionid = s_bundle->sensor_info->session_id;
          bus_msg.type = MCT_BUS_MSG_SENSOR_STARTING;
          bus_msg.msg = NULL;
          bus_msg.thread_wait_time = s_bundle->stream_thread_wait_time;
          ALOGE("%s: Sending start bus message\n", __func__);
          if(mct_module_post_bus_msg(module, &bus_msg) == FALSE)
            SERR("failed");
      }
      break;
    }
    case SENSOR_SET_NEW_RESOLUTION: {
      boolean is_retry;
      ret = modules_sensor_set_new_resolution(module, event, s_bundle,
        module_sensor_params, &stream_on_cfg, &is_retry, stream_info);
      if (ret == FALSE) {
        SERR("failed");
        return FALSE;
      } else {
        if (is_retry == TRUE) {
          ret = modules_sensor_set_new_resolution(module, event, s_bundle,
            module_sensor_params, &stream_on_cfg, &is_retry, stream_info);
        }
      }
      memset(&sensor_out_info, 0, sizeof(sensor_out_info));
      rc = module_sensor_params->func_tbl.process(
             module_sensor_params->sub_module_private,
             SENSOR_GET_RESOLUTION_INFO, &sensor_out_info);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }
      break;
    }
    case SENSOR_SEND_EVENT:
      /* Call send_event to propogate event to next module*/
      ret = sensor_util_post_event_on_src_port(module, event);
      if (ret == FALSE) {
        SERR("failed");
        return FALSE;
      }
      break;
    case SENSOR_SET_CSIPHY_CFG:
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_GET_CUR_CSIPHY_CFG, &sensor_get);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }

      sensor_get.csiphy_params->csid_core =
      s_bundle->sensor_info->subdev_id[SUB_MODULE_CSID];

      if (sensor_out_info.csi_clk_scale_enable == 1) {
        rc = module_sensor_params->func_tbl.process(
          module_sensor_params->sub_module_private,
          SENSOR_GET_CSI_CLK_SCALE_CFG, &csi_clk);
        if ((rc < 0) || (csi_clk <= 0)) {
          SERR("failed: unable to scale csi clk");
          csi_clk = -1;
        }
      } else
        csi_clk = -1;

      sensor_get.csiphy_params->csiphy_clk = csi_clk;
      /* Need to Calculate settle cnt for given csi clk and DDR clk *
       * this is done as part of csi phy  kernel driver*/
      rc = csiphy_module_params->func_tbl.process(
        csiphy_module_params->sub_module_private,
        CSIPHY_SET_CFG, sensor_get.csiphy_params);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }
      break;
    case SENSOR_SET_CSID_CFG:

      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_GET_CUR_CSID_CFG, &sensor_get);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }

      sensor_get.csid_params->csi_clk = csi_clk;
      rc = csid_module_params->func_tbl.process(
        csid_module_params->sub_module_private,
        CSID_SET_CFG, sensor_get.csid_params);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }
      break;
    case SENSOR_LOAD_CHROMATIX: {
      mct_event_t new_event;
      sensor_chromatix_params_t   chromatix_params;
      modulesChromatix_t          module_chromatix;
      sensor_get_af_algo_ptr_t    af_algo_ptr;
      actuator_driver_params_t   *af_driver_ptr = NULL;
      module_sensor_params_t     *eeprom_module_params = NULL;
      module_sensor_params_t     *led_module_params = NULL;
      red_eye_reduction_type     *rer_chromatix = NULL;

      /* Initialize the params */
      af_algo_ptr.af_tune_ptr = NULL;
      af_algo_ptr.cam_mode = ACTUATOR_CAM_MODE_CAMERA;

      eeprom_module_params =
        s_bundle->module_sensor_params[SUB_MODULE_EEPROM];
      eeprom_set_chroma_af_t eeprom_set;
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_GET_CUR_CHROMATIX_NAME, &sensor_get);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }
      SLOW("chromatix name %s", sensor_get.chromatix_name.chromatix);
      if (!sensor_get.chromatix_name.chromatix ||
          !sensor_get.chromatix_name.common_chromatix) {
        SERR("failed common %s chromatix %s",
          sensor_get.chromatix_name.common_chromatix,
          sensor_get.chromatix_name.chromatix);
        return FALSE;
      }
      rc = chromatix_module_params->func_tbl.process(
        chromatix_module_params->sub_module_private,
        CHROMATIX_OPEN_LIBRARY, &sensor_get.chromatix_name);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }
      chromatix_params.stream_mask = s_bundle->stream_mask;
      rc = chromatix_module_params->func_tbl.process(
        chromatix_module_params->sub_module_private,
        CHROMATIX_GET_PTR, &chromatix_params);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }
      if (!chromatix_params.common_chromatix_ptr ||
          !chromatix_params.chromatix_ptr) {
        SERR("failed common %s chromatix %s",
          sensor_get.chromatix_name.common_chromatix,
          sensor_get.chromatix_name.chromatix);
        return FALSE;
      }
      if (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
          actuator_cam_mode_t   cam_mode = ACTUATOR_CAM_MODE_CAMERA;
          cam_stream_type_t     stream_type __attribute__((unused));

          stream_type = stream_info->stream_type;

          if(stream_on_cfg.stream_mask & (1 << CAM_STREAM_TYPE_VIDEO))
            cam_mode = ACTUATOR_CAM_MODE_CAMCORDER;

          SLOW("%s:%d stream_type rc %d cam_mode %d stream_type %d %d\n",
            __func__, __LINE__, rc,
            cam_mode, stream_type, stream_on_cfg.stream_mask);

          /* Set the cam mode */
          af_algo_ptr.cam_mode = cam_mode;
          SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
            ACTUATOR_GET_AF_ALGO_PARAM_PTR, &af_algo_ptr, rc);
          if (rc < 0 || af_algo_ptr.af_tune_ptr == NULL) {
            SERR("sensor_failure : ACTUATOR_GET_AF_ALGO_PARAM_PTR failed");
            return FALSE;
          }
      }
      if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] != -1) {
        struct msm_camera_i2c_reg_setting cal_setting;
        int is_insensor;
        eeprom_set.chromatix = chromatix_params;
        eeprom_set.af_driver_ptr = NULL;
        eeprom_set.af_algo_ptr[0] = NULL;
        eeprom_set.af_algo_ptr[1] = NULL;

        rc = eeprom_module_params->func_tbl.process(
          eeprom_module_params->sub_module_private,
          EEPROM_SET_CALIBRATE_CHROMATIX, &eeprom_set);
        if (rc < 0) {
          SERR("failed");
          return FALSE;
        }
      }

      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
          ACTUATOR_SET_PARAMETERS, NULL, rc);
      if (rc < 0) {
          SERR("sensor_failure : ACTUATOR_SET_PARAMETERS failed");
          return FALSE;
      }

      if (s_bundle->sensor_info->subdev_id[SUB_MODULE_LED_FLASH] != -1) {
        led_module_params =
            s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
        rer_chromatix =
            &(chromatix_params.chromatix_ptr->AEC_algo_data.red_eye_reduction);

        // Get (RER) data from chromatix
        if (led_module_params->func_tbl.process != NULL) {
          led_module_params->func_tbl.process(
            led_module_params->sub_module_private,
            LED_FLASH_GET_RER_CHROMATIX, rer_chromatix);
        }
      }

      module_chromatix.chromatixComPtr =
        chromatix_params.common_chromatix_ptr;
      module_chromatix.chromatixPtr = chromatix_params.chromatix_ptr;
      /* Send chromatix pointer downstream */
      new_event.type = MCT_EVENT_MODULE_EVENT;
      new_event.identity = event->identity;
      new_event.direction = MCT_EVENT_DOWNSTREAM;
      new_event.u.module_event.type = MCT_EVENT_MODULE_SET_CHROMATIX_PTR;
      new_event.u.module_event.module_event_data =
        (void *)&module_chromatix;
      ret = sensor_util_post_event_on_src_port(module, &new_event);
      if (ret == FALSE) {
        SERR("failed");
        return FALSE;
      }

      if (af_algo_ptr.af_tune_ptr != NULL) {
        ret = sensor_util_post_downstream_event(module, event->identity,
          MCT_EVENT_MODULE_SET_AF_TUNE_PTR, af_algo_ptr.af_tune_ptr);
        if (ret == FALSE) {
         SERR("failed");
         return FALSE;
        }
      }
      break;
    }

    default:
      SERR("invalid event %d", res_cfg_table->res_cfg_type[i]);
      break;
    }
  }
  if (s_bundle->fps_info.max_fps) {
    rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_SET_FPS, &s_bundle->fps_info);
    if (rc < 0) {
      SERR("failed");
      return FALSE;
    }
  }

  return TRUE;
}

/** module_sensor_hal_set_parm: process event for
 *  sensor module
 *
 *  @module_sensor_params: pointer to sensor module params
 *  @event_control: pointer to control data that is sent with
 *                 S_PARM
 *
 *  Return: TRUE / FALSE
 *
 *  This function handles  events associated with S_PARM * */
static boolean module_sensor_hal_set_parm(
   module_sensor_params_t *module_sensor_params,
   mct_event_control_parm_t *event_control)
{
   boolean   ret = TRUE;
   int32_t   rc = SENSOR_SUCCESS;

  switch(event_control->type){
    case CAM_INTF_PARM_SATURATION: {
      if (!event_control->parm_data) {
        SERR("failed parm_data NULL");
          ret = FALSE;
          break;
      }
      rc = module_sensor_params->func_tbl.process(
             module_sensor_params->sub_module_private,
             SENSOR_SET_SATURATION, event_control->parm_data);
      if (rc < 0) {
        SERR("failed");
        ret = FALSE;
        break;
      }
    }
      break;

    case CAM_INTF_PARM_CONTRAST: {
      if (!event_control->parm_data) {
        SERR("failed parm_data NULL");
        ret = FALSE;
        break;
      }
      rc = module_sensor_params->func_tbl.process(
             module_sensor_params->sub_module_private,
             SENSOR_SET_CONTRAST, event_control->parm_data);
      if (rc < 0) {
        SERR("failed");
        ret = FALSE;
        break;
      }
    }
      break;

    case CAM_INTF_PARM_SHARPNESS:{
      if (!event_control->parm_data) {
        SERR("failed parm_data NULL");
        ret = FALSE;
        break;
      }
      rc = module_sensor_params->func_tbl.process(
             module_sensor_params->sub_module_private,
             SENSOR_SET_SHARPNESS, event_control->parm_data);
      if (rc < 0) {
        SERR("failed");
        ret = FALSE;
        break;
      }
    }
      break;

    case CAM_INTF_PARM_ISO: {
      if (!event_control->parm_data) {
        SERR("failed parm_data NULL");
        ret = FALSE;
        break;
      }
      rc = module_sensor_params->func_tbl.process(
             module_sensor_params->sub_module_private,
             SENSOR_SET_ISO, event_control->parm_data);
      if (rc < 0) {
        SERR("failed");
        ret = FALSE;
        break;
      }
    }
      break;

    case CAM_INTF_PARM_EXPOSURE_COMPENSATION: {
      if (!event_control->parm_data) {
        SERR("failed parm_data NULL");
        ret = FALSE;
        break;
      }
      rc = module_sensor_params->func_tbl.process(
             module_sensor_params->sub_module_private,
             SENSOR_SET_EXPOSURE_COMPENSATION, event_control->parm_data);
      if (rc < 0) {
        SERR("failed");
        ret = FALSE;
        break;
      }
      break;
    }

    case CAM_INTF_PARM_ANTIBANDING: {
      if (!event_control->parm_data) {
        SERR("failed parm_data NULL");
        ret = FALSE;
        break;
      }
      rc = module_sensor_params->func_tbl.process(
             module_sensor_params->sub_module_private,
             SENSOR_SET_ANTIBANDING, event_control->parm_data);
      if (rc < 0) {
        SERR("failed");
        ret = FALSE;
        break;
      }
    }
      break;

    case CAM_INTF_PARM_BESTSHOT_MODE: {
      if (!event_control->parm_data) {
        SERR("failed parm_data NULL");
        ret = FALSE;
        break;
      }
      rc = module_sensor_params->func_tbl.process(
             module_sensor_params->sub_module_private,
             SENSOR_SET_BESTSHOT_MODE, event_control->parm_data);
      if (rc < 0) {
        SERR("failed");
        ret = FALSE;
        break;
      }
    }
      break;

    case CAM_INTF_PARM_EFFECT: {
      if (!event_control->parm_data) {
        SERR("failed parm_data NULL");
        ret = FALSE;
        break;
      }
      rc = module_sensor_params->func_tbl.process(
             module_sensor_params->sub_module_private,
             SENSOR_SET_EFFECT, event_control->parm_data);
      if (rc < 0) {
        SERR("failed");
        ret = FALSE;
        break;
      }
    }
      break;

    case CAM_INTF_PARM_WHITE_BALANCE: {
      if (!event_control->parm_data) {
        SERR("failed parm_data NULL");
        ret = FALSE;
        break;
      }
      rc = module_sensor_params->func_tbl.process(
             module_sensor_params->sub_module_private,
             SENSOR_SET_WHITE_BALANCE, event_control->parm_data);
      if (rc < 0) {
        SERR("failed");
        ret = FALSE;
        break;
      }
    }
      break;

       default:
       break;
      }
   return ret;
}

/** module_sensor_event_control_set_parm: process event for
 *  sensor module
 *
 *  @s_bundle: pointer to sensor bundle
 *  @control_data: pointer to control data that is sent with
 *               S_PARM
 *
 *  Return: TRUE / FALSE
 *
 *  This function handles all events associated with S_PARM * */

static boolean module_sensor_event_control_set_parm(
   mct_module_t *module, mct_event_t* event,
   sensor_bundle_info_t *bundle)
{
  boolean                      ret = TRUE;
  int32_t                      rc = SENSOR_SUCCESS;
  mct_list_t                  *s_list = NULL;
  module_sensor_bundle_info_t *s_bundle = bundle->s_bundle;
  module_sensor_params_t      *module_sensor_params =
    s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  if (!module_sensor_params || !event) {
    SERR("failed");
    ret = FALSE;
    goto ERROR;
  }
  mct_event_control_parm_t    *event_control =
    (mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);

  if (!event_control) {
    SERR("Invalid event data ptr");
    ret = FALSE;
    goto ERROR;
  }

  SLOW("event type =%d", event_control->type);
  switch (event_control->type) {
  case CAM_INTF_PARM_FPS_RANGE:
  {
    mct_event_t          new_event;
    sensor_fps_update_t  sensor_update_fps;
    sensor_lib_params_t *lib = s_bundle->sensor_lib_params;
    if (!event_control->parm_data) {
      SERR("failed parm_data NULL");
      ret = FALSE;
      break;
    }
    s_bundle->fps_info = *(cam_fps_range_t *)event_control->parm_data;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_FPS, event_control->parm_data);
    if (rc < 0) {
      SERR("failed");
      ret = FALSE;
      break;
    }
    if (lib->sensor_lib_ptr->sensor_output->output_format != SENSOR_YCBCR) {
      sensor_ctrl_t *ctrl
        = (sensor_ctrl_t *)module_sensor_params->sub_module_private;
      sensor_update_fps.max_fps = ctrl->s_data->cur_fps;
      new_event.type = MCT_EVENT_MODULE_EVENT;
      new_event.identity = event->identity;
      new_event.direction = MCT_EVENT_DOWNSTREAM;
      new_event.u.module_event.type = MCT_EVENT_MODULE_SENSOR_UPDATE_FPS;
      new_event.u.module_event.module_event_data = (void *)&sensor_update_fps;
      ret = sensor_util_post_event_on_src_port(module, &new_event);
      if (ret == FALSE) {
        SERR("failed");
        return FALSE;
      }
    }
    break;
  }
  case CAM_INTF_PARM_HFR:
    rc = module_sensor_params->func_tbl.process(
            module_sensor_params->sub_module_private,
            SENSOR_SET_HFR_MODE, event_control->parm_data);
    if (rc < 0) {
      SERR("failed");
      ret = FALSE;
    }
    break;
  case CAM_INTF_PARM_HDR:
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_HDR_AE_BRACKET, event_control->parm_data);
    if (rc < 0) {
      SERR("failed");
      ret = FALSE;
      break;
    }

    cam_exp_bracketing_t *ae_bracket_config =
      (cam_exp_bracketing_t*)event_control->parm_data;
    if (ae_bracket_config->mode == CAM_EXP_BRACKETING_ON) {
      ret = module_sensor_set_hdr_zsl_mode(module, event, s_bundle);
      if (ret == FALSE) {
        SERR("Failed at sensor but downstream propagation successful");
        ret = TRUE;
      }
      mct_bus_msg_t bus_msg;
      bus_msg.sessionid = s_bundle->sensor_info->session_id;
      bus_msg.type = MCT_BUS_MSG_PREPARE_HDR_ZSL_DONE;
      cam_prep_snapshot_state_t state;
      state = NEED_FUTURE_FRAME;
      bus_msg.msg = &state;
      if (mct_module_post_bus_msg(module, &bus_msg) != TRUE) {
        SERR("Failure posting to the bus!");
      }
    }
    break;
  case CAM_INTF_PARM_FOCUS_BRACKETING:
    ret = module_sensor_set_param_af_bracket(s_bundle, event_control);
    if (ret == FALSE) {
      SERR("failed");
    }
    break;
  case CAM_INTF_PARM_MULTI_TOUCH_FOCUS_BRACKETING:
    ret = module_sensor_set_param_mtf_bracket(s_bundle, event_control);
    if (ret == FALSE) {
      SERR("failed");
    }
    break;
  case CAM_INTF_PARM_FLASH_BRACKETING:{
    sensor_bracket_params_t *flash_bracket_params = NULL;
    cam_flash_bracketing_t        *cam_flash_brkt = NULL;

    if (!event_control || !event_control->parm_data ) {
      SERR("Invalid event ptr");
      return FALSE;
    }

    cam_flash_brkt = (cam_flash_bracketing_t*) event_control->parm_data;
    flash_bracket_params = &(s_bundle->flash_bracket_params);
    /* Currently only expect burst of 2 */
    if (cam_flash_brkt->enable && cam_flash_brkt->burst_count != 2) {
      SERR("Invalid flash brkt burst num : %d, abort",
        cam_flash_brkt->burst_count);
      return FALSE;
    }
    SHIGH("set flash_bracket");
    memset(flash_bracket_params, 0, sizeof(sensor_bracket_ctrl_t));
    /* Copy from HAL */
    flash_bracket_params->flash_bracket.burst_count =
      cam_flash_brkt->burst_count;
    flash_bracket_params->flash_bracket.enable = cam_flash_brkt->enable;
    if (!cam_flash_brkt->enable) {
      /* Post downstream event to off bracketting */
      mct_bracket_ctrl_t bracket_ctrl;
      bracket_ctrl.state = MCT_BRACKET_CTRL_OFF;
      rc = sensor_util_post_downstream_event(module, event->identity,
        MCT_EVENT_MODULE_SENSOR_BRACKET_CTRL, &bracket_ctrl);
    }
    break;
  }
  case CAM_INTF_PARM_VIDEO_HDR:{
    int32_t tmp = *(int32_t *)event_control->parm_data;
    boolean need_restart = TRUE;
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_SET_VIDEO_HDR_ENABLE, event_control->parm_data);
      if (rc == SENSOR_FAILURE) {
        SERR("failed");
        ret = FALSE;
      } else if (rc == SENSOR_ERROR_INVAL) {
        /* This indicates that we are already in this mode
           so we do not need stream restart */
        need_restart = FALSE;
      }

      sensor_out_info_t sensor_out_info;
      mct_event_t new_event;
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_GET_RESOLUTION_INFO, &sensor_out_info);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }

      if((s_bundle->stream_on_count > 0) && need_restart) {
        mct_bus_msg_t bus_msg;
        mct_bus_msg_error_message_t cmd;

        cmd = MCT_ERROR_MSG_RSTART_VFE_STREAMING;

        bus_msg.sessionid = bundle->session_id;
        bus_msg.type = MCT_BUS_MSG_ERROR_MESSAGE;
        bus_msg.size = sizeof(cmd);
        bus_msg.msg = &cmd;
        ALOGE("psiven %s Restart",__func__);
        if(!mct_module_post_bus_msg(module, &bus_msg))
          SERR("Failed to send message to bus");
      }
    }
    break;
  case CAM_INTF_PARM_SENSOR_HDR:{
    int32_t tmp = *(int32_t *)event_control->parm_data;
    boolean need_restart = TRUE;
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_SET_SNAPSHOT_HDR_ENABLE, event_control->parm_data);
      if (rc == SENSOR_FAILURE) {
        SERR("failed");
        ret = FALSE;
      } else if (rc == SENSOR_ERROR_INVAL) {
        /* This indicates that we are already in this mode
           so we do not need stream restart */
        need_restart = FALSE;
      }

      sensor_out_info_t sensor_out_info;
      mct_event_t new_event;
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_GET_RESOLUTION_INFO, &sensor_out_info);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }

      if((s_bundle->stream_on_count > 0) && need_restart) {
        mct_bus_msg_t bus_msg;
        mct_bus_msg_error_message_t cmd;

        cmd = MCT_ERROR_MSG_RSTART_VFE_STREAMING;

        bus_msg.sessionid = bundle->session_id;
        bus_msg.type = MCT_BUS_MSG_ERROR_MESSAGE;
        bus_msg.size = sizeof(cmd);
        bus_msg.msg = &cmd;
        ALOGE("psiven %s Restart",__func__);
        if(!mct_module_post_bus_msg(module, &bus_msg))
          SERR("Failed to send message to bus");
      }
    }
    break;
  case CAM_INTF_PARM_DIS_ENABLE:
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_DIS_ENABLE, event_control->parm_data);
    if (rc < 0) {
      SERR("failed");
      ret = FALSE;
    }
    break;
  case CAM_INTF_PARM_LONGSHOT_ENABLE : {
    int8_t mode = *((int8_t *)event_control->parm_data);
     s_bundle->longshot = mode;
     break;
  }
  case CAM_INTF_PARM_LED_MODE: {
    int32_t mode = *((int32_t *)event_control->parm_data);
    module_sensor_params_t        *led_module_params = NULL;
    led_module_params = s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
    SERR("CAM_INTF_PARM_LED_MODE %d \n", mode);
    if (mode == LED_MODE_TORCH) {
      if (led_module_params->func_tbl.process != NULL) {
        rc = led_module_params->func_tbl.process(
          led_module_params->sub_module_private,
          LED_FLASH_SET_TORCH, NULL);
        if (rc < 0) {
          SERR("failed: LED_FLASH_SET_TORCH");
        } else {
          s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_TORCH;
          s_bundle->torch_on = 1;
          sensor_util_post_led_state_msg(module, s_bundle, event->identity);
        }
      }
    } else {
      if (led_module_params->func_tbl.process != NULL) {
        rc = led_module_params->func_tbl.process(
          led_module_params->sub_module_private,
          LED_FLASH_SET_OFF, NULL);
        if (rc < 0) {
          SERR("failed: LED_FLASH_SET_OFF");
        } else {
          s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
          s_bundle->torch_on = 0;
          sensor_util_post_led_state_msg(module, s_bundle, event->identity);
        }
      }
    }
    break;
  }

  case CAM_INTF_PARM_MAX_DIMENSION:
    SERR("CAM_INTF_PARM_MAX_DIMENSION");
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_MAX_DIMENSION, event_control->parm_data);
    if (rc < 0) {
      SERR("failed");
      ret = FALSE;
    }
    break;

  case CAM_INTF_PARM_SET_AUTOFOCUSTUNING: {
    module_sensor_params_t      *actuator_module_params =
          s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];
    if (!actuator_module_params) {
      SERR("failed");
      ret = FALSE;
      goto ERROR;
    }
    if (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
      actuator_module_params->func_tbl.process(
        actuator_module_params->sub_module_private,
        ACTUATOR_FOCUS_TUNING, event_control->parm_data);
      if (rc < 0) {
        SERR("failed");
        ret = FALSE;
      }
    }
  }
    break;

  case CAM_INTF_PARM_SET_RELOAD_CHROMATIX: {
    module_sensor_params_t      *chromatix_module_params =
      s_bundle->module_sensor_params[SUB_MODULE_CHROMATIX];
    sensor_chromatix_params_t chromatix_params;
    modulesChromatix_t module_chromatix;
    mct_event_module_t event_module;
    mct_event_t new_event;
    if (!event_control->parm_data) {
      SERR("failed parm_data NULL");
      ret = FALSE;
      break;
    }
    tune_chromatix_t *chromatix = (tune_chromatix_t *)event_control->parm_data;
    chromatix_params.stream_mask = s_bundle->stream_mask;
    rc = chromatix_module_params->func_tbl.process(
    chromatix_module_params->sub_module_private,
     CHROMATIX_GET_PTR, &chromatix_params);
    if (rc < 0) {
      SERR("failed");
      ret = FALSE;
      break;
    }

    memcpy(chromatix_params.chromatix_ptr,
      &chromatix->chromatixData, sizeof(chromatix_parms_type));
    memcpy(chromatix_params.snapchromatix_ptr,
      &chromatix->snapchromatixData, sizeof(chromatix_parms_type));
    memcpy(chromatix_params.common_chromatix_ptr,
      &chromatix->common_chromatixData, sizeof(chromatix_VFE_common_type));

    module_chromatix.chromatixComPtr =
      chromatix_params.common_chromatix_ptr;
    module_chromatix.chromatixPtr = chromatix_params.chromatix_ptr;
    /* Send chromatix pointer downstream */
    new_event.type = MCT_EVENT_MODULE_EVENT;
    new_event.identity = event->identity;
    new_event.direction = MCT_EVENT_DOWNSTREAM;
    new_event.u.module_event.type = MCT_EVENT_MODULE_SET_RELOAD_CHROMATIX;
    new_event.u.module_event.module_event_data =
      (void *)&module_chromatix;
    ret = sensor_util_post_event_on_src_port(module, &new_event);
    if (ret == FALSE) {
      SERR("failed");
      return FALSE;
    }
  break;
  }
  case CAM_INTF_PARM_SET_RELOAD_AFTUNE: {
    module_sensor_params_t      *actuator_module_params = NULL;
    sensor_get_af_algo_ptr_t    af_algo_ptr;
    actuator_driver_params_t    *af_driver_ptr = NULL;
    af_algo_tune_parms_t        *af_tune_ptr = NULL;
	mct_event_t                 new_event;

    if (!event_control->parm_data) {
      SERR("failed parm_data NULL");
      ret = FALSE;
      break;
    }
    tune_autofocus_t *afptr = (tune_autofocus_t *)event_control->parm_data;
    if (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
      actuator_module_params =
        s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];
      af_algo_ptr.cam_mode = ACTUATOR_CAM_MODE_CAMERA;
      rc = actuator_module_params->func_tbl.process(
        actuator_module_params->sub_module_private,
        ACTUATOR_GET_AF_ALGO_PARAM_PTR, &af_algo_ptr);
      if (rc < 0 || !af_algo_ptr.af_tune_ptr) {
        SERR("failed rc %d af_tune_ptr %p", rc, af_algo_ptr.af_tune_ptr);
        return FALSE;
      }

      rc = actuator_module_params->func_tbl.process(
        actuator_module_params->sub_module_private,
        ACTUATOR_GET_AF_DRIVER_PARAM_PTR, &af_driver_ptr);
      if (rc < 0 || !af_driver_ptr) {
        SERR("failed rc %d af_tune_ptr %p", rc, af_driver_ptr);
        return FALSE;
      }

     memcpy(af_driver_ptr, afptr->af_tuneData,
       sizeof(actuator_driver_params_t));
     memcpy(af_algo_ptr.af_tune_ptr,
       afptr->af_tuneData + sizeof(actuator_driver_params_t),
       sizeof(af_algo_tune_parms_t));

	 new_event.type = MCT_EVENT_MODULE_EVENT;
	 new_event.identity = event->identity;
	 new_event.direction = MCT_EVENT_DOWNSTREAM;
     new_event.u.module_event.type = MCT_EVENT_MODULE_SET_RELOAD_AFTUNE;
	 new_event.u.module_event.module_event_data =
		 (void *)af_algo_ptr.af_tune_ptr;
	 ret = sensor_util_post_event_on_src_port(module, &new_event);
     if (ret == FALSE) {
        SERR("failed");
        return FALSE;
     }
    }
    break;
  }

  case CAM_INTF_PARM_REDEYE_REDUCTION: {
    void *mode = event_control->parm_data;
    module_sensor_params_t        *led_module_params = NULL;
    led_module_params = s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
    if (led_module_params->func_tbl.process != NULL) {
      rc = led_module_params->func_tbl.process(
        led_module_params->sub_module_private,
        LED_FLASH_SET_RER_PARAMS, mode);
    }
    if (rc < 0) {
      SERR("failed");
      ret = FALSE;
    }
  break;
  }
  case CAM_INTF_PARM_FRAMESKIP:{

    if (!event_control->parm_data) {
      SERR("failed parm_data NULL");
      ret = FALSE;
      break;
    }

    s_bundle->hal_frame_skip_pattern = *((int32_t*)event_control->parm_data);

    break;
  }
  case CAM_INTF_PARM_SENSOR_DEBUG_MASK: {
      int32_t *log_level = (int32_t *)event_control->parm_data;
      SLOW("%s: %d: mask: %d",__func__, __LINE__, *log_level);
      if (*log_level >
        SENSOR_DEBUG_MASK_MAX ) {
         sensordebug_mask = 2;
         break;
      }
      sensordebug_mask = *log_level;
      break;
  }

  case CAM_INTF_PARM_EXPOSURE_TIME:{
    uint64_t exposure_time = *((uint64_t *)event_control->parm_data);
    uint8_t manual_exposure_mode = exposure_time > 0 ? 1 : 0;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_MANUAL_EXPOSURE_MODE, &manual_exposure_mode);
    if (rc < 0) {
      SERR("Set Manual Exposure Mode failed");
      return FALSE;
    }
    /*Limit increase for minimum 8 sec exposure time*/
    s_bundle->stream_thread_wait_time =
      (exposure_time/1000000000 > 5) ? (exposure_time/1000000000 + 3) : 8;
  }
  break;

  default:{
    sensor_output_format_t output_format;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_GET_SENSOR_FORMAT, &output_format);
     if(output_format == SENSOR_YCBCR) {
     ret = module_sensor_hal_set_parm(
       module_sensor_params, event_control);
    }
   }
   break;
  }
ERROR:
  return ret;
}

/** module_sensor_handle_parm_raw_dimension: process event for
 *  sensor module
 *
 *  @module: pointer to sensor mct module
 *  @s_bundle: handle to sensor bundle
 *  @module_sensor_params: handle to sensor sub module
 *  @identity: identity of current stream
 *  @event_data: event data associated with this event
 *
 *  This function handles CAM_INTF_PARM_RAW_DIMENSION event
 *
 *  Return: TRUE for success
 *          FALSE for failure
 **/
static boolean module_sensor_handle_parm_raw_dimension(
  mct_module_t *module, module_sensor_bundle_info_t *s_bundle,
  uint32_t identity, void *event_data)
{
  int32_t                     rc = 0;
  boolean                     ret = FALSE;
  sensor_get_raw_dimension_t  sensor_get;
  module_sensor_params_t     *module_sensor_params = NULL;

  /* Validate input parameters */
  if (!s_bundle || !event_data) {
    SERR("failed: invalid params %p %p %p", s_bundle, module_sensor_params,
      event_data);
    return FALSE;
  }

  if (!s_bundle || !s_bundle->module_sensor_params[SUB_MODULE_SENSOR]) {
    SERR("failed: invalid sensor params");
    return FALSE;
  }

  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  if (!module_sensor_params->func_tbl.process) {
    SERR("failed: invalid sensor function process pointer %p",
      module_sensor_params->func_tbl.process);
    return FALSE;
  }

  memset(&sensor_get, 0, sizeof(sensor_get));
  sensor_get.raw_dim = event_data;
  sensor_get.stream_mask = s_bundle->stream_mask;

  SLOW("stream mask %x", sensor_get.stream_mask);
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_GET_RAW_DIMENSION, &sensor_get);
  if (rc < 0) {
    SERR("failed: SENSOR_GET_RAW_DIMENSION rc %d", rc);
    ret = FALSE;
  }

  return TRUE;
}

/** module_sensor_event_control_get_parm: process event for
 *  sensor module
 *
 *  @module: pointert to sensor mct module
 *  @event: event to be handled
 *  @s_bundle: pointer to sensor bundle for this sensor
 *
 *  This function handles all events associated with G_PARM
 *
 *  Return: TRUE for success
 *          FALSE for failure
 **/
static boolean module_sensor_event_control_get_parm(
   mct_module_t *module, mct_event_t* event,
   module_sensor_bundle_info_t *s_bundle)
{
  boolean                      ret = TRUE;
  int32_t                      rc = SENSOR_SUCCESS;
  mct_list_t                  *s_list = NULL;
  module_sensor_params_t      *module_sensor_params =
    s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  module_sensor_params_t      *chromatix_module_params =
    s_bundle->module_sensor_params[SUB_MODULE_CHROMATIX];
  module_sensor_params_t      *actuator_module_params =
          s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];
  if (!module_sensor_params || !event ||
    !chromatix_module_params || !actuator_module_params) {
    SERR("failed");
    ret = FALSE;
    goto ERROR;
  }
  mct_event_control_parm_t    *event_control =
    (mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);
  switch (event_control->type) {
    case CAM_INTF_PARM_GET_CHROMATIX: {
      sensor_chromatix_params_t chromatix_params;
      if (!event_control->parm_data) {
        SERR("failed parm_data NULL");
        ret = FALSE;
        break;
      }
      tune_chromatix_t *chromatix =
        (tune_chromatix_t *)event_control->parm_data;
      chromatix_params.stream_mask = s_bundle->stream_mask;
      rc = chromatix_module_params->func_tbl.process(
      chromatix_module_params->sub_module_private,
       CHROMATIX_GET_PTR, &chromatix_params);
      if (rc < 0) {
        SERR("failed");
        ret = FALSE;
        break;
      }
      memcpy(&chromatix->chromatixData, chromatix_params.chromatix_ptr,
        sizeof(chromatix_parms_type));
      memcpy(&chromatix->snapchromatixData, chromatix_params.snapchromatix_ptr,
        sizeof(chromatix_parms_type));
      memcpy(&chromatix->common_chromatixData,
        chromatix_params.common_chromatix_ptr,
        sizeof(chromatix_VFE_common_type));
    break;
  }
   case CAM_INTF_PARM_GET_AFTUNE: {
      sensor_get_af_algo_ptr_t    af_algo_ptr;
      actuator_driver_params_t *af_driver_ptr = NULL;
      if (!event_control->parm_data) {
        SERR("failed parm_data NULL");
        ret = FALSE;
        break;
      }
      tune_autofocus_t *afptr = (tune_autofocus_t *)event_control->parm_data;
      if (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
        rc = actuator_module_params->func_tbl.process(
          actuator_module_params->sub_module_private,
          ACTUATOR_GET_AF_DRIVER_PARAM_PTR, &af_driver_ptr);
        if (rc < 0 || !af_driver_ptr) {
          SERR("failed rc %d af_tune_ptr %p", rc, af_driver_ptr);
          return FALSE;
        }

        af_algo_ptr.cam_mode = ACTUATOR_CAM_MODE_CAMERA;
        af_algo_ptr.af_tune_ptr = NULL;
        rc = actuator_module_params->func_tbl.process(
          actuator_module_params->sub_module_private,
          ACTUATOR_GET_AF_ALGO_PARAM_PTR, &af_algo_ptr);
        if (rc < 0 || !af_algo_ptr.af_tune_ptr) {
          SERR("failed rc %d af_tune_ptr %p", rc, af_algo_ptr.af_tune_ptr);
          return FALSE;
        }

        void *driver_tune_ptr = afptr->af_tuneData;
        void *algo_tune_ptr =
          (void *) ((size_t)driver_tune_ptr + sizeof(actuator_driver_params_t));

        memcpy(driver_tune_ptr, af_driver_ptr,
          sizeof(actuator_driver_params_t));
        memcpy(algo_tune_ptr, af_algo_ptr.af_tune_ptr,
          sizeof(af_algo_tune_parms_t));
      }

    break;
  }

  case CAM_INTF_PARM_RAW_DIMENSION:
    ret = module_sensor_handle_parm_raw_dimension(module, s_bundle,
      event->identity, event_control->parm_data);
    if (ret == FALSE) {
      SERR("failed: module_sensor_handle_parm_raw_dimension");
    }
    break;

  default:
    break;
  }
ERROR:
  return ret;
}

/** module_sensor_event_control_parm_stream_buf: process event for
 *  sensor module
 *
 *  @s_bundle: pointer to sensor bundle
 *  @control_data: pointer to control data that is sent with
 *               S_PARM
 *
 *  Return: TRUE / FALSE
 *
 *  This function handles all events associated with S_PARM * */

static boolean module_sensor_event_control_parm_stream_buf(
   mct_module_t *module, mct_event_t* event,
   module_sensor_bundle_info_t *s_bundle)
{
  boolean                      ret = TRUE;

  if (!event) {
    SERR("failed");
    ret = FALSE;
    goto ERROR;
  }

  cam_stream_parm_buffer_t   *stream_parm =
    event->u.ctrl_event.control_event_data;

  switch (stream_parm->type) {

  case CAM_STREAM_PARAM_TYPE_SET_BUNDLE_INFO: {
    SLOW("CAM_STREAM_PARAM_TYPE_SET_BUNDLE_INFO");

    sensor_util_assign_bundle_id(module, event->identity,
       &stream_parm->bundleInfo);
    break;
  }
  default:
    break;
  }

ERROR:
  return ret;
}

/** module_sensor_process_event: process event for sensor
 *  module
 *
 *  @streamid: streamid associated with event
 *  @module: mct module handle
 *  @event: event to be processed
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles all events and sends those events
 *  downstream / upstream *   */

static boolean module_sensor_module_process_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean                      ret = TRUE;
  int32_t                      rc = SENSOR_SUCCESS;
  module_sensor_ctrl_t        *module_ctrl = NULL;
  mct_event_control_t         *event_ctrl = NULL;
  sensor_bundle_info_t         bundle_info;

  if (!module || !event) {
    SERR("failed port %p event %p", module,
      event);
    return FALSE;
  }
  if (event->type != MCT_EVENT_CONTROL_CMD) {
    SERR("failed invalid event type %d",
      event->type);
    return FALSE;
  }

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  if (!module_ctrl) {
    SERR("failed");
    return FALSE;
  }

  event_ctrl = &event->u.ctrl_event;

  memset(&bundle_info, 0, sizeof(sensor_bundle_info_t));
  ret = sensor_util_get_sbundle(module, event->identity, &bundle_info);
  if (ret == FALSE) {
    SERR("failed");
    return FALSE;
  }
  SLOW("event id %d", event_ctrl->type);

  if (event_ctrl->type == MCT_EVENT_CONTROL_PREPARE_SNAPSHOT) {
    sensor_output_format_t output_format;
    mct_bus_msg_t bus_msg;
    module_sensor_params_t *module_sensor_params = NULL;

    bundle_info.s_bundle->state = 0;
    bundle_info.s_bundle->regular_led_trigger = 0;
    module_sensor_params =
      bundle_info.s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_GET_SENSOR_FORMAT, &output_format);
    SLOW("in Prepare snapshot, sensor type is %d\n", output_format);
    if (output_format == SENSOR_YCBCR) {
      bus_msg.sessionid = bundle_info.s_bundle->sensor_info->session_id;
      bus_msg.type = MCT_BUS_MSG_PREPARE_HW_DONE;
      cam_prep_snapshot_state_t state;
      state = DO_NOT_NEED_FUTURE_FRAME;
      bus_msg.msg = &state;
      if (mct_module_post_bus_msg(module,&bus_msg)!=TRUE)
        SERR("Failure posting to the bus!");
      return TRUE;
    }
  }
  switch (event_ctrl->type) {
  case MCT_EVENT_CONTROL_STREAMON:
    SLOW("CT_EVENT_CONTROL_STREAMON");
    memcpy(&module_ctrl->streaminfo, event->u.ctrl_event.control_event_data,
      sizeof(mct_stream_info_t));
    ret = module_sensor_stream_on(module, event, bundle_info.s_bundle);
    if (ret == FALSE) {
      SERR("failed");
      break;
    }
    break;
  case MCT_EVENT_CONTROL_STREAMOFF: {
    mct_stream_info_t* stream_info = (mct_stream_info_t*)
      event->u.ctrl_event.control_event_data;
    SHIGH("ide %x MCT_EVENT_CONTROL_STREAMOFF", event->identity);
    SLOW("CT_EVENT_CONTROL_STREAMOFF");
    memcpy(&module_ctrl->streaminfo, event->u.ctrl_event.control_event_data,
      sizeof(mct_stream_info_t));
    module_sensor_params_t *module_sensor_params =
      bundle_info.s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
    if (!module_sensor_params) {
      SERR("failed");
      ret = FALSE;
      break;
    }
    /* Call send_event to propogate event to next module*/
    ret = sensor_util_post_event_on_src_port(module, event);
    if (ret == FALSE) {
      SERR("failed");
      break;
    }
    if (TRUE == module_sensor_is_ready_for_stream_off(module, event,
      bundle_info.s_bundle)) {

      SHIGH("ide %x MCT_EVENT_CONTROL_STREAMOFF", event->identity);

      sensor_af_bracket_t *af_bracket =
        &(bundle_info.s_bundle->af_bracket_params);
      sensor_af_bracket_t *mtf_bracket =
        &(bundle_info.s_bundle->mtf_bracket_params);
      sensor_bracket_params_t *flash_bracket_params =
        &(bundle_info.s_bundle->flash_bracket_params);

      /* Check whether this is live snapshot stream ON */
      if ((bundle_info.s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)) &&
          (stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT)) {
        /* Unload live snapshot chromatix */
        module_sensor_params_t *chromatix_module_params = NULL;
        mct_event_t new_event;
        modules_liveshot_Chromatix_t module_chromatix;

        memset(&new_event, 0, sizeof(event));
        chromatix_module_params =
          bundle_info.s_bundle->module_sensor_params[SUB_MODULE_CHROMATIX];

        module_chromatix.liveshot_chromatix_ptr = NULL;
        /* Send chromatix pointer downstream */
        new_event.type = MCT_EVENT_MODULE_EVENT;
        new_event.identity = event->identity;
        new_event.direction = MCT_EVENT_DOWNSTREAM;
        new_event.u.module_event.type =
          MCT_EVENT_MODULE_SET_LIVESHOT_CHROMATIX_PTR;
        new_event.u.module_event.module_event_data = (void *)&module_chromatix;
        rc = sensor_util_post_event_on_src_port(module, &new_event);
        if (rc == FALSE) {
          SERR("failed");
        }
        rc = chromatix_module_params->func_tbl.process(
          chromatix_module_params->sub_module_private,
          CHROMATIX_CLOSE_LIVESHOT_LIBRARY, NULL);
        if (rc < 0) {
         SERR("failed");
        }
      }
      /*If streaming off preview, then turn off LED*/
      if (stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT ||
          stream_info->stream_type == CAM_STREAM_TYPE_POSTVIEW ||
          stream_info->stream_type == CAM_STREAM_TYPE_RAW ||
          bundle_info.s_bundle->longshot)
      {
        SLOW ("stream off preview or snapshot, turn off LED");
        module_sensor_params_t        *led_module_params = NULL;
        led_module_params =
          bundle_info.s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
        if (led_module_params->func_tbl.process != NULL) {
          rc = led_module_params->func_tbl.process(
          led_module_params->sub_module_private,
          LED_FLASH_SET_OFF, NULL);
          if (rc < 0) {
            SERR("failed: LED_FLASH_SET_OFF");
          } else {
            bundle_info.s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
            sensor_util_post_led_state_msg(module, bundle_info.s_bundle,
              event->identity);
          }
        }
      }

      mct_bus_msg_t bus_msg;
      bus_msg.sessionid = bundle_info.s_bundle->sensor_info->session_id;
      bus_msg.type = MCT_BUS_MSG_SENSOR_STOPPING;
      bus_msg.msg = NULL;
      ALOGE("%s: Sending stop bus message\n", __func__);
      if(mct_module_post_bus_msg(module, &bus_msg) == FALSE)
        SERR("failed");
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_STOP_STREAM, NULL);
      if (rc < 0) {
        SERR("failed");
        ret = FALSE;
        break;
      }

      if (stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW) {
        bundle_info.s_bundle->last_idx = 0;
      }
    }
    break;
  }
  case MCT_EVENT_CONTROL_SET_PARM: {
    ret = module_sensor_event_control_set_parm(
       module, event, &bundle_info);

    if (ret == FALSE) {
      SERR("failed");
    }
    mct_event_control_parm_t    *event_control =
      (mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);

    module_sensor_params_t *module_sensor_params =
      bundle_info.s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
    sensor_bracket_params_t *af_bracket_params =
      &(bundle_info.s_bundle->af_bracket_params);
    sensor_bracket_params_t *mtf_bracket_params =
      &(bundle_info.s_bundle->mtf_bracket_params);
    sensor_bracket_params_t *flash_bracket_params =
      &(bundle_info.s_bundle->flash_bracket_params);

    sensor_output_format_t output_format;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_GET_SENSOR_FORMAT, &output_format);
    if(output_format == SENSOR_BAYER ||
        (event_control->type == CAM_INTF_PARM_ZOOM ) ||
        (event_control->type == CAM_INTF_PARM_FD ) ||
        (event_control->type == CAM_INTF_PARM_VT) ||
        (event_control->type == CAM_INTF_PARM_FRAMESKIP) ||
        (event_control->type == CAM_INTF_PARM_FPS_RANGE)) {
      // Frame skip during bracketing must not be forwarded
      if (!((CAM_INTF_PARM_FRAMESKIP == event_control->type) &&
          (af_bracket_params->ctrl.enable || mtf_bracket_params->ctrl.enable)&&
          (flash_bracket_params->ctrl.enable))) {
        /* Call send_event to propogate event to next module*/
        ret = sensor_util_post_event_on_src_port(module, event);
        if (ret == FALSE) {
          SERR("failed");
          return FALSE;
        }
      }
    }
    break;
  }

  case MCT_EVENT_CONTROL_GET_PARM: {
    ret = module_sensor_event_control_get_parm(
       module, event, bundle_info.s_bundle);

    if (ret == FALSE) {
      SERR("failed");
    }
    /* Call send_event to propogate event to next module*/
    ret = sensor_util_post_event_on_src_port(module, event);
    if (ret == FALSE) {
      SERR("failed");
      return FALSE;
    }
    break;
  }

  case MCT_EVENT_CONTROL_PARM_STREAM_BUF: {
    ret = module_sensor_event_control_parm_stream_buf(
       module, event, bundle_info.s_bundle);

    if (ret == FALSE) {
      SERR("failed");
    }
    /* Call send_event to propogate event to next module*/
    ret = sensor_util_post_event_on_src_port(module, event);
    if (ret == FALSE) {
      SERR("failed");
      return FALSE;
    }
    break;
  }

  case MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT: {
      stats_get_data_t stats_get;
      mct_event_t new_event;
      float digital_gain = 0;
      sensor_output_format_t output_format;
      memset(&stats_get, 0, sizeof(stats_get_data_t));
      module_sensor_params_t *module_sensor_params =
      bundle_info.s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
      module_sensor_params_t *actuator_module_params =
        bundle_info.s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];
      SERR("zsl capture start ");
      SLOW("propagate zsl capture downstream");
      ret = sensor_util_post_event_on_src_port(module, event);

      rc = module_sensor_params->func_tbl.process(
              module_sensor_params->sub_module_private,
              SENSOR_GET_SENSOR_FORMAT, &output_format);
      if (rc < 0) {
        SERR("failed");
      } else
        SERR  (" Get data from stats");
        new_event.type = MCT_EVENT_MODULE_EVENT;
        new_event.identity = event->identity;
        new_event.direction = MCT_EVENT_DOWNSTREAM;
        new_event.u.module_event.type = MCT_EVENT_MODULE_STATS_GET_DATA;
        new_event.u.module_event.module_event_data = (void *)&stats_get;
        ret = sensor_util_post_event_on_src_port(module, &new_event);
        if (ret == FALSE) {
          SERR("failed");
          /* Continue START STREAM since this is not FATAL error */
          ret = TRUE;
        } else if (stats_get.flag & STATS_UPDATE_AEC) {

          mct_bus_msg_t bus_msg;
          SLOW("led capture, real gain: %f, linecount: %d, lux_idx: %f",
            stats_get.aec_get.real_gain[0], stats_get.aec_get.linecount[0],
            stats_get.aec_get.lux_idx);
          /* Send bus msg for cpp reprocessing */
          if (sizeof(stats_get_data_t) >
            sizeof(bundle_info.s_bundle->aec_metadata.private_data)) {
            SERR("failed");
          } else {
            memcpy(bundle_info.s_bundle->aec_metadata.private_data, &stats_get,
              sizeof(stats_get_data_t));
            bus_msg.sessionid = bundle_info.s_bundle->sensor_info->session_id;
            bus_msg.type = MCT_BUS_MSG_SET_STATS_AEC_INFO;
            bus_msg.size = sizeof(stats_get_data_t);
            bus_msg.msg = (void *)&bundle_info.s_bundle->aec_metadata;
            SLOW("bus msg size %d", bus_msg.size);
            if(mct_module_post_bus_msg(module, &bus_msg) == FALSE)
              SERR("failed");
          }

          aec_update_t aec_update;
          aec_update.real_gain = stats_get.aec_get.real_gain[0];
          aec_update.linecount = stats_get.aec_get.linecount[0];
          /* Save LED off gain and line count */
          if (bundle_info.s_bundle->flash_bracket_params.flash_bracket.enable) {
            bundle_info.s_bundle->led_off_gain =
              stats_get.aec_get.led_off_cf_gain;
            bundle_info.s_bundle->led_off_linecount =
              stats_get.aec_get.led_off_cf_linecount;
          } else {
            bundle_info.s_bundle->led_off_gain =
              stats_get.aec_get.led_off_gain;
            bundle_info.s_bundle->led_off_linecount =
              stats_get.aec_get.led_off_linecount;
          }
          SERR("get stats led trigger %d ",stats_get.aec_get.trigger_led);
          if (stats_get.aec_get.trigger_led)
          {
            module_sensor_params_t        *led_module_params = NULL;
            led_module_params =
              bundle_info.s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
            if (led_module_params->func_tbl.process != NULL) {
              int flash_mode;
              if(bundle_info.s_bundle->longshot)
                  flash_mode = LED_FLASH_SET_TORCH;
              else
                  flash_mode  = LED_FLASH_SET_MAIN_FLASH;
              rc = led_module_params->func_tbl.process(
                led_module_params->sub_module_private,
                flash_mode , NULL);
              if (rc < 0) {
                bundle_info.s_bundle->sensor_params.flash_mode =
                  CAM_FLASH_MODE_OFF;
                SERR("failed: LED_FLASH_SET_MAIN_FLASH");
              } else {
                bundle_info.s_bundle->sensor_params.flash_mode =
                  CAM_FLASH_MODE_ON;
              }
              sensor_util_post_led_state_msg(module, bundle_info.s_bundle,
                event->identity);
            }

            SLOW("led_off stats gain %f linecount %d",
              stats_get.aec_get.led_off_gain,
              stats_get.aec_get.led_off_linecount);

              bundle_info.s_bundle->regular_led_trigger = 0;
            SLOW(" led zsl capture start ");
            rc = module_sensor_params->func_tbl.process(
              module_sensor_params->sub_module_private,
            SENSOR_SET_WAIT_FRAMES, &bundle_info.s_bundle->num_skip);


            /* Fill metadata msg */
            mct_bus_msg_t bus_msg;
            bus_msg.sessionid = bundle_info.session_id;
            bus_msg.type = MCT_BUS_MSG_ZSL_TAKE_PICT_DONE;
            cam_frame_idx_range_t range;

            sensor_bracket_params_t * flash_bracket_params= NULL;
            sensor_bracket_ctrl_t * flash_ctrl = NULL;

            flash_bracket_params =
              &(bundle_info.s_bundle->flash_bracket_params);
            flash_ctrl = &(flash_bracket_params->ctrl);
            SLOW("current frame %d", bundle_info.s_bundle->last_idx);

            if (flash_bracket_params->flash_bracket.enable == TRUE) {
              /* @num_skip_exp_linecount : fixed delay for valid exp
               *                           and linecount
               * @num_skip_led_up : fixed delay for LED to take effect
               *                    copy from original code: 1 + 3
               */
              const uint8_t num_skip_exp_linecount = 3;
              const uint8_t num_skip_led_up = 4;

              flash_bracket_params->flash_bracket.snapshot_identity =
                sensor_util_find_stream_identity_by_stream_type_and_session(
                  module, CAM_STREAM_TYPE_SNAPSHOT,
                  SENSOR_SESSIONID(event->identity));
              flash_bracket_params->flash_bracket.preview_identity =
                event->identity;

              rc = sensor_util_set_frame_skip_to_isp(module,
                flash_bracket_params->flash_bracket.preview_identity, SKIP_ALL);
              if (!rc < 0) {
                SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
              }
              rc = sensor_util_set_frame_skip_to_isp(module,
                flash_bracket_params->flash_bracket.snapshot_identity,
                SKIP_ALL);
              if (!rc) {
                SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
              }

              /* Fill metadta msg */
              range.min_frame_idx = bundle_info.s_bundle->last_idx +
                num_skip_led_up;
              range.max_frame_idx = range.min_frame_idx +
                num_skip_exp_linecount;
              bus_msg.msg = &range;
              /* save frame info to local structure */
              flash_ctrl->min_frame_idx = range.min_frame_idx;
              flash_ctrl->max_frame_idx = range.max_frame_idx;

              SLOW(" Posting flash_bracket msg to bus, sof %d, min:%d max:%d",
                bundle_info.s_bundle->last_idx, range.min_frame_idx,
                range.max_frame_idx);
              if(mct_module_post_bus_msg(module, &bus_msg) != TRUE) {
                SERR("Failed to send flash_bracket msg");
              }

              /* enable internally */
              SERR("Marking Flash Bracket Enable!!");
              flash_bracket_params->ctrl.enable = TRUE;
            } else {
              /* regular Flash Burst range */
              range.min_frame_idx = bundle_info.s_bundle->last_idx+1+3;
              range.max_frame_idx = bundle_info.s_bundle->last_idx+1+2+10;
              bus_msg.msg = &range;
              if(mct_module_post_bus_msg(module,&bus_msg)!=TRUE) {
                SERR("error posting led frame range msg");
              }
            }
          } else {
            SHIGH("LED Trigger disabled.");
          }
          SLOW("led set to high");

          sensor_ctrl_t *ctrl
            = (sensor_ctrl_t *)module_sensor_params->sub_module_private;
          ctrl->s_data->apply_without_sync = TRUE;
          rc = module_sensor_params->func_tbl.process(
            module_sensor_params->sub_module_private,
            SENSOR_SET_AEC_UPDATE, &aec_update);
          if (1) {
            ret = sensor_util_set_digital_gain_to_isp(module,
              bundle_info.s_bundle, event->identity);
              if (ret == FALSE) {
                SERR("failed");
                /* Continue START STREAM since this is not FATAL error */
                ret = TRUE;
              }
            }

          /* set exposure settings for zsl */
          rc = module_sensor_params->func_tbl.process(
                  module_sensor_params->sub_module_private,
                  SENSOR_SET_AEC_ZSL_SETTINGS, (void*)(&(stats_get.aec_get)));
          if (rc < 0) {
            SERR("failed");
            return FALSE;
          }

       }

      /* Enable focus bracketing upon zsl snapshot */
      sensor_af_bracket_t *af_bracket_params =
         &(bundle_info.s_bundle->af_bracket_params);
      if (af_bracket_params->enable == TRUE) {
        rc = module_sensor_start_af_bracketing(module,
               bundle_info.s_bundle, event);
        if (!rc) {
           SERR("can't start af bracketing");
           return FALSE;
        }
      }
      sensor_af_bracket_t *mtf_bracket_params =
         &(bundle_info.s_bundle->mtf_bracket_params);
      if (mtf_bracket_params->enable == TRUE) {
        rc = module_sensor_start_mtf_bracketing(module,
               bundle_info.s_bundle, event);
        if (!rc) {
           SERR("can't start MTF bracketing");
           return FALSE;
        }
      }
    break;
  }
  case MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT:{
     SERR("stop ZSL capture, led off\n");
        module_sensor_params_t        *led_module_params = NULL;
        led_module_params =
          bundle_info.s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
        if (led_module_params->func_tbl.process != NULL) {
          rc = led_module_params->func_tbl.process(
            led_module_params->sub_module_private,
            LED_FLASH_SET_OFF, NULL);
          if (rc < 0) {
            SERR("failed: LED_FLASH_SET_OFF");
          } else {
            bundle_info.s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
            sensor_util_post_led_state_msg(module, bundle_info.s_bundle,
              event->identity);
          }
        }
        SHIGH("%s, post-flash: OFF", __func__);
      ret = sensor_util_post_event_on_src_port(module, event);
    break;
  }
  case MCT_EVENT_CONTROL_DO_AF:{
       sensor_output_format_t output_format;
        module_sensor_params_t *module_sensor_params =
        bundle_info.s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
        module_sensor_params->func_tbl.process(
              module_sensor_params->sub_module_private,
              SENSOR_GET_SENSOR_FORMAT, &output_format);
        if(output_format == SENSOR_YCBCR) {
          sensor_info_t sensor_info;
          sensor_info.module = module;
          sensor_info.session_id =
            bundle_info.s_bundle->sensor_info->session_id;
          module_sensor_params->func_tbl.process(
                module_sensor_params->sub_module_private,
                SENSOR_SET_AUTOFOCUS, &sensor_info);
          SHIGH("%s: Setting Auto Focus", __func__);
        }
        else {
          ret = sensor_util_post_event_on_src_port(module, event);
        }
    break;
  }
  case MCT_EVENT_CONTROL_CANCEL_AF:{
       sensor_output_format_t output_format;
        module_sensor_params_t *module_sensor_params =
        bundle_info.s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
        module_sensor_params->func_tbl.process(
              module_sensor_params->sub_module_private,
              SENSOR_GET_SENSOR_FORMAT, &output_format);
        if(output_format == SENSOR_YCBCR) {
          module_sensor_params->func_tbl.process(
                module_sensor_params->sub_module_private,
                SENSOR_CANCEL_AUTOFOCUS, module);
        SHIGH("%s: Cancelling Auto Focus", __func__);
        sensor_cancel_autofocus_loop();
        }
        else {
          ret = sensor_util_post_event_on_src_port(module, event);
        }
    break;
  }
  default:
    /* Call send_event to propogate event to next module*/
    ret = sensor_util_post_event_on_src_port(module, event);
    if (ret == FALSE) {
      SERR("failed");
      return FALSE;
    }
    break;
  }
  return ret;
}

/** module_sensor_set_mod: set mod function for sensor module
 *
 *  This function handles set mod events sent by mct **/

static void module_sensor_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  SLOW("Enter, module_type=%d", module_type);
  mct_module_add_type(module, module_type, identity);
  if (module_type == MCT_MODULE_FLAG_SOURCE) {
    mct_module_set_process_event_func(module,
      module_sensor_module_process_event);
  }
  return;
}

/** module_sensor_query_mod: query mod function for sensor
 *  module
 *
 *  @query_buf: pointer to module_sensor_query_caps_t struct
 *  @session: session id
 *  @s_module: mct module pointer for sensor
 *
 *  Return: 0 for success and negative error for failure
 *
 *  This function handles query module events to return
 *  information requested by mct any stream is created **/

static boolean module_sensor_query_mod(mct_module_t *module,
  void *buf, unsigned int sessionid)
{
  int32_t idx = 0, rc = SENSOR_SUCCESS;
  mct_pipeline_sensor_cap_t *sensor_cap = NULL;
  module_sensor_ctrl_t *module_ctrl = NULL;
  module_sensor_bundle_info_t *s_bundle = NULL;
  module_sensor_params_t *module_sensor_params = NULL;
  mct_list_t *s_list = NULL;
  sensor_output_format_t output_format;

  mct_pipeline_cap_t *query_buf = (mct_pipeline_cap_t *)buf;
  if (!query_buf || !module) {
    SERR("failed query_buf %p s_module %p",
      query_buf, module);
    return FALSE;
  }

  sensor_cap = &query_buf->sensor_cap;
  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  if (!module_ctrl) {
    return FALSE;
  }

  s_list = mct_list_find_custom(module_ctrl->sensor_bundle, &sessionid,
    sensor_util_find_bundle);
  if (!s_list) {
    SERR("session_id doesn't match idx");
    return FALSE;
  }
  s_bundle = (module_sensor_bundle_info_t *)s_list->data;
  if (!s_bundle) {
    return FALSE;
  }
  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  if (!module_sensor_params) {
    return FALSE;
  }
  SLOW("sensor name %s",
    s_bundle->sensor_info->sensor_name);

  rc = module_sensor_params->func_tbl.process(s_bundle->sensor_lib_params,
     SENSOR_GET_CAPABILITIES, sensor_cap);
  if (rc < 0) {
    SERR("failed rc %d", rc);
    return FALSE;
  }

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_LED_FLASH] != -1 ||
    s_bundle->sensor_info->subdev_id[SUB_MODULE_STROBE_FLASH] != -1) {
    sensor_cap->is_flash_supported = TRUE;
  } else {
    SERR("led flash is not supported for this sensor.");
    sensor_cap->is_flash_supported = FALSE;
  }

  sensor_cap->af_supported =
    (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) ?
                                                      TRUE : FALSE;
  /* Fill sensor init params */
  rc = module_sensor_params->func_tbl.process(
          module_sensor_params->sub_module_private,
          SENSOR_GET_SENSOR_FORMAT, &output_format);
  if (rc < 0) {
    SERR("failed rc %d", rc);
    return FALSE;
    }
  if (output_format == SENSOR_YCBCR) {
    sensor_cap->sensor_format = FORMAT_YCBCR;
   }
   else if (output_format == SENSOR_BAYER) {
    sensor_cap->sensor_format = FORMAT_BAYER;
   }

  /* Store the information from sensor capabilities in sensor bundle */
  s_bundle->frame_ctrl.max_pipeline_frame_delay =
    sensor_cap->max_pipeline_frame_delay;
  return TRUE;
}

static boolean module_sensor_delete_port(void *data, void *user_data)
{
  mct_port_t *s_port = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;
  if (!s_port || !module) {
    SERR("failed s_port %p module %p",
      s_port, module);
    return TRUE;
  }
  free(s_port->caps.u.data);
  return TRUE;
}

/** module_sensor_free_bundle: free bundle function for
 *  sensor module
 *
 *  @data: sensor bundle pointer
 *  @user_data: NULL
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function is called for each sensor when module close is
 *  called. It releases all resources and memory held by each
 *  sensor */

static boolean module_sensor_free_bundle(void *data, void *user_data)
{
  uint32_t i = 0;
  module_sensor_bundle_info_t *s_bundle = (module_sensor_bundle_info_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;
  pthread_mutex_t *lens_move_done_sig;
  pthread_mutex_t *lens_move_done_sig_mtf;

  if (!s_bundle || !module) {
    SERR("failed s_bundle %p, module %p",
      s_bundle, module);
    return TRUE;
  }
  mct_list_free_all(MCT_MODULE_SRCPORTS(module), module_sensor_delete_port);
  if (s_bundle->sensor_lib_params) {
    free(s_bundle->sensor_lib_params);
  }
  for (i = 0; i < SUB_MODULE_MAX; i++) {
    free(s_bundle->module_sensor_params[i]);
  }

  lens_move_done_sig =
    &s_bundle->af_bracket_params.lens_move_done_sig;
  pthread_mutex_destroy(lens_move_done_sig);

  lens_move_done_sig_mtf =
     &s_bundle->mtf_bracket_params.lens_move_done_sig;
  pthread_mutex_destroy(lens_move_done_sig_mtf);

  free(s_bundle->sensor_info);
  free(s_bundle);
  return TRUE;
}

/** module_sensor_free_mod: free module function for sensor
 *  module
 *
 *  @module: mct module pointer for sensor
 *
 *  This function releases all resources held by sensor mct
 *  module */

void module_sensor_deinit(mct_module_t *module)
{
  int32_t                      rc = SENSOR_SUCCESS;
  int32_t                      idx = -1, i = 0;
  module_sensor_ctrl_t        *module_ctrl = NULL;
  module_sensor_bundle_info_t *s_bundle = NULL;

  if (!module) {
    SERR("module NULL");
    return;
  }

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;

  mct_list_traverse(module_ctrl->sensor_bundle, module_sensor_free_bundle,
    module);

  free(module);

  return;
}

/** module_sensor_find_sensor_subdev: find sensor subdevs
 *
 *  @module_ctrl: sensor ctrl pointer
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function finds all sensor subdevs, creates sensor
 *  bundle for each sensor subdev and gets init params and
 *  subdev info from the subdev **/

static void module_sensor_find_sensor_subdev(
  module_sensor_ctrl_t *module_ctrl)
{
  struct media_device_info mdev_info;
  int32_t num_media_devices = 0;
  char dev_name[32];
  char subdev_name[32];
  int32_t rc = 0, dev_fd = 0, sd_fd = 0;
  module_sensor_bundle_info_t *sensor_bundle = NULL;
  struct sensorb_cfg_data cfg;
  uint32_t i = 0;
  pthread_mutex_t *lens_move_done_sig;
  pthread_mutex_t *lens_move_done_sig_mtf;
  int32_t internal_retry = 400 ; /*100; in stress test we got once issue due to
      timing mismatch. So, increased retry count to 400 . It will not effect.
      We can take this chnage */
  while (1) {
    int32_t num_entities = 1;
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd >= MAX_FD_PER_PROCESS) {
      dump_list_of_daemon_fd();
      dev_fd = -1;
      break;
    }
    if (dev_fd < 0) {
      SLOW("Done enumerating media devices");
      break;
    }
    num_media_devices++;
    rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0) {
      SLOW("Done enumerating media devices");
      close(dev_fd);
      break;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model) != 0)) {
      close(dev_fd);
      continue;
    }

    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      SLOW("entity id %d", entity.id);
      rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        SLOW("Done enumerating media entities");
        rc = 0;
        break;
      }
      SLOW("entity name %s type %d group id %d",
        entity.name, entity.type, entity.group_id);
      if (entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
          entity.group_id == MSM_CAMERA_SUBDEV_SENSOR) {
        snprintf(subdev_name, sizeof(dev_name), "/dev/%s", entity.name);
        while (internal_retry) {
          sd_fd = open(subdev_name, O_RDWR);
          if (sd_fd >= MAX_FD_PER_PROCESS) {
            dump_list_of_daemon_fd();
            sd_fd = -1;
            continue;
          }
          if ((sd_fd >= 0) || (internal_retry == 0))
            break;
          else
            SERR("Open subdev failed: %d : %s\n", sd_fd, strerror(errno));
          internal_retry--;
          usleep(5000);
        }
        if ((internal_retry == 0) && (sd_fd < 0)) {
          SERR("Open subdev failed");
          continue;
        }
        sensor_bundle = malloc(sizeof(module_sensor_bundle_info_t));
        if (!sensor_bundle) {
          SERR("failed");
          close(sd_fd);
          continue;
        }
        memset(sensor_bundle, 0, sizeof(module_sensor_bundle_info_t));

        cfg.cfgtype = CFG_GET_SENSOR_INFO;
        rc = ioctl(sd_fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
        if (rc < 0) {
          SERR("failed rc %d", rc);
          free(sensor_bundle);
          close(sd_fd);
          continue;
        }

        sensor_bundle->sensor_info = malloc(sizeof(struct msm_sensor_info_t));
        if (!sensor_bundle->sensor_info) {
          free(sensor_bundle);
          close(sd_fd);
          continue;
        }
        memset(sensor_bundle->sensor_info, 0, sizeof(struct msm_sensor_info_t));

        /* Fill sensor info structure in sensor bundle */
        *sensor_bundle->sensor_info = cfg.cfg.sensor_info;

        SLOW("sensor name %s session %d",
          sensor_bundle->sensor_info->sensor_name,
          sensor_bundle->sensor_info->session_id);

        /* Initialize chroamtix subdevice id */
        sensor_bundle->sensor_info->subdev_id[SUB_MODULE_SENSOR] =
          sensor_bundle->sensor_info->session_id;
        sensor_bundle->sensor_info->subdev_id[SUB_MODULE_CHROMATIX] = 0;
        for (i = 0; i < SUB_MODULE_MAX; i++) {
          SLOW("subdev_id[%d] %d", i,
            sensor_bundle->sensor_info->subdev_id[i]);
        }
        /* Copy sensor subdev name to open and use during camera session */
        memcpy(sensor_bundle->sensor_sd_name[SUB_MODULE_SENSOR], entity.name,
          MAX_SUBDEV_SIZE);

        lens_move_done_sig =
          &sensor_bundle->af_bracket_params.lens_move_done_sig;
        lens_move_done_sig_mtf =
          &sensor_bundle->mtf_bracket_params.lens_move_done_sig;
        if (pthread_mutex_init(lens_move_done_sig, NULL)
            || pthread_mutex_init(lens_move_done_sig_mtf, NULL)) {
          SERR("failed to init mutex errno %d", errno);
          free(sensor_bundle->sensor_info);
          free(sensor_bundle);
          close(sd_fd);
          continue;
        }

        sensor_bundle->hal_frame_skip_pattern = NO_SKIP;

        SLOW("sensor sd name %s",
          sensor_bundle->sensor_sd_name[SUB_MODULE_SENSOR]);

        /* Add sensor_bundle to module_ctrl list */
        module_ctrl->sensor_bundle =
          mct_list_append(module_ctrl->sensor_bundle, sensor_bundle, NULL,
          NULL);

        /* Increment sensor bundle size */
        module_ctrl->size++;

        close(sd_fd);
      }
    }
    close(dev_fd);
  }
  return;
}

/** module_sensor_set_sub_module_id: set subdev id for sensor sub
 *  modules
 *
 *  @data: module_sensor_bundle_info_t pointer
 *  @user_data: module_sensor_match_id_params_t pointer
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function matches subdev id against subdev id present in
 *  sensor info, if both matches, copy the subdev name for this
 *  subdev which will be used later to open and communicate with
 *  kernel driver **/

static boolean module_sensor_set_sub_module_id(void *data, void *user_data)
{
  module_sensor_bundle_info_t *sensor_bundle =
    (module_sensor_bundle_info_t *)data;
  module_sensor_match_id_params_t *match_id_params =
    (module_sensor_match_id_params_t *)user_data;
  if (!sensor_bundle || !match_id_params) {
    SERR("failed data1 %p data2 %p", data,
      user_data);
    return FALSE;
  }
  SLOW("sub module %d id %d subdev name %s",
    match_id_params->sub_module, match_id_params->subdev_id,
    match_id_params->subdev_name);
  SLOW("sensor_info->subdev_id %d ?= match_id_params->subdev_id %d",
    sensor_bundle->sensor_info->subdev_id[match_id_params->sub_module],
    match_id_params->subdev_id);

  if (sensor_bundle->sensor_info->subdev_id[match_id_params->sub_module] ==
    match_id_params->subdev_id) {
    memcpy(sensor_bundle->sensor_sd_name[match_id_params->sub_module],
      match_id_params->subdev_name, MAX_SUBDEV_SIZE);
    SLOW("match found sub module %d session id %d subdev name %s",
      match_id_params->sub_module,
      sensor_bundle->sensor_info->session_id, match_id_params->subdev_name);
  }
  return TRUE;
}


static boolean module_sensor_init_eeprom(void *data, void *user_data) {
    int32_t rc = 0;
    module_sensor_bundle_info_t *s_bundle = (module_sensor_bundle_info_t *)data;
    mct_module_t                *s_module = (mct_module_t *)user_data;
    sensor_func_tbl_t func_tbl;
    module_sensor_params_t *module_eeprom_params = NULL;

    SLOW("Enter");
    if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] == -1) {
      SERR("Exit subdev_id[SUB_MODULE_EEPROM] == -1");
      return TRUE;
    }
    eeprom_sub_module_init(&func_tbl);
    s_bundle->eeprom_data = (sensor_eeprom_data_t *)
       malloc(sizeof(sensor_eeprom_data_t));
    if (!s_bundle->eeprom_data) {
        SERR("failed to allocate memory");
        return TRUE;
    }
    rc = func_tbl.process(s_bundle->eeprom_data, EEPROM_OPEN_FD,
      s_bundle->sensor_sd_name[SUB_MODULE_EEPROM]);
    if (rc < 0) {
        SERR("Failed EEPROM_OPEN_FD");
        return TRUE;
    }
    rc = func_tbl.process(s_bundle->eeprom_data, EEPROM_READ_DATA,
      NULL);
    if (rc < 0) {
        SERR("Failed EEPROM_READ_DATA");
        return TRUE;
    }
    rc = func_tbl.process(s_bundle->eeprom_data, EEPROM_CLOSE_FD,
      NULL);
    if (rc < 0) {
        SERR("Failed EEPROM_CLOSE_FD");
        return TRUE;
    }

  SLOW("Exit");
  return TRUE;
}

/** module_sensor_find_other_subdev: find subdevs other than
 *  sensor
 *
 *  @module_ctrl: module_sensor_ctrl_t pointer
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function finds all subdevs other than sensor and fills
 *  the subdev name in sensor bundle for those sensor whose
 *  subdev id matches with current subdev **/

static boolean module_sensor_find_other_subdev(
  module_sensor_ctrl_t *module_ctrl)
{
  struct media_device_info mdev_info;
  int32_t num_media_devices = 0;
  char dev_name[32];
  uint32_t subdev_id;
  char subdev_name[32];
  int32_t rc = 0, dev_fd = 0, sd_fd = 0;
  uint8_t session_id = 0;
  module_sensor_match_id_params_t match_id_params;
  if (!module_ctrl) {
    SLOW("failed");
    return FALSE;
  }
  while (1) {
    int32_t num_entities = 1;
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd >= MAX_FD_PER_PROCESS) {
      dump_list_of_daemon_fd();
      dev_fd = -1;
      break;
    }
    SLOW("Opened Device %s",dev_name);
    if (dev_fd < 0) {
      SLOW("Done enumerating media devices");
      break;
    }
    num_media_devices++;
    rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0) {
      SERR("Error: ioctl media_dev failed: %s", strerror(errno));
      close(dev_fd);
      break;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model) != 0)) {
      close(dev_fd);
      continue;
    }

    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      SLOW("entity id %d", entity.id);
      rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        SLOW("Done enumerating media entities");
        rc = 0;
        break;
      }
      SLOW("entity name %s type %d group id %d",
        entity.name, entity.type, entity.group_id);
      if (entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
        (entity.group_id == MSM_CAMERA_SUBDEV_ACTUATOR ||
        entity.group_id == MSM_CAMERA_SUBDEV_EEPROM ||
        entity.group_id == MSM_CAMERA_SUBDEV_LED_FLASH ||
        entity.group_id == MSM_CAMERA_SUBDEV_STROBE_FLASH ||
        entity.group_id == MSM_CAMERA_SUBDEV_CSIPHY ||
        entity.group_id == MSM_CAMERA_SUBDEV_CSID)) {
        snprintf(subdev_name, sizeof(subdev_name), "/dev/%s", entity.name);
        sd_fd = open(subdev_name, O_RDWR);
        if (dev_fd >= MAX_FD_PER_PROCESS) {
          dump_list_of_daemon_fd();
          dev_fd = -1;
          continue;
        }
        if (sd_fd < 0) {
          SLOW("Open subdev failed");
          continue;
        }
        /* Read subdev index */
        rc = ioctl(sd_fd, VIDIOC_MSM_SENSOR_GET_SUBDEV_ID, &subdev_id);
        if (rc < 0) {
          SERR("failed rc %d", rc);
          close(sd_fd);
          continue;
        }
        SLOW("subdev_name %s subdev id %d", subdev_name, subdev_id);
        /* TODO: read id and fill in sensor_bundle.entity.actuator_name */
        switch (entity.group_id) {
           case MSM_CAMERA_SUBDEV_ACTUATOR:
            match_id_params.sub_module = SUB_MODULE_ACTUATOR;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_ACTUATOR subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_EEPROM:
            match_id_params.sub_module = SUB_MODULE_EEPROM;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_EEPROM subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_LED_FLASH:
            match_id_params.sub_module = SUB_MODULE_LED_FLASH;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_LED_FLASH subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_STROBE_FLASH:
            match_id_params.sub_module = SUB_MODULE_STROBE_FLASH;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_STROBE_FLASH subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_CSIPHY:
            match_id_params.sub_module = SUB_MODULE_CSIPHY;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_CSIPHY subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_CSID:
            match_id_params.sub_module = SUB_MODULE_CSID;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_CSID subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          default:
            SLOW("ERROR Default group_id %d subdev_name %s subdev_id %d",
              entity.group_id, entity.name, subdev_id);
            break;
        }
        close(sd_fd);
      }
    }
    close(dev_fd);
  }
  return TRUE;
};
/** module_sensor_init: sensor module init
 *
 *  Return: mct_module_t pointer corresponding to sensor
 *
 *  This function creates mct_module_t for sensor module,
 *  creates port, fills capabilities and add it to the sensor
 *  module **/

mct_module_t *module_sensor_init(const char *name)
{
  boolean                      ret = TRUE;
  mct_module_t                *s_module = NULL;
  module_sensor_ctrl_t        *module_ctrl = NULL;

  SHIGH("Enter");

  /* Create MCT module for sensor */
  s_module = mct_module_create(name);
  if (!s_module) {
    SERR("failed");
    return NULL;
  }

  /* Fill function table in MCT module */
  s_module->set_mod = module_sensor_set_mod;
  s_module->query_mod = module_sensor_query_mod;
  s_module->start_session = module_sensor_start_session;
  s_module->stop_session = module_sensor_stop_session;

  /* Create sensor module control structure that consists of bundle
     information */
  module_ctrl = malloc(sizeof(module_sensor_ctrl_t));
  if (!module_ctrl) {
    SERR("failed");
    goto ERROR1;
  }
  memset(module_ctrl, 0, sizeof(module_sensor_ctrl_t));

  s_module->module_private = (void *)module_ctrl;

  /* sensor module doesn't have sink port */
  s_module->numsinkports = 0;

  /* module_sensor_probe_sensors */
  ret = sensor_init_probe(module_ctrl);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR1;
  }

  /* Fill all detected sensors */
  module_sensor_find_sensor_subdev(module_ctrl);

  /* find all the actuator, etc with sensor */
  ret = module_sensor_find_other_subdev(module_ctrl);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR1;
  }

  /* Init sensor modules */
  ret = mct_list_traverse(module_ctrl->sensor_bundle, module_sensors_subinit,
    NULL);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR1;
  }

  /* Create ports based on CID info */
  ret = mct_list_traverse(module_ctrl->sensor_bundle, port_sensor_create,
    s_module);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR1;
  }

  /* intiialize the eeprom */
  ret = mct_list_traverse(module_ctrl->sensor_bundle, module_sensor_init_eeprom,
    s_module);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR1;
  }

  SLOW("Exit");
  return s_module;

  SERR("failed");
ERROR1:
  mct_module_destroy(s_module);
  return NULL;
}
