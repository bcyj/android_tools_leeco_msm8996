/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <string.h>
#include <stdlib.h>
#include "stats_proc_interface.h"
#include "stats_proc.h"
#include "camera_dbg.h"

//static stats_proc_t *stats_procCtrl[MAX_INSTANCES];
//static uint32_t stats_proc_handle_cnt = 0;
static statsproc_comp_root_t my_statsproc_struct;

static uint32_t statsproc_gen_handle(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[4bits obj idx]|[4 bits client idx] */
  uint32_t handle =	((++my_statsproc_struct.statsproc_handle_cnt) << 8) +
      (0xff & client_idx);
  return handle;
}

static int statsproc_parse_handle(uint32_t handle, uint8_t *client_idx)
{
  int tmp = (handle & 0xff);
  *client_idx = tmp & 0xff;
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, handle, *client_idx);
  return 0;
}
/*===========================================================================
 * FUNCTION    - get_statsproc_client_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
static stats_proc_t *get_statsproc_client_info(uint32_t handle)
{
  uint8_t client_idx;

  statsproc_parse_handle(handle, &client_idx);
  if(client_idx >= STATSPROC_MAX_CLIENT_NUM ||
	 my_statsproc_struct.client[client_idx].handle != handle) {
	CDBG_ERROR("%s: client_idx = %d, local_handle = 0x%x, input_handle = 0x%x",
			   __func__, client_idx,
			   my_statsproc_struct.client[client_idx].handle,
			   handle);
    return NULL;
  } else
    return &(my_statsproc_struct.client[client_idx]);
} /* get_statsproc_client_info */

/*===========================================================================
 * FUNCTION    - stats_proc_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int stats_proc_client_init(uint32_t handle, mctl_ops_t *ops, void *p_init_data)
//uint32_t handle, stats_proc_interface_input_t *init_data)
{
  stats_proc_interface_input_t *init_data = p_init_data;
  stats_proc_t *sproc = get_statsproc_client_info(handle);
  if (!sproc)
    return -1;
  /* Load Data */
  memcpy( &(sproc->input), init_data, sizeof(stats_proc_interface_input_t));
  sproc->ops = ops;
  if (aec_init(sproc) < 0)
    goto AECfree;
  if (awb_init(sproc) < 0)
    goto AWBfree;
  if (af_init(sproc) < 0)
    goto AFfree;
  if (asd_init(sproc) < 0)
    goto ASDfree;
  if (is_if_init(sproc) < 0)
    goto ISfree;
  if (afd_init(sproc) < 0)
    goto AFDfree;
  return 0;

  AFDfree:
  is_if_deinit(sproc);
  ISfree:
  asd_deinit(sproc);
  ASDfree:
  af_deinit(sproc);
  AFfree:
  awb_deinit(sproc);
  AWBfree:
  aec_deinit(sproc);
  AECfree:
  free(sproc);
  sproc = NULL;
  return -1;
} /* stats_proc_init */

/*===========================================================================
 * FUNCTION    - stats_proc_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int stats_proc_client_destroy(uint32_t handle)
{
  int index;
  stats_proc_t *sproc = get_statsproc_client_info(handle);

  pthread_mutex_lock(&my_statsproc_struct.mutex);
  if (!sproc) {
    pthread_mutex_unlock(&my_statsproc_struct.mutex);
    return 0;
  }
  asd_deinit(sproc);
  af_deinit(sproc);
  awb_deinit(sproc);
  aec_deinit(sproc);
  dis_if_deinit(sproc);
  afd_deinit(sproc);
  memset(sproc, 0, sizeof(stats_proc_t));
  pthread_mutex_unlock(&my_statsproc_struct.mutex);
  return 0;
} /* stats_proc_interface_destroy */

/*===========================================================================
 * FUNCTION    - stats_proc_get_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int stats_proc_client_get_params(uint32_t handle, int type,
                                 void *parm, int parm_len)
{
  int rc = -1;
  stats_proc_get_t *stats_proc_get = parm;
  stats_proc_t *sproc = get_statsproc_client_info(handle);
  if (!sproc)
    return rc;

  switch (type) {
    case STATS_PROC_AEC_TYPE:
      rc = aec_get_params(sproc, &(stats_proc_get->d.get_aec));
      break;
    case STATS_PROC_AWB_TYPE:
      rc = awb_get_params(sproc, &(stats_proc_get->d.get_awb));
      break;
    case STATS_PROC_AF_TYPE:
      rc = af_get_params(sproc, &(stats_proc_get->d.get_af));
      break;
    case STATS_PROC_ASD_TYPE:
      rc = asd_get_params(sproc, &(stats_proc_get->d.get_asd));
      break;
    case STATS_PROC_DIS_TYPE:
      rc = dis_get_params(sproc, &(stats_proc_get->d.get_dis));
      break;
    case STATS_PROC_AFD_TYPE:
      rc = afd_get_params(sproc, &(stats_proc_get->d.get_afd));
      break;
    case STATS_PROC_MOBICAT_TYPE:
      memcpy((QAECInfo_t *)
        &(stats_proc_get->d.get_mobicat.stats_proc_info.aec_info),
        &sproc->share.aec_ext.mobicat_aec, sizeof(QAECInfo_t));
      memcpy((QAWBInfo_t *)
        &(stats_proc_get->d.get_mobicat.stats_proc_info.awb_info),
        &sproc->share.awb_ext.mobicat_awb, sizeof(QAWBInfo_t));
      memcpy((QAFInfo_t *)
        &(stats_proc_get->d.get_mobicat.stats_proc_info.af_info),
        &sproc->share.af_ext.mobicat_af, sizeof(QAFInfo_t));
      rc = 0;
      break;
    default:
      CDBG_ERROR("Invalid STATS_PROC Get Param Type");
  }
  return rc;
} /* stats_proc_get_params */

