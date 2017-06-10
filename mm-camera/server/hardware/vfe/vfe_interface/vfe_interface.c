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

#include "vfe.h"
#include "vfe_util_common.h"
#include "camera_dbg.h"

/*#define ENABLE_VFE_LOGGING 1*/
#ifdef ENABLE_VFE_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

static vfe_comp_root_t my_vfe_struct;

static uint32_t vfe_gen_handle(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[8 bits client idx] */
  uint32_t handle =	((++my_vfe_struct.vfe_handle_cnt) << 8) +
                  (0xff & client_idx);
  return handle;
}
static int vfe_parse_handle(uint32_t handle, uint8_t *client_idx)
{
  int tmp = (handle & 0xff);
  *client_idx = tmp & 0xff;
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, handle, *client_idx);
  return 0;
}
static uint32_t vfe_hndl_cnt = 0;

/*===========================================================================
 * FUNCTION    - get_vfe_client_info -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
static vfe_client_t *get_vfe_client_info(uint32_t handle)
{
  uint8_t obj_idx, client_idx;

  vfe_parse_handle(handle, &client_idx);
  if(client_idx >= VFE_MAX_CLIENT_NUM ||
      my_vfe_struct.client[client_idx].handle != handle) {
    CDBG_ERROR("%s: client_idx = %d, local_handle = 0x%x, input_handle = 0x%x",
      __func__, client_idx,
      my_vfe_struct.client[client_idx].handle,
      handle);
    return NULL;
  } else
    return &(my_vfe_struct.client[client_idx]);
} /* get_vfe_ctrl_info */

/*===========================================================================
 * FUNCTION    - vfe_client_parse_stats -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int vfe_client_parse_stats(uint32_t handle,
  int isp_started, stats_type_t stats_type, void *stats, void *stats_output)
{
  vfe_ctrl_info_t *vfe_ctrl_obj;
  vfe_client_t *vfe_client;
  vfe_op_mode_t op_mode;
  int rc = 0;

  vfe_client = get_vfe_client_info(handle);
  if(!vfe_client) {
    CDBG_ERROR("%s: null vfe client",  __func__);
    return -1;
  }

  rc = vfe_client->vfe_ops.parse_stats(vfe_client, isp_started,
    stats_type, stats, stats_output);
  if(rc < 0) {
    CDBG_ERROR("%s: vfe process failed : %d ", __func__, rc);
    return -1;
  }
  return rc;
}
/*===========================================================================
 * FUNCTION    - vfe_client_init -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to called after vfe_client_open
 *==========================================================================*/
static int vfe_client_init(uint32_t handle, mctl_ops_t *ops, void *init_data)
{
  vfe_client_t *vfe_client;
  int rc = 0;

  CDBG("%s: E", __func__);
  pthread_mutex_lock(&my_vfe_struct.mutex);
  vfe_client = get_vfe_client_info(handle);
  if(!vfe_client) {
    CDBG_ERROR("%s Invalid handle 0x%x passed ", __func__, handle);
    pthread_mutex_unlock(&my_vfe_struct.mutex);
    return -EINVAL;
  }
  /* init_data is not used now. Reserved */
  if(!ops) {
    CDBG_ERROR("%s Invalid handle passed %d ", __func__, handle);
    pthread_mutex_unlock(&my_vfe_struct.mutex);
    return -EINVAL;
  }
  if (ops->fd < 0) {
    CDBG_ERROR("%s Invalid argument camera fd %d ", __func__, ops->fd);
    pthread_mutex_unlock(&my_vfe_struct.mutex);
    return -EINVAL;
  }
  rc = ioctl(vfe_client->sdev_fd, VIDIOC_MSM_VFE_INIT);
  if(rc < 0) {
    CDBG_ERROR("%s: subdev init failed; error: %d %s\n", __func__, rc, strerror(errno));
    return -EINVAL;
  }
  vfe_client->ops = ops;
  /* register the vfe operations vfe_ops */
  rc = vfe_ops_init(vfe_client);
  if(rc < 0) {
    CDBG_ERROR("%s: vfe_ops_init failed", __func__);
    pthread_mutex_unlock(&my_vfe_struct.mutex);
    return -EINVAL;
  }
  pthread_mutex_unlock(&my_vfe_struct.mutex);
  return rc;
}/* vfe_client_init */

