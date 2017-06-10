/*============================================================================

   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __SENSOR_INTERFACE_H__
#define __SENSOR_INTERFACE_H__

#include "chromatix.h"
#include "intf_comm_data.h"

typedef enum {
  SENSOR_PROCESS_CONFIG_MODE,
  SENSOR_PROCESS_UPDATE_EXPOSURE,
  SENSOR_PROCESS_SET_FPS,
} sensor_process_type_t;

typedef enum {
  SENSOR_SET_MODE,
  SENSOR_SET_EXPOSURE,
  SENSOR_SET_FPS,
  SENSOR_SET_CHROMATIX_TYPE,
  SENSOR_SET_SATURATION,
  SENSOR_SET_CONTRAST,
  SENSOR_SET_SHARPNESS,
  SENSOR_SET_EXPOSURE_COMPENSATION,
  SENSOR_SET_ISO,
  SENSOR_SET_SPECIAL_EFFECT,
  SENSOR_SET_WHITEBALANCE,
  SENSOR_SET_START_STREAM,
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_OEM,
} sensor_set_type_t;


typedef struct {
  int luma_target;
  int current_luma;
  float gain;
  float digital_gain;
  uint32_t linecount;
  uint32_t fps;
} sensor_set_aec_data_t;

typedef struct {
  uint8_t obj_idx;
  sensor_set_aec_data_t aec_data;
  uint32_t current_fps;
  int32_t saturation;
  int32_t contrast;
  int32_t sharpness;
  int32_t exposure;
  int32_t iso;
  int32_t effect;
  int32_t whitebalance;
  sensor_mode_t mode;
  sensor_load_chromatix_t chromatix_type;
  void* oem_setting;
} sensor_set_data_t;

typedef struct {
  sensor_set_type_t type;
  sensor_set_data_t data;
} sensor_set_t;

typedef struct {
  uint32_t fd;
/* TODO */
} sensor_init_data_t;

typedef struct {
  sensor_process_type_t p_type;
  /* TODO */
} sensor_process_t;

/********************************
     Sensor Interface APIs
*********************************/
int sensor_comp_create();
uint32_t sensor_client_open(module_ops_t *ops);
int sensor_comp_destroy();
#endif /* __SENSOR_INTERFACE_H__ */
