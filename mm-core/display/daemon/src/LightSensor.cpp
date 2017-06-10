/*
 * DESCRIPTION
 * This file contains the ambient light sensor functionalities
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#include "LightSensor.h"

#define LOG_TAG "LightSensor"

int LightSensor::ALSRun(bool isFirstRun)
{
    sensor1_error_e e = SENSOR1_EFAILED;
    sensor1_msg_header_s mMsg_hdr;
    sns_sam_sensor_thresh_enable_req_msg_v01 *mALS_enable_req;
    int thresh = isFirstRun ? 0 : 1;

    if (!mSampleRateSet) {// if we have receive the sensor info
        LOGE("%s(): Query for sensor_info has not returned", __func__);
        return e;
    }

    mMsg_hdr.service_number = SNS_SAM_SENSOR_THRESH_SVC_ID_V01;
    mMsg_hdr.msg_id = SNS_SAM_SENSOR_THRESH_ENABLE_REQ_V01;
    mMsg_hdr.msg_size = sizeof(sns_sam_sensor_thresh_enable_req_msg_v01);
    mMsg_hdr.txn_id = 0;

    e = sensor1_alloc_msg_buf(mALSHndl,
            sizeof(sns_sam_sensor_thresh_enable_req_msg_v01),
            (void**)&mALS_enable_req);
    if (SENSOR1_SUCCESS != e) {
        LOGE("%s(): sensor1_alloc_msg_buf returned %d", __func__, e);
        return e;
    }

    mALS_enable_req->sensor_id = SNS_SMGR_ID_PROX_LIGHT_V01;
    mALS_enable_req->data_type = SNS_SMGR_DATA_TYPE_SECONDARY_V01;
    mALS_enable_req->sample_rate = SELECT_SAMPLE_RATE(mALSSampleRate) << ALS_SHIFT_BIT;
    mALS_enable_req->threshold[0] = (thresh << ALS_SHIFT_BIT);
    mALS_enable_req->notify_suspend_valid = TRUE;
    mALS_enable_req->notify_suspend.proc_type = SNS_PROC_APPS_V01;
    mALS_enable_req->notify_suspend.send_indications_during_suspend = FALSE;

    LOGD_IF(mDebug, "Sample rate sent to sensor = %d", SELECT_SAMPLE_RATE(mALSSampleRate));
    e = sensor1_write(mALSHndl, &mMsg_hdr, mALS_enable_req);
    if (SENSOR1_SUCCESS != e) {
        LOGE("%s(): sensor1_write returned %d", __func__, e);
        sensor1_free_msg_buf(mALSHndl, mALS_enable_req);
        return e;
    }

    pthread_mutex_lock(&mALSLock);
    while (!mEnableResponse)
        pthread_cond_wait(&mALSCond, &mALSLock);
    mEnableResponse = false;
    pthread_mutex_unlock(&mALSLock);

    return 0;
}

int LightSensor::ALSReadSensor() {
    pthread_mutex_lock(&mALSLock);
    if(!mALSReady) {
        pthread_cond_wait(&mALSCond, &mALSLock);
    }
    mALSReady = 0;
    pthread_mutex_unlock(&mALSLock);

    return mALSValue;
}

int LightSensor::ALSInfoQuery(){

    LOGD_IF(mDebug, "%s(): Entering", __func__);
    sensor1_error_e e = SENSOR1_EFAILED;
    sensor1_msg_header_s smgr_signle_info_req_hdr;
    sns_smgr_single_sensor_info_req_msg_v01 *smgr_signle_info_req_msg;

    smgr_signle_info_req_hdr.service_number = SNS_SMGR_SVC_ID_V01;
    smgr_signle_info_req_hdr.msg_id = SNS_SMGR_SINGLE_SENSOR_INFO_REQ_V01;
    smgr_signle_info_req_hdr.msg_size = sizeof(sns_smgr_single_sensor_info_req_msg_v01);
    smgr_signle_info_req_hdr.txn_id = 12 + SNS_SMGR_ID_PROX_LIGHT_V01;

    e = sensor1_alloc_msg_buf(mALSHndl, sizeof(sns_smgr_single_sensor_info_req_msg_v01),
                              (void**)&smgr_signle_info_req_msg);
    if (SENSOR1_SUCCESS != e) {
        LOGE("%s(): sensor1_alloc_msg_buf returned %d", __func__, e);
        return  e;
    }

    smgr_signle_info_req_msg->SensorID = SNS_SMGR_ID_PROX_LIGHT_V01;

    //send the query message
    e = sensor1_write(mALSHndl, &smgr_signle_info_req_hdr, smgr_signle_info_req_msg);
    if (SENSOR1_SUCCESS != e) {
        LOGE("%s(): sensor1_write returned %d", __func__, e);
        sensor1_free_msg_buf(mALSHndl, smgr_signle_info_req_msg);
        return e;
    }

    pthread_mutex_lock(&mALSLock);
    while (!mSampleRateSet)
        pthread_cond_wait(&mALSCond, &mALSLock);
    pthread_mutex_unlock(&mALSLock);

    LOGD_IF(mDebug, "%s(): Exiting",__func__);
    return 0;
}

int LightSensor::ALSRegister(){
    sensor1_error_e e = SENSOR1_EFAILED;

    LOGD_IF(mDebug, "%s(): Entering", __func__);
    e = sensor1_init();
    if (e)
        return e;

    e = sensor1_open(&mALSHndl, LightSensor::als_cb_func, reinterpret_cast<intptr_t>(this));

    if (SENSOR1_SUCCESS != e) {
        LOGE("%s(): sensor1_open returned %d", __func__, e);
        return e;
    }

    if (ALSInfoQuery()) {
        LOGE("%s(): ALSInfoQuery failed", __func__);
        e = sensor1_close(mALSHndl);
        if(SENSOR1_SUCCESS != e) {
            LOGE("%s(): sensor1_close returned %d", __func__, e);
            return e;
        }
        return -1;
    }

    return 0;
}

int LightSensor::ALSDeRegister(){
    LOGD_IF(mDebug, "%s(): Entering", __func__);
    sensor1_error_e e = SENSOR1_EFAILED;
    sensor1_msg_header_s mMsg_hdr;
    sns_sam_sensor_thresh_disable_req_msg_v01 *mALS_disable_req;

    e = sensor1_alloc_msg_buf(mALSHndl,
            sizeof(sns_sam_sensor_thresh_disable_req_msg_v01),
            (void**)&mALS_disable_req);
    if (SENSOR1_SUCCESS != e) {
        LOGE("%s(): sensor1_alloc_msg_buf returned %d", __func__, e);
        return e;
    }

    mMsg_hdr.service_number = SNS_SAM_SENSOR_THRESH_SVC_ID_V01;
    mMsg_hdr.msg_id = SNS_SAM_SENSOR_THRESH_DISABLE_REQ_V01;
    mMsg_hdr.msg_size = sizeof(sns_sam_sensor_thresh_disable_req_msg_v01);
    mMsg_hdr.txn_id = 0;
    mALS_disable_req->instance_id = algo_instance_id;

    e = sensor1_write(mALSHndl, &mMsg_hdr, mALS_disable_req);
    if(SENSOR1_SUCCESS != e) {
        LOGE("%s(): sensor1_write returned %d", __func__, e);
        sensor1_free_msg_buf(mALSHndl, mALS_disable_req);
        return e;
    }

    pthread_mutex_lock(&mALSLock);
    while (!mDisableResponse)
        pthread_cond_wait(&mALSCond, &mALSLock);
    mDisableResponse = false;
    pthread_mutex_unlock(&mALSLock);

    LOGD_IF(mDebug, "%s(): Exiting",__func__);
    return 0;
}

void LightSensor::ProcessLightSensorWork(sensor1_msg_header_s *msg_hdr_ptr, sensor1_msg_type_e msg_type, void *msg_ptr) {

    LOGD_IF(mDebug, "%s:Entering", __func__);

    if (NULL == msg_hdr_ptr) {
        LOGE("Received NULL msg_hdr_ptr!");
        return;
    }

    if (NULL == msg_ptr) {
        LOGE("Received NULL msg_body!");
        return;
    }

    switch (msg_type) {
        case SENSOR1_MSG_TYPE_RESP:
            {
                if (msg_hdr_ptr->msg_id == SNS_SAM_SENSOR_THRESH_ENABLE_REQ_V01) {
                    sns_sam_sensor_thresh_enable_resp_msg_v01 *resp_ptr =
                             (sns_sam_sensor_thresh_enable_resp_msg_v01*) msg_ptr;
                    LOGD_IF(mDebug, "Got ENABLE response, status = %d, id = %d",
                            resp_ptr->resp.sns_result_t, resp_ptr->instance_id);
                    algo_instance_id = resp_ptr->instance_id;
                    pthread_mutex_lock(&mALSLock);
                    mEnableResponse = true;
                    pthread_mutex_unlock(&mALSLock);
                } else if (msg_hdr_ptr->msg_id == SNS_SAM_SENSOR_THRESH_DISABLE_REQ_V01) {
                    sns_sam_sensor_thresh_disable_resp_msg_v01 *resp_ptr =
                            (sns_sam_sensor_thresh_disable_resp_msg_v01*) msg_ptr;
                    pthread_mutex_lock(&mALSLock);
                    mDisableResponse = true;
                    pthread_mutex_unlock(&mALSLock);
                    LOGD_IF(mDebug, "Got DISABLE response, status = %d, id = %d",
                            resp_ptr->resp.sns_result_t, resp_ptr->instance_id);
                } else if (msg_hdr_ptr->msg_id == SNS_SMGR_SINGLE_SENSOR_INFO_RESP_V01) {
                    LOGD_IF(mDebug, "Got SENSOR_INFO response!");
                    sns_smgr_single_sensor_info_resp_msg_v01 *info =
                        (sns_smgr_single_sensor_info_resp_msg_v01 *) msg_ptr;
                    uint32_t info_len = info->SensorInfo.data_type_info_len;
                    for (int i = 0; i < info_len; i++) {
                        if (info->SensorInfo.data_type_info[i].DataType == SNS_SMGR_DATA_TYPE_SECONDARY_V01) {
                            mALSSampleRate =
                                info->SensorInfo.data_type_info[1].MaxSampleRate;
                            mSampleRateSet = true;
                            LOGD_IF(mDebug, "mALSSampleRate  = %d", mALSSampleRate );
                        }
                    }
                } else {
                    LOGE("%s(): Got INVALID response!!!", __func__);
                }
                pthread_mutex_lock(&mALSLock);
                if (mDisableResponse) {
                    mALSReady = 1;
                }
                pthread_cond_broadcast(&mALSCond);
                pthread_mutex_unlock(&mALSLock);
            }
            break;

        case SENSOR1_MSG_TYPE_IND:
            {
                if (msg_hdr_ptr->msg_id == SNS_SAM_SENSOR_THRESH_REPORT_IND_V01) {

                    LOGD_IF(mDebug, "Received indication from sensor framework!");
                    sns_sam_sensor_thresh_report_ind_msg_v01 *ind_ptr =
                            (sns_sam_sensor_thresh_report_ind_msg_v01*) msg_ptr;
                    pthread_mutex_lock(&mALSLock);
                    if (!mALSPaused) {
                        mALSValue =  (ind_ptr->sample_value[0] >> ALS_SHIFT_BIT);
                        mALSReady = 1;
                        pthread_cond_broadcast(&mALSCond);
                    }
                    pthread_mutex_unlock(&mALSLock);

                } else {
                    LOGE("%s(): Got INVALID indication!!!", __func__);
                }
            }
            break;

        default:
            LOGE("%s(): Received invalid message from sensor framework!", __func__);
            break;
    }

    if (NULL != msg_ptr) {
        sensor1_free_msg_buf(mALSHndl, msg_ptr);
    }

    LOGD_IF(mDebug, "%s:Exiting", __func__);
    return;
}

int LightSensor::ALSCleanup() {
    LOGD_IF(mDebug, "%s(): Entering",__func__);
    sensor1_error_e e = SENSOR1_EFAILED;
    mSampleRateSet = false;
    e = sensor1_close(mALSHndl);
    if(SENSOR1_SUCCESS != e) {
        LOGE("%s(): sensor1_close returned %d", __func__, e);
    }

    LOGD_IF(mDebug, "%s(): Exiting",__func__);
    return e;

}

int LightSensor::ALSPauseControl(bool isPaused) {
    LOGD_IF(mDebug, "%s(): Entering",__func__);
    pthread_mutex_lock(&mALSLock);
    mALSPaused = isPaused;
    pthread_mutex_unlock(&mALSLock);

    LOGD_IF(mDebug, "%s(): Exiting",__func__);
    return 0;
}

