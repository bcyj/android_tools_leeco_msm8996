/*==========================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
#ifndef __ACTUATOR_H__
#define __ACTUATOR_H__

#include "intf_comm_data.h"
#include "actuator_interface.h"

typedef struct {
  af_tune_parms_t af_tune;
} actuator_ctrl_t;

typedef struct {
  uint32_t fd;
  actuator_ctrl_t *ctrl;
  int16_t curr_step_pos;
  int16_t cur_restore_pos;
  uint16_t total_steps;
  uint8_t is_af_supported;
} actuator_t;

#define ACTUATOR_MAX_CLIENT_NUM 4

typedef struct {
  uint8_t client_idx;
  uint32_t handle;
  uint8_t my_comp_id;
  mctl_ops_t *ops;
  actuator_t actuator_ctrl_obj;
} actuator_client_t;

typedef struct {
  pthread_mutex_t mutex;
  uint32_t actuator_handle_cnt;
  actuator_client_t client[ACTUATOR_MAX_CLIENT_NUM];
} actuator_comp_root_t;

int af_actuator_load_params(void *ptr, struct msm_actuator_cfg_data *cfg);
int af_actuator_init(void *ptr);
int af_actuator_move_focus(void *ptr, int32_t direction, int32_t num_steps);
int af_actuator_restore_focus(void *ptr, int32_t direction);
int af_actuator_set_default_focus(void *ptr, int32_t af_step);
int af_actuator_get_info(void *ptr, actuator_get_data_t *actuator_get);
int af_actuator_set_info(void *ptr, actuator_set_data_t *actuator_set);
int af_actuator_ring_test(void *ptr, uint8_t stepsize);
int af_actuator_linear_test(void *ptr, uint8_t stepsize);

#endif /* __ACTUATOR_H__ */
