/*============================================================================

   Copyright (c) 2012 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

   This file defines the media/module/master controller's interface with the
   DSPS modules. The functionalities od this module include:

   1. Control communication with the sensor module
   2. Process data received from the sensors

============================================================================*/
#ifndef _DSPS_HW_H_
#define _DSPS_HW_H_


#include <sensor1.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include "camera_dbg.h"
#include "dsps_hw_interface.h"
#include <sns_sam_gyroint_v01.h>

#define SENSOR_SAMPLE_RATE  (200 << 16)
#define SENSOR_TIME_OUT     1000  /* Timeout for response message */
#define MSEC_TO_SEC         1/1000
#define USEC_TO_NSEC        1000
#define SEC_TO_MSEC         1000
#define MSEC_TO_NSEC        1000000
#define SEC_TO_USEC         1000000
#define SEC_TO_NSEC         1000000000
#define MAX_Q_SIZE          5
#define INVALID_INSTANCE_ID 0xFF
#define DSPS_HZ             32768U
#define SNS_SAM_ALGO_REPORT_IND 0x05
#define TIMESYNC_FREQ       16

#define DSPS_PROC_DEBUG     0

#if(DSPS_PROC_DEBUG)
  #include <utils/Log.h>
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-DSPS"
  #define CDBG_DSPS(fmt, args...) ALOGE(fmt, ##args)
  #define CDBG_ERROR(fmt, args...) ALOGE(fmt, ##args)
#else
  #define CDBG_DSPS(fmt, args...) do{}while(0)
  #define CDBG_ERROR(fmt, args...) ALOGE(fmt, ##args)
#endif
typedef struct {
  dsps_req_msg_type_t msg_type;
  uint64_t t_start;
  uint64_t t_end;
  uint8_t seqnum;
  int enable_integrate_angle;
  int enable_buffered_samples;
  int enable_frame_stats;
} sensor1_req_data_t;

typedef enum {
  DSPS_RUNNING = 1,
  DSPS_STOPPED = 0,
  DSPS_BROKEN_PIPE = -1,
  DSPS_FAILED_CREATE = -2,
} dsps_thread_status_t;

typedef struct {
  uint8_t seqnum;
  int32_t x;
  int32_t y;
  int32_t z;
  uint32_t sample_len;  /**< Must be set to # of elements in sample */
  sns_sam_gyroint_sample_t_v01 sample[SNS_SAM_GYROINT_MAX_BUFSIZE_V01 * 2];
} buffer_data_t;

typedef struct cirq_node {
  buffer_data_t data;
} cirq_node;

/** _is_info:
 *    @thread_id: thread id used to join
 *    @handle: sensor1 handle
 *    @callback_mutex: mutex for use with sensor1 callback
 *    @callback_condvar: cvar for callback from sensor
 *    @callback_arrived: flag to indicate callback arrived
 *    @dsps_callback: callback into upper gyro layer
 *    @thread_mutex: mutex for DSPS thread
 *    @thread_condvar: cvar for DSPS thread
 *    @error: error indication
 *    @instance_id: GYROINT service instance id
 *    @status: current status of DSPS thread
 *    @data_mutex: mutex for gyro data
 *    @queue: gyro data circular queue
 *    @q_pos: tail of circular queue
 *    @q_size: queue size
 *    @seqnum: DSPS layer - sensor1 transaction id
 **/
typedef struct _sensor1_config {
  pthread_t thread_id;
  sensor1_handle_s *handle;
  pthread_mutex_t callback_mutex;
  pthread_cond_t callback_condvar;
  int callback_arrived;
  dsps_callback_func dsps_callback;
  pthread_mutex_t thread_mutex;
  pthread_cond_t thread_condvar;
  int error;
  int instance_id;
  dsps_thread_status_t status;
  pthread_mutex_t data_mutex;
  void *port;
  cirq_node queue[MAX_Q_SIZE];
  int q_pos;
  int q_size;
  uint8_t seqnum;
} sensor1_config_t;


/* Function Prototypes */
int dsps_send_request(void *sensor_config, void *req_data, int wait);
int dsps_disconnect(void *sensor_config);
int dsps_open(void *sensor_config);
void dsps_cirq_init(void *sensor_config);

#endif /* _DSPS_HW_H_ */
