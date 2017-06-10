/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "camif.h"
#include "camif_interface.h"
#include "camera_dbg.h"

static camif_comp_root_t my_camif_struct;

#ifdef ENABLE_CAMIF_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

static uint32_t camif_gen_handle(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[4bits obj idx]|[4 bits client idx] */
  uint32_t handle = ((++my_camif_struct.camif_handle_cnt) << 8) +
    (0xff & client_idx);
  return handle;
}

static int camif_parse_handle(uint32_t handle, uint8_t *client_idx)
{
  int tmp = (handle & 0xff);
  *client_idx = tmp & 0xff;
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, handle, *client_idx);
  return 0;
}
/*===========================================================================
 * FUNCTION    - get_camif_client_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static camif_client_t *get_camif_client_info(uint32_t handle)
{
  uint8_t client_idx;

  camif_parse_handle(handle, &client_idx);
  if(client_idx >= AXI_MAX_CLIENT_NUM ||
    my_camif_struct.client[client_idx].handle != handle) {
    CDBG_ERROR("%s: client_idx = %d, local_handle = 0x%x, input_handle = 0x%x",
      __func__, client_idx, my_camif_struct.client[client_idx].handle, handle);
    return NULL;
  } else {
    return &(my_camif_struct.client[client_idx]);
  }
} /* get_camif_client_info */

static camif_obj_t *camif_get_obj(camif_comp_root_t *camif_root,
  camif_client_t *client)
{
  camif_obj_t *camif_obj = NULL;
  /* now we have not implemented the use case of using 2 CAMIF obj */
  if(client->obj_idx_mask == 1)
    camif_obj= &camif_root->camif_obj[0];
  else if(client->obj_idx_mask == 2)
    camif_obj= &camif_root->camif_obj[1];
  return camif_obj;
}

/*============================================================================
 * FUNCTION    - camif_interface_set_params -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int camif_client_set_params(uint32_t handle, int type, void *parm_in,
  void *parm_out)
{
  camif_input_t *input = parm_in;
  int rc = 0;
  camif_client_t *client;
  camif_obj_t *camif_obj = NULL;

  if(!input) {
    CDBG_ERROR("%s Input argument is NULL ", __func__);
    return -EINVAL;
  }
  CDBG("%s: type = %d, obj_idx = %d", __func__, type, input->obj_idx);
  pthread_mutex_lock(&my_camif_struct.mutex);
  client = get_camif_client_info(handle);
  if(!client) {
    CDBG_ERROR("%s Invalid handle passed ", __func__);
    pthread_mutex_unlock(&my_camif_struct.mutex);
    return -EINVAL;
  }
  camif_obj = &my_camif_struct.camif_obj[input->obj_idx];
  switch(type) {
  case CAMIF_PARAMS_ADD_OBJ_ID:
    if(client->obj_idx_mask > 0) {
      CDBG_ERROR("%s: camif has associated with obj mask 0x%x",
                 __func__, client->obj_idx_mask);
      rc = -1;
    } else {
      client->obj_idx_mask = (1 << input->obj_idx);
      camif_obj = &my_camif_struct.camif_obj[input->obj_idx];
      camif_obj->ref_count++;
      camif_obj->params.fd = client->ops->fd;
    }
    break;
  case CAMIF_PARAMS_HW_VERSION:
    client->vfe_version = input->d.vfe_version;
    break;
  case CAMIF_PARAMS_SENSOR_DIMENSION: {
    sensor_get_t sensor_get;
    rc = client->ops->fetch_params(
                      client->my_comp_id, client->ops->parent,
                      ((MCTL_COMPID_SENSOR << 24)|SENSOR_GET_CAMIF_CFG),
                      &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG_HIGH("%s: stats_proc fetch parms failed %d\n", __func__, rc);
      return rc;
    }
    camif_obj->params.sensor_dim.width = sensor_get.data.camif_setting.width;
    camif_obj->params.sensor_dim.height = sensor_get.data.camif_setting.height;
    break;
  }
  case CAMIF_PARAMS_SENSOR_CROP_WINDOW: {
    sensor_get_t sensor_get;
    rc = client->ops->fetch_params(
                      client->my_comp_id, client->ops->parent,
                      ((MCTL_COMPID_SENSOR << 24)|SENSOR_GET_CAMIF_CFG),
                      &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG_HIGH("%s: stats_proc fetch parms failed %d\n", __func__, rc);
      return rc;
    }
    camif_obj->params.sensor_crop_info.first_pixel =
      sensor_get.data.camif_setting.first_pixel;
    camif_obj->params.sensor_crop_info.last_pixel =
      sensor_get.data.camif_setting.last_pixel;
    camif_obj->params.sensor_crop_info.first_line =
      sensor_get.data.camif_setting.first_line;
    camif_obj->params.sensor_crop_info.last_line =
      sensor_get.data.camif_setting.last_line;
    /*update camif window*/
    camif_obj->params.camif_window.width =
      (camif_obj->params.sensor_crop_info.last_pixel -
      camif_obj->params.sensor_crop_info.first_pixel) + 1;
    camif_obj->params.camif_window.height =
      (camif_obj->params.sensor_crop_info.last_line -
       camif_obj->params.sensor_crop_info.first_line) + 1;
    camif_obj->params.camif_window.first_line =
      camif_obj->params.sensor_crop_info.first_line;
    camif_obj->params.camif_window.last_line =
      camif_obj->params.sensor_crop_info.last_line;
    camif_obj->params.camif_window.first_pixel =
      camif_obj->params.sensor_crop_info.first_pixel;
    camif_obj->params.camif_window.last_pixel =
      camif_obj->params.sensor_crop_info.last_pixel;
    break;
  }
  case CAMIF_PARAMS_MODE:
    camif_obj->params.mode = input->d.mode;
    break;
  case CAMIF_PARAMS_SENSOR_FORMAT:
    camif_obj->params.format = input->d.format;
    break;
  case CAMIF_PARAMS_STROBE_INFO: {
    stats_proc_get_t sp_get;
    sp_get.d.get_aec.type = AEC_FLASH_DATA;
    rc = client->ops->fetch_params(
                         client->my_comp_id, client->ops->parent,
                         ((MCTL_COMPID_STATSPROC << 24)|STATS_PROC_AEC_TYPE),
                         &sp_get, sizeof(sp_get));
    if (rc < 0) {
      CDBG_HIGH("%s: stats_proc fetch parms failed %d\n", __func__, rc);
      return rc;
    }
    camif_obj->params.strobe_info.enabled =
      (sp_get.d.get_aec.d.flash_parms.flash_mode == CAMERA_FLASH_STROBE);
    camif_obj->params.strobe_info.duration =
      sp_get.d.get_aec.d.flash_parms.strobe_duration;
    break;
  }
  case CAMIF_PARAMS_CONNECTION_MODE:
    camif_obj->params.connection_mode = input->d.connection_mode;
    break;
  default:
    CDBG_ERROR("%s: invalid parameter %d", __func__, type);
    rc = -1;
    break;
  }
  pthread_mutex_unlock(&my_camif_struct.mutex);
  return rc;
}/*camif_interface_set_params*/

