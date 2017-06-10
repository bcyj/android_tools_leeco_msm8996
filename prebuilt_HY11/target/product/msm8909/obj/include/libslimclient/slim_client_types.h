#ifndef SLIM_CLIENT_TYPES_H
#define SLIM_CLIENT_TYPES_H
/*============================================================================
@file slim_client_types.h

  SLIM API header containing the client types needed for SLIM communication.

               Copyright (c) 2013-2014 QUALCOMM Atheros, Inc.
               All Rights Reserved.
               Qualcomm Atheros Confidential and Proprietary
============================================================================*/
/* $Header: //components/rel/gnss.mpss/6.0/gnss/slim/common/client/inc/slim_client_types.h#9 $ */

/*----------------------------------------------------------------------------
* Include Files
* -------------------------------------------------------------------------*/
#include "comdef.h"
#include <stdint.h>

/*----------------------------------------------------------------------------
* Preprocessor Definitions and Constants
* -------------------------------------------------------------------------*/
/*! Check if value is set in mask */
#define SLIM_MASK_IS_SET(mask, value) (mask & (1 << value))
/*! Add value to mask */
#define SLIM_MASK_SET(mask, value) mask |= (1 << value)
/*! Remove value from mask */
#define SLIM_MASK_CLEAR(mask, value) mask &= ~(1 << value)

/** Maximum number of device sensor samples in a message */
#define SLIM_SENSOR_MAX_SAMPLE_SETS 50

/** Value for the first service message id */
#define SLIM_FIRST_SERVICE_MSG_ID 0x100
/** Final value that can be used as the SLIM message id. */
#define SLIM_LAST_MSG_ID_VALUE 0xFFFF

/*----------------------------------------------------------------------------
* Type Declarations
* -------------------------------------------------------------------------*/

/** Enum for SLIM service provider flags */
typedef enum
{
  eSLIM_SERVICE_PROVIDER_DEFAULT               = 0,
  /**< Default provider should be used. If this value is set in the provider mask
       of type slimServiceProviderMaskT, other bits of the mask are ignored. */
  eSLIM_SERVICE_PROVIDER_SSC                   = 1,
  /**< Only SSC provider should be used */
  eSLIM_SERVICE_PROVIDER_SAMLITE               = 2,
  /**< Only SAMLite provider should be used */
  eSLIM_SERVICE_PROVIDER_NATIVE                = 3,
  /**< Only native provider should be used */

  eSLIM_SERVICE_PROVIDER_MAX                   = 2147483647 /* Force 32bit */
} slimServiceProviderEnumT;

/** Service provider mask can be set to represent one or a group of providers.
    The index of each bit corresponds a value in slimServiceProviderEnumT.
    Mask can be easily manipulated using following macros:
    - Set value:              SLIM_MASK_SET
      Example:                SLIM_MASK_SET(mask, eSLIM_SERVICE_PROVIDER_SSC);
    - Clear value:            SLIM_MASK_CLEAR
      Example:                SLIM_MASK_CLEAR(mask, eSLIM_SERVICE_PROVIDER_SSC);
    - Check if value is set:  SLIM_MASK_IS_SET
      Example:                if (SLIM_MASK_IS_SET(mask, eSLIM_SERVICE_PROVIDER_SSC))
                              {
                                ...
                              }
*/
typedef uint32_t slimServiceProviderMaskT;

/** Device sensor types */
typedef enum
{
  eSLIM_SENSOR_TYPE_ACCEL        = 0, /**< Accelerometer */
  eSLIM_SENSOR_TYPE_GYRO         = 1, /**< Gyroscope */
  eSLIM_SENSOR_TYPE_ACCEL_TEMP   = 2, /**< Accelerometer temperature */
  eSLIM_SENSOR_TYPE_GYRO_TEMP    = 3, /**< Gyroscope temperature */
  eSLIM_SENSOR_TYPE_MAGNETOMETER = 4, /**< Magnetometer */
  eSLIM_SENSOR_TYPE_BAROMETER    = 5, /**< Barometer */

  eSLIM_SENSOR_TYPE_MAX          = 2147483647 /* Force 32bit */
} slimSensorTypeEnumT;

/** Time source */
typedef enum
{
  eSLIM_TIME_SOURCE_UNSPECIFIED = 0,
  /**< The sensor time source is unspecified.
       Can be converted to common with timesync. */
  eSLIM_TIME_SOURCE_COMMON      = 1,
  /**< The time source is common between the sensors and
       the SLIM engine. Monotonous millisecond time.
       This does not include leap seconds. */
  eSLIM_TIME_SOURCE_GPS         = 2,
  /**< GPS time in milliseconds since GPS epoch 00:00:00 January 6th 1980.
       This does not include leap seconds. */
  eSLIM_TIME_SOURCE_UTC         = 3,
  /**< UTC time in milliseconds since UNIX epoch 00:00:00 January 1st 1970.
       Like UNIX time, this includes leap seconds. */
  eSLIM_TIME_SOURCE_LEGACY      = 4,
  /**< Legacy sensor timestamp. Used only when playing back old logs which
       require sensor to GPS timestamp conversion using legacy timesync. */

  eSLIM_TIME_SOURCE_MAX         = 2147483647 /* Force 32bit */
} slimTimeSourceEnumT;

/** SLIM flags */
#define  SLIM_FLAGS_MASK_TIME_JUMP     0x01
  /**< Time has jumped such that timestamp is not related to
       previous timestamp. This happens when SLIM timesync is reset. */
#define  SLIM_FLAGS_MASK_TIME_UNC_HIGH 0x02
  /**< Timestamp uncertainty is high, and it should not be trusted.
       This may happen if there is too much noise in timesync measurements,
       or if timesync has not yet converged to low-uncertainty estimate. */
#define  SLIM_FLAGS_MASK_DATA_OUTAGE   0x04
  /**< Service has a temporary outage. All fields (except the flags, time source
       and timestamp) are invalid in the data indication. */

typedef struct
{
  uint32_t sampleTimeOffset;
  /**<   Sample time offset. This time offset must be
       relative to the sensor time of the first sample. \n
       - Type: Unsigned integer \n
       - Units: 1/32768 seconds */

  float    sample[3];
  /**<   3-axis Sample
       - Type: Floating point \n
       - Axes: Device has a right-handed coordinate system: \n
               x = Right (when holding phone in hand and looking at the display)
               y = Up
               z = Out of the display
       - Units: eSLIM_SENSOR_TYPE_ACCEL: m/s2 \n
                eSLIM_SENSOR_TYPE_GYRO: rad/s \n
                eSLIM_SENSOR_TYPE_MAGNETOMETER: uTesla

         1-axis Sample
       - Type: Floating point \n
       - Axes: only sample[0] has valid measurement
       - Units: eSLIM_SENSOR_TYPE_ACCEL_TEMP: Celsius \n
                eSLIM_SENSOR_TYPE_GYRO_TEMP: Celsius \n
                eSLIM_SENSOR_TYPE_BAROMETER: hectopascal (hPa) */
} slimSensorSampleStructT;

