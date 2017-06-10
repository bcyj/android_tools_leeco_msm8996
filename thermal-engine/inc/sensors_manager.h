/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef __SENSORS_MANAGER_H__
#define __SENSORS_MANAGER_H__

#include <stdint.h>

struct thresholds_req_t {
	uint8_t high_valid;
	int     high; /* milli degrees C units */
	uint8_t low_valid;
	int     low; /* milli degrees C units */
};

typedef struct sensor_client_type *sensor_clnt_handle;

#define SENSOR_DEFAULT_POLLING_INTERVAL 10000  /* 10 seconds */

#define MAX_SENSOR_NAME_LEN 32

struct sensor_info_type {
	char name[MAX_SENSOR_NAME_LEN];
};

enum sensor_notify_event_type {
	SENSOR_NOTIFY_LOW_THRESH_EVENT = 0,
	SENSOR_NOTIFY_NORMAL_THRESH_EVENT,
	SENSOR_NOTIFY_HIGH_THRESH_EVENT,
};

typedef void (*sensor_notify_cb_func) (sensor_clnt_handle  clnt,
				    enum sensor_notify_event_type   event,
				    int                    reading,
				    void                  *notify_cb_data);

struct sensor_thresh_req {
	sensor_notify_cb_func notify_cb_func;
	void             *notify_cb_data;
	struct thresholds_req_t  thresh;
	uint8_t           polling_interval_valid;
	uint32_t          polling_interval; /* milliseconds units */
};

/* Sensor Manager init function */
int sensors_manager_init(void);

/* Sensor Manager release/exit function */
void sensors_manager_release(void);

/* Sensor get list */
int sensors_manager_get_list(struct sensor_info_type *info_arr, uint32_t *info_arr_len);

int sensors_get_alias(char *sensor_name, char *alias_dest);

/* Sensor register client */
sensor_clnt_handle sensors_manager_reg_clnt(const char *sensor_name);

/* Sensor de-register client */
int sensors_manager_dereg_clnt(sensor_clnt_handle clnt);

/* Sensor read */
int sensors_manager_read(sensor_clnt_handle clnt);

/* Sensor set threshold levels */
int sensors_manager_set_thresh_lvl(sensor_clnt_handle clnt,
		       struct sensor_thresh_req *thresh_info);
#endif /* __SENSORS_MANAGER_H__ */
