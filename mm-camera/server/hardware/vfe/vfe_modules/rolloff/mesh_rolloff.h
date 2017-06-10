/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __MESH_ROLLOFF_H__
#define __MESH_ROLLOFF_H__

#include "vfe_util_common.h"

#ifdef VFE_31
  #define Y_DELTA_Q_LEN 12
#else
  #define Y_DELTA_Q_LEN 13
#endif

#define MESH_ROLL_OFF_HORIZONTAL_GRIDS  16
#define MESH_ROLL_OFF_VERTICAL_GRIDS    12
#define MESH_ROLL_OFF_INIT_TABLE_SIZE   13   /* 13x1  */
#define MESH_ROLL_OFF_DELTA_TABLE_SIZE  208  /* 13x16 */

/* Start: Data structures to hold re-arranged tables before writing them
 *        into HW specific format */
typedef struct MESH_RollOffTable {
  /* Init table for red.  */
  uint16_t initTableR[MESH_ROLL_OFF_INIT_TABLE_SIZE];
  /* Init table for Gr.   */
  uint16_t initTableGr[MESH_ROLL_OFF_INIT_TABLE_SIZE];
  /* Init table for blue. */
  uint16_t initTableB[MESH_ROLL_OFF_INIT_TABLE_SIZE];
  /* Init table for Gb.   */
  uint16_t initTableGb[MESH_ROLL_OFF_INIT_TABLE_SIZE];
  /* Delta table for red.  */
  int16_t deltaTableR[MESH_ROLL_OFF_DELTA_TABLE_SIZE];
  /* Delta table for Gr.   */
  int16_t deltaTableGr[MESH_ROLL_OFF_DELTA_TABLE_SIZE];
  /* Delta table for blue. */
  int16_t deltaTableB[MESH_ROLL_OFF_DELTA_TABLE_SIZE];
  /* Delta table for Gb.   */
  int16_t deltaTableGb[MESH_ROLL_OFF_DELTA_TABLE_SIZE];
}MESH_RollOffTable;
/* End: Data structures to hold re-arranged tables before writing them
 *      into HW specific format */

/* Start: Data structures to hold HW specific table formats and params */
typedef struct MESH_RollOffConfigParams {
  /* Rolloff 0 Config */
  uint32_t                      gridWidth               : 9;
  uint32_t                      gridHeight              : 9;
#ifdef VFE_31
  uint32_t                      yDelta                  : 9;
  uint32_t                     /* reserved */           : 5;
#elif VFE_2X
  uint32_t                      yDelta                  : 9;
  uint32_t                     /* reserved */           : 5;
#else /* vfe 3.2*/
  uint32_t                      yDelta                  : 10;
  uint32_t                     /* reserved */           : 4;
#endif
  /* Rolloff 1 Config*/
  uint32_t                      gridXIndex              : 4;
  uint32_t                      gridYIndex              : 4;
  uint32_t                      gridPixelXIndex         : 9;
  uint32_t                     /* reserved */           : 3;
  uint32_t                      gridPixelYIndex         : 9;
  uint32_t                     /* reserved */           : 3;
  /* Rolloff 2 Config */
#ifdef VFE_31
  uint32_t                      yDeltaAccum             : 12;
  uint32_t                     /* reserved */           : 20;
#elif VFE_2X
  uint32_t                      yDeltaAccum             : 13;
  uint32_t                     /* reserved */           : 3;
#else
  uint32_t                      yDeltaAccum             : 13;
  uint32_t                     /* reserved */           : 19;
#endif
  /* Rolloff 3 Config */
#ifdef VFE_2X
  uint32_t                      pixelOffset             : 9;
  uint32_t                      pixelShift              : 1;
  uint32_t                     /* reserved */           : 6;
#else
  uint32_t                      pixelOffset             : 9;
  uint32_t                     /* reserved */           : 3;
  uint32_t                      pixelShift              : 1;
  uint32_t                     /* reserved */           : 19;
#endif
}__attribute__((packed, aligned(4))) MESH_RollOffConfigParams;

typedef struct MESH_RollOffConfigTable {
  uint32_t initTable[MESH_ROLL_OFF_INIT_TABLE_SIZE*2];
  int32_t  deltaTable[MESH_ROLL_OFF_DELTA_TABLE_SIZE*2];
}MESH_RollOffConfigTable;

typedef struct MESH_RollOffConfigCmdType {
  MESH_RollOffConfigParams CfgParams;
  MESH_RollOffConfigTable Table;
}MESH_RollOffConfigCmdType;
/* End: Data structures to hold HW specific table formats and params */

typedef struct {
  mesh_rolloff_array_type input_table;
}mesh_rolloff_params_t;

typedef struct {
  uint32_t mesh_rolloff_enable;
  uint32_t mesh_rolloff_update;
  /* Driven only by EzTune */
  uint32_t mesh_rolloff_trigger_enable;
  /* Drivern only by EzTune */
  uint32_t mesh_rolloff_reload_params;
  vfe_op_mode_t cur_vfe_mode;
  trigger_ratio_t mesh_rolloff_trigger_ratio;
  MESH_RollOffConfigCmdType mesh_rolloff_prev_cmd;
  MESH_RollOffConfigCmdType mesh_rolloff_snap_cmd;
  mesh_rolloff_params_t mesh_rolloff_prev_param;
  mesh_rolloff_params_t mesh_rolloff_snap_param;
}mesh_rolloff_mod_t;

vfe_status_t mesh_rolloff_init(mesh_rolloff_mod_t* mesh_rolloff_ctrl,
  vfe_rolloff_info_t *mesh_tbls, vfe_params_t* vfe_params);
vfe_status_t mesh_rolloff_config(mesh_rolloff_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params);
vfe_status_t mesh_rolloff_update(mesh_rolloff_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params);
vfe_status_t mesh_rolloff_trigger_update(mesh_rolloff_mod_t* mesh_rolloff_ctrl,
  vfe_rolloff_info_t *mesh_tbls, vfe_params_t* vfe_params);
vfe_status_t mesh_rolloff_trigger_enable(mesh_rolloff_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params, int8_t enable);
vfe_status_t mesh_rolloff_reload_params(mesh_rolloff_mod_t* mesh_rolloff_ctrl,
  vfe_params_t* vfe_params);
#endif /* __MESH_ROLLOFF_H__ */
