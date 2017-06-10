/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __ISPIF_INTERFACE_H__
#define __ISPIF_INTERFACE_H__

#include "tgtcommon.h"
#include "intf_comm_data.h"

typedef enum {
  ISPIF_SET_INTF_PARAMS,
  ISPIF_SESSION_LOCK,
  ISPIF_GET_CHANNEL_INFO,
  ISPIF_PROCESS_INIT,
  ISPIF_PROCESS_CFG,
  ISPIF_PROCESS_START_ON_FRAME_BOUNDARY,
  ISPIF_PROCESS_STOP_ON_FRAME_BOUNDARY,
  ISPIF_PROCESS_STOP_IMMEDIATELY,
  ISPIF_PROCESS_RELEASE,
  ISPIF_PARM_ADD_OBJ_ID,
  ISPIF_MAX_NUM
} ispif_params_type_t;

typedef struct {
  uint8_t acquire;
  uint8_t vfe_id;
} session_lock_t;

typedef struct {
  struct msm_ispif_params_list ispif_params_list;
  uint8_t output_port_info;
  uint32_t channel_stream_info;
  uint8_t vfe_interface;
  session_lock_t session_lock;
} ispif_set_data_t;

typedef struct {
  ispif_set_data_t data;
} ispif_set_t;

typedef struct {
  uint32_t channel_interface_mask;
  uint32_t channel_stream_info;
} ispif_get_data_t;

typedef struct {
  ispif_get_data_t data;
} ispif_get_t;


typedef union {
  uint32_t data;
} ispif_process_data_t;

typedef struct {
  ispif_process_data_t data;
} ispif_process_t;

/********************************
     ISPIF Interface APIs
*********************************/
int ISPIF_comp_create();
uint32_t ISPIF_client_open(module_ops_t *ops);
int ISPIF_comp_destroy();

#endif

