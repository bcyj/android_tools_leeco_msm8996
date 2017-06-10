/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __GAMMA32_H__
#define __GAMMA32_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "isp_pipeline32.h"
#include "gamma32_reg.h"
#include "isp_pipeline_util.h"

#define ISP_GAMMA_NUM_ENTRIES         64
#define GAMMA_TABLE_CHROMATICS_SIZE   1024
#define GAMMA_REFLEX_POINT 128

typedef enum {
  ISP_GAMMA_LINEAR_MAPPING_OF_10_BIT_GAMMA_TABLE,
  ISP_GAMMA_PIECEWISE_LINEAR_MAPPING_OF_10_BIT_GAMMA_TABLE,
  ISP_LAST_GAMMA_MAPPING_MODE_ENUM
    = ISP_GAMMA_PIECEWISE_LINEAR_MAPPING_OF_10_BIT_GAMMA_TABLE
} ISP_GammaMappingModeType;

typedef struct VFE_GammaTable {
  int16_t table[ISP_GAMMA_NUM_ENTRIES];
} ISP_GammaTable;

typedef struct ISP_GammaConfigCmdType {
  ISP_GammaLutSelect LutSel;
  ISP_GammaTable Gamatbl;
} ISP_GammaConfigCmdType;

typedef enum {
  GAMMA_TABLE_DEFAULT = 0,
  GAMMA_TABLE_OUTDOOR,
  GAMMA_TABLE_LOWLIGHT,
  GAMMA_TABLE_BACKLIGHT,
  GAMMA_TABLE_SOLARIZE,
  GAMMA_TABLE_POSTERIZE,
  GAMMA_TABLE_WHITE_BOARD,
  GAMMA_TABLE_BLACK_BOARD,
  GAMMA_TABLE_INVALID
} isp_gamma_table_t;

typedef struct {
  int fd;
  int8_t enable;
  int8_t trigger_enable;
  int8_t trigger_update;
  ISP_GammaConfigCmdType ISP_GammaCfgCmd;
  uint8_t gamma_table[GAMMA_TABLE_SIZE];
  uint8_t* p_gamma_table;
  uint8_t p_gamma_table_size_bits;
  uint8_t solarize_gamma_table[GAMMA_TABLE_SIZE];
  trigger_ratio_t gamma_ratio;
  int32_t contrast;
  isp_gamma_table_t gamma_table_type;
  uint32_t backlight_severity;
  int enable_backlight_compensation;
  int reload_params;
  uint32_t gamma_retrigger;
  uint8_t hw_update_pending;
  cam_streaming_mode_t old_streaming_mode;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
} isp_gamma_mod_t;

#endif //__GAMMA32_H__
