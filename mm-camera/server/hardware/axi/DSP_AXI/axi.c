/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <inttypes.h>
#include <media/msm_isp.h>
#include "camera_dbg.h"
#include "axi.h"

#define ENABLE_AXI_LOGGING 1
#ifdef ENABLE_AXI_LOGGING
#undef CDBG
#define CDBG LOGE
#endif

typedef enum {
  VFE_OUTPUT_PATH1 = 0x1,
  VFE_OUTPUT_PATH2,
  VFE_OUTPUT_PATH1_AND_PATH2,
} vfe_output_path_t;

static axi_obj_t *axi_get_obj(axi_comp_root_t *axi_root, axi_client_t *axi_client)
{
  axi_obj_t *axi_obj = NULL;
  /* now we have not implemented the use case of using 2 AXI obj */
  if(axi_client->obj_idx_mask == 1)
    axi_obj= &axi_root->axi_obj[0];
  else if(axi_client->obj_idx_mask == 2)
    axi_obj= &axi_root->axi_obj[1];
  return axi_obj;
}

/*===========================================================================
 * FUNCTION    - axi_cfg_cmd_print -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void axi_cfg_cmd_print(VFE_AXIOutputConfigCmdType *vcmd)
{
  CDBG("VFE_AXIOutputConfigCmd.outputMode = %d\n",
    vcmd->outputMode);
  CDBG("VFE_AXIOutputConfigCmd.format = %d\n",
    vcmd->format);
  CDBG("out1YImageHeight = %d\n",
    vcmd->out1YImageHeight);
  CDBG("VFE_AXIOutputConfigCmd.out1YImageWidthIn64BitWords = %d\n",
    vcmd->out1YImageWidthIn64BitWords);
  CDBG("out1YBurstLen = %d\n",
    vcmd->out1YBurstLen);
  CDBG("out1YNumRows = %d\n",
    vcmd->out1YNumRows);
  CDBG("out1YRowIncIn64bitIncs = %d\n",
    vcmd->out1YRowIncIn64bitIncs);
  CDBG("out1CbCrImageHeight = %d\n",
    vcmd->out1CbCrImageHeight);
  CDBG("out1CbCrImageWidthIn64BitWords = %d\n",
    vcmd->out1CbCrImageWidthIn64BitWords);
  CDBG("out1CbCrBurstLen = %d\n",
    vcmd->out1CbCrBurstLen);
  CDBG("out1CbCrNumRows = %d\n",
    vcmd->out1CbCrNumRows);
  CDBG("out1CbCrRowIncIn64bitIncs = %d\n",
    vcmd->out1CbCrRowIncIn64bitIncs);
  CDBG("out2YImageHeight = %d\n",
    vcmd->out2YImageHeight);
  CDBG("out2YImageWidthIn64BitWords = %d\n",
    vcmd->out2YImageWidthIn64BitWords);
  CDBG("out2YBurstLen = %d\n",
    vcmd->out2YBurstLen);
  CDBG("out2YNumRows = %d\n",
    vcmd->out2YNumRows);
  CDBG("out2YRowIncIn64bitIncs = %d\n",
    vcmd->out2YRowIncIn64bitIncs);
  CDBG("out2CbCrImageHeight = %d\n",
    vcmd->out2CbCrImageHeight);
  CDBG("out2CbCrImageWidthIn64BitWords = %d\n",
    vcmd->out2CbCrImageWidthIn64BitWords);
  CDBG("out2CbCrBurstLen = %d\n",
    vcmd->out2CbCrBurstLen);
  CDBG("out2CbCrNumRows = %d\n",
    vcmd->out2CbCrNumRows);
  CDBG("out2CbCrRowIncIn64bitIncs = %d\n",
    vcmd->out2CbCrRowIncIn64bitIncs);

} /* axi_cfg_cmd_print */

/*===========================================================================
 * FUNCTION    - axi_preview_config -
*
 * DESCRIPTION:
 *==========================================================================*/
