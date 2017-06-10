/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "camera_dbg.h"
#include "flash_interface.h"
#include "led.h"

static led_comp_root_t led_controller;

static uint32_t led_gen_handle(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[4bits obj idx]|[4 bits client idx] */
  uint32_t handle =	((++led_controller.led_handle_cnt) << 8) +
	                (0xff & client_idx);
  return handle;
}

static int led_parse_handle(uint32_t handle, uint8_t *client_idx)
{
  int tmp = (handle & 0xff);
  *client_idx = tmp & 0xff;
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, handle, *client_idx);
  return 0;
}
/*===========================================================================
 * FUNCTION    - get_led_client_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static led_client_t *get_led_client_info(uint32_t handle)
{
  uint8_t client_idx;
  led_parse_handle(handle, &client_idx);
  if(client_idx >= LED_MAX_CLIENT_NUM ||
	 led_controller.client[client_idx].handle != handle) {
	CDBG_ERROR("%s: client_idx = %d, local_handle = 0x%x, input_handle = 0x%x",
			   __func__, client_idx,
			   led_controller.client[client_idx].handle,
			   handle);
    return NULL;
  } else
    return &(led_controller.client[client_idx]);
} /* get_led_client_info */

/*===========================================================================
 * FUNCTION    - flash_led_interface_init -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
int flash_led_client_init(uint32_t handle, mctl_ops_t *ops, void *p_init_data)
{
  int status = 0;
  flash_init_data_t *init_data = p_init_data;
  uint32_t fd = init_data->fd;
  led_client_t *led_client = get_led_client_info(handle);
  if(!led_client)
    return -1;
  else {
	led_client->ops = ops;
	led_client->led_ctrl.camfd = fd;
    led_ctrl_init(&led_client->led_ctrl, fd);
  }
  return status;
}

/*===========================================================================
 * FUNCTION    - flash_led_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int  flash_led_client_set_params(uint32_t handle, int type, void *parm_in, void *parm_out)
{
  int status = 0;
  led_client_t *led_client = get_led_client_info(handle);
  flash_led_set_t *flash_led_set = parm_in;

  if(!led_client)
    return -1;

  switch(type) {
  case FLASH_SET_STATE:
    set_led_state(flash_led_set->data.led_state);
    break;
  case FLASH_SET_MODE:
    set_led_mode(flash_led_set->data.led_mode);
    break;
  }
  return status;
}
/*===========================================================================
 * FUNCTION    - flash_led_get_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int flash_led_client_get_params(uint32_t handle, int type,
                                       void *parm, int parm_len)
{
  flash_led_get_t *flash_led_get = parm;
  led_client_t *led_client = get_led_client_info(handle);

  if(!led_client)
    return -1;

  switch(type) {
  case FLASH_GET_STATE:
    flash_led_get->data.led_state = led_client->led_ctrl.led_data.led_state;
    break;
  case FLASH_GET_MODE:
    flash_led_get->data.led_mode = led_client->led_ctrl.led_data.led_mode;
    break;
  }
  return 0;
}

/*===========================================================================
 * FUNCTION    - flash_led_interface_destroy -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int flash_led_client_destroy(uint32_t handle)
{
  led_client_t *led_client = get_led_client_info(handle);

  pthread_mutex_lock(&led_controller.mutex);
  if(!led_client) {
    pthread_mutex_unlock(&led_controller.mutex);
    return 0;
  }
  led_ctrl_release();
  memset(led_client, 0, sizeof(led_client_t));
  pthread_mutex_unlock(&led_controller.mutex);
  return 0;
}


/*============================================================================
 * FUNCTION    - led_interface_create -
 *
 * DESCRIPTION:
 *===========================================================================*/
uint32_t flash_led_client_open(module_ops_t *ops)
{
  uint32_t handle = 0;
  int idx;
  led_client_t *led_client = NULL;

  if(!ops) {
	CDBG_ERROR("%s: null ops pointer",  __func__);
	return 0;
  }
  memset(ops,  0,  sizeof(module_ops_t));

  pthread_mutex_lock(&led_controller.mutex);
  for(idx = 0; idx < LED_MAX_CLIENT_NUM; idx++) {
    if(led_controller.client[idx].handle == 0) {
	  led_client = &led_controller.client[idx];
      break;
	}
  }
  /* if not found return null */
  if (!led_client) {
	pthread_mutex_unlock(&led_controller.mutex);
    return (uint32_t)NULL;
  } else {
    memset(led_client, 0, sizeof(led_client_t));
	led_client->obj_idx_mask = 0;
	led_client->client_idx = idx;
    led_client->my_comp_id = MCTL_COMPID_FLASHLED;
    led_client->handle = led_gen_handle((uint8_t)idx);
	ops->handle = (uint32_t)led_client->handle;
	ops->init = flash_led_client_init;
	ops->set_params = flash_led_client_set_params;
	ops->get_params = flash_led_client_get_params;
	ops->process = NULL;
	ops->abort = NULL;
	ops->destroy= flash_led_client_destroy;
  }
  pthread_mutex_unlock(&led_controller.mutex);
  CDBG("%s: client_idx = %d, handle = 0x%x",
	   __func__, idx, led_client->handle);
  return led_client->handle;
}/*stats_interface_create*/

/*===========================================================================
 * FUNCTION    - led_comp_create -
 *
 * DESCRIPTION: initialize AXI component.
 *==========================================================================*/
int flash_led_comp_create()
{
  memset(&led_controller, 0, sizeof(led_controller));
  pthread_mutex_init(&led_controller.mutex, NULL);
  return 0;
}
/*===========================================================================
 * FUNCTION    - led_comp_destroy -
 *
 * DESCRIPTION: destroy the component
 *==========================================================================*/
int flash_led_comp_destroy()
{
  pthread_mutex_destroy(&led_controller.mutex);
  memset(&led_controller, 0, sizeof(led_controller));
  return 0;
}
