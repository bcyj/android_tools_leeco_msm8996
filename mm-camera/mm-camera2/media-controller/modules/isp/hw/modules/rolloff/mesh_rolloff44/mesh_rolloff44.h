/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __MESH_ROLLOFF44_H__
#define __MESH_ROLLOFF44_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "mesh_rolloff44_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "isp_pipeline40.h"
#include "chromatix.h"
#include "chromatix_common.h"

#define Y_DELTA_Q_LEN 13

#define CHROMATIX_MESH_ROLL_NUM_ROW 17
#define CHROMATIX_MESH_ROLL_NUM_COL 13

#define HW_MESH_ROLL_NUM_ROW 13
#define HW_MESH_ROLL_NUM_COL 10

#define MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS  12
#define MESH_ROLL_OFF_V4_VERTICAL_GRIDS    9
#define MESH_ROLL_OFF_V4_TABLE_SIZE  (13 * 10)
#define CHROMATIX_MESH_TABLE_SIZE  (17 * 13)

typedef struct {
  /* Table for red.  */
  uint16_t TableR[MESH_ROLL_OFF_V4_TABLE_SIZE];
  /* Table for Gr.   */
  uint16_t TableGr[MESH_ROLL_OFF_V4_TABLE_SIZE];
  /* Table for blue. */
  uint16_t TableB[MESH_ROLL_OFF_V4_TABLE_SIZE];
  /* Table for Gb.   */
  uint16_t TableGb[MESH_ROLL_OFF_V4_TABLE_SIZE];
}MESH_RollOffTable_V4;

typedef struct {
  MESH_RollOffTable_V4 left[ISP_ROLLOFF_MAX_LIGHT];
  MESH_RollOffTable_V4 right[ISP_ROLLOFF_MAX_LIGHT];
}isp_rolloff_info_t;

typedef struct {
  isp_rolloff_info_t *rolloff_tableset[ISP_ROLLOFF_LENS_POSITION_MAX];
}isp_rolloff_tableset_t;

/* Start: Data structures to hold re-arranged tables before writing them
 *        into HW specific format */
typedef struct MESH_RollOff_V4_ConfigTable {
  uint32_t Table[MESH_ROLL_OFF_V4_TABLE_SIZE*2];
}MESH_RollOff_V4_ConfigTable;

typedef struct MESH_RollOff_V4_ConfigCmdType {
  uint32_t dmi_set[2];
  MESH_RollOff_V4_ConfigTable Table;
  uint32_t dmi_reset[2];
  MESH_RollOff_v4_ConfigParams CfgParams;
}MESH_RollOff_V4_ConfigCmdType;
/* End: Data structures to hold HW specific table formats and params */

typedef struct {
  MESH_RollOffTable_V4 input_table;
}mesh_rolloff_V4_params_t;

typedef struct {
  uint32_t mesh_rolloff_enable;
  uint32_t mesh_rolloff_update;
  int fd;
  float cur_lux;
  float cur_real_gain;
  float cur_mired_color_temp;
  cam_flash_mode_t cur_flash_mode;
  /* Driven only by EzTune */
  uint32_t mesh_rolloff_trigger_enable;
  /* Drivern only by EzTune */
  uint32_t mesh_rolloff_reload_params;
  trigger_ratio_t mesh_rolloff_trigger_ratio;
  MESH_RollOff_V4_ConfigCmdType mesh_rolloff_cmd;
  mesh_rolloff_V4_params_t mesh_rolloff_param;
  isp_rolloff_tableset_t rolloff_calibration_table;
  isp_rolloff_tableset_t rolloff_tbls;
  uint8_t hw_update_pending;
  MESH_RollOff_V4_ConfigTable applied_table;
  cam_streaming_mode_t old_streaming_mode;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  uint16_t af_macro;
  uint16_t af_infinity;
}isp_mesh_rolloff_mod_t;

typedef struct {
  uint32_t rolloff_update;
  isp_rolloff_info_t *rolloff_tbls;
  isp_mesh_rolloff_mod_t mesh_v4_ctrl;
  int hw_enable_cmd;
}isp_rolloff_mod_t;

#endif /* __MESH_ROLLOFF44_H__ */
