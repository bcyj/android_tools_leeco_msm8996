/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "camif.h"

#define ENABLE_CAMIF_LOGGING 1
#ifdef ENABLE_CAMIF_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*============================================================================
 * FUNCTION    - camif_init -
 *
 * DESCRIPTION: Configures camif module
 *===========================================================================*/
int  camif_init(camif_obj_t *p_camif)
{
  p_camif->camif_cmd.vSyncEdge = VFE_CAMIF_SYNC_EDGE_ActiveHigh;
  p_camif->camif_cmd.hSyncEdge = VFE_CAMIF_SYNC_EDGE_ActiveHigh;
  p_camif->camif_cmd.syncMode = VFE_CAMIF_SYNC_MODE_APS;
  p_camif->camif_cmd.vfeSubSampleEnable = VFE_DISABLE_CAMIF_TO_VFE_SUBSAMPLE;
  p_camif->camif_cmd.busSubSampleEnable = VFE_DISABLE_BUS_SUBSAMPLE;
  p_camif->camif_cmd.irqSubSampleEnable = VFE_DISABLE_IRQ_SUBSAMPLE;
  p_camif->camif_cmd.mipi_sel = 0;

  p_camif->camif_cmd.efsEndOfLine = 0;
  p_camif->camif_cmd.efsStartOfLine = 0;
  p_camif->camif_cmd.efsEndOfFrame = 0;
  p_camif->camif_cmd.efsStartOfFrame = 0;

  p_camif->camif_cmd.pixelsPerLine = 1600;
  p_camif->camif_cmd.linesPerFrame = 1200;

  p_camif->camif_cmd.firstPixel = 0;
  p_camif->camif_cmd.lastPixel = 1599;

  p_camif->camif_cmd.firstLine = 0;
  p_camif->camif_cmd.lastLine = 1199;

  p_camif->camif_cmd.pixelSkipMask = 0xFFFF;
  p_camif->camif_cmd.lineSkipMask = 0xFFFF;

  p_camif->camif_cmd.frameSkip = CAMIF_SUBSAMPLE_FRAME_SKIP_0;
  p_camif->camif_cmd.pixelSkipWrap = VFE_USE_ALL_16_BITS_OF_PIXEL_SKIP_PATTERN;

  p_camif->camif_cmd.epoch1Line = 0x3FFF;
  p_camif->camif_cmd.epoch2Line = 0x3FFF;
  p_camif->camif_cmd.epoch1IntEna = VFE_DISABLE_EPOCH_1_INTERRUPT;
  p_camif->camif_cmd.epoch2IntEna = VFE_DISABLE_EPOCH_2_INTERRUPT;

  return CAMERA_STATUS_SUCCESS;
}

/*============================================================================
 * FUNCTION    - camif_debug_params -
 *
 * DESCRIPTION: Configures camif module
 *===========================================================================*/
void camif_debug_params(camif_params_t *params)
{
  CDBG("%s:\n", __func__);
  CDBG("Mode %d", params->mode);
  CDBG("camif win %dx%d", params->camif_window.width,
    params->camif_window.height);
  CDBG("format %d", params->format);
  CDBG("fd %d", params->fd);
  CDBG("sensor dim %dx%d", params->sensor_dim.width,
    params->sensor_dim.height);
  CDBG("crop fp %d lp %d fl %d ll %d", params->sensor_crop_info.first_pixel,
    params->sensor_crop_info.last_pixel, params->sensor_crop_info.first_line,
    params->sensor_crop_info.last_line);
}/*camif_debug_params*/

/*============================================================================
 * FUNCTION    - camif_debug -
 *
 * DESCRIPTION:
 *===========================================================================*/
