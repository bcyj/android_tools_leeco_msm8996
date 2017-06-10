/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __DEMOSAIC_BPC_H__
#define __DEMOSAIC_BPC_H__

#include "vfe_util_common.h"

typedef struct VFE_abccLutType {
  unsigned int kernelIdx:3;
  unsigned int skip0:3;
  unsigned int skip1:3;
  unsigned int pixelIdx:23;
} __attribute__ ((packed, aligned(4))) VFE_abccLutType;

typedef struct VFE_DemosaicDBPC_CmdType {
  unsigned int dbpcEnable:1;
  unsigned int dbccenable:1;
  unsigned int abccEnable:1;
  unsigned int abccLutSel:1;
  unsigned int /*reserved */ :12;
  unsigned int dbpcFmin:8;
  unsigned int dbpcFmax:8;

  unsigned int RdbpcOffLo:10;
  unsigned int RdbpcOffHi:10;
  unsigned int GRdbpcOffLo:10;
  unsigned int /*reserved */ :2;

  unsigned int GBdbpcOffLo:10;
  unsigned int GBdbpcOffHi:10;
  unsigned int GRdbpcOffHi:10;
  unsigned int /*reserved */ :2;

  unsigned int BdbpcOffLo:10;
  unsigned int BdbpcOffHi:10;
  unsigned int /*reserved */ :12;


  unsigned int dbccFmin:8;
  unsigned int dbccFmax:8;
  unsigned int /*reserved */ :16;

  unsigned int RdbccOffLo:10;
  unsigned int RdbccOffHi:10;
  unsigned int GRdbccOffLo:10;
  unsigned int /*reserved */ :2;

  unsigned int GBdbccOffLo:10;
  unsigned int GBdbccOffHi:10;
  unsigned int GRdbccOffHi:10;
  unsigned int /*reserved */ :2;
  unsigned int BdbccOffLo:10;
  unsigned int BdbccOffHi:10;
  unsigned int /*reserved */ :12;
  VFE_abccLutType abccLUT[512];
} __attribute__ ((packed, aligned(4))) VFE_DemosaicDBPC_CmdType;

typedef struct VFE_DemosaicBPCUpdate_CmdType {
  unsigned int dbpcEnable:1;
  unsigned int dbccenable:1;
  unsigned int abccEnable:1;
  unsigned int abccLutSel:1;
  unsigned int /*reserved */ :12;
  unsigned int dbpcFmin:8;
  unsigned int dbpcFmax:8;

  unsigned int RdbpcOffLo:10;
  unsigned int RdbpcOffHi:10;
  unsigned int GRdbpcOffLo:10;
  unsigned int /*reserved */ :2;

  unsigned int GBdbpcOffLo:10;
  unsigned int GBdbpcOffHi:10;
  unsigned int GRdbpcOffHi:10;
  unsigned int /*reserved */ :2;

  unsigned int /*reserved */ :10;
  unsigned int BdbpcOffLo:10;
  unsigned int BdbpcOffHi:10;
  unsigned int /*reserved */ :2;

  unsigned int dbccFmin:8;
  unsigned int dbccFmax:8;
  unsigned int /*reserved */ :16;

  unsigned int RdbccOffLo:10;
  unsigned int RdbccOffHi:10;
  unsigned int GRdbccOffLo:10;
  unsigned int /*reserved */ :2;

  unsigned int GBdbccOffLo:10;
  unsigned int GBdbccOffHi:10;
  unsigned int GRdbccOffHi:10;
  unsigned int /*reserved */ :2;

  unsigned int BdbccOffLo:10;
  unsigned int BdbccOffHi:10;
  unsigned int /*reserved */ :12;
} __attribute__ ((packed, aligned(4))) VFE_DemosaicBPCUpdate_CmdType;

typedef struct {
  int enable;
  uint32_t trigger_update;
  /* Driven only by EzTune */
  uint32_t trigger_enable;
  /* Drivern only by EzTune */
  uint32_t reload_params;
  float bpc_trigger_ratio;
  float bcc_trigger_ratio;
  VFE_DemosaicDBPC_CmdType bpc_prev_cmd;
  VFE_DemosaicDBPC_CmdType bpc_snap_cmd;
  VFE_DemosaicBPCUpdate_CmdType bpc_update_cmd;

  int hw_enable_cmd;
} bpc_mod_t;

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
#endif //__DEMOSAIC_BPC_H__
