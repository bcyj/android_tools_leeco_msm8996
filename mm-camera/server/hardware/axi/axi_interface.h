/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __AXI_INTERFACE_H__
#define __AXI_INTERFACE_H__

#include "tgtcommon.h"
#include "intf_comm_data.h"

#define VFE_PIXEL_IF_MIPI 3

typedef enum {
  AXI_PARM_OUTPUT_INFO,
  AXI_PARM_UPDATE_CONFIG,
  AXI_PARM_PREVIEW_FORMAT,
  AXI_PARM_SNAPSHOT_FORMAT,
  AXI_PARM_RECORDING_FORMAT,
  AXI_PARM_THUMBNAIL_FORMAT,
  AXI_PARM_RDI_FORMAT,
  AXI_PARM_SENSOR_DATA,
  AXI_PARM_HW_VERSION,
  AXI_PARM_ADD_OBJ_ID,
  AXI_PARM_RESERVE_INTF,
  AXI_PARM_RESERVE_BUS_INTF,
  AXI_PARM_STATS_VERSION,
}axi_set_parm_type_t;

typedef struct {
  sensor_output_format_t sensor_output_format;
  sensor_raw_output_t sensor_raw_depth;
}axi_sensor_data_t;

typedef struct {
  axi_intf_type_t intf_type;
  uint32_t interface_mask;
  current_output_info_t output_info;
  struct msm_camera_vfe_params_t mod_params;
  cam_format_t prev_format;
  cam_format_t rec_format;
  cam_format_t snap_format;
  cam_format_t thumb_format;
  cam_format_t rdi_format;
  axi_sensor_data_t sensor_data;
  uint32_t vfe_version;
  uint8_t axi_obj_idx;
  uint32_t stats_version;
  int current_target;
}axi_set_parm_data_t;

typedef struct {
  axi_set_parm_type_t type;
  axi_set_parm_data_t data;
}axi_set_t;

typedef enum {
  AXI_PROC_EVENT_NULL,
  AXI_PROC_EVENT_CONFIG,
  AXI_PROC_CMD_OPS,
  AXI_PROC_EVENT_REG_UPDATE,
  AXI_PROC_EVENT_UNREGISTER_WMS,
  AXI_PROC_EVENT_MAX
} axi_proc_event_t;

typedef struct {
  camera_op_mode_t mode;
  vfe_ports_used_t vfe_port;
} axi_config_t;

typedef struct {
  mod_cmd_ops_t cmd_type;
  vfe_ports_used_t port;
} axi_proc_cmd_ops_t;

/*===========================================================================
 *  AXI Interface APIs
 *==========================================================================*/
//uint32_t axi_interface_create();
//int axi_interface_init(uint32_t handle, uint32_t vfe_version, int camfd);
//int axi_set_params(uint32_t handle, axi_set_t *axi_set);
//int axi_process(uint32_t handle, camera_op_mode_t mode,
//                vfe_ports_used_t vfe_port);
//int axi_interface_destroy(uint32_t handle);
int AXI_comp_create();
uint32_t AXI_client_open(module_ops_t *ops, int sdev_number);
int AXI_comp_destroy();

#endif //__AXI_INTERFACE_H__
