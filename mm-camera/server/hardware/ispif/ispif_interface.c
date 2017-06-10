/*==========================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
#include "ispif.h"
#include "camera_dbg.h"

static ispif_comp_root_t my_ispif_struct;

#if ISPIF_DEBUG
#undef CDBG
#define CDBG LOGE
#endif
/*===========================================================================
 * FUNCTION    - ispif_get_obj -
 *
 * DESCRIPTION:
 *==========================================================================*/
static ispif_t *ispif_get_obj(ispif_comp_root_t *ispif_root,
  ispif_client_t *ispif_client)
{
  ispif_t *ispif_obj = NULL;
  /* now we have not implemented the use case of using 2 VFE obj */
  if (ispif_client->obj_idx_mask == 1)
    ispif_obj = &ispif_root->ispif_obj[0];
  else if (ispif_client->obj_idx_mask == 2)
    ispif_obj = &ispif_root->ispif_obj[1];
  return ispif_obj;
}

/*===========================================================================
 * FUNCTION    - ispif_client_add_obj -
 *
 * DESCRIPTION:
 *==========================================================================*/

static int ispif_client_add_obj(ispif_client_t *ispif_client, int type,
  void *parm_in, void *parm_out)
{
  ispif_t *ispif_obj;
  int i = 0;
  int ispif_obj_id = *((int *)parm_in);

  if (ispif_client->obj_idx_mask > 0) {
    CDBG("%s: error enable second obj",  __func__);
    return -1;
  }
  ispif_client->obj_idx_mask |= (1 << ispif_obj_id);
  ispif_obj = &my_ispif_struct.ispif_obj[ispif_obj_id];

  if (!ispif_obj->ref_count) {
    csi_get_t csi_get;
    ispif_obj->session_lock_owner = -1;

    for (i = 0; i < ISPIF_MAX_OBJ; i++)
      pthread_mutex_init(&ispif_obj->session_mutex[i], NULL);

    ispif_client->ops->fetch_params(ispif_client->my_comp_id,
      ispif_client->ops->parent,
      ((MCTL_COMPID_CSI << 24) | CSI_GET_CSID_VERSION),
      &csi_get, sizeof(csi_get));
    ispif_obj->csid_version = csi_get.csid_version;

    CDBG("%s get csid version %x\n", __func__, ispif_obj->csid_version);
    ispif_process_init(ispif_client,
      ispif_obj);
  }
  ispif_obj->ref_count++;
  return 0;
}

/*===========================================================================
 * FUNCTION    - ispif_gen_handle -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint32_t ispif_gen_handle(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[4bits obj idx]|[4 bits client idx] */
  uint32_t handle = ((++my_ispif_struct.ispif_handle_cnt) << 8) +
                    (0xff & client_idx);
  return handle;
} /* ispif_gen_handle */


/*===========================================================================
 * FUNCTION    - ispif_parse_handle -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int ispif_parse_handle(uint32_t handle, uint8_t *client_idx)
{
  int tmp = (handle & 0xff);
  *client_idx = tmp & 0xff;
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, handle, *client_idx);
  return 0;
} /* ispif_parse_handle */

/*===========================================================================
 * FUNCTION    - get_ispif_client_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static ispif_client_t *get_ispif_client_info(uint32_t handle)
{
  uint8_t client_idx;

  ispif_parse_handle(handle, &client_idx);
  if ((client_idx >= ISPIF_MAX_CLIENT_NUM) ||
      (my_ispif_struct.client[client_idx].handle != handle)) {
    CDBG_ERROR("%s: client_idx = %d, local_handle = 0x%x, input_handle = 0x%x",
                __func__, client_idx,
                my_ispif_struct.client[client_idx].handle,
                handle);
    return NULL;
  } else
    return &(my_ispif_struct.client[client_idx]);
} /* get_ispif_client_info */

