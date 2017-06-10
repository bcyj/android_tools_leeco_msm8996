/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _WIFI_FTM_H__
#define _WIFI_FTM_H__
int wifi_ftmd_start();
int wifi_ftm_send_cmd(const char* cmd);
int wifi_ftm_send_cmd(const char* cmd, char* reply, size_t *reply_len);

#endif
