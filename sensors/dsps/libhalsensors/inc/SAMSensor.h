/*============================================================================
  @file SAMSensor.h

  @brief
  SAMSensor class definition.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef ANDROID_SAM_SENSOR_H
#define ANDROID_SAM_SENSOR_H

#include "Sensor.h"
#include "Utility.h"
#include "sns_sam_common_v01.h"
#include "fixed_point.h"

class TimeSyncService;

/* Maximum data fields per SAM algorithm report */
#define SAM_MAX_DATA_LENGTH 16

typedef struct hal_sam_sample {
    float data[SAM_MAX_DATA_LENGTH];
    uint32_t timestamp;
    uint8_t accuracy;
} hal_sam_sample_t;

class SAMSensor : public Sensor {
protected:
    /* the time sync service object */
    TimeSyncService *time_service;
    /* mutex and cond for the report sensor1 callback*/
    hal_sensor1_cb_t *sensor1_cb;
    /* Android Sensor handles that this algorithm serves */
    int svc_num;
    /* Used to store instance IDs for SAM req/resp */
    uint8_t instance_id;
    /* Q16 batch rate or report rate in Q16 */
    uint32_t batch_rate;
    /* To know if sensor supports batching or NOT */
    bool batch_support;
/*===========================================================================
  FUNCTION:  sendEnableReq
    Send the enable command for svc_num sensor.
    Parameters
        @msg_hdr : Sensor1 message header pointer
        @msg_ptr : Sensor1 message pointer
    Return value
        @int : return 0 on success, negative errno code otherwise.
===========================================================================*/
    int sendEnableReq(sensor1_msg_header_s *req_hdr, void *sam_req);
/*===========================================================================
  FUNCTION:  sendCancel
    Send cancel command for svc_num sensor.
===========================================================================*/
    void sendCancel();
/*===========================================================================
  FUNCTION:  sendAlgoAttribReq
    Call the sensor1 API to get the sensor attribute from SAM Algos
    Return value
        true  : Success.
        false : Failured.
===========================================================================*/
    bool sendAlgoAttribReq();
/*===========================================================================
  FUNCTION:  processAlgoAttribResp
    This function processes response message of attributes request from SAM algo
    and sets the attributes for the sensor
    Parameters
        @msg_hdr : Sensor1 message header
        @msg_ptr : Sensor1 message pointer
    Return value
        true  : Success
        false : Failed
===========================================================================*/
    bool processAlgoAttribResp(sensor1_msg_header_s *msg_hdr, void *msg_ptr);
public:
/*============================================================================
  FUNCTION Constructor
============================================================================*/
    SAMSensor(int handle);
/*============================================================================
  FUNCTION Destructor
============================================================================*/
    virtual ~SAMSensor();
/*============================================================================
  FUNCTION getSensor1Cb
    Get the sensor1 callback data.
    Return value
        @hal_sensor1_cb_t* : The sensor1 callback data.
============================================================================*/
    hal_sensor1_cb_t* getSensor1Cb();
/*===========================================================================
  FUNCTION:  enable
    Enable or disable the sensor of handle.
    Parameters
        @en : 1 - enable ; 0 - disable.
    Return value
        @int : return 0 on success, negative errno code otherwise.
===========================================================================*/
    virtual int enable(int en) = 0;
/*===========================================================================
  FUNCTION:  batch
    Enable the batch and the freq and rpt_data for the sensor of handle.
    Parameters
        @flags : This parameter is deprecated from Sensor HAL Version 1_3
            Two flags are available in the current framework.
            SENSORS_BATCH_DRY_RUN
            It is used to check if batch mode is available for a given configuration.
            SENSORS_BATCH_WAKE_UPON_FIFO_FULL
            The sensor will wake up the soc if the sensor FIFO is full.
        @period_ns : set the events's period in nanosecond
        @timeout : timeout value of zero disables batch mode
    Return value
        @int : Batch the sensor successful or not.
===========================================================================*/
    int batch(int flags, int64_t period_ns, int64_t timeout);
/*===========================================================================
  FUNCTION:  flush
    Flush the sensor data of handle.
    Parameters
        NA
    Return value
        @int : Flush the sensor successful or not.
===========================================================================*/
    int flush();
/*===========================================================================
  FUNCTION:  processResp
    Process the response to the sensor1 SENSOR1_MSG_TYPE_RESP.
    Parameters
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message data
===========================================================================*/
    virtual void processResp(sensor1_msg_header_s *msg_hdr, void *msg_ptr) = 0;
/*===========================================================================
  FUNCTION:  processInd
    Process the response to the sensor1 SENSOR1_MSG_TYPE_IND
    Parameters
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message data
===========================================================================*/
    virtual void processInd(sensor1_msg_header_s *msg_hdr, void *msg_ptr) = 0;
/*===========================================================================
  FUNCTION:  sendBatchReq
===========================================================================*/
    virtual int sendBatchReq();
/*===========================================================================
  FUNCTION:  setSensorInfo
===========================================================================*/
    virtual void setSensorInfo();
};

/*===========================================================================
  FUNCTION:  SAMSensor_sensor1_cb
    Handle the sensor1 callback for the SAM sensors. Each SAM sensor has its own
    sensor1 handle but they share the same callback function.
    Parameters
        @cb_data : pointer of the callback data, SensorsContext is passed in
            this function
        @msg_hdr : sensor1 message header
        @msg_type : sensor1 message type, two major types are listed in the below:
                    SENSOR1_MSG_TYPE_RESP
                    SENSOR1_MSG_TYPE_IND
        @msg_ptr : sensor1 message pointer, do free this memory before return
===========================================================================*/
void SAMSensor_sensor1_cb (intptr_t cb_data,
                     sensor1_msg_header_s *msg_hdr,
                     sensor1_msg_type_e msg_type,
                     void *msg_ptr);
#endif
