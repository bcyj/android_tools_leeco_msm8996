/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "axi.h"
#include "camera_dbg.h"

static axi_comp_root_t my_axi_struct;

static uint32_t axi_gen_handle(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[4bits obj idx]|[4 bits client idx] */
  uint32_t handle =	((++my_axi_struct.axi_handle_cnt) << 8) +
	                (0xff & client_idx);
  return handle;
}
static int axi_parse_handle(uint32_t handle, uint8_t *client_idx)
{
  int tmp = (handle & 0xff);
  *client_idx = tmp & 0xff;
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, handle, *client_idx);
  return 0;
}
/*===========================================================================
 * FUNCTION    - get_axi_ctrl_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static axi_client_t *get_axi_client_info(uint32_t handle)
{
  uint8_t obj_idx, client_idx;

  axi_parse_handle(handle, &client_idx);
  if(client_idx >= AXI_MAX_CLIENT_NUM ||
	 my_axi_struct.client[client_idx].handle != handle) {
	CDBG_ERROR("%s: client_idx = %d, local_handle = 0x%x, input_handle = 0x%x",
			   __func__, client_idx,
			   my_axi_struct.client[client_idx].handle,
			   handle);
    return NULL;
  } else
    return &(my_axi_struct.client[client_idx]);
} /* get_axi_ctrl_info */

/*===========================================================================
 * FUNCTION    - axi_interface_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int axi_client_init(uint32_t handle, mctl_ops_t *ops, void *init_data)
{
  axi_client_t *axi_client;
  int rc;
  uint8_t concurrent_enabled = *((uint8_t *)init_data);

  pthread_mutex_lock(&my_axi_struct.mutex);
  axi_client = get_axi_client_info(handle);
  if(!axi_client) {
    CDBG_ERROR("%s Invalid handle 0x%x passed ", __func__, handle);
    pthread_mutex_unlock(&my_axi_struct.mutex);
    return -EINVAL;
  }
  /* init_data is not used now. Reserved */
  if(!ops) {
    CDBG_ERROR("%s Invalid handle passed %d ", __func__, handle);
    pthread_mutex_unlock(&my_axi_struct.mutex);
    return -EINVAL;
  }
  if (ops->fd < 0) {
    CDBG_ERROR("%s Invalid argument camera fd %d ", __func__, ops->fd);
    pthread_mutex_unlock(&my_axi_struct.mutex);
    return -EINVAL;
  }
  CDBG_ERROR("%s axi client %d\n ", __func__, concurrent_enabled);
  axi_client->ops = ops;
  if(axi_client->sdev_fd >= 0) {
    rc = ioctl(axi_client->ops->fd, MSM_CAM_IOCTL_AXI_INIT, &concurrent_enabled);
    if(rc < 0) {
      CDBG_ERROR("%s: subdev init failed; error: %d\n", __func__, rc);
      return -EINVAL;
    }
  }

  pthread_mutex_unlock(&my_axi_struct.mutex);
  return 0;
} /* axi_client_init */


/*===========================================================================
 * FUNCTION    - axi_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int axi_client_set_params(uint32_t handle, int type, void *parm_in, void *parm_out)
{
  axi_set_t *axi_set_parm = parm_in;
  int rc = 0;
  axi_client_t *axi_client;

  pthread_mutex_lock(&my_axi_struct.mutex);
  axi_client = get_axi_client_info(handle);
  if(!axi_client) {
    CDBG_ERROR("%s Invalid handle passed ", __func__);
    pthread_mutex_unlock(&my_axi_struct.mutex);
	return -EINVAL;
  }
  if(!axi_set_parm) {
    CDBG_ERROR("%s Input argument is NULL ", __func__);
    pthread_mutex_unlock(&my_axi_struct.mutex);
	return -EINVAL;
  }
  switch(type) {
  case AXI_PARM_SENSOR_DATA: {
    sensor_get_t sensor_get;
    axi_client->ops->fetch_params(
                     axi_client->my_comp_id, axi_client->ops->parent,
                     ((MCTL_COMPID_SENSOR << 24)|SENSOR_GET_OUTPUT_CFG),
                     &sensor_get, sizeof(sensor_get));
    axi_set_parm->data.sensor_data.sensor_output_format =
      sensor_get.data.sensor_output.output_format;
    axi_set_parm->data.sensor_data.sensor_raw_depth =
      sensor_get.data.sensor_output.raw_output;
    break;
  }
  default:
    break;
  }
  rc = axi_set_params(&my_axi_struct, axi_client, type, axi_set_parm);
  pthread_mutex_unlock(&my_axi_struct.mutex);
  return rc;
} /* axi_set_params */

/*===========================================================================
 * FUNCTION    - axi_process -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int axi_client_process(uint32_t handle, int event, void *data)
{
  axi_client_t *axi_client;
  axi_config_t *cfg = data;
  int rc = 0;

  pthread_mutex_lock(&my_axi_struct.mutex);
  axi_client = get_axi_client_info(handle);
  if(!axi_client) {
    CDBG_ERROR("%s Invalid handle passed %d ", __func__, handle);
    pthread_mutex_unlock(&my_axi_struct.mutex);
    return -EINVAL;
  }
  switch(event) {
  case AXI_PROC_EVENT_CONFIG:
    rc = axi_config(&my_axi_struct, axi_client, cfg);
	break;
  case AXI_PROC_CMD_OPS:
    rc = axi_command_ops(&my_axi_struct, axi_client, data);
    break;
  case AXI_PROC_EVENT_REG_UPDATE:
    rc = axi_proc_reg_update(&my_axi_struct, axi_client, data);
    break;
  case AXI_PROC_EVENT_UNREGISTER_WMS:
    rc = axi_proc_unregister_wms(&my_axi_struct, axi_client, data);
    break;
  default:
	rc = -1;
	break;
  }
  pthread_mutex_unlock(&my_axi_struct.mutex);
  return rc;
} /* axi_process */

