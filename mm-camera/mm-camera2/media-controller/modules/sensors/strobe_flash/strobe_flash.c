/* strobe_flash.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "strobe_flash.h"
#include "sensor_common.h"

/** strobe_flash_open:
 *    @strobe_flash_ctrl: address of pointer to
 *                   sensor_strobe_flash_data_t struct
 *    @subdev_name: strobe flash subdev name
 *
 * 1) Allocates memory for strobe flash control structure
 * 2) Opens strobe flash subdev node
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t strobe_flash_open(void **strobe_flash_ctrl,
  const char *subdev_name)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_strobe_flash_data_t *ctrl = NULL;
  char subdev_string[32];

  SLOW("Enter");
  if (!strobe_flash_ctrl || !subdev_name) {
    SERR("failed sctrl %p subdev name %p",
      strobe_flash_ctrl, subdev_name);
    return SENSOR_ERROR_INVAL;
  }
  ctrl = malloc(sizeof(sensor_strobe_flash_data_t));
  if (!ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  memset(ctrl, 0, sizeof(sensor_strobe_flash_data_t));

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s", subdev_name);
  SLOW("sd name %s", subdev_string);
  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if (ctrl->fd < 0) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR;
  }

  *strobe_flash_ctrl = (void *)ctrl;
  SLOW("Exit");
  return rc;

ERROR:
  free(ctrl);
  return rc;
}

/** strobe_flash_process:
 *    @strobe_flash_ctrl: EEPROM control handle
 *    @event: configuration event type
 *    @data: NULL
 *
 * Handle all EERPOM events
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t strobe_flash_process(void *strobe_flash_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_strobe_flash_data_t *ctrl =
    (sensor_strobe_flash_data_t *)strobe_flash_ctrl;
  if (!strobe_flash_ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  return SENSOR_SUCCESS;
}

/** strobe_flash_close:
 *    @strobe_flash_ctrl: EERPOM control handle
 *
 * 2) Close fd
 * 3) Free strobe flash control structure
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t strobe_flash_close(void *strobe_flash_ctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_strobe_flash_data_t *ctrl =
    (sensor_strobe_flash_data_t *)strobe_flash_ctrl;

  SLOW("Enter");

  /* close subdev */
  close(ctrl->fd);

  free(ctrl);
  SLOW("Exit");
  return rc;
}

/** strobe_flash_sub_module_init:
 *    @func_tbl: pointer to sensor function table
 *
 * Initialize function table for strobe flash device to be used
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t strobe_flash_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  if (!func_tbl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  func_tbl->open = strobe_flash_open;
  func_tbl->process = strobe_flash_process;
  func_tbl->close = strobe_flash_close;
  return SENSOR_SUCCESS;
}
