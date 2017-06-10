/* listen_hw.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef ANDROID_LISTEN_HARDWARE_H
#define ANDROID_LISTEN_HARDWARE_H

#include <stdbool.h>
#include <pthread.h>
#include <cutils/list.h>
#include <tinyalsa/asoundlib.h>
#include <audio_route/audio_route.h>
#include <listen_types.h>
#include "sound/lsm_params.h"

/* for dumping lab data */
enum {
    DUMP_INIT,
    DUMP_DEINIT,
    DUMP_DATA
};

#define PCM_DEVICE_FILE "/proc/asound/pcm"

#define LISTEN_DEFAULT_SAMPLING_RATE (16000)
#define LISTEN_DEFAULT_CHANNEL_MODE  (1)
#define LISTEN_DEFAULT_BYTES_PER_SAMPLE  (2)

/* Configure CPE buffer size of 1sec. Align period size on 32bit boundary.
   buffer size in samples = period_size_in_samples * period_count
 */
#define LISTEN_CPE_PERIOD_COUNT (8)
#define LISTEN_CPE_PERIOD_SIZE  (2016) /* 125ms */

#define LISTEN_APE_PERIOD_COUNT (8)
#define LISTEN_APE_PERIOD_SIZE  (2016)

#define LISTEN_MAX_EVNT_PAYLOAD_SIZE (256)

#define RETRY_NUMBER (10)
#define RETRY_US (500000)

#define SET_BIT(a, b) (a |= b)
#define CLEAR_BIT(a, b) (a &= ~b)
#define CHECK_BIT(a, b) ((a) & (b))

#define SET_STATE(a, b) SET_BIT(a, b)
#define CLEAR_STATE(a, b) CLEAR_BIT(a, b)
#define CHECK_STATE(a, b) CHECK_BIT(a, b)

/* MAD device enabled/disabled State */
#define MAD_ENABLED   true
#define MAD_DISABLED  false

#define USECASE_STRING_SIZE (50)

typedef enum {
    LISTEN_HW_NONE = 0x0,
    LISTEN_HW_APE = 0x1,
    LISTEN_HW_CPE = 0x2
}listen_hw_type_t;

typedef enum {
    LISTEN_EXEC_MODE_INVALID = -1,
    LISTEN_EXEC_MODE_APE = 0,
    LISTEN_EXEC_MODE_CPE,
    LISTEN_EXEC_MODE_MAX
} listen_exec_mode_t;

typedef enum lsm_vw_status lsm_vw_status_t;

enum listen_ses_states {
    SES_OPENED = 0x01,
    SES_SM_REGD = 0x02,
    SES_STARTED = 0x04,
    SES_SSR_IN_PROGRESS = 0x08,
    SES_SSR_COMPLETED = 0x10,
    SES_IN_BAD_STATE = 0x20,
    SES_BUFFERING = 0x40,
    SES_CONCURRENCY_ACTIVE = 0x80,
};

struct use_case_info {
    bool active;
    int pcm_id;
    char use_case[USECASE_STRING_SIZE];
};

struct audio_listen_session {
    listen_session_t handle;
    struct listnode list_node; /* used to manage this session node in session list */
    struct listen_session_fptrs *lfptrs;
    struct pcm *pcm;
    struct pcm_config  config;
    int pcm_id;
    pthread_mutex_t lock;
    pthread_cond_t  ssr_cond;
    pthread_t event_thread;
    listen_callback_t ses_cb_func;
    listen_sound_model_params_t reg_params;
    bool kill_event_thread;
    bool event_thread_alive;
    void *client_data;
    listen_exec_mode_t exec_mode;
    unsigned int use_case_idx;
    unsigned int acdb_id;
    listen_app_type_enum_t app_type;
    unsigned int op_state;
    unsigned int num_keywords;
    unsigned int num_user_pairings;
    FILE *dump_fptr;
};
typedef struct audio_listen_session audio_listen_session_t;

/* mixer controls, push session calibration, pcm_open, create event thread */
typedef int (*listen_open_t)(audio_listen_session_t *p_handle);
/* destroy event thread, mixer controls, pcm_close, */
typedef int (*listen_close_t)(audio_listen_session_t *p_handle);
/* SNDRV_LSM_REG_SND_MODEL, store soundmodel for SSR */
typedef int (*listen_reg_sm_t)(audio_listen_session_t *p_handle,
                               listen_sound_model_params_t *params);
/* SNDRV_LSM_DEREG_SND_MODEL,  */
typedef int (*listen_dereg_sm_t)(audio_listen_session_t *p_handle);
/* SNDRV_LSM_START */
typedef int (*listen_start_t)(audio_listen_session_t *p_handle);
/* SNDRV_LSM_STOP */
typedef int (*listen_stop_t)(audio_listen_session_t *p_handle);
/* pcm_read */
typedef int (*listen_read_buffer_t)(audio_listen_session_t *p_handle,
                                    uint8_t *p_buf,
                                    uint32_t *p_bytes);
/* SNDRV_LSM_STOP_LAB */
typedef int (*listen_stop_buffering_t)(audio_listen_session_t *p_handle);

struct listen_session_fptrs {
    listen_open_t open;
    listen_close_t close;
    listen_reg_sm_t reg_sm;
    listen_dereg_sm_t dereg_sm;
    listen_start_t start;
    listen_stop_t stop;
    listen_read_buffer_t read_buffer;
    listen_stop_buffering_t stop_buffering;
};

typedef int listen_device_t;

struct audio_listen_hardware
{
    unsigned snd_card;
    struct mixer *mixer;
    struct audio_route *audio_route;
    pthread_mutex_t lock;
    pthread_mutex_t list_lock;
    listen_callback_t mad_cb_func;
    void *acdb_handle;
    struct listnode ses_list;
    int tx_concurrency_active;
    int rx_concurrency_active;
    bool rx_concurrency_disabled;
    bool sw_mad;
    bool reqd_mad_state;
    bool actual_mad_state;
    listen_hw_type_t hw_type;
    listen_device_t cur_device;
    bool run_on_ape;
    int max_cpe_sessions;
    int avail_cpe_kw;
    int avail_cpe_kw_user_pairs;
    int max_ape_sessions;
    int avail_ape_kw;
    int avail_ape_kw_user_pairs;
    struct use_case_info *ape_pcm_use_cases;
    struct use_case_info *cpe_pcm_use_cases;
    void* platform;
    bool debug_dump_lab_data;
    unsigned int rx_conc_max_st_ses;
};
typedef struct audio_listen_hardware audio_listen_hardware_t;
#endif    /* ANDROID_LISTEN_HARDWARE_H */
