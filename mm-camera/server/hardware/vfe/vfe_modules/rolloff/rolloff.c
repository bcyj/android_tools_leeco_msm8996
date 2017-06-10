/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <string.h>
#include <stdlib.h>
#include "camera_dbg.h"
#include "vfe_tgtcommon.h"
#include "vfe.h"

#ifdef ENABLE_ROLLOFF_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

#define USE_PCA(v) ((v == MSM8960V2) || (v == MSM8930))
/*===========================================================================
 * FUNCTION    - rolloff_normalize_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void rolloff_normalize_table(rolloff_mod_t* rolloff_ctrl,
  vfe_params_t* vfe_params, int is_left_table)
{
  int i, j;
  float min_value = 1.0, scaling_val;
  chromatix_parms_type *chrPtr = NULL;
  mesh_rolloff_array_type *inTbl = NULL;
  mesh_rolloff_array_type *outTbl = NULL;
  mesh_rolloff_array_type *deltaTbl = NULL;

  chrPtr = vfe_params->chroma3a;

  for (i = VFE_ROLLOFF_TL84_LIGHT; i < VFE_ROLLOFF_MAX_LIGHT; i++) {
    for (j = 0; j < MESH_ROLLOFF_SIZE; j++) {
      if (is_left_table) {
        outTbl = &(rolloff_ctrl->rolloff_tbls->left[i]);
        deltaTbl = &(vfe_params->rolloff_info.left[i]);
      } else {
        outTbl = &(rolloff_ctrl->rolloff_tbls->right[i]);
        deltaTbl = &(vfe_params->rolloff_info.right[i]);
      }

      if (i == VFE_ROLLOFF_LED_FLASH)
        inTbl = &(chrPtr->chromatix_mesh_rolloff_table_LED);
      else if (i == VFE_ROLLOFF_STROBE_FLASH)
        inTbl = &(chrPtr->chromatix_mesh_rolloff_table_Strobe);
      else
        inTbl = &(chrPtr->chromatix_mesh_rolloff_table[i]);

      /* RED Channel */
      outTbl->r_gain[j] = inTbl->r_gain[j] * deltaTbl->r_gain[j];
      if (outTbl->r_gain[j] < min_value)
        min_value = outTbl->r_gain[j];
      /* GR Channel */
      outTbl->gr_gain[j] = inTbl->gr_gain[j] * deltaTbl->gr_gain[j];
      if (outTbl->gr_gain[j] < min_value)
        min_value = outTbl->gr_gain[j];
      /* BLUE Channel */
      outTbl->b_gain[j] = inTbl->b_gain[j] * deltaTbl->b_gain[j];
      if (outTbl->b_gain[j] < min_value)
        min_value = outTbl->b_gain[j];
      /* GB Channel */
      outTbl->gb_gain[j] = inTbl->gb_gain[j] * deltaTbl->gb_gain[j];
      if (outTbl->gb_gain[j] < min_value)
        min_value = outTbl->gb_gain[j];
    }
    if (min_value >= 1.0)
      continue;

    scaling_val = 1.0 / min_value;

    for (j = 0; j < MESH_ROLLOFF_SIZE; j++) {
      /* RED Channel */
      outTbl->r_gain[j]  *= scaling_val;
      /* GR Channel */
      outTbl->gr_gain[j] *= scaling_val;
      /* BLUE Channel */
      outTbl->b_gain[j]  *= scaling_val;
      /* GB Channel */
      outTbl->gb_gain[j] *= scaling_val;
    }
    min_value = 1.0;
  }
} /* rolloff_normalize_table */

/*===========================================================================
 * FUNCTION    - rolloff_prepare_tables -
 *
 * DESCRIPTION:
 *==========================================================================*/