/*===========================================================================
 * FUNCTION    - stats_proc_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int stats_proc_client_set_params(uint32_t handle, int type, void *parm_in, void *parm_out)
{
  int rc = -1;
  stats_proc_set_t *stats_proc_set = parm_in;
  stats_proc_interface_t *stats_proc_intf = parm_out;
  stats_proc_t *sproc = get_statsproc_client_info(handle);
  if (!sproc) {
	CDBG_ERROR("%s: no stats proc client",  __func__);
    return rc;
  }

  memcpy( &(sproc->input), &(stats_proc_intf->input),
    sizeof(stats_proc_interface_input_t));

  CDBG("%s: E, type = %d", __func__, stats_proc_set->type);
  switch (stats_proc_set->type) {
    case STATS_PROC_AEC_TYPE:
      rc = aec_set_params(sproc, &(stats_proc_set->d.set_aec));
      memcpy(&stats_proc_intf->output.aec_d, &sproc->share.aec_ext,
        sizeof(stats_proc_aec_data_t));
      break;
    case STATS_PROC_AWB_TYPE:
      rc = awb_set_params(sproc, &(stats_proc_set->d.set_awb));
      memcpy(&stats_proc_intf->output.awb_d, &sproc->share.awb_ext,
        sizeof(stats_proc_awb_data_t));
      break;
    case STATS_PROC_AF_TYPE:
      rc = af_set_params(sproc, &(stats_proc_set->d.set_af));
      memcpy(&stats_proc_intf->output.af_d, &sproc->share.af_ext,
        sizeof(stats_proc_af_data_t));
      break;
    case STATS_PROC_ASD_TYPE:
      rc = asd_set_params(sproc, &(stats_proc_set->d.set_asd));
      memcpy(&stats_proc_intf->output.asd_d, &sproc->share.asd_ext,
        sizeof(stats_proc_asd_data_t));
      break;
    case STATS_PROC_DIS_TYPE:
      rc = dis_set_params(sproc, &(stats_proc_set->d.set_dis));
      if (rc)
        return rc;
      rc = eis_set_params(sproc, &(stats_proc_set->d.set_dis));
      break;
    case STATS_PROC_AFD_TYPE:
      rc = afd_set_params(sproc, &(stats_proc_set->d.set_afd));
      memcpy(&stats_proc_intf->output.afd_d, &sproc->share.afd_ext,
        sizeof(stats_proc_afd_data_t));
      break;
    case STATS_PROC_EZ_TUNE_TYPE:
      if (stats_proc_set->d.set_eztune.type == EZ_TUNE_ENABLE)
        sproc->share.eztune_enable =
          stats_proc_set->d.set_eztune.d.eztune_enable;
      break;
    case STATS_PROC_CHROMATIX_RELOAD_TYPE:
      aec_chromatix_reload(sproc);
      memcpy(&stats_proc_intf->output.aec_d, &sproc->share.aec_ext,
        sizeof(stats_proc_aec_data_t));
      awb_chromatix_reload(sproc);
      memcpy(&stats_proc_intf->output.awb_d, &sproc->share.awb_ext,
        sizeof(stats_proc_awb_data_t));
      af_chromatix_reload(sproc);
      memcpy(&stats_proc_intf->output.af_d, &sproc->share.af_ext,
        sizeof(stats_proc_af_data_t));
      break;
    case STATS_PROC_MOBICAT_TYPE:
      sproc->share.mobicat_enable =
       stats_proc_set->d.set_mobicat.d.mobicat_enable;
      rc = 0;
      break;
    default:
      CDBG_ERROR("Invalid STATS_PROC Set Param Type");
  }
  CDBG("%s: X, type = %d, rc = %d", __func__, stats_proc_set->type, rc);
  return rc;
} /* stats_proc_set_params */

/*===========================================================================
 * FUNCTION    - stats_proc_process -
 *
 * DESCRIPTION:
 *==========================================================================*/
