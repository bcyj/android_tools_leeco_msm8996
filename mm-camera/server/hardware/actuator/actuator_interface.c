/*==========================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
#include <string.h>
#include <inttypes.h>
#include <sys/time.h>
#include "af_tuning.h"
#include "actuator.h"
#include "camera_dbg.h"
#include "af_tuning.h"

static actuator_comp_root_t my_actuator_struct;

/*===========================================================================
 * FUNCTION    - actuator_gen_handle -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint32_t actuator_gen_handle(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[4bits obj idx]|[4 bits client idx] */
  uint32_t handle = ((++my_actuator_struct.actuator_handle_cnt) << 8) +
                    (0xff & client_idx);
  return handle;
} /* actuator_gen_handle */

/*===========================================================================
 * FUNCTION    - actuator_parse_handle -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int actuator_parse_handle(uint32_t handle, uint8_t *client_idx)
{
  int tmp = (handle & 0xff);
  *client_idx = tmp & 0xff;
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, handle, *client_idx);
  return 0;
} /* actuator_parse_handle */

/*===========================================================================
 * FUNCTION    - get_actuator_client_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static actuator_client_t *get_actuator_client_info(uint32_t handle)
{
  uint8_t client_idx;

  actuator_parse_handle(handle, &client_idx);
  if ((client_idx >= ACTUATOR_MAX_CLIENT_NUM) ||
      (my_actuator_struct.client[client_idx].handle != handle)) {
    CDBG_ERROR("%s: client_idx = %d, local_handle = 0x%x, input_handle = 0x%x",
                __func__, client_idx,
                my_actuator_struct.client[client_idx].handle,
                handle);
    return NULL;
  } else
    return &(my_actuator_struct.client[client_idx]);
} /* get_actuator_client_info */

/*==========================================================
 * FUNCTION    - actuator_client_set_params -
 *
 * DESCRIPTION:
 *==========================================================*/
static int actuator_client_set_params(uint32_t handle, int type, void *parm_in,
  void *parm_out)
{
  int rc = 0;
  actuator_client_t *actuator_client = NULL;
  actuator_set_t *actuator_set = NULL;

  pthread_mutex_lock(&my_actuator_struct.mutex);

  CDBG("%s: parm type =%d\n", __func__, type);

  actuator_client = get_actuator_client_info(handle);
  if(!actuator_client) {
    CDBG_ERROR("%s: null actuator client\n", __func__);
    pthread_mutex_unlock(&my_actuator_struct.mutex);
    return -1;
  }

  actuator_set = (actuator_set_t *)parm_in;

  switch(type) {
  case ACTUATOR_MOVE_FOCUS:
    rc = af_actuator_move_focus(&actuator_client->actuator_ctrl_obj,
           actuator_set->data.move.direction,
           actuator_set->data.move.num_steps);
    break;
  case ACTUATOR_DEF_FOCUS:
    rc = af_actuator_set_default_focus(&actuator_client->actuator_ctrl_obj, 0);
    break;
  case ACTUATOR_RESTORE_FOCUS:
    rc = af_actuator_restore_focus(&actuator_client->actuator_ctrl_obj,
      actuator_set->data.move.direction);
    break;
  case ACTUATOR_LOAD_PARAMS:{
    struct msm_actuator_cfg_data cfg;
    rc = af_actuator_load_params(&actuator_client->actuator_ctrl_obj, &cfg);
    break;
    }
  case ACTUATOR_TEST_RING:
    af_actuator_ring_test(&actuator_client->actuator_ctrl_obj,
      actuator_set->data.test.stepsize);
    break;
  case ACTUATOR_TEST_LINEAR:
    af_actuator_linear_test(&actuator_client->actuator_ctrl_obj,
      actuator_set->data.test.stepsize);
    break;

  default:
    rc = -EINVAL;
  }

  pthread_mutex_unlock(&my_actuator_struct.mutex);

  return rc;
} /* actuator_client_set_params */

/*==========================================================
 * FUNCTION    - actuator_client_get_params -
 *
 * DESCRIPTION:
 *==========================================================*/
static int actuator_client_get_params(uint32_t handle, int type,
                                      void *parm, int parm_len)
{
  int rc = 0;
  actuator_client_t *actuator_client = NULL;
  actuator_get_t *actuator_get = NULL;

  pthread_mutex_lock(&my_actuator_struct.mutex);

  CDBG("%s: parm type =%d\n", __func__, type);

  actuator_client = get_actuator_client_info(handle);
  if(!actuator_client) {
    CDBG_ERROR("%s: null actuator client\n", __func__);
    pthread_mutex_unlock(&my_actuator_struct.mutex);
    return -1;
  }

  actuator_get = (actuator_get_t *)parm;

  switch(type) {
  case ACTUATOR_GET_INFO:
    af_actuator_get_info(&actuator_client->actuator_ctrl_obj,
      &(actuator_get->data));
    break;
  case ACTUATOR_GET_AF_TUNE_PTR:
    actuator_get->data.af_tune_ptr=
      &(actuator_client->actuator_ctrl_obj.ctrl->af_tune);
    break;

  default:
    rc = -EINVAL;
  }

  pthread_mutex_unlock(&my_actuator_struct.mutex);

  return rc;
} /* actuator_client_get_params */

