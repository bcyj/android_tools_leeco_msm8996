/*============================================================================
  @file Recovery.h

  @brief
  Recovery class definition.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef ANDROID_RECOVERY_H
#define ANDROID_RECOVERY_H

#include "Sensor.h"

/*============================================================================
 * Class Recovery
 *=============================================================================*/
class Recovery {
/*============================================================================
  FUNCTION Constructor
    Put the constructor to be private.
============================================================================*/
    Recovery();
/*============================================================================
  FUNCTION Destructor
    Put the destructor to be private.
============================================================================*/
    ~Recovery();
public:
/*============================================================================
  FUNCTION handleBrokenPipe
    Handle the sensor1 broken pipe.
    Parameters
        @sensor1_cb : The sensor1 callback data.
        @notify_cb : Then sensor1 callback function.
        @cb_data: The callback private data.
============================================================================*/
    static void handleBrokenPipe(hal_sensor1_cb_t *sensor1_cb,
                    sensor1_notify_data_cb_t notify_cb,
                    intptr_t cb_data);
/*============================================================================
  FUNCTION Destructor
    Re-init the sensor connection.
    Parameters
        @sensor1_cb : The sensor1 callback data.
        @notify_cb : Then sensor1 callback function.
        @cb_data: The callback private data.
    Return value
        @int : Successful or not.
============================================================================*/
    static int reInit(hal_sensor1_cb_t *sensor1_cb,
                    sensor1_notify_data_cb_t notify_cb,
                    intptr_t cb_data);
};

#endif
