/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "camera_dbg.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_hw.h"
#include "isp.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "isp_pipeline32.h"
#include "isp_pipeline_util.h"
#include "isp_log.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef ENABLE_ISP_PIPELINE32_LOGGING
  #undef CDBG
  #define CDBG ALOGE
#endif

static const uint32_t max_mod_mask_continuous_bayer =
(
  (1 << ISP_MOD_LINEARIZATION) |
  (1 << ISP_MOD_ROLLOFF) |
  (1 << ISP_MOD_DEMUX) |
  (1 << ISP_MOD_DEMOSAIC) |
  (1 << ISP_MOD_BPC) |
  (1 << ISP_MOD_ABF) |
  (1 << ISP_MOD_ASF) |
  (1 << ISP_MOD_COLOR_CONV) |
  (1 << ISP_MOD_COLOR_CORRECT) |
  (1 << ISP_MOD_CHROMA_SS) |
  (1 << ISP_MOD_CHROMA_SUPPRESS) |
  (1 << ISP_MOD_LA) |
  (1 << ISP_MOD_MCE) |
  (1 << ISP_MOD_SCE) |
  (1 << ISP_MOD_CLF) |
  (1 << ISP_MOD_WB) |
  (1 << ISP_MOD_GAMMA) |
  (1 << ISP_MOD_FOV) |
  (1 << ISP_MOD_SCALER) |
  (1 << ISP_MOD_BCC) |
  (1 << ISP_MOD_CLAMP) |
  //(1 << ISP_MOD_FRAME_SKIP) |
  (1 << ISP_MOD_STATS)
);

static const uint32_t max_mod_mask_burst_bayer =
(
  (1 << ISP_MOD_LINEARIZATION) |
  (1 << ISP_MOD_ROLLOFF) |
  (1 << ISP_MOD_DEMUX) |
  (1 << ISP_MOD_DEMOSAIC) |
  (1 << ISP_MOD_BPC) |
  (1 << ISP_MOD_ABF) |
  (1 << ISP_MOD_ASF) |
  (1 << ISP_MOD_COLOR_CONV) |
  (1 << ISP_MOD_COLOR_CORRECT) |
  (1 << ISP_MOD_CHROMA_SS) |
  (1 << ISP_MOD_CHROMA_SUPPRESS) |
  (1 << ISP_MOD_LA) |
  (1 << ISP_MOD_MCE) |
  (1 << ISP_MOD_SCE) |
  (1 << ISP_MOD_CLF) |
  (1 << ISP_MOD_WB) |
  (1 << ISP_MOD_GAMMA) |
  (1 << ISP_MOD_FOV) |
  (1 << ISP_MOD_SCALER) |
  (1 << ISP_MOD_BCC) |
  (1 << ISP_MOD_CLAMP) |
  //(1 << ISP_MOD_FRAME_SKIP)
  (1 << ISP_MOD_STATS)
);

static const uint32_t max_supported_stats =
(
  //(1 << MSM_ISP_STATS_AWB)   |
  (1 << MSM_ISP_STATS_RS)    |  /* share with BG */
  (1 << MSM_ISP_STATS_CS)    |  /* shared with BF */
  (1 << MSM_ISP_STATS_IHIST) |  /* for both CS and RS stats modules */
  (1 << MSM_ISP_STATS_BF)    |
  (1 << MSM_ISP_STATS_BG)    |
  (1 << MSM_ISP_STATS_BHIST)
);

static const uint32_t max_mod_mask_continuous_yuv =
  (1 << ISP_MOD_FOV) |
  (1 << ISP_MOD_SCALER) |
  (1 << ISP_MOD_DEMUX) |
  (1 << ISP_MOD_CLAMP) |
  (1 << ISP_MOD_CHROMA_SS);

static const uint32_t max_mod_mask_burst_yuv =
  (1 << ISP_MOD_FOV) |
  (1 << ISP_MOD_SCALER) |
  (1 << ISP_MOD_DEMUX) |
  (1 << ISP_MOD_CLAMP) |
  (1 << ISP_MOD_CHROMA_SS);

static uint16_t mod_cfg_order_bayer[] =
{
  ISP_MOD_LINEARIZATION,
  ISP_MOD_MCE,
  ISP_MOD_COLOR_CONV,
  ISP_MOD_CLAMP,
  ISP_MOD_CHROMA_SUPPRESS,
  ISP_MOD_CHROMA_SS,
  ISP_MOD_WB,
  ISP_MOD_DEMOSAIC,
  ISP_MOD_BPC,
  ISP_MOD_BCC,
  ISP_MOD_DEMUX,
  ISP_MOD_COLOR_CORRECT,
  ISP_MOD_ABF,
  ISP_MOD_CLF,
  ISP_MOD_GAMMA,
  ISP_MOD_ROLLOFF,
  ISP_MOD_LA,
  ISP_MOD_SCE,
  ISP_MOD_FOV,
  ISP_MOD_SCALER,
  ISP_MOD_ASF,
  ISP_MOD_STATS
};

static uint16_t mod_cfg_order_yuv[] =
{
  ISP_MOD_DEMUX,
  ISP_MOD_FOV,
  ISP_MOD_CHROMA_SS,
  ISP_MOD_SCALER,
  ISP_MOD_CLAMP
};

static uint16_t mod_trigger_update_order_bayer[] =
{
  ISP_MOD_LINEARIZATION,
  ISP_MOD_ROLLOFF,
  ISP_MOD_WB,
  ISP_MOD_DEMOSAIC,
  ISP_MOD_ASF,
  ISP_MOD_BPC,
  ISP_MOD_BCC,
  ISP_MOD_ABF,
  ISP_MOD_LA,
  ISP_MOD_DEMUX,
  ISP_MOD_COLOR_CORRECT,
  ISP_MOD_COLOR_CONV,
  ISP_MOD_GAMMA,
  ISP_MOD_CLF,
  ISP_MOD_MCE,
  ISP_MOD_SCE,
  ISP_MOD_CHROMA_SUPPRESS
};