typedef struct
{
  slimServiceProviderEnumT provider;
  /**< Provider of this data. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  slimTimeSourceEnumT    timeSource;
  /**< Source of timeBase. One of: \n
         eSLIM_TIME_SOURCE_UNSPECIFIED \n
         eSLIM_TIME_SOURCE_COMMON \n
         eSLIM_TIME_SOURCE_GPS \n
         eSLIM_TIME_SOURCE_UTC \n
         eSLIM_TIME_SOURCE_LEGACY   */

  uint64_t               timeBase;
  /**< Base time to which the sample time offsets are added
       to get full sample timestamps. Units: milliseconds */

  uint32_t               flags;
  /**< SLIM flags. Combination of:  \n
         SLIM_FLAGS_MASK_TIME_JUMP  \n
         SLIM_FLAGS_MASK_TIME_UNC_HIGH */

  slimSensorTypeEnumT    sensorType;
  /**< Sensor type. Any of:
         eSLIM_SENSOR_TYPE_ACCEL
         eSLIM_SENSOR_TYPE_GYRO
         eSLIM_SENSOR_TYPE_ACCEL_TEMP
         eSLIM_SENSOR_TYPE_GYRO_TEMP
         eSLIM_SENSOR_TYPE_MAGNETOMETER
         eSLIM_SENSOR_TYPE_BAROMETER */

  uint8_t                 samples_len; /**< Number of samples */
  slimSensorSampleStructT samples[SLIM_SENSOR_MAX_SAMPLE_SETS];
  /**< Variable length array to specify sensor samples;
       maximum length of the array is 50. */
} slimSensorDataStructT;


typedef enum
{
  eSLIM_QMD_MOTION_STATE_UNSPECIFIED = 0,
  /**< The motion state is unspecified */
  eSLIM_QMD_MOTION_STATE_REST        = 1,
  /**< Rest motion state */
  eSLIM_QMD_MOTION_STATE_MOVE        = 2,
  /**< Move motion state */

  eSLIM_QMD_MOTION_STATE_MAX         = 2147483647 /* Force 32bit */
} slimQmdMotionStateEnumT;

/* Note that AMD and RMD use both this structure */
typedef struct
{
  slimServiceProviderEnumT provider;
  /**< Provider of this data. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  slimTimeSourceEnumT      timeSource;
  /**< Source of timestamp. One of: \n
         eSLIM_TIME_SOURCE_UNSPECIFIED \n
         eSLIM_TIME_SOURCE_COMMON \n
         eSLIM_TIME_SOURCE_GPS \n
         eSLIM_TIME_SOURCE_UTC \n
         eSLIM_TIME_SOURCE_LEGACY   */

  uint64_t                 timestamp;
  /**< Timestamp of input used to generate the algorithm output.
        - Type: Unsigned integer \n
        - Units: Milliseconds */

  uint32_t                 flags;
  /**< SLIM flags. Combination of:  \n
         SLIM_FLAGS_MASK_TIME_JUMP  \n
         SLIM_FLAGS_MASK_TIME_UNC_HIGH */

  slimQmdMotionStateEnumT  motionState;
  /**< Motion state output of QMD algorithm instance. */

} slimQmdDataStructT;

typedef enum
{
  eSLIM_SMD_STATE_UNKNOWN    = 0,
  /**< The state is unknown */
  eSLIM_SMD_STATE_NO_MOTION  = 1,
  /**< No motion state */
  eSLIM_SMD_STATE_MOTION     = 2,
  /**< Motion state */

  eSLIM_SMD_MOTION_STATE_MAX = 2147483647 /* Force 32bit */
} slimSmdStateEnumT;

typedef struct
{
  slimServiceProviderEnumT provider;
  /**< Provider of this data. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  slimTimeSourceEnumT      timeSource;
  /**< Source of timestamp. One of: \n
         eSLIM_TIME_SOURCE_UNSPECIFIED \n
         eSLIM_TIME_SOURCE_COMMON \n
         eSLIM_TIME_SOURCE_GPS \n
         eSLIM_TIME_SOURCE_UTC \n
         eSLIM_TIME_SOURCE_LEGACY   */

  uint64_t                 timestamp;
  /**< Timestamp of input used to generate the algorithm output.
        - Type: Unsigned integer \n
        - Units: Milliseconds */

  uint32_t                 flags;
  /**< SLIM flags. Combination of:  \n
         SLIM_FLAGS_MASK_TIME_JUMP  \n
         SLIM_FLAGS_MASK_TIME_UNC_HIGH */

  slimSmdStateEnumT motionState;
  /**< Detected motion state. */

  uint8_t motionStateProbability;
  /**< Probability of the reported motion state scaled to percentage
       (range 0 to 100). */

} slimSmdDataStructT;

typedef enum
{
  eSLIM_INJECT_PEDOMETER_REPORT_ON_STEP_EVENT    = 0,
  /**< Report is injected on detection of a step event */
  eSLIM_INJECT_PEDOMETER_REPORT_PERIODICALLY  = 1,
  /**< Report is injected periodically */

  eSLIM_INJECT_PEDOMETER_REPORT_MAX = 2147483647 /* Force 32bit */
} slimInjectPedometerReportEnumT;

typedef struct
{
  slimServiceProviderEnumT provider;
  /**< Provider of this data. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  slimTimeSourceEnumT      timeSource;
  /**< Source of timestamp. One of: \n
         eSLIM_TIME_SOURCE_UNSPECIFIED \n
         eSLIM_TIME_SOURCE_COMMON \n
         eSLIM_TIME_SOURCE_GPS \n
         eSLIM_TIME_SOURCE_UTC \n
         eSLIM_TIME_SOURCE_LEGACY   */

  uint64_t                 timestamp;
  /**<   Time stamp of the last step event in this report, i.e., the time stamp
         of the step event that caused this report to be generated.
         The time stamp is in the time reference scale that is
         used by the pedometer time source. \n
         - Units: Milliseconds */

  uint32_t                 flags;
  /**< SLIM flags. Combination of:  \n
         SLIM_FLAGS_MASK_TIME_JUMP  \n
         SLIM_FLAGS_MASK_TIME_UNC_HIGH */

  float                    timeInterval;
  /**<   Time interval during which the step count was calculated. Subtracting
         timeInterval from the timestamp field yields the time when
         the step detection for the first step in this report started. \n
         - Units: Milliseconds */

  /*  Step Count */
  uint32_t                 stepCount;
  /**<   Number of steps counted during the time interval. */

  uint8_t                  stepConfidence_valid;
  /**< Must be set to true if stepConfidence is being passed */
  uint8_t                  stepConfidence;
  /**<   Confidence associated with the step. This field is only applicable
         for a single step report, i.e., if the step count is one. \n
         - Range: 0 to 100 \n
         \textbf{Note:} The report is ignored if confidence is 0. */

  uint8_t                  stepCountUncertainty_valid;
  /**< Must be set to true if stepCountUncertainty is being passed */
  float                    stepCountUncertainty;
  /**<   Uncertainty (in steps) associated with the step count. */

  uint8_t                  stepRate_valid;
  /**< Must be set to true if stepRate is being passed */
  float                    stepRate;
  /**<   Current estimate for the rate of steps per second. \n
       - Units: steps/second \n
       - Range: >= 0.0 */

  uint8_t                  strideLength_valid;
  /**< Must be set to true if strideLength is being passed */
  float                    strideLength;
  /**<   Current estimate of the stride length. \n
       - Type: Float \n
       - Units: Meters/stride \n
       - Range: >= 0 */

  uint8_t                  strideLengthUncertainty_valid;
  /**< Must be set to true if strideLengthUncertainty is being passed */
  float                    strideLengthUncertainty;
  /**<   Uncertainty associated with the stride length, provided at a 1-sigma
       (68 percent) confidence. \n
       - Type: Float \n
       - Units: Meters/stride \n
       - Range: >= 0 */

  uint8_t                  stepCountFractional_valid;
  /**< Must be set to true if stepCountFractional is being passed */
  float                    stepCountFractional;
  /**<   Sub-integer portion of the number of steps counted during the time
       interval. If the report is generated on event, this will be 0. \n
       - Type: Float \n
       - Units: steps \n
       - Range: >= 0.0 and < 1.0 */

  uint8_t                  reportType_valid;
  /**< Must be set to true if reportType is being passed */
  slimInjectPedometerReportEnumT reportType;
  /**< Identifies the type of report generated. One of: \n
         eSLIM_INJECT_PEDOMETER_REPORT_ON_STEP_EVENT \n
         eSLIM_INJECT_PEDOMETER_REPORT_PERIODICALLY \n */
} slimPedometerDataStructT;

