
/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <math.h>
#include "active_crop.h"

#undef CDBG
#define CDBG LOGE

/*===========================================================================
 * FUNCTION    - vfe_active_crop_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_active_crop_debug(VFE_ActiveRegionConfigCmdType* pcmd)
{
  CDBG("lastColorCompOfActiveRegion = %d\n",
    pcmd->lastColorCompOfActiveRegion);
  CDBG("firstColorCompOfActiveRegion = %d\n",
    pcmd->firstColorCompOfActiveRegion);
  CDBG("lastLineOfActiveRegion = %d\n",
    pcmd->lastLineOfActiveRegion);
  CDBG("firstLineOfActiveRegion = %d\n",
    pcmd->firstLineOfActiveRegion);
}

/*===========================================================================
 * FUNCTION    - vfe_active_crop_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_active_crop_init(active_crop_mod_t *mod, vfe_params_t* parm)
{
  memset(&mod->active_crop_cfg_cmd, 0x0, sizeof(active_crop_mod_t));
  return VFE_SUCCESS;
} /* vfe_active_crop_init */

/*===========================================================================
 * FUNCTION.   - vfe_active_crop_enable -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_active_crop_enable(active_crop_mod_t *mod,
  vfe_params_t *p_obj, int8_t enable, int8_t hw_write)
{
  mod->enable = enable;
  CDBG("%s, enable/disable active_crop module = %d",__func__, enable);
  return VFE_SUCCESS;
} /* vfe_active_crop_enable */

/*===========================================================================
 * FUNCTION    - vfe_active_crop_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_active_crop_config(active_crop_mod_t *mod,
  vfe_params_t* p_obj)
{
  uint32_t pixel_skip = 1;
  uint32_t line_skip = 1;
  if (!mod->enable) {
    CDBG("%s: active_crop not enabled", __func__);
    return VFE_SUCCESS;
  }

  mod->active_crop_cfg_cmd.firstColorCompOfActiveRegion = 0;
  //TODO: need to check if -1 to be subtracted from width
  mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion =
    ((p_obj->sensor_parms.lastPixel - p_obj->sensor_parms.firstPixel + 1) / pixel_skip) - 1;
  mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion += 1;
  mod->active_crop_cfg_cmd.firstLineOfActiveRegion = 0;
  mod->active_crop_cfg_cmd.lastLineOfActiveRegion =
    ((p_obj->sensor_parms.lastLine - p_obj->sensor_parms.firstLine + 1) / line_skip) - 1;
  CDBG("From CAMIF\n");
  vfe_active_crop_debug(&(mod->active_crop_cfg_cmd));

  if (IS_BAYER_FORMAT(p_obj)) {
    /* Bayer Input
     * The number of components on a line must be even for bayer. */
    mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion -=
      (mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion -
      mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion + 1) % 2;
  } else {
    /* YCbCr input */
    /* For YCbCr 422 data, the number of components must be
     * a multiple of four. */
    mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion -=
      (mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion -
       mod->active_crop_cfg_cmd.firstColorCompOfActiveRegion + 1) % 4;
  }

  p_obj->active_crop_info.firstPixel =
    mod->active_crop_cfg_cmd.firstColorCompOfActiveRegion;
  p_obj->active_crop_info.lastPixel =
    mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion;
  p_obj->active_crop_info.firstLine =
    mod->active_crop_cfg_cmd.firstLineOfActiveRegion;
  p_obj->active_crop_info.lastLine =
    mod->active_crop_cfg_cmd.lastLineOfActiveRegion;

  /* enable the command once region is defined */
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(p_obj->camfd, CMD_GENERAL,
    (void *) &(mod->active_crop_cfg_cmd), sizeof(mod->active_crop_cfg_cmd),
    VFE_CMD_ACTIVE_REGION_CFG)) {
    CDBG_HIGH("%s: active_crop config for operation mode = %d failed\n", __func__,
      p_obj->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }

  vfe_active_crop_debug(&(mod->active_crop_cfg_cmd));
  return VFE_SUCCESS;
} /* vfe_active_crop_config */

/*===========================================================================
 * FUNCTION    - vfe_get_active_crop_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_init_active_crop_info(active_crop_mod_t *mod,
  vfe_params_t* p_obj)
{
  uint32_t pixel_skip = 1;
  uint32_t line_skip = 1;

  CDBG("%s: Entered \n", __func__);
  mod->active_crop_cfg_cmd.firstColorCompOfActiveRegion = 0;
  //TODO: need to check if -1 to be subtracted from width
  mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion =
    ((p_obj->sensor_parms.lastPixel - p_obj->sensor_parms.firstPixel + 1) / pixel_skip) - 1;
#ifdef VFE_2X
  mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion += 1;
#endif
  mod->active_crop_cfg_cmd.firstLineOfActiveRegion = 0;
  mod->active_crop_cfg_cmd.lastLineOfActiveRegion =
    ((p_obj->sensor_parms.lastLine - p_obj->sensor_parms.firstLine + 1) / line_skip) - 1;

  if (IS_BAYER_FORMAT(p_obj)) {
    /* Bayer Input
     * The number of components on a line must be even for bayer. */
    mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion -=
      (mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion -
      mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion + 1) % 2;
  } else {
    /* YCbCr input */
    /* For YCbCr 422 data, the number of components must be
     * a multiple of four. */
    mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion -=
      (mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion -
       mod->active_crop_cfg_cmd.firstColorCompOfActiveRegion + 1) % 4;
  }

  p_obj->active_crop_info.firstPixel =
    mod->active_crop_cfg_cmd.firstColorCompOfActiveRegion;
  p_obj->active_crop_info.lastPixel =
    mod->active_crop_cfg_cmd.lastColorCompOfActiveRegion;
  p_obj->active_crop_info.firstLine =
    mod->active_crop_cfg_cmd.firstLineOfActiveRegion;
  p_obj->active_crop_info.lastLine =
    mod->active_crop_cfg_cmd.lastLineOfActiveRegion;

  p_obj->demosaic_op_params.first_pixel =
    p_obj->active_crop_info.firstPixel;
  p_obj->demosaic_op_params.last_pixel =
    p_obj->active_crop_info.lastPixel;
  p_obj->demosaic_op_params.first_line =
    p_obj->active_crop_info.firstLine;
  p_obj->demosaic_op_params.last_line =
    p_obj->active_crop_info.lastLine;

  vfe_active_crop_debug(&(mod->active_crop_cfg_cmd));
  return VFE_SUCCESS;
}