/*============================================================================
 * FUNCTION    - camif_interface_get_params -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int camif_client_get_params(uint32_t handle, int type, void *parm, int parm_len)
{
  camif_output_t *output = parm;
  camif_client_t *client;
  camif_obj_t *camif_obj = NULL;
  int rc = 0;

  CDBG("%s: type %d", __func__, type);
  if(!output) {
    CDBG_ERROR("%s Input argument is NULL ", __func__);
    return -EINVAL;
  }
  pthread_mutex_lock(&my_camif_struct.mutex);
  client = get_camif_client_info(handle);
  if(!client) {
    CDBG_ERROR("%s Invalid handle passed ", __func__);
    pthread_mutex_unlock(&my_camif_struct.mutex);
    return -EINVAL;
  }
  camif_obj = camif_get_obj(&my_camif_struct, client);
  if (NULL == camif_obj) {
    CDBG_ERROR("%s Invalid camif_obj", __func__);
    pthread_mutex_unlock(&my_camif_struct.mutex);
    return -EINVAL;
  }
  switch(type) {
  case CAMIF_PARAMS_CAMIF_DIMENSION:
    output->d.camif_window = camif_obj->params.camif_window;
    break;
  default:
    CDBG_ERROR("%s: invalid parameter %d", __func__, type);
    rc = -1;
    break;
  }
  pthread_mutex_unlock(&my_camif_struct.mutex);
  return rc;
}/*camif_client_get_params*/

/*============================================================================
 * FUNCTION    - camif_interface_process -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int camif_client_process(uint32_t handle, int event, void *data)
{
  camif_client_t *client;
  camif_obj_t *camif_obj = NULL;
  int rc = 0;
  camif_ops_t ops = (camif_ops_t)event;

  CDBG("%s: ops %d", __func__, ops);
  pthread_mutex_lock(&my_camif_struct.mutex);
  client = get_camif_client_info(handle);
  if(!client) {
    CDBG_ERROR("%s Invalid handle passed ", __func__);
    pthread_mutex_unlock(&my_camif_struct.mutex);
    return -EINVAL;
  }
  camif_obj = camif_get_obj(&my_camif_struct, client);
  if (NULL == camif_obj) {
    CDBG_ERROR("%s Invalid camif_obj", __func__);
    pthread_mutex_unlock(&my_camif_struct.mutex);
    return -EINVAL;
  }
  CDBG("%s: ops %d", __func__, ops);
  switch(ops) {
  case CAMIF_MODULE_INIT:
    rc = camif_init(camif_obj);
    break;
  case CAMIF_OPS_CONFIG:
    rc = camif_config(camif_obj);
    break;
  case CAMIF_OPS_TIMER_CONFIG:
    rc = camif_timer_config(camif_obj);
    break;
  case CAMIF_PROC_CMD_OPS:
    rc = camif_command_ops(camif_obj, data);
    break;
  default:
    CDBG_ERROR("%s: invalid ops %d", __func__, ops);
    rc = -1;
    break;
  }
  pthread_mutex_unlock(&my_camif_struct.mutex);
  return rc;
}/*camif_client_process*/