/*
 *  Motion data
 */

typedef enum
{
  eSLIM_MOTION_STATE_UNKNOWN      = 0,
  /**< Device state is not known. */
  eSLIM_MOTION_STATE_STATIONARY   = 1,
  /**< Device state is Stationary. */
  eSLIM_MOTION_STATE_IN_MOTION    = 2,
  /**< Device state is In Motion. */

  eSLIM_MOTION_STATE_MAX = 2147483647  /* Force 32bit */
} slimMotionStateEnumT;

typedef enum
{

  eSLIM_MOTION_MODE_UNKNOWN              =  0,
  /**< Device movement is not known. */
  /* Add more unknown motion modes here */

  eSLIM_MOTION_MODE_STATIONARY,
  /**< Device is not moving. */
  /* Add more stationary motion modes here */

  eSLIM_MOTION_MODE_PEDESTRIAN_UNKNOWN   =  200,
  /**< Device movement is in Pedestrian mode; nothing else is known about the movement. */
  eSLIM_MOTION_MODE_PEDESTRIAN_WALKING,
  /**< Device movement is in pedestrian Walking mode. */
  eSLIM_MOTION_MODE_PEDESTRIAN_RUNNING,
  /**< Device movement is in pedestrian Running mode. */
  /* Add more pedestrian modes here */

  eSLIM_MOTION_MODE_VEHICLE_UNKNOWN      =  300,
  /**< Device movement is in Vehicular mode; nothing else is known about the movement. */
  /* Add more vehicles modes here */

  eSLIM_MOTION_MODE_MAX                  = 2147483647 /* Force 32bit */
} slimMotionModeEnumT;

typedef struct
{
  slimServiceProviderEnumT provider;
  /**< Provider of this data. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  slimTimeSourceEnumT      timeSource;
  /**< Source of timeBase. One of: \n
         eSLIM_TIME_SOURCE_UNSPECIFIED \n
         eSLIM_TIME_SOURCE_COMMON \n
         eSLIM_TIME_SOURCE_GPS \n
         eSLIM_TIME_SOURCE_UTC \n
         eSLIM_TIME_SOURCE_LEGACY   */

  uint64_t                 timestamp;
  /**<   Time stamp of the motion data in this report.
         The time stamp is in the time reference scale that is
         used by the motion data time source. \n
         - Units: Milliseconds */

  uint32_t                 flags;
  /**< SLIM flags. Combination of:  \n
         SLIM_FLAGS_MASK_TIME_JUMP  \n
         SLIM_FLAGS_MASK_TIME_UNC_HIGH */

  slimMotionStateEnumT     motionState;
  /**< Current motion state of the user. \n

       Absolute rest and relative rest are both indicated by setting motion_state
       to Stationary. The relative rest state can be distinguished from absolute
       rest by reducing probabilityOfState.
  */

  slimMotionModeEnumT      motionMode;
  /**< Modes of user motion. \n

       The motionMode value is independent of the motionState value.
  */

  float                    probabilityOfState;
  /**< Probability that the device is actually undergoing the motion state
       specified by the combination of the values of motion_state, motion_mode,
       and motion_sub_mode. \vspace{0.06in}

       This value is a floating point number in the range of 0 to 100, in
       units of percent probability. Any value greater than 99.9999 is
       applied as 99.9999. \vspace{0.06in}

       It is recommended that if a particular combination of motion_state and
       motion_mode cannot be determined with more than 50 percent confidence,
       that a more general statement of user motion be made.
       For example, if the mode of In-Motion + Pedestrian-Running can only be
       determined with 50 percent probability, and the simpler statement of In-Motion
       can be determined with 90 percent probability, it is recommended that this field
       be used to simply state In-Motion with 90 percent probability. \vspace{0.06in}

       If the motion_state is not known, the value in this field is not used.
  */

  uint16_t                 age;
  /**< Age of the motion data in milliseconds at the time of injection.
  */

  uint16_t                 timeout;
  /**< If the age of the motion data input exceeds the timeout value, the data
       will no longer be used. The timeout value is in units of milliseconds.
       Values in the range of 0 to 10000 are accepted. If 65535 is provided,
       the motion data input is applied until the next input is
       received. \vspace{0.06in}

       If the determination of motion data is an instantaneous observation
       and no notice is guaranteed to be given via the QMI on a change in the
       state of the motion data, it is recommended that this field be set to 0. \vspace{0.06in}

       If the determination of motion data is continuously monitored
       external to the QMI and an update is always applied to the QMI upon any
       change in state, a value of 65535 is used for this field.
       Note that in this case, if a certain mode is set and is not later
       unset (e.g., by sending in the request message with a user motion
       state of Unknown), the value is applied indefinitely.
  */

} slimMotionDataStructT;

/*
 *  Vehicle sensors
 */

typedef struct
{
  uint32_t sampleTimeOffset;
  /**<   Sample time offset. This time offset must be
       relative to the sensor time of the first sample. \n
       - Type: Unsigned integer \n
       - Units: microseconds */

  float    sample[3];
  /**<   3-axis Sample
       - Type: Floating point \n
       - Axes: x = Front \n
               y = Right \n
               z = Down \n
       - Units: ACCEL       : m/s2 \n
                GYRO        : rad/s */

} slimVehicleSensorSampleStructT;

/** Maximum number of vehicle sensor samples in one message */
#define SLIM_VEHICLE_SENSOR_MAX_SAMPLE_SETS 65

/** Vehicle sensor types: */
typedef enum
{
  eSLIM_VEHICLE_SENSOR_TYPE_ACCEL    = 0, /**< Accelerometer */
  eSLIM_VEHICLE_SENSOR_TYPE_GYRO     = 1, /**< Gyroscope */

  eSLIM_VEHICLE_SENSOR_TYPE_MAX      = 2147483647 /* Force 32bit */
} slimVehicleSensorTypeEnumT;

/* The vehicle has a right-handed coordinate system, such that:
      X = Front
      Y = Right
      Z = Down

   For instance, turning right produces a positive turn rate on
   Z-axis. */
#define SLIM_MASK_VEHICLE_SENSOR_X_AXIS   0x01
  /**< The x-axis is valid \n */
#define SLIM_MASK_VEHICLE_SENSOR_Y_AXIS   0x02
  /**< The y-axis is valid \n */
#define SLIM_MASK_VEHICLE_SENSOR_Z_AXIS   0x04
  /**< The z-axis is valid. */

typedef struct
{
  slimServiceProviderEnumT       provider;
  /**< Provider of this data. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  slimTimeSourceEnumT            timeSource;
  /**< Source of timeBase. One of: \n
         eSLIM_TIME_SOURCE_UNSPECIFIED \n
         eSLIM_TIME_SOURCE_COMMON \n
         eSLIM_TIME_SOURCE_GPS \n
         eSLIM_TIME_SOURCE_UTC \n
         eSLIM_TIME_SOURCE_LEGACY   */

  uint64_t                       timeBase;
  /**< Base time to which the sample time offsets are added
       to get full sample timestamps. Units: milliseconds */

  uint32_t                       flags;
  /**< SLIM flags. Combination of:  \n
         SLIM_FLAGS_MASK_TIME_JUMP  \n
         SLIM_FLAGS_MASK_TIME_UNC_HIGH */

  slimVehicleSensorTypeEnumT     sensorType;
  /**< Vehicle sensor type. One of: \n
         eSLIM_VEHICLE_SENSOR_TYPE_ACCEL \n
         eSLIM_VEHICLE_SENSOR_TYPE_GYRO */


  uint8_t                        axesValidity;
  /**< Identifies the axes that are valid for all the sensor samples.
       Valid bitmasks: SLIM_MASK_VEHICLE_SENSOR_*_AXIS */

  uint8_t                        samples_len;
  slimVehicleSensorSampleStructT samples[SLIM_VEHICLE_SENSOR_MAX_SAMPLE_SETS];
  /* Variable length array to specify the on-vehicle sensor samples;
     Max Length of array is: 65 */
} slimVehicleSensorDataStructT;