/*===========================================================================
 * FUNCTION    - ispif_client_get_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int ispif_client_get_params(uint32_t handle, int parm_type,
                                 void *parm, int parm_len)
{
  int rc = 0;

  ispif_client_t *ispif_client = NULL;
  ispif_get_t *ispif_get = NULL;
  ispif_t *ispif_obj = NULL;

  pthread_mutex_lock(&my_ispif_struct.mutex);

  CDBG("%s: parm type =%d\n", __func__, parm_type);

  ispif_client = get_ispif_client_info(handle);
  if (!ispif_client) {
    CDBG_ERROR("%s: null ispif client\n", __func__);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -1;
  }

  ispif_obj = ispif_get_obj(&my_ispif_struct, ispif_client);
  if (!ispif_obj) {
    CDBG_ERROR("%s: no ISPIF OBJ associated with client",  __func__);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -1;
  }

  ispif_get = (ispif_get_t *)parm;
  if (!ispif_get) {
    CDBG_ERROR("%s: null ispif get\n", __func__);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -1;
  }

  switch (parm_type) {
  case ISPIF_GET_CHANNEL_INFO: {
    ispif_get->data.channel_interface_mask =
      ispif_client->channel_interface_mask;
    ispif_get->data.channel_stream_info =
      ispif_client->channel_stream_info;
    break;
  }
  default:
    rc = -EINVAL;
  }
  pthread_mutex_unlock(&my_ispif_struct.mutex);
  return rc;
}

/*==========================================================
 * FUNCTION    - ispif_client_set_params -
 *
 * DESCRIPTION:
 *==========================================================*/
