/******************************************************************************
  @file	 SubSystemShutdown.cpp
  @brief   Qualcomm Shutdown specific code.

  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

#define LOG_TAG "SubSystemShutdown"

#include "SubSystemShutdown.h"
#include <cutils/log.h>

extern "C" {
#include <subsystem_control.h>
#include <mdm_detect.h>
}

int shutdown()
{
    int rc;
    int i;
    int shutdown_incomplete = 0;
    struct dev_info devinfo;
    if (get_system_info(&devinfo) != RET_SUCCESS) {
            ALOGE("Failed to get system information");
            return RET_FAILED;
    }
    for (i = 0; i < devinfo.num_modems; i++)
    {
            if (devinfo.mdm_list[i].type == MDM_TYPE_EXTERNAL)
                    rc = subsystem_control_shutdown((unsigned)PROC_MDM,
                                    devinfo.mdm_list[i].mdm_name);
            else if (devinfo.mdm_list[i].type == MDM_TYPE_INTERNAL)
                    rc = subsystem_control_shutdown((unsigned)PROC_MSM,
                                    devinfo.mdm_list[i].mdm_name);
            if (rc) {
                     ALOGE("Failed to shut down %s",
                            devinfo.mdm_list[i].mdm_name);
                     shutdown_incomplete = 1;
            }
    }
    if (shutdown_incomplete) {
            ALOGE("Error during modem shutdown");
            return RET_FAILED;
    }
    ALOGI("Shutdown completed successfully!");
    return rc;
}

/*
 * Class:     com_qti_server_power_SubSystemShutdown
 * Method:    shutdown
 * Signature: ()
 */
JNIEXPORT jint JNICALL Java_com_qti_server_power_SubSystemShutdown_shutdown
(JNIEnv *, jclass)
{
    return shutdown();
}
