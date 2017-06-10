/*============================================================================
  @file SMGRSensor.cpp

  @brief
  SMGRSensor class implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "SMGRSensor.h"
#include "Utility.h"
#include "Recovery.h"
#include "TimeSyncService.h"
#include "Latency.h"
#include "SensorsContext.h"
#include <pthread.h>
#include <utils/SystemClock.h>
#include "fixed_point.h"

hal_sensor1_cb_t *SMGRSensor::smgr_sensor1_cb;

/*============================================================================
  SMGRSensor Constructor
============================================================================*/
SMGRSensor::SMGRSensor(int handle)
    :Sensor(handle)
{
    last_event.timestamp = 0;
    HAL_LOG_INFO("%s: Sensor(%s)", __FUNCTION__,Utility::SensorTypeToSensorString(getType()));
    time_service = TimeSyncService::getTimeSyncService();
    data_cb = Utility::getDataCb();
}

/*============================================================================
  SMGRSensor Destructor
============================================================================*/
SMGRSensor::~SMGRSensor()
{
    HAL_LOG_INFO("%s: Sensor(%s)", __FUNCTION__,Utility::SensorTypeToSensorString(getType()));
}

/*===========================================================================
  FUNCTION:  getSMGRSensor1Cb
    Return the static smgr_sensor1_cb.
===========================================================================*/
hal_sensor1_cb_t* SMGRSensor::getSMGRSensor1Cb()
{
    if (smgr_sensor1_cb == NULL) {
        pthread_mutexattr_t attr;
        smgr_sensor1_cb = new hal_sensor1_cb_t;
        memset(smgr_sensor1_cb, 0, sizeof(*smgr_sensor1_cb));
        smgr_sensor1_cb->is_resp_arrived = false;
        smgr_sensor1_cb->error = false;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&(smgr_sensor1_cb->cb_mutex), &attr);
        pthread_cond_init(&(smgr_sensor1_cb->cb_cond), NULL);
        pthread_mutexattr_destroy(&attr);
        HAL_LOG_INFO("Init the smgr_sensor1_cb for SMGR sensor1 connection.");
    }
    return smgr_sensor1_cb;
}

/*===========================================================================
  FUNCTION:  enable
    enable or disable the sensor of handle
    Parameters
    @en : 1 - enable ; 0 - disable
===========================================================================*/
int SMGRSensor::enable(int en)
{
    int err;
    const bool wait_for_resp = true;
    if (enabled == en) {
        HAL_LOG_INFO("%s: Sensor %s is already enabled/disabled %d",
            __FUNCTION__,Utility::SensorTypeToSensorString(getType()), enabled);
        return 0;
    }
    pthread_mutex_lock(&smgr_sensor1_cb->cb_mutex);
    /* store the en value */
    enabled = en;
    if (en) {
        /* enable the sensor */
        if (0 == freq) {
            /* By default, enable sensors at the fastest rate */
            freq = calcSampleRate(0);
        }
        HAL_LOG_DEBUG("%s:sensor(%s) handle %d, freq=%f report_rate=%d batched=%d",
            __FUNCTION__, Utility::SensorTypeToSensorString(getType()),handle, freq, report_rate, batching);
        err = SMGRReportAdd(freq, report_rate, batching, wait_for_resp);
        if (true != err) {
            HAL_LOG_ERROR("%s:sensor(%s) Failed for handle %d @ samp %f Hz rpt %d Hz batched %d",
                __FUNCTION__, Utility::SensorTypeToSensorString(getType()), handle, freq, report_rate, batching);
            pthread_mutex_unlock(&smgr_sensor1_cb->cb_mutex);
            enabled = 0;
            return -1;
        }
    } else {
        /* disable the sensor */
        HAL_LOG_DEBUG("%s:sensor(%s) Deactivating sensor handle=%d",
            __FUNCTION__,Utility::SensorTypeToSensorString(getType()), handle);
        err = SMGRReportDelete();
        if (true != err) {
            HAL_LOG_ERROR( "%s:sensor(%s) Failed to deactivate sensor handle=%d",
                __FUNCTION__,Utility::SensorTypeToSensorString(getType()), handle );
            pthread_mutex_unlock(&smgr_sensor1_cb->cb_mutex);
            return -1;
        }
    }
    pthread_mutex_unlock(&smgr_sensor1_cb->cb_mutex);
    return 0;
}