static int ispif_client_set_params(uint32_t handle, int type, void *parm_in,
  void *parm_out)
{
  int rc = 0;
  ispif_client_t *ispif_client = NULL;
  ispif_set_t *ispif_set = NULL;
  ispif_t *ispif_obj = NULL;

  pthread_mutex_lock(&my_ispif_struct.mutex);

  CDBG("%s: parm type =%d\n", __func__, type);

  ispif_client = get_ispif_client_info(handle);
  if (!ispif_client) {
    CDBG_ERROR("%s: null ispif client\n", __func__);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -1;
  }

  if (type == ISPIF_PARM_ADD_OBJ_ID) {
    rc = ispif_client_add_obj(ispif_client, type, parm_in, parm_out);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return rc;
  }

  ispif_obj = ispif_get_obj(&my_ispif_struct, ispif_client);
  if (!ispif_obj) {
    CDBG_ERROR("%s: no ISPIF OBJ associated with client",  __func__);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -1;
  }

  ispif_set = (ispif_set_t *)parm_in;
  if (!ispif_set) {
    CDBG_ERROR("%s: null ispif set\n", __func__);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -1;
  }

  switch (type) {
  case ISPIF_SESSION_LOCK:
    if (ispif_set->data.session_lock.acquire) {
      if (ispif_obj->session_lock_owner != ispif_client->client_idx) {
        pthread_mutex_unlock(&my_ispif_struct.mutex);
        CDBG("before lock ispif client idx %d session mutex = %p\n",ispif_client->client_idx,
            (void *)&(ispif_obj->session_mutex[ispif_set->data.session_lock.vfe_id]));
        pthread_mutex_lock(&ispif_obj->session_mutex[
            ispif_set->data.session_lock.vfe_id]);
        CDBG("after lock ispif client idx %d session mutex = %p\n",ispif_client->client_idx,
             (void *)&(ispif_obj->session_mutex[ispif_set->data.session_lock.vfe_id]));
        ispif_obj->session_lock_owner = ispif_client->client_idx;
        pthread_mutex_lock(&my_ispif_struct.mutex);
      }
    } else {
      if (ispif_obj->session_lock_owner == ispif_client->client_idx) {
        CDBG(" before unlock ispif client idx %d session mutex = %p\n",ispif_client->client_idx,
            (void *)&(ispif_obj->session_mutex[ispif_set->data.session_lock.vfe_id]));
        ispif_obj->session_lock_owner = -1;
        pthread_mutex_unlock(&ispif_obj->session_mutex[
            ispif_set->data.session_lock.vfe_id]);
        CDBG("after unlock ispif client idx %d session mutex = %p\n",ispif_client->client_idx,
            (void *)&(ispif_obj->session_mutex[ispif_set->data.session_lock.vfe_id]));
      }
    }
    break;
  case ISPIF_SET_INTF_PARAMS: {
    sensor_get_t sensor_get;
    int intftype;
    struct msm_ispif_params_list ispif_params_list;
    int len = 0, i = 0, index = 0x1;
    uint32_t intf_mask = ispif_client->channel_interface_mask;
    uint32_t stream_mask = ispif_set->data.channel_stream_info;
    rc = ispif_client->ops->fetch_params(
                         ispif_client->my_comp_id, ispif_client->ops->parent,
                         ((MCTL_COMPID_SENSOR << 24)|SENSOR_GET_CSI_PARAMS),
                         &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG("%s: ispif failed %d\n", __func__, rc);
      return rc;
    }
    CDBG("%s: channel stream mask %d\n", __func__,
      ispif_client->channel_interface_mask);
    while (intf_mask != 0) {
      switch (ispif_client->channel_interface_mask & index) {
      case PIX_0:
        ispif_obj->ispif_ctrl[PIX0].used = 0;
        ispif_obj->ispif_ctrl[PIX0].client_idx = 0;
        break;
      case RDI_0:
        ispif_obj->ispif_ctrl[RDI0].used = 0;
        ispif_obj->ispif_ctrl[RDI0].client_idx = 0;
        break;
      case RDI_1:
        ispif_obj->ispif_ctrl[RDI1].used = 0;
        ispif_obj->ispif_ctrl[RDI1].client_idx = 0;
        break;
      case RDI_2:
        ispif_obj->ispif_ctrl[RDI2].used = 0;
        ispif_obj->ispif_ctrl[RDI2].client_idx = 0;
        break;
      default:
        break;
      }
      index <<= 1;
      intf_mask >>= 1;
    }
    index = 0x1;
    intf_mask =
      ispif_client->channel_interface_mask;
    ispif_client->channel_interface_mask = 0;
    while (stream_mask != 0) {
        switch (ispif_set->data.channel_stream_info & index) {
        case STREAM_RAW:
          if (ispif_obj->ispif_ctrl[RDI0].used == 0) {
            ispif_params_list.params[len++].intftype = RDI0;
            ispif_client->channel_interface_mask |= RDI_0;
          } else
            return -1;
          ispif_client->channel_stream_info |= STREAM_RAW;
          break;
        case STREAM_RAW1:
          if (ispif_obj->ispif_ctrl[RDI1].used == 0) {
            ispif_params_list.params[len++].intftype = RDI1;
            ispif_client->channel_interface_mask |= RDI_1;
          } else
            return -1;
          ispif_client->channel_stream_info |= STREAM_RAW1;
          break;
        case STREAM_RAW2:
          if (ispif_obj->ispif_ctrl[RDI2].used == 0) {
            ispif_params_list.params[len++].intftype = RDI2;
            ispif_client->channel_interface_mask |= RDI_2;
          } else
            return -1;
          ispif_client->channel_stream_info |= STREAM_RAW2;
          break;
        case STREAM_IMAGE:
          if (ispif_obj->ispif_ctrl[PIX0].used == 0) {
            ispif_params_list.params[len++].intftype = PIX0;
            ispif_client->channel_interface_mask |= PIX_0;
            ispif_client->channel_stream_info |= STREAM_IMAGE;
          }
          break;
        default:
          break;
        }
      index <<= 1;
      stream_mask >>= 1;
    }
    ispif_client->stream_mask = ispif_client->channel_interface_mask;

    for (i = 0; i < len; i++) {
      intftype = ispif_params_list.params[i].intftype;
      ispif_obj->ispif_ctrl[intftype].ispif_params.intftype =
        intftype;
      ispif_obj->ispif_ctrl[intftype].ispif_params.csid =
        sensor_get.data.sensor_csi_params->csi_lane_params.csid_core[0];
      ispif_obj->ispif_ctrl[intftype].ispif_params.cid_mask =
        (1 << 0);
      ispif_obj->ispif_ctrl[intftype].ispif_params.vfe_intf =
        ispif_set->data.vfe_interface;
      ispif_obj->ispif_ctrl[intftype].client_idx =
      ispif_client->client_idx;
      if (intf_mask != ispif_client->channel_interface_mask)
        ispif_obj->ispif_ctrl[intftype].pending = 1;
      ispif_obj->ispif_ctrl[intftype].used = 1;
    }
    break;
  }
  default:
    rc = -EINVAL;
    break;
  }

  pthread_mutex_unlock(&my_ispif_struct.mutex);

  return rc;
} /* ispif_client_set_params */

