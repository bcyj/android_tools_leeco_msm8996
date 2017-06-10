/*==========================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
#ifndef __ACTUATOR_DRIVER_H__
#define __ACTUATOR_DRIVER_H__

#include <media/msm_cam_sensor.h>

/**
 * msm_actuator_reg_tbl:
 * @reg_tbl_size: Table size
 * @reg_params: Register params
 *
 **/
struct msm_actuator_reg_tbl_t {
  uint8_t reg_tbl_size;
  struct msm_actuator_reg_params_t reg_params[MAX_ACTUATOR_REG_TBL_SIZE];
}__attribute__((packed, aligned(4)));

/**
 * damping_t:
 * @ringing_params: ringing params in all regions
 **/
struct damping_t {
  struct damping_params_t ringing_params[MAX_ACTUATOR_REGION];
}__attribute__((packed, aligned(4)));

/** _actuator_tuned_params:
 * @scenario_size: Scenario size in both directios.
 * @ringing_scenario: ringing parameters.
 * @initial_code: initial code.
 * @region_size: region size.
 * @region_params: region_params
 * @damping: damping params
 **/
typedef struct _actuator_tuned_params {
  uint16_t scenario_size[NUM_ACTUATOR_DIR];
  uint16_t ringing_scenario[NUM_ACTUATOR_DIR][MAX_ACTUATOR_SCENARIO];
  int16_t initial_code;
  uint16_t region_size;
  struct region_params_t region_params[MAX_ACTUATOR_REGION];
  struct damping_t damping[NUM_ACTUATOR_DIR][MAX_ACTUATOR_SCENARIO];
}__attribute__((packed, aligned(4))) actuator_tuned_params_t;

/**
 * _actuator_params:
 * @module_name: Module name
 * @actuator_name: actuator name
 * @i2c_addr: I2C address of slave
 * @i2c_data_type: data width
 * @i2c_addr_type: address width
 * @data_size: data size
 * @reg_tbl: actuator table info
 * @af_restore_pos: af restore position
 * @init_settings: Initial register settings
 * @init_setting_size: initial tabel size
 **/
typedef struct _actuator_params {
  char module_name[MAX_ACT_MOD_NAME_SIZE];
  char actuator_name[MAX_ACT_NAME_SIZE];
  uint32_t i2c_addr;
  enum msm_actuator_data_type i2c_data_type;
  enum msm_actuator_addr_type i2c_addr_type;
  enum actuator_type act_type;
  uint16_t data_size;
  uint8_t af_restore_pos;
  struct msm_actuator_reg_tbl_t reg_tbl;
  uint16_t init_setting_size;
  struct reg_settings_t init_settings[MAX_ACTUATOR_INIT_SET];
}__attribute__((packed, aligned(4))) actuator_params_t;

/**
 * _actuator_driver_params:
 * @actuator_params: parameters specific to actuator
 * @actuator_tuned_params:
**/
typedef struct _actuator_driver_params {
  actuator_params_t actuator_params;
  actuator_tuned_params_t actuator_tuned_params;
}__attribute__((packed, aligned(4))) actuator_driver_params_t;

/**
 * _actuator_driver_ctrl:
 * @actuator_driver_params: actuator_driver_params
 **/
typedef struct _actuator_driver_ctrl {
  actuator_driver_params_t actuator_driver_params;
}__attribute__((packed, aligned(4))) actuator_driver_ctrl_t;

#endif /* __ACTUATOR_DRIVER_H__ */