static vfe_status_t rolloff_prepare_tables(rolloff_mod_t* rolloff_ctrl,
  vfe_params_t* vfe_params)
{
  int i, k;
  vfe_rolloff_info_t *finalTbls = NULL;

#if 1
  /* Todo: Remove this once Sensor Provides Delta Tables */
  for (k = 0; k < 6; k++) {
    for (i = 0; i < 221; i++) {
      vfe_params->rolloff_info.left[k].r_gain[i] = 1.0;
      vfe_params->rolloff_info.left[k].b_gain[i] = 1.0;
      vfe_params->rolloff_info.left[k].gr_gain[i] = 1.0;
      vfe_params->rolloff_info.left[k].gb_gain[i] = 1.0;
    }
  }
#endif

  finalTbls = (vfe_rolloff_info_t *)malloc(sizeof(vfe_rolloff_info_t));
  if (!finalTbls) {
    CDBG_ERROR("%s: Not enough memory\n", __func__);
    return VFE_ERROR_GENERAL;
  }

  memset(finalTbls, 0x0, sizeof(vfe_rolloff_info_t));
  rolloff_ctrl->rolloff_tbls = finalTbls;

  /* Left frame tables */
  rolloff_normalize_table(rolloff_ctrl, vfe_params, TRUE);

  /* Right frame tables */
  if ((vfe_params->cam_mode == CAM_MODE_3D) &&
    vfe_params->stereocam_info.support_3d)
    rolloff_normalize_table(rolloff_ctrl, vfe_params, FALSE);

  return VFE_SUCCESS;
} /* rolloff_prepare_tables */

/*===========================================================================
 * Function:           vfe_rolloff_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_rolloff_enable(int mod_id, void *module, void *vparams,
  int8_t enable, int8_t hw_write)
{
  rolloff_mod_t *rolloff_ctrl = (rolloff_mod_t*)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (!IS_BAYER_FORMAT(vfe_params))
    enable = FALSE;
  vfe_params->moduleCfg->lensRollOffEnable = enable;

  CDBG("%s: enable=%d, hw_write=%d\n", __func__, enable, hw_write);

  if (hw_write && (rolloff_ctrl->rolloff_enable == enable))
    return VFE_SUCCESS;

  rolloff_ctrl->rolloff_enable = enable;

  if (hw_write) {
    rolloff_ctrl->hw_enable_cmd = TRUE;
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_ROLLOFF)
      : (vfe_params->current_config & ~VFE_MOD_ROLLOFF);
  }

  return VFE_SUCCESS;
} /* vfe_rolloff_enable */

#ifdef VFE_2X
/*===========================================================================
 * Function:           is_vfe_rolloff_reconfig
 *
 * Description:
 *=========================================================================*/
vfe_status_t is_vfe_rolloff_reconfig(vfe_ctrl_info_t *p_obj, void *parm)
{
  uint32_t *ptr = (uint32_t *)parm;

  *ptr = p_obj->vfe_module.rolloff_mod.vfe_reconfig;
  p_obj->vfe_module.rolloff_mod.vfe_reconfig = 0;
  return VFE_SUCCESS;
}
#endif

/*===========================================================================
 * FUNCTION    - vfe_rolloff_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_rolloff_init(int mod_id, void *module, void *vparams)
{
  rolloff_mod_t *rolloff_ctrl = (rolloff_mod_t*)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  vfe_status_t rc = VFE_ERROR_GENERAL;

  if (VFE_SUCCESS != rolloff_prepare_tables(rolloff_ctrl, vfe_params)) {
    rolloff_ctrl->rolloff_enable = FALSE;
    CDBG_ERROR("%s: ROLLOFF init failed\n", __func__);
    goto end;
  }

  rolloff_ctrl->rolloff_enable = TRUE;
  rolloff_ctrl->hw_enable_cmd = FALSE;
  if (rolloff_ctrl->rolloff_enable) {
    if (vfe_params->vfe_version == MSM8974)
      rc = mesh_rolloff_V4_init(&(rolloff_ctrl->mesh_v4_ctrl),
        rolloff_ctrl->rolloff_tbls, vfe_params);
    else if (USE_PCA(vfe_params->vfe_version)) {
      rc = pca_rolloff_init(&(rolloff_ctrl->pca_ctrl),
        rolloff_ctrl->rolloff_tbls, vfe_params);
    } else {
      rc = mesh_rolloff_init(&(rolloff_ctrl->mesh_ctrl),
        rolloff_ctrl->rolloff_tbls, vfe_params);
    }

    if (rc != VFE_SUCCESS)
      CDBG_ERROR("%s: ROLLOFF init failed\n", __func__);
  }

end:
  return rc;
} /* vfe_rolloff_init */

