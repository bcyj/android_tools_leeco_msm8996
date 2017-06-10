/*============================================================================
  @file SMGRSensor.h

  @brief
  SMGRSensor class definition.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef ANDROID_SMGR_SENSOR_H
#define ANDROID_SMGR_SENSOR_H

#include "Sensor.h"
#include "sns_smgr_api_v01.h"
#include "Utility.h"

class TimeSyncService;

/*============================================================================
 * Class SMGRSensor
 *============================================================================*/
class SMGRSensor : public Sensor {
protected:
    /* The last event of this sensor */
    sensors_event_t last_event;
    /* The time sync service object */
    TimeSyncService *time_service;
    /* The sensor1 call back shared by all the SMGR sensors */
    static hal_sensor1_cb_t *smgr_sensor1_cb;

/*===========================================================================
  FUNCTION:  SMGRPrepareAddMsg
    Prepare the sensor1 message for enable and batch function.
    Parameters
        @sample_rate : the sensor sample data rate
        @report_rate : the sensor report data rate
        @buff_req : sensor1 message buffer
    Return value
        true : The buffer preparation is successful.
        false : The buffer preparation is failed.
===========================================================================*/
    bool SMGRPrepareAddMsg(float sample_rate,
            uint32_t report_rate, sns_smgr_buffering_req_msg_v01 **buff_req);
/*===========================================================================
  FUNCTION:  SMGRReportAdd
    Send the sensor1 message and enable/batch the sensor.
    Parameters
        @sample_rate : the sensor sample data rate in Hz
        @report_rate : the sensor report data rate in Hz
        @do_batching : enable batch or not
        @wait_for_resp : wait for the sensor1 response
    Return value
        true : Add the sensor is successful.
        false : Add the sensor is failed.
===========================================================================*/
    bool SMGRReportAdd(float sample_rate,
            uint32_t report_rate, bool do_batching, bool wait_for_resp);
/*===========================================================================
  FUNCTION:  SMGRReportDelete
    Delete the sensor of handle from SMGR.
    Parameters
        NA
    Return value
        true : Delete the sensor is successful.
        false : Delete the sensor is failed.
===========================================================================*/
    bool SMGRReportDelete();

public:
/*============================================================================
  FUNCTION Constructor
============================================================================*/
    SMGRSensor(int handle);
/*============================================================================
  FUNCTION Destructor
============================================================================*/
    virtual ~SMGRSensor();
/*===========================================================================
  FUNCTION:  getSMGRSensor1Cb
    Return the static smgr_sensor1_cb.
    Return value
        @hal_sensor1_cb_t* : The pointer to the hal_sensor1_cb_t.
===========================================================================*/
    static hal_sensor1_cb_t* getSMGRSensor1Cb();
/*===========================================================================
  FUNCTION:  getLastEvent
    Get the sensor last event data
    Return value:
        @sensors_event_t *: The point to the last_event of the sensor.
===========================================================================*/
    sensors_event_t *getLastEvent();
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
        @datatype : sensor information got from the sensor1 callback
===========================================================================*/
    virtual void setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* datatype) = 0;
/*===========================================================================
  FUNCTION:  processReportInd
    process the sensor data indicator from the sensor1 type to sensor event type
    Parameters
        @smgr_data : the sensor1 data message from the sensor1 callback
        @sensor_data : the sensor event message that will be send to framework
===========================================================================*/
    virtual void processReportInd(sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
                sns_smgr_data_item_s_v01* smgr_data,
                sensors_event_t &sensor_data) = 0;
/*===========================================================================
  FUNCTION:  prepareAddMsg
    SMGRSensor::SMGRPrepareAddMsg will call this function and this function will
    fill the item like the sensor id that needed for this type of sensor.
    Parameters
        @buff_req : the sensor1 message buffer
===========================================================================*/
    virtual void prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req) = 0;
/*===========================================================================
  FUNCTION:  enable
    Enable or disable the sensor of handle.
    Parameters
        @en : 1 - enable ; 0 - disable.
    Return value
        @int : return 0 on success, negative errno code otherwise.
===========================================================================*/
    int enable(int en);
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
    static void processResp(Sensor** msensors, sensor1_msg_header_s *msg_hdr,
            void *msg_ptr);
/*===========================================================================
  FUNCTION:  processBufferingResp
    Process the response to the sensor1 SNS_SMGR_BUFFERING_RESP_V01.
    Parameters
        @msensors : the pointer to the sensor class
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message data
===========================================================================*/
    static void processBufferingResp(Sensor** mSensors,
            sns_smgr_buffering_resp_msg_v01* smgr_resp,
            sensor1_msg_header_s* msg_hdr);
/*===========================================================================
  FUNCTION:  processInd
    Process the response to the sensor1 SENSOR1_MSG_TYPE_IND.
    Parameters
	@mSensors : the sensors handle
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message data
===========================================================================*/
    static void processInd(Sensor** msensors, sensor1_msg_header_s *msg_hdr,
            void *msg_ptr);
/*===========================================================================
  FUNCTION:  processInd
    Process the response to the sensor1 SNS_SMGR_BUFFERING_IND_V01.
    Parameters
	@mSensors : the sensors handle
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message data
===========================================================================*/
    static void processBufferingInd(Sensor** mSensors,
            sns_smgr_buffering_ind_msg_v01* smgr_ind);
/*===========================================================================
  FUNCTION:  processReportInd
    Process a report indication from SMGR.
    Parameters
	@mSensors : the sensors handle
        @mSensors : the pointer to the sensor class
        @smgr_ind : sensor periodic report data
===========================================================================*/
    static void processReportInd(Sensor** msensors,
            sns_smgr_periodic_report_ind_msg_v01* smgr_ind);
};

/*===========================================================================
  FUNCTION:  SMGRSensor_sensor1_cb
    Data call back function. Processes response and indicator messages for
    the SMGR sensor1 handle.
    Parameters
        @cb_data : call back data
        @msg_hdr : Message header
        @msg_type : Message type
        @msg_ptr : Message pointer
===========================================================================*/
void SMGRSensor_sensor1_cb (intptr_t cb_data,
                     sensor1_msg_header_s *msg_hdr,
                     sensor1_msg_type_e msg_type,
                     void *msg_ptr);
#endif