/** Enum for pedestrian alignment algorithm type */
typedef enum
{
  eSLIM_PED_ALIGNMENT_INPUT_TYPE_ROTATION_VECTOR        = 0,
  /**<  9-axis fusion algorithm  */
  eSLIM_PED_ALIGNMENT_INPUT_TYPE_GAME_ROTATION_VECTOR   = 1,
  /**<  6-axis accel + gyro fusion algorithm */
  eSLIM_PED_ALIGNMENT_INPUT_TYPE_ORIENTATION            = 2,
  /**<  6-axis accel + mag fusion algorithm */

  eSLIM_PED_ALIGNMENT_INPUT_TYPE_MAX = 2147483647 /* Force 32bit */
} slimPedAlignmentInputTypeEnumT;

/** Pedestrian alignment input mask can be set to represent one or a group
    of algorithm types.
    The index of each bit corresponds a value in slimPedAlignmentInputTypeEnumT.
    Mask can be easily manipulated using following macros:
    - Set value:              SLIM_MASK_SET
      Example:                SLIM_MASK_SET(mask, eSLIM_PED_ALIGNMENT_INPUT_TYPE_ROTATION_VECTOR);
    - Clear value:            SLIM_MASK_CLEAR
      Example:                SLIM_MASK_CLEAR(mask, eSLIM_PED_ALIGNMENT_INPUT_TYPE_ROTATION_VECTOR);
    - Check if value is set:  SLIM_MASK_IS_SET
      Example:                if (SLIM_MASK_IS_SET(mask, eSLIM_PED_ALIGNMENT_INPUT_TYPE_ROTATION_VECTOR))
                              {
                                ...
                              }
*/
typedef uint32_t slimPedAlignmentInputTypeMaskT;

/** Pedestrian alignment event */
typedef enum
{
  eSLIM_PED_ALIGNMENT_EVENT_NO_EVENT                = 0,
  /**< No events detected since last step  */
  eSLIM_PED_ALIGNMENT_EVENT_TURN_DETECTED           = 1,
  /**< Pedestrian turn detected  */
  eSLIM_PED_ALIGNMENT_EVENT_ALIGNMENT_CHANGED       = 2,
  /**< Device alignment change detected */
  eSLIM_PED_ALIGNMENT_EVENT_TURN_AND_ALIGN_CHANGED  = 3,
  /**< Turn and alignment change detected */
  /* Add new events here. */
  eSLIM_PED_ALIGNMENT_EVENT_COUNT,
  /**< Event count used to validate data */
  eSLIM_PED_ALIGNMENT_EVENT_MAX         = 2147483647 /* Force 32bit */
} slimPedAlignmentEventEnumT;

typedef struct
{
  slimServiceProviderEnumT       provider;
  /**< Provider of this data. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  slimTimeSourceEnumT            timeSource;
  /**< Source of timeBase. One of: \n
         eSLIM_TIME_SOURCE_UNSPECIFIED \n
         eSLIM_TIME_SOURCE_COMMON \n
         eSLIM_TIME_SOURCE_GPS \n
         eSLIM_TIME_SOURCE_UTC \n
         eSLIM_TIME_SOURCE_LEGACY   */

  uint64_t                       timestamp;
  /**<   Time stamp of the pedestrian alignment data in this report.
         The time stamp is in the time reference scale that is
         used by the pedestrian alignment data time source. \n
         - Units: Milliseconds */

  uint32_t                       flags;
  /**< SLIM flags. Combination of:  \n
         SLIM_FLAGS_MASK_TIME_JUMP  \n
         SLIM_FLAGS_MASK_TIME_UNC_HIGH */

  slimPedAlignmentInputTypeEnumT algorithm;
  /**< Pedestrian alignment algorithm type of this sample. One of: \n
         eSLIM_PED_ALIGNMENT_INPUT_TYPE_ROTATION_VECTOR \n
         eSLIM_PED_ALIGNMENT_INPUT_TYPE_GAME_ROTATION_VECTOR \n
         eSLIM_PED_ALIGNMENT_INPUT_TYPE_ORIENTATION */

  float                          cogAngle;
  /**<   Course over ground (CoG) angle, in radians.
         Relative to magnetic north, if magnetometer is available
         Relative to arbitrary azimuth, if magnetometer is not available */

  float                          cogAccuracy;
  /**<   Predicted standard deviation of the computed CoG, in radians.
         Ranges from 0 to pi. Lower values correspond to more reliable
         estimates.
         Value < pi/12: highly reliable CoG estimate.
         Value > pi/3: CoG cannot be trusted. */

  float                          relativeTurnAngle;
  /**<   Estimated relative turn angle since last device alignment change,
         in radians. */

  uint8_t                        relativeTurnAngleAccuracy_valid;
  /**<   Must be set to true if relativeTurnAngleAccuracy is being passed */

  float                          relativeTurnAngleAccuracy;
  /**<   Estimated uncertainty of the relative turn angle since last device
         alignment change, in radians. */

  uint32_t                       alignmentChangeAge;
  /**<   Age of the last detected alignment change event.\n
         - Units: Milliseconds */

  slimPedAlignmentEventEnumT     events;
  /**<   Pedestrian events detected since the last step. */

} slimPedestrianAlignmentDataStructT;

typedef struct
{
  uint8_t                        inputTypeMask_valid;
  /**<   Must be set to true if inputTypeMask is being passed */

  slimPedAlignmentInputTypeMaskT inputTypeMask;
  /**< Mask containing all enabled algorithm types of a successfull enable
       request. */

} slimPedestrianAlignmentEnableResponseStructT;

/** Enum for magnetic field data flags */
typedef enum
{
  eSLIM_MAG_FIELD_FLAG_MAG_PARAM_INVALID        = 0,
  /**<  Magnetic field data is invalid.  */
  eSLIM_MAG_FIELD_FLAG_LONGITUDE_INVALID        = 1,
  /**<  Longitude is invalid. */

  eSLIM_MAG_FIELD_FLAG_MAX = 2147483647 /* Force 32bit */
} slimMagneticFieldDataFlagsEnumT;


/** Magnetic field data flags mask describes the properties of the magnetic
    field data.
    The index of each bit corresponds a value in slimMagneticFieldDataFlagsEnumT.
    Mask can be easily manipulated using following macros:
    - Set value:              SLIM_MASK_SET
      Example:                SLIM_MASK_SET(mask, eSLIM_MAG_FIELD_FLAG_MAG_PARAM_INVALID);
    - Clear value:            SLIM_MASK_CLEAR
      Example:                SLIM_MASK_CLEAR(mask, eSLIM_MAG_FIELD_FLAG_MAG_PARAM_INVALID);
    - Check if value is set:  SLIM_MASK_IS_SET
      Example:                if (SLIM_MASK_IS_SET(mask, eSLIM_MAG_FIELD_FLAG_MAG_PARAM_INVALID))
                              {
                                ...
                              }
*/
typedef uint32_t slimMagneticFieldDataFlagsMaskT;

