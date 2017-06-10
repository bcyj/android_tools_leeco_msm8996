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
#include "strobe_flash.h"

static strobe_comp_root_t strobe_controller;

static uint32_t strobe_gen_handle(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[4bits obj idx]|[4 bits client idx] */
  uint32_t handle =	((++strobe_controller.strobe_handle_cnt) << 8) +
	                (0xff & client_idx);
  return handle;
}

static int strobe_parse_handle(uint32_t handle, uint8_t *client_idx)
{
  int tmp = (handle & 0xff);
  *client_idx = tmp & 0xff;
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, handle, *client_idx);
  return 0;
}
/*===========================================================================
 * FUNCTION    - get_strobe_client_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static strobe_client_t *get_strobe_client_info(uint32_t handle)
{
  uint8_t client_idx;
  strobe_parse_handle(handle, &client_idx);
  if(client_idx >= STROBE_MAX_CLIENT_NUM ||
	 strobe_controller.client[client_idx].handle != handle) {
	CDBG_ERROR("%s: client_idx = %d, local_handle = 0x%x, input_handle = 0x%x",
			   __func__, client_idx,
			   strobe_controller.client[client_idx].handle,
			   handle);
    return NULL;
  } else
    return &(strobe_controller.client[client_idx]);
} /* get_strobe_client_info */

/*===========================================================================
 * FUNCTION    - flash_strobe_interface_init -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be calstrobe after set_parms.
 *==========================================================================*/
int flash_strobe_client_init(uint32_t handle, mctl_ops_t *ops, void *p_init_data)
{
  int status = 0;
  flash_init_data_t *init_data = p_init_data;
  uint32_t fd = init_data->fd;
  strobe_client_t *strobe_client = get_strobe_client_info(handle);
  if(!strobe_client)
    return -1;
  else {
	strobe_client->ops = ops;
	strobe_client->strobe_ctrl.camfd = fd;
    strobe_flash_device_init(&strobe_client->strobe_ctrl, fd);
  }
  return status;
}

/*===========================================================================
 * FUNCTION    - flash_strobe_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int  flash_strobe_client_set_params(uint32_t handle, int type, void *parm_in, void *parm_out)
{
  int status = 0;
  strobe_client_t *strobe_client = get_strobe_client_info(handle);
  flash_strobe_set_t *flash_strobe_set = parm_in;

  if(!strobe_client)
    return -1;

  switch(type) {
  case FLASH_SET_STATE:
    strobe_flash_device_charge(&strobe_client->strobe_ctrl, 
                               flash_strobe_set->charge_enable);
    break;
  case FLASH_SET_MODE:
    break;
  }
  return status;
}
/*===========================================================================
 * FUNCTION    - flash_strobe_get_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int flash_strobe_client_get_params(uint32_t handle, int type,
                                          void *parm, int parm_len)
{
  strobe_client_t *strobe_client = get_strobe_client_info(handle);
  flash_strobe_get_t *flash_strobe_get = parm;

  if(!strobe_client)
    return -1;

  switch(type) {
  case FLASH_GET_STATE:
    flash_strobe_get->data.strobe_ready = strobe_client->strobe_ctrl.strobe_data.strobe_ready;
    break;
  case FLASH_GET_MODE:
    break;
  }
  return 0;
}

/*===========================================================================
 * FUNCTION    - flash_strobe_interface_destroy -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int flash_strobe_client_destroy(uint32_t handle)
{
	strobe_client_t *strobe_client = get_strobe_client_info(handle);

	pthread_mutex_lock(&strobe_controller.mutex);
	if(!strobe_client) {
	  pthread_mutex_unlock(&strobe_controller.mutex);
	  return 0;
	}

	memset(strobe_client, 0, sizeof(strobe_client_t));
	pthread_mutex_unlock(&strobe_controller.mutex);
	return 0;
}

/*============================================================================
 * FUNCTION    - strobe_interface_create -
 *
 * DESCRIPTION:
 *===========================================================================*/
uint32_t flash_strobe_client_open(module_ops_t *ops)
{
  uint32_t handle = 0;
  int idx;
  strobe_client_t *strobe_client = NULL;

  if(!ops) {
	CDBG_ERROR("%s: null ops pointer",  __func__);
	return 0;
  }
  memset(ops,  0,  sizeof(module_ops_t));

  pthread_mutex_lock(&strobe_controller.mutex);
  for(idx = 0; idx < STROBE_MAX_CLIENT_NUM; idx++) {
    if(strobe_controller.client[idx].handle == 0) {
	  strobe_client = &strobe_controller.client[idx];
      break;
	}
  }
  /* if not found return null */
  if (!strobe_client) {
	pthread_mutex_unlock(&strobe_controller.mutex);
    return (uint32_t)NULL;
  } else {
    memset(strobe_client, 0, sizeof(strobe_client_t));
	strobe_client->obj_idx_mask = 0;
	strobe_client->client_idx = idx;
    strobe_client->my_comp_id = MCTL_COMPID_FLASHSTROBE;
    strobe_client->handle = strobe_gen_handle((uint8_t)idx);
	ops->handle = (uint32_t)strobe_client->handle;
	ops->init = flash_strobe_client_init;
	ops->set_params = flash_strobe_client_set_params;
	ops->get_params = flash_strobe_client_get_params;
	ops->process = NULL;
	ops->abort = NULL;
	ops->destroy= flash_strobe_client_destroy;
  }
  pthread_mutex_unlock(&strobe_controller.mutex);
  CDBG("%s: client_idx = %d, handle = 0x%x",
	   __func__, idx, strobe_client->handle);
  return strobe_client->handle;
}/*stats_interface_create*/

/*===========================================================================
 * FUNCTION    - strobe_comp_create -
 *
 * DESCRIPTION: initialize AXI component.
 *==========================================================================*/
int flash_strobe_comp_create()
{
  memset(&strobe_controller, 0, sizeof(strobe_controller));
  pthread_mutex_init(&strobe_controller.mutex, NULL);
  return 0;
}
/*===========================================================================
 * FUNCTION    - strobe_comp_destroy -
 *
 * DESCRIPTION: destroy the component
 *==========================================================================*/
int flash_strobe_comp_destroy()
{
  pthread_mutex_destroy(&strobe_controller.mutex);
  memset(&strobe_controller, 0, sizeof(strobe_controller));
  return 0;
}

