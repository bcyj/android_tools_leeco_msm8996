/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
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
#include "isp_pipeline44.h"
#include "isp_log.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef ENABLE_ISP_PIPELINE44_LOGGING
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
  (1 << ISP_MOD_COLOR_CONV) |
  (1 << ISP_MOD_COLOR_CORRECT) |
  (1 << ISP_MOD_COLOR_XFORM) |
  (1 << ISP_MOD_CHROMA_SUPPRESS) |
  (1 << ISP_MOD_LA) |
  (1 << ISP_MOD_MCE) |
  (1 << ISP_MOD_SCE) |
  (1 << ISP_MOD_WB) |
  (1 << ISP_MOD_GAMMA) |
  (1 << ISP_MOD_FOV) |
  (1 << ISP_MOD_SCALER) |
  (1 << ISP_MOD_BCC) |
  (1 << ISP_MOD_CLAMP)|
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
  (1 << ISP_MOD_COLOR_CONV) |
  (1 << ISP_MOD_COLOR_CORRECT) |
  (1 << ISP_MOD_CHROMA_SUPPRESS) |
  (1 << ISP_MOD_LA) |
  (1 << ISP_MOD_MCE) |
  (1 << ISP_MOD_SCE) |
  (1 << ISP_MOD_WB) |
  (1 << ISP_MOD_GAMMA) |
  (1 << ISP_MOD_FOV) |
  (1 << ISP_MOD_SCALER) |
  (1 << ISP_MOD_BCC) |
  (1 << ISP_MOD_CLAMP)
  //(1 << ISP_MOD_STATS)
);

static const uint32_t max_supported_stats =
(
  //(1 << MSM_ISP_STATS_AWB)   |
  (1 << MSM_ISP_STATS_RS)    |  /* share with BG */
  (1 << MSM_ISP_STATS_CS)    |  /* shared with BF */
  (1 << MSM_ISP_STATS_IHIST) |  /* for both CS and RS stats modules */
  (1 << MSM_ISP_STATS_BE)    |
  (1 << MSM_ISP_STATS_BF)    |
  (1 << MSM_ISP_STATS_BG)    |
  (1 << MSM_ISP_STATS_BHIST)
);

static const uint32_t max_mod_mask_continuous_yuv =
    (1 << ISP_MOD_FOV) |
    (1 << ISP_MOD_SCALER) |
    (1 << ISP_MOD_DEMUX) |
    (1 << ISP_MOD_CLAMP);

static const uint32_t max_mod_mask_burst_yuv =
(
  (1 << ISP_MOD_FOV) |
  (1 << ISP_MOD_SCALER) |
  (1 << ISP_MOD_DEMUX) |
  (1 << ISP_MOD_CLAMP)
);

static uint16_t mod_cfg_order_bayer[] =
{
  ISP_MOD_LINEARIZATION, ISP_MOD_MCE,             ISP_MOD_COLOR_CONV,
  ISP_MOD_CLAMP,         ISP_MOD_CHROMA_SUPPRESS, ISP_MOD_WB,
  ISP_MOD_DEMOSAIC,      ISP_MOD_BPC,             ISP_MOD_BCC,
  ISP_MOD_DEMUX,         ISP_MOD_COLOR_CORRECT,   ISP_MOD_ABF,
  ISP_MOD_CLF,           ISP_MOD_GAMMA,           ISP_MOD_ROLLOFF,
  ISP_MOD_LA,            ISP_MOD_SCE,             ISP_MOD_SCALER,
  ISP_MOD_FOV,           ISP_MOD_COLOR_XFORM
};

static uint16_t mod_cfg_order_yuv[] =
{
  ISP_MOD_CLAMP, ISP_MOD_DEMUX, ISP_MOD_SCALER, ISP_MOD_FOV
};

static uint16_t mod_trigger_update_order_bayer[] =
{
  ISP_MOD_LINEARIZATION, ISP_MOD_ROLLOFF,         ISP_MOD_WB,
  ISP_MOD_DEMOSAIC,      ISP_MOD_BPC,             ISP_MOD_BCC,
  ISP_MOD_ABF,           ISP_MOD_STATS,           ISP_MOD_LA,
  ISP_MOD_DEMUX,         ISP_MOD_COLOR_CORRECT,   ISP_MOD_COLOR_CONV,
  ISP_MOD_GAMMA,         ISP_MOD_CLF,             ISP_MOD_MCE,
  ISP_MOD_SCE,           ISP_MOD_CHROMA_SUPPRESS
};

/** isp_hw_pix_dump_sub_stats_enable
 *
 *    @op_cmd:
 *    @current_stats_mask:
 *    @stats_enb:
 *
 **/
