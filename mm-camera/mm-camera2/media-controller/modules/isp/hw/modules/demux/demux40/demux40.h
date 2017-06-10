/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMUX40_H__
#define __DEMUX40_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "demux40_reg.h"
#include "chromatix.h"

#define DEMUX_GAIN(x) FLOAT_TO_Q(7 ,(x))

typedef enum {
  ISP_INPUT_FORMAT_FIRST_ROW_BAYER_RGRG_PATTERN,
  ISP_INPUT_FORMAT_FIRST_ROW_BAYER_GRGR_PATTERN,
  ISP_INPUT_FORMAT_FIRST_ROW_BAYER_BGBG_PATTERN,
  ISP_INPUT_FORMAT_FIRST_ROW_BAYER_GBGB_PATTERN,
  ISP_INPUT_FORMAT_FIRST_ROW_422_YCBYCR_PATTERN,
  ISP_INPUT_FORMAT_FIRST_ROW_422_YCRYCB_PATTERN,
  ISP_INPUT_FORMAT_FIRST_ROW_422_CBYCRY_PATTERN,
  ISP_INPUT_FORMAT_FIRST_ROW_422_CRYCBY_PATTERN,
  ISP_LAST_INPUT_FORMAT_FIRST_ROW_PATTERN_ENUM
    = ISP_INPUT_FORMAT_FIRST_ROW_422_CRYCBY_PATTERN
} ISP_InputFormatFirstRowPatternType;

typedef enum {
  ISP_DISABLE_CHROMA_COSITING_WITH_LUMA_SAMPLES,
  ISP_ENABLE_CHROMA_COSITING_WITH_LUMA_SAMPLES,
  ISP_LAST_ENABLE_CHROMA_COSITING_WITH_LUMA_SAMPLES_ENUM =
    ISP_ENABLE_CHROMA_COSITING_WITH_LUMA_SAMPLES,/* For count purposes */
} ISP_ChromaCositingForYCbCrInputEnableType;

typedef struct {
  int fd;
  ISP_DemuxConfigCmdType ISP_DemuxConfigCmd;
  ISP_DemuxConfigCmdType applied_cmd;
  ISP_DemuxGainCfgCmdType ISP_RImageGainConfigCmd; /* for 3D, not used now */
  chromatix_channel_balance_gains_type gain;
  chromatix_channel_balance_gains_type r_gain;
  float dig_gain;
  uint8_t is_3d;
  uint8_t enable;
  uint8_t hw_update_pending;
  int trigger_enable;
  float remaining_digital_gain;
  cam_streaming_mode_t old_streaming_mode;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
}isp_demux_mod_t;


#endif //__DEMUX40_H__
