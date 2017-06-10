/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __CNSS_QMI_CLIENT_H__
#define __CNSS_QMI_CLIENT_H__

#define TIMEOUT_MS 3000

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof((_arr)[0]))
#endif

int wlan_service_start(void);
void wlan_service_stop(void);
int wlan_service_process_msg(int type, void *data, int len);
int wlan_service_init(void);
void wlan_service_release(void);

#endif /* __CNSS_QMI_CLIENT_H__ */
