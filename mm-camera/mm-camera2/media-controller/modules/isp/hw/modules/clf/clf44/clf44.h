/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CLF44_H__
#define __CLF44_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "clf44_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "../../abf/common/abf_common.h"
#include "chromatix.h"

#define CLF_CF_COEFF(x) MIN(128, FLOAT_TO_Q(6, (x)))

/** ISP_CLF_Luma_Update_CmdType
 *    @Cfg: Luma filter configuration
 *    @pos_LUT: positive LUT
 *    @neg_LUT: negative LUT
 *
 * Luma Update Command stucture
 **/
typedef struct ISP_CLF_Luma_Update_CmdType {
  ISP_CLF_Luma_Cfg      Cfg;
  ISP_CLF_Luma_Lut      pos_LUT[8];
  ISP_CLF_Luma_Lut      neg_LUT[4];
}__attribute__((packed, aligned(4))) ISP_CLF_Luma_Update_CmdType;

/** ISP_CLF_Chroma_Update_CmdType
 *    @chroma_coeff: Chroma coefficients
 *
 * Chroma Update Command stucture
 **/
typedef struct ISP_CLF_Chroma_Update_CmdType {
  ISP_CLF_Chroma_Coeff  chroma_coeff;
}__attribute__((packed, aligned(4))) ISP_CLF_Chroma_Update_CmdType;

/** ISP_CLF_CmdType
 *    @clf_cfg: CLF general config
 *    @lumaUpdateCmd: Luma update command
 *    @chromaUpdateCmd: Chroma update command
 *
 * CLF Update Command stucture
 **/
typedef struct ISP_CLF_CmdType {
  ISP_CLF_Cfg                    clf_cfg;
  ISP_CLF_Luma_Update_CmdType    lumaUpdateCmd;
  ISP_CLF_Chroma_Update_CmdType  chromaUpdateCmd;
}__attribute__((packed, aligned(4))) ISP_CLF_CmdType;

/** isp_clf_enable_type_t
 *    @ISP_CLF_LUMA_CHROMA_DISABLE: CLF disabled
 *    @ISP_CLF_LUMA_ENABLE: CLF Luma only
 *    @ISP_CLF_CHROMA_ENABLE: CLF Chroma only
 *    @ISP_CLF_LUMA_CHROMA_ENABLE: CLF both Luma nad Chroma
 *
 * CLF filter enable enumeration
 **/
typedef enum {
  ISP_CLF_LUMA_CHROMA_DISABLE,
  ISP_CLF_LUMA_ENABLE,
  ISP_CLF_CHROMA_ENABLE,
  ISP_CLF_LUMA_CHROMA_ENABLE,
}isp_clf_enable_type_t;

/** clf_params_t
 *    @cf_param: Chroma filter parameters
 *    @lf_param: Adaptive bayer filter parameters
 *
 * CLF parameters structure
 **/
typedef struct {
  Chroma_filter_type cf_param;
  chromatix_adaptive_bayer_filter_data_type2 lf_param;
}clf_params_t;

/**
 *    @fd: isp subdevice file descriptor
 *    @ops: module ops
 *    @notify_ops: module notify ops
 *    @old_streaming_mode: previous streaming mode
 *    @reg_cmd: HW command
 *    @cur_cf_aec_ratio: current chroma filter AEC ratio
 *    @cur_lf_aec_ratio: current luma filter AEC ratio
 *    @clf_params: CLF module params
 *    @hw_update_pending: is hw ipdate pending
 *    @trigger_enable: enable trigger update feature flag from PIX
 *    @skip_trigger: should skip trigger update
 *    @enable: enable flag from PIX
 *    @cf_enable: enable chroma filtering
 *    @lf_enable: enable luma filtering
 *    @cf_update: chroma trigger update
 *    @lf_update: luma trigger update
 *    @cf_enable_trig: chroma trigger update allowed
 *    @lf_enable_trig: luma trigger update allowed
 *
 **/
typedef struct {
  /* ISP Related*/
  int fd;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  cam_streaming_mode_t old_streaming_mode;

  /* Module Params*/
  ISP_CLF_CmdType reg_cmd;
  float cur_cf_aec_ratio;
  trigger_ratio_t cur_lf_aec_ratio;
  clf_params_t clf_params;

  /* Module Control */
  uint8_t hw_update_pending;
  uint8_t trigger_enable;
  uint8_t skip_trigger;
  uint8_t enable;

  int8_t cf_enable;
  int8_t lf_enable;
  int8_t cf_update;
  int8_t lf_update;
  int8_t cf_enable_trig;
  int8_t lf_enable_trig;
} isp_clf_mod_t;

#endif //__CLF44_H__