static int axi_preview_config(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  vfe_ports_used_t vfe_port_used, VFE_AXIOutputConfigCmdType *cmd)
{
  axi_obj_t *axi_obj = NULL;
  uint32_t imageWidth = 0;
  uint32_t imageHeight = 0;
  CDBG("%s: begin", __func__);

  if(axi_client->obj_idx_mask)
    axi_obj = axi_get_obj(axi_root, axi_client);
  if(!axi_obj) {
    CDBG_ERROR("%s: no axi obj associated with teh client",  __func__);
    return -1;
  }
  memset(cmd, 0, sizeof(VFE_AXIOutputConfigCmdType));

  if (vfe_port_used == VFE_OUTPUT_SECONDARY) {
    imageWidth  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].image_width;
    imageHeight = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].image_height;
  } else {
    imageWidth  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_width;
    imageHeight = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_height;
  }

  cmd->format = VFE_8_BITS_PER_COLOR_COMP_8_BIT_BAYER;
  cmd->out1YBurstLen = VFE_BURST_LENGTH_4;
  cmd->out2YBurstLen = VFE_BURST_LENGTH_4;
  cmd->out1CbCrBurstLen = VFE_BURST_LENGTH_4;
  cmd->out2CbCrBurstLen = VFE_BURST_LENGTH_4;

  if (vfe_port_used == VFE_OUTPUT_SECONDARY) {
    /* Set the proper dimension, passed from the UI */
    cmd->out1YImageHeight = imageHeight;
    cmd->out1YImageWidthIn64BitWords = imageWidth / 8;
    cmd->out1YNumRows = imageHeight;
    cmd->out1YRowIncIn64bitIncs = imageWidth / 8;
    cmd->out1CbCrImageHeight = imageHeight / 2;
    cmd->out1CbCrImageWidthIn64BitWords = imageWidth / 8;
    cmd->out1CbCrNumRows = imageHeight / 2;
    cmd->out1CbCrRowIncIn64bitIncs = imageWidth / 8;
    cmd->outputMode = VFE_ONLY_OUTPUT_1_ENABLED;
 }

 if (vfe_port_used == VFE_OUTPUT_PRIMARY) {
   /* Set the proper dimension, passed from the UI */
   cmd->out2YImageHeight = imageHeight;
   cmd->out2YImageWidthIn64BitWords = imageWidth / 8;
   cmd->out2YNumRows = imageHeight;
   cmd->out2YRowIncIn64bitIncs = imageWidth / 8;
   cmd->out2CbCrImageHeight = imageHeight / 2;
   cmd->out2CbCrImageWidthIn64BitWords = imageWidth / 8;
   cmd->out2CbCrNumRows = imageHeight / 2;
   cmd->out2CbCrRowIncIn64bitIncs = imageWidth / 8;
   cmd->outputMode = VFE_ONLY_OUTPUT_2_ENABLED;
 }

  CDBG("axi_preview_config\n");
  axi_cfg_cmd_print(cmd);
  return 0;
} /* axi_preview_config */

/*===========================================================================
 * FUNCTION    - axi_raw_snapshot_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int axi_raw_snapshot_config(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  VFE_AXIOutputConfigCmdType *cmd)
{
  int status = 0;
  uint32_t num_pixels_per_64_bits = 0;
  uint32_t imageWidth = 0, imageHeight = 0;
  axi_obj_t *axi_obj = NULL;

  CDBG("%s begin ", __func__);
  if(axi_client->obj_idx_mask)
    axi_obj = axi_get_obj(axi_root, axi_client);
  if(!axi_obj) {
    CDBG_ERROR("%s: no axi obj associated with teh client",  __func__);
    return -1;
  }
  memset(cmd, 0, sizeof(VFE_AXIOutputConfigCmdType));

  cmd->format = VFE_8_BITS_PER_COLOR_COMP_8_BIT_BAYER;
  cmd->out1YBurstLen = VFE_BURST_LENGTH_16;
  cmd->out2YBurstLen = VFE_BURST_LENGTH_16;
  cmd->out1CbCrBurstLen = VFE_BURST_LENGTH_16;
  cmd->out2CbCrBurstLen = VFE_BURST_LENGTH_16;
  cmd->outputMode = VFE_CAMIF_TO_AXI_VIA_OUTPUT_2_ENABLED;

  switch (axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.sensor_data.sensor_raw_depth) {
    case SENSOR_8_BIT_DIRECT:
      cmd->format = VFE_8_BITS_PER_COLOR_COMP_8_BIT_BAYER;
      num_pixels_per_64_bits = 8;
      break;

    case SENSOR_10_BIT_DIRECT:
      cmd->format = VFE_10_BITS_PER_COLOR_COMP_10_BIT_BAYER;
      num_pixels_per_64_bits = 6;
      break;

    default:
      CDBG_ERROR("%s raw depth %d not supported yet.\n", __func__,
        axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.sensor_data.sensor_raw_depth);
      status = -EINVAL;
      break;
  }
  if(status < 0)
    return status;

  imageWidth  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_width;
  imageHeight = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_height;
  cmd->out2YImageHeight = imageHeight;
  cmd->out2YImageWidthIn64BitWords = imageWidth / 8;
  cmd->out2YNumRows = imageHeight;
  cmd->out2YRowIncIn64bitIncs = imageWidth / 8;

  cmd->out2CbCrImageHeight = imageHeight;
  cmd->out2CbCrNumRows = imageHeight;
  if (axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.sensor_data.sensor_output_format == SENSOR_BAYER) {
    cmd->out2CbCrImageWidthIn64BitWords =
      (imageWidth + num_pixels_per_64_bits - 1) /
      num_pixels_per_64_bits;
    cmd->out2CbCrRowIncIn64bitIncs =
      (imageWidth + num_pixels_per_64_bits - 1) /
      num_pixels_per_64_bits;
  } else {
    cmd->out2CbCrImageWidthIn64BitWords =
      (imageWidth + num_pixels_per_64_bits - 1) /
      num_pixels_per_64_bits * 2;
    cmd->out2CbCrRowIncIn64bitIncs =
      (imageWidth + num_pixels_per_64_bits - 1) /
      num_pixels_per_64_bits * 2;
  }
  CDBG("axi_raw_snapshot_config\n");
  axi_cfg_cmd_print(cmd);
  CDBG("%s: axi_write master 0 config\n", __func__);

  return status;
} /* axi_raw_snapshot_config */

