/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __DEMOSAIC_BPC_H__
#define __DEMOSAIC_BPC_H__

#include "chromatix.h"
#include "vfe_util_common.h"

/* Demosaic BPC Update Command */
typedef struct VFE_DemosaicBPC_CmdType {
  /* Demosaic Config */
  uint32_t     enable                           : 1;
  /* Reserved for abf enable  */
  uint32_t     /* reserved */                   : 1;
  /* Reserved for abfForceOn enable  */
  uint32_t     /* reserved */                   : 1;
  uint32_t     /* reserved */                   : 9;
  uint32_t     fminThreshold                    : 6;
  uint32_t     /*  reserved */                  : 2;
  uint32_t     fmaxThreshold                    : 7;
  uint32_t     /* reserved */                   : 1;
  /* Reserved for slopeShift  */
  uint32_t     /* reserved */                   : 3;
  uint32_t     /* reserved */                   : 1;

  /* Demosaic BPC Config 0 */
  uint32_t      bOffsetHi                       :10;
  uint32_t      gOffsetLo                       :10;
  uint32_t      gOffsetHi                       :10;
  uint32_t     /* reserved */                   : 2;
  /* Demosaic BPC Config 1 */
  uint32_t      rOffsetLo                       :10;
  uint32_t      rOffsetHi                       :10;
  uint32_t      bOffsetLo                       :10;
  uint32_t     /* reserved */                   : 2;
}__attribute__((packed, aligned(4))) VFE_DemosaicBPC_CmdType;

typedef struct {
  int bpc_enable;
  uint32_t bpc_trigger_update;
  /* Driven only by EzTune */
  uint32_t bpc_trigger_enable;
  /* Drivern only by EzTune */
  uint32_t bpc_reload_params;
  float bpc_trigger_ratio;
  VFE_DemosaicBPC_CmdType bpc_prev_cmd;
  VFE_DemosaicBPC_CmdType bpc_snap_cmd;
  vfe_op_mode_t cur_mode;
  int hw_enable_cmd;
} bpc_mod_t;


vfe_status_t vfe_demosaic_bpc_update(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params);
vfe_status_t vfe_demosaic_bpc_trigger_update(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params);
vfe_status_t vfe_demosaic_bpc_config(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params);
vfe_status_t vfe_demosaic_bpc_enable(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params, int8_t enable, int8_t hw_write);
vfe_status_t vfe_demosaic_bpc_init(bpc_mod_t *bpc_ctrl,
  vfe_params_t *vfe_params);
vfe_status_t vfe_bpc_trigger_enable(bpc_mod_t* bpc_ctrl,
  vfe_params_t* vfe_params, int8_t enable);
vfe_status_t vfe_bpc_reload_params(bpc_mod_t* bpc_ctrl,
  vfe_params_t* vfe_params);

#endif //__DEMOSAIC_H__
