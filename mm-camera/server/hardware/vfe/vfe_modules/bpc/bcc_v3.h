/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __DEMOSAIC_BCC_H__
#define __DEMOSAIC_BCC_H__

#define VFE_DEMOSAICV3_0_OFF    0x00000298
#define VFE_DEMOSAICV3_DBCC_OFF 0x0000060C

/* Demosaic DBCC config command */
typedef struct VFE_DemosaicDBCC_CmdType {
  /* Demosaic Config */
  /* Reserved for dbpc enable  */
  uint32_t     /* reserved */                   : 1;
  /* Reserved for dbcc enable  */
  uint32_t     enable                           : 1;
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

  /* DBCC_CFG */
  uint32_t     fminThreshold                    : 8;
  uint32_t     fmaxThreshold                    : 8;
  uint32_t     /* reserved */                   : 16;

  /* DBCC_OFFSET_CFG_0 */
  uint32_t     rOffsetLo                        : 10;
  uint32_t     rOffsetHi                        : 10;
  uint32_t     grOffsetLo                       : 10;
  uint32_t     /* reserved */                   :  2;

  /* DBCCC_OFFSET_CFG_1 */
  uint32_t     gbOffsetLo                       :10;
  uint32_t     gbOffsetHi                       :10;
  uint32_t     grOffsetHi                       :10;
  uint32_t     /* reserved */                   : 2;

  /* DBCC_OFFSET_CFG_2 */
  uint32_t     bOffsetLo                        :10;
  uint32_t     bOffsetHi                        :10;
  uint32_t     /* reserved */                   :12;

}__attribute__((packed, aligned(4))) VFE_DemosaicDBCC_CmdType;

typedef struct {
  int bcc_enable;
  uint32_t bcc_trigger_update;
  /* Driven only by EzTune */
  uint32_t bcc_trigger_enable;
  /* Drivern only by EzTune */
  uint32_t bcc_reload_params;
  float bcc_trigger_ratio;
  VFE_DemosaicDBCC_CmdType bcc_prev_cmd;
  VFE_DemosaicDBCC_CmdType bcc_snap_cmd;
  int hw_enable_cmd;
  vfe_op_mode_t cur_mode;
  vfe_module_ops_t ops;
} bcc_mod_t;

/*===========================================================================
 *  BCC Interface APIs
 *==========================================================================*/
vfe_status_t vfe_bcc_ops_init(void *mod);
vfe_status_t vfe_bcc_ops_deinit(void *mod);
vfe_status_t vfe_demosaic_bcc_update(int mod_id, void *bcc_mod, void *params);
vfe_status_t vfe_demosaic_bcc_trigger_update(int mod_id, void *bcc_mod,
  void *params);
vfe_status_t vfe_demosaic_bcc_config(int mod_id, void *bcc_mod, void *params);
vfe_status_t vfe_demosaic_bcc_enable(int mod_id, void *bcc_mod,
  void *params, int8_t enable, int8_t hw_write);
vfe_status_t vfe_demosaic_bcc_deinit(int mod_id, void *bcc_mod, void *params);
vfe_status_t vfe_demosaic_bcc_init(int mod_id, void *bcc_mod, void *params);

/*===========================================================================
 *  BCC Interface APIs for EzTune
 *==========================================================================*/
vfe_status_t vfe_bcc_trigger_enable(int mod_id, void *bcc_mod,
  void *params, int enable);
vfe_status_t vfe_bcc_reload_params(int mod_id, void *bcc_mod, void *params);
vfe_status_t vfe_bcc_test_vector_validation(int mod_id, void *in, void *op);
vfe_status_t vfe_bcc_plugin_update(int module_id, void *mod,
  void *vparams);
#endif //__DEMOSAIC_BCC_H__
