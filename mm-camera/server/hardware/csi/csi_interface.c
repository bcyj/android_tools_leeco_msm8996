/*==========================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
#include "csi.h"
#include "camera_dbg.h"

static csi_comp_root_t my_csi_struct;

#ifdef CSI_DEBUG
#undef CDBG
#define CDBG LOGE
#endif
/*===========================================================================
 * FUNCTION    - csi_gen_handle -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint32_t csi_gen_handle(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[4bits obj idx]|[4 bits client idx] */
  uint32_t handle = ((++my_csi_struct.csi_handle_cnt) << 8) +
                    (0xff & client_idx);
  return handle;
} /* csi_gen_handle */

/*===========================================================================
 * FUNCTION    - csi_parse_handle -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int csi_parse_handle(uint32_t handle, uint8_t *client_idx)
{
  int tmp = (handle & 0xff);
  *client_idx = tmp & 0xff;
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, handle, *client_idx);
  return 0;
} /* csi_parse_handle */

/*===========================================================================
 * FUNCTION    - get_csi_client_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static csi_client_t *get_csi_client_info(uint32_t handle)
{
  uint8_t client_idx;

  csi_parse_handle(handle, &client_idx);
  if ((client_idx >= CSI_MAX_CLIENT_NUM) ||
      (my_csi_struct.client[client_idx].handle != handle)) {
    CDBG_ERROR("%s: client_idx = %d, local_handle = 0x%x, input_handle = 0x%x",
                __func__, client_idx,
                my_csi_struct.client[client_idx].handle,
                handle);
    return NULL;
  } else
    return &(my_csi_struct.client[client_idx]);
} /* get_csi_client_info */

/*==========================================================
 * FUNCTION    - csi_client_get_params -
 *
 * DESCRIPTION:
 *==========================================================*/
static int csi_client_get_params(uint32_t handle, int type, void *parm,
  int parm_len)
{
  int rc = 0;
  csi_client_t *csi_client = NULL;
  csi_get_t *csi_get = NULL;

  pthread_mutex_lock(&my_csi_struct.mutex);

  CDBG("%s: parm type =%d\n", __func__, type);

  csi_client = get_csi_client_info(handle);
  if(!csi_client) {
    CDBG_ERROR("%s: null csi client\n", __func__);
    pthread_mutex_unlock(&my_csi_struct.mutex);
    return -1;
  }

  csi_get = (csi_get_t *)parm;

  switch(type) {
  case CSI_GET_CSID_VERSION:
    csi_get->csid_version = csi_client->csi_ctrl_obj.csid_version;
    CDBG("%s csid version = %x\n", __func__,
      csi_get->csid_version);
    break;

  default:
    rc = -EINVAL;
  }

  pthread_mutex_unlock(&my_csi_struct.mutex);

  return rc;
} /* csi_client_set_params */

/*==========================================================
 * FUNCTION    - csi_client_set_params -
 *
 * DESCRIPTION:
 *==========================================================*/
static int csi_client_set_params(uint32_t handle, int type, void *parm_in,
  void *parm_out)
{
  int rc = 0;
  csi_client_t *csi_client = NULL;
  csi_set_t *csi_set = NULL;

  pthread_mutex_lock(&my_csi_struct.mutex);

  CDBG("%s: parm type =%d\n", __func__, type);

  csi_client = get_csi_client_info(handle);
  if(!csi_client) {
    CDBG_ERROR("%s: null csi client\n", __func__);
    pthread_mutex_unlock(&my_csi_struct.mutex);
    return -1;
  }

  csi_set = (csi_set_t *)parm_in;

  switch(type) {
  case CSI_SET_DATA:
    csi_client->csi_ctrl_obj.csi_params = csi_set->data.csi_params;
    CDBG("%s in, out csi_params = %p, %p\n", __func__,
      csi_set->data.csi_params,
      csi_client->csi_ctrl_obj.csi_params);
    break;

  case CSI_SET_CFG:
    csi_util_set_cfg(&csi_client->csi_ctrl_obj, &csi_set->data);
    break;

  default:
    rc = -EINVAL;
  }

  pthread_mutex_unlock(&my_csi_struct.mutex);

  return rc;
} /* csi_client_set_params */

/*==========================================================
 * FUNCTION    - csi_client_process -
 *
 * DESCRIPTION:
 *==========================================================*/
static int csi_client_process(uint32_t handle, int event, void *data)
{
  int rc = 0;
  csi_client_t *csi_client = NULL;

  pthread_mutex_lock(&my_csi_struct.mutex);

  CDBG("%s: event type =%d\n", __func__, event);

  csi_client = get_csi_client_info(handle);
  if(!csi_client) {
    CDBG_ERROR("%s: null csi client\n", __func__);
    pthread_mutex_unlock(&my_csi_struct.mutex);
    return -1;
  }

  switch(event) {
  case CSI_PROCESS_INIT:
    csi_util_process_init(&csi_client->csi_ctrl_obj);
    break;

  default:
    rc = -EINVAL;
  }

  pthread_mutex_unlock(&my_csi_struct.mutex);

  return rc;
} /* csi_client_set_params */


