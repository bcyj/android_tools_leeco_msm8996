/*==============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/
#include "mesh_rolloff.h"
#include "camera_dbg.h"
#include "vfe_tgtcommon.h"

#ifdef ENABLE_MESH_ROLLOFF_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*==============================================================================
 * Function:           mesh_rolloff_table_interpolate
 *
 * Description:
 *============================================================================*/
static void mesh_rolloff_table_interpolate(mesh_rolloff_array_type *in1,
  mesh_rolloff_array_type *in2, mesh_rolloff_array_type *out, float ratio)
{
  int i = 0;
  out->mesh_rolloff_table_size = 13*17;
  TBL_INTERPOLATE(in1->r_gain, in2->r_gain, out->r_gain, ratio,
    out->mesh_rolloff_table_size, i);
  TBL_INTERPOLATE(in1->gb_gain, in2->gb_gain, out->gb_gain, ratio,
    out->mesh_rolloff_table_size, i);
  TBL_INTERPOLATE(in1->gr_gain, in2->gr_gain, out->gr_gain, ratio,
    out->mesh_rolloff_table_size, i);
  TBL_INTERPOLATE(in1->b_gain, in2->b_gain, out->b_gain, ratio,
    out->mesh_rolloff_table_size, i);
} /* mesh_rolloff_table_interpolate */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_calc_flash_trigger -
 *
 * DESCRIPTION:
 *============================================================================*/
static void mesh_rolloff_calc_flash_trigger(mesh_rolloff_array_type *tblCCT,
  mesh_rolloff_array_type *tblOut, vfe_rolloff_info_t *mesh_tbls,
  vfe_params_t *vfe_params)
{
  float ratio;
  float flash_start, flash_end;
  mesh_rolloff_array_type *tblFlash = NULL;
  vfe_flash_parms_t *flash_params = &(vfe_params->flash_params);
  chromatix_parms_type *chrPtr = vfe_params->chroma3a;

  ratio = flash_params->sensitivity_led_off / flash_params->sensitivity_led_hi;

  if ((int)flash_params->flash_mode == VFE_FLASH_STROBE) {
    tblFlash = &(mesh_tbls->left[VFE_ROLLOFF_STROBE_FLASH]);
    flash_start = chrPtr->rolloff_Strobe_start;
    flash_end = chrPtr->rolloff_Strobe_end;
  } else {
    tblFlash = &(mesh_tbls->left[VFE_ROLLOFF_LED_FLASH]);
    flash_start = chrPtr->rolloff_LED_start;
    flash_end = chrPtr->rolloff_LED_end;
  }

  CDBG("%s: flash_start %5.2f flash_end %5.2f \n", __func__, flash_start,
    flash_end);

  if (ratio >= flash_end)
    *tblOut = *tblFlash;
  else if (ratio <= flash_start)
    *tblOut = *tblCCT;
  else
    mesh_rolloff_table_interpolate(tblCCT, tblFlash, tblOut,
        ratio/(flash_end - flash_start));
} /* mesh_rolloff_calc_flash_trigger */

/*==============================================================================
 * Function:           mesh_rolloff_calc_awb_trigger
 *
 * Description:
 *============================================================================*/