static void isp_hw_pix_dump_sub_stats_enable(ISP_OperationConfigCmdType *op_cmd,
  uint32_t current_stats_mask, uint32_t stats_enb)
{
    uint8_t enb = 0;

    ISP_DBG(ISP_MOD_COM,"%s:config sub_stats_enable, stats_mod_enb = %d, stats mask = 0x%x\n",
         __func__, stats_enb, current_stats_mask);

    /* only show enabled stats module,
       kernel to set hw enable bit due to timing concern
       kernel set the hw bit when each sub-stats module request stream during streamon*/
    enb = (current_stats_mask & (1 << MSM_ISP_STATS_AWB))? 1 : 0;
    enb = enb & stats_enb;
    ISP_DBG(ISP_MOD_COM,"%s: AWB enb = %d\n", __func__, enb);

    enb = (current_stats_mask & (1 << MSM_ISP_STATS_RS))? 1 : 0;
    enb = enb & stats_enb;
    ISP_DBG(ISP_MOD_COM,"%s: RS enb = %d\n", __func__, enb);

    enb = (current_stats_mask & (1 << MSM_ISP_STATS_CS))? 1 : 0;
    enb = enb & stats_enb;
    ISP_DBG(ISP_MOD_COM,"%s: CS enb = %d\n", __func__, enb);

    enb = (current_stats_mask & (1 << MSM_ISP_STATS_IHIST))? 1 : 0;
    enb = enb & stats_enb;
    ISP_DBG(ISP_MOD_COM,"%s: IHIST enb = %d\n", __func__, enb);

    enb = (current_stats_mask & (1 << MSM_ISP_STATS_BG))? 1 : 0;
    enb = enb & stats_enb;
    ISP_DBG(ISP_MOD_COM,"%s: BG enb = %d\n", __func__, enb);

    enb = (current_stats_mask & (1 << MSM_ISP_STATS_BE))? 1 : 0;
    enb = enb & stats_enb;
    ISP_DBG(ISP_MOD_COM,"%s: BE enb = %d\n", __func__, enb);

    enb = (current_stats_mask & (1 << MSM_ISP_STATS_BF))? 1 : 0;
    enb = enb & stats_enb;
    ISP_DBG(ISP_MOD_COM,"%s: BF enb = %d\n", __func__, enb);

    enb = (current_stats_mask & (1 << MSM_ISP_STATS_BHIST))? 1 : 0;
    enb = enb & stats_enb;
    ISP_DBG(ISP_MOD_COM,"%s: BHIST enb = %d\n", __func__, enb);

}

/** isp_hw_pix_set_module_cfg_bit
 *
 *    @pix:
 *    @mod_id:
 *    @end:
 *
 **/
static void isp_hw_pix_set_module_cfg_bit(isp_pipeline_t *pix,
  isp_hw_module_id_t mod_id, uint8_t enb)
{
  isp_pix_params_t *params = &pix->pix_params;
  isp_operation_cfg_t *op_ptr = (isp_operation_cfg_t *)(pix->dep.op_ptr);
  ISP_OperationConfigCmdType *op_cmd = &op_ptr->op_cmd;

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
    op_cmd->moduleCfg.cropViewEnable = enb;
    op_cmd->moduleCfg.cropEncEnable = enb;
  }
    break;

  case ISP_MOD_SCALER: {
    op_cmd->moduleCfg.scalerViewEnable = enb;
    op_cmd->moduleCfg.scalerEncEnable = enb;
  }
    break;

  case ISP_MOD_COLOR_XFORM: {
    op_cmd->moduleCfg.colorXformEncEnable = enb;
    op_cmd->moduleCfg.colorXformViewEnable = enb;
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
  }
    break;

  default: {
  }
    break;
  }
}

/** isp_pipeline44_operation_config
 *
 *    @pix_ptr:
 *    @is_bayer_input:
 *
 **/
int isp_pipeline44_operation_config(void *pix_ptr, int is_bayer_input)
{
  int i;
  isp_pipeline_t *pix = (isp_pipeline_t *)pix_ptr;
  uint8_t enb;
  uint32_t mce_and_chroma_suppress = ((1 << ISP_MOD_CHROMA_SUPPRESS) | (1 << ISP_MOD_MCE));
  isp_pix_params_t *params = &pix->pix_params;
  isp_operation_cfg_t *op_ptr = (isp_operation_cfg_t *)(pix->dep.op_ptr);
  ISP_OperationConfigCmdType *op_cmd = &op_ptr->op_cmd;
  isp_hw_pix_setting_params_t *cfg = &params->cfg_and_3a_params.cfg;

  /* zero the moduleCfg first */
  memset(&op_cmd->moduleCfg, 0, sizeof(op_cmd->moduleCfg));

  /* config all enable bits
   stats module hw cfg not set here
   kernel set hw bit when sub-stats modules request stream when streamon*/
  for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
    enb = ((1 << i) & pix->pix_params.cur_module_mask)? 1 : 0;

    if (i == (int)ISP_MOD_STATS) {
      isp_hw_pix_dump_sub_stats_enable(op_cmd, (params->cur_stats_mask), enb);
      continue;
    }

    isp_hw_pix_set_module_cfg_bit(pix, (isp_hw_module_id_t)i, enb);
  }

  /* MCE and Chroma Suppress share the same enable bit */
  if (!(mce_and_chroma_suppress & pix->pix_params.cur_module_mask))
    op_cmd->moduleCfg.chromaSuppressionMceEnable= 0;

  /*YUV sensor bypass demosaic and use chroma upsample*/
  if (is_bayer_input == 0) {
    op_cmd->moduleCfg.chromaUpsampleEnable = 1;
  } else {
    op_cmd->moduleCfg.chromaUpsampleEnable = 0;
  }

  op_cmd->moduleCfg.realignmentBufEnable = 0;

  /*op_cmd->statisticsComposite = 1;*/

  if (is_bayer_input) {
    op_cmd->ispStatsCfg.colorConvEnable = 1;
    op_cmd->ispStatsCfg.bayerHistSelect = 1;
  } else {
    op_cmd->ispStatsCfg.colorConvEnable = 0;
  }

  /* isp Cfg , set format*/
  op_cmd->ispCfg.inputPixelPattern =
    isp_fmt_to_pix_pattern(cfg->camif_cfg.sensor_output_fmt);

  return 0;
}