/*===========================================================================
 * FUNCTION    - axi_snapshot_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int axi_snapshot_config(axi_comp_root_t *axi_root, axi_client_t *axi_client,
                               VFE_AXIOutputConfigCmdType *cmd)
{
  uint32_t imageWidth = 0, imageHeight = 0;
  axi_obj_t *axi_obj = NULL;

  CDBG("%s begin ", __func__);
  if(axi_client->obj_idx_mask)
    axi_obj = axi_get_obj(axi_root, axi_client);
  if(!axi_obj) {
    CDBG_ERROR("%s: no axi obj associated with teh client",  __func__);
    return -1;
  }
  memset(cmd, 0, sizeof(VFE_AXIOutputConfigCmdType));

  cmd->format = VFE_8_BITS_PER_COLOR_COMP_8_BIT_BAYER;
  cmd->out1YBurstLen = VFE_BURST_LENGTH_16;
  cmd->out2YBurstLen = VFE_BURST_LENGTH_16;
  cmd->out1CbCrBurstLen = VFE_BURST_LENGTH_16;
  cmd->out2CbCrBurstLen = VFE_BURST_LENGTH_16;
  cmd->outputMode = VFE_BOTH_OUTPUT_1_AND_2_ENABLED;

  imageWidth  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].image_width;
  imageHeight = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[SECONDARY].image_height;
  /* Set the proper dimension, passed from the UI */
  cmd->out1YImageHeight = imageHeight;
  cmd->out1YImageWidthIn64BitWords = imageWidth / 8;
  cmd->out1YNumRows = imageHeight;
  cmd->out1YRowIncIn64bitIncs = imageWidth / 8;
  cmd->out1CbCrImageHeight = imageHeight / 2;
  cmd->out1CbCrImageWidthIn64BitWords = imageWidth / 8;
  cmd->out1CbCrNumRows = imageHeight / 2;
  cmd->out1CbCrRowIncIn64bitIncs = imageWidth / 8;

  imageWidth  = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_width;
  imageHeight = axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info.output[PRIMARY].image_height;
  /* Set the proper dimension, passed from the UI */
  cmd->out2YImageHeight = imageHeight;
  cmd->out2YImageWidthIn64BitWords = imageWidth / 8;
  cmd->out2YNumRows = imageHeight;
  cmd->out2YRowIncIn64bitIncs = imageWidth / 8;
  cmd->out2CbCrImageHeight = imageHeight / 2;
  cmd->out2CbCrImageWidthIn64BitWords = imageWidth / 8;
  cmd->out2CbCrNumRows = imageHeight / 2;
  cmd->out2CbCrRowIncIn64bitIncs = imageWidth / 8;

  axi_cfg_cmd_print(cmd);
  return 0;
} /* axi_video_config */


