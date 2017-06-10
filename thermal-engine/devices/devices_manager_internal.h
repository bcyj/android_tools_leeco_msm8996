/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef __DEVICES_MANAGER_INTERNAL_H__
#define __DEVICES_MANAGER_INTERNAL_H__

#include <stdint.h>
#include "devices_manager.h"

/* TMD manager types */
struct devices_manager_dev;

struct device_clnt {
	struct devices_manager_dev *dev_mgr;
	uint8_t request_active;
	union device_request request;
	struct device_clnt *next_clnt;
	device_notify_cb_func cb_func;
	void *cb_usr_data;
};

struct devices_manager_dev {
	struct device_info dev_info;
	struct device_lvl_info *lvl_info;
	union device_request active_req;
	struct device_clnt *client_list;
	struct devices_manager_dev *next_dev;
	int (*action)(struct devices_manager_dev *dev_mgr);
	void (*release)(struct devices_manager_dev *dev_mgr);
	void *data;
};

typedef int (*device_action)(struct devices_manager_dev *dev_mgr);

/* Device manager functions */
int devices_manager_add_dev(struct devices_manager_dev *dev_mgr);

int devices_manager_rm_dev(struct devices_manager_dev *dev_mgr);

#endif /* __DEVICES_MANAGER_INTERNAL_H__ */
