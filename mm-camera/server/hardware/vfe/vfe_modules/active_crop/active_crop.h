
/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __ACTIVE_CROP_H__
#define __ACTIVE_CROP_H__
#include "vfe_util_common.h"

/* Active Region Config Command */
typedef struct VFE_ActiveRegionConfigCmdType {
	/* Active Region Config, Part 1 */
	unsigned int lastColorCompOfActiveRegion:13;
	unsigned int /* reserved */ :3;
	unsigned int firstColorCompOfActiveRegion:13;
	unsigned int /* reserved */ :3;
	/* Active Region Config, Part 2 */
	unsigned int lastLineOfActiveRegion:13;
	unsigned int /* reserved */ :3;
	unsigned int firstLineOfActiveRegion:13;
	unsigned int /* reserved */ :3;
}__attribute__((packed, aligned(4))) VFE_ActiveRegionConfigCmdType;

typedef struct {
  VFE_ActiveRegionConfigCmdType active_crop_cfg_cmd;
  int8_t enable;
}active_crop_mod_t;

vfe_status_t vfe_active_crop_init(active_crop_mod_t *mod, vfe_params_t* parms);
vfe_status_t vfe_active_crop_config(active_crop_mod_t *mod,
  vfe_params_t *parm);
vfe_status_t vfe_active_crop_enable(active_crop_mod_t *mod,
  vfe_params_t *p_obj, int8_t enable, int8_t hw_write);
vfe_status_t vfe_init_active_crop_info(active_crop_mod_t *mod,
  vfe_params_t* p_obj);

#endif// __ACTIVE_CROP_H__