/*===========================================================================
  FUNCTION:  batch
    enable the batch and the freq and rpt_data for the sensor of handle
    Parameters
    @flags : This parameter is deprecated from Sensor HAL Version 1_3
        Two flags are available in the current framework.
        SENSORS_BATCH_DRY_RUN
        It is used to check if batch mode is available for a given configuration.
        SENSORS_BATCH_WAKE_UPON_FIFO_FULL
        The sensor will wake up the soc if the sensor FIFO is full.
    @period_ns : set the events's period in nanosecond
    @timeout : timeout value of zero disables batch mode
===========================================================================*/
int SMGRSensor::batch(int flags, int64_t period_ns, int64_t timeout)
{
    int err;
    float sample_rate = 0;
    float report_rate_f = 0;
    uint32_t report_rate_Q16 = 0;
    bool do_buffering = false;
    const bool wait_for_resp = true;

    HAL_LOG_INFO("%s:sensor(%s) handle:%d flags:0x%x period_ns %lld",
        __FUNCTION__, Utility::SensorTypeToSensorString(getType()), handle, flags, period_ns);
    if (period_ns < 0) {
        HAL_LOG_ERROR( "Error in %s: argument ns < 0", __FUNCTION__ );
        return -1;
    }
    sample_rate = calcSampleRate((uint64_t)period_ns);
    if (timeout == 0) {
        report_rate_f = 0;
        do_buffering = false;
    } else {
        report_rate_f = NSEC_TO_HZ(timeout);
        do_buffering = true;
    }
    /* The batch/report rate can never be greater than the sample rate */
    if (report_rate_f > sample_rate) {
        report_rate_f = sample_rate;
        HAL_LOG_DEBUG("Report rate has been adjusted to: %fHz", report_rate_f);
    }
    report_rate_Q16 = FX_FLTTOFIX_Q16(report_rate_f);
    /* Force the next sample-equality test to fail, even if the data
     * is the same (Light Sensor) */
    last_event.reserved1[0] += 1;
    HAL_LOG_DEBUG( "%s:sensor(%s) sample_rate=%fHz report_rate_f=%fHz curr sample rate:%f cur rpt rate:%u max:%f min:%f",
            __FUNCTION__, Utility::SensorTypeToSensorString(getType()),sample_rate, report_rate_f, freq, report_rate, max_freq, min_freq);

    pthread_mutex_lock(&smgr_sensor1_cb->cb_mutex);
    if (sample_rate  == freq && report_rate_Q16 == report_rate &&
        do_buffering == batching ) {
        pthread_mutex_unlock(&smgr_sensor1_cb->cb_mutex);
        HAL_LOG_INFO("%s: current sample rate, report rate & buffering are equal to requested (%f,%f,%d)",
            __FUNCTION__, sample_rate, report_rate_f, do_buffering);
    } else {
        freq = sample_rate;
        report_rate = report_rate_Q16;
        batching = do_buffering;
        if ( enabled ) {
            err = SMGRReportAdd(freq, report_rate, batching, wait_for_resp);
            if (true != err) {
                HAL_LOG_ERROR("%s:sensor(%s) Failed for handle %d @ samp %f Hz rpt %d Hz batched %d",
                    __FUNCTION__,Utility::SensorTypeToSensorString(getType()), handle, freq, report_rate, batching);
                pthread_mutex_unlock(&smgr_sensor1_cb->cb_mutex);
                return -1;
            }
        }
        pthread_mutex_unlock(&smgr_sensor1_cb->cb_mutex);
    }
    return 0;
}

/*===========================================================================
  FUNCTION:  flush
    flush the sensor data of handle
===========================================================================*/
int SMGRSensor::flush()
{
    int err;
    flush_requested++;

    if (!enabled) {
        flush_requested--;
        HAL_LOG_DEBUG("%s: handle %d is inactive", __FUNCTION__, handle);
        return -EINVAL;
    }
    else if (batching) {
        HAL_LOG_DEBUG("SMGRSensor::flush for batching");
        pthread_mutex_lock(&smgr_sensor1_cb->cb_mutex);
        err = SMGRReportAdd(freq, report_rate, batching, false);
        pthread_mutex_unlock(&smgr_sensor1_cb->cb_mutex);
        if (err != true) {
            HAL_LOG_ERROR("%s: Failed for handle %d @ samp %d Hz rpt %d Hz batched %d",
                __FUNCTION__, handle, freq, report_rate, batching);
            return -EINVAL;
        }
    }
    else {
        HAL_LOG_DEBUG("%s: handle %d is not batching", __FUNCTION__, handle);
        pthread_mutex_lock(&data_cb->data_mutex);
        flushSendCmplt();
        pthread_mutex_unlock(&data_cb->data_mutex);
    }
    return 0;
}

/*===========================================================================
  FUNCTION:  SMGRPrepareAddMsg
    Prepare the sensor1 message for enable and batch function
    Parameters
    @sample_rate : the sensor sample data rate
    @report_rate : the sensor report data rate
    @buff_req : sensor1 message buffer
===========================================================================*/
bool SMGRSensor::SMGRPrepareAddMsg(float sample_rate,
                             uint32_t report_rate,
                             sns_smgr_buffering_req_msg_v01 **buff_req)
{
    sensor1_error_e error;
    uint32_t subhz_sample_rate_ms = 0;
    uint32_t sample_rate_hz = 0;

    /* Message Body */
    error = sensor1_alloc_msg_buf(smgr_sensor1_cb->sensor1_handle,
                                 sizeof(sns_smgr_buffering_req_msg_v01),
                                 (void**)buff_req);
    if (SENSOR1_SUCCESS != error) {
        HAL_LOG_ERROR("%s: sensor1_alloc_msg_buf() error: %d", __FUNCTION__, error);
        (*buff_req) = NULL;
        return false;
    }

    if (report_rate == 0) {
        report_rate = (int)lroundf(sample_rate)*UNIT_Q16;
    }
    /* convert sample_rate from Hz to milli sec to support subHz */
    if (sample_rate < 1) {
        subhz_sample_rate_ms = (int)lroundf(HZ_TO_MSEC(sample_rate));
        (*buff_req)->Item[0].SamplingRate = subhz_sample_rate_ms;
        (*buff_req)->Item[1].SamplingRate = subhz_sample_rate_ms;
    }
    else {
        sample_rate_hz = (int)lroundf(sample_rate);
        (*buff_req)->Item[0].SamplingRate = sample_rate_hz;
        (*buff_req)->Item[1].SamplingRate = sample_rate_hz;
    }
    HAL_LOG_DEBUG("%s sample_rate %f report_rate %d subhz_sample_rate_ms %d sample_rate_hz %d",
                  __FUNCTION__, sample_rate, report_rate, subhz_sample_rate_ms, sample_rate_hz);

    /* Report Request */
    (*buff_req)->ReportId = handle;
    (*buff_req)->Action = SNS_SMGR_BUFFERING_ACTION_ADD_V01;
    (*buff_req)->ReportRate = report_rate;
    /* Most HAL apis don't use a 2nd report item */
    (*buff_req)->Item_len = 1;
    /* Most requests are for primary data */
    (*buff_req)->Item[0].DataType = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
    /* set default behavior for indications during suspend */
    (*buff_req)->notify_suspend_valid = true;
    (*buff_req)->notify_suspend.proc_type = SNS_PROC_APPS_V01;
    (*buff_req)->notify_suspend.send_indications_during_suspend = bWakeUp;
    (*buff_req)->Item[0].Calibration = SNS_SMGR_CAL_SEL_FULL_CAL_V01;
    (*buff_req)->Item[0].Decimation = SNS_SMGR_DECIMATION_FILTER_V01;
    /* Call the prepareAddMsg for each sensor class */
    if(bWakeUp == true) {
        HAL_LOG_DEBUG("%s:sensor %s (wake_up)",__FUNCTION__,
                    Utility::SensorTypeToSensorString(getType()));
    } else {
        HAL_LOG_DEBUG("%s:sensor %s ",__FUNCTION__,
                    Utility::SensorTypeToSensorString(getType()));
    }
    prepareAddMsg(buff_req);
    return true;
}

