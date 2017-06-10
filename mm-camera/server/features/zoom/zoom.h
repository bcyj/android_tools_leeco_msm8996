/*==============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
 *============================================================================*/
#ifndef __VFECONFIG_ZOOM_H__
#define __VFECONFIG_ZOOM_H__

#include "tgtcommon.h"

#define MAX_ZOOM_STEPS      182
#define ZOOM_TABLE_MAX_DEF  182

typedef struct {
  uint32_t input1_width;
  uint32_t output1_width;
  uint32_t input1_height;
  uint32_t output1_height;
  uint32_t input2_width;
  uint32_t output2_width;
  uint32_t input2_height;
  uint32_t output2_height;
  int8_t update_flag;
} zoom_scaling_params_t;

typedef struct {
  uint32_t *ext_zoom_table;
  uint32_t *zoomTable;
  uint32_t zoomTableBump[ZOOM_TABLE_MAX_DEF];
  int zoom_table_size;
} zoom_info_t;

typedef struct {
  zoom_info_t zoomInfo;
  cam_parm_info_t cam_parm_zoom;
  uint32_t zoom_step_size;
  int32_t zoom_val;
  uint32_t resize_factor;
  zoom_scaling_params_t zoomscaling;
} zoom_ctrl_t;

typedef enum {
  ZOOM_PARM_TYPE_NOTUSED = 0,
  ZOOM_PARM_GET_SCALING_INFO,
  ZOOM_PARM_GET_CROP_FACTOR,
  ZOOM_PARM_TYPE_MAX,
} zoom_parm_type_t;

typedef enum {
  ZOOM_PROC_CMD_NOTUSED = 0,
  ZOOM_PROC_CMD_ZOOM_RATIOS,
  ZOOM_PROC_CMD_MAX
} zoom_process_cmd_t;

extern int zoom_ctrl_save_plugin_table(zoom_ctrl_t *pme, int num_entries, uint32_t *zoom_table_ptr);
extern int zoom_init_ctrl(zoom_ctrl_t *pme);
extern int zoom_process(zoom_ctrl_t *pme, zoom_process_cmd_t p_type, void *parm);
extern int zoom_get_parms(zoom_ctrl_t *pme, zoom_parm_type_t parm_type, void *parm_in, void* parm_out);
extern void zoom_ctrl_deinit(zoom_ctrl_t *pme);
#endif /* __VFECONFIG_ZOOM_H__ */