/** isp_pix_dump_ISP_ModuleCfgPacked
 *
 *    @module_cfg:
 *
 **/
static void isp_pix_dump_ISP_ModuleCfgPacked(ISP_ModuleCfgPacked *module_cfg)
{
  ISP_DBG(ISP_MOD_COM,"%s: "
       "blackLevelCorrectionEnable = %d,\n"
       "lensRollOffEnable = %d,\n"
       "demuxEnable = %d,\n"
       "chromaUpsampleEnable = %d,\n"
       "demosaicEnable = %d,\n"
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
       "colorXformEncEnable = %d,\n"
       "colorXformViewEnable = %d,\n"
       "scalerEncEnable = %d,\n"
       "scalerViewEnable = %d,\n"
       "cropEncEnable = %d,\n"
       "cropViewEnable = %d,\n"
       "realignmentBufEnable = %d\n", __func__,
  module_cfg->blackLevelCorrectionEnable,   /* bit 0  */
  module_cfg->lensRollOffEnable,            /* bit 1  */
  module_cfg->demuxEnable,                  /* bit 2  */
  module_cfg->chromaUpsampleEnable,         /* bit 3  */
  module_cfg->demosaicEnable,               /* bit 4  */
  module_cfg->whiteBalanceEnable,           /* bit 11 */
  module_cfg->clfEnable,                    /* bit 12 */
  module_cfg->colorCorrectionEnable,        /* bit 13 */
  module_cfg->rgbLUTEnable,                 /* bit 14 */
  module_cfg->statsIhistEnable,             /* bit 15 */
  module_cfg->lumaAdaptationEnable,         /* bit 16 */
  module_cfg->chromaEnhanEnable,            /* bit 17 */
  module_cfg->statsSkinBhistEnable,         /* bit 18 */
  module_cfg->chromaSuppressionMceEnable,   /* bit 19 */
  module_cfg->skinEnhancementEnable,        /* bit 20 */
  module_cfg->colorXformEncEnable,          /* bit 21 */
  module_cfg->colorXformViewEnable,         /* bit 22 */
  module_cfg->scalerEncEnable,              /* bit 23 */
  module_cfg->scalerViewEnable,             /* bit 24 */
  module_cfg->cropEncEnable,                /* bit 27 */
  module_cfg->cropViewEnable,               /* bit 28 */
  module_cfg->realignmentBufEnable);        /* bit 29 */
}

/** isp_pipeline_reset_hist_dmi
 *
 *    @fd:
 *    @dmi_channel
 *
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

  cfg_cmd.cfg_data = (void *)&hist_reset_cfg;
  cfg_cmd.cmd_len = sizeof(hist_reset_cfg);
  cfg_cmd.cfg_cmd = (void *) &reg_cfg_cmd;
  cfg_cmd.num_cfg = 5;

  /* set dmi to proper hist stats bank */
  hist_reset_cfg.set_channel = ISP44_DMI_CFG_DEFAULT + dmi_cahnnel;
  cmd_offset = 0;
  cmd_len = 1 * sizeof(uint32_t);

  isp_pipeline_util_pack_cfg_cmd(fd, &reg_cfg_cmd[0], cmd_offset, cmd_len,
    VFE_WRITE_MB, ISP44_DMI_CFG_OFF);

  /* set start addr = 0*/
  hist_reset_cfg.set_start_addr = 0;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  isp_pipeline_util_pack_cfg_cmd(fd, &reg_cfg_cmd[1], cmd_offset, cmd_len,
    VFE_WRITE_MB, ISP44_DMI_ADDR);

  /* memset hi and lo tbl = all 0,
     64 bit write: regarding interlieve uint64 table
       hi_tbl_offset = table offset
       lo_tbl_offset = table offset + sizeof(uint32_t) */
  tbl_len = sizeof(uint64_t) * 256;
  memset(hist_reset_cfg.table, 0, tbl_len);
  cmd_offset += cmd_len;
  cmd_len = tbl_len;
  isp_pipeline_util_pack_dmi_cmd(fd, &reg_cfg_cmd[2], cmd_offset,
    cmd_offset + sizeof(uint32_t), cmd_len, VFE_WRITE_DMI_64BIT);

  /* reset the sart addr = 0 */
  hist_reset_cfg.reset_start_addr = 0;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  isp_pipeline_util_pack_cfg_cmd(fd, &reg_cfg_cmd[3], cmd_offset, cmd_len,
    VFE_WRITE_MB, ISP44_DMI_ADDR);

  /* set dmi to proper hist stats bank */
  hist_reset_cfg.reset_channel =
    ISP44_DMI_CFG_DEFAULT + ISP44_DMI_NO_MEM_SELECTED;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  isp_pipeline_util_pack_cfg_cmd(fd, &reg_cfg_cmd[4], cmd_offset, cmd_len,
    VFE_WRITE_MB, ISP44_DMI_CFG_OFF);

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);

  if (rc < 0) {
    CDBG_ERROR("%s: VFE reset hist DMI error = %d\n", __func__, rc);
    return rc;
  }

  return rc;
}