/*===========================================================================
  FUNCTION:  SMGRReportAdd
    Send the sensor1 message and enable/batch the sensor
    Parameters
    @sample_rate : the sensor sample data rate
    @report_rate : the sensor report data rate
    @do_batching : enable batch or not
    @wait_for_resp : wait for the sensor1 response
===========================================================================*/
bool SMGRSensor::SMGRReportAdd(float sample_rate, uint32_t report_rate,
                            bool buffer, bool wait_for_resp)
{
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_smgr_buffering_req_msg_v01 *smgr_req;
    bool resp = false;

    HAL_LOG_DEBUG("%s:sensor(%s) handle=%d, sample_rate=%f report_rate=%d buffer=%d",
        __FUNCTION__, Utility::SensorTypeToSensorString(getType()), handle, sample_rate,report_rate,buffer);

    if (!SMGRPrepareAddMsg(sample_rate, report_rate, &smgr_req)) {
        HAL_LOG_ERROR("%s: SMGRPrepareAddMsg return failed!", __FUNCTION__);
        return false;
    }

    smgr_sensor1_cb->error = false;
    req_hdr.txn_id = (wait_for_resp) ? handle : TXN_ID_NO_RESP_SIGNALLED;
    req_hdr.service_number = SNS_SMGR_SVC_ID_V01;
    req_hdr.msg_id = SNS_SMGR_BUFFERING_REQ_V01;
    req_hdr.msg_size = sizeof(sns_smgr_buffering_req_msg_v01);

    if ((error = sensor1_write(smgr_sensor1_cb->sensor1_handle, &req_hdr,
                smgr_req)) != SENSOR1_SUCCESS) {
        sensor1_free_msg_buf(smgr_sensor1_cb->sensor1_handle, smgr_req);
        HAL_LOG_ERROR("%s: sensor1_write() error: %d", __FUNCTION__, error);
        return false;
    }
    if (wait_for_resp) {
        resp = Utility::waitForResponse(TIME_OUT_MS,
                    &smgr_sensor1_cb->cb_mutex,
                    &smgr_sensor1_cb->cb_cond,
                    &smgr_sensor1_cb->is_resp_arrived);
        if (!resp) {
            HAL_LOG_ERROR("%s: ERROR: No response from request", __FUNCTION__);
            return false;
        }
        HAL_LOG_DEBUG("%s: Received Response: %d", __FUNCTION__, smgr_sensor1_cb->error);
        /* received response */
        if(smgr_sensor1_cb->error) {
            return false;
        }
    }
    return true;
}

/*===========================================================================
  FUNCTION:  SMGRReportDelete
    delete the sensor of handle from smgr
===========================================================================*/
bool SMGRSensor::SMGRReportDelete()
{
    sensor1_error_e error;
    sensor1_msg_header_s req_hdr;
    sns_smgr_buffering_req_msg_v01 *smgr_req;
    bool resp = false;

    HAL_LOG_DEBUG("%s:sensor(%s) handle=%d", __FUNCTION__,Utility::SensorTypeToSensorString(getType()), handle);
    error = sensor1_alloc_msg_buf(smgr_sensor1_cb->sensor1_handle,
                    sizeof(sns_smgr_buffering_req_msg_v01),
                    (void**)&smgr_req);
    if (SENSOR1_SUCCESS != error) {
        HAL_LOG_ERROR( "%s: sensor1_alloc_msg_buf() failed: %u", __FUNCTION__, error);
        return false;
    }
    /* Message header */
    req_hdr.service_number = SNS_SMGR_SVC_ID_V01;
    req_hdr.msg_id = SNS_SMGR_BUFFERING_REQ_V01;
    req_hdr.msg_size = sizeof(sns_smgr_buffering_req_msg_v01);
    req_hdr.txn_id = handle;
    /* Message body */
    smgr_req->ReportId = handle;
    smgr_req->Action = SNS_SMGR_BUFFERING_ACTION_DELETE_V01;
    /* Send Enable Request */
    smgr_sensor1_cb->error = false;
    error = sensor1_write(smgr_sensor1_cb->sensor1_handle, &req_hdr, smgr_req);
    if(error != SENSOR1_SUCCESS) {
        sensor1_free_msg_buf(smgr_sensor1_cb->sensor1_handle, smgr_req);
        HAL_LOG_ERROR("%s: sensor1_write() error: %u", __FUNCTION__, error);
        return false;
    }
    resp = Utility::waitForResponse(TIME_OUT_MS,
                &(smgr_sensor1_cb->cb_mutex),
                &(smgr_sensor1_cb->cb_cond),
                &(smgr_sensor1_cb->is_resp_arrived));
    if(!resp) {
        HAL_LOG_ERROR("%s: ERROR: no response from request", __FUNCTION__);
    } else if(smgr_sensor1_cb->error) {
        HAL_LOG_ERROR("%s: Error in report delete", __FUNCTION__);
    } else {
        HAL_LOG_DEBUG("%s: Rcvd success response from request", __FUNCTION__);
        return true;
    }
    return false;
}

/*===========================================================================
  FUNCTION:  getLastEvent
    Get the sensor last event data
    Return value:
    sensors_event_t *: the point to the last_event of the sensor
===========================================================================*/
sensors_event_t *SMGRSensor::getLastEvent()
{
    return &last_event;
}

