/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CSI_INTERFACE_H__
#define __CSI_INTERFACE_H__

#include "tgtcommon.h"
#include "sensor_interface.h"

typedef enum {
  CSI_SET_DATA,
  CSI_SET_INIT,
  CSI_SET_CFG,
  CSI_SET_RELEASE,
  CSI_PROCESS_INIT,
  CSI_MAX_NUM
} csi_set_params_type_t;

typedef union {
  enum msm_sensor_resolution_t res;
  sensor_csi_params_t *csi_params;
} csi_set_data_t;

typedef struct {
  csi_set_data_t data;
} csi_set_t;

/********************************
     CSI Interface APIs
*********************************/
int CSI_comp_create();
uint32_t CSI_client_open(module_ops_t *ops);
int CSI_comp_destroy();
#endif