static void mesh_rolloff_calc_awb_trigger(mesh_rolloff_array_type *tblOut,
  vfe_rolloff_info_t *mesh_tbls, vfe_params_t *vfe_params)
{
  float ratio = 0.0;
  cct_trigger_info trigger_info;
  awb_cct_type cct_type;
  chromatix_parms_type *chrPtr = vfe_params->chroma3a;

  trigger_info.mired_color_temp = MIRED(vfe_params->awb_params.color_temp);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_A,
    chrPtr->rolloff_A_trigger_snapshot);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d65,
    chrPtr->rolloff_D65_trigger_snapshot);

  cct_type = vfe_util_get_awb_cct_type(&trigger_info, vfe_params);

  switch (cct_type) {
    case AWB_CCT_TYPE_A:
      *tblOut = mesh_tbls->left[VFE_ROLLOFF_A_LIGHT];
      break;
    case AWB_CCT_TYPE_TL84_A:
      ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_A.mired_start, trigger_info.trigger_A.mired_end);
      mesh_rolloff_table_interpolate(&(mesh_tbls->left[VFE_ROLLOFF_TL84_LIGHT]),
        &(mesh_tbls->left[VFE_ROLLOFF_A_LIGHT]), tblOut, ratio);
      break;
    case AWB_CCT_TYPE_D65_TL84:
      ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_d65.mired_end,
        trigger_info.trigger_d65.mired_start);
      mesh_rolloff_table_interpolate(&(mesh_tbls->left[VFE_ROLLOFF_D65_LIGHT]),
        &(mesh_tbls->left[VFE_ROLLOFF_TL84_LIGHT]), tblOut, ratio);
      break;
    case AWB_CCT_TYPE_D65:
      *tblOut = mesh_tbls->left[VFE_ROLLOFF_D65_LIGHT];
      break;
    case AWB_CCT_TYPE_TL84:
    default:
      *tblOut = mesh_tbls->left[VFE_ROLLOFF_TL84_LIGHT];
      break;
  }
} /* mesh_rolloff_calc_awb_trigger */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_prepare_hw_table -
 *
 * DESCRIPTION:
 *============================================================================*/
static void mesh_rolloff_prepare_hw_table(const MESH_RollOffTable *pIn,
  MESH_RollOffConfigCmdType *cmd)
{
  uint16_t i;
  uint32_t data;

  const uint16_t* pInitGr = pIn->initTableGr;
  const uint16_t* pInitGb = pIn->initTableGb;
  const uint16_t* pInitB =  pIn->initTableB;
  const uint16_t* pInitR =  pIn->initTableR;

  const int16_t* pDeltaGr = pIn->deltaTableGr;
  const int16_t* pDeltaGb = pIn->deltaTableGb;
  const int16_t* pDeltaB =  pIn->deltaTableB;
  const int16_t* pDeltaR =  pIn->deltaTableR;

  /* first pack and write init table */
  for (i = 0; i < MESH_ROLL_OFF_INIT_TABLE_SIZE; i++) {
    CDBG("%s: i=%d, pInitR=%d, pInitGr=%d, pInitB=%d, pInitGb=%d\n", __func__,
      i, *pInitR, *pInitGr, *pInitB, *pInitGb);
    cmd->Table.initTable[i*2] = (((uint32_t)(*pInitR)) &0x0000FFFF) |
      (((uint32_t)(*pInitGr))<<16);
    pInitR++;
    pInitGr++;
    cmd->Table.initTable[i*2+1] = (((uint32_t)(*pInitB)) & 0x0000FFFF) |
      (((uint32_t)(*pInitGb))<<16);
    pInitB++;
    pInitGb++;

  }
  /* now pack and write delta table */
  for (i = 0; i < MESH_ROLL_OFF_DELTA_TABLE_SIZE; i++) {
    CDBG("%s: i=%d, pDeltaR=%d, pDeltaGr=%d, pDeltaB=%d, pDeltaGb=%d\n",
      __func__, i, *pDeltaR, *pDeltaGr, *pDeltaB, *pDeltaGb);
    cmd->Table.deltaTable[i*2] = (((uint32_t)(*pDeltaR)) & 0x0000FFFF) |
      (((uint32_t)(*pDeltaGr))<<16);
    pDeltaR++;
    pDeltaGr++;
    cmd->Table.deltaTable[i*2+1] = (((uint32_t)(*pDeltaB)) &0x0000FFFF) |
      (((uint32_t)(*pDeltaGb))<<16);
    pDeltaB++;
    pDeltaGb++;
  }
} /* mesh_rolloff_prepare_hw_table */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_debug -
 *
 * DESCRIPTION:
 *============================================================================*/
