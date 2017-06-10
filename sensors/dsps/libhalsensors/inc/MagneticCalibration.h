/*============================================================================
  @file MagneticCalibration.h

  @brief
  MagneticCalibration class definition.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef ANDROID_MAG_CAL_SENSOR_H
#define ANDROID_MAG_CAL_SENSOR_H

#include "SAMSensor.h"
#include "sns_sam_mag_cal_v01.h"

/*============================================================================
 * Class MagneticCalibration
=============================================================================*/

class MagneticCalibration : public SAMSensor {
    /* the data shared between the MAG SAM and UNCALIBRATED_SAM sensor.*/
    hal_sensor_mag_cal_sample_s mag_cal_cur_sample;

public:
    MagneticCalibration(int handle);
    ~MagneticCalibration();
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
};

#endif
