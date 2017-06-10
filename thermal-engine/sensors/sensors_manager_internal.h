/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef __SENSOR_MANAGER_INTERNAL_H__
#define __SENSOR_MANAGER_INTERNAL_H__

#include <stdint.h>
#include <pthread.h>

#include "sensors_manager.h"

struct sensors_mgr_sensor_info;

/* Common sensor struct */
struct sensor_info {
	/* sensor name */
	char *name;
	/* setup function needs to be called before get_temperature */
	int (*setup)(struct sensor_info *sensor);
	/* shutdown function for resource cleanup */
	void (*shutdown)(struct sensor_info *sensor);
	/* get_temperature function to query sensor reading */
	int (*get_temperature)(struct sensor_info *sensor);
	/* wait on threshold interrupt */
	void (*interrupt_wait)(struct sensor_info *sensor);
	/* update sensor thresholds */
	void (*update_thresholds)(struct sensor_info *sensor,
				  struct thresholds_req_t *thresh);
	/* sysfs thermal zone number */
	int tzn;
	/* misc data */
	void *data;
	/* Sensor Mgr*/
	struct sensors_mgr_sensor_info *sensor_mgr;
	/* enable interrupt */
	int interrupt_enable;
	/* File descriptor */
	int fd;
	/* sensor value scaling factor */
	int scaling_factor;
};

struct sensor_client_type {
	struct sensors_mgr_sensor_info *sensor_mgr;
	uint8_t request_active;
	struct sensor_thresh_req request;
	struct sensor_client_type *next_clnt;
	sensor_notify_cb_func cb_func;
	void *cb_usr_data;
};

struct sensors_mgr_sensor_info {
	char *name;
	char *alias;
	int last_reading;
	struct sensor_thresh_req active_thresh;
	uint8_t active_thresh_valid;
	struct sensor_client_type *client_list;
	struct sensors_mgr_sensor_info *next_sensor;
	void *data;
	int interrupt_enable;
	pthread_t monitor_thread;
	pthread_mutex_t req_wait_mutex;
	pthread_cond_t req_wait_cond;
	uint8_t req_active;
	int thread_shutdown;
	void (*wait)(struct sensors_mgr_sensor_info *sensor); /* Optional */
	int (*get_temperature)(struct sensors_mgr_sensor_info *sensor);
	void (*update_thresholds)(struct sensors_mgr_sensor_info *sensor); /* Optional */
	void (*shutdown)(struct sensors_mgr_sensor_info *sensor);
	uint32_t default_polling_interval;
};

/* Sensor manager functions */
int sensors_manager_add_sensor(struct sensors_mgr_sensor_info *sensor_info);

int sensors_manager_rm_sensor(struct sensors_mgr_sensor_info *sensor_info);
#endif /* __SENSOR_MANAGER_INTERNAL_H__ */
