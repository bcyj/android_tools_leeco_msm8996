/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/
#include "mesh_rolloff_v4.h"
#include "camera_dbg.h"
#include "vfe_tgtcommon.h"

#ifdef ENABLE_MESH_ROLLOFF_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

#define NUM_OF_SUB_GRID 8 /* init 0 or 64? compute by system team code*/
#define INTERP_FACTOR 3 /* 2^INTERP_FACTOR = NUM_OF_SUB_GRID */
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
 * Function:           mesh_rolloff_V4_ScaleMesh
 *
 * Description:
 *============================================================================*/
void mesh_rolloff_V4_ScaleMesh(float *Mesh, uint16_t *meshOut)
{
  float tempMesh[MESH_ROLL_OFF_V4_TABLE_SIZE];
  double cxm, cx0, cx1, cx2, cym, cy0, cy1, cy2;
  double am, a0, a1, a2, bm, b0, b1, b2;
  double tx , ty;
  int ix, iy;
  int i, j;

  /* Down scale mesh table by bicubic interpolation
     x ratio: 16/MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS
     y ratio: 12/MESH_ROLL_OFF_V4_VERTICAL_GRIDS  */
  for (i = 0; i < (MESH_ROLL_OFF_V4_VERTICAL_GRIDS + 1); i++) {
    for (j = 0; j < (MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS + 1); j++) {
      tx = (double)j * 16.0 / 12.0;
      ix = (int)tx;
      tx -= (double)ix;

      ty = (double)i * 12.0 / 9.0;
      iy = (int)ty;
      ty -= (double)iy;

      /*get x direction coeff and y direction coeff*/
      CUBIC_F(tx, cxm, cx0, cx1, cx2);
      CUBIC_F(ty, cym, cy0, cy1, cy2);

      if (ty == 0 && tx == 0)
        tempMesh[(i * (MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS + 1)) + j] =
          Mesh[(iy * 17) + ix];
      else if (ty == 0) {
        am = Mesh[(iy * 17) + (ix - 1)];
        a0 = Mesh[(iy * 17) + ix];
        a1 = Mesh[(iy * 17) + (ix + 1)];
        a2 = Mesh[(iy * 17) + (ix + 2)];
        tempMesh[i * (MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS + 1) + j] =
          (float)((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));
      }
      else if (tx == 0) {
        bm = Mesh[((iy - 1) * 17) + ix];
        b0 = Mesh[(iy * 17) + ix];
        b1 = Mesh[((iy + 1) * 17) + ix];
        b2 = Mesh[((iy + 2) * 17) + ix];
        tempMesh[i * (MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS + 1) + j] =
          (float)((cym * bm) + (cy0 * b0) + (cy1 * b1) + (cy2 * b2));
      }
      else {
        am = Mesh[((iy-1) * 17) + (ix - 1)];
        a0 = Mesh[((iy-1) * 17) + ix];
        a1 = Mesh[((iy-1) * 17) + (ix + 1)];
        a2 = Mesh[((iy-1) * 17) + (ix + 2)];
        bm = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Mesh[(iy * 17) + (ix-1)];
        a0 = Mesh[(iy * 17) + ix];
        a1 = Mesh[(iy * 17) + (ix + 1)];
        a2 = Mesh[(iy * 17) + (ix + 2)];
        b0 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Mesh[((iy + 1) * 17) + (ix-1)];
        a0 = Mesh[((iy + 1) * 17) + ix];
        a1 = Mesh[((iy + 1) * 17) + (ix + 1)];
        a2 = Mesh[((iy + 1) * 17) + (ix + 2)];
        b1 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Mesh[((iy + 2) * 17) + (ix - 1)];
        a0 = Mesh[((iy + 2) * 17) + ix];
        a1 = Mesh[((iy + 2) * 17) + (ix + 1)];
        a2 = Mesh[((iy + 2) * 17) + (ix + 2)];
        b2 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));
        tempMesh[(i * (MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS + 1)) + j] =
          (float)((cym * bm) + (cy0 * b0) + (cy1 * b1) + (cy2 * b2));
      }
    }
  }
  /* Fill out Q10 tables*/
  for (i = 0; i < MESH_ROLL_OFF_V4_TABLE_SIZE; i++) {
    meshOut[i] = FLOAT_TO_Q(10, tempMesh[i]);
  }

}/*mesh_rolloff_V4_ScaleMesh*/

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
static void mesh_rolloff_prepare_hw_table(const MESH_RollOffTable_V4 *pIn,
  MESH_RollOff_V4_ConfigCmdType *cmd)
{
  uint16_t i;
  uint32_t data;

  const uint16_t* Gr = pIn->TableGr;
  const uint16_t* Gb = pIn->TableGb;
  const uint16_t* B =  pIn->TableB;
  const uint16_t* R =  pIn->TableR;

    for (i = 0; i < MESH_ROLL_OFF_V4_TABLE_SIZE; i++) {
      CDBG("%s: i=%d, R=%d, Gr=%d\n",
        __func__, i, *R, *Gr);
      cmd->Table.Table[i] = (((uint32_t)(*R)) & 0x0000FFFF) |
        (((uint32_t)(*Gr))<<16);
      R++;
      Gr++;
    }
    for (i = MESH_ROLL_OFF_V4_TABLE_SIZE; i < MESH_ROLL_OFF_V4_TABLE_SIZE * 2; i++) {
      CDBG("%s: i=%d, B=%d, Gb=%d\n",
        __func__, i, *B, *Gb);

      cmd->Table.Table[i] = (((uint32_t)(*B)) &0x0000FFFF) |
        (((uint32_t)(*Gb))<<16);
      B++;
      Gb++;
    }
} /* mesh_rolloff_prepare_hw_table */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_debug -
 *
 * DESCRIPTION:
 *============================================================================*/