/** isp_hw_pix_set_sub_stats_cfg_bit:
 *
 *    @op_cmd: pipeline private data
 *    @current_stats_mask: enable modules
 *    @stats_enb: supported module
 *
 * Enable/Disable stats sub-modules.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static void isp_hw_pix_set_sub_stats_cfg_bit(ISP_OperationConfigCmdType *op_cmd,
  uint32_t current_stats_mask, uint32_t stats_enb)
{
  uint8_t enb = 0;

  enb = (current_stats_mask & (1 << MSM_ISP_STATS_AWB)) ? 1 : 0;
  enb = enb & stats_enb;
  //op_cmd->moduleCfg.statsAwbEnable = enb;

  enb = (current_stats_mask & (1 << MSM_ISP_STATS_RS)) ? 1 : 0;
  enb = enb & stats_enb;
  //op_cmd->moduleCfg.statsRsEnable = enb;

  enb = (current_stats_mask & (1 << MSM_ISP_STATS_CS)) ? 1 : 0;
  enb = enb & stats_enb;
  //op_cmd->moduleCfg.statsCsEnable = enb;

  enb = (current_stats_mask & (1 << MSM_ISP_STATS_IHIST)) ? 1 : 0;
  enb = enb & stats_enb;
  //op_cmd->moduleCfg.statsIhistEnable = enb;

  enb = (current_stats_mask & (1 << MSM_ISP_STATS_BG)) ? 1 : 0;
  enb = enb & stats_enb;
  //op_cmd->moduleCfg.statsAeBgEnable = enb;

  enb = (current_stats_mask & (1 << MSM_ISP_STATS_BF)) ? 1 : 0;
  enb = enb & stats_enb;
  //op_cmd->moduleCfg.statsAfBfEnable = enb;

  enb = (current_stats_mask & (1 << MSM_ISP_STATS_BHIST)) ? 1 : 0;
  enb = enb & stats_enb;
  //op_cmd->moduleCfg.statsSkinBhistEnable = enb;
}

/** isp_hw_pix_set_module_cfg_bit:
 *
 *    @pix_ptr: pipeline private data
 *    @mod_id: mudule id
 *    @enb: enable status
 *
 * Enable/Disable VFE sub-modules.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static void isp_hw_pix_set_module_cfg_bit(isp_pipeline_t *pix,
  isp_hw_module_id_t mod_id, uint8_t enb)
{
  isp_pix_params_t *params = NULL;
  isp_operation_cfg_t *op_ptr = NULL;
  ISP_OperationConfigCmdType *op_cmd = NULL;

  if (!pix) {
    CDBG_ERROR("%s: invalid input", __func__);
    return;
  }

  op_ptr = (isp_operation_cfg_t *)(pix->dep.op_ptr);
  if (!op_ptr) {
    CDBG_ERROR("%s: invalid op_ptr", __func__);
    return;
  }
  op_cmd = &op_ptr->op_cmd;

  params = &pix->pix_params;
  if (!params) {
    CDBG_ERROR("%s: invalid params", __func__);
    return;
  }

  switch (mod_id) {
  case ISP_MOD_LINEARIZATION: {
    op_cmd->moduleCfg.blackLevelCorrectionEnable = enb;
  }
    break;

  case ISP_MOD_ROLLOFF: {
    op_cmd->moduleCfg.lensRollOffEnable = enb;
  }
    break;

  case ISP_MOD_DEMUX: {
    op_cmd->moduleCfg.demuxEnable = enb;
  }
    break;

  case ISP_MOD_DEMOSAIC: {
    op_cmd->moduleCfg.demosaicEnable = enb;
  }
    break;

  case ISP_MOD_BPC: {
  }
    break;

  case ISP_MOD_ABF: {
  }
    break;

  case ISP_MOD_ASF: {
    op_cmd->moduleCfg.asfEnable = enb;
  }
    break;

  case ISP_MOD_COLOR_CONV: {
    op_cmd->moduleCfg.chromaEnhanEnable = enb;
  }
    break;

  case ISP_MOD_COLOR_CORRECT: {
    op_cmd->moduleCfg.colorCorrectionEnable = enb;
  }
    break;

  case ISP_MOD_CHROMA_SS: {
    op_cmd->moduleCfg.chromaSubsampleEnable = enb;
  }
    break;

  case ISP_MOD_CHROMA_SUPPRESS: {
    op_cmd->moduleCfg.chromaSuppressionMceEnable |= enb;
  }
    break;

  case ISP_MOD_LA: {
    op_cmd->moduleCfg.lumaAdaptationEnable = enb;
  }
    break;

  case ISP_MOD_MCE: {
    op_cmd->moduleCfg.chromaSuppressionMceEnable |= enb;
  }
    break;

  case ISP_MOD_SCE: {
    op_cmd->moduleCfg.skinEnhancementEnable = enb;
  }
    break;

  case ISP_MOD_CLF: {
    op_cmd->moduleCfg.clfEnable = enb;
  }
    break;

  case ISP_MOD_WB: {
    op_cmd->moduleCfg.whiteBalanceEnable = enb;
  }
    break;

  case ISP_MOD_GAMMA: {
    op_cmd->moduleCfg.rgbLUTEnable = enb;
  }
    break;

  case ISP_MOD_FOV: {
    op_cmd->moduleCfg.cropEnable = enb;
  }
    break;

  case ISP_MOD_SCALER: {
    op_cmd->moduleCfg.mainScalerEnable = enb;
    op_cmd->moduleCfg.scaler2YEnable = enb;
    op_cmd->moduleCfg.scaler2CbcrEnable = enb;
  }
    break;

  case ISP_MOD_BCC: {
  }
    break;

  case ISP_MOD_CLAMP: {
  }
    break;

  case ISP_MOD_FRAME_SKIP: {
  }
    break;

  case ISP_MOD_STATS: {
    op_cmd->moduleCfg.statsAeBgEnable = enb;
    op_cmd->moduleCfg.statsAfBfEnable = enb;
    op_cmd->moduleCfg.statsRsEnable = enb;
    op_cmd->moduleCfg.statsCsEnable = enb;
    op_cmd->moduleCfg.statsIhistEnable = enb;
  }
    break;

  default: {
  }
    break;
  }
}

/** isp_pipeline32_operation_config:
 *
 *    @pix_ptr: pipeline private data
 *    @is_bayer_input: bayer of yuv input
 *
 * Configure VFE module.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
int isp_pipeline32_operation_config(void *pix_ptr, int is_bayer_input)
{
  int i;
  isp_pipeline_t *pix = NULL;
  uint8_t enb;
  isp_pix_params_t *params;
  isp_operation_cfg_t *op_ptr = NULL;
  ISP_OperationConfigCmdType *op_cmd = NULL;
  isp_hw_pix_setting_params_t *cfg = NULL;
  uint32_t mce_and_chroma_suppress =
    ((1 << ISP_MOD_CHROMA_SUPPRESS) | (1 << ISP_MOD_MCE));

  pix = (isp_pipeline_t *)pix_ptr;
  params = (isp_pix_params_t *)&pix->pix_params;
  if (!pix) {
    CDBG_ERROR("%s: invalid input", __func__);
    return -EINVAL;
  }

  op_ptr = (isp_operation_cfg_t *)(pix->dep.op_ptr);
  if (!op_ptr) {
    CDBG_ERROR("%s: invalid op_ptr", __func__);
    return -EINVAL;
  }

  op_cmd = &op_ptr->op_cmd;
  if (!op_cmd) {
    CDBG_ERROR("%s: invalid op_cmd", __func__);
    return -EINVAL;
  }

  cfg = &pix->pix_params.cfg_and_3a_params.cfg;
  if (!op_cmd) {
    CDBG_ERROR("%s: invalid cfg_and_3a_params.cfg", __func__);
    return -EINVAL;
  }

  /* zero the moduleCfg first */
  memset(&op_cmd->moduleCfg, 0, sizeof(op_cmd->moduleCfg));

  /* config all enable bits
   stats module hw cfg not set here
   kernel set hw bit when sub-stats modules request stream when streamon*/
  for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
    enb = ((1 << i) & pix->pix_params.cur_module_mask) ? 1 : 0;

    if (i == (int)ISP_MOD_STATS) {
      isp_hw_pix_set_sub_stats_cfg_bit(op_cmd, (params->cur_stats_mask), enb);
      continue;
    }

    isp_hw_pix_set_module_cfg_bit(pix, (isp_hw_module_id_t)i, enb);
  }

  /* MCE and Chroma Suppress share the same enable bit */
  if (!(mce_and_chroma_suppress & pix->pix_params.cur_module_mask))
    op_cmd->moduleCfg.chromaSuppressionMceEnable = 0;

  /*YUV sensor bypass demosaic and use chroma upsample*/
  if (is_bayer_input == 0) {
    op_cmd->moduleCfg.chromaUpsampleEnable = 1;
  } else {
    op_cmd->moduleCfg.chromaUpsampleEnable = 0;
  }

  op_cmd->moduleCfg.realignmentBufEnable = 0;

  if (is_bayer_input) {
    op_cmd->ispStatsCfg.colorConvEnable = 1;
    op_cmd->ispStatsCfg.bayerGridSelect = 1;
    op_cmd->ispStatsCfg.bayerFocusSelect = 1;
    op_cmd->ispStatsCfg.bayerHistSelect = 1;
  } else {
    op_cmd->ispStatsCfg.colorConvEnable = 0;
  }

  /* isp Cfg , set format*/
  op_cmd->ispCfg.inputPixelPattern =
    isp_fmt_to_pix_pattern(cfg->camif_cfg.sensor_output_fmt);

  return 0;
}

