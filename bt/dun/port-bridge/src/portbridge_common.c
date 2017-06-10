/******************************************************************************

  @file    portbridge_common.c
  @brief   Main Portbridge Module

  DESCRIPTION
  Implementation of the Main Process thread for the port-bridge process.
  This process listens to events on the core, platform and ipc state pipes
  and processes them to pass them onto their appropriate state machines.

 ******************************************************************************/
/*===========================================================================

  Copyright (c) 2010,2012-2013 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

  ===========================================================================*/
/*Including necessary header files*/
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <cutils/properties.h>


/*Header File Declarations*/
#include "portbridge_common.h"
#include "platform_call_arb_kevents.h"
#include "portbridge_ext_host_mon.h"
#include "portbridge_core_xfer.h"
#include "platform_call_arb.h"
#include "portbridge_core.h"

#define FAIL_TO_CREATE -1

#define PB_DUN_PROP_BASEBAND                    "ro.baseband"
#define PB_DUN_PROP_BASEBAND_VALUE_MDMUSB       "mdm"
#define PB_DUN_PROP_BASEBAND_VALUE_MDM2USB      "mdm2"
#define PB_DUN_PROP_BASEBAND_VALUE_MSM          "msm"
#define PB_DUN_PROP_TARGET_PLATFORM             "ro.board.platform"
#define PB_DUN_PROP_TARGET_PLATFORM_APQ8084     "apq8084"
#define PB_DUN_PROP_TARGET_PLATFORM_MSM8994     "msm8994"
#define PB_DUN_PROP_TARGET_PLATFORM_MSM8916     "msm8916" /* For 8916, 8939 */
#define PB_DUN_PROP_TARGET_PLATFORM_MSM8909     "msm8909" /* For 8909 */

#define PB_DUN_MODEM_PORT_APQ8084               "/dev/ttyUSB0"

/* Macros representing physical network devices */
#define PB_DUN_PHYS_NET_DEV_RMNET               "rmnet0"
#define PB_DUN_PHYS_NET_DEV_RMNET_USB           "rmnet_usb0"
#define PB_DUN_PHYS_NET_DEV_RMNET_DATA_0        "rmnet_data0"

/*Port Parameter Variable*/
dun_portparams_s pb_dun_portparams;

/* Physical network device name */
char pb_dun_phys_net_dev_name[MAX_PHY_NET_DEV_NAME_LEN];

#undef LOGD
#undef LOGE
#undef LOGI
#undef LOGV

#define LOGD ALOGD
#define LOGI ALOGI
#define LOGE ALOGE
#define LOGV ALOGV

/*The mutexes associated with pipe*/
static pthread_mutex_t pipe_mutex;

/*===========================================================================

FUNCTION     : pipe_lock

DESCRIPTION  : This function is wrapper function for the pipe call.It makes
               sure all the pipe calls should go in sequencial order.This
               function will be called during the initialization of dun
               server by platform and core SM threads.

DEPENDENCIES : None

RETURN VALUE : On success, zero is returned. On error, -1 is returned

============================================================================*/

int pipe_lock(int fildes[2])
{
    int status;
    port_log_dflt("%s locking ", __func__);
    pthread_mutex_lock (&pipe_mutex);
    usleep(500*1000);
    status = pipe(fildes);
    pthread_mutex_unlock (&pipe_mutex);
    port_log_dflt("%s unlocked ", __func__);
    return status;
}

/*===========================================================================

FUNCTION     : pb_main_exit_handler

DESCRIPTION  : This function does port monitor thread exit handling

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/

static void pb_main_exit_handler(int sig)
{
    port_log_high("%s>", __func__);
    /*Stop platform thread*/
    stop_platform_sm_thread();

    /*Stop the monitoring threads*/
    pb_stop_conn_mon_thread();
    pb_stop_rmnet_mon_thread();

    port_log_high("%<s", __func__);
    return;
}