/*============================================================================
 * FUNCTION    - actuator_interface_destroy -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int actuator_client_destroy(uint32_t handle)
{
  actuator_client_t *client;

  CDBG("%s: handle = 0x%x", __func__, handle);
  pthread_mutex_lock(&my_actuator_struct.mutex);

  client = get_actuator_client_info(handle);
  if(!client) {
    CDBG_ERROR("%s Invalid handle passed ", __func__);
    pthread_mutex_unlock(&my_actuator_struct.mutex);
    return 0;
  }
  af_actuator_set_default_focus(&client->actuator_ctrl_obj, 0);

  memset(client, 0, sizeof(actuator_client_t));
  pthread_mutex_unlock(&my_actuator_struct.mutex);
  return 0;
} /*actuator_client_destroy*/

/*============================================================================
 * FUNCTION    - actuator_interface_init -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int actuator_client_init(uint32_t handle, mctl_ops_t *ops,
  void *init_data)
{
  actuator_client_t *client;
  int rc = 0;

  pthread_mutex_lock(&my_actuator_struct.mutex);

  client = get_actuator_client_info(handle);
  if(!client) {
    CDBG_ERROR("%s Invalid handle 0x%x passed ", __func__, handle);
    pthread_mutex_unlock(&my_actuator_struct.mutex);
    return -EINVAL;
  }

  /* init_data is not used now. Reserved */
  if(!ops) {
    CDBG_ERROR("%s Invalid handle passed %d ", __func__, handle);
    pthread_mutex_unlock(&my_actuator_struct.mutex);
    return -EINVAL;
  }

  if (ops->fd < 0) {
    CDBG_ERROR("%s Invalid argument camera fd %d ", __func__, ops->fd);
    pthread_mutex_unlock(&my_actuator_struct.mutex);
    return -EINVAL;
  }

  client->ops = ops;
  client->actuator_ctrl_obj.fd = ops->fd;
  CDBG("%s ops->fd = %d\n", __func__, ops->fd);
  rc = af_actuator_init(&client->actuator_ctrl_obj);
  pthread_mutex_unlock(&my_actuator_struct.mutex);
  return rc;
} /*actuator_interface_init*/

/*============================================================================
 * FUNCTION    - ACTUATOR_client_open -
 *
 * DESCRIPTION:
 *===========================================================================*/
uint32_t ACTUATOR_client_open(module_ops_t *ops)
{
  uint32_t handle = 0;
  int idx = 0;
  actuator_client_t *actuator_client = NULL;

  if (!ops) {
    CDBG_ERROR("%s: null ops pointer", __func__);
    return 0;
  }

  memset(ops, 0, sizeof(module_ops_t));

  pthread_mutex_lock(&my_actuator_struct.mutex);

  for (idx = 0; idx < ACTUATOR_MAX_CLIENT_NUM; idx++) {
    if (my_actuator_struct.client[idx].handle == 0) {
      actuator_client = &my_actuator_struct.client[idx];
      break;
    }
  }

  /* if not found return null */
  if (!actuator_client) {
    pthread_mutex_unlock(&my_actuator_struct.mutex);
    return (uint32_t)NULL;
  } else {
    memset(actuator_client, 0, sizeof(actuator_client_t));
    actuator_client->client_idx = idx;
    actuator_client->my_comp_id = MCTL_COMPID_ACTUATOR;
    actuator_client->handle = actuator_gen_handle((uint8_t)idx);
    ops->handle = (uint32_t)actuator_client->handle;
    ops->init = actuator_client_init;
    ops->set_params = actuator_client_set_params;
    ops->get_params = actuator_client_get_params;
    ops->process = NULL;
    ops->abort = NULL;
    ops->destroy= actuator_client_destroy;
  }

  pthread_mutex_unlock(&my_actuator_struct.mutex);

  CDBG("%s: client_idx = %d, handle = 0x%x",
        __func__, idx, actuator_client->handle);

  return actuator_client->handle;
} /*ACTUATOR_client_open */

/*===========================================================================
 * FUNCTION    - ACTUATOR_comp_create -
 *
 * DESCRIPTION: initialize actuator component.
 *==========================================================================*/
int ACTUATOR_comp_create()
{
  memset(&my_actuator_struct, 0, sizeof(my_actuator_struct));
  pthread_mutex_init(&my_actuator_struct.mutex, NULL);
  return 0;
} /* ACTUATOR_comp_create */

/*===========================================================================
 * FUNCTION    - ACTUATOR_comp_destroy -
 *
 * DESCRIPTION: destroy actuator component
 *==========================================================================*/
int ACTUATOR_comp_destroy()
{
  pthread_mutex_destroy(&my_actuator_struct.mutex);
  memset(&my_actuator_struct, 0, sizeof(my_actuator_struct));
  return 0;
} /* ACTUATOR_comp_destroy */
