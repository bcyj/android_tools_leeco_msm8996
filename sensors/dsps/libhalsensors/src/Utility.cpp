/*============================================================================
  @file Utility.cpp

  @brief
  Utility class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Resereted.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <errno.h>
#include <time.h>
#include <stdlib.h>
extern "C" {
#include "sensor_reg.h"
}
#include "sns_reg_api_v02.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "Utility.h"
#include "Sensor.h"
#include "SensorsContext.h"

hal_sensor_dataq_t* Utility::q_head_ptr = NULL;
hal_sensor_dataq_t* Utility::q_tail_ptr = NULL;
hal_data_cb_t Utility::data_cb = {
                         PTHREAD_MUTEX_INITIALIZER,
                         PTHREAD_COND_INITIALIZER,
                         false, false};

hal_data_cb_t* Utility::getDataCb()
{
    return &data_cb;
}

/*============================================================================
  FUNCTION Utility::SensorTypeToSensorString
    Return the SensorType in String
    Parameters:
    @SensorType :SensorType in Int
============================================================================*/
const char * Utility::SensorTypeToSensorString(int SensorType)
{
    switch(SensorType){
        case SENSOR_TYPE_ACCELEROMETER:
            return SENSOR_STRING_TYPE_ACCELEROMETER;
        case SENSOR_TYPE_GEOMAGNETIC_FIELD:
            return SENSOR_STRING_TYPE_MAGNETIC_FIELD;
        case SENSOR_TYPE_ORIENTATION:
            return SENSOR_STRING_TYPE_ORIENTATION;
        case SENSOR_TYPE_GYROSCOPE:
            return SENSOR_STRING_TYPE_GYROSCOPE;
        case SENSOR_TYPE_LIGHT:
            return SENSOR_STRING_TYPE_LIGHT;
        case SENSOR_TYPE_PRESSURE:
            return SENSOR_STRING_TYPE_PRESSURE;
        case SENSOR_TYPE_TEMPERATURE:
            return SENSOR_STRING_TYPE_TEMPERATURE;
        case SENSOR_TYPE_PROXIMITY:
            return SENSOR_STRING_TYPE_PROXIMITY;
        case SENSOR_TYPE_GRAVITY:
            return SENSOR_STRING_TYPE_GRAVITY;
        case SENSOR_TYPE_LINEAR_ACCELERATION:
            return SENSOR_STRING_TYPE_LINEAR_ACCELERATION;
        case SENSOR_TYPE_ROTATION_VECTOR:
            return SENSOR_STRING_TYPE_ROTATION_VECTOR;
        case SENSOR_TYPE_RELATIVE_HUMIDITY:
            return SENSOR_STRING_TYPE_RELATIVE_HUMIDITY;
        case SENSOR_TYPE_AMBIENT_TEMPERATURE:
            return SENSOR_STRING_TYPE_AMBIENT_TEMPERATURE;
        case SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED:
            return SENSOR_STRING_TYPE_MAGNETIC_FIELD_UNCALIBRATED;
        case SENSOR_TYPE_GAME_ROTATION_VECTOR:
            return SENSOR_STRING_TYPE_GAME_ROTATION_VECTOR;
        case SENSOR_TYPE_GYROSCOPE_UNCALIBRATED:
            return SENSOR_STRING_TYPE_GYROSCOPE_UNCALIBRATED;
        case SENSOR_TYPE_SIGNIFICANT_MOTION:
            return SENSOR_STRING_TYPE_SIGNIFICANT_MOTION;
        case SENSOR_TYPE_STEP_DETECTOR:
            return SENSOR_STRING_TYPE_STEP_DETECTOR;
        case SENSOR_TYPE_STEP_COUNTER:
            return SENSOR_STRING_TYPE_STEP_COUNTER;
        case SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR:
            return SENSOR_STRING_TYPE_GEOMAGNETIC_ROTATION_VECTOR;
        case SENSOR_TYPE_HEART_RATE:
            return SENSOR_STRING_TYPE_HEART_RATE;
        case SENSOR_TYPE_MOTION_ABSOLUTE:
            return SENSOR_STRING_TYPE_MOTION_ABSOLUTE;
#ifdef FEATURE_SNS_HAL_SAM_INT
        case SENSOR_TYPE_GESTURE_BRING_TO_EAR:
            return SENSOR_STRING_TYPE_GESTURE_BRING_TO_EAR;
        case SENSOR_TYPE_GESTURE_FACE_N_SHAKE:
            return SENSOR_STRING_TYPE_GESTURE_FACE_N_SHAKE;
#endif /* FEATURE_SNS_HAL_SAM_INT */
        case SENSOR_TYPE_CMC:
            return SENSOR_STRING_TYPE_CMC;
        case SENSOR_TYPE_FACING:
            return SENSOR_STRING_TYPE_FACING;
        case SENSOR_TYPE_BASIC_GESTURES:
            return SENSOR_STRING_TYPE_BASIC_GESTURES;
        case SENSOR_TYPE_TAP:
            return SENSOR_STRING_TYPE_TAP;
        case SENSOR_TYPE_HALL_EFFECT:
            return SENSOR_STRING_TYPE_HALL_EFFECT;
        case SENSOR_TYPE_IR_GESTURE:
            return SENSOR_STRING_TYPE_IR_GESTURE;
        case SENSOR_TYPE_SCREEN_ORIENTATION:
            return SENSOR_STRING_TYPE_SCREEN_ORIENTATION;
        case SENSOR_TYPE_PAM:
            return SENSOR_STRING_TYPE_PAM;
        case SENSOR_TYPE_PEDOMETER:
            return SENSOR_STRING_TYPE_PEDOMETER;
        case SENSOR_TYPE_RGB:
            return SENSOR_STRING_TYPE_RGB;
        case SENSOR_TYPE_MOTION_RELATIVE:
            return SENSOR_STRING_TYPE_MOTION_RELATIVE;
        case SENSOR_TYPE_SAR:
            return SENSOR_STRING_TYPE_SAR;
        case SENSOR_TYPE_TILT:
            return SENSOR_STRING_TYPE_TILT;
        case SENSOR_TYPE_MOTION_VEHICLE:
            return SENSOR_STRING_TYPE_MOTION_VEHICLE;
        default:
            return "";
    }
}