/** isp_pix_dump_ISP_ModuleCfgPacked:
 *
 *    @module_cfg: VFE configuration
 *
 * Print VFE configuration.
 *
 * This function executes in ISP thread context
 *
 * Return none.
 **/
static void isp_pix_dump_ISP_ModuleCfgPacked(ISP_ModuleCfgPacked *module_cfg)
{
  ISP_DBG(ISP_MOD_COM,"%s: "
    "blackLevelCorrectionEnable = %d,\n"
    "lensRollOffEnable = %d,\n"
    "demuxEnable = %d,\n"
    "chromaUpsampleEnable = %d,\n"
    "demosaicEnable = %d,\n"
    "statsAeBgEnable = %d,\n"
    "statsAfBfEnable = %d,\n"
    "statsAwbEnable = %d,\n"
    "statsRsEnable = %d,\n"
    "statsCsEnable = %d,\n"
    "cropEnable = %d,\n"
    "mainScalerEnable = %d,\n"
    "whiteBalanceEnable = %d,\n"
    "clfEnable = %d,\n"
    "colorCorrectionEnable = %d,\n"
    "rgbLUTEnable = %d,\n"
    "statsIhistEnable = %d,\n"
    "lumaAdaptationEnable = %d,\n"
    "chromaEnhanEnable = %d,\n"
    "statsSkinBhistEnable = %d,\n"
    "chromaSuppressionMceEnable = %d,\n"
    "skinEnhancementEnable = %d,\n"
    "asfEnable = %d,\n"
    "chromaSubsampleEnable = %d,\n"
    "scaler2YEnable = %d,\n"
    "scaler2CbcrEnable = %d,\n"
    "realignmentBufEnable = %d\n", __func__,
    module_cfg->blackLevelCorrectionEnable,
    module_cfg->lensRollOffEnable,
    module_cfg->demuxEnable,
    module_cfg->chromaUpsampleEnable,
    module_cfg->demosaicEnable,
    module_cfg->statsAeBgEnable,
    module_cfg->statsAfBfEnable,
    module_cfg->statsAwbEnable,
    module_cfg->statsRsEnable,
    module_cfg->statsCsEnable,
    module_cfg->cropEnable,
    module_cfg->mainScalerEnable,
    module_cfg->whiteBalanceEnable,
    module_cfg->clfEnable,
    module_cfg->colorCorrectionEnable,
    module_cfg->rgbLUTEnable,
    module_cfg->statsIhistEnable,
    module_cfg->lumaAdaptationEnable,
    module_cfg->chromaEnhanEnable,
    module_cfg->statsSkinBhistEnable,
    module_cfg->chromaSuppressionMceEnable,
    module_cfg->skinEnhancementEnable,
    module_cfg->asfEnable,
    module_cfg->chromaSubsampleEnable,
    module_cfg->scaler2YEnable,
    module_cfg->scaler2CbcrEnable,
    module_cfg->realignmentBufEnable);
}