typedef struct
{
  slimServiceProviderEnumT       provider;
  /**< Provider of this data. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  slimTimeSourceEnumT            timeSource;
  /**< Source of timeBase. One of: \n
         eSLIM_TIME_SOURCE_UNSPECIFIED \n
         eSLIM_TIME_SOURCE_COMMON \n
         eSLIM_TIME_SOURCE_GPS \n
         eSLIM_TIME_SOURCE_UTC \n
         eSLIM_TIME_SOURCE_LEGACY   */

  uint64_t                       timestamp;
  /**<   Time stamp of the magnetic field data in this report.
         The time stamp is in the time reference scale that is
         used by the magnetic field data time source. \n
         - Units: Milliseconds */

  uint32_t                       flags;
  /**< SLIM flags. Combination of:  \n
         SLIM_FLAGS_MASK_TIME_JUMP  \n
         SLIM_FLAGS_MASK_TIME_UNC_HIGH */

  uint16_t                       gpsWeek;
  /**<   GPS week of the Earth magnetic-field data assosiated with the current
         position estimate. */

  uint32_t                       gpsMsec;
  /**<   GPS millisecond counter of the Earth magnetic-field data assosiated
         with the current position estimate. \n
         - Units: Milliseconds */

  slimMagneticFieldDataFlagsMaskT magneticFieldDataFlags;
  /**<   Mask containing magnetic field data flags.*/

  int32_t                        longitude;
  /**<   Approximate degree-level-Longitude.  This should serve as
         an estimate of timezone. \n
         - Units: Degrees \n
         - Range: -180 to 180 \n
         - Positive values indicate eastern longitude \n
         - Negative values indicate western longitude. */

  double                         intensity;
  /**<   Intensity of magnetic field at current location. \n
         - Units: nanotesla (nT) (1 Gauss = 100,000 nT). */

  double                         inclination;
  /**<   The inclination is given by an angle that can assume values between
         -90° (up) to 90° (down). In the northern hemisphere, the field points
         downwards. It is straight down at the North Magnetic Pole and rotates
         upwards as the latitude decreases until it is horizontal (0°) at the
         magnetic equator. \n
         - Units: degrees. */

  double                         declination;
  /**<   Declination is positive for an eastward deviation of the field relative
         to true north. It can be estimated by comparing the magnetic north/south
         heading on a compass with the direction of a celestial pole. Maps typically
         include information on the declination as an angle or a small diagram
         showing the relationship between magnetic north and true north. \n
         - Units: degrees. */
} slimMagneticFieldDataStructT;

/*
 *  Vehicle odometry
 */

/** Maximum number of odometry samples in one message */
#define SLIM_VEHICLE_ODOMETRY_MAX_SAMPLE_SETS 65

/** Vehicle odometry flags: */
#define SLIM_MASK_VEHICLE_ODOMETRY_REVERSE_MOVEMENT     0x01
  /**< Odometry data includes at least some data where
       the vehicle may have been moving in the reverse direction.
       If odometry data may be in reverse, this bit must be set.
       If odometry data is all in the forward direction, this bit
       must not be set. */
#define SLIM_MASK_VEHICLE_ODOMETRY_AFFECTED_BY_ERRORS   0x02
  /**< Odometry data includes at least some data affected
       by a major error source affecting distance-traveled accuracy,
       such as wheel slippage due to skidding, gravel, snow, or ice as
       detected by the vehicle, e.g., via an ABS or other system. */

#define SLIM_MASK_VEHICLE_ODOMETRY_ABSOLUTE_MEASUREMENT 0x04
  /**< Odometry data is an absolute amount since the vehicle
       began service, and is the same vehicle that is regularly used with
       this device (so that the offset of this value, since the last time
       this measurement was used by the location engine, can be safely used
       as a likely correct estimate of the distance traveled since last
       use). */

/** Vehicle odometry wheel flags: */
#define SLIM_MASK_VEHICLE_ODOMETRY_LEFT_AND_RIGHT_AVERAGE  0x01
  /**< Average of left and right non-turning wheels */
#define SLIM_MASK_VEHICLE_ODOMETRY_LEFT                    0x02
  /**< Left side, non-turning wheel */
#define SLIM_MASK_VEHICLE_ODOMETRY_RIGHT                   0x04
  /**< Right side, non-turning wheel */

/**< Delineates for which wheels the measurements are being provided
     in the following samples, where one or more of the following bits
     must be set, and data samples aligned with these axes must appear
     in groups, in this order.

     Note: At least one bit must be set. */

typedef struct
{
  uint32_t sampleTimeOffset;
  /**<   Sample time offset. This time offset must be
       relative to the sensor time of the first sample. \n
       - Type: Unsigned integer \n
       - Units: microseconds */

  uint32_t distanceTravelled[3];
  /**<  Distance traveled (odometry) sample offset. \n
        - Type: Unsigned integer                      \n
        - Units of accumulated distance: Millimeters  \n
        - Range: Over 4000 km

        \vspace{3pt}
        This measurement (in units of millimeters) is added to
        the distance_travelled_base measurement (in meters) to
        get the total distance traveled sample value.

        \vspace{3pt}
        Note: The order of measurements must be: \n
              - Average of left and right  \n
              - Left                       \n
              - Right

   */

} slimVehicleOdometrySampleStructT;

typedef struct
{
  slimServiceProviderEnumT              provider;
  /**< Provider of this data. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  slimTimeSourceEnumT                   timeSource;
  /**< Source of timeBase. One of: \n
         eSLIM_TIME_SOURCE_UNSPECIFIED \n
         eSLIM_TIME_SOURCE_COMMON \n
         eSLIM_TIME_SOURCE_GPS \n
         eSLIM_TIME_SOURCE_UTC \n
         eSLIM_TIME_SOURCE_LEGACY   */

  uint64_t                              timeBase;
  /**< Base time to which the sample time offsets are added
       to get full sample timestamps. Units: milliseconds */

  uint32_t                              flags;
  /**< SLIM flags. Combination of:  \n
         SLIM_FLAGS_MASK_TIME_JUMP  \n
         SLIM_FLAGS_MASK_TIME_UNC_HIGH */

  uint32_t                              odometryFlags;
  /**< Flags to indicate any deviation from the default measurement
       assumptions. Combination of: \n
         SLIM_MASK_VEHICLE_ODOMETRY_REVERSE_MOVEMENT \n
         SLIM_MASK_VEHICLE_ODOMETRY_AFFECTED_BY_ERRORS \n
         SLIM_MASK_VEHICLE_ODOMETRY_ABSOLUTE_MEASUREMENT */

  uint32_t                              wheelFlags;
  /**< Delineates for which wheels the measurements are provided
       in the samples, where one or more of the following
       bits must be set. Combination of: \n
         SLIM_MASK_VEHICLE_ODOMETRY_LEFT_AND_RIGHT_AVERAGE \n
         SLIM_MASK_VEHICLE_ODOMETRY_LEFT \n
         SLIM_MASK_VEHICLE_ODOMETRY_RIGHT */

  uint32_t                              distanceTravelledBase;
  /**< Distance traveled base. \n
        - Type: Unsigned integer                 \n
        - Units of accumulated distance: Meters  \n
        - Range: Over 4,000,0000 km

        \vspace{3pt}
        Distance traveled (odometry) is to be reported in a continuously
        accumulating way from device power-up. It may be an incremental
        distance starting at zero, or another arbitrary point from device
        power-up, or the absolute distance traveled by the vehicle
        (and if so, set SLIM_MASK_VEHICLE_ODOMETRY_ABSOLUTE_MEASUREMENT),
        as long as the distance grows incrementally from device power-up.

        \vspace{3pt}
        This distanceTravelledBase is added to the distranceTravelled offset
        of each sample to get the absolute distance of each sample
        point.

        \vspace{3pt}
        Distance traveled errors are expected to be primarily due to
        scale-factor, with some allowance for "noise" due to minor slippage
        events, e.g., gravel. Major wheel slippage events that affect
        odometry, must be flagged; see the flags field.

        Note: Other events, such as a vehicle traveling in reverse, may
        also affect the available accuracy of this information, and notification
        of those events must be provided; see the flags field. */

  uint8_t                          samples_len;
  slimVehicleOdometrySampleStructT samples[SLIM_VEHICLE_ODOMETRY_MAX_SAMPLE_SETS];

} slimVehicleOdometryDataStructT;

