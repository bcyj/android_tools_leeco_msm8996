/*===========================================================================

  Copyright (c) 2012,2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef __QMI_TS_SENSOR_H__
#define __QMI_TS_SENSOR_H__

#include "sensors_manager_internal.h"

int  qmi_ts_setup(struct sensor_info *sensor);
void qmi_ts_shutdown(struct sensor_info *sensor);
int  qmi_ts_get_temperature(struct sensor_info *sensor);
void qmi_ts_interrupt_wait(struct sensor_info *sensor);
void qmi_ts_update_thresholds(struct sensor_info *sensor, struct thresholds_req_t *thresh);

#endif  /* __QMI_TS_SENSOR_H__ */