/*============================================================================
  FUNCTION waitForResponse
    Calculate the sensor sample rate
    Parameters:
    @timeout : the pthread wait time out time, 0 means wait forever
    @cb_mutex_ptr : the callback metux to protect the cond
    @cond_ptr : the callback cond
    @cond_var : the variable used to indicated the response arrived
============================================================================*/
bool Utility::waitForResponse(int timeout,
                        pthread_mutex_t *cb_mutex_ptr,
                        pthread_cond_t *cond_ptr,
                        bool *cond_var)
{
    bool ret = false;            /* the return value of this function */
    int rc = 0;                  /* return code from pthread calls */
    struct timeval present_time;
    struct timespec expire_time;

    HAL_LOG_DEBUG("%s: timeout=%d", __FUNCTION__, timeout);

    /* special case where callback is issued before the main function
       can wait on cond */
    if (*cond_var == true) {
        HAL_LOG_DEBUG("%s: cb has arrived without waiting", __FUNCTION__);
        ret = true;
    }
    else {
        /* Calculate absolute expire time */
        gettimeofday(&present_time, NULL);
        /* Convert from timeval to timespec */
        expire_time.tv_sec  = present_time.tv_sec;
        expire_time.tv_nsec = present_time.tv_usec * 1000;
        expire_time.tv_sec += timeout / 1000;
        /* calculate carry over */
        if ((present_time.tv_usec + (timeout % 1000) * 1000) >= 1000000) {
            expire_time.tv_sec += 1;
        }
        expire_time.tv_nsec =
                (expire_time.tv_nsec + (timeout % 1000) * 1000000) % 1000000000;

        while (*cond_var != true && rc != ETIMEDOUT) {
            if(0 == timeout) {
                rc = pthread_cond_wait(cond_ptr, cb_mutex_ptr);
            }
            else {
                /* Wait for the callback until timeout expires */
                rc = pthread_cond_timedwait(cond_ptr, cb_mutex_ptr,
                                            &expire_time);
            }
            if(0 != rc) {
                HAL_LOG_ERROR("%s: pthread_cond_timedwait() rc=%d (cond: %i)",
                                __FUNCTION__, rc, *cond_var);
            }
        }
        ret = ( rc == 0 || *cond_var ) ? true : false;
    }

    *cond_var = false;
    return ret;
}

