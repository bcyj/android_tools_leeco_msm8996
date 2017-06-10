/* led_flash.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "led_flash.h"
#include "sensor_common.h"
#include "server_debug.h"

/** led_flash_open:
 *    @led_flash_ctrl: address of pointer to
 *                   sensor_led_flash_data_t struct
 *    @subdev_name: LED flash subdev name
 *
 * 1) Allocates memory for LED flash control structure
 * 2) Opens LED flash subdev node
 * 3) Initialize LED hardware by passing control to kernel
 * driver
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t led_flash_open(void **led_flash_ctrl, const char *subdev_name)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_led_flash_data_t *ctrl = NULL;
  struct msm_camera_led_cfg_t cfg;
  char subdev_string[32];

  SLOW("Enter");
  if (!led_flash_ctrl || !subdev_name) {
    SERR("failed sctrl %p subdev name %p",
      led_flash_ctrl, subdev_name);
    return SENSOR_ERROR_INVAL;
  }
  ctrl = malloc(sizeof(sensor_led_flash_data_t));
  if (!ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  memset(ctrl, 0, sizeof(sensor_led_flash_data_t));

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s", subdev_name);
  SLOW("sd name %s", subdev_string);
  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if ((ctrl->fd) >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    ctrl->fd = -1;
    goto ERROR1;
  }
  if (ctrl->fd < 0) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR1;
  }

  ctrl->rer = malloc(sizeof(rer_cfg_t));
  if (!ctrl->rer) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR1;
  }
  memset(ctrl->rer, 0, sizeof(rer_cfg_t));

  ctrl->rer->cfg = malloc(sizeof(red_eye_reduction_type));
  if (!ctrl->rer->cfg) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR2;
  }
  memset(ctrl->rer->cfg, 0, sizeof(red_eye_reduction_type));

  ctrl->dual_led_setting = malloc(sizeof(awb_dual_led_settings_t));
  if (!ctrl->dual_led_setting) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR3;
  }
  memset(ctrl->dual_led_setting, 0, sizeof(awb_dual_led_settings_t));
  memset(ctrl->flash_max_duration, 0xFF, sizeof(ctrl->flash_max_duration));

  cfg.cfgtype = MSM_CAMERA_LED_INIT;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_FLASH_LED_DATA_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_FLASH_LED_DATA_CFG failed %s", strerror(errno));
    goto ERROR4;
  }

  *led_flash_ctrl = (void *)ctrl;
  SLOW("Exit");
  return rc;

ERROR4:
  free(ctrl->dual_led_setting);
ERROR3:
  free(ctrl->rer->cfg);
ERROR2:
  free(ctrl->rer);
ERROR1:
  free(ctrl);
  return rc;
}

/** led_flash_init:
 *    @led_flash_ctrl: LED flash control handle
 *    @data: NULL
 *
 * Handled all LED flash trigger events and passes control to
 * kernel to configure LED hardware
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/
static int32_t led_flash_init(void *ptr)
{
  int32_t rc = SENSOR_SUCCESS;
  int32_t i = 0;

  sensor_led_flash_data_t *led_flash_ctrl = (sensor_led_flash_data_t*)ptr;
  struct msm_camera_led_cfg_t cfg;

  cfg.cfgtype = MSM_CAMERA_LED_INIT;
  rc = ioctl(led_flash_ctrl->fd, VIDIOC_MSM_FLASH_LED_DATA_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_FLASH_LED_DATA_CFG failed %s", strerror(errno));
    return SENSOR_FAILURE;
  }

  for (i = 0; i < MAX_LED_TRIGGERS; i++) {
    led_flash_ctrl->flash_max_current[i] =
      (int32_t)cfg.flash_current[i];
    led_flash_ctrl->flash_max_duration[i] =
      (int32_t)cfg.flash_duration[i];
    SLOW("i = %d flash_current = %d flash_duration = %d",
      i, led_flash_ctrl->flash_max_current[i],
      led_flash_ctrl->flash_max_duration[i]);
  }

  return SENSOR_SUCCESS;
}
/** led_flash_process:
 *    @led_flash_ctrl: LED flash control handle
 *    @event: configuration event type
 *    @data: NULL
 *
 * Handled all LED flash trigger events and passes control to
 * kernel to configure LED hardware
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t led_flash_process(void *led_flash_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t                     rc = SENSOR_SUCCESS;
  sensor_led_flash_data_t *ctrl = (sensor_led_flash_data_t *)led_flash_ctrl;
  rer_cfg_t                  *rer = NULL;
  module_sensor_params_t     *led_module_params = NULL;
  red_eye_reduction_type     *rer_chromatix = NULL;
  awb_dual_led_settings_t    *dual_led_setting = NULL;
  int32_t                  mode = 0;
  int32_t                     temp = 0;

  struct msm_camera_led_cfg_t cfg;

  if (!led_flash_ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  rer = ((sensor_led_flash_data_t *)led_flash_ctrl)->rer;
  dual_led_setting =
    ((sensor_led_flash_data_t *)led_flash_ctrl)->dual_led_setting;

  /* Set default current values = 0 */
  cfg.torch_current[0] = 0;
  cfg.torch_current[1] = 0;
  cfg.flash_current[0] = 0;
  cfg.flash_current[1] = 0;

  switch (event) {
  case LED_FLASH_GET_MAX_CURRENT: {
    int32_t ** flash_current = (int32_t **)data;
    *flash_current = &(ctrl->flash_max_current[0]);
  }
    break;
  case LED_FLASH_GET_MAX_DURATION: {
    int32_t ** flash_duration = (int32_t **)data;
    *flash_duration = &(ctrl->flash_max_duration[0]);
  }
    break;
  case LED_FLASH_GET_CURRENT: {
    awb_update_t *awb_update = (awb_update_t*) data;
    /* Get currents for dual LED */
    rc = led_flash_get_current(awb_update, dual_led_setting);
    return rc;
  }
  case LED_FLASH_GET_RER_CHROMATIX: {
    rer_chromatix = (red_eye_reduction_type *)data;
    // Get (RER) data from chromatix
    rc = led_flash_rer_get_chromatix(rer, rer_chromatix);
    return rc;
  }
  case LED_FLASH_SET_RER_PARAMS: {
    mode = *(int32_t *)data;
    rc = led_flash_rer_set_parm(rer, mode);
    return rc;
  }
  case LED_FLASH_SET_RER_PROCESS: {
    led_module_params = (module_sensor_params_t *)data;
    rc = led_flash_rer_sequence_process(rer, led_module_params);
    return rc;
  }
  case LED_FLASH_SET_OFF:
    cfg.cfgtype = MSM_CAMERA_LED_OFF;
    break;
  case LED_FLASH_SET_TORCH:
    /* Torch mode */
    cfg.cfgtype = MSM_CAMERA_LED_LOW;
    cfg.torch_current[0] = data ? *(uint32_t *)data : 0;
    cfg.torch_current[1] = data ? *(uint32_t *)data : 0;
    break;
  case LED_FLASH_SET_PRE_FLASH:
    /* Pre flash mode */
    cfg.cfgtype = MSM_CAMERA_LED_LOW;

    if (dual_led_setting) {
      cfg.torch_current[0] = dual_led_setting->led1_low_setting;
      cfg.torch_current[1] = dual_led_setting->led2_low_setting;
    } else {
      cfg.torch_current[0] = data ? *(uint32_t *)data : 0;
      cfg.torch_current[1] = data ? *(uint32_t *)data : 0;
    }
    break;
  case LED_FLASH_SET_RER_PULSE_FLASH:
    /* RER flash pulses */
    cfg.cfgtype = MSM_CAMERA_LED_HIGH;

    /* Use chromatix current if exist (set in RER_PROCESS)*/
    cfg.flash_current[0] = data ? *(uint32_t *)data : 0;
    cfg.flash_current[1] = data ? *(uint32_t *)data : 0;

    if (dual_led_setting) {
      temp = (dual_led_setting->led1_high_setting +
        dual_led_setting->led2_high_setting);

      if (temp > 0) {
        /* Update with Dual LED current */
        cfg.flash_current[0] = (cfg.flash_current[0] *
          dual_led_setting->led1_high_setting) / temp;
        cfg.flash_current[1] = (cfg.flash_current[1] *
          dual_led_setting->led2_high_setting) / temp;
      }
    }
    break;
  case LED_FLASH_SET_MAIN_FLASH:
    /* Main flash mode */
    cfg.cfgtype = MSM_CAMERA_LED_HIGH;

    if (dual_led_setting) {
      cfg.flash_current[0] = dual_led_setting->led1_high_setting;
      cfg.flash_current[1] = dual_led_setting->led2_high_setting;
    } else {
      cfg.flash_current[0] = data ? *(uint32_t *)data : 0;
      cfg.flash_current[1] = data ? *(uint32_t *)data : 0;
    }
    rc = led_flash_rer_wait_pupil_contract(rer, led_module_params);
    break;
  default:
    SERR("invalid event %d", event);
    return SENSOR_FAILURE;
  }
  rc = ioctl(ctrl->fd, VIDIOC_MSM_FLASH_LED_DATA_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_FLASH_LED_DATA_CFG failed %s", strerror(errno));
    rc = SENSOR_FAILURE;
  }
  return rc;
}