/*===========================================================================
 * FUNCTION    - vfe_update_preview_format -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t vfe_update_preview_format(axi_client_t *axi_client, cam_format_t prev_format)
{
  return TRUE;
} /* vfe_update_preview_format */

/*===========================================================================
 * FUNCTION    - axi_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
int axi_config(axi_comp_root_t *axi_root, axi_client_t *axi_client, axi_config_t *cfg)
{
  int status = 0;
  struct msm_vfe_cfg_cmd cfgCmd;
  struct msm_isp_cmd ispcmd;
  VFE_AXIOutputConfigCmdType VFE_AXIOutputConfigCmd;

  ispcmd.id = VFE_CMD_AXI_OUT_CFG;
  ispcmd.length = sizeof(VFE_AXIOutputConfigCmd);
  ispcmd.value = (void *)&VFE_AXIOutputConfigCmd;

  switch(cfg->mode) {
  case CAM_OP_MODE_PREVIEW:
    status = axi_preview_config(axi_root, axi_client, cfg->vfe_port, &VFE_AXIOutputConfigCmd);
    cfgCmd.cmd_type = CMD_AXI_CFG_PRIM;
    cfgCmd.length = sizeof(struct msm_isp_cmd);
    cfgCmd.value = &ispcmd;
    break;
  case CAM_OP_MODE_VIDEO:
    status = axi_snapshot_config(axi_root, axi_client, &VFE_AXIOutputConfigCmd);
    cfgCmd.cmd_type = CMD_AXI_CFG_PRIM|CMD_AXI_CFG_SEC;
    cfgCmd.length = sizeof(struct msm_isp_cmd);
    cfgCmd.value = &ispcmd;
    break;
  case CAM_OP_MODE_SNAPSHOT:
    status = axi_snapshot_config(axi_root, axi_client, &VFE_AXIOutputConfigCmd);
    cfgCmd.cmd_type = CMD_AXI_CFG_PRIM|CMD_AXI_CFG_SEC;
    cfgCmd.length = sizeof(struct msm_isp_cmd);
    cfgCmd.value = &ispcmd;
    break;
  case CAM_OP_MODE_ZSL:
    status = axi_snapshot_config(axi_root, axi_client, &VFE_AXIOutputConfigCmd);
    cfgCmd.cmd_type = CMD_AXI_CFG_ZSL;
    cfgCmd.length = sizeof(struct msm_isp_cmd);
    cfgCmd.value = &ispcmd;
    break;
  case CAM_OP_MODE_RAW_SNAPSHOT:
    status = axi_raw_snapshot_config(axi_root, axi_client, &VFE_AXIOutputConfigCmd);
    cfgCmd.cmd_type = CMD_RAW_PICT_AXI_CFG;
    cfgCmd.length = sizeof(struct msm_isp_cmd);
    cfgCmd.value = &ispcmd;
    break;
  default:
    status = -EINVAL;
    break;
  }
  if (status < 0) {
    CDBG_HIGH("%s: axi_process failed for %d \n", __func__, cfg->mode);
    return status;
  }

  status = ioctl(axi_client->ops->fd, MSM_CAM_IOCTL_AXI_CONFIG, &cfgCmd);
  if(status < 0) {
    CDBG("%s: MSM_CAM_IOCTL_AXI_CONFIG failed!\n", __func__);
    return status;
  }
  CDBG("%s AXI Configuration success ", __func__);
  return status;
} /* axi_config */