int stats_proc_client_process(uint32_t handle, int event, void *data)
//uint32_t handle, stats_proc_interface_t *stats_proc_intf)
{
  int rc = -1;
  stats_proc_interface_t *stats_proc_intf = data;
  stats_proc_t *sproc = get_statsproc_client_info(handle);
  if (!sproc) {
    CDBG_ERROR("%s: statsproc handle is invalid", __func__);
    return rc;
  }
  memcpy( &(sproc->input), &(stats_proc_intf->input),
    sizeof(stats_proc_interface_input_t));

  switch (stats_proc_intf->input.mctl_info.type) {
    case STATS_PROC_AEC_TYPE:
      if((rc = aec_process(sproc)) == 0)
        memcpy(&stats_proc_intf->output.aec_d, &sproc->share.aec_ext,
          sizeof(stats_proc_aec_data_t));
      break;
    case STATS_PROC_AWB_TYPE:
      if((rc = awb_process(sproc)) == 0)
        memcpy(&stats_proc_intf->output.awb_d, &sproc->share.awb_ext,
          sizeof(stats_proc_awb_data_t));
      break;
    case STATS_PROC_AF_TYPE:
      if((rc = af_process(sproc)) == 0)
        memcpy(&stats_proc_intf->output.af_d, &sproc->share.af_ext,
          sizeof(stats_proc_af_data_t));
      break;
    case STATS_PROC_ASD_TYPE:
      if((rc = asd_process(sproc)) == 0)
        memcpy(&stats_proc_intf->output.asd_d, &sproc->share.asd_ext,
          sizeof(stats_proc_asd_data_t));
      break;
    case STATS_PROC_DIS_TYPE:
      if((rc = is_process(sproc,
          stats_proc_intf->input.mctl_info.vfe_stats_out->vfe_stats_struct.cs_op.frame_id))== 0)
      memcpy(&stats_proc_intf->output.dis_d, &sproc->share.dis_ext,
        sizeof(stats_proc_dis_data_t));
      break;
    case STATS_PROC_AFD_TYPE:
      if((rc = afd_process(sproc)) == 0)
        memcpy(&stats_proc_intf->output.afd_d, &sproc->share.afd_ext,
          sizeof(stats_proc_afd_data_t));
      break;
    default:
      CDBG_ERROR("Invalid STATS_PROC Process Type");
      break;
  }
  return rc;
} /* stats_proc_process */

/*============================================================================
 * FUNCTION    - STATSPROC_interface_create -
 *
 * DESCRIPTION:
 *===========================================================================*/
uint32_t STATSPROC_client_open(module_ops_t *ops)
{
  uint32_t handle = 0;
  int idx;
  stats_proc_t *stats_client = NULL;

  if(!ops) {
	CDBG_ERROR("%s: null ops pointer",  __func__);
	return 0;
  }
  memset(ops,  0,  sizeof(module_ops_t));

  pthread_mutex_lock(&my_statsproc_struct.mutex);
  for(idx = 0; idx < STATSPROC_MAX_CLIENT_NUM; idx++) {
    if(my_statsproc_struct.client[idx].handle == 0) {
	  stats_client = &my_statsproc_struct.client[idx];
      break;
	}
  }
  /* if not found return null */
  if (!stats_client) {
	pthread_mutex_unlock(&my_statsproc_struct.mutex);
    return (uint32_t)NULL;
  } else {
    memset(stats_client, 0, sizeof(stats_proc_t));
	stats_client->obj_idx_mask = 0;
	stats_client->client_idx = idx;
    stats_client->my_comp_id = MCTL_COMPID_STATSPROC;
    stats_client->handle = statsproc_gen_handle((uint8_t)idx);
	ops->handle = (uint32_t)stats_client->handle;
	ops->init = stats_proc_client_init;
	ops->set_params = stats_proc_client_set_params;
	ops->get_params = stats_proc_client_get_params;
	ops->process = stats_proc_client_process;
	ops->abort = NULL;
	ops->destroy= stats_proc_client_destroy;
  }
  pthread_mutex_unlock(&my_statsproc_struct.mutex);
  CDBG("%s: client_idx = %d, handle = 0x%x",
	   __func__, idx, stats_client->handle);
  return stats_client->handle;
}/*stats_interface_create*/

/*===========================================================================
 * FUNCTION    - STATSPROC_comp_create -
 *
 * DESCRIPTION: initialize STATSPROC component.
 *==========================================================================*/
int STATSPROC_comp_create()
{
  memset(&my_statsproc_struct, 0, sizeof(my_statsproc_struct));
  pthread_mutex_init(&my_statsproc_struct.mutex, NULL);
  CDBG("%s: STATSPROC_comp_create success", __func__);

  return 0;
}

/*===========================================================================
 * FUNCTION    - STATSPROC_comp_destroy -
 *
 * DESCRIPTION: destroy the STATSPROC component
 *==========================================================================*/
int STATSPROC_comp_destroy()
{
  pthread_mutex_destroy(&my_statsproc_struct.mutex);
  memset(&my_statsproc_struct, 0, sizeof(my_statsproc_struct));
  return 0;
}