/** isp_pipeline44_module_start
 *
 *    @pix_ptr:
 *
 **/
int isp_pipeline44_module_start(void *pix_ptr)
{
  int rc = 0;
  isp_pipeline_t *pix = (isp_pipeline_t *)pix_ptr;
  uint32_t data_offset = 0;
  int fd = pix->fd;
  isp_pix_params_t *params = &pix->pix_params;
  isp_operation_cfg_t *op_ptr = (isp_operation_cfg_t *)(pix->dep.op_ptr);
  ISP_OperationConfigCmdType *op_cmd = &op_ptr->op_cmd;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[5];
  uint32_t *module_cfg;

  rc = isp_pipeline_reset_hist_dmi(fd, STATS_BHIST_RAM0);
  rc = isp_pipeline_reset_hist_dmi(fd, STATS_BHIST_RAM1);
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
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP44_CORE_CFG;
  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = data_offset;
  reg_cfg_cmd[0].u.rw_info.len = ISP44_CORE_CFG_LEN;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;

  /*ISP44_MODULE_CFG*/
  data_offset += reg_cfg_cmd[0].u.rw_info.len;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP44_MODULE_CFG;
  reg_cfg_cmd[1].u.rw_info.cmd_data_offset = data_offset;
  reg_cfg_cmd[1].u.rw_info.len = ISP44_MODULE_CFG_LEN;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE;

  /*ISP44_REALIGN_BUF_CFG*/
  module_cfg = (uint32_t *)((uint8_t *)cfg_cmd.cfg_data + data_offset);
  ISP_DBG(ISP_MOD_COM,"%s: module_cfg = 0x%x\n", __func__, *module_cfg);
  data_offset += reg_cfg_cmd[1].u.rw_info.len;
  reg_cfg_cmd[2].u.rw_info.reg_offset = ISP44_REALIGN_BUF_CFG;
  reg_cfg_cmd[2].u.rw_info.cmd_data_offset = data_offset;
  reg_cfg_cmd[2].u.rw_info.len = ISP44_REALIGN_BUF_CFG_LEN;
  reg_cfg_cmd[2].cmd_type = VFE_WRITE;

  /*ISP44_CHROMA_UPSAMPLE_CFG*/
  data_offset += reg_cfg_cmd[2].u.rw_info.len;
  reg_cfg_cmd[3].u.rw_info.reg_offset = ISP44_CHROMA_UPSAMPLE_CFG;
  reg_cfg_cmd[3].u.rw_info.cmd_data_offset = data_offset;
  reg_cfg_cmd[3].u.rw_info.len = ISP44_CHROMA_UPSAMPLE_CFG_LEN;
  reg_cfg_cmd[3].cmd_type = VFE_WRITE;

  /*ISP44_STATS_CFG*/
  data_offset += reg_cfg_cmd[3].u.rw_info.len;
  reg_cfg_cmd[4].u.rw_info.reg_offset = ISP44_STATS_CFG;
  reg_cfg_cmd[4].u.rw_info.cmd_data_offset = data_offset;
  reg_cfg_cmd[4].u.rw_info.len = ISP44_STATS_CFG_LEN;
  reg_cfg_cmd[4].cmd_type = VFE_WRITE;

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);

  if (rc < 0) {
    CDBG_ERROR("%s: VFE core/module cfg error = %d\n", __func__, rc);
    return rc;
  }

  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d", __func__, rc);

  return rc;
}

/** isp44_util_get_aec_ratio_lowlight
 *
 *    @tunning_type:
 *    @trigger_ptr:
 *    @aec_out:
 *    @is_snap_mode
 *
 *   Get trigger ratio based on lowlight trigger.
 *   Please note that ratio is the weight of the normal light.
 *    LOW           Mix            Normal
 *    ----------|----------------|----------
 *        low_end(ex: 400)    low_start(ex: 300)
 *
 **/
static float isp44_util_get_aec_ratio_lowlight(unsigned char tunning_type,
  void *trigger_ptr, aec_update_t* aec_out, int8_t is_snap_mode)
{
  float normal_light_ratio = 0.0, real_gain;
  float ratio_to_low_start = 0.0;
  tuning_control_type tunning = (tuning_control_type)tunning_type;
  trigger_point_type *trigger = (trigger_point_type *)trigger_ptr;

  switch (tunning) {
  /* 0 is Lux Index based */
  case 0: {
    ratio_to_low_start =isp_util_calc_interpolation_weight(
      aec_out->lux_idx, trigger->lux_index_start, trigger->lux_index_end);
    }
      break;
  /* 1 is Gain Based */
  case 1: {
    real_gain = aec_out->real_gain;
    ratio_to_low_start = isp_util_calc_interpolation_weight(real_gain,
      trigger->gain_start, trigger->gain_end);
  }
    break;

  default: {
    CDBG_ERROR("get_trigger_ratio: tunning type %d is not supported.\n",
      tunning);
  }
    break;
  }

  /*ratio_to_low_start is the sitance to low start,
    the smaller distance to low start,
    the higher ratio applied normal light*/
  normal_light_ratio = 1 - ratio_to_low_start;

  if (normal_light_ratio < 0) {
    normal_light_ratio = 0;
  } else if (normal_light_ratio > 1.0) {
    normal_light_ratio = 1.0;
  }

  return normal_light_ratio;
} /* isp_util_get_aec_ratio */

