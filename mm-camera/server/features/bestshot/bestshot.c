/*============================================================================

   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <math.h>

#include "camera_dbg.h"
#include "bestshot.h"
#include "mctl.h"

const camera_bestshot_config_t bestshot_config_table[] =
{
#include "bestshot_config.h"
};

/*===========================================================================
FUNCTION      bestshot_init

DESCRIPTION
===========================================================================*/
void bestshot_init(bestshot_ctrl_t * bestshot)
{
  CDBG("%s: ",__func__);
  bestshot->bestshotModeInfo.parm.minimum_value = CAMERA_BESTSHOT_OFF;
  bestshot->bestshotModeInfo.parm.maximum_value = CAMERA_BESTSHOT_MAX;
  bestshot->bestshotModeInfo.parm.default_value = CAMERA_BESTSHOT_OFF;
  bestshot->bestshotModeInfo.parm.current_value = CAMERA_BESTSHOT_OFF;
  bestshot->bestshotModeInfo.parm.step_value = 1;
  /*default mode values, just in case */
  bestshot->bestshotdefModeInfo.iso_mode = CAMERA_ISO_AUTO;
  bestshot->bestshotdefModeInfo.aec_mode = CAMERA_AEC_CENTER_WEIGHTED;
  bestshot->bestshotdefModeInfo.af_mode_value = AF_MODE_MACRO;
  bestshot->bestshotdefModeInfo.WB_value = CAMERA_WB_AUTO;
  bestshot->bestshotdefModeInfo.effect_value = CAMERA_EFFECT_OFF;
  bestshot->bestshotdefModeInfo.fps_mode = FPS_MODE_AUTO;
  bestshot->bestshotdefModeInfo.antibanding = CAMERA_ANTIBANDING_AUTO;
  bestshot->soft_focus_dgr = 1.0;
} /* bestshot_init */

/*===========================================================================
FUNCTION      bestshot_set_mode_config

DESCRIPTION
===========================================================================*/
static int bestshot_set_mode_config(void *cctrl, camera_bestshot_mode_type mode,
                                    void *config)
{
  CDBG("%s: ",__func__);
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cctrl;
  camera_bestshot_config_t *bestshot_mode = (camera_bestshot_config_t *)config;
  stats_proc_interface_output_t *sp_output =
    &(ctrl->stats_proc_ctrl.intf.output);
  stats_proc_set_t stat_set_param;

  stat_set_param.type = STATS_PROC_AEC_TYPE;
  stat_set_param.d.set_aec.type = AEC_BESTSHOT;
  stat_set_param.d.set_aec.d.bestshot_mode = mode;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
  ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, stat_set_param.type, &stat_set_param,
     &(ctrl->stats_proc_ctrl.intf));

  stat_set_param.type = STATS_PROC_AWB_TYPE;
  stat_set_param.d.set_awb.type = AWB_BESTSHOT;
  stat_set_param.d.set_awb.d.bestshot_mode = mode;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
  ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, stat_set_param.type, &stat_set_param,
     &(ctrl->stats_proc_ctrl.intf));

  stat_set_param.type = STATS_PROC_AF_TYPE;
  stat_set_param.d.set_af.type = AF_BESTSHOT;
  stat_set_param.d.set_af.d.bestshot_mode = mode;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
  ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, stat_set_param.type, &stat_set_param,
     &(ctrl->stats_proc_ctrl.intf));

  if(ctrl->stats_proc_ctrl.intf.output.af_d.reset_lens &&
    ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle) {
      CDBG("%s: Reset lens", __func__);
    rc = ctrl->comp_ops[MCTL_COMPID_ACTUATOR].set_params(
      ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle,
        ACTUATOR_DEF_FOCUS, NULL, NULL);
    if (rc != 0) {
      CDBG_ERROR("%s(%d)Failure:Reset lens failed\n",
       __FILE__, __LINE__);
    }
  }

  stat_set_param.type = STATS_PROC_ASD_TYPE;
  stat_set_param.d.set_aec.type = ASD_BESTSHOT;
  stat_set_param.d.set_aec.d.bestshot_mode = mode;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
             ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
             stat_set_param.type, &stat_set_param,
             &(ctrl->stats_proc_ctrl.intf));

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
             ctrl->comp_ops[MCTL_COMPID_VFE].handle,
             VFE_SET_AEC_PARAMS, NULL, NULL);
  if (rc != VFE_SUCCESS)
    CDBG_HIGH("%s VFE Set AEC params failed ", __func__);

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
             ctrl->comp_ops[MCTL_COMPID_VFE].handle,
             VFE_SET_AWB_PARMS, NULL, NULL);
  if (rc != VFE_SUCCESS)
    CDBG_HIGH("%s VFE SET AWB params failed ", __func__);

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
             ctrl->comp_ops[MCTL_COMPID_VFE].handle,
             VFE_SET_ASD_PARMS, NULL, NULL);
  if (rc != VFE_SUCCESS)
    CDBG_HIGH("%s VFE SET ASD params failed ", __func__);

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
             ctrl->comp_ops[MCTL_COMPID_VFE].handle,
             VFE_SET_BESTSHOT, &mode, NULL);
  if (rc != VFE_SUCCESS)
    CDBG_HIGH("%s VFE Set bestshot params failed ", __func__);

  return TRUE;
} /* bestshot_set_mode_config */

