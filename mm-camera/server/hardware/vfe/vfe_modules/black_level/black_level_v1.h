/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __BLACK_LEVEL_H__
#define __BLACK_LEVEL_H__

#include "chromatix.h"
#include "vfe_util_common.h"

typedef enum {
  VFE_MANUAL_BLACK_LEVEL_CORRECTION,
  VFE_AUTOMATIC_BLACK_LEVEL_CORRECTION,
  VFE_LAST_BLACK_LEVEL_MODE_ENUM = VFE_AUTOMATIC_BLACK_LEVEL_CORRECTION,
} VFE_BlackLevelModeType;

typedef struct VFE_BlackLevelConfigCmdType {
  /* Black Level Selection */
  VFE_BlackLevelModeType blackLevelMode:1;
  uint32_t /* reserved */ :31;
  /* Black Level Configuration, Part 1 */
  uint32_t lastPixel:13;
  uint32_t /* reserved */ :3;
  uint32_t firstPixel:13;
  uint32_t /* reserved */ :3;
  /* Black Level Configuration, Part 2 */
  uint32_t lastLine:13;
  uint32_t /* reserved */ :2;
  uint32_t firstLine:13;
  uint32_t log2PixelsInBlackRegion:4;
  /* Black Level Configuration, Part 3 */
  uint32_t oddColManualBlackCorrection:9;
  uint32_t /* reserved */ :7;
  uint32_t evenColManualBlackCorrection:9;
  uint32_t /* reserved */ :7;
} __attribute__ ((packed, aligned(4))) VFE_BlackLevelConfigCmdType;

typedef struct {
  VFE_BlackLevelConfigCmdType preview_cmd;
  VFE_BlackLevelConfigCmdType snapshot_cmd;
  chromatix_black_level_offset_type bl_info[2]; /* 0 - preview 1 - snap */
  int hw_enable;
  uint8_t trigger_enable;
  uint8_t update;
  uint8_t enable;
  uint8_t vfe_reconfig;
  float blk_inc_comp;
  float ratio;
  vfe_op_mode_t op_mode;
}black_level_mod_t;

vfe_status_t vfe_black_level_init(black_level_mod_t *mod,
  vfe_params_t *vfe_params);
vfe_status_t vfe_black_level_config(black_level_mod_t *mod,
  vfe_params_t *vfe_params);
vfe_status_t vfe_black_level_trigger_update(black_level_mod_t *mod,
  vfe_params_t *vfe_params);
vfe_status_t vfe_black_level_update(black_level_mod_t *mod,
  vfe_params_t *vfe_params);
vfe_status_t vfe_black_level_enable(black_level_mod_t* mod,
  vfe_params_t* vfe_params, int8_t enable, int8_t hw_write);
vfe_status_t vfe_black_level_trigger_enable(black_level_mod_t *mod,
  vfe_params_t *vfe_params, int enable);
#endif //__BLACK_LEVEL_H__