/*==========================================================
 * FUNCTION    - ispif_client_process -
 *
 * DESCRIPTION:
 *==========================================================*/
static int ispif_client_process(uint32_t handle, int event, void *data)
{
  int rc = 0;
  ispif_client_t *ispif_client = NULL;
  ispif_process_t *ispif_process = NULL;
  ispif_t *ispif_obj = NULL;

  pthread_mutex_lock(&my_ispif_struct.mutex);

  CDBG("%s: event type =%d\n", __func__, event);

  ispif_client = get_ispif_client_info(handle);
  if (!ispif_client) {
    CDBG_ERROR("%s: null ispif client\n", __func__);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -1;
  }

  ispif_obj = ispif_get_obj(&my_ispif_struct, ispif_client);
  if (!ispif_obj) {
    CDBG_ERROR("%s: no ISPIF OBJ associated with client",  __func__);
	pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -1;
  }

  ispif_process = (ispif_process_t *)data;

  switch (event) {
  case ISPIF_PROCESS_INIT: {
    csi_get_t csi_get;
    ispif_client->ops->fetch_params(ispif_client->my_comp_id,
      ispif_client->ops->parent,
      ((MCTL_COMPID_CSI << 24) | CSI_GET_CSID_VERSION),
      &csi_get, sizeof(csi_get));
    ispif_obj->csid_version = csi_get.csid_version;
    CDBG("%s get csid version %x\n", __func__, ispif_obj->csid_version);
    ispif_process_init(ispif_client,
      ispif_obj);
    }
    break;
  case ISPIF_PROCESS_CFG:
    ispif_process_cfg(ispif_client,
      ispif_obj);
    break;

  case ISPIF_PROCESS_START_ON_FRAME_BOUNDARY:
    ispif_process_start_on_frame_boundary(ispif_client,
      ispif_obj);
    break;

  case ISPIF_PROCESS_STOP_ON_FRAME_BOUNDARY:
    ispif_process_stop_on_frame_boundary(ispif_client,
      ispif_obj);
    break;

  case ISPIF_PROCESS_STOP_IMMEDIATELY:
    ispif_process_stop_immediately(ispif_client,
      ispif_obj);
    break;

  case ISPIF_PROCESS_RELEASE:
    ispif_process_release(ispif_client,
      ispif_obj);
    break;

  default:
    rc = -EINVAL;
  }
  pthread_mutex_unlock(&my_ispif_struct.mutex);
  return rc;
} /* ispif_client_set_params */


/*============================================================================
 * FUNCTION    - ispif_interface_destroy -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int ispif_client_destroy(uint32_t handle)
{
  ispif_client_t *client;
  int idx, i = 0;
  ispif_t *ispif_obj = NULL;

  CDBG("%s: handle = 0x%x", __func__, handle);
  pthread_mutex_lock(&my_ispif_struct.mutex);

  client = get_ispif_client_info(handle);
  if (!client) {
    CDBG_ERROR("%s Invalid handle passed ", __func__);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -1;
  }

  ispif_obj = ispif_get_obj(&my_ispif_struct, client);
  if (!ispif_obj) {
    CDBG_ERROR("%s: no ISPIF OBJ associated with client",  __func__);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -1;
  }

  for (idx = 0; idx < INTF_MAX; idx++) {
    if (ispif_obj->ispif_ctrl[idx].used &&
      ispif_obj->ispif_ctrl[idx].client_idx == client->client_idx) {
      memset(&ispif_obj->ispif_ctrl[idx], 0,  sizeof(ispif_ctrl_t));
    }
  }

  ispif_obj->ref_count--;
  if (!ispif_obj->ref_count) {
    ispif_process_release(client,
      ispif_obj);

    for (i = 0; i < ISPIF_MAX_OBJ; i++)
      pthread_mutex_destroy(&ispif_obj->session_mutex[i]);
    memset(ispif_obj, 0, sizeof(ispif_t));
  }
  memset(client, 0, sizeof(ispif_client_t));
  pthread_mutex_unlock(&my_ispif_struct.mutex);
  return 0;
} /*ispif_client_destroy*/

