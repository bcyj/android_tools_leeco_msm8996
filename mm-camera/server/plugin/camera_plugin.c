/**********************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

/*===========================================================================
 *                         INCLUDE FILES
 *===========================================================================*/
#include "camera.h"
#include "camera_plugin.h"
#include "camera_dbg.h"

#if 1
#undef CDBG
#define CDBG LOGE
#endif

static camera_plugin_root_t cam_plugin_root;

static camera_plugin_client_t *camera_plugin_util_get_client_no_lock(
  void *plugin_handle, uint32_t client_handle)
{
  camera_plugin_client_t *client = NULL;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)plugin_handle;
  int idx;

  idx = client_handle & 0xFF;
  if(idx >= CAMERA_PLUGIN_CLIENT_MAX || idx < 0) {
    CDBG_ERROR("%s: invalid client index %d for client handle 0x%x, error out",
      __func__,  idx, client_handle);
    goto end;
  }
  client = &plugin->client[idx];
  if(client->client_handle != client_handle) {
    CDBG_ERROR("%s: input client handle 0x%x mismatch plugin cklient handle 0x%x, error out",
      __func__, client_handle, client->client_handle);
    goto end;
  }
end:
  return client;
}

plugin_status_t camera_plugin_vfe_init(void *plugin_handle,
  uint32_t client_handle)
{
  camera_plugin_client_t *client = NULL;
  client = camera_plugin_util_get_client_no_lock (plugin_handle, client_handle);

  client->vfe_reg_update_data.entries[MOD_LINEARIZATION].module_type = VFE_MOD_LINEARIZATION;
  client->vfe_reg_update_data.entries[MOD_LINEARIZATION].len = sizeof(linear_module_t);
  client->vfe_reg_update_data.entries[MOD_LINEARIZATION].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_linear;

  client->vfe_reg_update_data.entries[MOD_ROLLOFF].module_type = VFE_MOD_ROLLOFF;
  client->vfe_reg_update_data.entries[MOD_ROLLOFF].len = sizeof(rolloff_module_t);
  client->vfe_reg_update_data.entries[MOD_ROLLOFF].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_roll_off;

  client->vfe_reg_update_data.entries[MOD_DEMUX].module_type = VFE_MOD_DEMUX;
  client->vfe_reg_update_data.entries[MOD_DEMUX].len = sizeof(demux_module_t);
  client->vfe_reg_update_data.entries[MOD_DEMUX].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_demux;

  client->vfe_reg_update_data.entries[MOD_DEMOSAIC].module_type = VFE_MOD_DEMOSAIC;
  client->vfe_reg_update_data.entries[MOD_DEMOSAIC].len = sizeof(demosaic_module_t);
  client->vfe_reg_update_data.entries[MOD_DEMOSAIC].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_demosaic;

  client->vfe_reg_update_data.entries[MOD_BPC].module_type = VFE_MOD_BPC;
  client->vfe_reg_update_data.entries[MOD_BPC].len = sizeof(bpc_module_t);
  client->vfe_reg_update_data.entries[MOD_BPC].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_bpc;

  client->vfe_reg_update_data.entries[MOD_ABF].module_type = VFE_MOD_ABF;
  client->vfe_reg_update_data.entries[MOD_ABF].len = sizeof(abf_module_t);
  client->vfe_reg_update_data.entries[MOD_ABF].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_abf;

  client->vfe_reg_update_data.entries[MOD_ASF].module_type = VFE_MOD_ASF;
  client->vfe_reg_update_data.entries[MOD_ASF].len = sizeof(asf_module_t);
  client->vfe_reg_update_data.entries[MOD_ASF].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_asf;

  client->vfe_reg_update_data.entries[MOD_COLOR_CONV].module_type = VFE_MOD_COLOR_CONV ;
  client->vfe_reg_update_data.entries[MOD_COLOR_CONV].len = sizeof(chroma_enhan_module_t);
  client->vfe_reg_update_data.entries[MOD_COLOR_CONV].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_ce;

  client->vfe_reg_update_data.entries[MOD_COLOR_CORRECT].module_type = VFE_MOD_COLOR_CORRECT;
  client->vfe_reg_update_data.entries[MOD_COLOR_CORRECT].len = sizeof(color_correct_module_t);
  client->vfe_reg_update_data.entries[MOD_COLOR_CORRECT].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_cc;

  client->vfe_reg_update_data.entries[MOD_CHROMA_SS].module_type = VFE_MOD_CHROMA_SS;
  client->vfe_reg_update_data.entries[MOD_CHROMA_SS].len = sizeof(chroma_ss_module_t);
  client->vfe_reg_update_data.entries[MOD_CHROMA_SS].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_ss;

  client->vfe_reg_update_data.entries[MOD_CHROMA_SUPPRESS].module_type = VFE_MOD_CHROMA_SUPPRESS;
  client->vfe_reg_update_data.entries[MOD_CHROMA_SUPPRESS].len = sizeof(chroma_supp_module_t);
  client->vfe_reg_update_data.entries[MOD_CHROMA_SUPPRESS].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_chrom_supp;

  client->vfe_reg_update_data.entries[MOD_LA].module_type = VFE_MOD_LA;
  client->vfe_reg_update_data.entries[MOD_LA].len = sizeof(la_module_t);
  client->vfe_reg_update_data.entries[MOD_LA].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_la;

  client->vfe_reg_update_data.entries[MOD_MCE].module_type = VFE_MOD_MCE;
  client->vfe_reg_update_data.entries[MOD_MCE].len = sizeof(mce_module_t);
  client->vfe_reg_update_data.entries[MOD_MCE].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_mce;

  client->vfe_reg_update_data.entries[MOD_SCE].module_type = VFE_MOD_SCE;
  client->vfe_reg_update_data.entries[MOD_SCE].len = sizeof(sce_module_t);
  client->vfe_reg_update_data.entries[MOD_SCE].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_sce;

  client->vfe_reg_update_data.entries[MOD_CLF].module_type = VFE_MOD_CLF;
  client->vfe_reg_update_data.entries[MOD_CLF].len = sizeof(clf_module_t);
  client->vfe_reg_update_data.entries[MOD_CLF].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_clf;

  client->vfe_reg_update_data.entries[MOD_WB].module_type = VFE_MOD_WB;
  client->vfe_reg_update_data.entries[MOD_WB].len = sizeof(wb_module_t);
  client->vfe_reg_update_data.entries[MOD_WB].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_wb;

  client->vfe_reg_update_data.entries[MOD_GAMMA].module_type = VFE_MOD_GAMMA;
  client->vfe_reg_update_data.entries[MOD_GAMMA].len = sizeof(gamma_module_t);
  client->vfe_reg_update_data.entries[MOD_GAMMA].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_gamma;

  client->vfe_reg_update_data.entries[MOD_AWB_STATS].module_type = VFE_MOD_AWB_STATS;
  client->vfe_reg_update_data.entries[MOD_AWB_STATS].len = sizeof(awb_stats_module_t);
  client->vfe_reg_update_data.entries[MOD_AWB_STATS].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_awb;

  client->vfe_reg_update_data.entries[MOD_FOV].module_type = VFE_MOD_FOV;
  client->vfe_reg_update_data.entries[MOD_FOV].len = sizeof(fov_module_t);
  client->vfe_reg_update_data.entries[MOD_FOV].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_fov;

  client->vfe_reg_update_data.entries[MOD_SCALER].module_type = VFE_MOD_SCALER;
  client->vfe_reg_update_data.entries[MOD_SCALER].len = sizeof(scaler_module_t);
  client->vfe_reg_update_data.entries[MOD_SCALER].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_scaler;

  client->vfe_reg_update_data.entries[MOD_BCC].module_type = VFE_MOD_BCC;
  client->vfe_reg_update_data.entries[MOD_BCC].len = sizeof(bcc_module_t);
  client->vfe_reg_update_data.entries[MOD_BCC].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_bcc;

  client->vfe_reg_update_data.entries[MOD_CLAMP].module_type = VFE_MOD_CLAMP;
  client->vfe_reg_update_data.entries[MOD_CLAMP].len = sizeof(clamp_module_t);
  client->vfe_reg_update_data.entries[MOD_CLAMP].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_clamp;

  client->vfe_reg_update_data.entries[MOD_FRAME_SKIP].module_type = VFE_MOD_FRAME_SKIP;
  client->vfe_reg_update_data.entries[MOD_FRAME_SKIP].len = sizeof(frame_skip_module_t);
  client->vfe_reg_update_data.entries[MOD_FRAME_SKIP].reg_update_data =
    (void *)&client->vfe_reg_update_data.reg_update_fs;

  return PLGN_SUCCESS;
}

