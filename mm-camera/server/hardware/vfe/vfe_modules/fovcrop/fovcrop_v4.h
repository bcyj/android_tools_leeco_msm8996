/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __FOVCROP_V4_H__
#define __FOVCROP_V4_H__

/* Crop Config */
typedef struct VFE_FOV_CropConfig {
  uint32_t  lastPixel        : 13;
  uint32_t  /* reserved */   :  3;
  uint32_t  firstPixel       : 13;
  uint32_t  /* reserved */   :  3;

  uint32_t  lastLine         : 12;
  uint32_t  /* reserved */   :  4;
  uint32_t  firstLine        : 12;
  uint32_t  /* reserved */   :  4;
}__attribute__((packed, aligned(4))) VFE_FOV_CropConfig;

/* Field Of View (FOV) Crop Config Command */
typedef struct VFE_FOV_CropConfigCmdType {
  /* Y config */
  VFE_FOV_CropConfig  y_crop_cfg;
  /* CbCr Config */
  VFE_FOV_CropConfig  cbcr_crop_cfg;
}__attribute__((packed, aligned(4))) VFE_FOV_CropConfigCmdType;

typedef struct {
  VFE_FOV_CropConfigCmdType fov_enc_cmd;
  VFE_FOV_CropConfigCmdType fov_view_cmd;
  vfe_module_ops_t ops;
  uint8_t fov_update;
  uint8_t fov_enc_enable;
  uint8_t fov_view_enable;
}fov_mod_t;

typedef enum {
  ENCODER,
  VIEWFINER,
}crop_type;

vfe_status_t vfe_fov_ops_init(void *mod);
vfe_status_t vfe_fov_ops_deinit(void *mod);
vfe_status_t vfe_fov_config(int mod_id, void *mod, void *params);
vfe_status_t vfe_crop_config(void *params);
vfe_status_t vfe_crop_enc_config(void *params);
vfe_status_t vfe_crop_view_config(void *params);
vfe_status_t vfe_fov_update(int mod_id, void *fov_mod, void *params);
vfe_status_t vfe_fov_enable(int mod_id, void *fov_mod, void *params,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_fov_enc_enable(int mod_id, void *fov_mod, void *params,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_fov_view_enable(int mod_id, void *fov_mod, void *params,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_fov_init(int mod_id, void *mod_fov, void* parm);
vfe_status_t vfe_fov_deinit(int mod_id, void *mod_fov, void* parm);

#endif //__FOVCROP_H__
