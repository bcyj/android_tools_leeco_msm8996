/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __FOVCROP_H__
#define __FOVCROP_H__

/* Field Of View (FOV) Crop Config Command */
typedef struct VFE_FOV_CropConfigCmdType {
#if (!defined(VFE_31) || !defined(VFE_2X))
  uint32_t  lastPixel        : 13;
  uint32_t  /* reserved */   :  3;
  uint32_t  firstPixel       : 13;
  uint32_t  /* reserved */   :  3;
#else
  uint32_t  lastPixel        : 12;
  uint32_t  /* reserved */   :  4;
  uint32_t  firstPixel       : 12;
  uint32_t  /* reserved */   :  4;
#endif
  /* FOV Corp, Part 2 */
  uint32_t  lastLine         : 12;
  uint32_t  /* reserved */   :  4;
  uint32_t  firstLine        : 12;
  uint32_t  /* reserved */   :  4;
}__attribute__((packed, aligned(4))) VFE_FOV_CropConfigCmdType;

typedef struct {
  VFE_FOV_CropConfigCmdType fov_cmd;
  vfe_module_ops_t ops;
  uint8_t fov_update;
  uint8_t fov_enable;
}fov_mod_t;

vfe_status_t vfe_fov_ops_init(void *mod);
vfe_status_t vfe_fov_ops_deinit(void *mod);
vfe_status_t vfe_fov_config(int mod_id, void *mod, void *params);
vfe_status_t vfe_crop_config(void *params);
vfe_status_t vfe_fov_update(int mod_id, void *fov_mod, void *params);
vfe_status_t vfe_fov_enable(int mod_id, void *fov_mod, void *params,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_fov_init(int mod_id, void *mod_fov, void* parm);
vfe_status_t vfe_fov_deinit(int mod_id, void *mod_fov, void* parm);
vfe_status_t vfe_fov_plugin_update(int module_id, void *mod,
  void *vparams);

#endif //__FOVCROP_H__