/*===========================================================================
  FUNCTION:  processResp
    Process the response to the sensor1 SENSOR1_MSG_TYPE_RESP
    Parameters
    @msg_hdr : sensor1 message header
    @msg_ptr : sensor1 message data
===========================================================================*/
void SMGRSensor::processResp(Sensor** mSensors, sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    HAL_LOG_DEBUG("%s: %d", __FUNCTION__,  msg_hdr->msg_id);
    switch(msg_hdr->msg_id) {
        case SNS_SMGR_BUFFERING_RESP_V01:
            processBufferingResp(mSensors, (sns_smgr_buffering_resp_msg_v01*)msg_ptr, msg_hdr);
            break;
    }
    return;
}

/*===========================================================================
  FUNCTION:  processBufferingResp
    This function processes SMGR response messages for adding/deleting a report.
    Parameters
    @msensors : the pointer to the sensor class
    @msg_hdr : sensor1 message header
    @msg_ptr : sensor1 message data
===========================================================================*/
void SMGRSensor::processBufferingResp(Sensor** mSensors,
                sns_smgr_buffering_resp_msg_v01* smgr_resp,
                sensor1_msg_header_s* msg_hdr)
{
    bool error = false;
    int i;
    hal_data_cb_t *data_cb = Utility::getDataCb();

    if (smgr_resp->Resp.sns_result_t != 0) {
        HAL_LOG_ERROR("%s: Result: %u, Error: %u", __FUNCTION__,
            smgr_resp->Resp.sns_result_t, smgr_resp->Resp.sns_err_t);
        error = true;
    }
    if (smgr_resp->AckNak != SNS_SMGR_RESPONSE_ACK_SUCCESS_V01 &&
        smgr_resp->AckNak != SNS_SMGR_RESPONSE_ACK_MODIFIED_V01) {
        HAL_LOG_ERROR( "%s: %d Error: %u Reason: %u", __FUNCTION__, smgr_resp->ReportId,
            smgr_resp->AckNak, smgr_resp->ReasonPair[0].Reason);
        error = true;
    }
    if (smgr_resp->AckNak == SNS_SMGR_RESPONSE_NAK_REPORT_ID_V01) {
        error = false;
    }
    HAL_LOG_DEBUG("%s: Id: %u Resp: %u txn id %d", __FUNCTION__,
        smgr_resp->ReportId, smgr_resp->AckNak, msg_hdr->txn_id);

    pthread_mutex_lock(&smgr_sensor1_cb->cb_mutex);
    if(msg_hdr->txn_id != TXN_ID_NO_RESP_SIGNALLED) {
        Utility::signalResponse(error, smgr_sensor1_cb);
    }
    pthread_mutex_unlock(&smgr_sensor1_cb->cb_mutex);

    pthread_mutex_lock(&data_cb->data_mutex);
    for (i = 0; i < SAM_HANDLE_BASE; i++) {
        if(mSensors[i] != NULL)
            mSensors[i]->flushSendCmplt();
    }
    pthread_mutex_unlock(&data_cb->data_mutex );
    return;
}

/*===========================================================================
  FUNCTION:  processInd
    Process the response to the sensor1 SENSOR1_MSG_TYPE_IND
    Parameters
    @msg_hdr : sensor1 message header
    @msg_ptr : sensor1 message data
===========================================================================*/
void SMGRSensor::processInd(Sensor** mSensors, sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
    switch (msg_hdr->msg_id) {
        case SNS_SMGR_BUFFERING_IND_V01:
            processBufferingInd(mSensors, (sns_smgr_buffering_ind_msg_v01*)msg_ptr);
            break;
        case SNS_SMGR_SENSOR_POWER_STATUS_IND_V01:
        default:
            break;
    }
    return;
}

/*===========================================================================
  FUNCTION:  processInd
    Process the response to the sensor1 SNS_SMGR_BUFFERING_IND_V01
    Parameters
    @msg_hdr : sensor1 message header
    @msg_ptr : sensor1 message data
===========================================================================*/
void SMGRSensor::processBufferingInd(Sensor** mSensors, sns_smgr_buffering_ind_msg_v01* smgr_ind)
{
    sns_smgr_periodic_report_ind_msg_v01 report_msg;
    uint32_t i,j;
    uint_fast16_t max_reports_per_index = 0;
    /* This array keeps track of the current index to the item requested
    * in the buffering request */
    uint32_t ts_offset[SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01] = {0,};
    hal_data_cb_t *data_cb = Utility::getDataCb();

    pthread_mutex_lock(&data_cb->data_mutex);
    /* acquire wakelock to make sure system doesn't go into suspend
     * till data/indication is received by Android */
    if(data_cb->sensors_wakelock_held != true &&
        (mSensors[smgr_ind->ReportId]->getFlags() & SENSOR_FLAG_WAKE_UP)) {
        acquire_wake_lock( PARTIAL_WAKE_LOCK, SENSORS_WAKE_LOCK );
        data_cb->sensors_wakelock_held = true;
        HAL_LOG_DEBUG("%s: acquired wakelock %s", __FUNCTION__, SENSORS_WAKE_LOCK);
    }

    for (i = 0; i < smgr_ind->Indices_len; i++) {
        max_reports_per_index = MAX(max_reports_per_index,
                smgr_ind->Indices[i].SampleCount);
    }
    HAL_LOG_DEBUG("%s: Samples_len=%d Items=%d max_reports_per_index=%d",
        __FUNCTION__, smgr_ind->Samples_len, smgr_ind->Indices_len,
        max_reports_per_index);

    for (i = 0; i < max_reports_per_index; i++) {
        report_msg.ReportId = smgr_ind->ReportId;
        report_msg.status = SNS_SMGR_REPORT_OK_V01;
        /* The CurrentRate isn't used for much -- just a debug printout.
        * This conversion may be wrong if there's more than one sampling rate,
        * but the only side effect is an incorrect debug print */
        report_msg.CurrentRate = smgr_ind->Indices[0].SamplingRate;
        report_msg.Item_len = 0;
        for (j = 0; j < smgr_ind->Indices_len; j++) {
            if (i < smgr_ind->Indices[j].SampleCount) {
                report_msg.Item_len++;
                report_msg.Item[j].SensorId = smgr_ind->Indices[j].SensorId;
                report_msg.Item[j].DataType = smgr_ind->Indices[j].DataType;
                report_msg.Item[j].ItemData[0] = smgr_ind->Samples[smgr_ind->Indices[j].FirstSampleIdx+i].Data[0];
                report_msg.Item[j].ItemData[1] = smgr_ind->Samples[smgr_ind->Indices[j].FirstSampleIdx+i].Data[1];
                report_msg.Item[j].ItemData[2] = smgr_ind->Samples[smgr_ind->Indices[j].FirstSampleIdx+i].Data[2];
                ts_offset[j] += smgr_ind->Samples[smgr_ind->Indices[j].FirstSampleIdx+i].TimeStampOffset;
                report_msg.Item[j].TimeStamp = (smgr_ind->Indices[j].FirstSampleTimestamp +
                                ts_offset[j]);
                report_msg.Item[j].ItemFlags = smgr_ind->Samples[smgr_ind->Indices[j].FirstSampleIdx+i].Flags;
                report_msg.Item[j].ItemQuality = smgr_ind->Samples[smgr_ind->Indices[j].FirstSampleIdx+i].Quality;
                report_msg.Item[j].ItemSensitivity = 0;
            }
        }
        processReportInd(mSensors, &report_msg);
    }
    pthread_mutex_unlock(&data_cb->data_mutex);
}

