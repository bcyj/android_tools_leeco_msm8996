/*============================================================================
  @file Gravity.h

  @brief
  Gravity class definition.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef ANDROID_GRAVITY_SENSOR_H
#define ANDROID_GRAVITY_SENSOR_H

#include "SAMSensor.h"
#include "sns_sam_gravity_vector_v01.h"
#include "sns_sam_orientation_v01.h"

/*============================================================================
 * Class Gravity
=============================================================================*/

class Gravity : public SAMSensor {

public:
    Gravity(int handle);
    ~Gravity();
    int enable(int en);
/*===========================================================================
  FUNCTION:  processResp
    Process the response to the sensor1 SENSOR1_MSG_TYPE_RESP
    Parameters
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message data
===========================================================================*/
    void processResp(sensor1_msg_header_s *msg_hdr, void *msg_ptr);
/*===========================================================================
  FUNCTION:  processInd
    Process the response to the sensor1 SENSOR1_MSG_TYPE_IND
    Parameters
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message data
===========================================================================*/
    void processInd(sensor1_msg_header_s *msg_hdr, void *msg_ptr);
/*===========================================================================
  FUNCTION:  sendBatchReq
===========================================================================*/
    int sendBatchReq();
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from Accel sensor.
===========================================================================*/
    void setSensorInfo();
};

#endif