static int camera_plugin_vfe_reg_update(camera_plugin_client_t *client,
  uint8_t hw_write_immediate)
{
  int rc = 0;
  camera_plugin_process_vfe_t vfe_process;

  vfe_process.reg_update.entry = NULL;
  vfe_process.type = CAM_OEM_PLUGIN_PROC_REG_UPDATE;

  rc = client->mctl_ops.process_vfe(client->mctl_ops.user_data,
         &vfe_process, 1);
  if(rc < 0) {
    CDBG_ERROR("%s: Error vfe reg update rc :%d\n", __func__, rc);
    return -1;
  }
  return rc;
}

/*the module command needs to be filled in before calling this
  Funtion.
  mod_mask is the module type*/
static int camera_plugin_vfe_mod_update(camera_plugin_client_t *client,
  uint32_t mod_mask)
{
  int rc = 0;
  camera_plugin_process_vfe_t vfe_process;

  vfe_process.reg_update.entry =
    &(client->vfe_reg_update_data.entries[mod_mask]);
  vfe_process.type = CAM_OEM_PLUGIN_PROC_MOD_UPDATE;

  rc = client->mctl_ops.process_vfe(client->mctl_ops.user_data,
         &vfe_process, 0);
  if(rc < 0) {
    CDBG_ERROR("%s: Error vfe reg update rc :%d\n", __func__, rc);
    return -1;
  }
  return rc;
}