typedef enum
{
  eSLIM_DISTANCE_BOUND_BREACH_EVENT_UNKNOWN    = 0,
  /**< Unknown event */
  eSLIM_DISTANCE_BOUND_BREACH_EVENT_FALSE      = 1,
  /**< False event */
  eSLIM_DISTANCE_BOUND_BREACH_EVENT_TRUE       = 2,
  /**< True event */

  eSLIM_DISTANCE_BOUND_BREACH_EVENT_MAX        = 2147483647 /* Force 32bit */
} slimDistanceBoundBreachEventEnumT;

typedef struct
{
  slimDistanceBoundBreachEventEnumT breachEvent;
  /**< Breach event. */

  uint8_t sessionKey;
  /**< The unique session key provided by client associated with this report. */

} slimDistanceBoundBreachEventDataStructT;

typedef struct
{
  slimServiceProviderEnumT provider;
  /**< Provider of this data. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  slimTimeSourceEnumT      timeSource;
  /**< Source of timestamp. One of: \n
         eSLIM_TIME_SOURCE_UNSPECIFIED \n
         eSLIM_TIME_SOURCE_COMMON \n
         eSLIM_TIME_SOURCE_GPS \n
         eSLIM_TIME_SOURCE_UTC \n
         eSLIM_TIME_SOURCE_LEGACY   */

  uint64_t                 timestamp;
  /**< Timestamp of input used to generate the algorithm output.
        - Type: Unsigned integer \n
        - Units: Milliseconds */

  uint32_t                 flags;
  /**< SLIM flags. Combination of:  \n
         SLIM_FLAGS_MASK_TIME_JUMP  \n
         SLIM_FLAGS_MASK_TIME_UNC_HIGH */

  slimDistanceBoundBreachEventDataStructT breachEventData;
  /**< Breach event data. */

  uint8_t distanceAccumulated_valid;
  /**< Must be set to true if distanceAccumulated is being passed */

  float distanceAccumulated;
  /**< Distance accumulated in meters since the last set bound request */

} slimDistanceBoundDataStructT;

typedef struct
{
  slimServiceProviderEnumT provider;
  /**< Provider of this data. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  slimTimeSourceEnumT      timeSource;
  /**< Source of timestamp. One of: \n
         eSLIM_TIME_SOURCE_UNSPECIFIED \n
         eSLIM_TIME_SOURCE_COMMON \n
         eSLIM_TIME_SOURCE_GPS \n
         eSLIM_TIME_SOURCE_UTC \n
         eSLIM_TIME_SOURCE_LEGACY   */

  uint8_t                  timestamp_valid;
  /**< Must be set to true if timestamp is being passed */

  uint64_t                 timestamp;
  /**< Timestamp of input used to generate the algorithm output.
        - Type: Unsigned integer \n
        - Units: Milliseconds */

  uint32_t                 flags;
  /**< SLIM flags. Combination of:  \n
         SLIM_FLAGS_MASK_TIME_JUMP  \n
         SLIM_FLAGS_MASK_TIME_UNC_HIGH */

  uint8_t                  breachEventData_valid;
  /**< Must be set to true if breachEventData is being passed */

  slimDistanceBoundBreachEventDataStructT breachEventData;
  /**< Breach event data. */

  uint8_t                  distanceAccumulated_valid;
  /**< Must be set to true if distanceAccumulated is being passed */

  float                    distanceAccumulated;
  /**< Distance accumulated in meters since the last set bound request */

} slimDistanceBoundGetReportResponseStructT;


/** Enum for SLIM service ids */
typedef enum
{
  eSLIM_SERVICE_NONE               = -1,
  /**< Service not defined */

  eSLIM_SERVICE_SENSOR_ACCEL       = 0,
  /**< Accelerometer sensor data service */
  eSLIM_SERVICE_SENSOR_ACCEL_TEMP  = 1,
  /**< Accelerometer temperature sensor data service */
  eSLIM_SERVICE_SENSOR_GYRO        = 2,
  /**< Gyro sensor data service */
  eSLIM_SERVICE_SENSOR_GYRO_TEMP   = 3,
  /**< Gyro temperature sensor data service */

  /* Derived data */
  eSLIM_SERVICE_AMD                = 4,
  /**< Absolute Motion Data service */
  eSLIM_SERVICE_RMD                = 5,
  /**< Relative Motion Data service */
  eSLIM_SERVICE_SMD                = 6,
  /**< Significant Motion Data service */
  eSLIM_SERVICE_MOTION_DATA        = 7,
  /**< Motion Data service */
  eSLIM_SERVICE_PEDOMETER          = 8,
  /**< Pedometer service */
  eSLIM_SERVICE_DISTANCE_BOUND     = 9,
  /**< Distance Bound (CMC-based) Data service */

  /* Vehicle data */
  eSLIM_SERVICE_VEHICLE_ACCEL      = 10,
  /**< Vehicle accelerometer data service */
  eSLIM_SERVICE_VEHICLE_GYRO       = 11,
  /**< Vehicle angular rate data service */
  eSLIM_SERVICE_VEHICLE_ODOMETRY   = 12,
  /**< Vehicle odometry data service */

  eSLIM_SERVICE_SENSOR_MAG_CALIB   = 13,
  /**< Calibrated magnetometer sensor data service */
  eSLIM_SERVICE_SENSOR_MAG_UNCALIB = 14,
  /**< Uncalibrated magnetometer sensor data service */

  eSLIM_SERVICE_PED_ALIGNMENT      = 15,
  /**< Pedestrian alignment data service */

  eSLIM_SERVICE_SENSOR_BARO        = 16,
  /**< Barometer sensor data service */

  eSLIM_SERVICE_MAG_FIELD_DATA     = 17,
  /**< Earth's magnetic field data (inclination/declination) service */

  eSLIM_SERVICE_LAST = eSLIM_SERVICE_MAG_FIELD_DATA,
  /**< Last SLIM service. Note that slimAvailableServiceMaskT type supports
       currently 32 services so if the service count exceeds that count,
       also size of the slimAvailableServiceMaskT type should be modified.
   */

  eSLIM_SERVICE_MAX = 2147483647 /* Force 32bit */
} slimServiceEnumT;

/** Service mask can be set to represent one or a group of services.
    The index of each bit corresponds a value in slimServiceEnumT.
    Mask can be easily manipulated using following macros:
    - Set value:              SLIM_MASK_SET
      Example:                SLIM_MASK_SET(mask, eSLIM_SERVICE_SENSOR_ACCEL);
    - Clear value:            SLIM_MASK_CLEAR
      Example:                SLIM_MASK_CLEAR(mask, eSLIM_SERVICE_SENSOR_ACCEL);
    - Check if value is set:  SLIM_MASK_IS_SET
      Example:                if (SLIM_MASK_IS_SET(mask, eSLIM_SERVICE_SENSOR_ACCEL))
                              {
                                ...
                              }
*/
typedef uint32_t slimAvailableServiceMaskT;


/** Enum for SLIM service status events.*/
typedef enum
{
  eSLIM_SERVICE_STATUS_NONE,
  /**< Event is not service specific. */
  eSLIM_SERVICE_STATUS_AVAILABLE,
  /**< SLIM service is available */
  eSLIM_SERVICE_STATUS_UNAVAILABLE,
  /**< SLIM service is unavailable */
  eSLIM_SERVICE_STATUS_PROVIDERS_CHANGED,
  /**< Available providers for the service changed */

  eSLIM_SERVICE_STATUS_MAX = 2147483647 /* Force 32bit */
} slimServiceStatusEnumT;