/** isp44_util_get_aec_ratio_bright
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
static float isp44_util_get_aec_ratio_bright(unsigned char tunning_type,
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

/** isp44_util_get_aec_ratio_bright_low
 *
 *    @tuning_type:
 *    @outdoor_trigger_ptr:
 *    @lowlight_trigger_ptr:
 *    @aec_out:
 *    @is_snap_mode:
 *    @rt:
 *
 *   Get trigger ratio based on outdoor trigger & lowlight
 *   trigger Please note that rt.ratio means the weight of the normal light.
 *
 *   LOW      MIX      NORMAL   LIGHT             MIX            BRIGHT
 *   ------|---------|---------------------------|-------------|-------------
 *       low_end   low_start                bright_start    bright_end
 *
 **/
static int isp44_util_get_aec_ratio_bright_low(unsigned char tuning_type,
  void *outdoor_trigger_ptr, void *lowlight_trigger_ptr, aec_update_t* aec_out,
  int8_t is_snap_mode, trigger_ratio_t *rt)
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

  /* 0 is Lux Index based, 1 is gain base */
  switch (tuning) {
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

/** isp44_util_get_awb_cct_type
 *
 *    @trigger:
 *    @chromatix_ptr:
 *
 **/
static awb_cct_type isp44_util_get_awb_cct_type(cct_trigger_info* trigger,
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
             (trigger->mired_color_temp <= trigger->trigger_d65.mired_start)) {
    cct_type = AWB_CCT_TYPE_D65_TL84;
  } else if ((trigger->mired_color_temp >= trigger->trigger_A.mired_start) &&
             (trigger->mired_color_temp < trigger->trigger_A.mired_end)) {
    cct_type = AWB_CCT_TYPE_TL84_A;
  } else if (trigger->mired_color_temp >= trigger->trigger_A.mired_end) {
    cct_type = AWB_CCT_TYPE_A;
  }
  /* else its TL84*/

  return cct_type;
} /*isp_util_get_awb_cct_type*/

/** isp44_util_set_uv_subsample
 *
 *    @pipeline_ptr:
 *    @enable:
 *
 **/
static int isp44_util_set_uv_subsample(void *pipeline_ptr, boolean enable)
{
  int rc = 0;
  int i = 0;
  isp_pipeline_t *pipeline = (isp_pipeline_t *)pipeline_ptr;
  isp_hw_pix_setting_params_t *pix_setting =
    &pipeline->pix_params.cfg_and_3a_params.cfg;

  for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
    if (enable) {
      rc = pipeline->mod_ops[ISP_MOD_SCALER]->get_params(
             pipeline->mod_ops[ISP_MOD_SCALER]->ctrl,
             ISP_PIX_GET_UV_SUBSAMPLE_SUPPORTED, (void *)pix_setting,
             sizeof(isp_hw_pix_setting_params_t),
             (void *)&pix_setting->outputs[i], sizeof(isp_hwif_output_cfg_t));

      if (rc < 0) {
        CDBG_ERROR("%s: ISP_PIX_GET_UV_SUBSAMPLE_SUPPORTED error in scaler, rc = %d\n",
          __func__, rc);
        return rc;
      }
    } else {
      pix_setting->outputs[i].need_uv_subsample = 0;
    }
  }

  /* config scaler according to the extra chroma subsample mode*/
  rc = pipeline->mod_ops[ISP_MOD_SCALER]->set_params(
         pipeline->mod_ops[ISP_MOD_SCALER]->ctrl, ISP_HW_MOD_SET_MOD_CONFIG,
         (void *)pix_setting, sizeof(isp_hw_pix_setting_params_t));

  if (rc < 0) {
    CDBG_ERROR("%s: ISP_HW_MOD_SET_MOD_CONFIG error in scaler, rc = %d\n",
      __func__, rc);
    return rc;
  }

  rc = pipeline->mod_ops[ISP_MOD_FOV]->set_params(
         pipeline->mod_ops[ISP_MOD_FOV]->ctrl, ISP_HW_MOD_SET_MOD_CONFIG,
         (void *)pix_setting, sizeof(isp_hw_pix_setting_params_t));

  if (rc < 0) {
    CDBG_ERROR("%s: ISP_HW_MOD_SET_MOD_CONFIG error in scaler, rc = %d\n",
      __func__, rc);
    return rc;
  }

  return rc;
}

/** isp_pipeline44_module_enable_notify
 *
 *    @pix_ptr:
 *    @notify_data:
 *
 **/
static int isp_pipeline44_module_enable_notify(void *pix_ptr,
  isp_mod_enable_motify_t *notify_data)
{
  isp_pipeline_t *pix = (isp_pipeline_t *)pix_ptr;
  isp_hw_pix_set_module_cfg_bit(pix, notify_data->mod_id, notify_data->enable);

  return 0;
}

/** isp_pipeline44_get_roi_map
 *
 *    @pipeline_ptr:
 *    @hw_zoom_entry:
 *
 **/