/*============================================================================
 * FUNCTION    - ispif_interface_init -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int ispif_client_init(uint32_t handle, mctl_ops_t *ops,
  void *init_data)
{
  ispif_client_t *client;

  pthread_mutex_lock(&my_ispif_struct.mutex);

  client = get_ispif_client_info(handle);
  if (!client) {
    CDBG_ERROR("%s Invalid handle 0x%x passed ", __func__, handle);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -EINVAL;
  }

  /* init_data is not used now. Reserved */
  if (!ops) {
    CDBG_ERROR("%s Invalid handle passed %d ", __func__, handle);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -EINVAL;
  }

  if (ops->fd < 0) {
    CDBG_ERROR("%s Invalid argument camera fd %d ", __func__, ops->fd);
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return -EINVAL;
  }

  client->ops = ops;
  CDBG("%s ops->fd = %d\n", __func__, ops->fd);
  pthread_mutex_unlock(&my_ispif_struct.mutex);
  return 0;
} /*ispif_interface_init*/

/*============================================================================
 * FUNCTION    - ISPIF_client_open -
 *
 * DESCRIPTION:
 *===========================================================================*/
uint32_t ISPIF_client_open(module_ops_t *ops)
{
  uint32_t handle = 0;
  int idx = 0;
  ispif_client_t *ispif_client = NULL;

  if (!ops) {
    CDBG_ERROR("%s: null ops pointer", __func__);
    return 0;
  }

  memset(ops, 0, sizeof(module_ops_t));

  pthread_mutex_lock(&my_ispif_struct.mutex);

  for (idx = 0; idx < ISPIF_MAX_CLIENT_NUM; idx++) {
    if (my_ispif_struct.client[idx].handle == 0) {
      ispif_client = &my_ispif_struct.client[idx];
      break;
    }
  }

  /* if not found return null */
  if (!ispif_client) {
    pthread_mutex_unlock(&my_ispif_struct.mutex);
    return (uint32_t)NULL;
  } else {
    memset(ispif_client, 0, sizeof(ispif_client_t));
    ispif_client->client_idx = idx;
	ispif_client->my_comp_id = MCTL_COMPID_ISPIF;
    ispif_client->handle = ispif_gen_handle((uint8_t)idx);
    ops->handle = (uint32_t)ispif_client->handle;
    ops->init = ispif_client_init;
    ops->set_params = ispif_client_set_params;
    ops->get_params = ispif_client_get_params;
    ops->process = ispif_client_process;
    ops->abort = NULL;
    ops->destroy = ispif_client_destroy;
  }

  pthread_mutex_unlock(&my_ispif_struct.mutex);

  CDBG("%s: client_idx = %d, handle = 0x%x",
        __func__, idx, ispif_client->handle);

  return ispif_client->handle;
} /*ISPIF_client_open */

/*===========================================================================
 * FUNCTION    - ISPIF_comp_create -
 *
 * DESCRIPTION: initialize ispif component.
 *==========================================================================*/
int ISPIF_comp_create()
{
  memset(&my_ispif_struct, 0, sizeof(my_ispif_struct));
  pthread_mutex_init(&my_ispif_struct.mutex, NULL);
  return 0;
} /* ISPIF_comp_create */

/*===========================================================================
 * FUNCTION    - ISPIF_comp_destroy -
 *
 * DESCRIPTION: destroy ispif component
 *==========================================================================*/
int ISPIF_comp_destroy()
{
  pthread_mutex_destroy(&my_ispif_struct.mutex);
  memset(&my_ispif_struct, 0, sizeof(my_ispif_struct));
  return 0;
} /* ISPIF_comp_destroy */

