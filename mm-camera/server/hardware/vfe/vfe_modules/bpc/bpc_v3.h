/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __DEMOSAIC_BPC_H__
#define __DEMOSAIC_BPC_H__

#define VFE_DEMOSAICV3_0_OFF    0x00000298
#define VFE_DEMOSAICV3_DBPC_CFG_OFF  0x0000029C
#define VFE_DEMOSAICV3_DBPC_CFG_OFF0 0x000002a0
#define VFE_DEMOSAICV3_DBPC_CFG_OFF1 0x00000604
#define VFE_DEMOSAICV3_DBPC_CFG_OFF2 0x00000608

/* Demosaic DBPC Update Command */
typedef struct VFE_DemosaicDBPC_CmdType {
  /* Demosaic Config */
  /* Reserved for dbpc enable  */
  uint32_t     enable                           : 1;
  /* Reserved for dbcc enable  */
  uint32_t     /* reserved */                   : 1;
  /* Reserved for abcc enable */
  uint32_t     /* reserved */                   : 1;
  /* Reserved for abf enable  */
  uint32_t     /* reserved */                   : 1;
  /* Reserved for cosited rgb enable */
  uint32_t     /* reserved */                   : 1;
  uint32_t     /* reserved */                   : 3;

  /* reserved for abcc lut bank selection   */
  uint32_t      /* reserved */                  : 1;
  uint32_t     /* reserved */                   : 7;

  /* reserved for  pipe flush count   */
  uint32_t     /* reserved */                   : 13;
  uint32_t     /* reserved */                   : 1;
  /* reserved for pipe flush ovd */
  uint32_t     /* reserved */                   : 1;
  /* reserved for flush halt ovd */
  uint32_t     /* reserved */                   : 1;
  /* DBPC_CFG */
  uint32_t     fminThreshold                    : 8;
  uint32_t     fmaxThreshold                    : 8;
  uint32_t     /* reserved */                   : 16;

  /* DBPC_OFFSET_CFG_0 */
  uint32_t     rOffsetLo                        : 10;
  uint32_t     rOffsetHi                        : 10;
  uint32_t     grOffsetLo                       : 10;
  uint32_t     /* reserved */                   :  2;

  /* DBPC_OFFSET_CFG_1 */
  uint32_t     gbOffsetLo                       :10;
  uint32_t     gbOffsetHi                       :10;
  uint32_t     grOffsetHi                       :10;
  uint32_t     /* reserved */                   : 2;

  /* DBPC_OFFSET_CFG_2 */
  uint32_t     bOffsetLo                        :10;
  uint32_t     bOffsetHi                        :10;
  uint32_t     /* reserved */                   :12;
}__attribute__((packed, aligned(4))) VFE_DemosaicDBPC_CmdType;

typedef struct {
  int bpc_enable;
  uint32_t bpc_trigger_update;
  /* Driven only by EzTune */
  uint32_t bpc_trigger_enable;
  /* Drivern only by EzTune */
  uint32_t bpc_reload_params;
  float bpc_trigger_ratio;
  VFE_DemosaicDBPC_CmdType bpc_prev_cmd;
  VFE_DemosaicDBPC_CmdType bpc_snap_cmd;
  int hw_enable_cmd;
  vfe_op_mode_t cur_mode;
  vfe_module_ops_t ops;
} bpc_mod_t;

/*===========================================================================
 *  BPC Interface APIs
 *==========================================================================*/
vfe_status_t vfe_bpc_v3_ops_init(void *mod);
vfe_status_t vfe_bpc_v3_ops_deinit(void *mod);
vfe_status_t vfe_demosaic_bpc_update(int mod_id, void *mod, void *params);
vfe_status_t vfe_demosaic_bpc_trigger_update(int mod_id, void *mod,
  void *params);
vfe_status_t vfe_demosaic_bpc_config(int mod_id, void *bpc_mod, void *params);
vfe_status_t vfe_demosaic_bpc_enable(int mod_id, void *mod, void *params,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_demosaic_bpc_deinit(int mod_id, void *mod, void *params);
vfe_status_t vfe_demosaic_bpc_init(int mod_id, void *mod, void *params);

/*===========================================================================
 *  BPC Interface APIs for EzTune
 *==========================================================================*/
vfe_status_t vfe_bpc_trigger_enable(int mod_id, void* bpc_mod,
  void *params, int enable);
vfe_status_t vfe_bpc_reload_params(int mod_id, void *mod, void *params);
vfe_status_t vfe_bpc_test_vector_validation(int mod_id, void *in, void *op);
vfe_status_t vfe_bpc_plugin_update(int module_id, void *mod,
  void *vparams);

#if 0
/*===========================================================================
 *  BPC Interface APIs
 *==========================================================================*/
vfe_status_t vfe_demosaic_bpc_update(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params);
vfe_status_t vfe_demosaic_bpc_trigger_update(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params);
vfe_status_t vfe_demosaic_bpc_config(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params);
vfe_status_t vfe_demosaic_bpc_enable(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params, int8_t enable, int8_t hw_write);
vfe_status_t vfe_demosaic_bpc_deinit(bpc_mod_t* bpc_ctrl,
  vfe_params_t* vfe_params);
vfe_status_t vfe_demosaic_bpc_init(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params);

/*===========================================================================
 *  BPC Interface APIs for EzTune
 *==========================================================================*/
vfe_status_t vfe_bpc_trigger_enable(bpc_mod_t* bpc_ctrl,
  vfe_params_t* vfe_params, int8_t enable);
vfe_status_t vfe_bpc_reload_params(bpc_mod_t* bpc_ctrl,
  vfe_params_t* vfe_params);
vfe_status_t vfe_bpc_test_vector_validation(
  vfe_test_module_input_t *mod_in,
  vfe_test_module_output_t *mod_op);
#endif
#endif //__DEMOSAIC_BPC_H__