/** isp_pipeline_reset_hist_dmi:
 *
 *    @pix_ptr: pipeline private data
 *
 * Reset the hist and bhist DMI
 *
 * Return 0 on success.
 **/
static int isp_pipeline_reset_hist_dmi(int fd, uint32_t dmi_cahnnel)
{
  int rc = 0;
  Hist_DMI_CfgCmdType hist_reset_cfg;
  uint32_t cmd_offset, cmd_len, tbl_len;

  /* 1. program DMI default value, write auto increment bit
     2. write DMI table
     3. reset DMI cfg */
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[5];

  ALOGE("%s, Enter\n", __func__);

  cfg_cmd.cfg_data = (void *)&hist_reset_cfg;
  cfg_cmd.cmd_len = sizeof(hist_reset_cfg);
  cfg_cmd.cfg_cmd = (void *) &reg_cfg_cmd;
  cfg_cmd.num_cfg = 5;

  /* set dmi to proper hist stats bank */
  hist_reset_cfg.set_channel = ISP32_DMI_CFG_DEFAULT + dmi_cahnnel;
  cmd_offset = 0;
  cmd_len = 1 * sizeof(uint32_t);
  isp_pipeline_util_pack_cfg_cmd(fd, &reg_cfg_cmd[0],
    cmd_offset, cmd_len,
    VFE_WRITE_MB, ISP32_DMI_CFG_OFF);

  /* set start addr = 0*/
  hist_reset_cfg.set_start_addr = 0;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  isp_pipeline_util_pack_cfg_cmd(fd, &reg_cfg_cmd[1],
    cmd_offset, cmd_len,
    VFE_WRITE_MB, ISP32_DMI_ADDR);

  /* memset hi and lo tbl = all 0,
     64 bit write: regarding interlieve uint64 table
       hi_tbl_offset = table offset
       lo_tbl_offset = table offset + sizeof(uint32_t) */
  tbl_len = sizeof(uint64_t) * 256;
  memset(hist_reset_cfg.table, 0, tbl_len);
  cmd_offset += cmd_len;
  cmd_len = tbl_len;
  isp_pipeline_util_pack_dmi_cmd(fd, &reg_cfg_cmd[2],
    cmd_offset, cmd_offset + sizeof(uint32_t), cmd_len,
    VFE_WRITE_DMI_64BIT);

  /* reset the sart addr = 0 */
  hist_reset_cfg.reset_start_addr = 0;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  isp_pipeline_util_pack_cfg_cmd(fd, &reg_cfg_cmd[3],
    cmd_offset, cmd_len,
    VFE_WRITE_MB, ISP32_DMI_ADDR);

  /* set dmi to proper hist stats bank */
  hist_reset_cfg.reset_channel =
    ISP32_DMI_CFG_DEFAULT + ISP32_DMI_NO_MEM_SELECTED;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  isp_pipeline_util_pack_cfg_cmd(fd, &reg_cfg_cmd[4],
    cmd_offset, cmd_len,
    VFE_WRITE_MB, ISP32_DMI_CFG_OFF);

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: VFE reset hist DMI error = %d\n", __func__, rc);
    return rc;
  }

  return rc;
}

