/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#include <math.h>

#include "camera_dbg.h"
#include "effects.h"
#include "mctl.h"

/*===========================================================================
FUNCTION      effects_init

DESCRIPTION
===========================================================================*/
void effects_init(effects_ctrl_t * effect)
{
  effect->nightshotInfo.minimum_value = CAMERA_NIGHTSHOT_MODE_OFF;
  effect->nightshotInfo.maximum_value = CAMERA_NIGHTSHOT_MODE_ON;
  effect->nightshotInfo.default_value = CAMERA_NIGHTSHOT_MODE_OFF;
  effect->nightshotInfo.current_value = CAMERA_NIGHTSHOT_MODE_OFF;
  effect->nightshotInfo.step_value = 1;

  effect->reflectInfo.minimum_value = CAMERA_NO_REFLECT;
  effect->reflectInfo.maximum_value = CAMERA_MAX_REFLECT - 1;
  effect->reflectInfo.default_value = CAMERA_NO_REFLECT;
  effect->reflectInfo.current_value = CAMERA_NO_REFLECT;
  effect->reflectInfo.step_value = 1;

  effect->antibandingInfo.minimum_value = 0;
  effect->antibandingInfo.maximum_value = CAMERA_MAX_ANTIBANDING - 1;
  effect->antibandingInfo.default_value = CAMERA_ANTIBANDING_OFF;
  effect->antibandingInfo.current_value = CAMERA_ANTIBANDING_OFF;
  effect->antibandingInfo.step_value = 1;

  effect->redEyeReductionInfo.minimum_value = FALSE;
  effect->redEyeReductionInfo.maximum_value = TRUE;
  effect->redEyeReductionInfo.default_value = FALSE;
  effect->redEyeReductionInfo.current_value = FALSE;
  effect->redEyeReductionInfo.step_value = 1;

  effect->brightnessInfo.minimum_value = CAMERA_MIN_BRIGHTNESS;
  effect->brightnessInfo.maximum_value = CAMERA_MAX_BRIGHTNESS;
  effect->brightnessInfo.default_value = CAMERA_DEF_BRIGHTNESS;
  effect->brightnessInfo.current_value = CAMERA_DEF_BRIGHTNESS;
  effect->brightnessInfo.step_value = 1;

  effect->contrastInfo.minimum_value = CAMERA_MIN_CONTRAST;
  effect->contrastInfo.maximum_value = CAMERA_MAX_CONTRAST;
  effect->contrastInfo.default_value = CAMERA_DEF_CONTRAST;
  effect->contrastInfo.current_value = CAMERA_DEF_CONTRAST;
  effect->contrastInfo.step_value = 1;

  effect->specialEffectsInfo.parm.minimum_value = CAMERA_EFFECT_OFF;
  effect->specialEffectsInfo.parm.maximum_value = CAMERA_EFFECT_SOLARIZE;
  effect->specialEffectsInfo.parm.default_value = CAMERA_EFFECT_OFF;
  effect->specialEffectsInfo.parm.current_value = CAMERA_EFFECT_OFF;
  effect->specialEffectsInfo.parm.step_value = 1;

  effect->sharpnessInfo.minimum_value = CAMERA_MIN_SHARPNESS;
  effect->sharpnessInfo.maximum_value = CAMERA_MAX_SHARPNESS;
  effect->sharpnessInfo.default_value = CAMERA_DEF_SHARPNESS;
  effect->sharpnessInfo.current_value = CAMERA_DEF_SHARPNESS;
  effect->sharpnessInfo.step_value = 1;

  effect->hueInfo.minimum_value = CAMERA_MIN_HUE;
  effect->hueInfo.maximum_value = CAMERA_MAX_HUE;
  effect->hueInfo.default_value = CAMERA_DEF_HUE;
  effect->hueInfo.current_value = CAMERA_DEF_HUE;
  effect->hueInfo.step_value = CAMERA_HUE_STEP;

  effect->saturationInfo.minimum_value = CAMERA_MIN_SATURATION;
  effect->saturationInfo.maximum_value = CAMERA_MAX_SATURATION;
  effect->saturationInfo.default_value = CAMERA_DEF_SATURATION;
  effect->saturationInfo.current_value = CAMERA_DEF_SATURATION;
  effect->saturationInfo.step_value = 1;

  /* Start with ISO set to AUTO */
  effect->isoInfo.minimum_value = (int) CAMERA_ISO_AUTO;
  effect->isoInfo.maximum_value = (int) CAMERA_ISO_MAX;
  effect->isoInfo.default_value = (int) CAMERA_ISO_AUTO;
  effect->isoInfo.current_value = (int) CAMERA_ISO_AUTO;
  effect->isoInfo.step_value = 1;

  effect->histogramInfo.minimum_value = CAMERA_HISTOGRAM_OFF;
  effect->histogramInfo.maximum_value = CAMERA_HISTOGRAM_YRGB;
  effect->histogramInfo.default_value = CAMERA_HISTOGRAM_OFF;
  effect->histogramInfo.current_value = CAMERA_HISTOGRAM_OFF;
  effect->histogramInfo.step_value = 1;

  effect->init = 1;
}