static int camera_plugin_diff_clk_for_rdi_camera (void *plugin_handle,
                       uint32_t client_handle, uint8_t *use_diff_clock)
{
  int rc = 0;
  camera_plugin_client_t *client = NULL;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)plugin_handle;

  *use_diff_clock = 0;
  pthread_mutex_lock (&plugin->mutex);
  client = camera_plugin_util_get_client_no_lock (plugin_handle,
    client_handle);
  if (!client) {
    CDBG_ERROR("%s: Cannot find client (plugin = %p, client_handle = 0x%x",
      __func__, plugin_handle, client_handle);
    rc = -1;
    pthread_mutex_unlock (&plugin->mutex);
    goto err;
  }
  pthread_mutex_lock (&client->client_mutex);
  pthread_mutex_unlock (&plugin->mutex);
  *use_diff_clock = plugin->diff_clk_for_rdi_pix;
  pthread_mutex_unlock (&client->client_mutex);
err:
  return rc;

}

static int camera_plugin_grant_pix_interface(void *plugin_handle,
  uint32_t client_handle, uint8_t *grant_pix, uint8_t *concurrent_enabled,
  uint8_t *default_vfe_id, struct msm_camsensor_info *sensor_info,
  uint8_t max_hw_num)
{
  int rc = 0;
  camera_plugin_client_t *client = NULL;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)plugin_handle;

  *default_vfe_id = 0; /* set default */
  pthread_mutex_lock (&plugin->mutex);
  client = camera_plugin_util_get_client_no_lock(plugin_handle,
    client_handle);
  if (!client) {
    CDBG_ERROR("%s: Cannot find client plugin = %p, client_handle = 0x%x",
      __func__, plugin_handle, client_handle);
    rc = -1;
    pthread_mutex_unlock (&plugin->mutex);
    goto err;
  }
  pthread_mutex_lock (&client->client_mutex);
  pthread_mutex_unlock (&plugin->mutex);
  CDBG("%s Support simultaneous camera %d Sensor type %d BAYER %d",
    __func__, cam_plugin_root.support_simultaneous_camera,
    sensor_info->sensor_type, BAYER);
  /* if not support simultaneous camera always
   * grant pix interface to the camera */
  *concurrent_enabled = cam_plugin_root.support_simultaneous_camera;
  if(cam_plugin_root.support_simultaneous_camera == 0) {
    *grant_pix = 1;
    goto end;
  }
  *grant_pix = 0;
  switch(sensor_info->sensor_type) {
  case BAYER:
    *grant_pix = 1;
    break;
  case YUV:
  case JPEG_SOC:
  default:
    break;
  }
