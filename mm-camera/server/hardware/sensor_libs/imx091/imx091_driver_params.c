/*============================================================================
SENSOR DRIVER PARAMS

DESCRIPTION

Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
Export of this technology or software is regulated by the U.S. Government.
Diversion contrary to U.S. law prohibited.
============================================================================*/
#ifndef SENSOR_DRIVER_PARAMS_TYPE_H_
#define SENSOR_DRIVER_PARAMS_TYPE_H_
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "camera_dbg.h"
#include "sensor_util_bayer.h"
#include "sensor.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#include "imx091_params.h"


/*============================================================================
*                         CONSTANTS
============================================================================*/


static struct sensor_driver_params_type imx091_driver_params = {
		&sensor_init_conf[0],
		ARRAY_SIZE(sensor_init_conf),
		&sensor_confs[0],
		ARRAY_SIZE(sensor_confs),
		&sensor_reg_addr,
		&sensor_start_settings_array[0],
		&sensor_stop_settings_array[0],
		&sensor_groupon_settings_array[0],
		&sensor_groupoff_settings_array[0],
		&sensor_exp_gain_info,
		&sensor_dimensions[0],
};

void load_driver_params(void *ctrl)
{
	sensor_ctrl_t *sctrl = (sensor_ctrl_t *) ctrl;
  sctrl->driver_params = &imx091_driver_params;
}
#endif /* SENSOR_DRIVER_PARAMS_TYPE_H_ */
