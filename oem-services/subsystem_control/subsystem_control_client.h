/******************************************************************************

  @file	 subsystem_control_client.h

  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

#ifndef __SUBSYSTEM_CONTROL_CLIENT_H__
#define __SUBSYSTEM_CONTROL_CLIENT_H__

#include <subsystem_control.h>
#include <pm-service.h>
#define MSG_LENGTH 90
#define TIMEOUT_MS 3000

enum command {
    CMD_START,
    CMD_SHUTDOWN = CMD_START,
    CMD_END = CMD_SHUTDOWN,
};

unsigned int qmi_command [] = {
    [CMD_SHUTDOWN] = QMI_SSCTL_SHUTDOWN_REQ_V01,
};

qmi_service_instance qmi_instance_id[] = {
    [PROC_MSM] = 0,
};

struct ssc_req {
    enum procs proc_num;
    enum command cmd;
};

struct ssc_resp {
    int rc;
    int length;
    char msg[MSG_LENGTH];
};

#endif