/** isp_pipeline32_module_start:
 *
 *    @pix_ptr: pipeline private data
 *
 * Start VFE module.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
int isp_pipeline32_module_start(void *pix_ptr)
{
  int fd, rc = 0;
  uint32_t data_offset = 0;
  isp_pipeline_t *pix = NULL;
  isp_operation_cfg_t *op_ptr = NULL;
  ISP_OperationConfigCmdType *op_cmd = NULL;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[5];

  pix = (isp_pipeline_t *)pix_ptr;
  if (!pix) {
    CDBG_ERROR("%s: invalid input", __func__);
    return -EINVAL;
  }

  fd = pix->fd;

  op_ptr = (isp_operation_cfg_t *)(pix->dep.op_ptr);
  if (!op_ptr) {
    CDBG_ERROR("%s: invalid op_ptr", __func__);
    return -EINVAL;
  }

  op_cmd = &op_ptr->op_cmd;
  if (!op_cmd) {
    CDBG_ERROR("%s: invalid op_cmd", __func__);
    return -EINVAL;
  }

  rc = isp_pipeline_reset_hist_dmi(fd, STATS_BHIST_RAM0);
  if (rc < 0) {
    CDBG_ERROR("%s: reset bhist DMI RAM0 failed, rc = %d\n", __func__, rc);
    return rc;
  }

  rc = isp_pipeline_reset_hist_dmi(fd, STATS_BHIST_RAM1);
  if (rc < 0) {
    CDBG_ERROR("%s: reset bhist DMI RAM1 failed, rc = %d\n", __func__, rc);
    return rc;
  }

  rc = isp_pipeline_reset_hist_dmi(fd, STATS_IHIST_RAM);
  if (rc < 0) {
    CDBG_ERROR("%s: reset hist DMI failed, rc = %d\n", __func__, rc);
    return rc;
  }

  isp_pix_dump_ISP_ModuleCfgPacked(&op_cmd->moduleCfg);

  /* PIX operation configuration */
  memset(&cfg_cmd, 0, sizeof(cfg_cmd));
  memset(reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));

  cfg_cmd.num_cfg = 5;
  cfg_cmd.cmd_len = sizeof(ISP_OperationConfigCmdType);
  cfg_cmd.cfg_data = op_cmd;
  cfg_cmd.cfg_cmd = reg_cfg_cmd;

  /* starting point. Will del the 3 uint32 later */
  data_offset = 0;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP32_CORE_CFG;
  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = data_offset;
  reg_cfg_cmd[0].u.rw_info.len = ISP32_CORE_CFG_LEN;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;

  /*ISP32_MODULE_CFG*/
  data_offset += reg_cfg_cmd[0].u.rw_info.len;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP32_MODULE_CFG;
  reg_cfg_cmd[1].u.rw_info.cmd_data_offset = data_offset;
  reg_cfg_cmd[1].u.rw_info.len = ISP32_MODULE_CFG_LEN;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE;

  data_offset += reg_cfg_cmd[1].u.rw_info.len;
  reg_cfg_cmd[2].u.rw_info.reg_offset = ISP32_REALIGN_BUF_CFG;
  reg_cfg_cmd[2].u.rw_info.cmd_data_offset = data_offset;
  reg_cfg_cmd[2].u.rw_info.len = ISP32_REALIGN_BUF_CFG_LEN;
  reg_cfg_cmd[2].cmd_type = VFE_WRITE;

  /*ISP32_CHROMA_UPSAMPLE_CFG*/
  data_offset += reg_cfg_cmd[2].u.rw_info.len;
  reg_cfg_cmd[3].u.rw_info.reg_offset = ISP32_CHROMA_UPSAMPLE_CFG;
  reg_cfg_cmd[3].u.rw_info.cmd_data_offset = data_offset;
  reg_cfg_cmd[3].u.rw_info.len = ISP32_CHROMA_UPSAMPLE_CFG_LEN;
  reg_cfg_cmd[3].cmd_type = VFE_WRITE;

  /*ISP32_STATS_CFG*/
  data_offset += reg_cfg_cmd[3].u.rw_info.len;
  reg_cfg_cmd[4].u.rw_info.reg_offset = ISP32_STATS_CFG;
  reg_cfg_cmd[4].u.rw_info.cmd_data_offset = data_offset;
  reg_cfg_cmd[4].u.rw_info.len = ISP32_STATS_CFG_LEN;
  reg_cfg_cmd[4].cmd_type = VFE_WRITE;

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  if (rc < 0)
    CDBG_ERROR("%s: VFE core/module cfg error = %d\n", __func__, rc);

  return rc;
}

/** isp_pipeline32_module_enable_notify:
 *
 *    @pix_ptr: pipeline private data
 *    @notify_data: contain destination module and enable/disable cmd
 *
 * Set enable bit for submodule.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int isp_pipeline32_module_enable_notify(void *pix_ptr,
  isp_mod_enable_motify_t *notify_data)
{
  isp_hw_pix_set_module_cfg_bit((isp_pipeline_t *)pix_ptr, notify_data->mod_id,
    notify_data->enable);

  return 0;
}

/** isp32_util_get_aec_ratio_lowlight:
 *
 *    @tunning_type:
 *    @trigger_ptr:
 *    @aec_out:
 *    @is_snap_mode:
 *
 *  Get trigger ratio based on lowlight trigger.
 *  Please note that ratio is the weight of the normal light.
 *
 **/
static float isp32_util_get_aec_ratio_lowlight(unsigned char tunning_type,
  void *trigger_ptr, aec_update_t* aec_out, int8_t is_snap_mode)
{
  float ratio = 0.0, real_gain;
  tuning_control_type tunning = (tuning_control_type)tunning_type;
  trigger_point_type *trigger = (trigger_point_type *)trigger_ptr;

  switch (tunning) {
  /* 0 is Lux Index based */
  case 0: {
    ratio = 1.0 - isp_util_calc_interpolation_weight(aec_out->lux_idx,
                    trigger->lux_index_start, trigger->lux_index_end);
  }
    break;

  /* 1 is Gain Based */
  case 1: {
    real_gain = aec_out->real_gain;
    ratio = 1.0 - isp_util_calc_interpolation_weight(real_gain,
                    trigger->gain_start, trigger->gain_end);
  }
    break;

  default: {
    CDBG_ERROR("get_trigger_ratio: tunning type %d is not supported.\n",
      tunning);
  }
    break;
  }

  if (ratio < 0) {
    ratio = 0;
  } else if (ratio > 1.0) {
    ratio = 1.0;
  }

  return ratio;
} /* isp_util_get_aec_ratio */

/** isp32_util_get_aec_ratio_bright
 *
 *    @tunning_type:
 *    @trigger_ptr:
 *    @aec_out:
 *    @is_snap_mode
 *
 *   Get trigger ratio based on lowlight trigger.
 *   Please note that ratio is the weight of the normal light.
 *    NORMAL          Mix              BRIGHT(OUTDOOR)
 *    ------------|-----------------|-----------------
 *        bright_start(ex: 150)    bright_end(ex: 100)
 *
 **/