static int isp_pipeline44_get_roi_map(void *pipeline_ptr,
  isp_hw_zoom_param_entry_t *hw_zoom_entry)
{
  int rc = 0;
  int i, j;
  isp_pipeline_t *pipeline = (isp_pipeline_t *)pipeline_ptr;
  isp_hw_pix_setting_params_t *pix_settings = &pipeline->pix_params.cfg_and_3a_params.cfg;
  isp_pixel_window_info_t scaler_output[ISP_PIX_PATH_MAX];
  isp_pixel_line_info_t fov_output[ISP_PIX_PATH_MAX];
  /* path idx = ENC or VIEW*/
  uint32_t path_idx;

  rc = pipeline->mod_ops[ISP_MOD_SCALER]->get_params(
         pipeline->mod_ops[ISP_MOD_SCALER]->ctrl, ISP_PIX_GET_SCALER_OUTPUT,
         NULL, 0, scaler_output,
         sizeof(isp_pixel_window_info_t) * ISP_PIX_PATH_MAX);

  if (rc < 0) {
    CDBG_ERROR("%s: get scaler output error, rc = %d\n", __func__, rc);
    return rc;
  }

  rc = pipeline->mod_ops[ISP_MOD_FOV]->get_params(
         pipeline->mod_ops[ISP_MOD_FOV]->ctrl, ISP_PIX_GET_FOV_OUTPUT, NULL, 0,
         fov_output, sizeof(isp_pixel_line_info_t) * ISP_PIX_PATH_MAX);

  if (rc < 0) {
    CDBG_ERROR("%s: get fov output error, rc = %d\n", __func__, rc);
    return rc;
  }

  /* fill in roi info into all the zoom entries corresponding to stream*/
  for (i = 0; i < ISP_ZOOM_MAX_ENTRY_NUM; i++) {
    if (hw_zoom_entry[i].stream_id == 0) {
        ISP_DBG(ISP_MOD_COM,"%s: no stream on zoom entry [%d], stream id = %d\n",
          __func__, i, hw_zoom_entry[i].stream_id);
        continue;
    }

    /* add a guard value for path_idx */
    path_idx = ISP_PIX_PATH_MAX;
    /* by strream id, get path idx for each zoom entry*/
    path_idx = ISP_PIX_PATH_MAX;

    for (j = 0; j < ISP_PIX_PATH_MAX; j++) {
      if (pix_settings->outputs[j].stream_param.stream_id ==
        hw_zoom_entry[i].stream_id) {
          path_idx = j;
          break;
       }
    }

    if (path_idx == ISP_PIX_PATH_MAX) {
      /* path not found */
      CDBG_ERROR("%s: Zoom entry path not found!\n", __func__);
      return -EINVAL;
    }

    /* calculate the roi mapping from vfe ouput to camif input*/
    hw_zoom_entry[i].roi_map_info.first_pixel =
      fov_output[path_idx].first_pixel * scaler_output[path_idx].scaling_factor;
    hw_zoom_entry[i].roi_map_info.last_pixel = hw_zoom_entry[i].roi_map_info.first_pixel +
      ((fov_output[path_idx].last_pixel - fov_output[path_idx].first_pixel + 1) *
      scaler_output[path_idx].scaling_factor) -1;
    hw_zoom_entry[i].roi_map_info.first_line =
      fov_output[path_idx].first_line * scaler_output[path_idx].scaling_factor;
    hw_zoom_entry[i].roi_map_info.last_line = hw_zoom_entry[i].roi_map_info.first_line +
      ((fov_output[path_idx].last_line - fov_output[path_idx].first_line + 1) *
      scaler_output[path_idx].scaling_factor) - 1;

    ISP_DBG(ISP_MOD_COM,"%s: ROI[%d]: first pix %d, last pix %d, first ln %d, last line %d\n",
      __func__, i, hw_zoom_entry[i].roi_map_info.first_pixel,
      hw_zoom_entry[i].roi_map_info.last_pixel,
      hw_zoom_entry[i].roi_map_info.first_line,
      hw_zoom_entry[i].roi_map_info.last_line);
  }

  return rc;
}

/** isp_pipeline44_do_zoom
 *
 *    @pipeline_ptr:
 *    @crop_factor:
 *
 **/