/*============================================================================
  FUNCTION isMagAvailable
    Check the magnetic sensor is available or not
============================================================================*/
bool Utility::isMagAvailable()
{
    SensorsContext *context = SensorsContext::getInstance();
    return((context->getSensor(HANDLE_MAGNETIC_FIELD) != NULL) ? true : false);
}

/*============================================================================
  FUNCTION isGyroAvailable
    Check the Gyroscope sensor is available or not
============================================================================*/
bool Utility::isGyroAvailable()
{
    SensorsContext *context = SensorsContext::getInstance();
    return((context->getSensor(HANDLE_GYRO) != NULL) ? true : false);
}

/*===========================================================================
  FUNCTION:  QuatToRotMat
  Convert quaternion to rotation matrix

    Quaternion
                Q = |X Y Z W|

    Rotation Matrix
                /  R[ 0]   R[ 1]   R[ 2]   0  \
                |  R[ 4]   R[ 5]   R[ 6]   0  |
                |  R[ 8]   R[ 9]   R[10]   0  |
                \  0       0       0       1  /

   M = 1- 2(Y*Y + Z*Z)  2XY - 2ZW       2XZ + 2YW       0
       2XY + 2ZW        1 - 2(XX + ZZ)  2YZ - 2XW       0
       2XZ - 2YW        2YZ + 2XW       1 - 2(XX + ZZ)  0
       0                0               0               1
===========================================================================*/
void Utility::QuatToRotMat(float rot_mat[9], float quat[4])
{
    float X = quat[0];
    float Y = quat[1];
    float Z = quat[2];
    float W = quat[3];

    float xx = X * X;
    float xy = X * Y;
    float xz = X * Z;
    float xw = X * W;
    float yy = Y * Y;
    float yz = Y * Z;
    float yw = Y * W;
    float zz = Z * Z;
    float zw = Z * W;

    HAL_LOG_DEBUG("%s: X=%f, Y=%f, Z=%f, W=%f", __FUNCTION__, X, Y, Z, W);

    rot_mat[0]  = 1 - 2 * ( yy + zz );
    rot_mat[1]  =     2 * ( xy - zw );
    rot_mat[2]  =     2 * ( xz + yw );
    rot_mat[3]  =     2 * ( xy + zw );
    rot_mat[4]  = 1 - 2 * ( xx + zz );
    rot_mat[5]  =     2 * ( yz - xw );
    rot_mat[6]  =     2 * ( xz - yw );
    rot_mat[7]  =     2 * ( yz + xw );
    rot_mat[8]  = 1 - 2 * ( xx + yy );
}

