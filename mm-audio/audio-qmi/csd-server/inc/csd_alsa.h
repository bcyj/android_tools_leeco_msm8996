/* Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef CSD_ALSA_H
#define CSD_ALSA_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "core_sound_driver_v01.h"
#include "msm8960_use_cases.h"
#include "alsa_ucm.h"
#include "qmi_csi.h"

#ifndef ANDROID
#define strlcat g_strlcat
#define strlcpy g_strlcpy
#define LOGI(...)      fprintf(stdout, __VA_ARGS__)
#define LOGE(...)      fprintf(stderr, __VA_ARGS__)
#define LOGV(...)      fprintf(stderr, __VA_ARGS__)
#define LOGD(...)      fprintf(stderr, __VA_ARGS__)
#endif

#define index_to_percent(per, min, max) \
        ((per - ((min) + .5)) / (((max) - (min)) * 0.01))

enum {
    STATE_WAIT,
    STATE_PROCESS_CMD,
    STATE_EXIT_THREAD
}pthread_state;

#define TIMEOUT_MS 3000

#define UI_PROP_SLOW_TALK 0x1
#define UI_PROP_FENS 0x2

#define NB_SAMPLE_RATE 8000
#define WB_SAMPLE_RATE 16000

#define MODULE_ID_VOICE_MODULE_FENS 0x00010EEB
#define MODULE_ID_VOICE_MODULE_ST 0x00010EE3

#define ALSA_PCM_IN_MONO  (PCM_MONO | PCM_IN)
#define ALSA_PCM_OUT_MONO (PCM_MONO | PCM_OUT)

#define VOICE_CHANNELS_MONO 1

#define MAX_CSD_QMI_CLIENTS 2
#define MAX_SESSIONS_PER_CLIENT 2

#define CALL_RESUME   0
#define CALL_PAUSE    1

#define ALL_SESSION_VSID    0xFFFFFFFF
#define DEFAULT_MUTE_RAMP_DURATION      500
#define DEFAULT_VOLUME_RAMP_DURATION_MS 20

struct modifier{
    void               *mod;
    struct modifier    *next;
};

struct alsa_usecase_list{
    char               verb[MAX_STR_LEN];
    uint32_t           mod_size;
    struct modifier    *mod_head;
};

enum {
    VOICE_SESSION_ID,
    VOLTE_SESSION_ID,
};

enum {
    OFF,
    HCO,
    VCO,
    FULL,
};

struct csd_dev_attribute
{
    uint32_t    sample_rate;
    uint32_t    bits_per_sample;
};

struct csd_dev_entry {
    uint32_t                    dev_id;
    struct csd_dev_attribute    dev_attrib;
};

struct devices_enable_info {
    struct csd_dev_entry    *devs;
};

#define MAX_NO_ACTIVE_DEVICES 2
struct device_data{
    struct csd_dev_entry    devices[MAX_NO_ACTIVE_DEVICES];
};

struct mute{
    uint16_t    direction;
    uint16_t    mute_flag;
};

struct voice_stream{
    struct mute    mute;
    uint8_t        ui_prop_mask;
    uint32_t       fens;
    uint32_t       slow_talk;
};

struct voice_stream_manager{
    uint32_t    wide_voice;
    uint32_t    tty_mode;
    uint32_t    hd_voice;
};

struct voice_context_type{
    uint16_t    direction;
    uint32_t    network_id;
    uint32_t    tx_dev_id;
    uint32_t    rx_dev_id;
    uint32_t    tx_dev_sr;
    uint32_t    rx_dev_sr;
    uint32_t    ec_ref_dev_id;
    bool        ec_ref_dev_id_valid;
    uint32_t    volume;
};

struct alsa_handle {
    snd_pcm_format_t    format;
    uint32_t            channels;
    unsigned int        latency;         // Delay in usec
    unsigned int        bufferSize;      // Size of sample buffer
    unsigned int        periodSize;
    struct pcm *        rxHandle;
    struct pcm*         txHandle;
    char                useCase[MAX_STR_LEN];
};

struct session_data_t{
    struct voice_stream            voice_passive_control_stream_data;
    struct voice_stream_manager    voice_stream_manager_data;
    struct voice_context_type      voice_context_data;
    struct alsa_handle             alsa_handle;
};

struct client_data{
    qmi_client_handle        qmi_client_handle_t;
    bool                     index_in_use;
    struct session_data_t    session[MAX_SESSIONS_PER_CLIENT];
};

struct command_data{
    uint32_t              token;
    int32_t               msg_id;
    qmi_req_handle        req_handle;
    uint32_t              command_handle;
    struct client_data    *command_client_data;
};

struct csd_common_data{
    snd_use_case_mgr_t     *ucm_mgr;
    struct mixer*          alsa_mixer;
    pthread_t              thread;
    int                    thread_state;
    struct client_data     client_data[MAX_CSD_QMI_CLIENTS];
    struct command_data    command;
    struct device_data     device_data;
};

int csd_alsa_preinit(void);
int csd_alsa_init(void);
int csd_alsa_deinit(void);
int get_session_id_from_vs_handle(uint32_t vs_handle,
                                  struct client_data *cmd_clnt_data,
                                  uint *session_id);
int get_session_id_from_vc_handle(uint32_t vc_handle,
                                  struct client_data *cmd_clnt_data,
                                  uint *session_id);
int get_session_id_from_vm_handle(uint32_t vm_handle,
                                  struct client_data *cmd_clnt_data,
                                  uint *session_id);

bool is_valid_voice_stream_handle(struct client_data *clnt_data,
                                  uint32_t vs_handle);
bool is_valid_voice_manager_handle(struct client_data *clnt_data,
                                   uint32_t vm_handle);
bool is_valid_voice_context_handle(struct client_data *clnt_data,
                                   uint32_t vc_handle);
bool is_valid_device_handle(struct client_data *clnt_data,
                            uint32_t device_handle);
int csd_alsa_enable_device(struct client_data *clnt_data, int acdbid1,
                           int acdbid2);
int csd_alsa_disable_device(struct client_data *clnt_data, int acdbid1,
                            int acdbid2);

#endif /*CSD_ALSA_H*/
