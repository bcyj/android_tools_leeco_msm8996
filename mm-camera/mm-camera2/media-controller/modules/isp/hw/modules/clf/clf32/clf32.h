/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CLF_H__
#define __CLF_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "clf32_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "../../abf/common/abf_common.h"
#include "chromatix.h"

#define CLF_CF_COEFF(x) MIN(128, FLOAT_TO_Q(6, (x)))


/* Luma Update Command  */
typedef struct ISP_CLF_Luma_Update_CmdType {
  ISP_CLF_Luma_Cfg      Cfg;
  ISP_CLF_Luma_Lut      pos_LUT[8];
  ISP_CLF_Luma_Lut      neg_LUT[4];
}__attribute__((packed, aligned(4))) ISP_CLF_Luma_Update_CmdType;

/* Chroma Update Command  */
typedef struct ISP_CLF_Chroma_Update_CmdType {
  ISP_CLF_Chroma_Coeff  chroma_coeff;
}__attribute__((packed, aligned(4))) ISP_CLF_Chroma_Update_CmdType;

/* CLF config Command  */
typedef struct ISP_CLF_CmdType {
  ISP_CLF_Cfg                    clf_cfg;
  ISP_CLF_Luma_Update_CmdType    lumaUpdateCmd;
  ISP_CLF_Chroma_Update_CmdType  chromaUpdateCmd;
}__attribute__((packed, aligned(4))) ISP_CLF_CmdType;

typedef enum {
  ISP_CLF_LUMA_CHROMA_DISABLE,
  ISP_CLF_LUMA_ENABLE,
  ISP_CLF_CHROMA_ENABLE,
  ISP_CLF_LUMA_CHROMA_ENABLE,
}isp_clf_enable_type_t;

typedef struct {
  Chroma_filter_type cf_param;
  chromatix_adaptive_bayer_filter_data_type2 lf_param;
}clf_params_t;

typedef struct {

  ISP_CLF_CmdType ISP_PrevCLF_Cmd;
  float cur_cf_ratio;
  clf_params_t clf_params;
  trigger_ratio_t cur_lf_trig_ratio;

  int trigger_enable;

}clf_mod_t;

typedef struct {
  /* ISP Related*/
  int fd; //handle for module in session
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  cam_streaming_mode_t old_streaming_mode;

  /* Module Params*/
  ISP_CLF_CmdType reg_cmd;
  float cur_cf_aec_ratio;
  trigger_ratio_t cur_lf_aec_ratio;
  clf_params_t clf_params;
  clf_params_t applied_clf_params;

  /* Module Control */
  uint8_t hw_update_pending;
  uint8_t trigger_enable; /* enable trigger update feature flag from PIX*/
  uint8_t skip_trigger;
  uint8_t enable;         /* enable flag from PIX */

  //old
  int8_t cf_enable;
  int8_t lf_enable;
  int8_t cf_update;
  int8_t lf_update;
  int8_t cf_enable_trig;
  int8_t lf_enable_trig;
} isp_clf_mod_t;

#endif //__CLF_H__