static void mesh_rolloff_debug(MESH_RollOff_V4_ConfigCmdType *cmd)
{
  int i;
  CDBG("%s: blockWidth = %d, blockHeight = %d, interp_factor = %d", __func__,
    cmd->CfgParams.blockWidth, cmd->CfgParams.blockHeight,
    cmd->CfgParams.interpFactor);

  CDBG("%s: subGridWidth=%d, subGridHeight=%d\n", __func__,
    cmd->CfgParams.subGridWidth, cmd->CfgParams.subGridHeight);

  CDBG("%s: subGridXDelta = %d,subGridYDelta=%d\n", __func__,
    cmd->CfgParams.subGridXDelta, cmd->CfgParams.subGridYDelta);

  CDBG("%s: BlockXIndex=%d, BlockYIndex=%d\n", __func__,
    cmd->CfgParams.blockXIndex, cmd->CfgParams.blockYIndex);

  CDBG("%s: PixelXIndex=%d, PixelYIndex=%d \n", __func__,
    cmd->CfgParams.PixelXIndex, cmd->CfgParams.PixelYIndex);

  CDBG("%s: subGridXIndex = %d, subGridYIndex = %d\n", __func__,
    cmd->CfgParams.subGridXIndex, cmd->CfgParams.subGridYIndex);

  CDBG("%s: yDeltaAccum=%d, pixelOffset = %d pcaLutBankSel =%d\n", __func__,
    cmd->CfgParams.yDeltaAccum, cmd->CfgParams.pixelOffset,
    cmd->CfgParams.pcaLutBankSel);

  for (i=0; i<(MESH_ROLL_OFF_V4_TABLE_SIZE * 2); i++)
    CDBG("%s: HW Table[%d]=0x%x\n", __func__, i, cmd->Table.Table[i]);
} /* mesh_rolloff_debug */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_update_table -
 *
 * DESCRIPTION:
 *============================================================================*/