static int isp_pipeline44_do_zoom(void *pipeline_ptr,
  isp_hw_set_crop_factor_t *crop_factor)
{
  int rc = 0;
  isp_pipeline_t *pipeline = (isp_pipeline_t *)pipeline_ptr;

  rc = pipeline->mod_ops[ISP_MOD_SCALER]->set_params(
         pipeline->mod_ops[ISP_MOD_SCALER]->ctrl, ISP_HW_MOD_SET_ZOOM_RATIO,
         (void *)&pipeline->pix_params.cfg_and_3a_params.cfg,
         sizeof(isp_hw_pix_setting_params_t));

  if (rc < 0) {
    CDBG_ERROR("%s: SET_ZOOM_RATIO error in scaler, rc = %d\n",
      __func__, rc);
    return rc;
  }

  rc = pipeline->mod_ops[ISP_MOD_FOV]->set_params(
         pipeline->mod_ops[ISP_MOD_FOV]->ctrl, ISP_HW_MOD_SET_ZOOM_RATIO,
         (void *)&pipeline->pix_params.cfg_and_3a_params.cfg,
         sizeof(isp_hw_pix_setting_params_t));

  if (rc < 0) {
    CDBG_ERROR("%s: SET_ZOOM_RATIO error in fov, rc = %d\n",
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

/** isp_pipeline44_read_dmi_tbl_kernel
 *
 *    @pipeline:
 *
 **/
static int isp_pipeline44_read_dmi_tbl_kernel(void *pix_ptr,
  isp_hw_read_info *dmi_read_info, void *dump_entry)
{
  int rc;
  int fd;
  isp_hw_dmi_dump_t dmi_cfg;
  uint32_t cmd_offset, cmd_len, dmi_channel;

  isp_pipeline_t *pipeline = (isp_pipeline_t *)pix_ptr;
  fd = pipeline->fd;
  dmi_channel = dmi_read_info->read_bank + dmi_read_info->bank_idx;

  /* 1. program DMI default value, write auto increment bit
     2. write DMI table
     3. reset DMI cfg */
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[5];

  cfg_cmd.cfg_data = (void *)&dmi_cfg;
  cfg_cmd.cmd_len = sizeof(dmi_cfg);
  cfg_cmd.cfg_cmd = (void *) &reg_cfg_cmd;
  cfg_cmd.num_cfg = 5;

  /* set dmi to proper hist stats bank */
  dmi_cfg.set_channel = ISP44_DMI_CFG_DEFAULT + dmi_channel;
  cmd_offset = 0;
  cmd_len = 1 * sizeof(uint32_t);
  isp_pipeline_util_pack_cfg_cmd(fd, &reg_cfg_cmd[0],
    cmd_offset, cmd_len,
    VFE_WRITE_MB, ISP44_DMI_CFG_OFF);

  /* set start addr = 0*/
  dmi_cfg.set_start_addr = 0;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  isp_pipeline_util_pack_cfg_cmd(fd, &reg_cfg_cmd[1],
    cmd_offset, cmd_len,
    VFE_WRITE_MB, ISP44_DMI_ADDR);

  /* memset dmi tbl = all 0,
     DMI read: according to read_type
     cmd_len= size of the void pointer
     table len : size of the dmi table size from read_len
     hi_tbl_offset = 0
     lo_tbl_offset = dmi_tbl offset_offset */
  memset(dmi_cfg.dmi_tbl, 0, dmi_read_info->read_lengh);
  cmd_offset += cmd_len;
  cmd_len = MAX_DMI_TBL_SIZE * sizeof(uint32_t);
  isp_pipeline_util_pack_dmi_cmd(fd, &reg_cfg_cmd[2], 0,
    cmd_offset, dmi_read_info->read_lengh, dmi_read_info->read_type);

  /* reset the sart addr = 0 */
  dmi_cfg.reset_start_addr = 0;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  isp_pipeline_util_pack_cfg_cmd(fd, &reg_cfg_cmd[3],
    cmd_offset, cmd_len,
    VFE_WRITE_MB, ISP44_DMI_ADDR);

  /* set dmi to proper hist stats bank */
  dmi_cfg.reset_channel =
    ISP44_DMI_CFG_DEFAULT + ISP44_DMI_NO_MEM_SELECTED;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  isp_pipeline_util_pack_cfg_cmd(fd, &reg_cfg_cmd[4],
    cmd_offset, cmd_len,
    VFE_WRITE_MB, ISP44_DMI_CFG_OFF);

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: VFE reset hist DMI error = %d\n", __func__, rc);
    return rc;
  }

  memcpy(dump_entry, &dmi_cfg.dmi_tbl[0], dmi_read_info->read_lengh);

  return rc;
}

/** isp_pipeline44_read_dmi_tbl_user
 *
 *    @pipeline:
 *
 **/
static int isp_pipeline44_read_dmi_tbl_user(isp_pipeline_t *pipeline,
  isp_hw_read_info *dmi_read_info, void *dump_entry)
{
  int rc = 0;
  uint32_t module_id;

  switch (dmi_read_info->read_bank) {
  case ROLLOFF_RAM0_BANK0:
  case ROLLOFF_RAM0_BANK1:
    module_id = ISP_MOD_ROLLOFF;
    break;
  case RGBLUT_RAM_CH0_BANK0:
  case RGBLUT_RAM_CH0_BANK1:
  case RGBLUT_RAM_CH1_BANK0:
  case RGBLUT_RAM_CH1_BANK1:
  case RGBLUT_RAM_CH2_BANK0:
  case RGBLUT_RAM_CH2_BANK1:
    module_id = ISP_MOD_GAMMA;
    break;
  case LA_LUT_RAM_BANK0:
  case LA_LUT_RAM_BANK1:
    module_id = ISP_MOD_LA;
    break;
  case BLACK_LUT_RAM_BANK0:
  case BLACK_LUT_RAM_BANK1:
    module_id = ISP_MOD_LINEARIZATION;
    break;
  default:
    ISP_DBG(ISP_MOD_COM,"%s: no supported dump type, read_channel = %x\n",
      __func__, dmi_read_info->read_bank);
    return rc;
  }

  rc = pipeline->mod_ops[module_id]->get_params(
    pipeline->mod_ops[module_id]->ctrl,
    ISP_HW_MOD_GET_DMI_DUMP_USER, dmi_read_info, sizeof(isp_hw_read_info),
    dump_entry, dmi_read_info->read_lengh);
  if (rc < 0) {
    CDBG_ERROR("%s: ISP_HW_MOD_GET_DMI_DUMP_USER error, rc = %d\n", __func__, rc);
    return rc;
  }

  return rc;
}

/** isp_pipeline44_read_dmi_tbl
 *
 *    @pipeline:
 *
 **/
static int isp_pipeline44_read_dmi_tbl(void *pipeline_ptr,
  isp_hw_read_info *dmi_read_info, void *dump_entry)
{
   int rc = 0;
   isp_pipeline_t *pipeline = (isp_pipeline_t *)pipeline_ptr;

   if (dmi_read_info->is_kernel_dump == 1) {
      ISP_DBG(ISP_MOD_COM,"%s: dump kernel DMI\n", __func__);
      rc = isp_pipeline44_read_dmi_tbl_kernel(pipeline,
        dmi_read_info, dump_entry);
   } else {
      ISP_DBG(ISP_MOD_COM,"%s: dump USERSPACE DMI\n", __func__);
      rc = isp_pipeline44_read_dmi_tbl_user(pipeline,
        dmi_read_info, dump_entry);
   }

  return rc;
}

/** isp_pipeline44_destroy
 *
 *    @dep:
 *
 **/
static void isp_pipeline44_destroy(isp_hw_pix_dep_t *dep)
{
  if (dep->op_ptr) {
    free(dep->op_ptr);
    dep->op_ptr = NULL;
  }
}

/** isp44_util_get_cds_trigger_pts:
 *    @pipeline_ptr: pointer to pipeline dependency data
 *    @enable
 * Free private resources.
 *
 * Return none.
 **/
static void isp44_util_get_params(void *in_params, uint32_t in_params_size,
  uint32_t param_id, void* out_param, uint32_t out_params_size, void *ctrl)
{
  ISP_DBG(ISP_MOD_COM,"%s:E, param_id %d", __func__, param_id);
  switch (param_id) {
  case ISP_PIPELINE_GET_CDS_TRIGGER_VAL: {
    modulesChromatix_t *chromatix_ptr = (modulesChromatix_t *)in_params;
    isp_uv_subsample_t *uv_subsample_ctrl = (isp_uv_subsample_t *)out_param;
    if (NULL != uv_subsample_ctrl) {
      /*
      chromatix_parms_type *chromatixPtr = NULL;
      chromatix_CDS_type *chromatix_CDS = NULL;
      if(chromatix_ptr)
       chromatixPtr = (chromatix_parms_type *)chromatix_ptr->chromatixPtr;
      if(chromatixPtr)
        chromatix_CDS = &chromatixPtr->chromatix_post_processing
        .chromatix_chroma_sub_sampling;
      if(!chromatix_CDS) {
        uv_subsample_ctrl->trigger_A = 0;
        uv_subsample_ctrl->trigger_B = 0;
      } else {
      */
        uv_subsample_ctrl->trigger_A = 300;
        /*chromatix_CDS->lux_index_trigger_value_A;*/
        uv_subsample_ctrl->trigger_B = 330;
        /*chromatix_CDS->lux_index_trigger_value_B;*/
      /*}*/
      ISP_DBG(ISP_MOD_COM,"%s: chromatix %x got trigger A= %ld, B= %ld ", __func__,
        (unsigned int)chromatix_ptr, uv_subsample_ctrl->trigger_A,
        uv_subsample_ctrl->trigger_B);
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

/** isp_pipeline44_init
 *
 *    @dep:
 *
 **/
int isp_pipeline44_init(isp_hw_pix_dep_t *dep)
{
  int i =0;

  dep->max_mod_mask_continuous_bayer = max_mod_mask_continuous_bayer;
  dep->max_mod_mask_burst_bayer = max_mod_mask_burst_bayer;
  dep->max_supported_stats = max_supported_stats;
  dep->max_mod_mask_continuous_yuv = max_mod_mask_continuous_yuv;
  dep->max_mod_mask_burst_yuv = max_mod_mask_burst_yuv;
  dep->num_mod_cfg_order_bayer= sizeof(mod_cfg_order_bayer) / sizeof(uint16_t);
  dep->mod_cfg_order_bayer = mod_cfg_order_bayer;
  dep->num_mod_cfg_order_yuv= sizeof(mod_cfg_order_yuv) / sizeof(uint16_t);
  dep->mod_cfg_order_yuv = mod_cfg_order_yuv;
  dep->num_mod_trigger_update_order_bayer = sizeof(mod_trigger_update_order_bayer) / sizeof(uint16_t);
  dep->mod_trigger_update_order_bayer = mod_trigger_update_order_bayer;

  dep->op_ptr =  malloc(sizeof(ISP_OperationConfigCmdType));
  if (dep->op_ptr == NULL) {
    CDBG_ERROR("%s: no mem\n", __func__);
    return -1;
  }

  memset(dep->op_ptr, 0, sizeof(ISP_OperationConfigCmdType));
  dep->destroy = isp_pipeline44_destroy;
  dep->operation_config = isp_pipeline44_operation_config;
  dep->module_start = isp_pipeline44_module_start;
  dep->do_zoom = isp_pipeline44_do_zoom;
  dep->read_dmi_tbl = isp_pipeline44_read_dmi_tbl;
  dep->get_roi_map = isp_pipeline44_get_roi_map;
  dep->util_get_aec_ratio_lowlight = isp44_util_get_aec_ratio_lowlight;
  dep->util_get_aec_ratio_bright_low = isp44_util_get_aec_ratio_bright_low;
  dep->util_get_aec_ratio_bright = isp44_util_get_aec_ratio_bright;
  dep->util_get_awb_cct_type = isp44_util_get_awb_cct_type;
  dep->util_set_uv_subsample = isp44_util_set_uv_subsample;
  dep->util_get_param = isp44_util_get_params;

  return 0;
}