/** led_flash_close:
 *    @led_flash_ctrl: LED flash control handle
 *
 * 1) Release LED flash hardware
 * 2) Close fd
 * 3) Free LED flash control structure
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t led_flash_close(void *led_flash_ctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_led_flash_data_t *ctrl = (sensor_led_flash_data_t *)led_flash_ctrl;
  struct msm_camera_led_cfg_t cfg;

  SLOW("Enter");
  cfg.cfgtype = MSM_CAMERA_LED_RELEASE;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_FLASH_LED_DATA_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_FLASH_LED_DATA_CFG failed %s",
      strerror(errno));
  }

  /* close subdev */
  close(ctrl->fd);

  if (ctrl->rer != NULL) {
    if (ctrl->rer->cfg != NULL) {
      /* Free rer->cfg */
      free(ctrl->rer->cfg);
      ctrl->rer->cfg = NULL;
    }
    /* Free rer */
    free(ctrl->rer);
    ctrl->rer = NULL;
  }

  if (ctrl->dual_led_setting != NULL) {
    /* Free dual_led_setting */
    free(ctrl->dual_led_setting);
    ctrl->dual_led_setting = NULL;
  }

  free(ctrl);
  SLOW("Exit");
  return rc;
}

/** led_flash_sub_module_init:
 *    @func_tbl: pointer to sensor function table
 *
 * Initialize function table for LED flash to be used
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t led_flash_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  if (!func_tbl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  func_tbl->open = led_flash_open;
  func_tbl->process = led_flash_process;
  func_tbl->close = led_flash_close;
  return SENSOR_SUCCESS;
}

/** led_flash_get_current:
 *    @awb_update: Input parameter
 *    @dual_led_setting: Output parameter
 *
 * 1) Copy dual LED currents from awb_update to
 *    dual_led_setting
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t led_flash_get_current(
  awb_update_t *awb_update,
  awb_dual_led_settings_t *dual_led_setting)
{
  if (!awb_update || !dual_led_setting) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  memcpy(dual_led_setting, &(awb_update->dual_led_setting),
    sizeof(awb_dual_led_settings_t));

  return SENSOR_SUCCESS;
}

/** led_flash_rer_get_chromatix:
 *    @rer_cfg: Internal flash data for RER
 *    @rer_chromatix: RER data from Chromatix
 *
 * 1) Get Red eye reduction (RER) data from chromatix
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t led_flash_rer_get_chromatix(
  rer_cfg_t *rer,
  red_eye_reduction_type *rer_chromatix)
{
  int led_flash_enable;
  red_eye_reduction_type *rer_cfg = (rer_cfg_t *)rer->cfg;

  if ((rer_cfg == NULL) ||
      (rer_chromatix == NULL)) {
    // Check input
    SERR("Red Eye Reduction process Skip ->\n \
          rer_cfg           = 0x%08x \n \
          rer_chromatix     = 0x%08x \n",
          (unsigned int)rer_cfg,
          (unsigned int)rer_chromatix);
    return SENSOR_FAILURE;
  }

  // Save red_eye_reduction_led_flash_enable state
  led_flash_enable = rer_cfg->red_eye_reduction_led_flash_enable;
  memcpy(rer_cfg, rer_chromatix, sizeof(red_eye_reduction_type));
  // Restore red_eye_reduction_led_flash_enable state
  rer_cfg->red_eye_reduction_led_flash_enable = led_flash_enable;

  return SENSOR_SUCCESS;
}

/** input_check:
 *    @value: Input value
 *    @min: Minimum value
 *    @max: Maximum value
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

inline int input_check(float value, float min, float max)
{
  if(value < min)
    return SENSOR_FAILURE;
  else if(value > max)
    return SENSOR_FAILURE;
  else
    return SENSOR_SUCCESS;
}

/** led_flash_rer_sequence_process:
 *    @led_module_params: Led module parameters
 *    @rer_cfg: Internal flash data for RER
 *
 * 1) Execute Red eye reduction (RER) sequence
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t led_flash_rer_sequence_process(
  rer_cfg_t *rer,
  module_sensor_params_t *led_module_params)
{
  int32_t rc = SENSOR_SUCCESS;
  red_eye_reduction_type *rer_cfg = (rer_cfg_t *)rer->cfg;
  int led_flash_enable;
  int preflash_cycles;
  int LED_pulse_duration_ms;
  int interval_pulese_ms;
  int LED_current_mA;

  if ((led_module_params == NULL) ||
      (rer_cfg == NULL)) {
    // Check input
    SERR("Red Eye Reduction process Skip ->\n \
        led_module_params = 0x%08x \n \
        rer_cfg           = 0x%08x \n",
        (unsigned int)led_module_params,
        (unsigned int)rer_cfg
    );
    return SENSOR_FAILURE;
  }

  // Get chromatix RER data from rer_cfg
  led_flash_enable = rer_cfg->red_eye_reduction_led_flash_enable;
  preflash_cycles  = rer_cfg->number_of_preflash_cycles;
  LED_pulse_duration_ms = rer_cfg->preflash_LED_pulse_duration;
  interval_pulese_ms = rer_cfg->preflash_interval_between_pulese;
  LED_current_mA = rer_cfg->preflash_LED_current;

  rer->status = RER_START;

  if (led_flash_enable == 1) {
    // Red eye procedure is Enabled
    int rc = SENSOR_SUCCESS;
    int sequence_time = ((preflash_cycles
        * (LED_pulse_duration_ms + interval_pulese_ms))
        + RER_PUPIL_CONTRACT_TIME);

    // Check Red Eye Tuning parameters
    rc += input_check(preflash_cycles, PREFLASH_CYCLES_MIN, PREFLASH_CYCLES_MAX);
    rc += input_check(LED_pulse_duration_ms, LED_ON_MS_MIN, LED_ON_MS_MAX);
    rc += input_check(interval_pulese_ms, LED_OFF_MS_MIN, LED_OFF_MS_MAX);
    rc += input_check(sequence_time, RER_DURATION_MS_MIN, RER_DURATION_MS_MAX);

    if (rc < 0) {
      SERR("Error: RER parameters out of range \n");
      rer->status = RER_DONE;
      return SENSOR_FAILURE;
    }

    // RER procedure
    while (preflash_cycles) {
      led_module_params->func_tbl.process(
        led_module_params->sub_module_private,
        LED_FLASH_SET_RER_PULSE_FLASH , &LED_current_mA);

      usleep(LED_pulse_duration_ms*1000);

      led_module_params->func_tbl.process(
        led_module_params->sub_module_private,
        LED_FLASH_SET_OFF , NULL);

      if (preflash_cycles <= 1) {
        /* Last flash pulse */
        rer->status = RER_WAIT_PUPIL_CONTRACT;
        gettimeofday(&rer->last_rer_flash_ts, NULL);
        break;
      } else {
        /* Generate interval between the pulses */
        usleep(interval_pulese_ms*1000);
      }
      preflash_cycles--;
    }
  }

  if (rer->status == RER_START) {
    rer->status = RER_DONE;
  }

  return SENSOR_SUCCESS;
}