static void mesh_rolloff_update_table(MESH_RollOff_V4_ConfigCmdType *cmd,
  mesh_rolloff_array_type *tableIn, vfe_params_t *vfe_params)
{
  uint32_t row, col, i;
  MESH_RollOffTable_V4 Tblcfg;

  CDBG("%s: sensor_parms.lastPixel=%d sensor_parms.firstPixel=%d\n", __func__,
    vfe_params->sensor_parms.lastPixel, vfe_params->sensor_parms.firstPixel);
  CDBG("%s: sensor_parms.lastLine=%d sensor_parms.firstLine=%d\n", __func__,
    vfe_params->sensor_parms.lastLine, vfe_params->sensor_parms.firstLine);

  uint32_t camif_width = vfe_params->vfe_input_win.width;
  uint32_t camif_height = vfe_params->vfe_input_win.height;
  uint16_t block_width  = (camif_width + 11) / MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS;
  uint16_t block_height = (camif_height + 8) / MESH_ROLL_OFF_V4_VERTICAL_GRIDS;
    /* decide how many sub grids in a block, num_sub_grid = 2^interpFactor*/
  uint32_t interp_factor =  (uint32_t) INTERP_FACTOR;
  uint16_t sub_grid_width = block_width / NUM_OF_SUB_GRID;
  uint16_t sub_grid_height = block_height / NUM_OF_SUB_GRID;

  /* Update RollOffTableConfig command and send command */
  /* Note: Bank selection will be handled in the kernel. */
  cmd->CfgParams.pcaLutBankSel = 0;
  cmd->CfgParams.pixelOffset = 0;

  cmd->CfgParams.blockWidth = (block_width / 2) -1;
  cmd->CfgParams.blockHeight = (block_height / 2) -1;

  /* ROLLOFF STRIPE CFG: in non-striping mode, these registers programmed 0*/
  cmd->CfgParams.blockXIndex = 0;
  cmd->CfgParams.blockYIndex = 0;
  cmd->CfgParams.PixelXIndex = 0;
  cmd->CfgParams.PixelYIndex = 0;
  cmd->CfgParams.subGridXIndex = 0;
  cmd->CfgParams.subGridYIndex = 0;
  cmd->CfgParams.yDeltaAccum = 0;

  /*TODO: NEW REGISTER CONFIG need to be taken care of*/
  cmd->CfgParams.interpFactor = interp_factor;
  cmd->CfgParams.subGridXDelta = (1 << 20) / sub_grid_width;
  cmd->CfgParams.subGridYDelta = (1 << 13) / sub_grid_height;
  cmd->CfgParams.subGridWidth = sub_grid_width / 2 - 1;
  cmd->CfgParams.subGridHeight = sub_grid_height / 2 - 1;

  /* Down scale the mesh table from Chromatix header 17x13 to HW 13x10 size*/
  if (tableIn->mesh_rolloff_table_size > MESH_ROLL_OFF_V4_TABLE_SIZE) {
    CDBG("%s: Bicubuc downscale 13x17 mesh table to 13x10 mesh table", __func__);
    mesh_rolloff_V4_ScaleMesh(tableIn->r_gain, Tblcfg.TableR);
    mesh_rolloff_V4_ScaleMesh(tableIn->gr_gain, Tblcfg.TableGr);
    mesh_rolloff_V4_ScaleMesh(tableIn->gb_gain, Tblcfg.TableGb);
    mesh_rolloff_V4_ScaleMesh(tableIn->b_gain, Tblcfg.TableB);
  }

  mesh_rolloff_prepare_hw_table(&(Tblcfg), cmd);
  mesh_rolloff_debug(cmd);
} /* mesh_rolloff_update_table */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_V4_init -
 *
 * DESCRIPTION:
 *============================================================================*/