void camif_debug(VFE_CAMIFConfigCmdType *camif_config)
{
  CDBG("%s:\n", __func__);
  CDBG("VFE_CAMIFCfgCmd.vSyncEdge %d\n",camif_config->vSyncEdge);
  CDBG("VFE_CAMIFCfgCmd.hSyncEdge %d\n",camif_config->hSyncEdge);
  CDBG("VFE_CAMIFCfgCmd.syncMode %d\n",camif_config->syncMode);
  CDBG("VFE_CAMIFCfgCmd.vfeSubSampleEnable %d\n",
    camif_config->vfeSubSampleEnable);
  CDBG("VFE_CAMIFCfgCmd.busSubSampleEnable %d\n",
    camif_config->busSubSampleEnable);
  CDBG("VFE_CAMIFCfgCmd.irqSubSampleEnable %d\n",
    camif_config->irqSubSampleEnable);
  CDBG("VFE_CAMIFCfgCmd.mipi_sel %d\n",
    camif_config->mipi_sel);
  CDBG("VFE_CAMIFCfgCmd.efsEndOfLine %d\n",
    camif_config->efsEndOfLine);
  CDBG("VFE_CAMIFCfgCmd.efsStartOfLine %d\n",
    camif_config->efsStartOfLine);
  CDBG("VFE_CAMIFCfgCmd.efsEndOfFrame %d\n",
    camif_config->efsEndOfFrame);
  CDBG("VFE_CAMIFCfgCmd.efsStartOfFrame %d\n",
    camif_config->efsStartOfFrame);
  CDBG("VFE_CAMIFCfgCmd.firstPixel %d\n",camif_config->firstPixel);
  CDBG("VFE_CAMIFCfgCmd.lastPixel %d\n",camif_config->lastPixel);
  CDBG("VFE_CAMIFCfgCmd.firstLine %d\n",camif_config->firstLine);
  CDBG("VFE_CAMIFCfgCmd.lastLine %d\n",camif_config->lastLine);
  CDBG("VFE_CAMIFCfgCmd.pixelsPerLine %d\n",
    camif_config->pixelsPerLine);
  CDBG("VFE_CAMIFCfgCmd.linesPerFrame %d\n",
    camif_config->linesPerFrame);
  CDBG("VFE_CAMIFCfgCmd.pixelSkipMask %d\n",
    camif_config->pixelSkipMask);
  CDBG("VFE_CAMIFCfgCmd.lineSkipMask %d\n",
    camif_config->lineSkipMask);
  CDBG("VFE_CAMIFCfgCmd.frameSkip %d\n",camif_config->frameSkip);
  CDBG("VFE_CAMIFCfgCmd.pixelSkipWrap %d\n",
    camif_config->pixelSkipWrap);
}/*camif_debug*/

/*===========================================================================
FUNCTION      camif_send_cmd_to_hw

DESCRIPTION
===========================================================================*/
static int camif_send_cmd_to_hw(int fd, int type, void *pCmdData,
  unsigned int messageSize, int cmd_id)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd cfgCmd;
  vfe_message_t cmd;

  cmd.id = cmd_id;
  cmd.length = messageSize;
  cmd.value = pCmdData;

  cfgCmd.cmd_type = type;
  cfgCmd.length = sizeof(vfe_message_t);
  cfgCmd.value = &cmd;

  if ((rc = ioctl(fd, MSM_CAM_IOCTL_CONFIG_VFE, &cfgCmd)) < 0) {
    CDBG_ERROR("%s: MSM_CAM_IOCTL_CONFIG_VFE failed...%d %s\n", __func__, rc,
      strerror(errno));
    return CAMERA_STATUS_ERROR_GENERAL;
  }

  CDBG("%s: type = %d, Cmd = %d, length = %d\n",
    __func__, cfgCmd.cmd_type, cmd_id, messageSize);

  return CAMERA_STATUS_SUCCESS;
} /* camif_send_cmd_to_hw */

/*============================================================================
 * FUNCTION    - camif_config -
 *
 * DESCRIPTION: Configures camif module
 *===========================================================================*/
