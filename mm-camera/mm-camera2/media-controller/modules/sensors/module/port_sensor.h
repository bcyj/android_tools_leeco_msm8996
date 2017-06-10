/* port_sensor.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __PORT_SENSOR_H__
#define __PORT_SENSOR_H__

#include "sensor_common.h"

#define SENSOR_MEASURE_FPS 0

boolean port_sensor_create(void *data, void *user_data);
boolean port_sensor_store_frame_control(module_sensor_bundle_info_t* s_bundle,
    uint32_t future_frame_id, sensor_frame_ctrl_type_t type, void *data);

#endif /* __PORT_SENSOR_H__ */
