/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __MCE_H__
#define __MCE_H__

#define V32_MCE_REG_OFF 0x000003EC

/******* Module:  MCE ********************************/
typedef struct VFE_mce_per_component_ConfigCmdType {
  /* config 0 */
  uint32_t         y1                      :   8;
  uint32_t         y2                      :   8;
  uint32_t         y3                      :   8;
  uint32_t         y4                      :   8;

  /*  config 1 */
  uint32_t         yM1                     :   7;
  uint32_t        /* reserved  */          :   1;
  uint32_t         yM3                     :   7;
  uint32_t         yS1                     :   4;
  uint32_t         yS3                     :   4;
  uint32_t         transWidth              :   5;
  uint32_t         transTrunc              :   4;

  /*  config 2 */
  int32_t          CRZone                  :   8;
  int32_t          CBZone                  :   8;
  int32_t          transSlope              :   5;
  int32_t          K                       :   9;
  int32_t         /* reserved  */          :   2;
}__attribute__((packed, aligned(4))) VFE_mce_per_component_ConfigCmdType;

typedef struct VFE_MCE_ConfigCmdType {
  /* Chroma Suppress 1 Config */
  /* reserved for chroma suppression.*/
  uint32_t     /* reserved  */        : 28;
  uint32_t     enable                 : 1;
  uint32_t     /* reserved  */        : 3;

  /* Chroma Suppress 2 Config */
  /* reserved for chroma suppression.*/
  uint32_t     /* reserved  */        : 28;
  uint32_t     qk                     : 4;

  VFE_mce_per_component_ConfigCmdType     redCfg;
  VFE_mce_per_component_ConfigCmdType     greenCfg;
  VFE_mce_per_component_ConfigCmdType     blueCfg;

}__attribute__((packed, aligned(4))) VFE_MCE_ConfigCmdType;

typedef struct{
  VFE_MCE_ConfigCmdType mce_cmd;
  int hw_enable;
  uint8_t mce_trigger;
  uint8_t mce_update;
  uint8_t mce_enable;
  float prev_lux_idx;
  vfe_op_mode_t prev_mode;
  vfe_module_ops_t ops;
}mce_mod_t;

vfe_status_t vfe_mce_ops_init(void *mod);
vfe_status_t vfe_mce_ops_deinit(void *mod);
vfe_status_t vfe_mce_init(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_mce_update(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_mce_trigger_update(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_mce_config(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_mce_enable(int module_id, void *mod, void *params,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_mce_reload_params(int module_id, void *mod, void* vfe_parms);
vfe_status_t vfe_mce_trigger_enable(int module_id, void *mod, void* vfe_parms,
  int enable);
vfe_status_t vfe_mce_tv_validate(int module_id, void* input, void* output);
vfe_status_t vfe_mce_deinit(int mod_id, void *module, void *params);
vfe_status_t vfe_mce_set_bestshot(int mod_id, void *module,
  void *vparams, camera_bestshot_mode_type mode);
vfe_status_t vfe_mce_plugin_update(int module_id, void *mod,
  void *vparams);

#endif //__MCE_H__