int camif_config(camif_obj_t *p_camif)
{
  camera_status_t status = CAMERA_STATUS_SUCCESS;
  VFE_CAMIFConfigCmdType *camif_config = &(p_camif->camif_cmd);
  camif_params_t *params = &(p_camif->params);
  uint32_t pixelSkip = 1;
  uint32_t lineSkip = 1;

  camif_config->pixelsPerLine = params->sensor_dim.width;
  camif_config->linesPerFrame = params->sensor_dim.height;
  camif_config->firstPixel = params->sensor_crop_info.first_pixel;
  camif_config->lastPixel = params->sensor_crop_info.last_pixel;
  camif_config->firstLine = params->sensor_crop_info.first_line;
  camif_config->lastLine = params->sensor_crop_info.last_line;

  /* TODO:  Calculate pixel skip and line skip */
  if ((pixelSkip + lineSkip) > 2)
    camif_config->vfeSubSampleEnable = 1;

  if (params->format == SENSOR_YCBCR) {
    /* For YUV sensor */
    switch (pixelSkip) {
      case 1:                    /* do nothing */
        break;

      case 2:
        camif_config->pixelSkipMask = 0xF0F0;
        break;

      case 3:
        camif_config->pixelSkipMask = 0xF000;
        camif_config->pixelSkipWrap =
          VFE_USE_12_MSB_BITS_OF_PIXEL_SKIP_PATTERN;
        break;

      case 4:
        camif_config->pixelSkipMask = 0xF000;
        break;

      default:
        /* force is to be 4 */
        pixelSkip = 4;
        camif_config->pixelSkipMask = 0xF000;
        break;
    }                           /* switch */
  } else {
    /* For BAYER sensor */
    switch (pixelSkip) {
      case 1:                    /* do nothing */
        break;

      case 2:
        camif_config->pixelSkipMask = 0xCCCC;
        break;

      case 3:
        camif_config->pixelSkipMask = 0xC300;
        camif_config->pixelSkipWrap =
          VFE_USE_12_MSB_BITS_OF_PIXEL_SKIP_PATTERN;
        break;

      case 4:
        camif_config->pixelSkipMask = 0xC0C0;
        break;

      case 5:
        pixelSkip = 6;           /* fall through */
      case 6:
        camif_config->pixelSkipMask = 0xC000;
        camif_config->pixelSkipWrap =
          VFE_USE_12_MSB_BITS_OF_PIXEL_SKIP_PATTERN;
        break;

      default:
        pixelSkip = 8;
        camif_config->pixelSkipMask = 0xC000;
        break;
    }
  }
  /* Update line skip in the VFE_CAMIFCfgCmd */
  switch (lineSkip) {
    case 1:                      /* do nothing */
      break;

    case 2:
      camif_config->lineSkipMask = 0xCCCC;
      break;

    case 3:
      lineSkip = 4;              /* fall through */
    case 4:
      camif_config->lineSkipMask = 0xC0C0;
      break;

    default:
      lineSkip = 8;
      camif_config->lineSkipMask = 0xC000;
      break;
  }

  if (params->connection_mode == 1)
     camif_config->mipi_sel = 1;
  else
     camif_config->mipi_sel = 0;
  camif_debug(camif_config);
  camif_debug_params(params);

  status = camif_send_cmd_to_hw(params->fd, CMD_GENERAL,
    (void *)camif_config, sizeof(*camif_config), VFE_CMD_CAMIF_CFG);

  return status;
} /* camif_config */
/*===========================================================================
 * FUNCTION    - camif_timer_config -
 *
 * DESCRIPTION: configure precision timer for strobe if needed
 *==========================================================================*/
int camif_timer_config(camif_obj_t *p_camif)
{
  return CAMERA_STATUS_SUCCESS;
} /* camif_timer_config */

/*===========================================================================
 * FUNCTION    - camif_command_ops -
 *
 * DESCRIPTION: process command ops
 *==========================================================================*/
int camif_command_ops(camif_obj_t *p_camif, void *data)
{
  camif_params_t *params = &(p_camif->params);
  int rc = 0;
  mod_cmd_t cmd = *(mod_cmd_t *)data;
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
   default:
     CDBG_ERROR("cmd_type = %d not supported", cmd.mod_cmd_ops);
     break;
  }
  if (0 != rc) {
    CDBG_ERROR("Failed configuring cmd_type = %d", cmd.mod_cmd_ops);
    return CAMERA_STATUS_ERROR_GENERAL;
  }
  return CAMERA_STATUS_SUCCESS;
} /* camif_command_ops */