end:
  if (max_hw_num > 1) {
    /* if num vfe == 1 front camera uses vfe 1
     * enum msm_camera_type in board.h */
    if (sensor_info->camera_type == 1) {
      /* front camera */
      *default_vfe_id = 1; /* use vfe 1 for VFE and AXI HW */
      }
    } else
    *default_vfe_id = 0;
  pthread_mutex_unlock (&client->client_mutex);
err:
  return rc;
}

static int camera_plugin_client_init (void *plugin_handle, uint32_t client_handle,
  camera_plugin_mctl_process_ops_t *mctl_ops)
{
  int rc = 0;
  camera_plugin_client_t *client = NULL;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)plugin_handle;
  pthread_mutex_lock (&plugin->mutex);
  client = camera_plugin_util_get_client_no_lock (plugin_handle,
                                                 client_handle);
  if (!client) {
    CDBG_ERROR("%s: Cannot find client (plugin = %p, client_handle = 0x%x",
      __func__, plugin_handle, client_handle);
    rc = -1;
    pthread_mutex_unlock (&plugin->mutex);
    goto err;
  }
  pthread_mutex_lock (&client->client_mutex);
  pthread_mutex_unlock (&plugin->mutex);
  client->mctl_ops = *mctl_ops;

  camera_plugin_vfe_init(plugin_handle, client_handle);
  pthread_mutex_unlock (&client->client_mutex);
err:
  return rc;
}

static void  camera_plugin_client_destroy_proc (
  camera_plugin_client_t *client)
{
  pthread_mutex_destroy (&client->client_mutex);
  memset (client,  0,  sizeof(camera_plugin_client_t));
}

static void camera_plugin_client_destroy (void *plugin_handle, uint32_t client_handle)
{
  camera_plugin_client_t *client = NULL;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)plugin_handle;

  pthread_mutex_lock (&plugin->mutex);
  client = camera_plugin_util_get_client_no_lock (plugin_handle,
                                                  client_handle);
  if (!client) {
    CDBG_ERROR("%s: Cannot find client (plugin = %p, client_handle = 0x%x",
      __func__, plugin_handle, client_handle);
    pthread_mutex_unlock (&plugin->mutex);
    goto err;
  }
  plugin->client[client_handle].used = 0;
  camera_plugin_client_destroy_proc (client);
  plugin->handle_cnt--;
  pthread_mutex_unlock (&plugin->mutex);
err:
  return;
}

static int camera_plugin_client_stats_notify_to_plugin (void *plugin_handle, uint32_t client_handle,
                       int stats_type, void *stats_data)
{
  int rc = 0;
  camera_plugin_client_t *client = NULL;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)plugin_handle;

  pthread_mutex_lock (&plugin->mutex);
  client = camera_plugin_util_get_client_no_lock (plugin_handle,
                                                  client_handle);
  if (!client) {
    CDBG_ERROR("%s: Cannot find client (plugin = %p, client_handle = 0x%x",
      __func__, plugin_handle, client_handle);
    rc = -1;
    pthread_mutex_unlock (&plugin->mutex);
    goto end;
  }
  pthread_mutex_lock (&client->client_mutex);
  pthread_mutex_unlock (&plugin->mutex);
  CDBG("%s: stats_type = %d",
    __func__, stats_type);
  /* do something */
  pthread_mutex_unlock (&client->client_mutex);
end:
  return rc;
}

