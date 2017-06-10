/*
 *mdm_helper: A module to monitor modem activities on fusion devices
 *
 *Copyright (C) 2012,2014 Qualcomm Technologies, Inc. All rights reserved.
 *                   Qualcomm Technologies Proprietary/GTDR
 *
 *All data and information contained in or disclosed by this document is
 *confidential and proprietary information of Qualcomm Technologies, Inc. and all
 *rights therein are expressly reserved.  By accepting this material the
 *recipient agrees that this material and the information contained therein
 *is held in confidence and in trust and will not be used, copied, reproduced
 *in whole or in part, nor its contents revealed in any manner to others
 *without the express written permission of Qualcomm Technologies, Inc.
 *
 *mdm_helper_private.h : Header file needed by mdm_helper
 */
#include "mdm9k_esoc.h"
#include "mdm_helper.h"

#define MDM_POLL_DELAY              (500)
#define NUM_RETRIES                 (100)
#define BASEBAND_NAME_LENGTH 30

typedef enum MdmHelperStates {
	MDM_HELPER_STATE_POWERUP = 0,
	MDM_HELPER_STATE_POST_POWERUP,
	MDM_HELPER_STATE_RAMDUMP,
	MDM_HELPER_STATE_REBOOT,
	MDM_HELPER_STATE_SHUTDOWN,
	MDM_HELPER_STATE_FAIL
} MdmHelperStates;

struct mdm_helper_ops mdm9k_ops = {
	.power_up = mdm9k_powerup,
	.shutdown = mdm9k_shutdown,
	.post_power_up = mdm9k_monitor,
	.reboot = NULL,
	.prep_for_ramdumps = NULL,
	.collect_ramdumps = mdm9k_ramdump_collect,
	.post_ramdump_collect = NULL,
	.failure_cleanup = mdm9k_cleanup,
	.standalone_mode = NULL,
};
