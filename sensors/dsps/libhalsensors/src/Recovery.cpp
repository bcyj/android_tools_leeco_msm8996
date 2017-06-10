/*============================================================================
  @file Recovery.cpp

  @brief
  Recovery class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <errno.h>
#include <stdlib.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "Recovery.h"
#include "Utility.h"

/*============================================================================
  Recovery constructor
============================================================================*/
Recovery::Recovery()
{

}

/*============================================================================
  Recovery destructor
============================================================================*/
Recovery::~Recovery()
{

}

int Recovery::reInit(hal_sensor1_cb_t *sensor1_cb,
                    sensor1_notify_data_cb_t notify_cb,
                    intptr_t cb_data)
{
    int err = SENSOR1_EFAILED;
    HAL_LOG_DEBUG("%s", __FUNCTION__);

    if (sensor1_cb->sensor1_handle == (sensor1_handle_s*)-1) {
        err = sensor1_open(&sensor1_cb->sensor1_handle, notify_cb,
                          (intptr_t)cb_data);
        HAL_LOG_DEBUG("%s: sensor1_open() err=%d hndl=%"PRIdPTR, __FUNCTION__, err,
                    (intptr_t)(sensor1_cb->sensor1_handle));

        if (err == SENSOR1_SUCCESS) {
            HAL_LOG_DEBUG("Sensor1 reopen success.");
        }
        else if (err == SENSOR1_EWOULDBLOCK) {
            HAL_LOG_ERROR("sensor1_open returned EWOULDBLOCK. Daemon not ready, will try again");
        }
        else {
            HAL_LOG_ERROR( "%s: sensor1_open() failed err=%d", __FUNCTION__, err);
        }
    }
    return err;
}

void Recovery::handleBrokenPipe(hal_sensor1_cb_t *sensor1_cb,
                    sensor1_notify_data_cb_t notify_cb,
                    intptr_t cb_data)
{
    int err;
    int retry = 10;
    Sensor *mSensor = (Sensor *)cb_data;
    HAL_LOG_DEBUG("The BrokenPipe handle is %d", mSensor->getHandle());
    /* close sensor1 connection */
    if (sensor1_cb->sensor1_handle != (sensor1_handle_s*)-1) {
        sensor1_close(sensor1_cb->sensor1_handle);
        HAL_LOG_DEBUG("%s: close the sensor1.", __FUNCTION__);
        sensor1_cb->sensor1_handle = (sensor1_handle_s*)-1;
    }

    /* clean some resources */
    Utility::signalResponse(false, sensor1_cb);
    sensor1_cb->error = false;

    /* reinit the sensor1 connection */
    err = reInit(sensor1_cb, notify_cb, cb_data);
    while (err == SENSOR1_EWOULDBLOCK && retry > 0) {
        HAL_LOG_WARN("open sensor1 failed at SENSOR1_EWOULDBLOCK, retrying...%d", retry);
        err = reInit(sensor1_cb, notify_cb, cb_data);
        retry--;
    }
}



