/*============================================================================
  @file TiltDetector.h

  @brief
  TiltDetector class definition.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef ANDROID_TILTDETECTOR_SENSOR_H
#define ANDROID_TILTDETECTOR_SENSOR_H

#include "SAMSensor.h"
#include "sns_sam_tilt_detector_v01.h"

/*============================================================================
 * Class TiltDetector
=============================================================================*/

class TiltDetector : public SAMSensor {

public:
    TiltDetector(int handle);
    ~TiltDetector();
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
};

#endif