/*===========================================================================
 * FUNCTION    - vfe_client_process -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after vfe_client_init.
 *==========================================================================*/
static int vfe_client_process(uint32_t handle, int event, void *parm)
{
  vfe_client_t *vfe_client;
  int rc = 0;

  CDBG("%s: E, handle = 0x%x, event = %d", __func__, handle, event);
  vfe_client = get_vfe_client_info(handle);
  if(!vfe_client) {
    CDBG_ERROR("%s: null vfe client",  __func__);
    return -1;
  }

  rc = vfe_client->vfe_ops.process(vfe_client, event, parm);
  if(rc < 0) {
    CDBG_ERROR("%s: vfe process failed : %d ", __func__, rc);
    return -1;
  }
  return rc;
} /* vfe_client_process */

/*===========================================================================
 * FUNCTION    - vfe_client_set_params -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after vfe_client_init.
 *==========================================================================*/
static int vfe_client_set_params(uint32_t handle, int type, void *parm_in,
  void *parm_out)
{
  vfe_client_t *vfe_client;
  int rc = 0;

  CDBG("%s: E, handle = 0x%x, type = %d", __func__, handle, type);
  vfe_client = get_vfe_client_info(handle);
  if(!vfe_client) {
    CDBG_ERROR("%s: null vfe client",  __func__);
    return -1;
  }

  rc = vfe_client->vfe_ops.set_param(vfe_client, type, parm_in, parm_out);
  if(rc < 0) {
    CDBG_ERROR("%s:, vfe set parm failed : %d", __func__, rc);
    return -1;
  }
  return rc;
} /* vfe_client_set_params */

/*===========================================================================
 * FUNCTION    - vfe_client_get_params -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after vfe_client_init.
 *==========================================================================*/
static int vfe_client_get_params(uint32_t handle, int parm_type,
  void *parm, int parm_len)
{
  vfe_client_t *vfe_client;
  int rc = 0;

  CDBG("%s: E, handle = 0x%x, type = %d", __func__, handle, parm_type);
  vfe_client = get_vfe_client_info(handle);
  if(!vfe_client) {
    CDBG_ERROR("%s: null vfe client",  __func__);
    return -1;
  }

  rc = vfe_client->vfe_ops.get_param(vfe_client, parm_type, parm, parm_len);
  if(rc < 0) {
    CDBG_ERROR("%s:, vfe set parm failed : %d", __func__, rc);
    return -1;
  }
  return rc;
} /* vfe_client_get_params */

/*===========================================================================
 * FUNCTION    - vfe_client_destroy -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after vfe_client_init.
 *==========================================================================*/
static int vfe_client_destroy(uint32_t handle)
{
  int idx;
  vfe_client_t *vfe_client;
  vfe_ctrl_info_t *vfe_ctrl_obj;
  int rc = 0;

  CDBG("%s: E", __func__);
  pthread_mutex_lock(&my_vfe_struct.mutex);
  vfe_client = get_vfe_client_info(handle);
  if(!vfe_client) {
    CDBG_ERROR("%s: null vfe client",  __func__);
    pthread_mutex_unlock(&my_vfe_struct.mutex);
    return -1;
  }

  vfe_ctrl_obj = vfe_get_obj(vfe_client);
  if(!vfe_ctrl_obj) {
    CDBG_ERROR("%s: no VFE OBJ associated with clientr",  __func__);
    return -1;
  }

  rc = vfe_client->vfe_ops.destroy(vfe_client);
  if(rc < 0) {
    CDBG_ERROR("%s:, vfe set parm failed : %d", __func__, rc);
    pthread_mutex_unlock(&my_vfe_struct.mutex);
    return -1;
  }
  /* set the vfe operations NULL*/
  rc = vfe_ops_deinit(vfe_client);
  if(rc < 0) {
    CDBG_ERROR("%s: vfe_ops_init failed", __func__);
    pthread_mutex_unlock(&my_vfe_struct.mutex);
    return -EINVAL;
  }
  rc = ioctl(vfe_client->sdev_fd, VIDIOC_MSM_VFE_RELEASE);
  if(rc < 0) {
    CDBG_ERROR("%s: subdev release failed; error: %d %s\n", __func__, rc, strerror(errno));
    return -EINVAL;
  }
  close(vfe_client->sdev_fd);
  memset(vfe_client, 0, sizeof(vfe_client_t));
  memset(vfe_ctrl_obj, 0, sizeof(vfe_ctrl_info_t));
  pthread_mutex_unlock(&my_vfe_struct.mutex);
  return rc;
} /* vfe_client_destroy */
/*===========================================================================
 * FUNCTION    - VFE_client_open -
 *
 * DESCRIPTION:
 *==========================================================================*/
