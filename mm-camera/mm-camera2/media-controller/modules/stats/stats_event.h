/* stats_event.h
 *
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __STATS_EVENT_H__
#define __STATS_EVENT_H__
#ifdef FEATURE_GYRO
#include "sns_sam_gyroint_v01.h"
#endif

/** mct_event_gyro_data_t
 *    @frame_id:      the id of frame
 *    @ready:         indicate whether the data is ready
 *    @sample_len:    indicate the number of samples in @sample
 *    @sample:        the array of sample
 *    @sof:           the timestamp of SOF
 *    @frame_time:    the duration of frame
 *    @exposure_time: the expoure value
 *
 *  This structure is used to store gyro data
 *
 **/
typedef struct _mct_event_gyro_data {
  uint32_t frame_id;
  int      ready;
  uint32_t sample_len;
#ifdef FEATURE_GYRO
  sns_sam_gyroint_sample_t_v01 sample[SNS_SAM_GYROINT_MAX_BUFSIZE_V01 * 2];
#endif
  uint64_t sof;
  uint64_t frame_time;
  float    exposure_time;
} mct_event_gyro_data_t;

/** mct_event_gyro_stats_t
 *    @q16_angle:  the angle for x, y, z
 *    @ts:         the timestamp
 *
 *  This structure is used to store gyro data
 *
 **/
typedef struct _mct_event_gyro_stats {
  int      q16_angle[3];
  uint64_t ts;
  mct_event_gyro_data_t is_gyro_data;
} mct_event_gyro_stats_t;
#endif /* __STATS_EVENT_H__ */