/*! Struct for service status event indication */
typedef struct
{
  slimServiceEnumT       service;
  /**< Updated service that caused this event. */
  slimServiceStatusEnumT serviceStatus;
  /**< Service status event type. */
  slimServiceProviderMaskT providerFlags;
  /**< Available providers for the service that caused this event. */

  slimAvailableServiceMaskT availableServicesMask;
  /**< Mask containing all currently available SLIM services.
       @see slim_ServiceAvailable
   */
} slimServiceStatusEventStructT;

/** Enum for SLIM message types */
typedef enum
{
  eSLIM_MESSAGE_TYPE_INDICATION = 0,
   /**< Message is unsolicited indication */
  eSLIM_MESSAGE_TYPE_RESPONSE = 1,
     /**< Message is a response to a request */
  eSLIM_MESSAGE_TYPE_MAX = 2147483647 /* Force 32bit */
} slimMessageTypeEnumT;

/** Enum for SLIM message ids. Payload is valid only with specific message id
    only if the error code is eSLIM_SUCCESS in the message header. */
typedef enum
{
  /* SLIM service related messages */
  eSLIM_MESSAGE_ID_NONE = 0,
  /**< no payload */

  /* Response to client open request */
  eSLIM_MESSAGE_ID_OPEN_RESP,
  /**< no payload */

  /* Service status indication */
  eSLIM_MESSAGE_ID_SERVICE_STATUS_IND,
  /**< slimServiceStatusEventStructT */


  /* Sensor data messages */
  eSLIM_MESSAGE_ID_SENSOR_DATA_IND = SLIM_FIRST_SERVICE_MSG_ID,
  /**< slimSensorDataStructT */
  eSLIM_MESSAGE_ID_SENSOR_DATA_ENABLE_RESP,
  /**< no payload */

  /* Motion data messages */
  eSLIM_MESSAGE_ID_MOTION_DATA_IND,
  /**< slimMotionDataStructT */
  eSLIM_MESSAGE_ID_MOTION_DATA_ENABLE_RESP,
  /**< no payload */

  /* Pedometer messages */
  eSLIM_MESSAGE_ID_PEDOMETER_IND,
  /**< slimPedometerDataStructT */
  eSLIM_MESSAGE_ID_PEDOMETER_ENABLE_RESP,
  /**< no payload */

  /* QMD data messages */
  eSLIM_MESSAGE_ID_QMD_DATA_IND,
  /**< slimQmdDataStructT */
  eSLIM_MESSAGE_ID_QMD_DATA_ENABLE_RESP,
  /**< no payload */

  /* SMD data messages */
  eSLIM_MESSAGE_ID_SMD_DATA_IND,
  /**< slimSmdDataStructT */
  eSLIM_MESSAGE_ID_SMD_DATA_ENABLE_RESP,
  /**< no payload */

  /* Distance Bound messages */
  eSLIM_MESSAGE_ID_DISTANCE_BOUND_DATA_IND,
  /**< slimDistanceBoundDataStructT */
  eSLIM_MESSAGE_ID_DISTANCE_BOUND_DATA_ENABLE_RESP,
  /**< no payload */
  eSLIM_MESSAGE_ID_DISTANCE_BOUND_DATA_SET_RESP,
  /**< no payload */
  eSLIM_MESSAGE_ID_DISTANCE_BOUND_DATA_GET_RESP,
  /**< slimDistanceBoundGetReportResponseStructT */

  /* Vehicle data messages */
  eSLIM_MESSAGE_ID_VEHICLE_SENSOR_IND,
  /**< slimVehicleSensorDataStructT */
  eSLIM_MESSAGE_ID_VEHICLE_ODOMETRY_IND,
  /**< slimVehicleOdometryDataStructT */
  eSLIM_MESSAGE_ID_VEHICLE_ENABLE_RESP,
  /**< no payload */

  /* Pedestrian alignment data messages */
  eSLIM_MESSAGE_ID_PED_ALIGNMENT_IND,
  /**< slimPedestrianAlignmentDataStructT */
  eSLIM_MESSAGE_ID_PED_ALIGNMENT_ENABLE_RESP,
  /**< slimPedestrianAlignmentEnableResponseStructT */

  /* Magnetic field data messages */
  eSLIM_MESSAGE_ID_MAG_FIELD_DATA_IND,
  /**< slimMagneticFieldDataStructT */
  eSLIM_MESSAGE_ID_MAG_FIELD_DATA_ENABLE_RESP,
  /**< no payload */


  eSLIM_MESSAGE_ID_MAX_VALUE = SLIM_LAST_MSG_ID_VALUE,
  /**< We assume that SLIM message ids will always fit to uint16 range.
       Add new values before this placeholder. */

  eSLIM_MESSAGE_ID_MAX = 2147483647 /* Force 32bit */
} slimMessageIdEnumT;

/** Enum for SLIM error codes */
typedef enum
{
  eSLIM_SUCCESS                        = 0,
  /**< No error - success */
  eSLIM_ERROR_CLIENT_HANDLE_INVALID    = -1,
  /**< Invalid client handle */
  eSLIM_ERROR_UNSUPPORTED              = -2,
  /**< Operation is not supported */
  eSLIM_ERROR_IPC_COMMUNICATION        = -3,
  /**< Failure with IPC communication */
  eSLIM_ERROR_INVALID_CALLBACK         = -4,
  /**< Client callback is invalid. */
  eSLIM_ERROR_INTERNAL                 = -5,
  /**< Internal SLIM error */
  eSLIM_ERROR_MAX_TXN_COUNT_REACHED    = -6,
  /**< Maximum transaction count exceeded.*/
  eSLIM_ERROR_BAD_ARGUMENT             = -7,
  /**< Client provided bad argument. */
  eSLIM_ERROR_QMI_INTERFACE            = -8,
  /**< Error happened in QMI-communication. */
  eSLIM_ERROR_ALLOCATION               = -9,
  /**< Error happened in memory allocation. */
  eSLIM_ERROR_SERVICE_RESERVED         = -10,
  /**< Service connection is already reserved for another client.*/
  eSLIM_ERROR_SERVICE_NOT_ENABLED      = -11,
  /**< Service is not enabled.*/
  eSLIM_ERROR_MAX_CLIENT_COUNT_REACHED = -12,
  /**< Maximum client count exceeded.*/
  eSLIM_ERROR_MIN                      = -2147483647 /* Force 32bit */
} slimErrorEnumT;

/*! Client handle type to be used with SLIM communication */
typedef const void *slimClientHandleT;

/** Enum for SLIM open flags */
typedef enum
{
  eSLIM_OPEN_FLAGS_NO_TIME_CONVERSION = 0,
  /**< Timestamps of the samples are not converted. Sample timestamps are in
       provider time scale. */

  eSLIM_OPEN_FLAGS_MAX = 2147483647 /* Force 32bit */
} slimOpenFlagsEnumT;

/** Flags given when opening SLIM client connection.
    The index of each bit corresponds a value in slimOpenFlagsEnumT.
    Mask can be easily manipulated using following macros:
    - Set value:              SLIM_MASK_SET
      Example:                SLIM_MASK_SET(mask, eSLIM_OPEN_FLAGS_NO_TIME_CONVERSION);
    - Clear value:            SLIM_MASK_CLEAR
      Example:                SLIM_MASK_CLEAR(mask, eSLIM_OPEN_FLAGS_NO_TIME_CONVERSION);
    - Check if value is set:  SLIM_MASK_IS_SET
      Example:                if (SLIM_MASK_IS_SET(mask, eSLIM_OPEN_FLAGS_NO_TIME_CONVERSION))
                              {
                                ...
                              }
*/
typedef uint32_t slimOpenFlagsT;

