/*============================================================================
  @file Latency.h

  @brief
  Latency class definition.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef ANDROID_LATENCY_H
#define ANDROID_LATENCY_H

#include "Sensor.h"
#include "sns_smgr_api_v01.h"

typedef enum {
    HAL_LTCY_MEASURE_ACCEL,
    HAL_LTCY_MEASURE_GYRO,
    HAL_LTCY_MEASURE_MAG,
    HAL_LTCY_MEASURE_PRESSURE,
    HAL_LTCY_MEASURE_PROX_LIGHT,
    HAL_LTCY_MEASURE_HUMIDITY,
    HAL_LTCY_MEASURE_RGB,
    HAL_LTCY_MEASURE_IR_GESTURES,
    HAL_LTCY_MEASURE_SAR,
    HAL_LTCY_MEASURE_HALL_EFFECT,
    HAL_LTCY_NUM_TYPES
} hal_ltcy_measure_t;

/*============================================================================
 * Class Latency
 *=============================================================================*/
class Latency {
    /* put the constructor to be private*/
    Latency();
    ~Latency();
    /* True if latency measure enabled for this sensor */
    static bool latency_en_table[HAL_LTCY_NUM_TYPES];
    static bool is_latency_measure_enabled;
    /* When latency measurement enabled, records DSPS tick for sensor data */
    static uint32_t latency_measure_dsps_tick;
/*===========================================================================
  FUNCTION: latencyCheckSSIRegFlag
    checks a particular CFG group about whether latency measurement enabled or
    not for the corresponding sensor, then update the latency enable control
    member and latency measurement enable table in global variable g_sensor_control.
    Parameters
        @ssi_flag_id : CFG group flag ID
        @ssi_sensor_id : CFG group sensor ID
===========================================================================*/
    static void latencyCheckSSIRegFlag(int ssi_flag_id, int ssi_sensor_id);

public:
/*============================================================================
  FUNCTION: isLatencyMeasureEnabled
    This function returns the value of is_latency_measure_enabled.
    Return value
        @bool : is_latency_measure_enabled value
 *=============================================================================*/
    static bool isLatencyMeasureEnabled();
/*============================================================================
  FUNCTION: setTick
    Set the TimeStamp to latency_measure_dsps_tick.
 *=============================================================================*/
    static void setTick(uint32 TimeStamp);
/*============================================================================
  FUNCTION: latencyCheckMeasure
    This function checks whether latency measurement enabled or not for each
    physical sensor.
 *=============================================================================*/
    static void latencyCheckMeasure();
/*===========================================================================
  FUNCTION: latencyMeasure
    Measure the delivery latency and report the result using log packet.
    Parameters
        @curr_ts : current apps timestamp
        @data : array of sensor data got by hal_sensors_data_poll
        @rcv_data_num : is the number of data got from hal_sensors_data_poll
===========================================================================*/
    static void latencyMeasure(uint64_t curr_ts,
                               const sensors_event_t data[],
                               int rcv_data_num);
};

#endif
