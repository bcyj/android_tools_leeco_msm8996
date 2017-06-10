/* sound_trigger_prop_intf.h
 *
 *  Interface between Audio Hal and Sound Trigger Hal for
 *  notifying events
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef SOUND_TRIGGER_PROP_INTF_H
#define SOUND_TRIGGER_PROP_INTF_H

enum sound_trigger_event_type {
    ST_EVENT_SESSION_REGISTER,
    ST_EVENT_SESSION_DEREGISTER
};
typedef enum sound_trigger_event_type sound_trigger_event_type_t;

enum audio_event_type {
    AUDIO_EVENT_CAPTURE_DEVICE_INACTIVE,
    AUDIO_EVENT_CAPTURE_DEVICE_ACTIVE,
    AUDIO_EVENT_PLAYBACK_STREAM_INACTIVE,
    AUDIO_EVENT_PLAYBACK_STREAM_ACTIVE,
    AUDIO_EVENT_STOP_LAB,
    AUDIO_EVENT_SSR,
    AUDIO_EVENT_NUM_ST_SESSIONS
};
typedef enum audio_event_type audio_event_type_t;

enum ssr_event_status {
    SND_CARD_STATUS_OFFLINE,
    SND_CARD_STATUS_ONLINE,
    CPE_STATUS_OFFLINE,
    CPE_STATUS_ONLINE
};

struct sound_trigger_session_info {
    int capture_handle;
    struct pcm *pcm;
    struct pcm_config config;
};
typedef struct sound_trigger_event_info sound_trigger_event_info_t;

struct sound_trigger_event_info {
    struct sound_trigger_session_info st_ses;
};
typedef struct sound_trigger_event_info sound_trigger_event_info_t;

typedef struct subsytem_restart_info subsytem_restart_info_t;

struct audio_event_info {
    union {
        enum ssr_event_status status;
        int value;
        struct sound_trigger_session_info ses_info;
    }u;
};
typedef struct audio_event_info audio_event_info_t;

/* STHAL callback which is called by AHAL */
typedef void (*sound_trigger_hw_call_back_t)(enum audio_event_type,
                                  struct audio_event_info*);

/* AHAL callback which is called by STHAL */
typedef void (*audio_hw_call_back_t)(enum sound_trigger_event_type,
                          struct sound_trigger_event_info*);

#endif /* SOUND_TRIGGER_PROP_INTF_H */