/*===========================================================================
  FUNCTION:  processReportInd
    Process a report indication from SMGR
    Parameters
    @msensors : the pointer to the sensor class
    @smgr_ind : sensor periodic report data
===========================================================================*/
void SMGRSensor::processReportInd(Sensor** mSensors, sns_smgr_periodic_report_ind_msg_v01* smgr_ind)
{
    int handle = 0;
    hal_data_cb_t *data_cb = Utility::getDataCb();
    TimeSyncService *time_service = TimeSyncService::getTimeSyncService();
    sns_smgr_data_item_s_v01* smgr_data;
    sensors_event_t sensor_data;
    sensors_event_t *last_event;

    HAL_LOG_VERBOSE("%s: St: %d ReportId: %d Rate: %d, Len: %d", __FUNCTION__,
            smgr_ind->status, smgr_ind->ReportId,
            smgr_ind->CurrentRate, smgr_ind->Item_len);
    /* Check report status */
    if (smgr_ind->status != SNS_SMGR_REPORT_OK_V01) {
        HAL_LOG_ERROR("%s: Report Status: %u", __FUNCTION__, smgr_ind->status);
        /* Release wakelock if held */
        if (data_cb->sensors_wakelock_held == true &&
            (mSensors[smgr_ind->ReportId]->getFlags() & SENSOR_FLAG_WAKE_UP)) {
            data_cb->sensors_wakelock_held = false;
            release_wake_lock( SENSORS_WAKE_LOCK );
            HAL_LOG_DEBUG("%s: released wakelock %s", __FUNCTION__, SENSORS_WAKE_LOCK);
        }
        return;
    }
    /* the HAL requests one item per report for most sensors */
    smgr_data = &smgr_ind->Item[0];

    HAL_LOG_VERBOSE("%s: Id: %s: Ty: %u Q: %u", __FUNCTION__,
        ((smgr_data->SensorId==SNS_SMGR_ID_ACCEL_V01)     ? "ACCEL_DATA" :
        (smgr_data->SensorId==SNS_SMGR_ID_MAG_V01)        ? "MAG_DATA" :
        (smgr_data->SensorId==SNS_SMGR_ID_PROX_LIGHT_V01) ? "PROX_LIGHT_DATA" :
        (smgr_data->SensorId==SNS_SMGR_ID_GYRO_V01)       ? "GYRO_DATA" :
        (smgr_data->SensorId==SNS_SMGR_ID_PRESSURE_V01)   ? "PRESSURE_DATA" :
        (smgr_data->SensorId==SNS_SMGR_ID_STEP_EVENT_V01) ? "STEP_EVT" :
        (smgr_data->SensorId==SNS_SMGR_ID_SMD_V01)        ? "SIG_MOTION" :
        (smgr_data->SensorId==SNS_SMGR_ID_STEP_COUNT_V01) ? "STEP_CNT" :
        (smgr_data->SensorId==SNS_SMGR_ID_GAME_ROTATION_VECTOR_V01)  ? "GAME_RV" :
        (smgr_data->SensorId==SNS_SMGR_ID_HUMIDITY_V01)   ? "HUMIDITY_DATA" :
        (smgr_data->SensorId==SNS_SMGR_ID_RGB_V01)        ? "RBG_DATA" :
        (smgr_data->SensorId==SNS_SMGR_ID_SAR_V01)        ? "SAR_DATA" :
        (smgr_data->SensorId==SNS_SMGR_ID_IR_GESTURE_V01) ? "IR_GESTURE_DATA" :
        (smgr_data->SensorId==SNS_SMGR_ID_HALL_EFFECT_V01)? "HALL_EFFECT_DATA" :
            "invalid"), smgr_data->DataType, smgr_data->ItemQuality);

    if (smgr_data->ItemQuality == SNS_SMGR_ITEM_QUALITY_INVALID_NOT_READY_V01) {
        HAL_LOG_DEBUG("%s: Received invalid/not ready sample for sensor ID %i",
                      __FUNCTION__, smgr_data->SensorId);
        return;
    }
    if (smgr_data->ItemQuality == SNS_SMGR_ITEM_QUALITY_INVALID_FAILED_SENSOR_V01 ||
        smgr_data->ItemQuality == SNS_SMGR_ITEM_QUALITY_INVALID_NOT_READY_V01 ||
        smgr_data->ItemQuality == SNS_SMGR_ITEM_QUALITY_INVALID_SUSPENDED_V01) {
        HAL_LOG_ERROR("%s: Bad item quality: %u ", __FUNCTION__, smgr_data->ItemQuality);
        /* Release wakelock if held */
        if (data_cb->sensors_wakelock_held == true &&
            (mSensors[smgr_ind->ReportId]->getFlags() & SENSOR_FLAG_WAKE_UP)) {
            data_cb->sensors_wakelock_held = false;
            release_wake_lock( SENSORS_WAKE_LOCK );
            HAL_LOG_DEBUG("%s: released wakelock %s", __FUNCTION__, SENSORS_WAKE_LOCK);
        }
        return;
    }

    /* todo .. item sensitivity and item flag (rail) ?? */
    memset(&sensor_data, 0, sizeof(sensor_data));
    switch(smgr_data->SensorId)
    {
    case SNS_SMGR_ID_ACCEL_V01:
        if (SNS_SMGR_DATA_TYPE_PRIMARY_V01 == smgr_data->DataType) {
            /* call the accel sensor class */
            handle = HANDLE_ACCELERATION;
            if(smgr_ind->ReportId == HANDLE_MOTION_ACCEL) {
                /* Corresponds to screen orientation req, fill in the right type */
                sensor_data.type = SENSOR_TYPE_SCREEN_ORIENTATION;
                sensor_data.sensor = HANDLE_MOTION_ACCEL;
            }
            if(smgr_ind->ReportId == HANDLE_ACCELERATION_WAKE_UP) {
                handle = HANDLE_ACCELERATION_WAKE_UP;
            }
        }
        break;
    case SNS_SMGR_ID_MAG_V01:
        if (smgr_ind->ReportId == HANDLE_MAGNETIC_FIELD_UNCALIBRATED_SAM) {
            /* the data is reported by the HANDLE_MAGNETIC_FIELD_SAM */
            handle = HANDLE_MAGNETIC_FIELD_UNCALIBRATED_SAM;
        }
        else if (smgr_ind->ReportId == HANDLE_MAGNETIC_FIELD_UNCALIBRATED) {
            handle = HANDLE_MAGNETIC_FIELD_UNCALIBRATED;
        }
        else if (smgr_ind->ReportId == HANDLE_MAGNETIC_FIELD_UNCALIBRATED_WAKE_UP) {
            handle = HANDLE_MAGNETIC_FIELD_UNCALIBRATED_WAKE_UP;
        }
        else if (smgr_ind->ReportId == HANDLE_MAGNETIC_FIELD) {
            handle = HANDLE_MAGNETIC_FIELD;
        }
        else if (smgr_ind->ReportId == HANDLE_MAGNETIC_FIELD_WAKE_UP) {
            handle = HANDLE_MAGNETIC_FIELD_WAKE_UP;
        }
        else {
            HAL_LOG_ERROR( "%s: MAG Unknown report ID",__FUNCTION__);
        }
        break;
    case SNS_SMGR_ID_PROX_LIGHT_V01:
        if(SNS_SMGR_DATA_TYPE_PRIMARY_V01 == smgr_data->DataType) {
            if(smgr_ind->ReportId == HANDLE_PROXIMITY)
                handle = HANDLE_PROXIMITY;
            if(smgr_ind->ReportId == HANDLE_PROXIMITY_NON_WAKE_UP)
                handle = HANDLE_PROXIMITY_NON_WAKE_UP;
        } else if (SNS_SMGR_DATA_TYPE_SECONDARY_V01 == smgr_data->DataType) {
            if(smgr_ind->ReportId == HANDLE_LIGHT)
                handle = HANDLE_LIGHT;
            if(smgr_ind->ReportId == HANDLE_LIGHT_WAKE_UP)
                handle = HANDLE_LIGHT_WAKE_UP;
        }
        break;
    case SNS_SMGR_ID_GYRO_V01:
        if (smgr_ind->ReportId == HANDLE_GYRO_UNCALIBRATED) {
            handle = HANDLE_GYRO_UNCALIBRATED;
        }  else if(smgr_ind->ReportId == HANDLE_GYRO_UNCALIBRATED_WAKE_UP) {
            handle = HANDLE_GYRO_UNCALIBRATED_WAKE_UP;
        }  else if(smgr_ind->ReportId == HANDLE_GYRO) {
            handle = HANDLE_GYRO;
        }  else if(smgr_ind->ReportId == HANDLE_GYRO_WAKE_UP) {
            handle = HANDLE_GYRO_WAKE_UP;
        }
        break;
    case SNS_SMGR_ID_PRESSURE_V01:
        if (SNS_SMGR_DATA_TYPE_PRIMARY_V01 == smgr_data->DataType) {
            if(smgr_ind->ReportId == HANDLE_PRESSURE)
                handle = HANDLE_PRESSURE;
            if(smgr_ind->ReportId == HANDLE_PRESSURE_WAKE_UP)
                handle = HANDLE_PRESSURE_WAKE_UP;
        }
        break;
    case SNS_SMGR_ID_HUMIDITY_V01:
        if (SNS_SMGR_DATA_TYPE_PRIMARY_V01 == smgr_data->DataType) {
            if(smgr_ind->ReportId == HANDLE_RELATIVE_HUMIDITY)
                handle = HANDLE_RELATIVE_HUMIDITY;
            if(smgr_ind->ReportId == HANDLE_RELATIVE_HUMIDITY_WAKE_UP)
                handle = HANDLE_RELATIVE_HUMIDITY_WAKE_UP;
        }
        else if (SNS_SMGR_DATA_TYPE_SECONDARY_V01 == smgr_data->DataType) {
            if(smgr_ind->ReportId == HANDLE_AMBIENT_TEMPERATURE)
                handle = HANDLE_AMBIENT_TEMPERATURE;
            if(smgr_ind->ReportId == HANDLE_AMBIENT_TEMPERATURE_WAKE_UP)
                handle = HANDLE_AMBIENT_TEMPERATURE_WAKE_UP;
        }
        break;
    case SNS_SMGR_ID_STEP_COUNT_V01:
        if (SNS_SMGR_DATA_TYPE_PRIMARY_V01 == smgr_data->DataType) {
            if(smgr_ind->ReportId == HANDLE_SMGR_STEP_COUNT)
               handle = HANDLE_SMGR_STEP_COUNT;
            if(smgr_ind->ReportId == HANDLE_SMGR_STEP_COUNT_WAKE_UP)
               handle = HANDLE_SMGR_STEP_COUNT_WAKE_UP;
        }
        break;
    case SNS_SMGR_ID_SMD_V01:
        if (SNS_SMGR_DATA_TYPE_PRIMARY_V01 == smgr_data->DataType) {
            handle = HANDLE_SMGR_SMD;
        }
        break;
    case SNS_SMGR_ID_STEP_EVENT_V01:
        if (SNS_SMGR_DATA_TYPE_PRIMARY_V01 == smgr_data->DataType) {
            if(smgr_ind->ReportId == HANDLE_SMGR_STEP_DETECTOR)
                handle = HANDLE_SMGR_STEP_DETECTOR;
            if(smgr_ind->ReportId == HANDLE_SMGR_STEP_DETECTOR_WAKE_UP)
                handle = HANDLE_SMGR_STEP_DETECTOR_WAKE_UP;
        }
        break;
    case SNS_SMGR_ID_GAME_ROTATION_VECTOR_V01:
        if (SNS_SMGR_DATA_TYPE_PRIMARY_V01 == smgr_data->DataType) {
            if(smgr_ind->ReportId == HANDLE_SMGR_GAME_RV)
                handle = HANDLE_SMGR_GAME_RV;
            if(smgr_ind->ReportId == HANDLE_SMGR_GAME_RV_WAKE_UP)
                handle = HANDLE_SMGR_GAME_RV_WAKE_UP;
        }
        break;
    case SNS_SMGR_ID_RGB_V01:
        if (smgr_ind->ReportId == HANDLE_RGB) {
            handle = HANDLE_RGB;
        }
        break;
    case SNS_SMGR_ID_IR_GESTURE_V01:
        if (SNS_SMGR_DATA_TYPE_PRIMARY_V01 == smgr_data->DataType) {
            handle = HANDLE_IR_GESTURE;
        }
        break;
    case SNS_SMGR_ID_SAR_V01:
        if (SNS_SMGR_DATA_TYPE_PRIMARY_V01 == smgr_data->DataType) {
            handle = HANDLE_SAR;
        }
        break;
    case SNS_SMGR_ID_HALL_EFFECT_V01:
        if(SNS_SMGR_DATA_TYPE_PRIMARY_V01 == smgr_data->DataType) {
            handle = HANDLE_HALL_EFFECT;
        }
        break;
    default:
        HAL_LOG_ERROR("No such ID %d, ", smgr_data->SensorId);
        /* Release wakelock if held */
        if (data_cb->sensors_wakelock_held == true &&
            (mSensors[smgr_ind->ReportId]->getFlags() & SENSOR_FLAG_WAKE_UP)) {
            data_cb->sensors_wakelock_held = false;
            release_wake_lock( SENSORS_WAKE_LOCK );
            HAL_LOG_DEBUG("%s: released wakelock %s", __FUNCTION__, SENSORS_WAKE_LOCK);
        }
        return;
    }
    if (mSensors[handle] != NULL) {
        (static_cast<SMGRSensor*>(mSensors[handle]))->processReportInd(smgr_ind, smgr_data, sensor_data);
    }
    else {
        HAL_LOG_ERROR("%s: mSensors[handle] is NULL!", __FUNCTION__);
        /* Release wakelock if held */
        if (data_cb->sensors_wakelock_held == true &&
            (mSensors[smgr_ind->ReportId]->getFlags() & SENSOR_FLAG_WAKE_UP)) {
            data_cb->sensors_wakelock_held = false;
            release_wake_lock( SENSORS_WAKE_LOCK );
            HAL_LOG_DEBUG("%s: released wakelock %s", __FUNCTION__, SENSORS_WAKE_LOCK);
        }
        return;
    }
    last_event = (static_cast<SMGRSensor*>(mSensors[handle]))->getLastEvent();
    sensor_data.version = sizeof(sensors_event_t);
    sensor_data.timestamp = time_service->timestampCalc((uint64_t)smgr_data->TimeStamp, sensor_data.sensor);
    HAL_LOG_DEBUG("%s: handle:%d SMGR TS:%d HAL TS:%lld elapsedRealtimeNano:%lld",
                  __FUNCTION__, handle, smgr_data->TimeStamp,
                  sensor_data.timestamp, android::elapsedRealtimeNano());

    if (Latency::isLatencyMeasureEnabled() == true) {
        if (sensor_data.sensor == HANDLE_ACCELERATION ||
            sensor_data.sensor == HANDLE_MAGNETIC_FIELD ||
            sensor_data.sensor == HANDLE_MAGNETIC_FIELD_UNCALIBRATED ||
            sensor_data.sensor == HANDLE_MAGNETIC_FIELD_SAM ||
            sensor_data.sensor == HANDLE_MAGNETIC_FIELD_UNCALIBRATED_SAM ||
            sensor_data.sensor == HANDLE_PROXIMITY ||
            sensor_data.sensor == HANDLE_GYRO ||
            sensor_data.sensor == HANDLE_PRESSURE ||
            sensor_data.sensor == HANDLE_RELATIVE_HUMIDITY ||
            sensor_data.sensor == HANDLE_AMBIENT_TEMPERATURE ||
            sensor_data.sensor == HANDLE_RGB ||
            sensor_data.sensor == HANDLE_IR_GESTURE ||
            sensor_data.sensor == HANDLE_SAR) {
        /*  Right now latency measurement is supported for only one sensor at a time. Thus, when it's enabled,
            HAL will receive one sensor data everytime. Record it's DSPS ticks here for latency measurement
            purpose.*/
            Latency::setTick(smgr_data->TimeStamp);
        }
    }

    /* Check for light changes here.
    * Update the timestamp in the saved event to the current time, and then compare
    * sensor events. If the events are identical, don't report them */
    if (handle != HANDLE_SMGR_SMD) {
        last_event->timestamp = sensor_data.timestamp;
    }
    if ((handle == HANDLE_LIGHT)||(handle == HANDLE_LIGHT_WAKE_UP)){
        if (memcmp(&sensor_data, last_event, sizeof(sensor_data)) == 0) {
            return;
        }
    }
    /* backup the last sensor event */
    memcpy(last_event, &sensor_data, sizeof(sensors_event_t));

    if (Utility::insertQueue(&sensor_data)) {
        Utility::signalInd(data_cb);
    }
    return;
}

