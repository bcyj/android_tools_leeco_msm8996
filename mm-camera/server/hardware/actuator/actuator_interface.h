/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __ACTUATOR_INTERFACE_H__
#define __ACTUATOR_INTERFACE_H__

#include "tgtcommon.h"
#include <media/msm_camera.h>
#include "af_tuning.h"

typedef enum {
  ACTUATOR_GET_INFO,
  ACTUATOR_GET_AF_TUNE_PTR,
  ACTUATOR_MOVE_FOCUS,
  ACTUATOR_DEF_FOCUS,
  ACTUATOR_RESTORE_FOCUS,
  ACTUATOR_LOAD_PARAMS,
  ACTUATOR_TEST_RING,
  ACTUATOR_TEST_LINEAR,
  ACTUATOR_PARAMS_MAX_NUM
} actuator_params_type_t;

typedef struct {
  uint8_t af_support;
  af_tune_parms_t *af_tune_ptr;
} actuator_get_data_t;

typedef struct {
  actuator_get_data_t data;
} actuator_get_t;

typedef struct {
  uint32_t far_end; /* Far end */
  uint32_t near_end; /* Near end */
  uint32_t undershoot_adjust; /* Undershoot adjust */
  uint16_t gross_step; /* Coarse jump step size */
  uint16_t fine_step; /* Fine step size */
} actuator_info_t;

typedef struct {
  int32_t num_steps; /* Steps to move */
  int32_t direction; /* Direction to move */
} actuator_move_t;

typedef struct {
  uint8_t stepsize;
} actuator_test_t;

typedef union {
  actuator_info_t info;
  actuator_move_t move;
  actuator_test_t test;
} actuator_set_data_t;

typedef struct {
  actuator_set_data_t data;
} actuator_set_t;

/********************************
     Actuator Interface APIs
*********************************/
int ACTUATOR_comp_create();
uint32_t ACTUATOR_client_open(module_ops_t *ops);
int ACTUATOR_comp_destroy();

#endif /* __ACTUATOR_INTERFACE_H__ */
