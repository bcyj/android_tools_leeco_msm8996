
/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "camera_dbg.h"
#include "vfe_util_common.h"
#include "vfe.h"
#include "color_proc.h"
#include "vfe_tgtcommon.h"

#define ENABLE_CP_LOGGING
#ifdef ENABLE_CP_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * Function:           vfe_color_proc_init
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_color_proc_init(color_proc_mod_t* mod,
  vfe_params_t* parms)
{
  int mod_id = 0;
  vfe_status_t status = VFE_SUCCESS;
  status = vfe_chroma_enhan_init(mod_id, &mod->cv_mod, parms);
  if (VFE_SUCCESS != status)
    return status;

  return vfe_color_correct_init(mod_id, &mod->cc_mod, parms);
}/*vfe_color_proc_init*/

/*===========================================================================
 * Function:           vfe_color_proc_trigger_update
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_color_proc_trigger_update(color_proc_mod_t* mod,
  vfe_params_t* parms)
{
  vfe_status_t status = VFE_SUCCESS;
  int mod_id = 0;
  status = vfe_color_conversion_trigger_update(mod_id, &mod->cv_mod, parms);
  if (VFE_SUCCESS != status)
    return status;
  return vfe_color_correct_trigger_update(mod_id, &mod->cc_mod, parms);
}/*vfe_color_proc_trigger_update*/

/*===========================================================================
 * Function:           vfe_color_proc_enable
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_color_proc_enable(color_proc_mod_t* mod,
  vfe_params_t *params, int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  int mod_id = 0;
  status = vfe_color_conversion_enable(mod_id, &mod->cv_mod, params, enable,
    FALSE);
  if (VFE_SUCCESS != status)
    return status;

  return vfe_color_correct_enable(mod_id, &mod->cc_mod, params, enable, FALSE);
}/*vfe_color_proc_enable*/

/*===========================================================================
 * Function:           vfe_color_proc_config
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_color_proc_config(color_proc_mod_t* mod,
  vfe_params_t* parms)
{
  vfe_status_t status = VFE_SUCCESS;
  int8_t is_snap = IS_SNAP_MODE(parms);
  VFE_ColorProcCfgCmdType* p_cmd = (!is_snap) ?
    &mod->VFE_PrevColorProcCmd :
    &mod->VFE_SnapColorProcCmd;
  VFE_ColorCorrectionCfgCmdType* p_cc_cmd = (!is_snap) ?
    &mod->cc_mod.VFE_PrevColorCorrectionCmd :
    &mod->cc_mod.VFE_SnapColorCorrectionCmd;
  int cc_size = sizeof(VFE_ColorCorrectionCfgCmdType);
  int mod_id = 0;

  status = vfe_color_conversion_config(mod_id, &mod->cv_mod, parms);
  if (VFE_SUCCESS != status)
    return status;

  status = vfe_color_correct_config(mod_id, &mod->cc_mod, parms);
  if (VFE_SUCCESS != status)
    return status;

  /* send command */
  memcpy(p_cmd, p_cc_cmd, cc_size);
  memcpy((uint8_t *)p_cmd + cc_size, &(mod->cv_mod.chroma_enhan_cmd),
    sizeof(mod->cv_mod.chroma_enhan_cmd));

  CDBG("sizeof(VFE_ColorCorrectionCfgCmdType) : %d\n", sizeof(VFE_ColorCorrectionCfgCmdType));
  CDBG("sizeof(VFE_Chroma_Enhance_CfgCmdType) : %d\n", sizeof(VFE_Chroma_Enhance_CfgCmdType));
  CDBG("sizeof(VFE_ColorProcCfgCmdType) : %d\n", sizeof(VFE_ColorProcCfgCmdType));
  status = vfe_util_write_hw_cmd(parms->camfd,
    CMD_GENERAL, p_cmd,
    sizeof(VFE_ColorProcCfgCmdType),
    VFE_CMD_COLOR_PROCESSING_CONFIG);
  return status;
}/*vfe_color_proc_config*/