/*===========================================================================

  FUNCTION:  SMGRSensor_sensor1_cb
    Handle the sensor1 callback for the SMGR sensors.
    Parameters
    @cb_data : pointer of the callback data, SensorsContext is passed in
            this function
    @msg_hdr : sensor1 message header
    @msg_type : sensor1 message type, two major types are listed in the below:
            SENSOR1_MSG_TYPE_RESP
            SENSOR1_MSG_TYPE_IND
    @msg_ptr : sensor1 message pointer, do free this memory before return

===========================================================================*/
void SMGRSensor_sensor1_cb (intptr_t cb_data,
                     sensor1_msg_header_s *msg_hdr,
                     sensor1_msg_type_e msg_type,
                     void *msg_ptr)
{
    int handle;
    SensorsContext *context = (SensorsContext *)cb_data;
    Sensor **mSensors = context->getSensors();
    hal_sensor1_cb_t *smgr_sensor1_cb = SMGRSensor::getSMGRSensor1Cb();

    if (msg_hdr != NULL) {
        HAL_LOG_VERBOSE("%s: msg_type %d, Sn %d, msg Id %d, txn Id %d", __FUNCTION__,
            msg_type, msg_hdr->service_number, msg_hdr->msg_id, msg_hdr->txn_id );
    }
    else {
        if (msg_type != SENSOR1_MSG_TYPE_BROKEN_PIPE &&
            msg_type != SENSOR1_MSG_TYPE_REQ &&
            msg_type != SENSOR1_MSG_TYPE_RETRY_OPEN ) {
            HAL_LOG_ERROR("%s: Error - invalid msg type with NULL msg_hdr: %u",
                __FUNCTION__, msg_type);
            return;
        }
        else {
            HAL_LOG_VERBOSE("%s: msg_type %d", __FUNCTION__, msg_type);
        }
    }

    switch(msg_type) {
    case SENSOR1_MSG_TYPE_RESP_INT_ERR:
        if (msg_hdr->service_number == SNS_SMGR_SVC_ID_V01) {
            pthread_mutex_lock(&smgr_sensor1_cb->cb_mutex);
            Utility::signalResponse(true, smgr_sensor1_cb);
            pthread_mutex_unlock(&smgr_sensor1_cb->cb_mutex);
        }
        break;
    case SENSOR1_MSG_TYPE_RESP:
        if (msg_hdr->service_number == SNS_SMGR_SVC_ID_V01) {
            SMGRSensor::processResp(mSensors, msg_hdr, msg_ptr);
        }
        break;
    case SENSOR1_MSG_TYPE_IND:
        if(msg_hdr->service_number == SNS_SMGR_SVC_ID_V01) {
            SMGRSensor::processInd(mSensors, msg_hdr, msg_ptr);
        }
        break;
    case SENSOR1_MSG_TYPE_BROKEN_PIPE:
        HAL_LOG_WARN("%s: SENSOR1_MSG_TYPE_BROKEN_PIPE", __FUNCTION__);
        pthread_mutex_lock(&smgr_sensor1_cb->cb_mutex);
        Recovery::handleBrokenPipe(smgr_sensor1_cb, &SMGRSensor_sensor1_cb, cb_data);
        pthread_mutex_unlock(&smgr_sensor1_cb->cb_mutex);
        /* Re-enable all the  SMGR sensors */
        for (handle = 0; handle < SAM_HANDLE_BASE; handle++) {
            if (mSensors[handle] == NULL)
                continue;
            if (mSensors[handle]->getAttribOK()) {
                if (mSensors[handle]->getEnabled()) {
                    /* Before enable the sensor, it is better to disable the
                       sensor to reset the enabled variable */
                    mSensors[handle]->enable(0);
                    /* Re-enable the sensor */
                    mSensors[handle]->enable(1);
                }
            }
        }
        break;
    case SENSOR1_MSG_TYPE_RETRY_OPEN:
        HAL_LOG_WARN("%s: SENSOR1_MSG_TYPE_RETRY_OPEN", __FUNCTION__);
        pthread_mutex_lock(&smgr_sensor1_cb->cb_mutex);
        Recovery::reInit(smgr_sensor1_cb, &SMGRSensor_sensor1_cb, cb_data);
        pthread_mutex_unlock(&smgr_sensor1_cb->cb_mutex);
        break;
    case SENSOR1_MSG_TYPE_REQ:
    default:
        HAL_LOG_ERROR("%s: Error - invalid msg type in cb: %u", __FUNCTION__, msg_type);
        break;
    }

    pthread_mutex_lock(&smgr_sensor1_cb->cb_mutex);
    if (NULL != msg_ptr && smgr_sensor1_cb->sensor1_handle) {
        sensor1_free_msg_buf(smgr_sensor1_cb->sensor1_handle, msg_ptr);
    }
    pthread_mutex_unlock(&smgr_sensor1_cb->cb_mutex);
    return;
}
