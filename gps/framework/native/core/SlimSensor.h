/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef SLIM_SENSOR_H
#define SLIM_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

#include "log_util.h"
#include "loc_log.h"

namespace lbs_core {

#define MAX_SAMPLES 50

typedef enum {
    ACCEL = 0,
    ACCEL_TEMPERATURE = 1,
    GYRO = 2,
    GYRO_TEMPERATURE = 3,
    MOTION = 7,
    PEDO = 8,
    MAG_CALIB = 13,
    MAG_UNCALIB = 14,
    BARO = 16,
    SPI
}SensorType;

typedef enum {
    REQUEST_START = 1,
    REQUEST_STOP = 0
}SensorRequestType;

typedef struct {
    uint16_t samplesPerBatch;
    uint16_t batchesPerSecond;
    uint16_t samplingRate;
}SensorRate;

typedef struct {
    SensorType requestType;
    SensorRequestType requestValue;
    int8_t frequencyValid;
    SensorRate requestRate;
}SensorRequest;

typedef struct {
    SensorRequestType requestValue;
    uint8_t resetStepCountValid;
    uint8_t resetStepCount;
    uint8_t stepCountThresholdValid;
    uint32_t stepCountThreshold;
}PedometerRequest;

typedef struct {
    uint32_t clientTxTime;
}TimeRequest;

typedef enum
{
   UNSPECIFIED = 0,
   COMMON = 1
}TimeSourceType;

typedef struct {
    uint16_t timeOffset;
    float xAxis;
    float yAxis;
    float zAxis;
}SensorDataSample;

typedef enum
{
   SIGN_REVERSAL = 1,
   SENSOR_TIME_IS_MODEM_TIME = 2,
   CALIBRATED_DATA = 4
}FlagType;

typedef struct {
    SensorType sensorType;
    uint32_t timeOfFirstSample;
    uint8_t flags;
    TimeSourceType timeSource;
    uint32_t sensorDataLength;
    SensorDataSample samples[MAX_SAMPLES];
}SensorDataBundle;

typedef struct {
    TimeSourceType timeSource;
    uint32_t timestamp;
    uint32_t timeInterval;
    uint32_t stepCount;
    uint8_t stepConfidenceValid;
    uint8_t stepConfidence;
    uint8_t stepCountUncertaintyValid;
    float stepCountUncertainty;
    uint8_t stepRateValid;
    float stepRate;
}PedometerData;

typedef enum
{
   MOTION_STATE_UNKNOWN = 0,
   MOTION_STATE_STATIONARY = 1,
   MOTION_STATE_IN_MOTION = 2,
}MotionStateType;

typedef enum
{
   MOTION_MODE_UNKNOWN = 0,
   MOTION_MODE_STATIONARY = 1,
   MOTION_MODE_PEDESTRIAN_UNKNOWN = 200,
   MOTION_MODE_PEDESTRIAN_WALKING = 201,
   MOTION_MODE_PEDESTRIAN_RUNNING = 202,
   MOTION_MODE_VEHICLE_UNKNOWN = 300,
}MotionModeType;

typedef struct {
    MotionStateType state;
    MotionModeType mode;
    float probability;
    uint16_t age;
    uint16_t timeout;
}MotionData;

typedef struct {
    uint8_t stationary;
    uint8_t confidenceStationaryValid;
    uint8_t confidenceStationary;
}SPIStatus;

typedef enum
{
    NOT_MOUNTED = 0,
    MOUNTED = 1,
    MOUNT_UNKNOWN = 2
}CradleMountType;

typedef struct {
    CradleMountType cradleMountState;
    uint8_t confidenceCradleMountStateValid;
    uint8_t confidenceCradleMountState;
}MountStatus;

typedef struct {
    uint32_t clientTxTime;
    uint32_t remoteProcRxTime;
    uint32_t remoteProcTxTime;
}TimeData;

} // lbs_core

#endif /* SLIM_SENSOR_H */