/*===========================================================================
 * Function:           vfe_color_proc_update
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_color_proc_update(color_proc_mod_t* mod,
  vfe_params_t* parms)
{
  vfe_status_t status = VFE_SUCCESS;
  int8_t is_snap = IS_SNAP_MODE(parms);
  VFE_ColorProcCfgCmdType* p_cmd = (!is_snap) ?
    &mod->VFE_PrevColorProcCmd :
    &mod->VFE_SnapColorProcCmd;
  VFE_ColorCorrectionCfgCmdType* p_cc_cmd = (!is_snap) ?
    &mod->cc_mod.VFE_PrevColorCorrectionCmd :
    &mod->cc_mod.VFE_SnapColorCorrectionCmd;
  int cc_size = sizeof(VFE_ColorCorrectionCfgCmdType);
  int mod_id = 0;

  status = vfe_color_conversion_update(mod_id, &mod->cv_mod, parms);
  if (VFE_SUCCESS != status)
    return status;

  status = vfe_color_correct_update(mod_id, &mod->cc_mod, parms);
  if (VFE_SUCCESS != status)
    return status;

  /* send command */
  memcpy(p_cmd, p_cc_cmd, cc_size);
  memcpy((uint8_t *)p_cmd + cc_size, &(mod->cv_mod.chroma_enhan_cmd),
    sizeof(mod->cv_mod.chroma_enhan_cmd));
  status = vfe_util_write_hw_cmd(parms->camfd,
    CMD_GENERAL, p_cmd,
    sizeof(VFE_ColorProcCfgCmdType),
    VFE_CMD_COLOR_PROCESSING_CONFIG);
  return status;
}/*vfe_color_proc_update*/

/*===========================================================================
 * Function:           vfe_color_proc_set_effect
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_color_proc_set_effect(color_proc_mod_t* mod,
  vfe_params_t* parms, vfe_effects_type_t type)
{
  vfe_status_t status = VFE_SUCCESS;
  int mod_id = 0;
  status = vfe_color_conversion_set_effect(mod_id, &mod->cv_mod, parms, type);
  if (VFE_SUCCESS != status)
    return status;

  status = vfe_color_correct_set_effect(mod_id, &mod->cc_mod, parms, type);
  if (VFE_SUCCESS != status)
    return status;

  return status;
}/*vfe_color_proc_set_effect*/

/*===========================================================================
 * Function:           vfe_color_proc_set_bestshot
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_color_proc_set_bestshot(color_proc_mod_t* mod,
  vfe_params_t* params, camera_bestshot_mode_type mode)
{
  vfe_status_t status = VFE_SUCCESS;
  int mod_id = 0;
  status = vfe_color_conversion_set_bestshot(mod_id, &mod->cv_mod, params, mode);
  if (VFE_SUCCESS != status)
    return status;

  status = vfe_color_correct_set_bestshot(mod_id, &mod->cc_mod, params, mode);
  if (VFE_SUCCESS != status)
    return status;

  return status;
}/*vfe_color_proc_set_bestshot*/

/*===========================================================================
 * Function:           vfe_color_proc_reload_params
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_color_proc_reload_params(color_proc_mod_t* mod,
  vfe_params_t* params)
{
  vfe_status_t status = VFE_SUCCESS;
  int mod_id = 0;
  status = vfe_color_conversion_reload_params(mod_id, &mod->cv_mod, params);
  if (VFE_SUCCESS != status)
    return status;

  return vfe_color_correct_reload_params(mod_id, &mod->cc_mod, params);
}/*vfe_color_proc_reload_params*/

/*===========================================================================
 * Function:           vfe_color_proc_trigger_enable
 *
 * Description:
===========================================================================*/
vfe_status_t vfe_color_proc_trigger_enable(color_proc_mod_t* mod,
  vfe_params_t* params, int enable)
{
  vfe_status_t status = VFE_SUCCESS;
  int mod_id = 0;
  status = vfe_color_conversion_trigger_enable(mod_id, &mod->cv_mod, params, enable);
  if (VFE_SUCCESS != status)
    return status;

  return vfe_color_correct_trigger_enable(mod_id, &mod->cc_mod, params, enable);
}/*vfe_color_proc_trigger_enable*/