static void mesh_rolloff_debug(MESH_RollOffConfigCmdType *cmd)
{
  int i;
  CDBG("%s: gridWidth=%d, gridHeight=%d, yDelta=%d\n", __func__,
    cmd->CfgParams.gridWidth, cmd->CfgParams.gridHeight, cmd->CfgParams.yDelta);

  CDBG("%s: gridXIndex=%d, gridYIndex=%d, gridPixelXIndex=%d, "
    "gridPixelYIndex=%d\n", __func__, cmd->CfgParams.gridXIndex,
    cmd->CfgParams.gridYIndex, cmd->CfgParams.gridPixelXIndex,
    cmd->CfgParams.gridPixelYIndex);

  CDBG("%s: yDeltaAccum=%d, =%d, pixelShift=%d\n", __func__,
    cmd->CfgParams.yDeltaAccum, cmd->CfgParams.pixelOffset,
    cmd->CfgParams.pixelShift);

  for (i=0; i<(MESH_ROLL_OFF_INIT_TABLE_SIZE*2); i++)
    CDBG("%s: initTable[%d]=%d\n", __func__, i, cmd->Table.initTable[i]);
  for (i=0; i<(MESH_ROLL_OFF_DELTA_TABLE_SIZE*2); i++)
    CDBG("%s: deltaTable[%d]=%d\n", __func__, i, cmd->Table.deltaTable[i]);
} /* mesh_rolloff_debug */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_update_table -
 *
 * DESCRIPTION:
 *============================================================================*/