/*============================================================================
 * FUNCTION    - camif_interface_destroy -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int camif_client_destroy(uint32_t handle)
{
  camif_client_t *client;
  camif_obj_t *camif_obj = NULL;

  CDBG("%s: handle = 0x%x", __func__, handle);
  pthread_mutex_lock(&my_camif_struct.mutex);
  client = get_camif_client_info(handle);
  if(!client) {
    CDBG_ERROR("%s Invalid handle passed ", __func__);
    pthread_mutex_unlock(&my_camif_struct.mutex);
    return 0;
  }
  camif_obj = camif_get_obj(&my_camif_struct, client);
  if(!camif_obj) {
    CDBG_ERROR("%s camif_obj = NULL ", __func__);
    pthread_mutex_unlock(&my_camif_struct.mutex);
    return 0;
  }
  camif_obj->ref_count--;
  if(!camif_obj->ref_count)
  memset(camif_obj, 0, sizeof(camif_obj_t));
  memset(client, 0, sizeof(camif_client_t));
  pthread_mutex_unlock(&my_camif_struct.mutex);
  return 0;
}/*camif_client_destroy*/
/*============================================================================
 * FUNCTION    - camif_interface_init -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int camif_client_init(uint32_t handle, mctl_ops_t *ops, void *init_data)
{
  camif_client_t *client;

  pthread_mutex_lock(&my_camif_struct.mutex);
  client = get_camif_client_info(handle);
  if(!client) {
    CDBG_ERROR("%s Invalid handle 0x%x passed ", __func__, handle);
    pthread_mutex_unlock(&my_camif_struct.mutex);
    return -EINVAL;
  }
  /* init_data is not used now. Reserved */
  if(!ops) {
    CDBG_ERROR("%s Invalid handle passed %d ", __func__, handle);
    pthread_mutex_unlock(&my_camif_struct.mutex);
    return -EINVAL;
  }
  if (ops->fd < 0) {
    CDBG_ERROR("%s Invalid argument camera fd %d ", __func__, ops->fd);
    pthread_mutex_unlock(&my_camif_struct.mutex);
    return -EINVAL;
  }
  client->ops = ops;
  pthread_mutex_unlock(&my_camif_struct.mutex);
  return 0;
}/*camif_interface_init*/
/*============================================================================
 * FUNCTION    - camif_interface_create -
 *
 * DESCRIPTION:
 *===========================================================================*/
uint32_t CAMIF_client_open(module_ops_t *ops)
{
  uint32_t handle = 0;
  int idx;
  camif_client_t *camif_client = NULL;

  if(!ops) {
    CDBG_ERROR("%s: null ops pointer",  __func__);
    return 0;
  }
  memset(ops,  0,  sizeof(module_ops_t));

  pthread_mutex_lock(&my_camif_struct.mutex);
  for(idx = 0; idx < CAMIF_MAX_CLIENT_NUM; idx++) {
    if(my_camif_struct.client[idx].handle == 0) {
      camif_client = &my_camif_struct.client[idx];
      break;
    }
  }
  /* if not found return null */
  if (!camif_client) {
    pthread_mutex_unlock(&my_camif_struct.mutex);
    return (uint32_t)NULL;
  } else {
    memset(camif_client, 0, sizeof(camif_client_t));
    camif_client->obj_idx_mask = 0;
    camif_client->client_idx = idx;
    camif_client->my_comp_id = MCTL_COMPID_CAMIF;
    camif_client->handle = camif_gen_handle((uint8_t)idx);
    ops->handle = (uint32_t)camif_client->handle;
    ops->init = camif_client_init;
    ops->set_params = camif_client_set_params;
    ops->get_params = camif_client_get_params;
    ops->process = camif_client_process;
    ops->abort = NULL;
    ops->destroy= camif_client_destroy;
  }
  pthread_mutex_unlock(&my_camif_struct.mutex);
  CDBG("%s: client_idx = %d, handle = 0x%x", __func__, idx,
    camif_client->handle);

  return camif_client->handle;
}/*camif_interface_create*/

/*===========================================================================
 * FUNCTION    - CAMIF_comp_create -
 *
 * DESCRIPTION: initialize AXI component.
 *==========================================================================*/
int CAMIF_comp_create()
{
  memset(&my_camif_struct, 0, sizeof(my_camif_struct));
  pthread_mutex_init(&my_camif_struct.mutex, NULL);
  return 0;
}
/*===========================================================================
 * FUNCTION    - CAMIF_comp_destroy -
 *
 * DESCRIPTION: destroy the component
 *==========================================================================*/
int CAMIF_comp_destroy()
{
  pthread_mutex_destroy(&my_camif_struct.mutex);
  memset(&my_camif_struct, 0, sizeof(my_camif_struct));
  return 0;
}