/** led_flash_rer_wait_pupil_contract:
 *    @rer_cfg: Internal flash data for RER
 *    @led_module_params: Led module parameters
 *
 * 1) Wait before to start the
 *    main flash after RER sequence
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t led_flash_rer_wait_pupil_contract(
  rer_cfg_t *rer,
  module_sensor_params_t *led_module_params)
{
  if (rer->status == RER_WAIT_PUPIL_CONTRACT) {
    /* Wait before to start the main flash after RER sequence */
    long int  delay;
    struct timeval ts_now, ts, ts_diff;

    ts = rer->last_rer_flash_ts;

    gettimeofday(&ts_now, NULL);
    timersub(&ts_now, &ts, &ts_diff);
    delay = (ts_diff.tv_sec * 1000000) + ts_diff.tv_usec;
    delay = (long int)(RER_PUPIL_CONTRACT_TIME * 1000) - delay;

    if (delay > 0) {
      /* Wait until Pupil contraction time RER_PUPIL_CONTRACT_TIME */
      if (delay <= (RER_PUPIL_CONTRACT_TIME * 1000)) {
        SLOW("Wait %ld us to reach RER_PUPIL_CONTRACT_TIME\n", delay);
        usleep(delay);
      } else {
        /* Error - delay can not be bigger than RER_PUPIL_CONTRACT_TIME */
        SERR("Error - RER delay out of range %d us\n", delay);
      }
    } else {
      /* Delay is bigger than the requested RER_PUPIL_CONTRACT_TIME */
      SERR("RER Over delay %ld us (total delay %ld us)\n",
        -(long int)delay,
        (long int)(RER_PUPIL_CONTRACT_TIME * 1000) - delay);
    }
    rer->status = RER_DONE;
  }
  return SENSOR_SUCCESS;
}

/** led_flash_rer_set_parm:
 *    @rer_cfg:
 *    @mode:
 *
 * 1) Execute Red eye reduction (RER) sequence
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t led_flash_rer_set_parm(
  rer_cfg_t *rer,
  int32_t mode)
{
  red_eye_reduction_type *rer_cfg = (rer_cfg_t *)rer->cfg;

  if (rer_cfg) {
    // Enable/Disable - Red Eye Reduction procedure (RER)
    rer_cfg->red_eye_reduction_led_flash_enable = mode;
    rer_cfg->red_eye_reduction_xenon_strobe_enable = mode;
  } else {
    return SENSOR_FAILURE;
  }
  return SENSOR_SUCCESS;
}

