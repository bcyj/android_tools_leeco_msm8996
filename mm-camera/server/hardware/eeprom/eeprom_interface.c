/*==========================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
#include "eeprom.h"
#include "camera_dbg.h"

static eeprom_comp_root_t eeprom_controller;

/*===========================================================================
 * FUNCTION    - eeprom_gen_handle -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint32_t eeprom_gen_handle(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[4bits obj idx]|[4 bits client idx] */
  uint32_t handle = ((++eeprom_controller.handle_cnt) << 8) +
                    (0xff & client_idx);
  return handle;
}

/*===========================================================================
 * FUNCTION    - eeprom_parse_handle -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int eeprom_parse_handle(uint32_t handle, uint8_t *client_idx)
{
  int tmp = (handle & 0xff);
  *client_idx = tmp & 0xff;
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, handle, *client_idx);
  return 0;
}

/*===========================================================================
 * FUNCTION    - get_eeprom_client_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static eeprom_client_t *get_eeprom_client_info(uint32_t handle)
{
  uint8_t client_idx;

  eeprom_parse_handle(handle, &client_idx);
  if ((client_idx >= EEPROM_MAX_CLIENT_NUM) ||
      (eeprom_controller.client[client_idx].handle != handle)) {
    CDBG_ERROR("%s: client_idx = %d, local_handle = 0x%x, input_handle = 0x%x",
                __func__, client_idx,
                eeprom_controller.client[client_idx].handle,
                handle);
    return NULL;
  } else
    return &(eeprom_controller.client[client_idx]);
}

/*==========================================================
 * FUNCTION    - eeprom_client_set_params -
 *
 * DESCRIPTION:
 *==========================================================*/
static int eeprom_client_set_params(uint32_t handle, int type, void *parm_in,
  void *parm_out)
{
  int rc = 0;

  eeprom_set_t *eeprom_set = (eeprom_set_t *)parm_in;
  eeprom_client_t *eeprom_client = get_eeprom_client_info(handle);
  if (!eeprom_client) {
    CDBG_ERROR("%s: null eeprom client\n", __func__);
    return -EINVAL;
  }
  eeprom_ctrl_t *ectrl = eeprom_client->eepromCtrl;
  CDBG("%s: parm type =%d\n", __func__, type);

  switch (type) {
  case EEPROM_SET_CHROMATIX:
    ectrl->chromatixPtr = eeprom_set->data.info.chromatixPtr;
    break;

  case EEPROM_SET_FOCUSPTR:
    ectrl->aftune_ptr = eeprom_set->data.info.aftune_ptr;
    break;

  case EEPROM_SET_DOCALIB:
    ectrl->fn_table.do_calibration(ectrl);
    break;

  default:
    rc = EEPROM_SET_INVALID;
  }

  return rc;
}

/*==========================================================
 * FUNCTION    - eeprom_client_get_params -
 *
 * DESCRIPTION:
 *==========================================================*/
static int eeprom_client_get_params(uint32_t handle, int type,
  void *parm_in, int parm_len)
{
  int rc = 0;
  eeprom_get_t *eeprom_get = (eeprom_get_t *)parm_in;
  eeprom_client_t *eeprom_client = get_eeprom_client_info(handle);
  if (!eeprom_client) {
    CDBG_ERROR("%s: null eeprom client\n", __func__);
    return -EINVAL;
  }
  eeprom_ctrl_t *ectrl = eeprom_client->eepromCtrl;
  CDBG("%s: parm type =%d\n", __func__, type);

  switch (eeprom_get->type) {

  case EEPROM_GET_SUPPORT:
    eeprom_get->data.is_eeprom_supported = ectrl->is_eeprom_supported;
    break;

  case EEPROM_GET_CALIB_2D_DPC:
    if (ectrl->fn_table.dpc_calibration_info != NULL)
      ectrl->fn_table.dpc_calibration_info(ectrl, type,
      &eeprom_get->data.dpc_info);
    break;
  case EEPROM_GET_RAW_DATA:
    if (ectrl->fn_table.eeprom_get_raw_data)
      rc = ectrl->fn_table.eeprom_get_raw_data(ectrl,
        eeprom_get->data.raw_data);
    else
      rc = -EFAULT;
    break;
  default:
    rc = EEPROM_GET_INVALID;
  }

  return rc;
}