/*============================================================================
 * FUNCTION    - csi_interface_destroy -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int csi_client_destroy(uint32_t handle)
{
  csi_client_t *client;

  CDBG("%s: handle = 0x%x", __func__, handle);
  pthread_mutex_lock(&my_csi_struct.mutex);

  client = get_csi_client_info(handle);
  if(!client) {
    CDBG_ERROR("%s Invalid handle passed ", __func__);
    pthread_mutex_unlock(&my_csi_struct.mutex);
    return 0;
  }

  memset(client, 0, sizeof(csi_client_t));
  pthread_mutex_unlock(&my_csi_struct.mutex);
  return 0;
} /*csi_client_destroy*/

/*============================================================================
 * FUNCTION    - csi_interface_init -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int csi_client_init(uint32_t handle, mctl_ops_t *ops,
  void *init_data)
{
  csi_client_t *client;

  pthread_mutex_lock(&my_csi_struct.mutex);

  client = get_csi_client_info(handle);
  if(!client) {
    CDBG_ERROR("%s Invalid handle 0x%x passed ", __func__, handle);
    pthread_mutex_unlock(&my_csi_struct.mutex);
    return -EINVAL;
  }

  /* init_data is not used now. Reserved */
  if(!ops) {
    CDBG_ERROR("%s Invalid handle passed %d ", __func__, handle);
    pthread_mutex_unlock(&my_csi_struct.mutex);
    return -EINVAL;
  }

  if (ops->fd < 0) {
    CDBG_ERROR("%s Invalid argument camera fd %d ", __func__, ops->fd);
    pthread_mutex_unlock(&my_csi_struct.mutex);
    return -EINVAL;
  }

  client->ops = ops;
  client->csi_ctrl_obj.fd = ops->fd;
  CDBG("%s ops->fd = %d\n", __func__, ops->fd);
  pthread_mutex_unlock(&my_csi_struct.mutex);
  return 0;
} /*csi_interface_init*/

/*============================================================================
 * FUNCTION    - CSI_client_open -
 *
 * DESCRIPTION:
 *===========================================================================*/
uint32_t CSI_client_open(module_ops_t *ops)
{
  uint32_t handle = 0;
  int idx = 0;
  csi_client_t *csi_client = NULL;

  if (!ops) {
    CDBG_ERROR("%s: null ops pointer", __func__);
    return 0;
  }

  memset(ops, 0, sizeof(module_ops_t));

  pthread_mutex_lock(&my_csi_struct.mutex);

  for (idx = 0; idx < CSI_MAX_CLIENT_NUM; idx++) {
    if (my_csi_struct.client[idx].handle == 0) {
      csi_client = &my_csi_struct.client[idx];
      break;
    }
  }

  /* if not found return null */
  if (!csi_client) {
    pthread_mutex_unlock(&my_csi_struct.mutex);
    return (uint32_t)NULL;
  } else {
    memset(csi_client, 0, sizeof(csi_client_t));
    csi_client->client_idx = idx;
    csi_client->handle = csi_gen_handle((uint8_t)idx);
    ops->handle = (uint32_t)csi_client->handle;
    ops->init = csi_client_init;
    ops->set_params = csi_client_set_params;
    ops->get_params = csi_client_get_params;
    ops->process = csi_client_process;
    ops->abort = NULL;
    ops->destroy= csi_client_destroy;
  }

  pthread_mutex_unlock(&my_csi_struct.mutex);

  CDBG("%s: client_idx = %d, handle = 0x%x",
        __func__, idx, csi_client->handle);

  return csi_client->handle;
} /*CSI_client_open */

/*===========================================================================
 * FUNCTION    - CSI_comp_create -
 *
 * DESCRIPTION: initialize csi component.
 *==========================================================================*/
int CSI_comp_create()
{
  memset(&my_csi_struct, 0, sizeof(my_csi_struct));
  pthread_mutex_init (&my_csi_struct.mutex, NULL);
  return 0;
} /* CSI_comp_create */

/*===========================================================================
 * FUNCTION    - CSI_comp_destroy -
 *
 * DESCRIPTION: destroy csi component
 *==========================================================================*/
int CSI_comp_destroy()
{
  pthread_mutex_destroy(&my_csi_struct.mutex);
  memset(&my_csi_struct, 0, sizeof(my_csi_struct));
  return 0;
} /* CSI_comp_destroy */
