/* sound_trigger_hw.h
 *
 * This library contains the API to load sound models with
 * DSP and start/stop detection of associated key phrases.
 *
 * Copyright (c) 2013-2014 QUALCOMM Technologies, Inc. All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 *
 */

#ifndef SOUND_TRIGGER_HW_H
#define SOUND_TRIGGER_HW_H

#include <cutils/list.h>
#include <pthread.h>
#include <hardware/sound_trigger.h>
#include <tinyalsa/asoundlib.h>
#include <audio_route/audio_route.h>

#include <sound/lsm_params.h>
#include "sound_trigger_prop_intf.h"

#define PCM_DEVICE_FILE "/proc/asound/pcm"

#define SOUND_TRIGGER_SAMPLING_RATE (16000)
#define SOUND_TRIGGER_CHANNEL_MODE  (1)

#define SOUND_TRIGGER_MAX_EVNT_PAYLOAD_SIZE (256)

#define SOUND_TRIGGER_PCM_MAX_RETRY (10)
#define SOUND_TRIGGER_PCM_SLEEP_WAIT (1000)

#define SOUND_TRIGGER_CPE_BUFFER_DURATION_MS (1000)
#define SOUND_TRIGGER_CPE_PERIOD_COUNT (8)
#define SOUND_TRIGGER_CPE_PERIOD_SIZE \
    ((((SOUND_TRIGGER_SAMPLING_RATE * SOUND_TRIGGER_CPE_BUFFER_DURATION_MS) \
       /(SOUND_TRIGGER_CPE_PERIOD_COUNT * 1000)) + 0x1f) & ~0x1f)

#define SOUND_TRIGGER_APE_BUFFER_DURATION_MS (1000)
#define SOUND_TRIGGER_APE_PERIOD_COUNT (8)
#define SOUND_TRIGGER_APE_PERIOD_SIZE \
    ((((SOUND_TRIGGER_SAMPLING_RATE * SOUND_TRIGGER_APE_BUFFER_DURATION_MS) \
       /(SOUND_TRIGGER_APE_PERIOD_COUNT * 1000)) + 0x1f) & ~0x1f)

#define SET_BIT(a, b) (a |= b)
#define CLEAR_BIT(a, b) (a &= ~(b))
#define CHECK_BIT(a, b) ((a) & (b))

#define SET_STATE(a, b) SET_BIT(a, b)
#define CLEAR_STATE(a, b) CLEAR_BIT(a, b)
#define CHECK_STATE(a, b) CHECK_BIT(a, b)

#define USECASE_STRING_SIZE (50)

typedef enum {
    ST_DEVICE_HW_NONE,
    ST_DEVICE_HW_APE,
    ST_DEVICE_HW_CPE
}st_hw_type_t;

typedef enum {
    ST_DEVICE_EXEC_MODE_NONE = -1,
    ST_DEVICE_EXEC_MODE_APE,
    ST_DEVICE_EXEC_MODE_CPE,
    ST_DEVICE_EXEC_MODE_MAX
}st_exec_mode_t;

enum sound_trigger_states {
    SES_STARTED = 0x01,
    SES_EVENT_RECEIVED = 0x02,
    SES_BUFFERING = 0x04
};

struct use_case_info {
    bool active;
    int pcm_id;
    char use_case[USECASE_STRING_SIZE];
};

struct sound_trigger_session {
    struct listnode list_node;
    struct sound_trigger_hw_device *device;
    struct sound_trigger_session_fptrs *fptrs;
    st_exec_mode_t exec_mode;

    sound_model_handle_t sm_handle;
    sound_trigger_sound_model_type_t sm_type;
    unsigned char *sm_data;
    unsigned int sm_size;

    recognition_callback_t callback;
    void *cookie;
    pthread_t callback_thread;
    bool exit_callback_thread;
    audio_io_handle_t capture_handle;
    bool capture_requested;
    unsigned int recognition_mode;
    struct sound_trigger_phrase_recognition_event *event;

    struct pcm *pcm;
    struct pcm_config  config;
    int pcm_id;

    enum sound_trigger_states state;
    unsigned int use_case_idx;
    unsigned int num_phrases;
    unsigned int num_users;
    unsigned int num_conf_levels;
    void *conf_levels;

    pthread_mutex_t lock;
    struct sound_trigger_event_info event_info;
};
typedef struct sound_trigger_session sound_trigger_session_t;

/* Function pointers to routing layers */
typedef int (*sound_trigger_reg_sm_t)(sound_trigger_session_t *);
typedef int (*sound_trigger_reg_sm_params_t)(sound_trigger_session_t *);
typedef int (*sound_trigger_dereg_sm_t)(sound_trigger_session_t *);
typedef int (*sound_trigger_dereg_sm_params_t)(sound_trigger_session_t *);
typedef int (*sound_trigger_start_t)(sound_trigger_session_t *);
typedef int (*sound_trigger_stop_t)(sound_trigger_session_t *);
typedef int (*sound_trigger_stop_buffering_t)(sound_trigger_session_t *);

struct sound_trigger_session_fptrs {
    sound_trigger_reg_sm_t reg_sm;
    sound_trigger_reg_sm_params_t reg_sm_params;
    sound_trigger_dereg_sm_t dereg_sm;
    sound_trigger_dereg_sm_params_t dereg_sm_params;
    sound_trigger_start_t start;
    sound_trigger_stop_t stop;
    sound_trigger_stop_buffering_t stop_buffering;
};

struct sound_trigger_device {
    struct sound_trigger_hw_device device;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    struct listnode sound_model_list;
    struct mixer *mixer;
    struct audio_route *audio_route;
    volatile int session_id;
    bool ssr_in_progress;
    audio_devices_t capture_device;
    int dev_ref_cnt;
    bool device_enabled;

    unsigned int tx_concurrency_active;
    unsigned int rx_concurrency_active;
    bool audio_concurrency_active;
    bool rx_concurrency_disabled;
    bool sw_mad;
    bool run_on_ape;
    bool transition_enabled;
    int num_sessions_configured;
    unsigned int max_cpe_sessions;
    unsigned int avail_cpe_phrases ;
    unsigned int avail_cpe_users ;
    unsigned int max_ape_sessions;
    unsigned int avail_ape_phrases;
    unsigned int avail_ape_users;
    unsigned int rx_conc_max_st_ses;
    struct use_case_info *ape_pcm_use_cases;
    struct use_case_info *cpe_pcm_use_cases;
    bool detect_failure;
    void *platform;
    st_hw_type_t hw_type;
    st_exec_mode_t exec_mode;

    void *audio_hal_handle;
    audio_hw_call_back_t audio_hal_cb;
};
typedef struct sound_trigger_device sound_trigger_device_t;

#endif /* SOUND_TRIGGER_HW_H */

