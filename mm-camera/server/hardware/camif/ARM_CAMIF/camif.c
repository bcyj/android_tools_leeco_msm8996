/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "camif.h"

#ifdef ENABLE_CAMIF_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*============================================================================
 * FUNCTION    - camif_init -
 *
 * DESCRIPTION: Configures camif module
 *===========================================================================*/
int camif_init(camif_obj_t *p_camif)
{
  p_camif->camif_cmd.vSyncEdge = VFE_CAMIF_SYNC_EDGE_ActiveHigh;
  p_camif->camif_cmd.hSyncEdge = VFE_CAMIF_SYNC_EDGE_ActiveHigh;
  p_camif->camif_cmd.syncMode = VFE_CAMIF_SYNC_MODE_APS;
  p_camif->camif_cmd.vfeSubSampleEnable = VFE_DISABLE_CAMIF_TO_VFE_SUBSAMPLE;
  p_camif->camif_cmd.busSubSampleEnable = VFE_DISABLE_BUS_SUBSAMPLE;
  p_camif->camif_cmd.camif2OutputEnable = 1;
  p_camif->camif_cmd.camif2busEnable = 0;
#ifndef VFE_40
  p_camif->camif_cmd.irqSubSampleEnable = VFE_DISABLE_IRQ_SUBSAMPLE;
#endif
  p_camif->camif_cmd.binningEnable = FALSE;
  /* TBD: need to use HW version information so that VFE_31 canbe removed */
#ifndef VFE_31
  p_camif->camif_cmd.frameBasedEnable = FALSE;
#endif
  p_camif->camif_cmd.misrEnable = FALSE;
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
  p_camif->camif_cmd.skipMode = FALSE;
  p_camif->camif_cmd.pixelSkipWrap = VFE_USE_ALL_16_BITS_OF_PIXEL_SKIP_PATTERN;
#ifdef VFE_40
  p_camif->camif_cmd.IRQSubsamplePattern = 0xFFFFFFFF;
#endif
  p_camif->camif_cmd.epoch1Line = 0x3FFF;
  p_camif->camif_cmd.epoch2Line = 0x3FFF;

  p_camif->params.strobe_info.enabled = 0;
  return 0;
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
  CDBG("Strobe enabled %d dur %ld", params->strobe_info.enabled,
     params->strobe_info.duration);
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
  CDBG("VFE_CAMIFCfgCmd.camif2OutputEnable %d\n",
    camif_config->camif2OutputEnable);
#ifndef VFE_40
  CDBG("VFE_CAMIFCfgCmd.irqSubSampleEnable %d\n",
    camif_config->irqSubSampleEnable);
#endif
  CDBG("VFE_CAMIFCfgCmd.binningEnable %d\n",
    camif_config->binningEnable);
  CDBG("VFE_CAMIFCfgCmd.misrEnable %d\n",
    camif_config->misrEnable);
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
  CDBG("VFE_CAMIFCfgCmd.frameSkipMode %d\n",camif_config->skipMode);
  CDBG("VFE_CAMIFCfgCmd.pixelSkipWrap %d\n",
    camif_config->pixelSkipWrap);
  #ifdef VFE_40
    CDBG("VFE_CAMIFCfgCmd.IRQSubsamplePattern %d\n",
      camif_config->IRQSubsamplePattern);
  #endif
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
    return rc;
  }

  CDBG("%s: type = %d, Cmd = %d, length = %d\n",
    __func__, cfgCmd.cmd_type, cmd_id, messageSize);

  return rc;
} /* camif_send_cmd_to_hw */

/*============================================================================
 * FUNCTION    - camif_config -
 *
 * DESCRIPTION: Configures camif module
 *===========================================================================*/
int camif_config(camif_obj_t *p_camif)
{
  int rc = 0;
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

  if (params->strobe_info.enabled && (params->mode == CAM_OP_MODE_SNAPSHOT)) {
    camif_config->skipMode = 1;
    camif_config->frameSkip =
      CAMIF_SUBSAMPLE_FRAME_SKIP_ONE_OUT_OF_EVERY_2Frame;
  } else {
    camif_config->skipMode = 0;
    camif_config->frameSkip = CAMIF_SUBSAMPLE_FRAME_SKIP_0;
  }

  if (params->mode == CAM_OP_MODE_RAW_SNAPSHOT) {
    p_camif->camif_cmd.camif2OutputEnable = 0;
    p_camif->camif_cmd.camif2busEnable = 1;
  } else {
    p_camif->camif_cmd.camif2OutputEnable = 1;
    p_camif->camif_cmd.camif2busEnable = 0;
  }
  camif_debug(camif_config);
  camif_debug_params(params);

  rc = camif_send_cmd_to_hw(params->fd, CMD_GENERAL,
    (void *)camif_config, sizeof(*camif_config), VFE_CMD_CAMIF_CFG);

  return rc;
} /* camif_config */

/*===========================================================================
 * FUNCTION    - camif_timer_config -
 *
 * DESCRIPTION: configure precision timer for strobe if needed
 *==========================================================================*/
int camif_timer_config(camif_obj_t *p_camif)
{
  camif_params_t *params = &(p_camif->params);

  p_camif->sync_timer_cmd.whichSyncTimer = 0;
  p_camif->sync_timer_cmd.operation = 1;
  p_camif->sync_timer_cmd.polarity = 0;
  p_camif->sync_timer_cmd.repeatCount = 0;
  p_camif->sync_timer_cmd.pclkCount = 0;
  p_camif->sync_timer_cmd.hsyncCount = params->camif_window.height;
  p_camif->sync_timer_cmd.outputDuration = params->strobe_info.duration;

  camif_debug_params(params);
  if (CAMERA_STATUS_SUCCESS != camif_send_cmd_to_hw(params->fd, CMD_GENERAL,
    (void *) &(p_camif->sync_timer_cmd), sizeof(VFE_SyncTimerCmdType),
    VFE_CMD_SYNC_TIMER_SETTING)) {
    CDBG_ERROR("%s: VFE_CMD_SYNC_TIMER_SETTING failed\n", __func__);
    return CAMERA_STATUS_ERROR_GENERAL;
  }

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
