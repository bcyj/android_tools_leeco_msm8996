/*============================================================================
  @file StepCounter.h

  @brief
  StepCounter class definition.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef ANDROID_STEPCOUNTER_SENSOR_H
#define ANDROID_STEPCOUNTER_SENSOR_H

#include "SAMSensor.h"
#include "sns_sam_ped_v01.h"

/*============================================================================
 * Class StepCounter
=============================================================================*/

class StepCounter : public SAMSensor {
    uint64_t step_counter_running_total;
    uint64_t step_counter_running_instance;
    uint64_t step_counter_current_instance;
    uint64_t step_counter_last_timestamp;
/*===========================================================================
  FUNCTION:  sendGetReportReq
    Get the last report data after enabling the sensor.
===========================================================================*/
    int sendGetReportReq();

public:
    StepCounter(int handle);
    ~StepCounter();
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
  FUNCTION:  setSensorInfo
    Fill the sensor specific information.
===========================================================================*/
    void setSensorInfo();
};

#endif
