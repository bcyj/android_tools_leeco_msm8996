#ifndef __RFSA_QMI_SERVER_H__
#define __RFSA_QMI_SERVER_H__

/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "rfsa_server.h"

/**
* Initialize server for QMI mode
*/
int32_t rfsa_qmi_server_init();
int32_t rfsa_qmi_server_deinit();
int32_t rfsa_qmi_server_response(rfsa_server_work_item_t *item);

#endif /* __RFSA_QMI_SERVER_H__ */
