/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __VFE_MODULES_H__
#define __VFE_MODULES_H__

#include "vfe_interface.h"
#include "vfe_util_common.h"
#include "vfe_tgtcommon.h"
#include "vfe_module_ops.h"
#include "vfe_interface.h"
#include "vfe_util_common.h"
#include "vfe_tgtcommon.h"
#include "vfe_module_ops.h"
#include "fovcrop.h"
#include "scaler.h"
#include "demosaic_v3.h"
#include "chroma_enhan.h"
#include "chroma_suppress.h"
#include "chroma_subsample.h"
#include "bcc_v3.h"
#include "bpc_v3.h"
#include "mce.h"
#include "demux.h"
#include "wb.h"
#include "abf.h"
#include "clf.h"
#include "colorcorrect.h"
#include "gamma.h"
#include "clamp.h"
#include "frame_skip.h"
#include "asf.h"
#include "rolloff.h"
#include "linearization_v1.h"
#include "sce.h"
#include "luma_adaptation.h"
#include "awb_stats.h"

typedef struct {
  VFE_FOV_CropConfigCmdType fov_cmd;
}fov_module_t;

typedef struct {
  VFE_Main_Scaler_ConfigCmdType main_scaler_cmd;
  VFE_Output_YScaleCfgCmdType y_scaler_cmd;
  VFE_Output_CbCrScaleCfgCmdType cbcr_scaler_cmd;
}scaler_module_t;

typedef struct {
  VFE_DemosaicV3ConfigCmdType demosaic_cmd;
  VFE_DemosaicV3UpdateCmdType demosaic_up_cmd;
}demosaic_module_t;

typedef struct {
  VFE_Chroma_Enhance_CfgCmdType chroma_enhan_cmd;
}chroma_enhan_module_t;

typedef struct {
  VFE_DemuxConfigCmdType demux_cfgCmd;
  VFE_DemuxGainCfgCmdType demux_GainConfigCmd;
}demux_module_t;

typedef struct {
  VFE_ChromaSuppress_ConfigCmdType chroma_supp_cmd;
}chroma_supp_module_t;

typedef struct {
  VFE_OutputClampConfigCmdType clamp_cfg_cmd;
}clamp_module_t;

typedef struct {
  VFE_DemosaicDBCC_CmdType bcc_cmd;
} bcc_module_t;

typedef struct {
  VFE_DemosaicDBPC_CmdType bpc_cmd;
} bpc_module_t;

typedef struct{
  VFE_MCE_ConfigCmdType mce_cmd;
}mce_module_t;

typedef struct  {
  VFE_WhiteBalanceConfigCmdType WB_cfgCmd;
} wb_module_t;

typedef struct {
  VFE_ColorCorrectionCfgCmdType cc_cfg_Cmd;
} color_correct_module_t;

typedef struct {
  VFE_DemosaicABF_CmdType  demosaicABFCfgCmd;
} abf_module_t;

typedef struct {
  VFE_CLF_CmdType clf_Cmd;
}clf_module_t;

typedef struct {
  VFE_GammaConfigCmdType GammaCfgCmd;
} gamma_module_t;

typedef struct {
 VFE_FrameSkipConfigCmdType frame_skip_cmd;
}frame_skip_module_t;

typedef struct {
 VFE_ChromaSubsampleConfigCmdType Chroma_ss_cmd;
}chroma_ss_module_t;

typedef struct {
  VFE_AdaptiveFilterConfigCmdType asf_prev_cmd;
}asf_module_t;

typedef struct PCA_RollOffRamTable_pl {
  uint64_t basisTable[PCA_ROLLOFF_BASIS_TABLE_SIZE];
  uint64_t coeffTable[PCA_ROLLOFF_COEFF_TABLE_SIZE];
}PCA_RollOffRamTable_pl;

typedef struct PCA_RollOffConfigTable_pl {
  PCA_RollOffRamTable_pl ram0;
  PCA_RollOffRamTable_pl ram1;
}PCA_RollOffConfigTable_pl;

typedef struct PCA_RollOffConfigCmdType_pl {
  uint32_t tableOffset;
  PCA_RollOffConfigParams CfgParams;
  PCA_RollOffConfigTable_pl Table;
}rolloff_module_t;

typedef struct {
  VFE_LinearizationCmdType  linear_cmd;
}linear_module_t;

typedef struct {
  VFE_Skin_enhan_ConfigCmdType sce_cmd;
}sce_module_t;

typedef struct {
  VFE_LA_ConfigCmdType la_cmd;
}la_module_t;

typedef struct {
  VFE_StatsAwb_CfgCmdType VFE_StatsAwb_ConfigCmd;
}awb_stats_module_t;

typedef struct {
  fov_module_t fov_mod;
  scaler_module_t scaler_mod;
  demosaic_module_t demosaic_mod;
  chroma_enhan_module_t chroma_enhan_mod;
  demux_module_t demux_mod;
  chroma_supp_module_t chroma_supp_mod;
  clamp_module_t clamp_mod;
  bcc_module_t bcc_mod;
  bpc_module_t bpc_mod;
  mce_module_t mce_mod;
  wb_module_t wb_mod;
  color_correct_module_t color_correct_mod;
  abf_module_t abf_mod;
  clf_module_t clf_mod;
  gamma_module_t gamma_mod;
  frame_skip_module_t frame_skip_mod;
  chroma_ss_module_t chroma_ss_mod;
  asf_module_t asf_mod;
  rolloff_module_t rolloff_mod;
  linear_module_t linear_mod;
  sce_module_t sce_mod;
  la_module_t la_mod;
  awb_stats_module_t awb_stats;
} vfe_modules_t;

#define MOD_LINEARIZATION    (0)
#define MOD_ROLLOFF          (1)
#define MOD_DEMUX            (2)
#define MOD_DEMOSAIC         (3)
#define MOD_BPC              (4)
#define MOD_ABF              (5)
#define MOD_ASF              (6)
#define MOD_COLOR_CONV       (7)
#define MOD_COLOR_CORRECT    (8)
#define MOD_CHROMA_SS        (9)
#define MOD_CHROMA_SUPPRESS  (10)
#define MOD_LA               (11)
#define MOD_MCE              (12)
#define MOD_SCE              (13)
#define MOD_CLF              (14)
#define MOD_WB               (15)
#define MOD_GAMMA            (16)
#define MOD_AWB_STATS        (17)
#define MOD_FOV              (20)
#define MOD_SCALER           (21)
#define MOD_BCC              (22)
#define MOD_CLAMP            (23)
#define MOD_FRAME_SKIP       (24)

#endif