/*===========================================================================
 * FUNCTION    - effects_set_special_effect -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t effects_set_special_effect(void *ctrl, int32_t parm)
{
  int8_t rc = FALSE;
  int result = 0;
  float h,s,c;
  effects_ctrl_t *effect = NULL;
  mctl_config_ctrl_t* config_ctrl = (mctl_config_ctrl_t*)ctrl;

  if (!config_ctrl) {
    CDBG_ERROR("%s: mctl_config_ctrl_t is NULL", __func__);
    return FALSE;
  }

  effect = &(config_ctrl->effectCtrl);
  if (!effect->init) {
    CDBG_ERROR("%s: effects_ctrl_t is not inited", __func__);
    return FALSE;
  }

  CDBG("effects_set_special_effect: %d, cur =%d\n",
    parm, effect->specialEffectsInfo.parm.current_value);

  if (parm != effect->specialEffectsInfo.parm.current_value) {
    if (!rc && config_ctrl->sensorCtrl.sensor_output.output_format ==
		 SENSOR_YCBCR) {
      sensor_get_t sensor_get;
      sensor_set_t sensor_set;
      sensor_set.data.effect = parm;
      rc = config_ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
        config_ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
        SENSOR_SET_SPECIAL_EFFECT, &sensor_set, NULL);
    } else {
      h = (float)effect->hueInfo.current_value/
          (float)(CAMERA_MAX_HUE - CAMERA_MIN_HUE);
      s = (float)effect->saturationInfo.current_value /
          (float)(CAMERA_MAX_SATURATION - CAMERA_MIN_SATURATION);
      c = (float)effect->contrastInfo.current_value /
          (float)(CAMERA_MAX_CONTRAST - CAMERA_MIN_CONTRAST);

      vfe_effects_type_t eft = VFE_SPL_EFFECT;
      vfe_effects_params_t efv;
      efv.hue = h;
      efv.saturation = s;
      efv.contrast = c;
      efv.spl_effect = parm;
      result = config_ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
               config_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_EFFECTS,
               &eft, &efv);
      if (parm == CAMERA_EFFECT_OFF) {
        vfe_effects_type_t eft = VFE_SATURATION;
        result = config_ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
        config_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_EFFECTS,
        &eft, &efv);
      } else
	result = VFE_SUCCESS;
    }
  }
  if (parm == CAMERA_EFFECT_OFF)
    effect->specialEffectsInfo.spl_effects_enabled = FALSE;
  if (result == VFE_SUCCESS)
    rc = TRUE;
  return rc;
}

int8_t effects_set_contrast(void *ctrlBlk, int32_t parm)
{
  int8_t rc = FALSE;
  int result = 0;
  float h,s,c;
  effects_ctrl_t *effect = NULL;
  mctl_config_ctrl_t* config_ctrl = (mctl_config_ctrl_t*)ctrlBlk;

  if (!config_ctrl) {
    CDBG("%s: mctl_config_ctrl_t is NULL", __func__);
    return FALSE;
  }

  effect = &(config_ctrl->effectCtrl);
  if (!effect->init) {
    CDBG("%s: effects_ctrl_t is not inited", __func__);
    return FALSE;
  }
  if (effect->contrastInfo.current_value != parm) {
    if (config_ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR) {
      sensor_get_t sensor_get;
      sensor_set_t sensor_set;
      sensor_set.data.contrast = parm;
      rc = config_ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
        config_ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
        SENSOR_SET_CONTRAST, &sensor_set, NULL);
    } else {
      h = (float)effect->hueInfo.current_value/
          (float)(CAMERA_MAX_HUE - CAMERA_MIN_HUE);
      s = (float)effect->saturationInfo.current_value /
          (float)(CAMERA_MAX_SATURATION - CAMERA_MIN_SATURATION);
      c = (float)parm /
          (float)(CAMERA_MAX_CONTRAST - CAMERA_MIN_CONTRAST);
      CDBG_HIGH("%s: contrast %f", __func__, c);

      vfe_effects_type_t eft = VFE_CONTRAST;
      vfe_effects_params_t efv;
      efv.hue = h;
      efv.saturation = s;
      efv.contrast = c;
      efv.spl_effect = effect->specialEffectsInfo.parm.current_value;
      result = config_ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
        config_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_EFFECTS,
        &eft, &efv);
    }
  } else {
    result = VFE_SUCCESS;
  }
  effect->contrastInfo.current_value = parm;
  if (result == VFE_SUCCESS)
    rc = TRUE;
  return rc;
}

int8_t effects_set_hue(void *ctrlBlk, int32_t hue)
{
  int8_t rc = FALSE;
  int result = 0;
  float h,s,c;
  effects_ctrl_t *effect = NULL;
  mctl_config_ctrl_t* config_ctrl = (mctl_config_ctrl_t*)ctrlBlk;

  if (!config_ctrl) {
    CDBG("%s: mctl_config_ctrl_t is NULL", __func__);
    return FALSE;
  }

  effect = &(config_ctrl->effectCtrl);
  if (!effect->init) {
    CDBG("%s: effects_ctrl_t is not inited", __func__);
    return FALSE;
  }


  h = (float)hue/
      (float)(CAMERA_MAX_HUE - CAMERA_MIN_HUE);
  s = (float)effect->saturationInfo.current_value /
      (float)(CAMERA_MAX_SATURATION - CAMERA_MIN_SATURATION);
  c = (float)effect->contrastInfo.current_value /
      (float)(CAMERA_MAX_CONTRAST - CAMERA_MIN_CONTRAST);

  vfe_effects_type_t eft = VFE_HUE;
  vfe_effects_params_t efv;
  efv.hue = h;
  efv.saturation = s;
  efv.contrast = c;
  efv.spl_effect = effect->specialEffectsInfo.parm.current_value;
  result = config_ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
    config_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_EFFECTS,
    &eft, &efv);

  effect->hueInfo.current_value = hue;
  if (result == VFE_SUCCESS)
    rc = TRUE;
  return rc;
}

int8_t effects_set_saturation(void *ctrlBlk, int32_t saturation)
{
  int8_t rc = FALSE;
  int result = 0;
  float h,s,c;
  effects_ctrl_t *effect = NULL;
  mctl_config_ctrl_t* config_ctrl = (mctl_config_ctrl_t*)ctrlBlk;
  if (!config_ctrl) {
   CDBG("%s: mctl_config_ctrl_t is NULL", __func__);
    return FALSE;
  }
  effect = &(config_ctrl->effectCtrl);
  if (!effect->init) {
    CDBG("%s: effects_ctrl_t is not inited", __func__);
    return FALSE;
  }
  if (effect->saturationInfo.current_value != saturation) {
    if (!rc && config_ctrl->sensorCtrl.sensor_output.output_format ==
		SENSOR_YCBCR) {
      sensor_get_t sensor_get;
      sensor_set_t sensor_set;
      sensor_set.data.saturation = saturation;
      rc = config_ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
        config_ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
        SENSOR_SET_SATURATION, &sensor_set, NULL);
    } else {
      if (saturation != effect->saturationInfo.current_value) {
        h = (float)effect->hueInfo.current_value/
            (float)(CAMERA_MAX_HUE - CAMERA_MIN_HUE);
        s = (float)saturation /
            (float)(CAMERA_MAX_SATURATION - CAMERA_MIN_SATURATION);
        c = (float)effect->contrastInfo.current_value /
            (float)(CAMERA_MAX_CONTRAST - CAMERA_MIN_CONTRAST);
        vfe_effects_type_t eft = VFE_SATURATION;
        vfe_effects_params_t efv;
        efv.hue = h;
        efv.saturation = s;
        efv.contrast = c;
        efv.spl_effect = effect->specialEffectsInfo.parm.current_value;
        result = config_ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
          config_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_EFFECTS,
          &eft, &efv);
  	} else {
          result = VFE_SUCCESS;
	}
    }
  }
  effect->saturationInfo.current_value = saturation;
  if (result == VFE_SUCCESS)
    rc = TRUE;
  return rc;
}