vfe_status_t mesh_rolloff_V4_init(mesh_rolloff_V4_mod_t* mesh_rolloff_ctrl,
  vfe_rolloff_info_t *mesh_tbls, vfe_params_t* vfe_params)
{
  chromatix_parms_type *chrPtr = NULL;
  memset(mesh_rolloff_ctrl, 0x0, sizeof(mesh_rolloff_V4_mod_t));
  if (!IS_BAYER_FORMAT(vfe_params)) {
    return VFE_SUCCESS;
  }
  mesh_rolloff_ctrl->mesh_rolloff_enable = TRUE;
  mesh_rolloff_ctrl->mesh_rolloff_update = FALSE;
  mesh_rolloff_ctrl->mesh_rolloff_trigger_enable = TRUE;
  mesh_rolloff_ctrl->mesh_rolloff_reload_params = FALSE;
  mesh_rolloff_ctrl->cur_vfe_mode = VFE_OP_MODE_PREVIEW;

  chrPtr = vfe_params->chroma3a;
  mesh_rolloff_ctrl->mesh_rolloff_prev_param.input_table =
    chrPtr->chromatix_mesh_rolloff_table[ROLLOFF_PREVIEW];

  mesh_rolloff_ctrl->mesh_rolloff_snap_param.input_table =
    mesh_tbls->left[VFE_ROLLOFF_TL84_LIGHT];

  mesh_rolloff_ctrl->mesh_rolloff_trigger_ratio.ratio = 0;
  mesh_rolloff_ctrl->mesh_rolloff_trigger_ratio.lighting = TRIGGER_NORMAL;

  mesh_rolloff_ctrl->mesh_rolloff_prev_param.input_table.\
    mesh_rolloff_table_size= CHROMATIX_MESH_TABLE_SIZE;
  mesh_rolloff_ctrl->mesh_rolloff_snap_param.input_table.\
    mesh_rolloff_table_size= CHROMATIX_MESH_TABLE_SIZE;

  return VFE_SUCCESS;
} /* mesh_rolloff_V4_init */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_V4_config -
 *
 * DESCRIPTION:
 *============================================================================*/
vfe_status_t mesh_rolloff_V4_config(mesh_rolloff_V4_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params)
{
  MESH_RollOff_V4_ConfigCmdType *cmd = NULL;
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
    meshRolloffTableCur = &(mesh_rolloff_ctrl->mesh_rolloff_prev_param.input_table);
  }

  mesh_rolloff_update_table(cmd, meshRolloffTableCur, vfe_params);

hw_cmd_send:
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) cmd, sizeof(MESH_RollOff_V4_ConfigCmdType),
    VFE_CMD_MESH_ROLL_OFF_CFG)) {
    CDBG_HIGH("%s: mesh rolloff config for operation mode = %d failed\n",
      __func__, vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* mesh_rolloff_V4_config */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_V4_update -
 *
 * DESCRIPTION: Mesh rolloff in VFE3.2 and before is not double buffered,
 *              so update is not required. Expand this routine when
 *              VFE4 is introduced.
 *============================================================================*/
vfe_status_t mesh_rolloff_V4_update(mesh_rolloff_V4_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params)
{
  MESH_RollOff_V4_ConfigCmdType *cmd = NULL;
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
    (void *) cmd, sizeof(MESH_RollOff_V4_ConfigCmdType),
    VFE_CMD_MESH_ROLL_OFF_CFG)) {
    CDBG_HIGH("%s: mesh rolloff config for operation mode = %d failed\n",
      __func__, vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* mesh_rolloff_V4_update */

/*==============================================================================
 * FUNCTION    - mesh_rolloff_V4_trigger_update -
 *
 * DESCRIPTION:
 *============================================================================*/
vfe_status_t mesh_rolloff_V4_trigger_update(mesh_rolloff_V4_mod_t* mesh_rolloff_ctrl,
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

  /***************************************************/
  /*TEMP CHANGE: NEED TO REMOVE ONCE STATS IS ENABLED*/
  return VFE_SUCCESS;
  /***************************************************/

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
} /* mesh_rolloff_V4_trigger_update */

/*=============================================================================
 * Function:               mesh_rolloff_V4_trigger_enable
 *
 * Description:
 *============================================================================*/
vfe_status_t mesh_rolloff_V4_trigger_enable(mesh_rolloff_V4_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params, int8_t enable)
{
  mesh_rolloff_ctrl->mesh_rolloff_trigger_enable = enable;

  return VFE_SUCCESS;
} /* mesh_rolloff_V4_trigger_enable */

/*===========================================================================
 * FUNCTION    - mesh_rolloff_V4_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t mesh_rolloff_V4_reload_params(mesh_rolloff_V4_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params)
{
  mesh_rolloff_ctrl->mesh_rolloff_reload_params = TRUE;

  return VFE_SUCCESS;
} /* mesh_rolloff_V4_reload_params */