/*===========================================================================
 * FUNCTION    - axi_client_destroy -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int axi_client_destroy(uint32_t handle)
{
  int idx;
  axi_client_t *axi_client;
  axi_obj_t *axi_obj = NULL;
  int rc = 0;

  pthread_mutex_lock(&my_axi_struct.mutex);
  axi_client = get_axi_client_info(handle);
  if(!axi_client) {
    CDBG_ERROR("%s Invalid handle passed %d ", __func__, handle);
	pthread_mutex_unlock(&my_axi_struct.mutex);
    return -EINVAL;
  }
  /* now we have not implemented the use case of using 2 AXI obj */
  if(axi_client->obj_idx_mask == 1)
    axi_obj= &my_axi_struct.axi_obj[0];
  else if(axi_client->obj_idx_mask == 2)
	axi_obj= &my_axi_struct.axi_obj[1];
  if (!axi_obj) {
    CDBG("axi_client_destroy: NULL axi_obj\n");
    return 0;
  }
  for(idx = 0; idx < AXI_INTF_MAX_NUM; idx++) {
	if(axi_obj->axi_ctrl[idx].used &&
	   axi_obj->axi_ctrl[idx].client_idx == axi_client->client_idx) {
      memset(&axi_obj->axi_ctrl[idx], 0,  sizeof(axi_ctrl_t));
	}
  }
  
  if(axi_client->sdev_fd >= 0) {
    rc = ioctl(axi_client->ops->fd, MSM_CAM_IOCTL_AXI_RELEASE);
    if(rc < 0) {
      CDBG_ERROR("%s: subdev release failed; error: %d\n", __func__, rc);
      return -EINVAL;
    }
    close(axi_client->sdev_fd);
  }
  memset(axi_client,  0, sizeof(axi_client_t));
  axi_obj->ref_count--;
  if (!axi_obj->ref_count)
	axi_obj->reset_done = false;
  pthread_mutex_unlock(&my_axi_struct.mutex);
  return 0;
} /* axi_client_destroy */

/*===========================================================================
 * FUNCTION    - axi_client_create -
 *
 * DESCRIPTION:
 *==========================================================================*/
uint32_t AXI_client_open(module_ops_t *ops, int sdev_number)
{
  int idx, rc;
  axi_obj_t *axi_obj = NULL;
  axi_client_t *axi_client = NULL;
  char sdev_name[32];
  if(!ops) {
	CDBG_ERROR("%s: null ops pointer",  __func__);
	return 0;
  }
  memset(ops,  0,  sizeof(module_ops_t));

  pthread_mutex_lock(&my_axi_struct.mutex);
  for(idx = 0; idx < AXI_MAX_CLIENT_NUM; idx++) {
    if(my_axi_struct.client[idx].handle == 0) {
	  axi_client = &my_axi_struct.client[idx];
      break;
	}
  }
  /* if not found return null */
  if (!axi_client) {
	pthread_mutex_unlock(&my_axi_struct.mutex);
    return (uint32_t)NULL;
  } else {
    memset(axi_client, 0, sizeof(axi_client_t));
    axi_client->obj_idx_mask = 0;
    axi_client->client_idx = idx;
    axi_client->my_comp_id = MCTL_COMPID_AXI;
    axi_client->handle = axi_gen_handle((uint8_t)idx);
    if(sdev_number >= 0) {
        snprintf(sdev_name, sizeof(sdev_name), "/dev/v4l-subdev%d", sdev_number);
        axi_client->sdev_fd = open(sdev_name, O_RDWR | O_NONBLOCK);
      if(axi_client->sdev_fd < 0) {
        CDBG_ERROR("%s: subdev open failed; %s\n", __func__, strerror(errno));
        goto ERROR_OPEN;
      }
    } else {
      axi_client->sdev_fd = -1;
    }

	ops->handle = (uint32_t)axi_client->handle;
	ops->init = axi_client_init;
	ops->set_params = axi_client_set_params;
	ops->get_params = NULL;
	ops->process = axi_client_process;
	ops->abort = NULL;
	ops->destroy= axi_client_destroy;
  }
  pthread_mutex_unlock(&my_axi_struct.mutex);
  CDBG("%s: client_idx = %d, handle = 0x%x",
	   __func__, idx, axi_client->handle);
  return axi_client->handle;

ERROR_OPEN:
  memset(axi_client, 0, sizeof(axi_client_t));
  pthread_mutex_unlock(&my_axi_struct.mutex);
  return (uint32_t)NULL;
} /* AXI_client_open */

/*===========================================================================
 * FUNCTION    - AXI_comp_create -
 *
 * DESCRIPTION: initialize AXI component.
 *==========================================================================*/
int AXI_comp_create()
{
  memset(&my_axi_struct, 0, sizeof(my_axi_struct));
  pthread_mutex_init(&my_axi_struct.mutex, NULL);
  return 0;
}
/*===========================================================================
 * FUNCTION    - AXI_comp_destroy -
 *
 * DESCRIPTION: destroy the component
 *==========================================================================*/
int AXI_comp_destroy()
{
  pthread_mutex_destroy(&my_axi_struct.mutex);
  memset(&my_axi_struct, 0, sizeof(my_axi_struct));
  return 0;
}