/*===========================================================================

  FUNCTION:  RotMatToOrient
  Convert rotation matrix to Orientation Sensor as defined in Sensor.TYPE_ORIENTATION:

    values[0]: Azimuth, angle between the magnetic north direction and the y-axis,
    around the z-axis (0 to 359). 0=North, 90=East, 180=South, 270=West

    values[1]: Pitch, rotation around x-axis (-180 to 180),
    with positive values when the z-axis moves toward the y-axis.

    values[2]: Roll, rotation around y-axis (-90 to 90),
    with positive values when the x-axis moves toward the z-axis.

===========================================================================*/
void Utility::RotMatToOrient(float values[3], float rot_mat[9])
{
    float xunit[3] = {rot_mat[0], rot_mat[3], rot_mat[6]};
    float yunit[3] = {rot_mat[1], rot_mat[4], rot_mat[7]};
    float zunit[3] = {rot_mat[2], rot_mat[5], rot_mat[8]};
    float xnorm = sqrt(xunit[0]*xunit[0] + xunit[1]*xunit[1]);

    if (fabs(zunit[2]) < MIN_FLT_TO_AVOID_SINGULARITY) {
        zunit[2] = MIN_FLT_TO_AVOID_SINGULARITY * (zunit[2] < 0 ? -1 : 1);
    }

    if (fabs(xunit[0]) < MIN_FLT_TO_AVOID_SINGULARITY) {
        xunit[0] = MIN_FLT_TO_AVOID_SINGULARITY * (xunit[0] < 0 ? -1 : 1);
    }

    if (fabs(xnorm) < MIN_FLT_TO_AVOID_SINGULARITY) {
        xnorm = MIN_FLT_TO_AVOID_SINGULARITY * (xnorm < 0 ? -1 : 1);
    }

    values[0] = RAD2DEG * atan2(xunit[1], xunit[0]);
    values[0] = fmodf(360.0f - values[0], 360.0f);
    values[1] = -RAD2DEG * atan2(yunit[2], zunit[2]);
    values[2] = RAD2DEG * atan2(xunit[2], xnorm);
}

/*===========================================================================

  FUNCTION:  insertQueue

===========================================================================*/
bool Utility::insertQueue(sensors_event_t const *data_ptr)
{
    hal_sensor_dataq_t* q_ptr;
    bool rv = true;

    q_ptr = (hal_sensor_dataq_t*)malloc(sizeof(hal_sensor_dataq_t));
    if (NULL != q_ptr) {
        q_ptr->next = NULL;
        q_ptr->data = *data_ptr;

        if (q_head_ptr == NULL) {
        /* queue is empty */
            q_tail_ptr = q_ptr;
            q_head_ptr = q_ptr;
        } else {
        /* append to tail and update tail ptr */
            q_tail_ptr->next = q_ptr;
            q_tail_ptr = q_ptr;
        }
    } else {
        HAL_LOG_ERROR("%s: malloc() error", __FUNCTION__);
        rv = false;
    }
    return rv;
}

/*===========================================================================

  FUNCTION:  removeFromQueue
  @brief
  Removes sensor data from head of the queue
  Helper function

  @param data_ptr: pointer to data

  @return true if data exists in the queue

  @dependencies Caller needs to lock the g_sensor_control->data_mutex before
                calling this function
===========================================================================*/
bool Utility::removeFromQueue(sensors_event_t* data_ptr)
{
    hal_sensor_dataq_t* q_ptr;
    bool rv = false;

    if (NULL != q_head_ptr) {
        /* copy the data from head */
        q_ptr = q_head_ptr;
        *data_ptr = q_head_ptr->data;

        /* update the pointers */
        if (q_head_ptr == q_tail_ptr) {
            /* queue has only one element */
            q_tail_ptr = NULL;
        }
        q_head_ptr = q_head_ptr->next;

        free(q_ptr);
        rv = true;
    }
    return rv;
}

/*===========================================================================
  FUNCTION:  signalResponse()
    Send the signal for the sensor1 response
    Parameters:
    @error : indicate the error is happened
    @cond_ptr : the pointer to the cond
===========================================================================*/
void Utility::signalResponse(bool error, hal_sensor1_cb_t* sensor1_cb)
{
    sensor1_cb->error = error;
    sensor1_cb->is_resp_arrived = true;
    pthread_cond_signal(&sensor1_cb->cb_cond);
}

/*===========================================================================
  FUNCTION:  signalInd()
    Send the siginal for the sensor1 data indicator
    Parameters:
    @is_ind_arrived : the reference to the is_ind_arrived
    @cond_ptr : the reference to the cond
===========================================================================*/
void Utility::signalInd(hal_data_cb_t* data_cb)
{
    data_cb->is_ind_arrived = true;
    pthread_cond_signal(&data_cb->data_cond);
}
