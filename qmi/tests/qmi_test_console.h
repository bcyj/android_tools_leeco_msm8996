/******************************************************************************
  @file    qmi_test_console.h
  @brief   The QMI Test console for simple RIL

  DESCRIPTION
  The QMI Test console for simple RIL

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_client_utils.h"

#ifndef QMI_TEST_CONSOLE_H
#define QMI_TEST_CONSOLE_H

typedef int (* qmi_test_console_downlink_msg_collector) (char* downlink_msg);
typedef void (* qmi_test_console_uplink_msg_distributor) (int nof_strings, char** uplinklink_msg);

typedef void (* qmi_test_console_shutdown_handler) (int cause);

void qmi_test_console_initialize(qmi_test_console_downlink_msg_collector downlink_collector, 
                                 qmi_test_console_uplink_msg_distributor uplink_distributor,
                                 qmi_test_console_shutdown_handler shutdown_handler);

void qmi_test_console_run(int is_modal_run);


#endif // QMI_TEST_CONSOLE_H