/*===========================================================================
 * FUNCTION    - vfe_rolloff_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_rolloff_deinit(int mod_id, void *module, void *vparams)
{
  rolloff_mod_t *rolloff_ctrl = (rolloff_mod_t*)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  if (rolloff_ctrl->rolloff_tbls)
    free(rolloff_ctrl->rolloff_tbls);
  rolloff_ctrl->rolloff_tbls = NULL;

  return VFE_SUCCESS;
} /* vfe_rolloff_init */

/*===========================================================================
 * FUNCTION    - vfe_rolloff_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_rolloff_config(int mod_id, void *module, void *vparams)
{
  rolloff_mod_t *rolloff_ctrl = (rolloff_mod_t*)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  vfe_status_t rc = VFE_SUCCESS;

  if (rolloff_ctrl->rolloff_enable) {
    if (vfe_params->vfe_version == MSM8974)
      rc = mesh_rolloff_V4_config(&(rolloff_ctrl->mesh_v4_ctrl), vfe_params);
    else if (USE_PCA(vfe_params->vfe_version))
      rc = pca_rolloff_config(&(rolloff_ctrl->pca_ctrl), vfe_params);
    else
      rc = mesh_rolloff_config(&(rolloff_ctrl->mesh_ctrl), vfe_params);

    if (rc != VFE_SUCCESS)
      CDBG_ERROR("%s: ROLLOFF config failed\n", __func__);
  }

end:
  return rc;
} /* vfe_rolloff_config */

/*===========================================================================
 * FUNCTION    - vfe_rolloff_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_rolloff_update(int mod_id, void *module, void *vparams)
{
  rolloff_mod_t *rolloff_ctrl = (rolloff_mod_t*)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  vfe_status_t rc = VFE_SUCCESS;

  if (rolloff_ctrl->hw_enable_cmd) {
    if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *)vfe_params->moduleCfg, sizeof(vfe_params->moduleCfg),
      VFE_CMD_MODULE_CFG)) {
      CDBG_HIGH("%s: Module config failed\n", __func__);
      return VFE_ERROR_GENERAL;
    }
    rolloff_ctrl->hw_enable_cmd = FALSE;
  }

  if (rolloff_ctrl->rolloff_enable) {
    if (vfe_params->vfe_version == MSM8974)
      rc = mesh_rolloff_V4_update(&(rolloff_ctrl->mesh_v4_ctrl), vfe_params);
    else if (USE_PCA(vfe_params->vfe_version))
      rc = pca_rolloff_update(&(rolloff_ctrl->pca_ctrl), vfe_params);
    else
      rc = mesh_rolloff_update(&(rolloff_ctrl->mesh_ctrl), vfe_params);

    if (rc != VFE_SUCCESS)
      CDBG_ERROR("%s: ROLLOFF update failed\n", __func__);
  }

end:
  return rc;
} /* vfe_rolloff_update */

