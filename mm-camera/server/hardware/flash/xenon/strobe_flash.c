/**********************************************************************
* Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#include "camera_dbg.h"
#include "strobe_flash.h"
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

int8_t strobe_flash_device_init(flash_strobe_ctrl_t *strobe_flash_ctrl,
  int camfd)
{
  int8_t rc = 0;
  uint32_t flash_type;
  int32_t enabled = 0;
  struct flash_ctrl_data  flash_ctrl;

  #ifdef _ANDROID_
  char value[PROPERTY_VALUE_MAX];

  property_get("persist.camera.strobe", value, "0");
  enabled = atoi(value);
  if (!enabled)
	  return rc;
  #endif

  flash_ctrl.flashtype = STROBE_FLASH;
  flash_ctrl.ctrl_data.strobe_ctrl.type = STROBE_FLASH_CTRL_INIT;
  flash_ctrl.ctrl_data.strobe_ctrl.charge_en = STROBE_FLASH_CHARGE_DISABLE;

  strobe_flash_ctrl->camfd = camfd;
  strobe_flash_ctrl->strobe_data.strobe_ready = STROBE_FLASH_NOT_READY;
  strobe_flash_ctrl->strobe_flash_charge_en = STROBE_FLASH_CHARGE_DISABLE;
  flash_type = MSM_CAMERA_STROBE_FLASH_XENON;
  rc = ioctl(camfd, MSM_CAM_IOCTL_FLASH_CTRL, &flash_ctrl);
  if(rc < 0)
    strobe_flash_ctrl->strobe_data.strobe_state = STROBE_FLASH_FAIL;
  else
    strobe_flash_ctrl->strobe_data.strobe_state = STROBE_FLASH_INIT;

  CDBG("strobe_flash_xenon_init: camfd = %d, rc=%d\n", camfd, rc);
  return rc;
}

int32_t strobe_flash_device_charge(
  flash_strobe_ctrl_t *strobe_flash_ctrl,
  msm_strobe_flash_charge_enable_t charge_en)
{
  int32_t rc = -1;

  if (strobe_flash_ctrl &&
      strobe_flash_ctrl->strobe_data.strobe_state == STROBE_FLASH_INIT ) {
    strobe_flash_ctrl->strobe_data.strobe_ready = STROBE_FLASH_NOT_READY;

    struct flash_ctrl_data  flash_ctrl;

    flash_ctrl.flashtype = STROBE_FLASH;
    flash_ctrl.ctrl_data.strobe_ctrl.type = STROBE_FLASH_CTRL_CHARGE;
    flash_ctrl.ctrl_data.strobe_ctrl.charge_en = charge_en;

    rc = ioctl(strobe_flash_ctrl->camfd,
       MSM_CAM_IOCTL_FLASH_CTRL, &flash_ctrl);

    if (rc < 0) {
      CDBG("strobe_flash_xenon_charge: camfd = %d failed\n",
        strobe_flash_ctrl->camfd);
      return rc;
    }
    strobe_flash_ctrl->strobe_flash_charge_en = charge_en;
    strobe_flash_ctrl->strobe_data.strobe_ready =
    strobe_flash_ctrl->strobe_flash_charge_en;
    CDBG("strobe_flash_xenon_charge: camfd = %d\n",
      strobe_flash_ctrl->camfd);
  }
  CDBG("%s returns %d, charge %d\n", __func__, rc, charge_en);
  return rc;
}

flash_strobe_ready_t get_flash_ready_status(
  flash_strobe_ctrl_t *strobe_flash_ctrl)
{
  if(strobe_flash_ctrl)
    return strobe_flash_ctrl->strobe_data.strobe_ready;
  else
    return TRUE;
}

int8_t strobe_flash_device_release(flash_strobe_ctrl_t *strobe_flash_ctrl)
{

  int8_t rc = -1;
  struct flash_ctrl_data  flash_ctrl;
  if (strobe_flash_ctrl == NULL)
    return rc;

  strobe_flash_ctrl->strobe_data.strobe_ready = STROBE_FLASH_NOT_READY;
  strobe_flash_ctrl->strobe_flash_charge_en = STROBE_FLASH_CHARGE_DISABLE;

  if (strobe_flash_ctrl->strobe_data.strobe_state == STROBE_FLASH_INIT) {
    flash_ctrl.flashtype = STROBE_FLASH;
    flash_ctrl.ctrl_data.strobe_ctrl.type = STROBE_FLASH_CTRL_RELEASE;
    flash_ctrl.ctrl_data.strobe_ctrl.charge_en = STROBE_FLASH_CHARGE_DISABLE;

    rc =
      ioctl(strobe_flash_ctrl->camfd, MSM_CAM_IOCTL_FLASH_CTRL, &flash_ctrl);
  }
  return rc;
}