static float isp32_util_get_aec_ratio_bright(unsigned char tunning_type,
  void *trigger_ptr, aec_update_t* aec_out, int8_t is_snap_mode)
{
  float normal_light_ratio = 0.0, real_gain;
  float ratio_to_birhgt_end = 0.0;
  tuning_control_type tunning = (tuning_control_type)tunning_type;
  trigger_point_type *trigger = (trigger_point_type *)trigger_ptr;

  switch (tunning) {
  /* 0 is Lux Index based */
  case 0: {
    ratio_to_birhgt_end =isp_util_calc_interpolation_weight(
      aec_out->lux_idx, trigger->lux_index_end, trigger->lux_index_start);
    }
      break;
  /* 1 is Gain Based */
  case 1: {
    real_gain = aec_out->real_gain;
    ratio_to_birhgt_end = isp_util_calc_interpolation_weight(real_gain,
      trigger->gain_end, trigger->gain_start);
  }
    break;

  default: {
    CDBG_ERROR("get_trigger_ratio: tunning type %d is not supported.\n",
      tunning);
  }
    break;
  }

  /*ratio_to_birhgt_end is the sitance to bright_end,
    the smaller distance to bright_end,
    the lower ratio applied on normal light*/
  normal_light_ratio = ratio_to_birhgt_end;

  if (normal_light_ratio < 0) {
    normal_light_ratio = 0;
  } else if (normal_light_ratio > 1.0) {
    normal_light_ratio = 1.0;
  }

  return normal_light_ratio;
} /* isp_util_get_aec_ratio */
/** isp32_util_get_aec_ratio_bright_low:
 *
 *    @tuning_type:
 *    @outdoor_trigger_ptr:
 *    @lowlight_trigger_ptr:
 *    @aec_out:
 *    @is_snap_mode:
 *    @rt:
 *
 *  Get trigger ratio based on outdoor trigger & lowlight
 *  trigger Please note that rt.ratio means the weight of the normal light.
 *
 **/
static int isp32_util_get_aec_ratio_bright_low(unsigned char tuning_type,
  void *outdoor_trigger_ptr, void *lowlight_trigger_ptr,
  aec_update_t* aec_out, int8_t is_snap_mode, trigger_ratio_t *rt)
{
  float real_gain;
  tuning_control_type tuning = (tuning_control_type)tuning_type;
  trigger_point_type *outdoor_trigger =
    (trigger_point_type *)outdoor_trigger_ptr;
  trigger_point_type *lowlight_trigger =
    (trigger_point_type *)lowlight_trigger_ptr;

  rt->ratio = 0.0;
  rt->lighting = TRIGGER_NORMAL;

  ISP_DBG(ISP_MOD_COM,"lux_idx %f, current_real_gain %f\n",
    aec_out->lux_idx, aec_out->real_gain);

  switch (tuning) {
  /* 0 is Lux Index based */
  case 0: {
    if (aec_out->lux_idx < outdoor_trigger->lux_index_start) {
      rt->ratio = isp_util_calc_interpolation_weight(
        aec_out->lux_idx, outdoor_trigger->lux_index_end,
        outdoor_trigger->lux_index_start);
      rt->lighting = TRIGGER_OUTDOOR;
    } else if (aec_out->lux_idx > lowlight_trigger->lux_index_start) {
      rt->ratio = 1.0 - isp_util_calc_interpolation_weight(
        aec_out->lux_idx, lowlight_trigger->lux_index_start,
        lowlight_trigger->lux_index_end);
      rt->lighting = TRIGGER_LOWLIGHT;
    } else {
      rt->ratio = 1.0;
      rt->lighting = TRIGGER_NORMAL;
    }
  }
    break;

  /* 1 is Gain Based */
  case 1: {
    real_gain = aec_out->real_gain;

    if (real_gain < outdoor_trigger->gain_start) {
      rt->ratio = isp_util_calc_interpolation_weight(real_gain,
        outdoor_trigger->gain_end, outdoor_trigger->gain_start);
      rt->lighting = TRIGGER_OUTDOOR;
    } else if (real_gain > lowlight_trigger->gain_start) {
      rt->ratio = 1.0 - isp_util_calc_interpolation_weight(real_gain,
        lowlight_trigger->gain_start, lowlight_trigger->gain_end);
      rt->lighting = TRIGGER_LOWLIGHT;
    } else {
      rt->ratio = 1.0;
      rt->lighting = TRIGGER_NORMAL;
    }
  }
    break;

  default: {
    CDBG_ERROR("get_trigger_ratio: tunning type %d is not supported.\n",
      tuning);
  }
    break;
  }

  return 0;
} /* isp_util_get_ratio2 */

/** isp32_util_get_awb_cct_type:
 *
 *    @trigger:
 *    @chromatix_ptr:
 *
 **/
static awb_cct_type isp32_util_get_awb_cct_type(cct_trigger_info* trigger,
  void *chromatix_ptr)
{
  chromatix_parms_type *p_chromatix = chromatix_ptr;
  awb_cct_type cct_type = AWB_CCT_TYPE_TL84;

  ISP_DBG(ISP_MOD_COM,"%s: CCT %f D65 %f %f A %f %f", __func__,
    trigger->mired_color_temp,
    trigger->trigger_d65.mired_end,
    trigger->trigger_d65.mired_start,
    trigger->trigger_A.mired_start,
    trigger->trigger_A.mired_end);

  if (trigger->mired_color_temp <= trigger->trigger_d65.mired_end) {
    cct_type = AWB_CCT_TYPE_D65;
  } else if ((trigger->mired_color_temp > trigger->trigger_d65.mired_end) &&
             (trigger->mired_color_temp < trigger->trigger_d65.mired_start)) {
    cct_type = AWB_CCT_TYPE_D65_TL84;
  } else if ((trigger->mired_color_temp > trigger->trigger_A.mired_start) &&
             (trigger->mired_color_temp < trigger->trigger_A.mired_end)) {
    cct_type = AWB_CCT_TYPE_TL84_A;
  } else if (trigger->mired_color_temp >= trigger->trigger_A.mired_end) {
    cct_type = AWB_CCT_TYPE_A;
  }
  /* else its TL84*/

  return cct_type;
} /*isp_util_get_awb_cct_type*/

/** isp_pipeline32_get_roi_map:
 *
 *    @pipeline_ptr: isp pipeline
 *    @hw_zoom_entry: zoom entry
 *
 * Get regions of interest map
 *
 * This function executes in ISP thread context
 *
 * Return none.
 **/