static int camera_plugin_client_dis_notify_to_plugin (void *plugin_handle, uint32_t client_handle,
                       camera_plugin_dis_notify_t *payload)
{
  int rc = 0;
  camera_plugin_client_t *client = NULL;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)plugin_handle;

  pthread_mutex_lock (&plugin->mutex);
  client = camera_plugin_util_get_client_no_lock (plugin_handle,
             client_handle);
  if (!client) {
    CDBG_ERROR("%s: Cannot find client (plugin = %p, client_handle = 0x%x",
      __func__, plugin_handle, client_handle);
    rc = -1;
    pthread_mutex_unlock (&plugin->mutex);
    goto end;
  }
  pthread_mutex_lock (&client->client_mutex);
  pthread_mutex_unlock (&plugin->mutex);
  CDBG("%s: dis notify type = %d",
    __func__, payload->notify_type);
  /* do something */
  pthread_mutex_unlock (&client->client_mutex);
end:
  return rc;
}

static int camera_plugin_client_isp_timing_notify_to_plugin (void *plugin_handle, uint32_t client_handle,
                       camera_plugin_isp_timing_type_t timing_type, void *data)
{
  int rc = 0;
  camera_plugin_client_t *client = NULL;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)plugin_handle;

  pthread_mutex_lock (&plugin->mutex);
  client = camera_plugin_util_get_client_no_lock (plugin_handle,
             client_handle);
  if (!client) {
    CDBG_ERROR("%s: Cannot find client (plugin = %p, client_handle = 0x%x",
      __func__, plugin_handle, client_handle);
    rc = -1;
    pthread_mutex_unlock (&plugin->mutex);
    goto end;
  }
  pthread_mutex_lock (&client->client_mutex);
  pthread_mutex_unlock (&plugin->mutex);
  CDBG("%s: timing event %d",
    __func__, timing_type);
  switch(timing_type) {
  case CAM_OEM_PLUGIN_ISP_TIMING_STREAMON:
  case CAM_OEM_PLUGIN_ISP_TIMING_STREAMOFF:
  case CAM_OEM_PLUGIN_ISP_TIMING_RESET_ACK_RECEIVED:
  case CAM_OEM_PLUGIN_ISP_TIMING_VFE_SOF_ACK:
  case CAM_OEM_PLUGIN_ISP_TIMING_VFE_START_ACK:
  case CAM_OEM_PLUGIN_ISP_TIMING_VFE_STOP_ACK:
    break;
  default:
    rc = -1;
    break;
  }
  pthread_mutex_unlock (&client->client_mutex);
end:
  return rc;
}

static int camera_plugin_client_get_sensor_parm_from_plugin (void *plugin_handle,
                       uint32_t client_handle, uint32_t type,
                       void *params, int params_len) /* sensor_data_t */
{
  int rc = -100;
  camera_plugin_client_t *client = NULL;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)plugin_handle;

  CDBG("%s: received private ioctl", __func__);
  pthread_mutex_lock (&plugin->mutex);
  client = camera_plugin_util_get_client_no_lock (plugin_handle,
            client_handle);
  if (!client) {
    CDBG_ERROR("%s: Cannot find client (plugin = %p, client_handle = 0x%x",
      __func__, plugin_handle, client_handle);
    rc = -1;
    pthread_mutex_unlock (&plugin->mutex);
    goto end;
  }
  pthread_mutex_lock (&client->client_mutex);
  pthread_mutex_unlock (&plugin->mutex);
  /* do something */
  pthread_mutex_unlock (&client->client_mutex);
end:
  return rc;
}

static int camera_plugin_client_sensor_notify_to_plugin (void *plugin_handle, uint32_t client_handle,
                       camera_plugin_sensor_notify_t *sensor_notify)
{
  int rc = 0;
  camera_plugin_client_t *client = NULL;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)plugin_handle;

  CDBG("%s: received private ioctl", __func__);
  pthread_mutex_lock (&plugin->mutex);
  client = camera_plugin_util_get_client_no_lock (plugin_handle,
             client_handle);
  if (!client) {
    CDBG_ERROR("%s: Cannot find client (plugin = %p, client_handle = 0x%x",
      __func__, plugin_handle, client_handle);
    rc = -1;
    pthread_mutex_unlock (&plugin->mutex);
    goto end;
  }
  pthread_mutex_lock (&client->client_mutex);
  pthread_mutex_unlock (&plugin->mutex);
  CDBG("%s: sensor_notify type = %d", __func__, sensor_notify->type);
  pthread_mutex_unlock (&client->client_mutex);
