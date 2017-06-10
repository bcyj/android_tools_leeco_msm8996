/* listen_platform.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef ANDROID_LISTEN_PLATFORM_H
#define ANDROID_LISTEN_PLATFORM_H

#include "listen_hw.h"

#ifdef PLATFORM_DEFAULT
  #define DEVICE_HANDSET_APE_ACDB_ID   (127)
  #define DEVICE_HANDSET_CPE_ACDB_ID   (128)
#else
  #define DEVICE_HANDSET_APE_ACDB_ID   (100)
  #define DEVICE_HANDSET_CPE_ACDB_ID   (128)
#endif

#define DEVICE_NAME_MAX_SIZE 128

enum {
    LISTEN_DEVICE_NONE = 0,
    LISTEN_DEVICE_MIN,
    LISTEN_DEVICE_HANDSET_MIC = LISTEN_DEVICE_MIN,
    LISTEN_DEVICE_MAX,
};

struct listen_device_index {
    char name[100];
    unsigned int index;
};

void *platform_listen_init(struct audio_listen_hardware *listen_hal);

void platform_listen_deinit(void *platform);

int platform_listen_get_device_name(
    void *platform,
    listen_device_t listen_device,
    char *device_name);

int platform_listen_send_calibration(
    void *platform,
    listen_device_t listen_device,
    listen_exec_mode_t exec_mode,
    int app_id);

void platform_listen_update_concurrency(
    void *platform,
    event_type_t event_type);

int platform_listen_connect_mad
(
    void *platform,
    bool en,
    audio_listen_session_t *p_ses
);
#endif /* ANDROID_LISTEN_PLATFORM_H */