static void mesh_rolloff_update_table(MESH_RollOffConfigCmdType *cmd,
  mesh_rolloff_array_type *tableIn, vfe_params_t *vfe_params)
{
  uint32_t row, col;
  MESH_RollOffTable Tblcfg;

  CDBG("%s: sensor_parms.lastPixel=%d sensor_parms.firstPixel=%d\n", __func__,
    vfe_params->sensor_parms.lastPixel, vfe_params->sensor_parms.firstPixel);
  CDBG("%s: sensor_parms.lastLine=%d sensor_parms.firstLine=%d\n", __func__,
    vfe_params->sensor_parms.lastLine, vfe_params->sensor_parms.firstLine);
#ifdef VFE_2X
  uint32_t camif_width = vfe_params->sensor_parms.lastPixel -
    vfe_params->sensor_parms.firstPixel + 1;
  uint32_t camif_height = vfe_params->sensor_parms.lastLine -
    vfe_params->sensor_parms.firstLine + 1;
#else
  uint32_t camif_width = vfe_params->vfe_input_win.width;
  uint32_t camif_height = vfe_params->vfe_input_win.height;
#endif
  uint16_t grid_width = (camif_width + 31) / 32;
  uint16_t grid_height = (camif_height + 23) / 24;

  /* Update RollOffTableConfig command and send command */
  cmd->CfgParams.gridWidth = grid_width - 1;
  cmd->CfgParams.gridHeight = grid_height - 1;
  cmd->CfgParams.yDelta = (1 << Y_DELTA_Q_LEN) / grid_height;
  cmd->CfgParams.gridXIndex = 0;
  cmd->CfgParams.gridYIndex = 0;
  cmd->CfgParams.gridPixelXIndex = 0;
  cmd->CfgParams.gridPixelYIndex = 0;
  cmd->CfgParams.yDeltaAccum = 0;
  /* Added below two new fields */
  cmd->CfgParams.pixelOffset = 0;
  cmd->CfgParams.pixelShift = 0;

  for (row = 0; row < MESH_ROLL_OFF_VERTICAL_GRIDS + 1; row++) {
    /* Fill out init tables for R, GR, B, GB channels respectively. */
    Tblcfg.initTableR[row] = FLOAT_TO_Q(13, tableIn->r_gain[row *
      (MESH_ROLL_OFF_HORIZONTAL_GRIDS + 1)]);

    Tblcfg.initTableGr[row] = FLOAT_TO_Q(13, tableIn->gr_gain[row *
      (MESH_ROLL_OFF_HORIZONTAL_GRIDS + 1)]);

    Tblcfg.initTableB[row] = FLOAT_TO_Q(13, tableIn->b_gain[row *
      (MESH_ROLL_OFF_HORIZONTAL_GRIDS + 1)]);

    Tblcfg.initTableGb[row] = FLOAT_TO_Q(13, tableIn->gb_gain[row *
      (MESH_ROLL_OFF_HORIZONTAL_GRIDS + 1)]);
  }

  /* Fill out delta tables for RED channel */
  for (row = 0, col = 0; col < (MESH_ROLL_OFF_VERTICAL_GRIDS + 1) *
    MESH_ROLL_OFF_HORIZONTAL_GRIDS; row++) {
    if ((row % (MESH_ROLL_OFF_HORIZONTAL_GRIDS + 1)) !=
      MESH_ROLL_OFF_HORIZONTAL_GRIDS) {
      Tblcfg.deltaTableR[col] = (int)((tableIn->r_gain[row + 1] -
        tableIn->r_gain[row]) * (1 << 20) / grid_width + 0.5);
      col++;
    }
  }

  /* Fill out delta tables for GR channel */
  for (row = 0, col = 0; col < (MESH_ROLL_OFF_VERTICAL_GRIDS + 1) *
    MESH_ROLL_OFF_HORIZONTAL_GRIDS; row++) {
    if ((row % (MESH_ROLL_OFF_HORIZONTAL_GRIDS + 1)) !=
      MESH_ROLL_OFF_HORIZONTAL_GRIDS) {
      Tblcfg.deltaTableGr[col] = (int)((tableIn->gr_gain[row + 1] -
        tableIn->gr_gain[row]) * (1 << 20) / grid_width + 0.5);
      col++;
    }
  }

  /* Fill out delta tables for BLUE channel */
  for (row = 0, col = 0; col < (MESH_ROLL_OFF_VERTICAL_GRIDS + 1) *
    MESH_ROLL_OFF_HORIZONTAL_GRIDS; row++) {
    if ((row % (MESH_ROLL_OFF_HORIZONTAL_GRIDS + 1)) !=
      MESH_ROLL_OFF_HORIZONTAL_GRIDS) {
      Tblcfg.deltaTableB[col] = (int)((tableIn->b_gain[row + 1] -
        tableIn->b_gain[row])* (1 << 20) / grid_width + 0.5);
      col++;
    }
  }

  /* Fill out delta tables for GB channel */
  for (row = 0, col = 0; col < (MESH_ROLL_OFF_VERTICAL_GRIDS + 1) *
    MESH_ROLL_OFF_HORIZONTAL_GRIDS; row++) {
    if ((row % (MESH_ROLL_OFF_HORIZONTAL_GRIDS + 1)) !=
      MESH_ROLL_OFF_HORIZONTAL_GRIDS) {
      Tblcfg.deltaTableGb[col] = (int)((tableIn->gb_gain[row + 1] -
        tableIn->gb_gain[row]) * (1 << 20) / grid_width + 0.5);
      col++;
    }
  }
  mesh_rolloff_prepare_hw_table(&(Tblcfg), cmd);
  mesh_rolloff_debug(cmd);
} /* mesh_rolloff_update_table */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_init -
 *
 * DESCRIPTION:
 *============================================================================*/