end:
  return rc;
}
static int camera_plugin_proc_oem_private_ioctl(camera_plugin_root_t *plugin,
  camera_plugin_client_t *client,
  camera_plugin_ioctl_data_t *data,
  int *status)
{
  int rc = 0;

  /* this is a dummy functiona nd expected OEM to fill in their own logic */
  *status = 1;
  return rc;
}
static int camera_plugin_client_private_ioctl_to_plugin (void *plugin_handle,
  uint32_t client_handle,
  camera_plugin_ioctl_data_t *data, int *status)
{
  int rc = 0;
  uint32_t tmp = 0;
  camera_plugin_client_t *client = NULL;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)plugin_handle;

  CDBG("%s: received private ioctl", __func__);
  pthread_mutex_lock (&plugin->mutex);
  client = camera_plugin_util_get_client_no_lock (plugin_handle,
             client_handle);
  if (!client) {
    CDBG_ERROR("%s: Cannot find client (plugin = %p, client_handle = 0x%x",
      __func__, plugin_handle, client_handle);
    rc = -1;
    pthread_mutex_unlock (&plugin->mutex);
    goto end;
  }
  pthread_mutex_lock (&client->client_mutex);
  pthread_mutex_unlock (&plugin->mutex);
  /* we reserve several reserved plugin private ioctls.
   * rest are processed by oem's own implementation */
  switch (data->type) {
  case MM_CAM_PLUGIN_PRIVATE_IOCTL_EVENT_SIMULTANEOUS_CAM:
    /* 1: support simultaneous camera, 0 - not support */
    tmp = *((uint32_t *)data->data);
    plugin->support_simultaneous_camera = (uint8_t)tmp;
    break;
  case MM_CAM_PLUGIN_PRIVATE_IOCTL_EVENT_DIFF_RDI_PIX_CLK:
    /* 0: RDI cam and PIX cam use the same clk,
     * 1 - RDI and pix cams use different clks
     * so pause and resume is needed */
    tmp = *((uint32_t *)data->data);
    plugin->diff_clk_for_rdi_pix = (uint8_t)tmp;
      break;
    default:
      rc = camera_plugin_proc_oem_private_ioctl(plugin, client, data, status);
      break;
    }
  pthread_mutex_unlock (&client->client_mutex);
end:
  return rc;
}

static int camera_plugin_client_query_supported_feature (void *plugin_handle, uint32_t client_handle,
                       camera_plugin_supported_feature_t feature_type,
                       uint8_t *is_support)
{
  int rc = 0;

  *is_support = 0;
  switch(feature_type) {
  case CAMERA_PLUGIN_FEATURE_DIS_PLUGIN:
    break;
  case CAMERA_PLUGIN_FEATURE_STATS_NOTIFY:
    *is_support = 1;
    break;
  case CAMERA_PLUGIN_FEATURE_EXT_3A_ALGORITHM:
    *is_support = 1;
    break;
  case CAMERA_PLUGIN_FEATURE_EXT_SENSOR_CTRL:
    *is_support = 1;
    break;
  case CAMERA_PLUGIN_FEATURE_EXT_VFE_CTRL:
    *is_support = 1;
    break;
  case CAMERA_PLUGIN_FEATURE_EXT_ZOOM_TABLE:
    *is_support = 1;
    break;
  default:
    rc = -100;
    break;
  }
  return rc;
}

