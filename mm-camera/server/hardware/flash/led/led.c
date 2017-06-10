/*============================================================================
   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "camera_dbg.h"
#include "led.h"

flash_led_ctrl_t *led_ctrl_p;

void led_ctrl_init(flash_led_ctrl_t *led_ctrl, int camfd)
{
  led_ctrl->camfd = camfd;
  CDBG("sensor_led_ctrl_init: camfd = %d\n", camfd);
  led_ctrl_p = led_ctrl;
  set_led_state(MSM_CAMERA_LED_INIT);
  led_ctrl->led_data.led_state = MSM_CAMERA_LED_OFF;
  led_ctrl->led_data.led_mode = LED_MODE_OFF;
}

void led_ctrl_release(void)
{
  set_led_state(MSM_CAMERA_LED_RELEASE);
}

msm_camera_led_state_t get_led_state()
{
  return led_ctrl_p->led_data.led_state;
}

int8_t set_led_state(msm_camera_led_state_t new_led_state)
{
  int8_t rc = TRUE;
  struct flash_ctrl_data  led_ctrl;

  CDBG("set_led_state: %d -> %d\n", led_ctrl_p->led_data.led_state,
    new_led_state);
  if (led_ctrl_p->led_data.led_state != new_led_state) {

    /* Set led state */
    CDBG("set_led_state: %d\n", new_led_state);
    led_ctrl.flashtype = LED_FLASH;
    led_ctrl.ctrl_data.led_state = new_led_state;
    rc = ioctl(led_ctrl_p->camfd, MSM_CAM_IOCTL_FLASH_CTRL,
      &led_ctrl);
    CDBG("set_led_state return %d\n", rc);

    if (rc>=0) {
      led_ctrl_p->led_data.led_state = new_led_state;
    }
  }
  return rc;
}

led_mode_t get_led_mode()
{
  return led_ctrl_p->led_data.led_mode;
}

int8_t set_led_mode(led_mode_t led_mode)
{
  int8_t rc = TRUE;
  CDBG("Set led_ctrl_p->led_mode = %d\n", led_ctrl_p->led_data.led_mode);
  if (led_ctrl_p != NULL && led_ctrl_p->led_data.led_mode != led_mode) {
    switch (led_mode) {
      case LED_MODE_AUTO:
      case LED_MODE_OFF:
      case LED_MODE_ON:
      /*LED ON only means that it'll be fired at snapshot regardless the light
      condition, and doesn't necessarily mean LED shall be on during preview*/
        rc = set_led_state(MSM_CAMERA_LED_OFF);
        break;

      case LED_MODE_TORCH:
        rc = set_led_state(MSM_CAMERA_LED_LOW);
        break;

      default:
        rc = FALSE;
        break;
    }

    led_ctrl_p->led_data.led_mode = led_mode;
    CDBG("led_ctrl_p->led_mode = %d\n", led_ctrl_p->led_data.led_mode);
  }
  return rc;
}

void update_led_state(msm_camera_led_state_t LED_state)
{
  set_led_state(LED_state);
}
