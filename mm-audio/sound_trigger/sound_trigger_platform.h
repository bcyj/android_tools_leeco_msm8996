/* sound_trigger_platform.h
 *
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef SOUND_TRIGGER_PLATFORM_H
#define SOUND_TRIGGER_PLATFORM_H

#include "sound_trigger_hw.h"

#define PLATFORM_PATH_XML "/system/etc/sound_trigger_platform_info.xml"
#define MIXER_PATH_XML "system/etc/sound_trigger_mixer_paths.xml"
#define MIXER_PATH_XML_WCD9306 "system/etc/sound_trigger_mixer_paths_wcd9306.xml"
#define LIB_ACDB_LOADER "libacdbloader.so"
#define BUF_SIZE 1024

#define SOUND_CARD 0
#define MIXER_OPEN_MAX_NUM_RETRY 10
#define RETRY_US 500000

#ifdef PLATFORM_DEFAULT
  #define DEVICE_HANDSET_APE_ACDB_ID   (127)
  #define DEVICE_HANDSET_CPE_ACDB_ID   (128)
#else
  #define DEVICE_HANDSET_APE_ACDB_ID   (100)
  #define DEVICE_HANDSET_CPE_ACDB_ID   (128)
#endif

#define DEVICE_NAME_MAX_SIZE 128

enum {
    ST_DEVICE_NONE = 0,
    ST_DEVICE_MIN,
    ST_DEVICE_HANDSET_MIC = ST_DEVICE_MIN,
    ST_DEVICE_MAX,
};

typedef int st_device_t; //TODO

struct st_device_index {
    char name[100];
    unsigned int index;
};

void *platform_stdev_init(sound_trigger_device_t *stdev);

void platform_stdev_deinit(void *platform);

int platform_stdev_get_device_name
(
    void *platform,
    audio_devices_t device,
    char *device_name
);

int platform_stdev_send_calibration
(
    void *platform,
    audio_devices_t device,
    st_exec_mode_t exec_mode,
    int app_id
);

bool platform_stdev_check_and_update_concurrency
(
   void *platform,
   audio_event_type_t event_type,
   unsigned int num_sessions
);

bool platform_stdev_is_session_allowed
(
   void *platform,
   unsigned int num_sessions,
   bool sound_model_loaded
);

int platform_stdev_connect_mad
(
    void *platform,
    bool is_ape
);
#endif /* SOUND_TRIGGER_PLATFORM_H */
