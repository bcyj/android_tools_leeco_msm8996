/*============================================================================
   Copyright (c) 2012 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

   This file defines the media/module/master controller's interface with the
   DSPS modules.

============================================================================*/

#ifndef _DSPS_HW_INTERFACE_H_
#define _DSPS_HW_INTERFACE_H_

#include "time.h"
#include <sns_sam_gyroint_v01.h>

#define SNS_CAM_MAX_SAMPLES_PER_FRAME_V01 10
#define FLOAT_FROM_Q16( float_output, q16_input )   \
  ( (float_output) = ((float)(q16_input))/65536.0 )

typedef enum {
  DSPS_ENABLE_REQ,
  DSPS_DISABLE_REQ,
  DSPS_GET_REPORT
} dsps_req_msg_type_t;

typedef enum {
  INTEGRATED_ANGLE,
  BUFFERED_ANGLE,
  FRAME_STATS,
  ENABLE_ALL,
} dsps_mode_t;

typedef struct {
  dsps_mode_t mode;
  int enable_integrate_angle;
  int enable_buffered_samples;
  int enable_frame_stats;
} sensor_feature_t;

typedef struct {
  uint8_t id;
  uint64_t t_start; /* Start Time (microsec) */
  uint64_t t_end;   /* End Time (microsec) */
} dsps_data_t;

typedef struct {
  dsps_req_msg_type_t msg_type;
  sensor_feature_t sensor_feature;
  dsps_data_t data;
} dsps_set_data_t;

typedef enum {
  GET_DATA_FRAME,
  GET_DATA_LAST
} dsps_get_t;

typedef enum {
  TYPE_Q16,
  TYPE_FLOAT
} dsps_format_t;

typedef struct {
  uint8_t seq_no;
  long q16[3];
  float flt[3];
  uint32_t sample_len;  /**< Must be set to # of elements in sample */
  uint32_t gyro_samples_included;
  sns_sam_gyroint_sample_t_v01 sample[SNS_SAM_GYROINT_MAX_BUFSIZE_V01 * 2];
} dsps_output_t;

typedef struct {
  dsps_get_t type;
  dsps_format_t format;
  uint8_t id;
  dsps_output_t output_data;  /* to populate data*/
} dsps_get_data_t;


typedef void (*dsps_callback_func)(void *port, unsigned int frame_id);

/* Function Prototypes */
void* dsps_proc_init(void *port, dsps_callback_func dsps_callback);
void dsps_proc_deinit(void *dsps_client_handle);
int dsps_proc_set_params(void *dsps_client_handle, dsps_set_data_t *data);
int dsps_get_params(void *dsps_client_handle, dsps_get_data_t *data,
  int include_gyro_samples);
int dsps_cirq_search(void * config, void *data, uint8_t id);
void dsps_cirq_add(void * config, void * data, uint8_t seqnum);
int dsps_cirq_get_last(void * config, void *data);

#endif /* _DSPS_HW_INTERFACE_H_ */