/*============================================================================
 * FUNCTION    - eeprom_interface_destroy -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int eeprom_client_destroy(uint32_t handle)
{
  eeprom_client_t *client;

  CDBG("%s: handle = 0x%x", __func__, handle);
  pthread_mutex_lock(&eeprom_controller.mutex);

  client = get_eeprom_client_info(handle);
  if (!client) {
    CDBG_ERROR("%s Invalid handle passed ", __func__);
    pthread_mutex_unlock(&eeprom_controller.mutex);
    return 0;
  }

  eeprom_destroy(client->eepromCtrl);
  memset(client, 0, sizeof(eeprom_client_t));
  pthread_mutex_unlock(&eeprom_controller.mutex);
  return 0;
}

/*============================================================================
 * FUNCTION    - eeprom_interface_init -
 *
 * DESCRIPTION:
 *===========================================================================*/
static int eeprom_client_init(uint32_t handle, mctl_ops_t *ops,
  void *init_data)
{
  eeprom_client_t *client;

  pthread_mutex_lock(&eeprom_controller.mutex);
  client = get_eeprom_client_info(handle);
  if (!client) {
    CDBG_ERROR("%s Invalid handle 0x%x passed ", __func__, handle);
    pthread_mutex_unlock(&eeprom_controller.mutex);
    return -EINVAL;
  }

  if (!ops) {
    CDBG_ERROR("%s Invalid handle passed %d ", __func__, handle);
    pthread_mutex_unlock(&eeprom_controller.mutex);
    return -EINVAL;
  }

  if (ops->fd < 0) {
    CDBG_ERROR("%s Invalid argument camera fd %d ", __func__, ops->fd);
    pthread_mutex_unlock(&eeprom_controller.mutex);
    return -EINVAL;
  }

  client->ops = ops;
  client->eepromCtrl->fd = ops->fd;
  eeprom_init(client->eepromCtrl);
  CDBG("%s ops->fd = %d\n", __func__, ops->fd);
  pthread_mutex_unlock(&eeprom_controller.mutex);
  return 0;
}

/*============================================================================
 * FUNCTION    - eeprom_client_open -
 *
 * DESCRIPTION:
 *===========================================================================*/
uint32_t eeprom_client_open(module_ops_t *ops)
{
  uint32_t handle = 0;
  int idx = 0;
  eeprom_client_t *eeprom_client = NULL;

  if (!ops) {
    CDBG_ERROR("%s: null ops pointer", __func__);
    return 0;
  }

  memset(ops, 0, sizeof(module_ops_t));

  pthread_mutex_lock(&eeprom_controller.mutex);

  for (idx = 0; idx < EEPROM_MAX_CLIENT_NUM; idx++) {
    if (eeprom_controller.client[idx].handle == 0) {
      eeprom_client = &eeprom_controller.client[idx];
      break;
    }
  }

  /* if not found return null */
  if (!eeprom_client) {
    pthread_mutex_unlock(&eeprom_controller.mutex);
    return (uint32_t)NULL;
  } else {
    memset(eeprom_client, 0, sizeof(eeprom_client_t));
    eeprom_client->client_idx = idx;
    eeprom_client->handle = eeprom_gen_handle((uint8_t)idx);
    ops->handle = (uint32_t)eeprom_client->handle;
    eeprom_client->eepromCtrl = (eeprom_ctrl_t *)
      malloc(sizeof(eeprom_ctrl_t));
    if (NULL == eeprom_client->eepromCtrl) {
      pthread_mutex_unlock(&eeprom_controller.mutex);
      return (uint32_t)NULL;
    }
    memset(eeprom_client->eepromCtrl, 0, sizeof(eeprom_ctrl_t));
    ops->init = eeprom_client_init;
    ops->set_params = eeprom_client_set_params;
    ops->get_params = eeprom_client_get_params;
    ops->process = NULL;
    ops->abort = NULL;
    ops->destroy = eeprom_client_destroy;
  }

  pthread_mutex_unlock(&eeprom_controller.mutex);

  CDBG("%s: client_idx = %d, handle = 0x%x",
        __func__, idx, eeprom_client->handle);

  return eeprom_client->handle;
}

/*===========================================================================
 * FUNCTION    - eeprom_comp_create -
 *
 * DESCRIPTION: initialize eeprom component.
 *==========================================================================*/
int eeprom_comp_create()
{
  memset(&eeprom_controller, 0, sizeof(eeprom_controller));
  pthread_mutex_init(&eeprom_controller.mutex, NULL);
  return 0;
}

/*===========================================================================
 * FUNCTION    - eeprom_comp_destroy -
 *
 * DESCRIPTION: destroy eeprom component
 *==========================================================================*/
int eeprom_comp_destroy()
{
  pthread_mutex_destroy(&eeprom_controller.mutex);
  memset(&eeprom_controller, 0, sizeof(eeprom_controller));
  return 0;
}
