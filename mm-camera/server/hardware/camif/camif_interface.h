/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __CAMIF_INTF_H__
#define __CAMIF_INTF_H__

#include <string.h>
#include "tgtcommon.h"
#include "intf_comm_data.h"
#include "camera_dbg.h"

//#define ENABLE_CAMIF_LOGGING

typedef enum {
  CAMIF_MODULE_INIT,
  CAMIF_OPS_CONFIG,
  CAMIF_OPS_TIMER_CONFIG,
  CAMIF_PROC_CMD_OPS,
}camif_ops_t;

typedef struct {
  int enabled;
  unsigned long duration;
} camif_strobe_info_t;

typedef struct {
  uint8_t obj_idx;
  union {
    /* this parameter will be removed once sensor dependency is removed */
    pixel_crop_info_t sensor_crop_info;
    camera_size_t sensor_dim;
    camera_op_mode_t mode;
    sensor_output_format_t format;
    camif_strobe_info_t strobe_info;
    uint32_t connection_mode;
	uint32_t vfe_version;
  }d;
}camif_input_t;

//camera_status_t camif_interface_create(uint32_t *handle);
//camera_status_t camif_interface_init(uint32_t handle, int fd);
//camera_status_t camif_interface_set_params(uint32_t handle,
//  camif_params_type_t type, camif_input_t* input);
//camera_status_t camif_interface_get_params(uint32_t handle,
//  camif_params_type_t type, camif_output_t* output);
//camera_status_t camif_interface_process(uint32_t handle, camif_ops_t ops);
//camera_status_t camif_interface_destroy(uint32_t handle);

int CAMIF_comp_create();
uint32_t CAMIF_client_open(module_ops_t *ops);
int CAMIF_comp_destroy();

#endif //__CAMIF_INTF_H__