vfe_status_t mesh_rolloff_init(mesh_rolloff_mod_t* mesh_rolloff_ctrl,
  vfe_rolloff_info_t *mesh_tbls, vfe_params_t* vfe_params)
{
  chromatix_parms_type *chrPtr = NULL;
  memset(mesh_rolloff_ctrl, 0x0, sizeof(mesh_rolloff_mod_t));
  if (!IS_BAYER_FORMAT(vfe_params)) {
    return VFE_SUCCESS;
  }
  mesh_rolloff_ctrl->mesh_rolloff_enable = TRUE;
  mesh_rolloff_ctrl->mesh_rolloff_update = FALSE;
  mesh_rolloff_ctrl->mesh_rolloff_trigger_enable = TRUE;
  mesh_rolloff_ctrl->mesh_rolloff_reload_params = FALSE;
#ifdef VFE_2X
  mesh_rolloff_ctrl->mesh_rolloff_prev_param.input_table =
    mesh_tbls->left[ROLLOFF_PREVIEW];
#else

  chrPtr = vfe_params->chroma3a;
  mesh_rolloff_ctrl->mesh_rolloff_prev_param.input_table =
    chrPtr->chromatix_mesh_rolloff_table[ROLLOFF_PREVIEW];
#endif

  mesh_rolloff_ctrl->cur_vfe_mode = VFE_OP_MODE_PREVIEW;
  mesh_rolloff_ctrl->mesh_rolloff_snap_param.input_table =
    mesh_tbls->left[VFE_ROLLOFF_TL84_LIGHT];

  mesh_rolloff_ctrl->mesh_rolloff_trigger_ratio.ratio = 0;
  mesh_rolloff_ctrl->mesh_rolloff_trigger_ratio.lighting = TRIGGER_NORMAL;

  return VFE_SUCCESS;
} /* mesh_rolloff_init */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_config -
 *
 * DESCRIPTION:
 *============================================================================*/