static int camera_plugin_client_open (void *handle, camera_plugin_client_ops_t *client_ops)
{
  int i;
  int rc = 0;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)handle;
  camera_plugin_client_t *client = NULL;

  pthread_mutex_lock (&plugin->mutex);
  for (i = 0; i < CAMERA_PLUGIN_CLIENT_MAX; i++) {
    if (!plugin->client[i].used) {
      /* found an empty client */
      client = &plugin->client[i];
      break;
    }
  }
  if (!client) {
    /* no client slot */
    CDBG_ERROR("%s: no empty plugin client index available, error out",  __func__);
    rc = -1;
    goto end;
  }
  memset (client,  0,  sizeof(camera_plugin_client_t));
  memset(client_ops,  0,  sizeof(camera_plugin_client_ops_t));
  pthread_mutex_init (&client->client_mutex, NULL);
  client->used = 1;
  plugin->handle_cnt++;
  client->client_handle |= i;
  if(client->client_handle >= 0xFFFFFF) {
    client->client_handle = 1;
  }
  client->client_ops.client_destroy = camera_plugin_client_destroy;
  client->client_ops.client_handle = client->client_handle;
  /* mctl calls this func to close the oem plugin */
  client->client_ops.client_init = camera_plugin_client_init;
  client->client_ops.client_destroy = camera_plugin_client_destroy;
  client->client_ops.stats_notify_to_plugin =
    camera_plugin_client_stats_notify_to_plugin;
  client->client_ops.dis_notify_to_plugin =
    camera_plugin_client_dis_notify_to_plugin;
  client->client_ops.isp_timing_notify_to_plugin =
    camera_plugin_client_isp_timing_notify_to_plugin;
  client->client_ops.get_sensor_parm_from_plugin =
    camera_plugin_client_get_sensor_parm_from_plugin; /* sensor_data_t */
  client->client_ops.sensor_notify_to_plugin =
    camera_plugin_client_sensor_notify_to_plugin;
  client->client_ops.private_ioctl_to_plugin =
    camera_plugin_client_private_ioctl_to_plugin;
  client->client_ops.query_supported_feature =
    camera_plugin_client_query_supported_feature;
  client->client_ops.grant_pix_interface =
    camera_plugin_grant_pix_interface;
  client->client_ops.diff_clk_for_rdi_camera =
    camera_plugin_diff_clk_for_rdi_camera;
  client->plugin_root = handle;
    *client_ops = client->client_ops;
end:
  CDBG("%s: plugin client open exit, rc = %d",
    __func__, rc);
  pthread_mutex_unlock (&plugin->mutex);
  return rc;
}

static void camera_plugin_destroy (void *handle)
{
  int i;
  camera_plugin_root_t *plugin = (camera_plugin_root_t *)handle;
  pthread_mutex_lock (&plugin->mutex);
  for (i = 0; i < CAMERA_PLUGIN_CLIENT_MAX; i++) {
    if (plugin->client[i].used) {
          plugin->client[i].used = 0;
      camera_plugin_client_destroy_proc (&plugin->client[i]);
    }
    CDBG("%s: plugin->client[%d].used: %d", __func__, i,
      plugin->client[i].used);
  }
  pthread_mutex_unlock (&plugin->mutex);
  pthread_mutex_destroy (&plugin->mutex);
  memset(&cam_plugin_root, 0, sizeof(cam_plugin_root));
}

void camera_plugin_create_func (camera_plugin_ops_t *camera_ops)
{
  memset(camera_ops, 0, sizeof(camera_plugin_ops_t));
  memset (&cam_plugin_root, 0, sizeof(cam_plugin_root));
  pthread_mutex_init (&cam_plugin_root.mutex, NULL);
  cam_plugin_root.my_ops.destroy = camera_plugin_destroy;
  cam_plugin_root.my_ops.handle = &cam_plugin_root;
  cam_plugin_root.my_ops.client_open =
    camera_plugin_client_open;
  /* if OEM supports simultaneous camera set
   	 cam_plugin_root.support_simultaneous_camera = 1 */
  cam_plugin_root.support_simultaneous_camera = 0;
  *camera_ops = cam_plugin_root.my_ops;
  CDBG("%s: handle = %p, client_open_fn = %p, destroy_fn = %p",
    __func__, camera_ops->handle,
    camera_ops->client_open, camera_ops->destroy);
}


