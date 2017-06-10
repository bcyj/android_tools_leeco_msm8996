/*
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _LIGHTSENSOR_H
#define _LIGHTSENSOR_H

#ifdef __cplusplus
extern "C" {
#include "sensor1.h"
#include "sns_common_v01.h"
#include "sns_smgr_api_v01.h"
#include "sns_sam_sensor_thresh_v01.h"
}
#endif

#include "als.h"

class LightSensor: public ALS {
    sensor1_handle_s *mALSHndl;
    int algo_instance_id;
    bool mDisableResponse;
    bool mEnableResponse;

    void ProcessLightSensorWork(sensor1_msg_header_s *msg_hdr_ptr, sensor1_msg_type_e msg_type, void *msg_ptr);
    static void als_cb_func(intptr_t data, sensor1_msg_header_s *msg_hdr_ptr, sensor1_msg_type_e msg_type, void *msg_ptr) {
        reinterpret_cast<LightSensor*>(data)->ProcessLightSensorWork(msg_hdr_ptr, msg_type, msg_ptr);
    }

public:
    LightSensor() : mALSHndl(NULL), mDisableResponse(false), mEnableResponse(false) {}
    int ALSInfoQuery();
    int ALSRegister();
    int ALSDeRegister();
    int ALSRun(bool isFirstRun);
    int ALSCleanup();
    int ALSPauseControl(bool);
    int ALSReadSensor();
};

#endif /* _LIGHTSENSOR_H */