/*! Struct for SLIM message header */
typedef struct
{
  slimServiceEnumT service;
  /**< SLIM service type */

  slimMessageTypeEnumT msgType;
  /**< SLIM message type */

  slimErrorEnumT msgError;
  /**< SLIM message error.
       Do not handle message payload if value is not eSLIM_SUCCESS.
   */

  slimMessageIdEnumT msgId;
  /**< SLIM message id */

  uint16_t size;
  /**< SLIM message size */

  uint8_t txnId;
  /**< Transaction id provided by client */

} slimMessageHeaderStructT;

/*! Client callback function type */
typedef void ( *slimNotifyCallbackFunctionT )
(
  uint64_t callbackData,
  const slimMessageHeaderStructT *messageHeader,
  void *message
);

/*! Struct for SLIM service enable/disable data */
typedef struct
{
  uint8_t enable;
  /**< TRUE if reporting should be enabled.
       FALSE if reporting should be disabled.
   */

  slimServiceProviderMaskT providerFlags;
  /**< SLIM shall use only the providers defined in the given mask for
       the service to enable.
       If the value is 0, SLIM shall use the default provider for the service.
   */

} slimEnableServiceDataStructT;

/*! Struct for sensor data reporting configuration parameters */
typedef struct
{
  slimEnableServiceDataStructT enableConf;
  /**< Configuration for service enabling/disabling. */

  slimServiceEnumT sensor;
  /**< Sensor service to enable/disable. Can be one of the following services:
       - eSLIM_SERVICE_SENSOR_ACCEL
       - eSLIM_SERVICE_SENSOR_ACCEL_TEMP
       - eSLIM_SERVICE_SENSOR_GYRO
       - eSLIM_SERVICE_SENSOR_GYRO_TEMP
       - eSLIM_SERVICE_SENSOR_MAG_CALIB
       - eSLIM_SERVICE_SENSOR_MAG_UNCALIB
       - eSLIM_SERVICE_SENSOR_BARO
   */

  uint16_t sampleCount;
  /**< Samples per batch (when enabling reporting).
       NOTE: Maximum sample count per batch is 50.
   */

  uint16_t reportRate;
  /**< Batches per second (when enabling reporting) */

} slimEnableSensorDataRequestStructT;

/*! Struct for motion data reporting configuration parameters */
typedef struct
{

  slimEnableServiceDataStructT enableConf;
  /**< Configuration for service enabling/disabling. */

} slimEnableMotionDataRequestStructT;

/*! Struct for pedometer reporting configuration parameters*/
typedef struct
{

  slimEnableServiceDataStructT enableConf;
  /**< Configuration for service enabling/disabling. */

  uint8_t  resetStepCount_valid;
  /**< Must be set to TRUE if resetStepCount is being passed */

  uint8_t  resetStepCount;
  /**< Indicates whether the service should reset the step count.
       This value is used only if reporting is already enabled.
   */

  uint8_t  stepCountThreshold_valid;
  /**< Must be set to TRUE if stepCountThreshold is being passed */

  uint32_t stepCountThreshold;
  /**< Specifies the number of steps to be sampled in a pedometer report.
       If the threshold is set to 0, every step event is reported.
   */

  uint32_t reportPeriod;
  /**< Specifies interval for periodic reporting period in seconds, Q16.
       P = 0 for asynchronous reporting.
       P > 0 for periodic reporting. Maximum reporting period is 3600 seconds.
   */

  uint8_t  sampleRate_valid;
  /**< Must be set to TRUE if sampleRate is being passed */

  uint32_t sampleRate;
  /**< Sampling rate in Hz, Q16. Default value of 20 Hz. Requires S >= 20 Hz. */

  uint8_t  dutyCycleOnPercent_valid;
  /**< Must be set to TRUE if dutyCycleOnPercent is being passed */

  uint8_t  dutyCycleOnPercent;
  /**< Duty cycle ON percentage (0 to 100) for pedometer service when it is
       duty-cycled. Sensor stream to pedometer service is active only during
       the duty cycle ON period.
   */

} slimEnablePedometerRequestStructT;

/*! Struct for AMD/RMD configuration */
typedef struct
{

  uint32_t varThres;
  /**< Accel Variance thresold, we declare motion if the sum of variances
       on all axes exceed this , Q16, in m^2/s^4.
   */

  uint32_t varWinLen;
  /**< Time period over which the variance is computed, in Q16 seconds */

} slimQmdDataConfigStructT;

/*! Struct for AMD/RMD data reporting configuration parameters */
typedef struct
{
  slimEnableServiceDataStructT enableConf;
  /**< Configuration for service enabling/disabling. */

  slimServiceEnumT service;
  /**< eSLIM_SERVICE_AMD or eSLIM_SERVICE_RMD */

  uint32_t reportPeriod;
  /**< Unit of seconds, Q16; value of 0 means reporting on new event only .*/

  uint8_t config_valid;
  /**< TRUE if configuration contains valid data */

  slimQmdDataConfigStructT config;
  /**< Configuration options for QMD */

} slimEnableQmdDataRequestStructT;

/*! Struct for SMD data reporting configuration parameters*/
typedef struct
{

  slimEnableServiceDataStructT enableConf;
  /**< Configuration for service enabling/disabling. */

} slimEnableSmdDataRequestStructT;

/*! Struct for Distance Bound data reporting configuration parameters*/
typedef struct
{

  slimEnableServiceDataStructT enableConf;
  /**< Configuration for service enabling/disabling. */

} slimEnableDistanceBoundRequestStructT;

/*! Struct for Distance Bound set request parameters*/
typedef struct
{
  float distanceBound;
  /**< The distance bound in meters. When the distance travelled exceeds this,
       an event is sent to the requestor, and the distance travelled
       accumulation is reset.
   */

  uint8_t sessionKey;
  /**< A unique key provided by the client that identifies the session
       associated with the set bound request. Session key is provided in
       the distance bound data indication to help the client to identify
       the session.
   */

} slimSetDistanceBoundRequestStructT;

/*! Struct for vehicle data reporting configuration parameters */
typedef struct
{

  slimEnableServiceDataStructT enableConf;
  /**< Configuration for service enabling/disabling. */

  slimServiceEnumT service;
  /**< Vehicle data service to enable/disable.
       Can be one of the following services:
       - eSLIM_SERVICE_VEHICLE_ACCEL
       - eSLIM_SERVICE_VEHICLE_GYRO
       - eSLIM_SERVICE_VEHICLE_ODOMETRY
   */

} slimEnableVehicleDataRequestStructT;

/*! Struct for pedestrian alignment data reporting configuration parameters */
typedef struct
{

  slimEnableServiceDataStructT enableConf;
  /**< Configuration for service enabling/disabling. */

  slimPedAlignmentInputTypeMaskT inputTypeMask;
  /**< SLIM shall try to use all algorithm types defined in the given mask for
       the pedestrian alignment service.
   */

  uint8_t sampleRate_valid;
  /**<  Must be set to true if sampleRate is being passed */
  uint32_t sampleRate;
  /**<  Sampling rate in Hz, Q16. Can range from 20-50Hz. Defaults to 20 Hz.
   */

} slimEnablePedestrianAlignmentRequestStructT;

/*! Struct for magnetic-field data reporting configuration parameters */
typedef struct
{

  slimEnableServiceDataStructT enableConf;
  /**< Configuration for service enabling/disabling. */

} slimEnableMagneticFieldDataRequestStructT;
#endif /* SLIM_CLIENT_TYPES_H */