static int isp_pipeline32_get_roi_map(void *pipeline_ptr,
  isp_hw_zoom_param_entry_t *hw_zoom_entry)
{
  int rc = 0;
  int i;
  isp_pipeline_t *pipeline = (isp_pipeline_t *)pipeline_ptr;
  isp_pixel_line_info_t fov_output;

  rc = pipeline->mod_ops[ISP_MOD_FOV]->get_params(
         pipeline->mod_ops[ISP_MOD_FOV]->ctrl, ISP_PIX_GET_FOV_OUTPUT, NULL, 0,
         &fov_output, sizeof(isp_pixel_line_info_t) * ISP_PIX_PATH_MAX);

  if (rc < 0) {
    CDBG_ERROR("%s: get fov output error, rc = %d\n", __func__, rc);
    return rc;
  }

  /* fill in roi info into, it is the same for all streams */
  for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
    hw_zoom_entry[i].roi_map_info.first_pixel = fov_output.first_pixel;
    hw_zoom_entry[i].roi_map_info.last_pixel  = fov_output.last_pixel;
    hw_zoom_entry[i].roi_map_info.first_line  = fov_output.first_line;
    hw_zoom_entry[i].roi_map_info.last_line   = fov_output.last_line;

  }
  CDBG_ERROR("%s: ROI: first pix %d, last pix %d, first ln %d, last line %d",
    __func__,
    hw_zoom_entry[0].roi_map_info.first_pixel,
    hw_zoom_entry[0].roi_map_info.last_pixel,
    hw_zoom_entry[0].roi_map_info.first_line,
    hw_zoom_entry[0].roi_map_info.last_line);

  return rc;
}

/** isp_pipeline32_do_zoom
 *
 *    @pipeline_ptr: isp pipeline
 *    @crop_factor: crop factor
 *
 *  Aplies the requred zoom.
 **/
static int isp_pipeline32_do_zoom(void *pipeline_ptr,
  isp_hw_set_crop_factor_t *crop_factor)
{
  int rc = -1;
  isp_pipeline_t *pipeline = (isp_pipeline_t *)pipeline_ptr;

  rc = pipeline->mod_ops[ISP_MOD_FOV]->set_params(
         pipeline->mod_ops[ISP_MOD_FOV]->ctrl, ISP_HW_MOD_SET_ZOOM_RATIO,
         (void *)&pipeline->pix_params.cfg_and_3a_params.cfg,
         sizeof(isp_hw_pix_setting_params_t));

  if (rc < 0) {
    CDBG_ERROR("%s: SET_ZOOM_RATIO error in fov, rc = %d\n",
      __func__, rc);
    return rc;
  }

  rc = pipeline->mod_ops[ISP_MOD_SCALER]->set_params(
         pipeline->mod_ops[ISP_MOD_SCALER]->ctrl, ISP_HW_MOD_SET_ZOOM_RATIO,
         (void *)&pipeline->pix_params.cfg_and_3a_params.cfg,
         sizeof(isp_hw_pix_setting_params_t));

  if (rc < 0) {
    CDBG_ERROR("%s: SET_ZOOM_RATIO error in scaler, rc = %d\n",
      __func__, rc);
    return rc;
  }

  rc = pipeline->mod_ops[ISP_MOD_FOV]->get_params(
         pipeline->mod_ops[ISP_MOD_FOV]->ctrl, ISP_HW_MOD_GET_FOV,
         (void *)&pipeline->pix_params.cfg_and_3a_params.cfg,
         sizeof(isp_hw_pix_setting_params_t), &crop_factor->hw_zoom_parm,
	       sizeof(crop_factor->hw_zoom_parm));

  if (rc < 0) {
    CDBG_ERROR("%s: ISP_PIX_GET_FOV error, rc = %d\n",
      __func__, rc);
    return rc;
  }

  return rc;
}

/** isp_pipeline32_destroy:
 *
 *    @dep: pointer to pipeline dependency data
 *
 * Free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return none.
 **/
static void isp_pipeline32_destroy(isp_hw_pix_dep_t *dep)
{
  if (dep && dep->op_ptr) {
    free(dep->op_ptr);
    dep->op_ptr = NULL;
  }
}

/** isp32_util_set_uv_subsample:
 *    @pipeline_ptr: pointer to pipeline dependency data
 *    @enable
 * Free private resources.
 *
 * Not implemented for ISP 32
 *
 * Return none.
 **/
static int isp32_util_set_uv_subsample(void *pipeline_ptr,
  boolean enable)
{
  CDBG_ERROR("%s: warning, not implemented for isp32", __func__);
  return 0;
}

/** isp32_reconfig_modules
 *
 *    @pix_ptr:
 *
 **/
int isp32_reconfig_modules(void *pix_ptr)
{
  int rc = 0, i;
  isp_pipeline_t *pix = (isp_pipeline_t *)pix_ptr;
  uint32_t data_offset = 0;
  uint8_t enb;
  int fd = pix->fd;
  isp_operation_cfg_t *op_ptr = (isp_operation_cfg_t *)(pix->dep.op_ptr);
  ISP_ModuleCfgPacked  *op_cmd = &op_ptr->op_cmd.moduleCfg;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];
  /* config all enable bits */
  for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
    enb = ((1 << i) & pix->pix_params.cur_module_mask)? 1 : 0;
    isp_hw_pix_set_module_cfg_bit(pix, (isp_hw_module_id_t)i, enb);
  }

  isp_pix_dump_ISP_ModuleCfgPacked(op_cmd);
  /* PIX operation configuration */
  memset(&cfg_cmd, 0, sizeof(cfg_cmd));
  memset(reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));

  cfg_cmd.num_cfg = 1;
  cfg_cmd.cmd_len = sizeof(ISP_ModuleCfgPacked);
  cfg_cmd.cfg_data = op_cmd;
  cfg_cmd.cfg_cmd = reg_cfg_cmd;

  /*ISP32_MODULE_CFG*/
  data_offset = 0;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP32_MODULE_CFG;
  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = data_offset;
  reg_cfg_cmd[0].u.rw_info.len = ISP32_MODULE_CFG_LEN;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: VFE core/module cfg error = %d\n", __func__, rc);
    return rc;
  }
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d", __func__, rc);

  return rc;
}