static int reload_chromatix(mctl_config_ctrl_t* ctrl,
                            sensor_load_chromatix_t new_chroma_type)
{
  sensor_set_t set_param;
  sensor_get_t get_param;
  int rc = 0;

  set_param.data.chromatix_type = new_chroma_type;

  ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
	    ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
	    SENSOR_SET_CHROMATIX_TYPE, &set_param, NULL);

  ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
	    ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
	    SENSOR_GET_CHROMATIX_PTR, &get_param, sizeof(get_param));
  ctrl->chromatix_ptr = get_param.data.chromatix_ptr;

  if(ctrl->chromatix_ptr) {
	rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
	           ctrl->comp_ops[MCTL_COMPID_VFE].handle,
               VFE_SET_CHROMATIX_PARM, ctrl->chromatix_ptr, NULL);

    if (rc < 0)
      CDBG_ERROR("%s VFE Set Chromatix parm failed ", __func__);
  }


  return 0;
}

/*===========================================================================
 * FUNCTION    - bestshot_set_mode -
 *
 * DESCRIPTION:
 *==========================================================================*/
int bestshot_set_mode(void *cctrl, bestshot_ctrl_t *bestshot, uint8_t parm)
{
  uint32_t bestshot_parm;
  mctl_config_ctrl_t *ctrl  = (mctl_config_ctrl_t *)cctrl;
  camera_bestshot_config_t *bestshot_mode;
  sensor_load_chromatix_t cur_chroma_type, new_chroma_type;

  if (bestshot->bestshotModeInfo.parm.current_value == parm){
      CDBG("%s: current= parm, skip set mode",__func__);
    return TRUE;
  }
  bestshot_parm = parm;
  /* bestshot_parm is unsigned, therefore, cannot be < 0 */
  /* This is a CR fix for removing all the warnings */
  if (bestshot_parm >= CAMERA_BESTSHOT_MAX) {
    CDBG("%s: Invalid mode %d, returning..\n", __func__, bestshot_parm);
    return FALSE;
  }
  CDBG("%s: new mode value = %d\n", __func__, parm);
  bestshot_mode = (camera_bestshot_config_t *)(&(bestshot_config_table[parm]));

  new_chroma_type = bestshot_mode->chromatix_type;
  cur_chroma_type = bestshot_config_table[
    bestshot->bestshotModeInfo.parm.current_value].chromatix_type;
  CDBG("%s: new_chroma_type =%d, cur_chroma_type =%d, current_value=%d", __func__,
    new_chroma_type, cur_chroma_type,
    bestshot->bestshotModeInfo.parm.current_value);
  if(new_chroma_type != cur_chroma_type) {
    CDBG("%s: re-load chromatix", __func__);
    reload_chromatix(ctrl,new_chroma_type);
  } else {
    CDBG("%s: No need to re-load chromatix", __func__);
  }

  /* Now call config function to actually set the mode */
  if (!bestshot_set_mode_config(ctrl, bestshot_parm, bestshot_mode)) {
    CDBG("%s: set_mode_config failure\n", __func__);
    return FALSE;
  }

  bestshot->bestshotModeInfo.parm.current_value = parm;
  return TRUE;
} /* bestshot_set_mode */

/*===========================================================================
 * FUNCTION    - bestshot_reconfig_mode -
 *
 * DESCRIPTION:
 *==========================================================================*/
int bestshot_reconfig_mode(void *cctrl, bestshot_ctrl_t *bestshot)
{
  uint32_t bestshot_parm;
  mctl_config_ctrl_t *ctrl  = (mctl_config_ctrl_t *)cctrl;
  camera_bestshot_config_t *bestshot_mode;
  sensor_load_chromatix_t new_chroma_type;
  bestshot_parm = bestshot->bestshotModeInfo.parm.current_value;

  if (bestshot_parm >= CAMERA_BESTSHOT_MAX) {
    CDBG("%s: Invalid mode %d, returning..\n", __func__, bestshot_parm);
    return FALSE;
  }
  CDBG("%s: new mode value = %d\n", __func__, bestshot_parm);
  bestshot_mode = (camera_bestshot_config_t *)(&(bestshot_config_table[bestshot_parm]));
  new_chroma_type = bestshot_mode->chromatix_type;

  CDBG("%s: new_chroma_type =%d, current_value=%d", __func__,
  new_chroma_type, bestshot_parm);

  CDBG("%s: re-load chromatix", __func__);
  //reload_chromatix(ctrl,new_chroma_type);

  /* Now call config function to actually set the mode */
  if (!bestshot_set_mode_config(ctrl, bestshot_parm, bestshot_mode)) {
    CDBG("%s: set_mode_config failure\n", __func__);
    return FALSE;
  }

  return TRUE;
} /* bestshot_reconfig_mode */
