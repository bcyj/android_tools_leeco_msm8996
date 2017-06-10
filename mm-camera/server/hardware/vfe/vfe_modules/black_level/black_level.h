/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __BLACK_LEVEL_H__
#define __BLACK_LEVEL_H__

#include "chromatix.h"
#include "vfe_util_common.h"

typedef struct VFE_BlackLevelConfigCmdType {
  /* Black Even-Even Value Config */
  uint32_t      evenEvenAdjustment          : 9;
  uint32_t     /* reserved */               :23;
  /* Black Even-Odd Value Config */
  uint32_t      evenOddAdjustment           : 9;
  uint32_t     /* reserved */               :23;
  /* Black Odd-Even Value Config */
  uint32_t      oddEvenAdjustment           : 9;
  uint32_t     /* reserved */               :23;
  /* Black Odd-Odd Value Config */
  uint32_t      oddOddAdjustment            : 9;
  uint32_t     /* reserved */               :23;

}__attribute__((packed, aligned(4))) VFE_BlackLevelConfigCmdType;

typedef struct {
  VFE_BlackLevelConfigCmdType preview_cmd;
  VFE_BlackLevelConfigCmdType snapshot_cmd;
  chromatix_4_channel_black_level bl_info[2]; /* 0 - preview 1 - snap */
  int hw_enable;
  uint8_t trigger_enable;
  uint8_t update;
  uint8_t enable;
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
