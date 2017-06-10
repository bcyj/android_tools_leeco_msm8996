/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __MESH_ROLLOFF_V4_H__
#define __MESH_ROLLOFF_V4_H__

#include "vfe_util_common.h"

#ifdef VFE_31
  #define Y_DELTA_Q_LEN 12
#else
  #define Y_DELTA_Q_LEN 13
#endif

#define MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS  12
#define MESH_ROLL_OFF_V4_VERTICAL_GRIDS    9
#define MESH_ROLL_OFF_V4_TABLE_SIZE  (13 * 10)
#define CHROMATIX_MESH_TABLE_SIZE  (17 * 13)

/* Start: Data structures to hold re-arranged tables before writing them
 *        into HW specific format */
typedef struct MESH_RollOffTable_V4 {
  /* Table for red.  */
  uint16_t TableR[MESH_ROLL_OFF_V4_TABLE_SIZE];
  /* Table for Gr.   */
  uint16_t TableGr[MESH_ROLL_OFF_V4_TABLE_SIZE];
  /* Table for blue. */
  uint16_t TableB[MESH_ROLL_OFF_V4_TABLE_SIZE];
  /* Table for Gb.   */
  uint16_t TableGb[MESH_ROLL_OFF_V4_TABLE_SIZE];
}MESH_RollOffTable_V4;
/* End: Data structures to hold re-arranged tables before writing them
 *      into HW specific format */

/* Start: Data structures to hold HW specific table formats and params */
typedef struct MESH_RollOff_V4_ConfigParams {
 /* VFE_ROLLOFF_CONFIG */
  uint32_t                     pixelOffset             : 9;
  uint32_t                     /* reserved */          : 7;
  uint32_t                     pcaLutBankSel           : 1;
  uint32_t                     /* reserved */          : 15;
   /* VFE_ROLLOFF_GRID_CFG_0 */
  uint32_t                     blockWidth              : 9;
  uint32_t                     blockHeight             : 9;
  uint32_t                     /* reserved */          : 14;
  /* VFE_ROLLOFF_GRID_CFG_1 */
  uint32_t                     subGridXDelta           : 17;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     subGridYDelta           : 10;
  uint32_t                     interpFactor            : 2;
    /* VFE_ROLLOFF_GRID_CFG_2 */
  uint32_t                     subGridWidth            : 9;
  uint32_t                     subGridHeight           : 9;
  uint32_t                     /* reserved */          : 14;
  /* VFE_ROLLOFF_RIGHT_GRID_CFG_0 */
  uint32_t                     blockWidthRight         : 9;
  uint32_t                     blockHeightRight        : 9;
  uint32_t                     /* reserved */          : 14;
  /* VFE_ROLLOFF_RIGHT_GRID_CFG_1 */
  uint32_t                     subGridXDeltaRight      : 17;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     subGridYDeltaRight      : 10;
  uint32_t                     interpFactorRight       : 2;
  /* VFE_ROLLOFF_RIGHT_GRID_CFG_2 */
  uint32_t                      subGridWidthRight      : 9;
  uint32_t                      subGridHeightRight     : 9;
  uint32_t                      /* reserved */         : 14;
  /* VFE_ROLLOFF_STRIPE_CFG_0 */
  uint32_t                     blockXIndex             : 4;
  uint32_t                     blockYIndex             : 4;
  uint32_t                     PixelXIndex             : 9;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     PixelYIndex             : 9;
  uint32_t                     /* reserved */          : 3;
  /* VFE_ROLLOFF_STRIPE_CFG_1 */
  uint32_t                     yDeltaAccum             : 13;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     subGridXIndex           : 3;
  uint32_t                     /* reserved */          : 5;
  uint32_t                     subGridYIndex           : 3;
  uint32_t                     /* reserved */          : 5;
}__attribute__((packed, aligned(4))) MESH_RollOff_V4_ConfigParams;

typedef struct MESH_RollOff_V4_ConfigTable {
  uint32_t Table[MESH_ROLL_OFF_V4_TABLE_SIZE*2];
}MESH_RollOff_V4_ConfigTable;

typedef struct MESH_RollOff_V4_ConfigCmdType {
  MESH_RollOff_V4_ConfigParams CfgParams;
  MESH_RollOff_V4_ConfigTable Table;
}MESH_RollOff_V4_ConfigCmdType;
/* End: Data structures to hold HW specific table formats and params */

typedef struct {
  mesh_rolloff_array_type input_table;
}mesh_rolloff_V4_params_t;

typedef struct {
  uint32_t mesh_rolloff_enable;
  uint32_t mesh_rolloff_update;
  /* Driven only by EzTune */
  uint32_t mesh_rolloff_trigger_enable;
  /* Drivern only by EzTune */
  uint32_t mesh_rolloff_reload_params;
  vfe_op_mode_t cur_vfe_mode;
  trigger_ratio_t mesh_rolloff_trigger_ratio;
  MESH_RollOff_V4_ConfigCmdType mesh_rolloff_prev_cmd;
  MESH_RollOff_V4_ConfigCmdType mesh_rolloff_snap_cmd;
  mesh_rolloff_V4_params_t mesh_rolloff_prev_param;
  mesh_rolloff_V4_params_t mesh_rolloff_snap_param;
}mesh_rolloff_V4_mod_t;

vfe_status_t mesh_rolloff_V4_init(mesh_rolloff_V4_mod_t* mesh_rolloff_ctrl,
  vfe_rolloff_info_t *mesh_tbls, vfe_params_t* vfe_params);
vfe_status_t mesh_rolloff_V4_config(mesh_rolloff_V4_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params);
vfe_status_t mesh_rolloff_V4_update(mesh_rolloff_V4_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params);
vfe_status_t mesh_rolloff_V4_trigger_update(mesh_rolloff_V4_mod_t* mesh_rolloff_ctrl,
  vfe_rolloff_info_t *mesh_tbls, vfe_params_t* vfe_params);
vfe_status_t mesh_rolloff_V4_trigger_enable(mesh_rolloff_V4_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params, int8_t enable);
vfe_status_t mesh_rolloff_V4_reload_params(mesh_rolloff_V4_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params);
#endif /* __MESH_ROLLOFF_V4_H__ */
