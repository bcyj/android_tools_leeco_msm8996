/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __LUMA_ADAPTATION44_H__
#define __LUMA_ADAPTATION44_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "isp_pipeline40.h"
#include "luma_adaptation44_reg.h"
#include "chromatix.h"

#define ISP_LA_TABLE_LENGTH           64

typedef struct
{
    float offset;  // default=3.3, range =0.0 to 8.0
    float low_beam;  // default=0.9, range =0.0 to 1.0
    float high_beam;  // default=0.1 , range =0.0 to 1.0
    float histogram_cap;  // default=5.0, range 2.0 to 8.0
    float cap_high;   // default=2.0, range=1.0 to 4.0
    float cap_low;   // default=0.75, range=0.0 to 2.0

    float cap_adjust;
    uint16_t CDF_50_thr;
} la_8k_type;


typedef struct ISP_LA_TblEntry {
  /* Luma adaptation table entries. */
  int16_t table[ISP_LA_TABLE_LENGTH];
} ISP_LA_TblEntry;

typedef struct ISP_LA_ConfigCmdType {
  /* LA Config */
  ISP_LABankSel CfgCmd;
  ISP_LA_TblEntry  TblEntry;
} ISP_LA_ConfigCmdType;

typedef struct {
  ISP_LA_ConfigCmdType la_cmd;
  int32_t LUT_Yratio[ISP_LA_TABLE_LENGTH]; /* LUT for LA*/
  int32_t solarize_la_tbl[ISP_LA_TABLE_LENGTH]; /* LUT for LA*/
  int32_t posterize_la_tbl[ISP_LA_TABLE_LENGTH]; /* LUT for LA*/
  // used for histogram calculation
  la_8k_type   la_config;
  int fd;
  uint8_t la_enable;
  uint8_t la_trigger_enable;
  uint8_t hw_update_pending;
  cam_streaming_mode_t old_streaming_mode;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  uint32_t isp_version;
  uint8_t la_curve[256];
  uint8_t la_curve_is_valid; /* TRUE(1) initialized; FALSE(0) not initialized */
  mct_bracket_ctrl_t bracketing_data;
}isp_la_mod_t;

#endif
