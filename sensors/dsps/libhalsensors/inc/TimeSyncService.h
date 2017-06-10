/*============================================================================
  @file TimeSyncService.h

  @brief
  TimeSyncService class definition.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef ANDROID_TIMER_H
#define ANDROID_TIMER_H

#include "sns_time_api_v02.h"
extern "C" {
#include "sensor1.h"
#define __bool_true_false_are_defined 1
#include "sensors_hal.h"
}

/*============================================================================
 * Class TimeSyncService
 *=============================================================================*/
class TimeSyncService {
private:
    /* time_service enabled status */
    int               enabled;
    /* sensor_handle for time_service */
    sensor1_handle_s  *sensor1_handle;
    /* Offset (in ns) between apps and dsps timestamps */
    int64_t           timestamp_offset_apps_dsps;
    /* Last DSPS timestamp received */
    uint32_t          dsps_ts_last;
    /* # of times the DSPS clock has "rolled over" and restarted at 0 */
    uint32_t          dsps_rollover_cnt;
    /* Rollover count as received in last time service report */
    uint32_t          dsps_rollover_cnt_rcv;
    /* Boottime timestamp of the last predicted rollover event */
    int64_t           boot_ts_last_rollover;
    /* The timestamp on the last sample sent to Android (per sensor) */
    int64_t           timestamp_last_sent[MAX_NUM_SENSORS];
    /* mutex lock for time service sensor1 callback */
    pthread_mutex_t   time_mutex;
    /* cond variable to signal time svc resp has arrived */
    pthread_cond_t    time_cond;
    /* flag to indicate callback has arrived */
    bool              is_resp_arrived;
    /* we only allow time_service class has one object */
    static TimeSyncService *self;
/*============================================================================
  FUNCTION Constructor
    Put the constructor to be private.
============================================================================*/
    TimeSyncService();
/*===========================================================================
  FUNCTION:  rolloverCntIs
    Update the rollover count
    parameters
        @rollover_cnt : the rollover count in the ssc side
===========================================================================*/
    void rolloverCntIs(uint32_t rollover_cnt);
/*===========================================================================
  FUNCTION:  tsOffsetIs
    Update local timestamp offset state.
    paramters
        @timestamp_ssc : the timestamp in the SSC side
        @timestamp_ap: the timestamp in the AS sdie
===========================================================================*/
    void tsOffsetIs(uint32_t timestamp_ssc, uint64_t timestamp_ap);

public:
    ~TimeSyncService();
/*============================================================================
  FUNCTION getTimeSyncService
    Get the TimeSyncService static object.
    Return value
        @TimeSyncService* : The TimeSyncService object
============================================================================*/
    static TimeSyncService* getTimeSyncService();
/*============================================================================
  FUNCTION getCb
    Get the TimeSyncService sensor1 handle;
    Return value
        @sensor1_handle_s* : Then sensor1 callback data
============================================================================*/
    sensor1_handle_s* getCb();
/*===========================================================================
  FUNCTION:  timeServiceStart
    Start the time_service service
    Return value
        1 : successful
        0 : failed
===========================================================================*/
    int timeServiceStart();
/*===========================================================================
  FUNCTION:  timeServiceStop
    Stop the time_service service
    Return value
        1 : successful
        0 : failed
===========================================================================*/
    int timeServiceStop();
/*===========================================================================
  FUNCTION:  getTimeSyncServiceStatus
    Get the time_service status
    Return value
        1 : time_service is started
        0 : time_service is stopped
===========================================================================*/
    int getTimeSyncServiceStatus();
/*===========================================================================
  FUNCTION:  processTimeResp
    Process response message from the time sync service.
    Parameters
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message pointer
===========================================================================*/
    void processTimeResp(sensor1_msg_header_s const *msg_hdr,
        sns_time_timestamp_resp_msg_v02 const *msg_ptr);
/*===========================================================================
  FUNCTION:  processTimeInd
    Process indication message from the time sync service.
    Parameters
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message pointer
===========================================================================*/
    void processTimeInd(sensor1_msg_header_s const *msg_hdr,
        sns_time_timestamp_ind_msg_v02 const *msg_ptr);
/*===========================================================================
  FUNCTION:  processCancerResp
    Process Cancer response to the time_service service.
===========================================================================*/
    void processCancerResp();
/*===========================================================================
  FUNCTION:  timestampCalc
    Converts the DSPS clock ticks from a sensor sample to a LA timestamp (ns
    since epoch).  Adjusts return value based on dsps timestamp rollover
    and makes minor adjustments to ensure sensor samples are sent with
    ascending timestamps.
    Parameters
        @dsps_timestamp : the time stamp in the sensor event
        @sensor_handle : one type of sensor handle
    Return value
        int64_t : the calculated timestamp
===========================================================================*/
    int64_t timestampCalc(uint64_t dsps_timestamp, int sensor_handle);
};

/*===========================================================================
  FUNCTION:  time_service_sensor1_cb
    Parameters
        @cb_data : callback data pointer
        @msg_hdr : sensor1 message header
        @msg_type : sensor1 message type
        @msg_ptr : sensor1 message pointer
===========================================================================*/
void time_service_sensor1_cb(intptr_t cb_data, sensor1_msg_header_s *msg_hdr,
        sensor1_msg_type_e msg_type, void *msg_ptr);

#endif