/** isp32_util_get_cds_trigger_pts:
 *    @pipeline_ptr: pointer to pipeline dependency data
 *    @enable
 * Free private resources.
 *
 * Return none.
 **/
static void isp32_util_get_params(void *in_params, uint32_t in_params_size,
  uint32_t param_id, void* out_param, uint32_t out_params_size, void *ctrl)
{
  ISP_DBG(ISP_MOD_COM,"%s:E, param_id %d", __func__, param_id);
  switch (param_id) {
  case ISP_PIPELINE_GET_CDS_TRIGGER_VAL: {
    modulesChromatix_t *chromatixPtr = (modulesChromatix_t *)ctrl;
    isp_uv_subsample_t *uv_subsample_ctrl = (isp_uv_subsample_t *)out_param;
    if (NULL != uv_subsample_ctrl) {
      uv_subsample_ctrl->trigger_A = 0;
      uv_subsample_ctrl->trigger_B = 0;
      ISP_DBG(ISP_MOD_COM,"%s: trigger pts set to 0 for isp32 ", __func__);
    }
  }
    break;

  case ISP_HW_MOD_GET_TINTLESS_RO: {
    isp_pipeline_t *pipeline = (isp_pipeline_t *)ctrl;
    tintless_mesh_rolloff_array_t *rolloff = (tintless_mesh_rolloff_array_t *)out_param;
    int rc = 0;
    if (pipeline->mod_ops[ISP_MOD_ROLLOFF] && pipeline->mod_ops[ISP_MOD_ROLLOFF]->get_params) {
      ISP_DBG(ISP_MOD_COM,"%s: ISP_HW_MOD_GET_TINTLESS_RO\n", __func__);
      rc = pipeline->mod_ops[ISP_MOD_ROLLOFF]->get_params(pipeline->mod_ops[ISP_MOD_ROLLOFF]->ctrl,
             ISP_HW_MOD_GET_TINTLESS_RO, in_params, in_params_size,
             rolloff, sizeof(tintless_mesh_rolloff_array_t));
    }

  }
    break;

  case ISP_HW_MOD_GET_VFE_DIAG_INFO_USER: {
      isp_pipeline_t *pipeline = (isp_pipeline_t *)ctrl;
      vfe_diagnostics_t *vfe_diag = (vfe_diagnostics_t *)out_param;
      int i, rc = 0;
      for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
        if (pipeline->mod_ops[i] && pipeline->mod_ops[i]->get_params) {
          ISP_DBG(ISP_MOD_COM,"%s: module id = %d, ISP_HW_MOD_GET_VFE_DIAG_INFO_USER\n",
            __func__, i);
          rc = pipeline->mod_ops[i]->get_params(pipeline->mod_ops[i]->ctrl,
                 ISP_HW_MOD_GET_VFE_DIAG_INFO_USER, in_params, in_params_size,
                 vfe_diag, sizeof(vfe_diagnostics_t));
        }
      }
  }
    break;

  default: {
    ISP_DBG(ISP_MOD_COM,"%s: param_id %d not supported ", __func__, param_id);
  }
    break;
  }
  return;
}

/** isp_pipeline32_init:
 *
 *    @dep: pointer to pipeline dependency data
 *
 * Allocate instance private data for pipeline.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
int isp_pipeline32_init(isp_hw_pix_dep_t *dep)
{
  if (!dep) {
    CDBG_ERROR("%s: invalid input", __func__);
    return -EINVAL;
  }

  dep->op_ptr = malloc(sizeof(ISP_OperationConfigCmdType));
  if (dep->op_ptr == NULL) {
    CDBG_ERROR("%s: no mem\n", __func__);
    return -ENOMEM;
  }

  memset(dep->op_ptr, 0, sizeof(ISP_OperationConfigCmdType));

  dep->max_mod_mask_continuous_bayer = max_mod_mask_continuous_bayer;
  dep->max_mod_mask_burst_bayer = max_mod_mask_burst_bayer;
  dep->max_supported_stats = max_supported_stats;
  dep->max_mod_mask_continuous_yuv = max_mod_mask_continuous_yuv;
  dep->max_mod_mask_burst_yuv = max_mod_mask_burst_yuv;
  dep->num_mod_cfg_order_bayer = sizeof(mod_cfg_order_bayer) / sizeof(uint16_t);
  dep->mod_cfg_order_bayer = mod_cfg_order_bayer;
  dep->num_mod_cfg_order_yuv = sizeof(mod_cfg_order_yuv) / sizeof(uint16_t);
  dep->mod_cfg_order_yuv = mod_cfg_order_yuv;
  dep->num_mod_trigger_update_order_bayer =
    sizeof(mod_trigger_update_order_bayer) / sizeof(uint16_t);
  dep->mod_trigger_update_order_bayer = mod_trigger_update_order_bayer;

  dep->destroy = isp_pipeline32_destroy;
  dep->operation_config = isp_pipeline32_operation_config;
  dep->module_start = isp_pipeline32_module_start;
  dep->do_zoom = isp_pipeline32_do_zoom;
  dep->get_roi_map = isp_pipeline32_get_roi_map;
  dep->util_get_aec_ratio_lowlight = isp32_util_get_aec_ratio_lowlight;
  dep->util_get_aec_ratio_bright_low = isp32_util_get_aec_ratio_bright_low;
  dep->util_get_aec_ratio_bright = isp32_util_get_aec_ratio_bright;
  dep->util_get_awb_cct_type = isp32_util_get_awb_cct_type;
  dep->util_set_uv_subsample = isp32_util_set_uv_subsample;
  dep->util_get_param = isp32_util_get_params;
  dep->module_reconf_module = isp32_reconfig_modules;
  return 0;
}
