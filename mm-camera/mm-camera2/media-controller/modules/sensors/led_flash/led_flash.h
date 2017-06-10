/*============================================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __LED_FLASH_H__
#define __LED_FLASH_H__

#include "sensor_common.h"


// Red Eye Reduction (RER) Timing Limitations
#define PREFLASH_CYCLES_MIN       (1)       // [times]
#define PREFLASH_CYCLES_MAX       (200)     // [times]
#define LED_ON_MS_MIN             (1)       // [ms]
#define LED_ON_MS_MAX             (200)     // [ms]
#define LED_OFF_MS_MIN            (1)       // [ms]
#define LED_OFF_MS_MAX            (200)     // [ms]
#define RER_DURATION_MS_MIN       (10)      // [ms]
#define RER_DURATION_MS_MAX       (2000)    // [ms]
#define RER_PUPIL_CONTRACT_TIME   (15)     // [ms]

typedef enum {
  RER_START = 0,
  RER_WAIT_PUPIL_CONTRACT,
  RER_DONE,
} rer_status_t;

typedef struct {
  red_eye_reduction_type  *cfg;
  struct timeval          last_rer_flash_ts;
  rer_status_t             status;
} rer_cfg_t;

typedef struct {
  int            fd;
  rer_cfg_t     *rer;
  int32_t flash_max_duration[MAX_LED_TRIGGERS];
  int32_t flash_max_current[MAX_LED_TRIGGERS];
  awb_dual_led_settings_t   *dual_led_setting;
} sensor_led_flash_data_t;

int32_t led_flash_rer_get_current(
  awb_update_t            *awb_update,
  awb_dual_led_settings_t *dual_led_setting);

int32_t led_flash_rer_get_chromatix(
  rer_cfg_t               *rer,
  red_eye_reduction_type  *rer_chromatix);

int32_t led_flash_rer_sequence_process(
  rer_cfg_t               *rer,
  module_sensor_params_t  *led_module_params);

int32_t led_flash_rer_wait_pupil_contract(
  rer_cfg_t              *rer,
  module_sensor_params_t *led_module_params);

int32_t led_flash_rer_set_parm(
  rer_cfg_t               *rer,
  int32_t                  mode);

#endif
