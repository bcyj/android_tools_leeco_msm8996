/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __DEMOSAIC_H__
#define __DEMOSAIC_H__

#include "chromatix.h"
#include "vfe_util_common.h"

typedef struct VFE_DemosaicConfigCmdType {
  /* Demosaic Config */
  /* Reserved for bpc enable  */
  uint32_t     /* reserved */                   : 1;
  /* Reserved for abf enable  */
  uint32_t     /* reserved */                   : 1;
  /* Reserved for abfForceOn enable  */
  uint32_t     /* reserved */                   : 1;
  uint32_t     /* reserved */                   : 9;

  /* reserved for bpc_min   */
  uint32_t      /* reserved */                  : 6;
  uint32_t     /* reserved */                   : 2;

  /* reserved for  bpc_max   */
  uint32_t     /* reserved */                   : 7;
  uint32_t     /* reserved */                   : 1;

  uint32_t      slopeShift                      : 3;
  uint32_t     /* reserved */                   : 1;
}__attribute__((packed, aligned(4))) VFE_DemosaicConfigCmdType;

typedef struct {
  VFE_DemosaicConfigCmdType cmd;
  int8_t enable;
  int reload_params;
  int hw_enable;
}demosaic_mod_t;

vfe_status_t vfe_demosaic_init(demosaic_mod_t *mod, vfe_params_t *params);
vfe_status_t vfe_demosaic_enable(demosaic_mod_t *mod, vfe_params_t *p_obj,
  int8_t enable, int8_t hw_write);
vfe_status_t vfe_demosaic_config(demosaic_mod_t *mod, vfe_params_t *p_obj);
vfe_status_t vfe_demosaic_update(demosaic_mod_t *mod, vfe_params_t *params);
vfe_status_t vfe_demosaic_trigger_update(demosaic_mod_t *mod,
  vfe_params_t *parm);
vfe_status_t vfe_demosaic_trigger_enable(demosaic_mod_t* mod,
  vfe_params_t* params, int enable);
vfe_status_t vfe_demosaic_reload_params(demosaic_mod_t* mod,
  vfe_params_t* params);
#endif //__DEMOSAIC_H__
