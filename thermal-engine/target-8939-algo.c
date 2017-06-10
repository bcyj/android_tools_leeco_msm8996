/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/time.h>
#include <stringl.h>
#include <signal.h>
#include "thermal.h"
#include "sensors_manager.h"
#include "devices_manager.h"

/* Place holder for target specific custom algorithms. */

static device_clnt_handle cluster0_clnt; /* Power cluster */
static device_clnt_handle cluster1_clnt; /* Perf cluster */

#define PERF_CLUSTER_MAX_NOMINAL_FREQ 1113600
#define PERF_CLUSTER_MAX_SVS_FREQ 400000
#define POWER_CLUTER_MAX_NOMINAL_FREQ 800000
#define POWER_CLUTER_MAX_SVS_FREQ 250000

static void cluster1_cb(device_clnt_handle clnt,
			union device_request *req,
			void *notify_cb_data)
{
	union device_request local_req;

	dbgmsg("%s: req.value %d.", __func__, req->value);
	if (req->value > PERF_CLUSTER_MAX_NOMINAL_FREQ) {
		device_clnt_cancel_request(cluster0_clnt);
	} else if (req->value > PERF_CLUSTER_MAX_SVS_FREQ) {
		local_req.value = POWER_CLUTER_MAX_NOMINAL_FREQ;
		device_clnt_request(cluster0_clnt, &local_req);
	} else if(req->value <= PERF_CLUSTER_MAX_SVS_FREQ) {
		local_req.value = POWER_CLUTER_MAX_SVS_FREQ;
		device_clnt_request(cluster0_clnt, &local_req);
	}
}

void target_algo_init(void)
{
	int err = 0;
	if ((therm_get_msm_id() != THERM_MSM_8939) &&
		(therm_get_msm_id() != THERM_MSM_8929))
		return;

	cluster0_clnt = devices_manager_reg_clnt("cluster0");
	if (cluster0_clnt == NULL) {
			msg("%s: Failed to create cluster0 client.", __func__);
			return;
	}

	cluster1_clnt = devices_manager_reg_clnt("cluster1");
	if (cluster1_clnt == NULL) {
			msg("%s: Failed to create cluster1 client.", __func__);
			return;
	}

	err = devices_manager_reg_notify(cluster1_clnt, cluster1_cb, NULL);
	if (err < 0) {
		msg("%s: Failed to reg_notify cluster0 client.", __func__);
		return;
	}
}