/*===========================================================================

FUNCTION     : main

DESCRIPTION  : This function is the main thread of the port-bridge execution.
It spawns of the rmnet & usb monitoring threads as well as
CORE SM and PLATFORM SM threads.

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
int main(int argc, char *argv[])
{
    struct sigaction actions;
    char prop_value[2][PROPERTY_VALUE_MAX];

    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    int rc = 0;
    actions.sa_handler = pb_main_exit_handler;
    if (sigaction(SIGTERM,&actions,NULL) < 0) {
        LOGE("Error in sigaction in %s:  %s\n", __func__, strerror(errno));
        return -1;
    }

    port_log_high("Port-Bridge Starting up...");

    /*Check for valid number of arguments*/
    if(argc != 3) {
        port_log_err("Error Usage: %s <SMD port dev node><USB serial port dev node>\n",
                argv[0]);
        return -1;
    }

    /* Find out baseband  & target type*/
    memset(prop_value, 0x00, sizeof(prop_value));
    property_get(PB_DUN_PROP_BASEBAND, prop_value[0], "");
    property_get(PB_DUN_PROP_TARGET_PLATFORM, prop_value[1], "");
    LOGI("prop_value[0]: %s, prop_value[1]: %s", prop_value[0], prop_value[1]);

    /*Setting the values of the smd and ext port names*/
    memset(pb_dun_portparams.smdportfname, 0, sizeof(pb_dun_portparams.smdportfname));

    if ((strcmp(PB_DUN_PROP_BASEBAND_VALUE_MDMUSB, prop_value[0]) == 0)
        && (strcmp(PB_DUN_PROP_TARGET_PLATFORM_APQ8084, prop_value[1]) == 0)) {
        strlcpy(pb_dun_portparams.smdportfname, PB_DUN_MODEM_PORT_APQ8084,
                sizeof(pb_dun_portparams.smdportfname));
        strlcpy(pb_dun_phys_net_dev_name, PB_DUN_PHYS_NET_DEV_RMNET_USB,
                sizeof(pb_dun_phys_net_dev_name));
    } else if ((strcmp(PB_DUN_PROP_BASEBAND_VALUE_MDM2USB, prop_value[0]) == 0)
               && (strcmp(PB_DUN_PROP_TARGET_PLATFORM_APQ8084, prop_value[1]) == 0)) {
        strlcpy(pb_dun_portparams.smdportfname, PB_DUN_MODEM_PORT_APQ8084,
                sizeof(pb_dun_portparams.smdportfname));
        strlcpy(pb_dun_phys_net_dev_name, PB_DUN_PHYS_NET_DEV_RMNET_DATA_0,
                sizeof(pb_dun_phys_net_dev_name));
    } else if ((strcmp(PB_DUN_PROP_BASEBAND_VALUE_MSM, prop_value[0]) == 0)
               && ((strcmp(PB_DUN_PROP_TARGET_PLATFORM_MSM8994, prop_value[1]) == 0)
               || (strcmp(PB_DUN_PROP_TARGET_PLATFORM_MSM8916, prop_value[1]) == 0)
               || (strcmp(PB_DUN_PROP_TARGET_PLATFORM_MSM8909, prop_value[1]) == 0))) {
        strlcpy(pb_dun_portparams.smdportfname, argv[1], sizeof(pb_dun_portparams.smdportfname));
        strlcpy(pb_dun_phys_net_dev_name, PB_DUN_PHYS_NET_DEV_RMNET_DATA_0,
                sizeof(pb_dun_phys_net_dev_name));
    } else {
        strlcpy(pb_dun_portparams.smdportfname, argv[1], sizeof(pb_dun_portparams.smdportfname));
        strlcpy(pb_dun_phys_net_dev_name, PB_DUN_PHYS_NET_DEV_RMNET,
                sizeof(pb_dun_phys_net_dev_name));
    }

    LOGI("Modem Port: %s, Physical network device: %s", pb_dun_portparams.smdportfname,
         pb_dun_phys_net_dev_name);

    pthread_mutex_init(&pipe_mutex, NULL);

    /*Create Connection Monitor Thread*/
    if (pb_start_conn_mon_thread() < 0) {
        port_log_err("Unable to create conn event thread : %s\n", strerror(errno));
        return FAIL_TO_CREATE;
    }

    /*Create kernel RMNET thread */
    if (pb_start_rmnet_mon_thread() < 0) {
        port_log_err("Unable to create rmnet event thread : %s\n", strerror(errno));
        pb_stop_conn_mon_thread();
        return FAIL_TO_CREATE;
    }

    /*Create Core SM Thread*/
    if (start_core_sm_thread() < 0) {
        port_log_err("Unable to create CORE SM thread : %s\n", strerror(errno));
        pb_stop_conn_mon_thread();
        pb_stop_rmnet_mon_thread();
        return FAIL_TO_CREATE;
    }

    /*Create Platform SM Thread*/
    if (start_platform_sm_thread() < 0) {
        port_log_err("Unable to create PLATFORM SM thread : %s\n", strerror(errno));
        pb_stop_conn_mon_thread();
        pb_stop_rmnet_mon_thread();
        stop_core_sm_thread();
        return FAIL_TO_CREATE;
    }

    /*Main does not exit unless core thread is killed due to some reason*/
    join_core_thread();
    pthread_mutex_destroy(&pipe_mutex);

    return 0;
}