/*===========================================================================
 * FUNCTION    - vfe_rolloff_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_rolloff_trigger_update(int mod_id, void *module,
  void *vparams)
{
  rolloff_mod_t *rolloff_ctrl = (rolloff_mod_t*)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  vfe_status_t rc = VFE_SUCCESS;

  if (rolloff_ctrl->rolloff_enable) {
    if (vfe_params->vfe_version == MSM8974)
      rc = mesh_rolloff_V4_trigger_update(&(rolloff_ctrl->mesh_v4_ctrl),
        rolloff_ctrl->rolloff_tbls, vfe_params);
    else if (USE_PCA(vfe_params->vfe_version))
      rc = pca_rolloff_trigger_update(&(rolloff_ctrl->pca_ctrl), vfe_params);
    else
      rc = mesh_rolloff_trigger_update(&(rolloff_ctrl->mesh_ctrl),
        rolloff_ctrl->rolloff_tbls, vfe_params);

    if (rc != VFE_SUCCESS)
      CDBG_ERROR("%s: ROLLOFF trigger update failed\n", __func__);
  }

end:
  return rc;
} /* vfe_rolloff_trigger_update */

/*=============================================================================
 * Function:               vfe_rolloff_trigger_enable
 *
 * Description:
 *============================================================================*/
vfe_status_t vfe_rolloff_trigger_enable(int mod_id, void *module, void *vparams,
  int enable)
{
  rolloff_mod_t *rolloff_ctrl = (rolloff_mod_t*)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  vfe_status_t rc = VFE_ERROR_GENERAL;

  if (vfe_params->vfe_version == MSM8974)
    rc = mesh_rolloff_V4_trigger_enable(&(rolloff_ctrl->mesh_v4_ctrl), vfe_params,
      enable);
  else if (USE_PCA(vfe_params->vfe_version))
    rc = pca_rolloff_trigger_enable(&(rolloff_ctrl->pca_ctrl), vfe_params,
      enable);
  else
    rc = mesh_rolloff_trigger_enable(&(rolloff_ctrl->mesh_ctrl), vfe_params,
      enable);

  if (rc != VFE_SUCCESS)
    CDBG_ERROR("%s: ROLLOFF trigger enable failed\n", __func__);

end:
  return rc;
} /* vfe_rolloff_trigger_enable */

/*===========================================================================
 * FUNCTION    - vfe_rolloff_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_rolloff_reload_params(int mod_id, void *module, void *vparams)
{
  rolloff_mod_t *rolloff_ctrl = (rolloff_mod_t*)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  vfe_status_t rc = VFE_ERROR_GENERAL;

  if (VFE_SUCCESS != rolloff_prepare_tables(rolloff_ctrl, vfe_params)) {
    rolloff_ctrl->rolloff_enable = FALSE;
    CDBG_ERROR("%s: ROLLOFF init failed\n", __func__);
    goto end;
  }

  if (vfe_params->vfe_version == MSM8974)
    rc = mesh_rolloff_V4_reload_params(&(rolloff_ctrl->mesh_v4_ctrl), vfe_params);
  else if (USE_PCA(vfe_params->vfe_version))
    rc = pca_rolloff_reload_params(&(rolloff_ctrl->pca_ctrl), vfe_params,
      rolloff_ctrl->rolloff_tbls);
  else
    rc = mesh_rolloff_reload_params(&(rolloff_ctrl->mesh_ctrl), vfe_params);

  if (rc != VFE_SUCCESS)
    CDBG_ERROR("%s: ROLLOFF trigger enable failed\n", __func__);

end:
  return rc;
} /* vfe_rolloff_reload_params */

/*===========================================================================
 * FUNCTION    - vfe_rolloff_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_rolloff_tv_validate(int mod_id, void *ip, void *op)
{
  vfe_PCA_Roll_off_test_vector_validation(ip, op);
  return VFE_SUCCESS;
}

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_rolloff_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_rolloff_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  rolloff_module_t *cmd = (rolloff_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

   if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) cmd, sizeof(rolloff_module_t),
    VFE_CMD_PCA_ROLL_OFF_UPDATE)) {
    CDBG_HIGH("%s: L frame update for operation mode = %d failed\n",
      __func__, vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }

  return VFE_SUCCESS;
}
#endif