uint32_t VFE_client_open(module_ops_t *ops, int sdev_number)
{
  int idx;
  vfe_client_t *vfe_client = NULL;
  char sdev_name[32];

  if(!ops) {
    CDBG_ERROR("%s: null ops pointer",  __func__);
    return 0;
  }
  memset(ops,  0,  sizeof(module_ops_t));

  pthread_mutex_lock(&my_vfe_struct.mutex);
  for(idx = 0; idx < VFE_MAX_CLIENT_NUM; idx++) {
    if(my_vfe_struct.client[idx].handle == 0) {
      vfe_client = &my_vfe_struct.client[idx];
      break;
    }
  }
  /* if not found return null */
  if (!vfe_client) {
    pthread_mutex_unlock(&my_vfe_struct.mutex);
    return (uint32_t)NULL;
  } else {
    memset(vfe_client, 0, sizeof(vfe_client_t));
    vfe_client->obj_idx_mask = 0;
    vfe_client->client_idx = idx;
    vfe_client->handle = vfe_gen_handle((uint8_t)idx);
    snprintf(sdev_name, sizeof(sdev_name), "/dev/v4l-subdev%d", sdev_number);
    vfe_client->sdev_fd = open(sdev_name, O_RDWR | O_NONBLOCK);
    if(vfe_client->sdev_fd < 0) {
      CDBG_ERROR("%s: subdev open failed; %s\n", __func__, strerror(errno));
      goto ERROR_OPEN;
    }
    ops->handle = (uint32_t)vfe_client->handle;
    ops->init = vfe_client_init;
    ops->set_params = vfe_client_set_params;
    ops->get_params = vfe_client_get_params;
    ops->process = vfe_client_process;
    ops->abort = NULL;
    ops->destroy= vfe_client_destroy;
    ops->parse_stats = vfe_client_parse_stats;
  }
  pthread_mutex_unlock(&my_vfe_struct.mutex);
  CDBG("%s: client_idx = %d, handle = 0x%x",
    __func__, idx, vfe_client->handle);
  return vfe_client->handle;
ERROR_OPEN:
  memset(vfe_client, 0, sizeof(vfe_client_t));
	pthread_mutex_unlock(&my_vfe_struct.mutex);
  return (uint32_t)NULL;
} /* VFE_client_open */

/*===========================================================================
 * FUNCTION    - VFE_comp_create -
 *
 * DESCRIPTION: initialize VFE component.
 *==========================================================================*/
int VFE_comp_create()
{
  memset(&my_vfe_struct, 0, sizeof(my_vfe_struct));
  pthread_mutex_init(&my_vfe_struct.mutex, NULL);
  return 0;
} /* VFE_comp_create */
/*===========================================================================
 * FUNCTION    - VFE_comp_destroy -
 *
 * DESCRIPTION: destroy the component
 *==========================================================================*/
int VFE_comp_destroy()
{
  pthread_mutex_destroy(&my_vfe_struct.mutex);
  memset(&my_vfe_struct, 0, sizeof(my_vfe_struct));
  return 0;

} /* VFE_comp_destroy */