int axi_set_params(axi_comp_root_t *axi_root, axi_client_t *axi_client, int type, axi_set_t *axi_set_parm)
{
  int rc = 0;
  axi_obj_t *axi_obj = NULL;

  switch(type) {
  case AXI_PARM_ADD_OBJ_ID:
    axi_client->obj_idx_mask = (1 << axi_set_parm->data.axi_obj_idx);
    axi_obj = &axi_root->axi_obj[axi_set_parm->data.axi_obj_idx];
    axi_obj->ref_count++;
    break;
  case AXI_PARM_HW_VERSION:
    axi_client->vfe_version = axi_set_parm->data.vfe_version;
    break;
  case AXI_PARM_OUTPUT_INFO:
    axi_obj = &axi_root->axi_obj[axi_set_parm->data.axi_obj_idx];
    axi_obj->ref_count++;
    axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.output_info =
      axi_set_parm->data.output_info;
    break;
  case AXI_PARM_RESERVE_INTF:
    axi_obj = &axi_root->axi_obj[axi_set_parm->data.axi_obj_idx];
    axi_obj->ref_count++;
    axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].client_idx = axi_client->client_idx;
    break;
  case AXI_PARM_PREVIEW_FORMAT:
    axi_obj = &axi_root->axi_obj[axi_set_parm->data.axi_obj_idx];
    axi_obj->ref_count++;
    axi_obj->axi_ctrl[axi_set_parm->data.intf_type].pix.prev_format = axi_set_parm->data.prev_format;
    break;
  case AXI_PARM_SNAPSHOT_FORMAT:
    axi_obj = &axi_root->axi_obj[axi_set_parm->data.axi_obj_idx];
    axi_obj->ref_count++;
    axi_obj->axi_ctrl[axi_set_parm->data.intf_type].pix.snap_format = axi_set_parm->data.snap_format;
    break;
  case AXI_PARM_RECORDING_FORMAT:
    axi_obj = &axi_root->axi_obj[axi_set_parm->data.axi_obj_idx];
    axi_obj->ref_count++;
    axi_obj->axi_ctrl[axi_set_parm->data.intf_type].pix.rec_format = axi_set_parm->data.rec_format;
    break;
  case AXI_PARM_THUMBNAIL_FORMAT:
    axi_obj = &axi_root->axi_obj[axi_set_parm->data.axi_obj_idx];
    axi_obj->ref_count++;
    axi_obj->axi_ctrl[axi_set_parm->data.intf_type].pix.thumb_format = axi_set_parm->data.thumb_format;
    break;
  case AXI_PARM_SENSOR_DATA:
    axi_obj = &axi_root->axi_obj[axi_set_parm->data.axi_obj_idx];
    axi_obj->ref_count++;
    axi_obj->axi_ctrl[AXI_INTF_PIXEL_0].pix.sensor_data =
      axi_set_parm->data.sensor_data;
    break;
  case AXI_PARM_STATS_VERSION:
  case AXI_PARM_UPDATE_CONFIG:
    break;
  default:
    CDBG_ERROR("%s Unsupported set parm type ", __func__);
    rc = -EINVAL;
    break;
  }
  return rc;
}

/*===========================================================================
 * FUNCTION    - axi_send_cmd -
 *
 * DESCRIPTION:
 *==========================================================================*/
int axi_send_cmd(int fd, int type,
  void *pCmdData, unsigned int messageSize, int cmd_id)
{
  int rc;
  struct msm_vfe_cfg_cmd cfgCmd;
  struct msm_isp_cmd ispcmd;

  ispcmd.id = 0;
  ispcmd.length = messageSize;
  ispcmd.value = pCmdData;

  cfgCmd.cmd_type = cmd_id;
  cfgCmd.length = sizeof(struct msm_isp_cmd);
  cfgCmd.value = &ispcmd;

  rc = ioctl(fd, MSM_CAM_IOCTL_AXI_CONFIG, &cfgCmd);
  if (rc < 0)
    CDBG_ERROR("%s: MSM_CAM_IOCTL_AXI_CONFIG failed %d\n",
      __func__, rc);
  return rc;
}

/*===========================================================================
 * FUNCTION    - axi_command_ops -
 *
 * DESCRIPTION:
 *==========================================================================*/
int axi_command_ops(axi_comp_root_t *axi_root, axi_client_t *axi_client,
               void *parm)
{
  int rc = 0;
  mod_cmd_t cmd = *(mod_cmd_t *)parm;
  CDBG("%s cmd_type = %d", __func__, cmd.mod_cmd_ops);
  switch(cmd.mod_cmd_ops) {
   case MOD_CMD_START:
     /* NOP */
     break;
   case MOD_CMD_STOP:
     /* NOP */
     break;
   case MOD_CMD_RESET:
     /* NOP */
     break;
   case MOD_CMD_ABORT:
     /* NOP */
     break;
   default:
     CDBG_ERROR("cmd_type = %d not supported", cmd.mod_cmd_ops);
     break;
  }
  if (0 != rc) {
    CDBG_ERROR("Failed configuring cmd_type = %d", cmd.mod_cmd_ops);
    return rc;
  }
  return rc;
}

/*===========================================================================
 * FUNCTION    - axi_command_ops -
 *
 * DESCRIPTION:
 *==========================================================================*/
int axi_proc_reg_update(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  void *parm)
{
	return 0;
}
int axi_proc_unregister_wms(axi_comp_root_t *axi_root, axi_client_t *axi_client,
  void *parm)
{
	return 0;
}