vfe_status_t mesh_rolloff_config(mesh_rolloff_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params)
{
  MESH_RollOffConfigCmdType *cmd = NULL;
  mesh_rolloff_array_type *meshRolloffTableCur = NULL;

  if (!mesh_rolloff_ctrl->mesh_rolloff_enable) {
    CDBG("%s: Mesh Rolloff is disabled. Skip the config.\n", __func__);
    return VFE_SUCCESS;
  }

  if (IS_SNAP_MODE(vfe_params)) {
    if (!mesh_rolloff_ctrl->mesh_rolloff_trigger_enable) {
      CDBG("%s: Snapshot should have the same config as Preview\n", __func__);
      cmd = &(mesh_rolloff_ctrl->mesh_rolloff_prev_cmd);
      goto hw_cmd_send;
    }
    if (!mesh_rolloff_ctrl->mesh_rolloff_update)
      CDBG_HIGH("%s: Trigger should be valid before snapshot config is called."
        " Disabling Roll-off for snapshot\n", __func__);
    cmd = &(mesh_rolloff_ctrl->mesh_rolloff_snap_cmd);
    meshRolloffTableCur =
      &(mesh_rolloff_ctrl->mesh_rolloff_snap_param.input_table);
  } else {
    cmd = &(mesh_rolloff_ctrl->mesh_rolloff_prev_cmd);
#ifdef VFE_2X
  meshRolloffTableCur = &(vfe_params->chroma3a->chromatix_mesh_rolloff_table[ROLLOFF_PREVIEW]);
#else
  meshRolloffTableCur = &(mesh_rolloff_ctrl->mesh_rolloff_prev_param.input_table);
#endif
  }
  mesh_rolloff_ctrl->cur_vfe_mode = vfe_params->vfe_op_mode;
  mesh_rolloff_update_table(cmd, meshRolloffTableCur, vfe_params);

hw_cmd_send:
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) cmd, sizeof(MESH_RollOffConfigCmdType),
    VFE_CMD_MESH_ROLL_OFF_CFG)) {
    CDBG_HIGH("%s: mesh rolloff config for operation mode = %d failed\n",
      __func__, vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* mesh_rolloff_config */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_update -
 *
 * DESCRIPTION: Mesh rolloff in VFE3.2 and before is not double buffered,
 *              so update is not required. Expand this routine when
 *              VFE4 is introduced.
 *============================================================================*/
vfe_status_t mesh_rolloff_update(mesh_rolloff_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params)
{
  MESH_RollOffConfigCmdType *cmd = NULL;
  mesh_rolloff_array_type *meshRolloffTableCur = NULL;

  if (!mesh_rolloff_ctrl->mesh_rolloff_enable) {
    CDBG("%s: Mesh Rolloff is disabled. Skip the config.\n", __func__);
    return VFE_SUCCESS;
  }

  if (!mesh_rolloff_ctrl->mesh_rolloff_trigger_enable) {
    CDBG("%s: trigger not enabled\n", __func__);
    return VFE_SUCCESS;
  }

  if (!mesh_rolloff_ctrl->mesh_rolloff_update) {
    CDBG("%s: no update required \n", __func__);
    return VFE_SUCCESS;
  }

  if (IS_SNAP_MODE(vfe_params)) {
    cmd = &(mesh_rolloff_ctrl->mesh_rolloff_snap_cmd);
    meshRolloffTableCur =
      &(mesh_rolloff_ctrl->mesh_rolloff_snap_param.input_table);
  } else {
    cmd = &(mesh_rolloff_ctrl->mesh_rolloff_prev_cmd);
    meshRolloffTableCur =
      &(mesh_rolloff_ctrl->mesh_rolloff_prev_param.input_table);
  }

  mesh_rolloff_update_table(cmd, meshRolloffTableCur, vfe_params);

hw_cmd_send:
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) cmd, sizeof(MESH_RollOffConfigCmdType),
    VFE_CMD_MESH_ROLL_OFF_CFG)) {
    CDBG_HIGH("%s: mesh rolloff config for operation mode = %d failed\n",
      __func__, vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* mesh_rolloff_update */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_trigger_update -
 *
 * DESCRIPTION:
 *============================================================================*/
vfe_status_t mesh_rolloff_trigger_update(mesh_rolloff_mod_t* mesh_rolloff_ctrl,
  vfe_rolloff_info_t *mesh_tbls, vfe_params_t* vfe_params)
{
  float aec_ratio = 0.0;
  static float cur_real_gain = 1.0;
  static float cur_lux_idx = 1.0;
  static float cur_mired_color_temp = 1.0;
  static vfe_flash_type cur_flash_mode = VFE_FLASH_NONE;
  float new_real_gain = cur_real_gain;
  float new_lux_idx;
  float new_mired_color_temp;
  vfe_flash_type new_flash_mode;
  chromatix_parms_type *chrPtr = vfe_params->chroma3a;
  mesh_rolloff_array_type *meshRolloffTableFinal = NULL;
  mesh_rolloff_array_type meshRolloffTableCCT;
  int is_snap = IS_SNAP_MODE(vfe_params);

  mesh_rolloff_ctrl->mesh_rolloff_update = FALSE;
  if(!is_snap) {
    if (mesh_rolloff_ctrl->mesh_rolloff_reload_params) {
      CDBG("%s: Preview chromatix updated", __func__);
      mesh_rolloff_ctrl->mesh_rolloff_prev_param.input_table =
        chrPtr->chromatix_mesh_rolloff_table[ROLLOFF_PREVIEW];
      mesh_rolloff_ctrl->mesh_rolloff_update = TRUE;
      mesh_rolloff_ctrl->mesh_rolloff_reload_params = FALSE;
    }
    return VFE_SUCCESS;
  }

  if (!mesh_rolloff_ctrl->mesh_rolloff_enable) {
    CDBG("%s: Mesh Rolloff is disabled. Skip the trigger.\n", __func__);
    return VFE_SUCCESS;
  }

  if (!mesh_rolloff_ctrl->mesh_rolloff_trigger_enable) {
    CDBG("%s: Trigger is disable. Skip the trigger update.\n", __func__);
    return VFE_SUCCESS;
  }

  if (IS_SNAP_MODE(vfe_params)) {
    meshRolloffTableFinal =
      &(mesh_rolloff_ctrl->mesh_rolloff_snap_param.input_table);
    *meshRolloffTableFinal =
      mesh_rolloff_ctrl->mesh_rolloff_prev_param.input_table;
    new_real_gain = vfe_params->aec_params.snapshot_real_gain;
  } else {
    meshRolloffTableFinal =
      &(mesh_rolloff_ctrl->mesh_rolloff_prev_param.input_table);
    if (!vfe_util_aec_check_settled(&(vfe_params->aec_params))) {
      if (!mesh_rolloff_ctrl->mesh_rolloff_reload_params) {
        CDBG("%s: AEC is not setteled. Skip the trigger\n", __func__);
        return VFE_SUCCESS;
      }
    }
    new_real_gain = vfe_params->aec_params.cur_real_gain;
  }
  chrPtr = vfe_params->chroma3a;
  new_lux_idx = vfe_params->aec_params.lux_idx;
  new_flash_mode = vfe_params->flash_params.flash_mode;
  new_mired_color_temp = MIRED(vfe_params->awb_params.color_temp);

  CDBG("%s: gain %f %f flash %d %d mode %d",
    __func__, cur_real_gain, new_real_gain, cur_flash_mode, new_flash_mode,
    mesh_rolloff_ctrl->cur_vfe_mode);
  if ((F_EQUAL(cur_real_gain, new_real_gain)) &&
    (cur_lux_idx == new_lux_idx) &&
    (cur_flash_mode == new_flash_mode) &&
    (!mesh_rolloff_ctrl->mesh_rolloff_reload_params) &&
    (mesh_rolloff_ctrl->cur_vfe_mode == vfe_params->vfe_op_mode)) {
    CDBG("%s: No change in trigger. Nothing to update\n", __func__);
    return VFE_SUCCESS;
  } else {
    CDBG("%s: Change in trigger. Update roll-off tables.\n", __func__);
    cur_real_gain = new_real_gain;
    cur_lux_idx = new_lux_idx;
    cur_mired_color_temp = new_mired_color_temp;
    cur_flash_mode = new_flash_mode;
    mesh_rolloff_ctrl->mesh_rolloff_reload_params = FALSE;
    mesh_rolloff_ctrl->cur_vfe_mode = vfe_params->vfe_op_mode;
  }

  mesh_rolloff_ctrl->mesh_rolloff_update = TRUE;

  /* Note: AWB's CCT interpolated tables are used regardeless Flash
   *       is on or not. So derive them before checking Flash on or not. */
  mesh_rolloff_calc_awb_trigger(&meshRolloffTableCCT, mesh_tbls, vfe_params);

  if (new_flash_mode != VFE_FLASH_NONE) {
    mesh_rolloff_calc_flash_trigger(&meshRolloffTableCCT, meshRolloffTableFinal,
      mesh_tbls, vfe_params);
  } else {
    aec_ratio = vfe_util_get_aec_ratio(chrPtr->control_rolloff,
      &(chrPtr->rolloff_lowlight_trigger), vfe_params);
    if (F_EQUAL(aec_ratio, 0.0)) {
      CDBG("%s: Low Light \n", __func__);
      *meshRolloffTableFinal = mesh_tbls->left[VFE_ROLLOFF_LOW_LIGHT];
    } else if (F_EQUAL(aec_ratio, 1.0)) {
      CDBG("%s: Bright Light \n", __func__);
      *meshRolloffTableFinal = meshRolloffTableCCT;
    } else {
      CDBG("%s: Interpolate between CCT and Low Light \n", __func__);
      mesh_rolloff_table_interpolate(&meshRolloffTableCCT,
        &(mesh_tbls->left[VFE_ROLLOFF_LOW_LIGHT]), meshRolloffTableFinal,
        aec_ratio);
    }
  }

  return VFE_SUCCESS;
} /* mesh_rolloff_trigger_update */

/*=============================================================================
 * Function:               mesh_rolloff_trigger_enable
 *
 * Description:
 *============================================================================*/
vfe_status_t mesh_rolloff_trigger_enable(mesh_rolloff_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params, int8_t enable)
{
  mesh_rolloff_ctrl->mesh_rolloff_trigger_enable = enable;

  return VFE_SUCCESS;
} /* mesh_rolloff_trigger_enable */

/*===========================================================================
 * FUNCTION    - mesh_rolloff_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t mesh_rolloff_reload_params(mesh_rolloff_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params)
{
  mesh_rolloff_ctrl->mesh_rolloff_reload_params = TRUE;

  return VFE_SUCCESS;
} /* mesh_rolloff_reload_params */
