/*
 *
 * This library contains the API to load sound models with
 * DSP and start/stop detection of associated key phrases.
 *
 * Copyright (c) 2013-2014 QUALCOMM Technologies, Inc. All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 *
 */

#define LOG_TAG "sound_trigger_hw"
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0

#include <dlfcn.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <system/thread_defs.h>
#include <hardware/sound_trigger.h>

#include "sound_trigger_platform.h"
#include "sound_trigger_hw.h"

#define XSTR(x) STR(x)
#define STR(x) #x

/* Routing layer functions to route to either ADSP or CPE */
static int route_reg_sm_ape(sound_trigger_session_t *p_ses);
static int route_reg_sm_params_ape(sound_trigger_session_t* p_ses);
static int route_dereg_sm_ape(sound_trigger_session_t* p_ses);
static int route_dereg_sm_params_ape(sound_trigger_session_t* p_ses);
static int route_start_ape(sound_trigger_session_t* p_ses);
static int route_stop_ape(sound_trigger_session_t* p_ses);
static int route_stop_buffering_ape(sound_trigger_session_t* p_ses);

static int route_reg_sm_cpe(sound_trigger_session_t *p_ses);
static int route_reg_sm_params_cpe(sound_trigger_session_t* p_ses);
static int route_dereg_sm_cpe(sound_trigger_session_t* p_ses);
static int route_dereg_sm_params_cpe(sound_trigger_session_t* p_ses);
static int route_start_cpe(sound_trigger_session_t* p_ses);
static int route_stop_cpe(sound_trigger_session_t* p_ses);
static int route_stop_buffering_cpe(sound_trigger_session_t* p_ses);

/* count of sound trigger hal clients */
static unsigned int stdev_ref_cnt = 0;
static pthread_mutex_t stdev_init_lock;
static struct sound_trigger_device *stdev = NULL;

/* default properties which will later be updated based on platform configuration */
static struct sound_trigger_properties hw_properties = {
        "QUALCOMM Technologies, Inc", // implementor
        "Sound Trigger HAL", // description
        1, // version
        { 0x68ab2d40, 0xe860, 0x11e3, 0x95ef, { 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b } }, // uuid
        1, // max_sound_models
        1, // max_key_phrases
        1, // max_users
        RECOGNITION_MODE_VOICE_TRIGGER | RECOGNITION_MODE_USER_IDENTIFICATION, // recognition_modes
        true, // capture_transition
        0, // max_capture_ms
        false, // concurrent_capture
        false, //trigger_in_event
        0 // power_consumption_mw
};

static struct pcm_config stdev_ape_pcm_config = {
    .channels = SOUND_TRIGGER_CHANNEL_MODE,
    .rate = SOUND_TRIGGER_SAMPLING_RATE,
    .period_size = SOUND_TRIGGER_APE_PERIOD_SIZE,
    .period_count = SOUND_TRIGGER_APE_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

static struct pcm_config stdev_cpe_pcm_config = {
    .channels = SOUND_TRIGGER_CHANNEL_MODE,
    .rate = SOUND_TRIGGER_SAMPLING_RATE,
    .period_size = SOUND_TRIGGER_CPE_PERIOD_SIZE,
    .period_count = SOUND_TRIGGER_CPE_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct sound_trigger_session_fptrs ape_fptrs = {
    .reg_sm = route_reg_sm_ape ,
    .reg_sm_params = route_reg_sm_params_ape,
    .dereg_sm = route_dereg_sm_ape ,
    .dereg_sm_params = route_dereg_sm_params_ape,
    .start = route_start_ape,
    .stop = route_stop_ape,
    .stop_buffering = route_stop_buffering_ape,
};

struct sound_trigger_session_fptrs cpe_fptrs = {
    .reg_sm = route_reg_sm_cpe ,
    .reg_sm_params = route_reg_sm_params_cpe,
    .dereg_sm = route_dereg_sm_cpe  ,
    .dereg_sm_params = route_dereg_sm_params_cpe,
    .start = route_start_cpe,
    .stop = route_stop_cpe,
    .stop_buffering = route_stop_buffering_cpe,
};

static int stdev_get_hw_type(void)
{
    int dev_id;
    unsigned int i = 0, j = 0;
    char buf[100], comp_string[USECASE_STRING_SIZE];
    FILE *fp;

    fp = fopen(PCM_DEVICE_FILE, "r");

    if (!fp) {
        ALOGE("%s: ERROR. %s file open failed", __func__, PCM_DEVICE_FILE);
        return -ENODEV;
    }
    snprintf(comp_string, USECASE_STRING_SIZE, "Listen %d Audio Service", i+1);

    while(1) {
        fgets(buf, 100, fp);
        if (feof(fp))
            break;
        sscanf(&buf[3],"%d", &dev_id);
        if((strstr(buf, comp_string)) &&
           (i < stdev->max_ape_sessions)) {

            stdev->hw_type |= ST_DEVICE_HW_APE;
            snprintf(stdev->ape_pcm_use_cases[i].use_case,
                     USECASE_STRING_SIZE, "listen-voice-wakeup-%d", i+1);
            stdev->ape_pcm_use_cases[i++].pcm_id = dev_id;
            snprintf(comp_string, USECASE_STRING_SIZE, "Listen %d Audio Service", i+1);
        }
        else if(strstr(buf, "CPE Listen") &&
                (j < stdev->max_cpe_sessions)) {

            stdev->hw_type |= ST_DEVICE_HW_CPE;
            stdev->cpe_pcm_use_cases[j++].pcm_id = dev_id;
        }
    }
    fclose(fp);

    if (!CHECK_BIT(stdev->hw_type, ST_DEVICE_HW_APE|ST_DEVICE_HW_CPE)) {
        ALOGE("%s: ERROR. No Listen device present", __func__);
        return -ENODEV;
    }

    if (stdev->max_ape_sessions > i)
        stdev->max_ape_sessions = i;

    if (stdev->max_cpe_sessions > j)
        stdev->max_cpe_sessions = j;

    return 0;
}

static void sound_trigger_set_device(bool enable, audio_devices_t device)
{
    char device_name[DEVICE_NAME_MAX_SIZE] = {0};

    if(platform_stdev_get_device_name(stdev->platform, device, device_name) < 0 ) {
        ALOGE("%s: Invalid sound trigger device returned", __func__);
        return;
    }

    if (enable) {
        ALOGD("%s: enable device (%x) = %s", __func__, device, device_name);
        audio_route_apply_and_update_path(stdev->audio_route, device_name);
        platform_stdev_connect_mad(stdev->platform, stdev->run_on_ape);
    } else {
        ALOGD("%s: disable device (%x) = %s", __func__, device, device_name);
        audio_route_reset_and_update_path(stdev->audio_route, device_name);
    }
}

unsigned int get_num_sessions(void)
{
    struct listnode *p_ses_node;
    unsigned int num_sessions = 0;

    if ((list_head(&stdev->sound_model_list) == NULL) ||
                  (list_tail(&stdev->sound_model_list) == NULL)) {
        ALOGE("%s: sound model list is yet to be initialized", __func__);
        return num_sessions;
    }

    list_for_each(p_ses_node, &stdev->sound_model_list)
        num_sessions++;

    return num_sessions;
}

static void handle_ssr_online(void)
{
    struct listnode *p_ses_node;
    sound_trigger_session_t *p_ses;
    int status = 0;

    ALOGD("%s: Enter", __func__);

    pthread_mutex_lock(&stdev->lock);
    /* reload and reactive each previously active session */
    list_for_each(p_ses_node, &stdev->sound_model_list) {
        p_ses = node_to_item(p_ses_node, sound_trigger_session_t, list_node);

        pthread_mutex_lock(&p_ses->lock);
        status = p_ses->fptrs->reg_sm(p_ses);

        if (!status && !stdev->audio_concurrency_active &&
            CHECK_STATE(p_ses->state, SES_STARTED)) {
            if(!stdev->device_enabled) {
                /* Reset the DAPM cached mixer control for MADONOFF switch as
                   kernel is not reseting this mixer control when codec resets */
                sound_trigger_set_device(false, stdev->capture_device);
                sound_trigger_set_device(true, stdev->capture_device);
                stdev->device_enabled = true;
            }
            p_ses->fptrs->reg_sm_params(p_ses);
            p_ses->fptrs->start(p_ses);
            //LISTEN_EVENT_STARTED
        }
        pthread_mutex_unlock(&p_ses->lock);
    }
    stdev->ssr_in_progress = false;
    pthread_cond_signal(&stdev->cond);

    pthread_mutex_unlock(&stdev->lock);
    ALOGD("%s: Exit", __func__);
}

static void handle_ssr_offline(void)
{
    struct listnode *p_ses_node;
    sound_trigger_session_t *p_ses;

    ALOGD("%s: Enter", __func__);

    pthread_mutex_lock(&stdev->lock);
    stdev->ssr_in_progress = true;
    stdev->device_enabled = false;
    //LISTEN_EVENT_STOPPED
    /* teardown each session */
    list_for_each(p_ses_node, &stdev->sound_model_list) {
        p_ses = node_to_item(p_ses_node, sound_trigger_session_t, list_node);

        pthread_mutex_lock(&p_ses->lock);
        if (!stdev->audio_concurrency_active &&
            CHECK_STATE(p_ses->state, SES_STARTED)) {
            p_ses->fptrs->stop_buffering(p_ses);
            p_ses->fptrs->stop(p_ses);
        }
        p_ses->fptrs->dereg_sm(p_ses);
        pthread_mutex_unlock(&p_ses->lock);
    }
    pthread_mutex_unlock(&stdev->lock);
    ALOGD("%s: Exit", __func__);
}

static void handle_audio_concurrency(audio_event_type_t event_type)
{
    struct listnode *p_ses_node;
    sound_trigger_session_t *p_ses;
    bool session_allowed;
    unsigned int num_sessions;
    int status = 0;
    ALOGV("%s: Enter, event type = %d", __func__, event_type);

    /*
    UC1:
        1. start_recognition
        2. audio record_active
        3. audio_record_inactive
        4. stop_recognition
    UC1:
        1. start_recognition
        2. audio record_active
        3. stop_recognition
        4. audio_record_inactive
    UC2:
        1. audio_record_active
        2. start_recognition
        3. stop_recogntion
        4. audio_record_inactive
    UC3:
        1. audio_record_active
        2. start_recognition
        3. audio_record_inactive
        4. stop_recogntion
    */
    pthread_mutex_lock(&stdev->lock);
    num_sessions = get_num_sessions();
    session_allowed = platform_stdev_check_and_update_concurrency(stdev->platform,
                                                event_type, num_sessions);

    if(!num_sessions) {
       pthread_mutex_unlock(&stdev->lock);
       return;
    }

    if(session_allowed && stdev->audio_concurrency_active) {
        stdev->audio_concurrency_active = false;
        list_for_each(p_ses_node, &stdev->sound_model_list) {
            p_ses = node_to_item(p_ses_node, sound_trigger_session_t, list_node);

            pthread_mutex_lock(&p_ses->lock);
            if (stdev->sw_mad)
                status = p_ses->fptrs->reg_sm(p_ses);

            if (!status && CHECK_STATE(p_ses->state, SES_STARTED)) {
                if (!stdev->device_enabled) {
                    sound_trigger_set_device(true, stdev->capture_device);
                    stdev->device_enabled = true;
                }
                if (stdev->sw_mad)
                    status = p_ses->fptrs->reg_sm_params(p_ses);

                if (!status)
                    p_ses->fptrs->start(p_ses);
                    //LISTEN_EVENT_STARTED
            }
            pthread_mutex_unlock(&p_ses->lock);
        }
    }
    else if (!session_allowed && !stdev->audio_concurrency_active) {
        stdev->audio_concurrency_active = true;
        list_for_each(p_ses_node, &stdev->sound_model_list) {
            p_ses = node_to_item(p_ses_node, sound_trigger_session_t, list_node);

            pthread_mutex_lock(&p_ses->lock);
            if(CHECK_STATE(p_ses->state, SES_STARTED)) {
                //LISTEN_EVENT_STOPPED
                p_ses->fptrs->stop_buffering(p_ses);
                p_ses->fptrs->stop(p_ses);
                if (stdev->device_enabled) {
                    sound_trigger_set_device(false, stdev->capture_device);
                    stdev->device_enabled = false;
                }
            }
            if (stdev->sw_mad)
                p_ses->fptrs->dereg_sm(p_ses);
            pthread_mutex_unlock(&p_ses->lock);
        }
    }

    pthread_mutex_unlock(&stdev->lock);
    ALOGV("%s: Exit", __func__);
}

static int get_confidence_levels_payload
(
   sound_trigger_session_t* p_ses,
   const struct sound_trigger_recognition_config *config
)
{
    int status = 0;
    unsigned int total_num_users = 0, num_conf_levels = 0;
    unsigned int user_level, user_id;
    unsigned int i, j;
    unsigned char *conf_levels = NULL;
    unsigned char *user_id_tracker;

    if((config->num_phrases == 0) ||
       (config->num_phrases > p_ses->num_phrases)) {
        ALOGE("%s: ERROR. Invalid phrases %d!=%d",__func__,
              config->num_phrases, p_ses->num_phrases);
        status = -EINVAL;
        goto exit;
    }
    for (i = 0; i < config->num_phrases; i++) {
        for (j = 0; j < config->phrases[i].num_levels; j++)
            total_num_users++;
    }
    if(total_num_users > p_ses->num_users) {
        ALOGE("%s: ERROR. Invalid users %d>%d",__func__,
              total_num_users, p_ses->num_users);
        status = -EINVAL;
        goto exit;
    }

    num_conf_levels = total_num_users + config->num_phrases;
    conf_levels = calloc(1, 2 * num_conf_levels);
    if (!conf_levels) {
        ALOGE("%s: ERROR. conf levels alloc failed",__func__);
        status = -ENOMEM;
        goto exit;
    }

    p_ses->event = calloc(1, sizeof(struct sound_trigger_phrase_recognition_event));
    if (!p_ses->event) {
        ALOGE("%s: ERROR. event alloc failed",__func__);
        status = -ENOMEM;
        goto exit;
    }
    /* Example: Say the recognition structure has 3 keywords with users
       |kid|
        [0] k1 |uid|
                [0] u1 - 1st trainer
                [1] u2 - 4th trainer
                [3] u3 - 3rd trainer
        [1] k2
                [2] u2 - 2nd trainer
                [4] u3 - 5th trainer
        [2] k3
                [5] u4 - 6th trainer

      Output confidence level array will be
      [k1, k2, k3, u1k1, u2k1, u2k2, u3k1, u3k2, u4k3]
    */

    user_id_tracker =  &conf_levels[num_conf_levels];

    for (i = 0; i < config->num_phrases; i++) {
        ALOGV("%s: [%d] kw level %d", __func__, i, config->phrases[i].confidence_level);
        for (j = 0; j < config->phrases[i].num_levels; j++) {
            ALOGV("%s: [%d] user_id %d level %d ", __func__, i,
                  config->phrases[i].levels[j].user_id,
                  config->phrases[i].levels[j].level);
        }
    }

    for (i = 0; i < config->num_phrases; i++) {
        conf_levels[i] = config->phrases[i].confidence_level;
        for (j = 0; j < config->phrases[i].num_levels; j++) {
            user_level = config->phrases[i].levels[j].level;
            user_id = config->phrases[i].levels[j].user_id;
            if ((user_id < config->num_phrases) || (user_id > num_conf_levels)) {
                ALOGE("%s: ERROR. Invalid params user id %d>%d",
                      __func__, user_id, total_num_users);
                status = -EINVAL;
                goto exit;
            }
            else {
                if (user_id_tracker[user_id] == 1) {
                    ALOGE("%s: ERROR. Duplicate user id %d",
                          __func__, user_id);
                    status = -EINVAL;
                    goto exit;
                }
                conf_levels[user_id] = (user_level < 100) ? user_level: 100;
                user_id_tracker[user_id] = 1;
                ALOGV("%s: user_conf_levels[%d] = %d", __func__, user_id, conf_levels[user_id]);
            }
        }
    }
    p_ses->conf_levels = conf_levels;
    p_ses->num_conf_levels = num_conf_levels;

    /* store the ids to fill the event from detection event payload from DSP */
    p_ses->event->num_phrases = config->num_phrases;
    p_ses->event->common.data_offset = sizeof(*p_ses->event);
    memcpy(p_ses->event->phrase_extras, config->phrases, sizeof(config->phrases));

    return status;

exit:
    if (conf_levels)
        free(conf_levels);
    if (p_ses->event)
        free(p_ses->event);

    return status;
}

static int fill_recognition_event
(
   sound_trigger_session_t *p_ses,
   int event_status,
   void *payload,
   unsigned int payload_size
)
{
    unsigned int i, j, user_id;
    int *ses_id;
    struct sound_trigger_phrase_recognition_event *event = p_ses->event;

    if (payload_size != p_ses->num_conf_levels) {
        ALOGE("%s: payload size %d!=%d", __func__,
              payload_size, p_ses->num_conf_levels);
        return -EINVAL;
    }

    event->common.status = event_status;
    event->common.type = p_ses->sm_type;
    event->common.model = p_ses->sm_handle;
    event->common.capture_available = p_ses->capture_requested;
    event->common.capture_delay_ms = 0;
    event->common.audio_config.sample_rate = SOUND_TRIGGER_SAMPLING_RATE;
    event->common.audio_config.channel_mask = AUDIO_CHANNEL_IN_MONO;
    event->common.audio_config.format = AUDIO_FORMAT_PCM_16_BIT;

    /* fill confidence levels */
    for (i = 0; i < event->num_phrases; i++) {
        event->phrase_extras[i].recognition_modes = p_ses->recognition_mode;
        event->phrase_extras[i].confidence_level = ((char *)payload)[i];
        for (j = 0; j < event->phrase_extras[i].num_levels; j++) {
            user_id = event->phrase_extras[i].levels[j].user_id;
            event->phrase_extras[i].levels[j].level = ((char *)payload)[user_id];
        }
    }

    for (i = 0; i < p_ses->num_conf_levels; i++)
        ALOGV("%s: payload[%d] = %d", __func__, i, ((char *)payload)[i]);

    for (i = 0; i < event->num_phrases; i++) {
        ALOGV("%s: [%d] kw level %d", __func__,i,
              event->phrase_extras[i].confidence_level);
        for (j = 0; j < event->phrase_extras[i].num_levels; j++) {
            ALOGV("%s: [%d] user_id %d level %d ", __func__, i,
                  event->phrase_extras[i].levels[j].user_id,
                  event->phrase_extras[i].levels[j].level);
        }
    }

    return 0;
}

static int ape_get_pcm_device_id(sound_trigger_session_t *p_ses)
{
    unsigned int i;
    int ret = -1;

    for (i = 0; i < stdev->max_ape_sessions; i++) {
        if(!stdev->ape_pcm_use_cases[i].active) {
            stdev->ape_pcm_use_cases[i].active = true;
            ret = stdev->ape_pcm_use_cases[i].pcm_id;
            p_ses->use_case_idx = i;
            break;
        }
    }
    if (ret < 0)
        ALOGE("%s: ERROR. no free pcm device available", __func__);

    return ret;
}

static void ape_free_pcm_device_id(sound_trigger_session_t *p_ses)
{
    unsigned int i;

    for (i = 0; i < stdev->max_ape_sessions; i++) {
        if(stdev->ape_pcm_use_cases[i].pcm_id == p_ses->pcm_id) {
            stdev->ape_pcm_use_cases[i].active = false;
            break;
        }
    }
}

static void ape_enable_use_case(bool enable, sound_trigger_session_t *p_ses)
{
    if (enable) {
        ALOGD("%s: enable use case = %s", __func__,
            stdev->ape_pcm_use_cases[p_ses->use_case_idx].use_case);

        audio_route_apply_and_update_path(stdev->audio_route,
            stdev->ape_pcm_use_cases[p_ses->use_case_idx].use_case);

    } else {
        ALOGD("%s: disable use case = %s", __func__,
              stdev->ape_pcm_use_cases[p_ses->use_case_idx].use_case);

        audio_route_reset_and_update_path(stdev->audio_route,
            stdev->ape_pcm_use_cases[p_ses->use_case_idx].use_case);
    }
}

static int cpe_get_pcm_device_id(sound_trigger_session_t *p_ses)
{
    unsigned int i;
    int ret = -1;

    for (i = 0; i < stdev->max_cpe_sessions; i++) {
        if(!stdev->cpe_pcm_use_cases[i].active) {
            stdev->cpe_pcm_use_cases[i].active = true;
            ret = stdev->cpe_pcm_use_cases[i].pcm_id;
            p_ses->use_case_idx = i;
            break;
        }
    }
    if (ret < 0)
        ALOGE("%s: ERROR. no free pcm device available", __func__);

    return ret;
}

static void cpe_free_pcm_device_id(sound_trigger_session_t *p_ses)
{
    unsigned int i;

    for (i = 0; i < stdev->max_cpe_sessions; i++) {
        if(stdev->cpe_pcm_use_cases[i].pcm_id == p_ses->pcm_id) {
            stdev->cpe_pcm_use_cases[i].active = false;
            break;
        }
    }
}

static void *callback_thread_loop(void *context)
{
    struct sound_trigger_session *st_session =
                                 (struct sound_trigger_session *)context;
    struct sound_trigger_device *stdev = NULL;
    struct snd_lsm_event_status *params;
    struct sound_trigger_phrase_recognition_event *data;
    unsigned int payload_alloc_size = SOUND_TRIGGER_MAX_EVNT_PAYLOAD_SIZE;
    int status = 0;
    int event_status;

    ALOGD("%s: Enter", __func__);

    if (st_session == NULL) {
        ALOGE("%s: ERROR. null context.. exiting", __func__);
        return NULL;
    }

    stdev = (struct sound_trigger_device *)st_session->device;
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_DEFAULT);
    prctl(PR_SET_NAME, (unsigned long)"sound trigger callback", 0, 0, 0);

    pthread_mutex_lock(&st_session->lock);
    params = calloc(1, sizeof(*params) + payload_alloc_size);
    if (params == NULL) {
        ALOGE("%s: ERROR. insufficient memory for payload", __func__);
        goto exit;
    }

    while (!st_session->exit_callback_thread) {
        params->payload_size = payload_alloc_size;
        ALOGI("%s: Waiting for SNDRV_LSM_EVENT_STATUS", __func__);
        pthread_mutex_unlock(&st_session->lock);
        status = pcm_ioctl(st_session->pcm, SNDRV_LSM_EVENT_STATUS, params);
        pthread_mutex_lock(&st_session->lock);
        ALOGI("%s: Received SNDRV_LSM_EVENT_STATUS status=%d", __func__, status);
        if (status < 0) {
            if (errno == ENOMEM) {
                payload_alloc_size = payload_alloc_size << 1;
                params = realloc(params, sizeof(*params) + payload_alloc_size);

                if (params == NULL) {
                    ALOGE("%s: ERROR. Not enough memory for payload",
                          __func__);
                    st_session->exit_callback_thread = true;
                    break;
                }
                continue;
            } else {
                ALOGE("%s: ERROR. SNDRV_LSM_EVENT_STATUS  failed status %d",__func__, status);
                st_session->exit_callback_thread = true;
                break;
            }
        }
        if (st_session->exit_callback_thread)
            break;
        if (!st_session->callback)
            continue;

        ALOGD("%s: params status %d", __func__, params->status);
        switch (params->status) {
        case LSM_VOICE_WAKEUP_STATUS_RUNNING:
            continue;
        case LSM_VOICE_WAKEUP_STATUS_DETECTED:
            event_status = RECOGNITION_STATUS_SUCCESS;
            if (st_session->capture_requested)
                SET_STATE(st_session->state, SES_BUFFERING);
            break;
        case LSM_VOICE_WAKEUP_STATUS_END_SPEECH:
        case LSM_VOICE_WAKEUP_STATUS_REJECTED:
            event_status = RECOGNITION_STATUS_FAILURE;
            break;
        default:
            ALOGW("%s: Unknown status %d", __func__, params->status);
            continue;
        }

        status = fill_recognition_event(st_session, event_status,
                                        params->payload, params->payload_size);
        if (status)
            continue;

        SET_STATE(st_session->state, SES_EVENT_RECEIVED);

        ALOGI("%s:[%d] send recognition event %d", __func__,
                         st_session->sm_handle, event_status);
        st_session->callback(&st_session->event->common, st_session->cookie);
    }

    if (params)
        free(params);

exit:
    pthread_mutex_unlock(&st_session->lock);
    ALOGD("%s: Exit", __func__);
    return NULL;
}

static int ape_reg_sm(sound_trigger_session_t *p_ses)
{
    int status = 0;
    struct snd_lsm_session_data ses_data;
    struct snd_lsm_sound_model_v2 lsm_params_v2 = {0,};
    pthread_attr_t attr;

    ALOGD("%s:[%d] Enter", __func__, p_ses->sm_handle);

    p_ses->pcm_id = ape_get_pcm_device_id(p_ses);
    if(p_ses->pcm_id < 0)
        return -ENODEV;

    status = platform_stdev_send_calibration(stdev->platform,
                                              stdev->capture_device,
                                              p_ses->exec_mode,
                                              LSM_VOICE_WAKEUP_APP_ID);
    if (status) {
        ALOGE("%s: ERROR. sending calibration failed", __func__);
        goto sm_error;
    }

    p_ses->config = stdev_ape_pcm_config;
    ALOGD("%s: opening pcm device=%d", __func__, p_ses->pcm_id);
    ALOGV("%s: config: channels=%d rate=%d, period_size=%d, period_cnt=%d, format=%d",
          __func__, p_ses->config.channels, p_ses->config.rate,
          p_ses->config.period_size, p_ses->config.period_count,
          p_ses->config.format);
    p_ses->pcm = pcm_open(SOUND_CARD, p_ses->pcm_id,
                          PCM_IN, &p_ses->config);
    if (!p_ses->pcm) {
        ALOGE("%s: ERROR. pcm_open failed", __func__);
        status = -ENODEV;
        goto sm_error;
    }
    if (!pcm_is_ready(p_ses->pcm)) {
        ALOGE("%s: ERROR. pcm_is_ready failed err=%s", __func__,
              pcm_get_error(p_ses->pcm));
        status = -ENODEV;
        goto sm_error;
    }

    ses_data.app_id = LSM_VOICE_WAKEUP_APP_ID_V2;
    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_SET_SESSION_DATA, &ses_data);
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_SET_SESSION_DATA failed status %d",
              __func__, status);
        goto sm_error;
    }

    lsm_params_v2.data = p_ses->sm_data;
    lsm_params_v2.data_size = p_ses->sm_size;
    ALOGD("%s: data=%p, data_size=%d, conf_level=%p, num_conf_level=%d",
           __func__, lsm_params_v2.data, lsm_params_v2.data_size,
          lsm_params_v2.confidence_level, lsm_params_v2.num_confidence_levels);
    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_REG_SND_MODEL_V2, &lsm_params_v2);
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_REG_SND_MODEL_V2 failed, status=%d", __func__, status);
        goto sm_error;
    }

    /* Callback thread to wait on event detection */
    p_ses->exit_callback_thread = false;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&p_ses->callback_thread, &attr,
                        callback_thread_loop, p_ses);

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return 0;

sm_error:
    ape_free_pcm_device_id(p_ses);
    if (p_ses->pcm) {
        pcm_close(p_ses->pcm);
        p_ses->pcm = NULL;
    }

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int ape_dereg_sm(sound_trigger_session_t *p_ses)
{
    int status = 0;

    ALOGD("%s:[%d] Enter", __func__, p_ses->sm_handle);

    if (!p_ses->pcm) {
        ALOGV("%s: pcm NULL", __func__);
        return status;
    }

    /* Exit the callback thread waiting on event detection */
    p_ses->exit_callback_thread = true;
    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_ABORT_EVENT);
    if (status)
        ALOGE("%s: SNDRV_LSM_ABORT_EVENT failed, status=%d", __func__, status);

    pthread_mutex_unlock(&p_ses->lock);
    pthread_join(p_ses->callback_thread, (void **) NULL);
    pthread_mutex_lock(&p_ses->lock);


    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_DEREG_SND_MODEL);

    if (status)
        ALOGE("%s: ERROR. SNDRV_LSM_DEREG_SND_MODEL failed", __func__);

    pcm_close(p_ses->pcm);
    p_ses->pcm = NULL;
    ape_free_pcm_device_id(p_ses);

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int ape_reg_sm_params(sound_trigger_session_t* p_ses)
{
    int status = 0, buf_en = 0;
    int retry_num = 0;
    struct snd_lsm_detection_params params;

    ALOGD("%s:[%d] Enter", __func__, p_ses->sm_handle);
    if (!p_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }
    ape_enable_use_case(true, p_ses);

    status = pcm_start(p_ses->pcm);
    while (status && (retry_num < SOUND_TRIGGER_PCM_MAX_RETRY)) {
        usleep(SOUND_TRIGGER_PCM_SLEEP_WAIT);
        retry_num++;
        ALOGI("%s: pcm_start retrying..status %d errno %d, retry cnt %d",
              __func__, status, errno, retry_num);
        status = pcm_start(p_ses->pcm);
    }
    if (status) {
        ALOGE("%s: ERROR. pcm_start failed, returned status %d",
              __func__, status);
        goto exit;
    }

    params.conf_level = p_ses->conf_levels;
    params.num_confidence_levels = p_ses->num_conf_levels;
    params.detect_failure = stdev->detect_failure;

    /* SVA doen't support per keyword recogntion mode.
       Use the per soundmodel recognition mode */
    if(p_ses->recognition_mode & RECOGNITION_MODE_VOICE_TRIGGER){
        params.detect_mode = LSM_MODE_KEYWORD_ONLY_DETECTION;
        if (p_ses->recognition_mode & RECOGNITION_MODE_USER_IDENTIFICATION)
            params.detect_mode = LSM_MODE_USER_KEYWORD_DETECTION;
    }
    else {
        ALOGE("%s: Unknown recognition mode %d", __func__, p_ses->recognition_mode);
        goto exit;
    }

    {
        unsigned int i;
        ALOGV("%s: dm %d df %d ncl %d lab %d", __func__,
              params.detect_mode, params.detect_failure,
              params.num_confidence_levels, p_ses->capture_requested);
        for (i = 0; i < params.num_confidence_levels; i++) {
            ALOGV("%s: conf_levels[%d] = %d", __func__,
                                i, params.conf_level[i]);
        }
    }

    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_SET_PARAMS, &params);
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_SET_PARAMS, status=%d", __func__, status);
        goto exit;
    }

    buf_en = p_ses->capture_requested ? 1: 0;
    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_LAB_CONTROL, &buf_en);
    if (status)
        ALOGE("%s: ERROR. SNDRV_LSM_LAB_CONTROL failed, status=%d", __func__, status);

exit:
    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int ape_dereg_sm_params(sound_trigger_session_t* p_ses)
{
     free(p_ses->conf_levels);
     free(p_ses->event);
     ape_enable_use_case(false, p_ses);

     return 0;
}

static int ape_start(sound_trigger_session_t* p_ses)
{
    int status = 0;
    pthread_attr_t attr;

    ALOGD("%s:[%d] Enter", __func__, p_ses->sm_handle);
    if (!p_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_START);
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_START failed, status=%d", __func__, status);
    }

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int ape_stop(sound_trigger_session_t* p_ses)
{
    int status = 0;

    ALOGI("%s:[%d] Enter", __func__, p_ses->sm_handle);
    if (!p_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_STOP);
    if(status)
        ALOGE("%s: ERROR. SNDDRV_LSM_STOP failed, status=%d", __func__, status);

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int ape_stop_buffering(sound_trigger_session_t* p_ses)
{
    int status = 0;

    ALOGD("%s:[%d] Enter pcm %p", __func__, p_ses->sm_handle, p_ses->pcm);
    if (!p_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_STOP_LAB);
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_STOP_BUFFERING failed status %d", __func__, status);
    } else {
        status = pcm_ioctl(p_ses->pcm, SNDRV_PCM_IOCTL_RESET);
        if(status)
            ALOGE("%s: ERROR. SNDRV_PCM_IOCTL_RESET failed status %d", __func__, status);
    }
    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int cpe_reg_sm(sound_trigger_session_t *p_ses)
{
    int status = 0;
    struct snd_lsm_session_data ses_data;
    struct snd_lsm_sound_model_v2 lsm_params_v2 = {0,};
    pthread_attr_t attr;

    ALOGD("%s:[%d] Enter", __func__, p_ses->sm_handle);

    p_ses->pcm_id = cpe_get_pcm_device_id(p_ses);
    if(p_ses->pcm_id < 0)
        return -ENODEV;

    status = platform_stdev_send_calibration(stdev->platform,
                                              stdev->capture_device,
                                              p_ses->exec_mode,
                                              LSM_VOICE_WAKEUP_APP_ID);
    if (status) {
        ALOGE("%s: ERROR. sending calibration failed", __func__);
        goto sm_error;
    }

    p_ses->config = stdev_cpe_pcm_config;
    ALOGD("%s: opening pcm device=%d", __func__, p_ses->pcm_id);
    ALOGV("%s: config: channels=%d rate=%d, period_size=%d, "
          "period_cnt=%d, format=%d", __func__, p_ses->config.channels,
          p_ses->config.rate, p_ses->config.period_size,
          p_ses->config.period_count, p_ses->config.format);
    p_ses->pcm = pcm_open(SOUND_CARD, p_ses->pcm_id,
                          PCM_IN, &p_ses->config);
    if (!p_ses->pcm) {
        ALOGE("%s: ERROR. pcm_open failed", __func__);
        status = -ENODEV;
        goto sm_error;
    }
    if (!pcm_is_ready(p_ses->pcm)) {
        ALOGE("%s: ERROR. pcm_is_ready failed err=%s", __func__,
              pcm_get_error(p_ses->pcm));
        status = -ENODEV;
        goto sm_error;
    }

    lsm_params_v2.data = p_ses->sm_data;
    lsm_params_v2.data_size = p_ses->sm_size;
    ALOGD("%s: data=%p, data_size=%d, conf_level=%p, num_conf_level=%d",
           __func__, lsm_params_v2.data, lsm_params_v2.data_size,
          lsm_params_v2.confidence_level, lsm_params_v2.num_confidence_levels);
    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_REG_SND_MODEL_V2, &lsm_params_v2);
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_REG_SND_MODEL_V2 failed, status=%d", __func__, status);
        goto sm_error;
    }

    /* Callback thread to wait on event detection */
    p_ses->exit_callback_thread = false;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&p_ses->callback_thread, &attr,
                        callback_thread_loop, p_ses);

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return 0;

sm_error:
    cpe_free_pcm_device_id(p_ses);
    if (p_ses->pcm) {
        pcm_close(p_ses->pcm);
        p_ses->pcm = NULL;
    }

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int cpe_dereg_sm(sound_trigger_session_t *p_ses)
{
    int status = 0;

    ALOGD("%s:[%d] Enter", __func__, p_ses->sm_handle);

    if (!p_ses->pcm) {
        ALOGV("%s: pcm NULL", __func__);
        return status;
    }

    /* Exit the callback thread waiting on event detection */
    p_ses->exit_callback_thread = true;
    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_ABORT_EVENT);
    if (status)
        ALOGE("%s: SNDRV_LSM_ABORT_EVENT failed, status=%d", __func__, status);

    pthread_mutex_unlock(&p_ses->lock);
    pthread_join(p_ses->callback_thread, (void **) NULL);
    pthread_mutex_lock(&p_ses->lock);


    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_DEREG_SND_MODEL);

    if (status)
        ALOGE("%s: ERROR. SNDRV_LSM_DEREG_SND_MODEL failed", __func__);

    pcm_close(p_ses->pcm);
    p_ses->pcm = NULL;
    cpe_free_pcm_device_id(p_ses);

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int cpe_reg_sm_params(sound_trigger_session_t* p_ses)
{
    int status = 0, buf_en = 0;
    int retry_num = 0;
    struct snd_lsm_detection_params params;

    ALOGD("%s:[%d] Enter", __func__, p_ses->sm_handle);
    if (!p_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    status = pcm_start(p_ses->pcm);
    while (status && (retry_num < SOUND_TRIGGER_PCM_MAX_RETRY)) {
        usleep(SOUND_TRIGGER_PCM_SLEEP_WAIT);
        retry_num++;
        ALOGI("%s: pcm_start retrying..status %d errno %d, retry cnt %d",
              __func__, status, errno, retry_num);
        status = pcm_start(p_ses->pcm);
    }
    if (status) {
        ALOGE("%s: ERROR. pcm_start failed, returned status %d",
              __func__, status);
        goto exit;
    }

    params.conf_level = p_ses->conf_levels;
    params.num_confidence_levels = p_ses->num_conf_levels;
    params.detect_failure = stdev->detect_failure;

    /* SVA doen't support per keyword recogntion mode.
       Use the per soundmodel recognition mode */
    if(p_ses->recognition_mode & RECOGNITION_MODE_VOICE_TRIGGER){
        params.detect_mode = LSM_MODE_KEYWORD_ONLY_DETECTION;
        if (p_ses->recognition_mode & RECOGNITION_MODE_USER_IDENTIFICATION)
            params.detect_mode = LSM_MODE_USER_KEYWORD_DETECTION;
    }
    else {
        ALOGE("%s: Unknown recognition mode %d", __func__, p_ses->recognition_mode);
        goto exit;
    }

    {
        unsigned int i;
        ALOGD("%s: dm %d df %d ncl %d lab %d", __func__,
              params.detect_mode, params.detect_failure,
              params.num_confidence_levels, p_ses->capture_requested);
        for (i = 0; i < params.num_confidence_levels; i++) {
            ALOGD("%s: conf_levels[%d] = %d", __func__, i, params.conf_level[i]);
        }
    }

    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_SET_PARAMS, &params);
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_SET_PARAMS, status=%d", __func__, status);
        goto exit;
    }

    buf_en = p_ses->capture_requested ? 1: 0;
    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_LAB_CONTROL, &buf_en);
    if (status)
        ALOGE("%s: ERROR. SNDRV_LSM_LAB_CONTROL failed, status=%d", __func__, status);

exit:
    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int cpe_dereg_sm_params(sound_trigger_session_t* p_ses)
{
    free(p_ses->conf_levels);
    free(p_ses->event);

    return 0;
}

static int cpe_start(sound_trigger_session_t* p_ses)
{
    int status = 0;
    pthread_attr_t attr;

    ALOGD("%s:[%d] Enter", __func__, p_ses->sm_handle);
    if (!p_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_START);
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_START failed, status=%d", __func__, status);
    }

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int cpe_stop(sound_trigger_session_t* p_ses)
{
    int status = 0;

    ALOGI("%s:[%d] Enter", __func__, p_ses->sm_handle);
    if (!p_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_STOP);
    if(status)
        ALOGE("%s: ERROR. SNDDRV_LSM_STOP failed, status=%d", __func__, status);

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int cpe_stop_buffering(sound_trigger_session_t* p_ses)
{
    int status = 0;

    ALOGD("%s:[%d] Enter pcm %p", __func__, p_ses->sm_handle, p_ses->pcm);
    if (!p_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_STOP_LAB);
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_STOP_BUFFERING failed status %d", __func__, status);
    } else {
        status = pcm_ioctl(p_ses->pcm, SNDRV_PCM_IOCTL_RESET);
        if(status)
            ALOGE("%s: ERROR. SNDRV_PCM_IOCTL_RESET failed status %d", __func__, status);
    }
    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int route_reg_sm_ape(sound_trigger_session_t *p_ses)
{
    return ape_reg_sm(p_ses);
}

static int route_reg_sm_params_ape(sound_trigger_session_t* p_ses)
{
    return ape_reg_sm_params(p_ses);
}

static int route_dereg_sm_params_ape(sound_trigger_session_t* p_ses)
{
    return ape_dereg_sm_params(p_ses);
}

static int route_dereg_sm_ape(sound_trigger_session_t* p_ses)
{
    int status = 0;

    status = ape_dereg_sm(p_ses);

    return status;
}

static int route_start_ape(sound_trigger_session_t* p_ses)
{
    /* Pass the pcm information to audio hal for capturing LAB */
    if (p_ses->capture_requested && stdev->audio_hal_cb) {
        ALOGD("%s: ST_EVENT_SESSION_REGISTER capture_handle %d pcm %p", __func__,
              p_ses->capture_handle, p_ses->pcm);
        p_ses->event_info.st_ses.capture_handle = p_ses->capture_handle;
        p_ses->event_info.st_ses.pcm = p_ses->pcm;
        p_ses->event_info.st_ses.config = p_ses->config;
        stdev->audio_hal_cb(ST_EVENT_SESSION_REGISTER, &p_ses->event_info);
    }

    return ape_start(p_ses);
}

static int route_stop_ape(sound_trigger_session_t* p_ses)
{
    /* Indicate to audio hal that buffering is stopped to stop reading LAB data */
    if (p_ses->capture_requested && stdev->audio_hal_cb) {
        ALOGD("%s: ST_EVENT_SESSION_DEREGISTER capture_handle %d pcm %p", __func__,
              p_ses->event_info.st_ses.capture_handle, p_ses->event_info.st_ses.pcm);
        stdev->audio_hal_cb(ST_EVENT_SESSION_DEREGISTER, &p_ses->event_info);
    }

    return ape_stop(p_ses);
}

static int route_stop_buffering_ape(sound_trigger_session_t* p_ses)
{
    int status = 0;

    if (!CHECK_STATE(p_ses->state, SES_BUFFERING)) {
        return status;
    }
    status = ape_stop_buffering(p_ses);

    CLEAR_STATE(p_ses->state, SES_BUFFERING);
    return status;
}

static int route_reg_sm_cpe(sound_trigger_session_t *p_ses)
{
    return cpe_reg_sm(p_ses);
}

static int route_reg_sm_params_cpe(sound_trigger_session_t* p_ses)
{
    return cpe_reg_sm_params(p_ses);
}

static int route_dereg_sm_params_cpe(sound_trigger_session_t* p_ses)
{
    return cpe_dereg_sm_params(p_ses);
}

static int route_dereg_sm_cpe(sound_trigger_session_t* p_ses)
{
    return cpe_dereg_sm(p_ses);
}

static int route_start_cpe(sound_trigger_session_t* p_ses)
{
    /* Pass the pcm information to audio hal for capturing LAB */
    if (p_ses->capture_requested && stdev->audio_hal_cb) {
        ALOGD("%s: ST_EVENT_SESSION_REGISTER capture_handle %d pcm %p", __func__,
              p_ses->capture_handle, p_ses->pcm);
        p_ses->event_info.st_ses.capture_handle = p_ses->capture_handle;
        p_ses->event_info.st_ses.pcm = p_ses->pcm;
        p_ses->event_info.st_ses.config = p_ses->config;
        stdev->audio_hal_cb(ST_EVENT_SESSION_REGISTER, &p_ses->event_info);
    }

    return cpe_start(p_ses);
}

static int route_stop_cpe(sound_trigger_session_t* p_ses)
{

    /* Indicate to audio hal that buffering is stopped to stop reading LAB data */
    if (p_ses->capture_requested && stdev->audio_hal_cb) {
        ALOGD("%s: ST_EVENT_SESSION_DEREGISTER capture_handle %d pcm %p", __func__,
              p_ses->event_info.st_ses.capture_handle, p_ses->event_info.st_ses.pcm);
        stdev->audio_hal_cb(ST_EVENT_SESSION_DEREGISTER, &p_ses->event_info);
    }

    return cpe_stop(p_ses);
}

static int route_stop_buffering_cpe(sound_trigger_session_t* p_ses)
{
    int status = 0;

    if (!CHECK_STATE(p_ses->state, SES_BUFFERING)) {
        return status;
    }
    status = cpe_stop_buffering(p_ses);

    CLEAR_STATE(p_ses->state, SES_BUFFERING);
    return status;
}

static struct sound_trigger_session* get_sound_trigger_session(
                    struct sound_trigger_device *stdev,
                    sound_model_handle_t sound_model_handle)
{
    struct sound_trigger_session *st_session = NULL;
    struct listnode *node;
    list_for_each(node, &stdev->sound_model_list) {
        st_session = node_to_item(node, struct sound_trigger_session, list_node);
        if (st_session->sm_handle == sound_model_handle)
            return st_session;
    }
    return NULL;
}

static int stop_recognition_l(struct sound_trigger_session *st_session)
{
    int status = 0;

    st_session->callback = NULL;
    CLEAR_STATE(st_session->state, SES_STARTED|SES_EVENT_RECEIVED);
    stdev->dev_ref_cnt--;
    /* Audio concurrency would have already stopped the recognition */
    if (stdev->audio_concurrency_active)  {
        ALOGD("%s: audio concurrency active", __func__);
        st_session->fptrs->dereg_sm_params(st_session);
        return status;
    }

    st_session->fptrs->stop_buffering(st_session);

    status = st_session->fptrs->stop(st_session);

    st_session->fptrs->dereg_sm_params(st_session);

    if(stdev->dev_ref_cnt == 0) {
        sound_trigger_set_device(false, stdev->capture_device);
        stdev->device_enabled = false;
    }

    return status;
}

static int stdev_get_properties(const struct sound_trigger_hw_device *dev,
                                struct sound_trigger_properties *properties)
{
    struct sound_trigger_device *stdev = (struct sound_trigger_device *)dev;

    ALOGI("%s", __func__);
    if (properties == NULL) {
        ALOGE("%s: NULL properties", __func__);
        return -EINVAL;
    }

    if (stdev->transition_enabled || stdev->run_on_ape) {
        hw_properties.max_sound_models = stdev->max_ape_sessions;
        hw_properties.max_key_phrases = stdev->avail_ape_phrases;
        hw_properties.max_users = stdev->avail_ape_users;
        hw_properties.max_buffer_ms = SOUND_TRIGGER_APE_BUFFER_DURATION_MS;
    }
    else {
        hw_properties.max_sound_models = stdev->max_cpe_sessions;
        hw_properties.max_key_phrases = stdev->avail_cpe_phrases ;
        hw_properties.max_users = stdev->avail_cpe_users ;
        hw_properties.max_buffer_ms = SOUND_TRIGGER_CPE_BUFFER_DURATION_MS;
    }

    memcpy(properties, &hw_properties, sizeof(struct sound_trigger_properties));
    return 0;
}

static st_exec_mode_t determine_exec_mode
(
   unsigned int num_phrases,
   unsigned int num_users
)
{
    unsigned int num_sessions = 0;
    st_exec_mode_t exec_mode = ST_DEVICE_EXEC_MODE_NONE;

    num_sessions = get_num_sessions();

    ALOGD("%s: Enter - KW=%d UP=%d AKW=%d AKWUP=%d CKW=%d CKWUP=%d pm=%d tm=%d"
          "ncs=%d ns=%d", __func__, num_phrases, num_users,
          stdev->avail_ape_phrases, stdev->avail_ape_users,
          stdev->avail_cpe_phrases , stdev->avail_cpe_users,
          stdev->run_on_ape, stdev->transition_enabled,
          stdev->num_sessions_configured, num_sessions);

    if (stdev->transition_enabled) {
        /* Platform configured to run either on CPE/ADSP. Ignore other platform
        configuration parameter to run on CPE only or ADSP only. If Client
        configured sessions are 1, open that first session either on CPE/ADSP,
        else open on ADSP. Client must configure number of sessions to be
        opened through setParameters() and close previous sessions before
        openening new set of sessions */
        if (stdev->num_sessions_configured > 1) {
            if (stdev->exec_mode == ST_DEVICE_EXEC_MODE_CPE) {
                ALOGE("%s: ERROR. Close existing CPE session", __func__);
            }
            else if ((num_sessions < stdev->max_ape_sessions) &&
                (num_phrases <= stdev->avail_ape_phrases ) &&
                (num_users <= stdev->avail_ape_users )) {
                exec_mode = ST_DEVICE_EXEC_MODE_APE;
            }
        }
        else if ((stdev->num_sessions_configured == 1) && !num_sessions) {
            /* Open the first session either on CPE/ADSP */
            if ((num_sessions < stdev->max_cpe_sessions) &&
                (num_phrases <= stdev->avail_cpe_phrases) &&
                (num_users <= stdev->avail_cpe_users)) {
                exec_mode = ST_DEVICE_EXEC_MODE_CPE;
            }
            else if ((num_sessions < stdev->max_ape_sessions) &&
                (num_phrases <= stdev->avail_ape_phrases ) &&
                (num_users <= stdev->avail_ape_users )) {
                exec_mode = ST_DEVICE_EXEC_MODE_APE;
            }
        }
    }
    else if (!stdev->run_on_ape) {
        /* Platform configured for CPE only */
        if ((num_sessions < stdev->max_cpe_sessions) &&
            (num_phrases <= stdev->avail_cpe_phrases) &&
            (num_users <= stdev->avail_cpe_users)) {
            exec_mode = ST_DEVICE_EXEC_MODE_CPE;
        }
    }
    else {
        /* Platform configured for ADSP only */
        if ((num_sessions < stdev->max_ape_sessions) &&
            (num_phrases <= stdev->avail_ape_phrases ) &&
            (num_users <= stdev->avail_ape_users )) {
            exec_mode = ST_DEVICE_EXEC_MODE_APE;
        }
    }

    ALOGV("%s: Exit  exec_mode=%d", __func__, exec_mode);
    return exec_mode;
}

static int stdev_load_sound_model(const struct sound_trigger_hw_device *dev,
                                  struct sound_trigger_sound_model *sound_model,
                                  sound_model_callback_t callback __unused,
                                  void *cookie __unused,
                                  sound_model_handle_t *handle)
{
    int status = 0;
    unsigned int i = 0, j = 0;
    unsigned int total_num_users = 0;
    st_exec_mode_t exec_mode;
    struct sound_trigger_session *st_session = NULL;
    struct sound_trigger_phrase_sound_model *phrase_sm =
            (struct sound_trigger_phrase_sound_model*)sound_model;

    ALOGD("%s", __func__);
    pthread_mutex_lock(&stdev->lock);
    if (stdev->ssr_in_progress) {
        /* Wait till SSR completes */
        ALOGV("%s: waiting on SSR completion", __func__);
        pthread_cond_wait(&stdev->cond, &stdev->lock);
    }

    if (handle == NULL || sound_model == NULL) {
        status = -EINVAL;
        goto exit;
    }
    if (!platform_stdev_is_session_allowed(stdev->platform, get_num_sessions(), false)) {
        ALOGW("%s: session prevented by concurrency", __func__);
        status = -ENODEV;
        goto exit;
    }

    /* Current version supports only key phrase detection */
    if ((phrase_sm->common.data_size == 0) ||
        (phrase_sm->common.data_offset < sizeof(*phrase_sm)) ||
        (phrase_sm->common.type != SOUND_MODEL_TYPE_KEYPHRASE) ||
        (phrase_sm->num_phrases == 0)) {
        ALOGE("%s: Invalid sound model params data size=%d, data offset=%d, "
              "type=%d phrases=%d", __func__, phrase_sm->common.data_size,
              phrase_sm->common.data_offset, phrase_sm->common.type,
              phrase_sm->num_phrases);
        status = -EINVAL;
        goto exit;
    }

    st_session = calloc(1, sizeof(struct sound_trigger_session));
    if (!st_session) {
        status = -ENOMEM;
        goto exit;
    }

    /* CPE takes time to become online, so parse for the pcm devices
       here instead during boot time */
    if (!CHECK_BIT(stdev->hw_type, ST_DEVICE_HW_APE|ST_DEVICE_HW_CPE)) {
        status = stdev_get_hw_type();
        if (status)
            goto exit;
    }

    for (i = 0; i < phrase_sm->num_phrases; i++) {
        for (j = 0; j < phrase_sm->phrases[i].num_users; j++) {
            total_num_users++;
        }
    }

    exec_mode = determine_exec_mode(phrase_sm->num_phrases, total_num_users);
    if (exec_mode == ST_DEVICE_EXEC_MODE_NONE) {
        status = -EINVAL;
        goto exit;
    }

    st_session->exec_mode = exec_mode;
    st_session->num_phrases = phrase_sm->num_phrases;
    st_session->num_users = total_num_users;

    if (ST_DEVICE_EXEC_MODE_CPE == exec_mode) {
        ALOGD("%s: CPE mode", __func__);
        st_session->fptrs = &cpe_fptrs;
    } else {
        ALOGD("%s: APE mode", __func__);
        st_session->fptrs = &ape_fptrs;
    }

    /* Store the sound model data to send again when SSR happens */
    st_session->sm_data = calloc(1, phrase_sm->common.data_size);
    if (!st_session->sm_data) {
        status = -ENOMEM;
        goto exit;
    }
    memcpy(st_session->sm_data,
           (char *)phrase_sm + phrase_sm->common.data_offset,
           phrase_sm->common.data_size);
    st_session->sm_size = phrase_sm->common.data_size;
    st_session->sm_type = phrase_sm->common.type;
    ALOGD("%s: sm magic number 0x%x rm %d", __func__, ((int *)st_session->sm_data)[0],
          phrase_sm->phrases[0].recognition_mode);

    /* TODO: SVA doesn't support per keyword recognition mode.
       So use the first phrase recognition mode. App is supposed
       to set the proper recognition mode in the first phrase */
    st_session->recognition_mode = phrase_sm->phrases[0].recognition_mode;
    pthread_mutex_init(&st_session->lock, (const pthread_mutexattr_t *) NULL);

    /* No need for session lock as session is not pushed to list and
       the ssr/concurrencies are guarded with dev lock */
    status = st_session->fptrs->reg_sm(st_session);
    if (status)
        goto exit;

    st_session->sm_handle = android_atomic_inc(&stdev->session_id);
    st_session->device = (void *)dev;
    *handle = st_session->sm_handle;

    stdev->exec_mode = exec_mode;
    if (ST_DEVICE_EXEC_MODE_CPE == exec_mode) {
        stdev->run_on_ape = false;
        stdev->avail_cpe_phrases  -= st_session->num_phrases;
        stdev->avail_cpe_users  -= st_session->num_users;
    } else {
        stdev->run_on_ape = true;
        stdev->avail_ape_phrases -= st_session->num_phrases;
        stdev->avail_ape_users -= st_session->num_users;
    }

    /* Add the session to the list of registered sound models list */
    list_add_tail(&stdev->sound_model_list, &st_session->list_node);
    pthread_mutex_unlock(&stdev->lock);

    ALOGD("%s: success, sound_model_handle %d", __func__, st_session->sm_handle);
    return 0;

exit:
    if (st_session != NULL) {
        if (st_session->sm_data) {
            free(st_session->sm_data);
            pthread_mutex_destroy(&st_session->lock);
        }
        free(st_session);
        *handle = -1;
    }
    pthread_mutex_unlock(&stdev->lock);
    return status;
}

static int stdev_unload_sound_model(const struct sound_trigger_hw_device *dev,
                                    sound_model_handle_t handle)
{
    struct sound_trigger_device *stdev = (struct sound_trigger_device *)dev;
    struct sound_trigger_session *st_session = NULL;
    int status = 0;

    ALOGD("%s: Enter sound model %d", __func__, handle);

    pthread_mutex_lock(&stdev->lock);
    if (stdev->ssr_in_progress) {
        /* Wait till SSR completes */
        ALOGV("%s: waiting on SSR completion", __func__);
        pthread_cond_wait(&stdev->cond, &stdev->lock);
    }

    st_session = get_sound_trigger_session(stdev, handle);
    if (!st_session || !st_session->fptrs) {
        ALOGE("%s Could not find sound model %d", __func__, handle);
        status = -EINVAL;
        goto exit;
    }

    pthread_mutex_lock(&st_session->lock);
    if (st_session->callback)
        status = stop_recognition_l(st_session);

    status = st_session->fptrs->dereg_sm(st_session);

    list_remove(&st_session->list_node);

    if (st_session->exec_mode == ST_DEVICE_EXEC_MODE_CPE) {
        stdev->avail_cpe_phrases  += st_session->num_phrases;
        stdev->avail_cpe_users  += st_session->num_users;
    } else {
        st_session->fptrs = &ape_fptrs;
        stdev->avail_ape_phrases += st_session->num_phrases;
        stdev->avail_ape_users += st_session->num_users;
    }
    pthread_mutex_unlock(&st_session->lock);

    pthread_mutex_destroy(&st_session->lock);
    if(!get_num_sessions())
        stdev->exec_mode = ST_DEVICE_EXEC_MODE_NONE;
    free(st_session->sm_data);
    free(st_session);

exit:
    pthread_mutex_unlock(&stdev->lock);
    ALOGD("%s: Exit status %d", __func__, status);
    return status;
}

static int stdev_start_recognition
(
   const struct sound_trigger_hw_device *dev,
   sound_model_handle_t sound_model_handle,
   const struct sound_trigger_recognition_config *config,
   recognition_callback_t callback,
   void *cookie
)
{
    struct sound_trigger_device *stdev = (struct sound_trigger_device *)dev;
    struct sound_trigger_session *st_session = NULL;
    int status = 0;
    unsigned int total_num_users = 0;

    ALOGD("%s:[%d] Enter", __func__, sound_model_handle);

    pthread_mutex_lock(&stdev->lock);
    if (stdev->ssr_in_progress) {
        /* Wait till SSR completes */
        ALOGV("%s: waiting on SSR completion", __func__);
        pthread_cond_wait(&stdev->cond, &stdev->lock);
    }

    if (!callback || !config) {
        ALOGE("%s: ERROR. NULL params", __func__);
        status = -EINVAL;
        goto exit;
    }

    st_session = get_sound_trigger_session(stdev, sound_model_handle);
    if (!st_session) {
        ALOGE("%s: ERROR. Could not find session for sound model %d", __func__, sound_model_handle);
        status = -EINVAL;
        goto exit;
    }

    pthread_mutex_lock(&st_session->lock);
    /* Client might have called this function after event is detected.
       As we haven't stopped recognition after detection, exit from here */
    if (CHECK_STATE(st_session->state, SES_EVENT_RECEIVED))
        goto ses_exit;

    if (!st_session->pcm || !st_session->fptrs) {
        ALOGE("%s: ERROR. Could not find sound model %d", __func__, sound_model_handle);
        status = -EINVAL;
        goto ses_exit;
    }

    status = get_confidence_levels_payload(st_session, config);
    if (status)
        goto ses_exit;

    /* If audio capture is active, start recogntion later after capture is inactive */
    if (!platform_stdev_is_session_allowed(stdev->platform, get_num_sessions(), true)) {
        ALOGD("%s: audio concurrency active", __func__);
        stdev->dev_ref_cnt++;
        SET_STATE(st_session->state, SES_STARTED);
        goto ses_exit;
    }

    st_session->callback = callback;
    st_session->cookie = cookie;
    /* capture_handle will be shared with AHAL for reading LAB data */
    st_session->capture_handle = config->capture_handle;
    st_session->capture_requested = config->capture_requested;
    stdev->capture_device = config->capture_device;

    if(stdev->dev_ref_cnt == 0) {
        sound_trigger_set_device(true, stdev->capture_device);
        stdev->device_enabled = true;
    }
    status = st_session->fptrs->reg_sm_params(st_session);
    if(status)
        goto ses_exit;

    status = st_session->fptrs->start(st_session);
    if (status)
        goto ses_exit;

    stdev->dev_ref_cnt++;
    SET_STATE(st_session->state, SES_STARTED);
    pthread_mutex_unlock(&st_session->lock);
    pthread_mutex_unlock(&stdev->lock);

    ALOGD("%s:[%d] Exit", __func__, sound_model_handle);
    return 0;

ses_exit:
    if(stdev->dev_ref_cnt == 0 && stdev->device_enabled) {
        sound_trigger_set_device(false, stdev->capture_device);
        stdev->device_enabled = false;
    }
    pthread_mutex_unlock(&st_session->lock);

exit:
    pthread_mutex_unlock(&stdev->lock);
    ALOGD("%s:[%d] Exit status %d", __func__, sound_model_handle, status);
    return status;
}

static int stdev_stop_recognition(const struct sound_trigger_hw_device *dev,
                                 sound_model_handle_t sound_model_handle)
{
    struct sound_trigger_device *stdev = (struct sound_trigger_device *)dev;
    struct sound_trigger_session *st_session = NULL;
    int status = 0;

    ALOGD("%s:[%d] Enter", __func__, sound_model_handle);

    pthread_mutex_lock(&stdev->lock);
    if (stdev->ssr_in_progress) {
        /* Wait till SSR completes */
        ALOGV("%s: waiting on SSR completion", __func__);
        pthread_cond_wait(&stdev->cond, &stdev->lock);
    }
    st_session = get_sound_trigger_session(stdev, sound_model_handle);
    if (st_session == NULL) {
        ALOGE("%s: Could not find sound model %d", __func__, sound_model_handle);
        status = -EINVAL;
        pthread_mutex_unlock(&stdev->lock);
        return status;
    }

    pthread_mutex_lock(&st_session->lock);
    status = stop_recognition_l(st_session);
    pthread_mutex_unlock(&st_session->lock);

    pthread_mutex_unlock(&stdev->lock);
    ALOGD("%s:[%d] Exit status %d", __func__, sound_model_handle, status);
    return status;
}

static int stdev_close(hw_device_t *device)
{
    struct sound_trigger_device *stdev = (struct sound_trigger_device *)device;

    ALOGD("%s: count=%d", __func__, stdev_ref_cnt);

    pthread_mutex_lock(&stdev_init_lock);
    if (!stdev || (--stdev_ref_cnt != 0)) {
        goto exit;
    }

    pthread_mutex_lock(&stdev->lock);
    platform_stdev_deinit(stdev->platform);
    free(stdev->cpe_pcm_use_cases);
    free(stdev->ape_pcm_use_cases);
    pthread_mutex_unlock(&stdev->lock);

    pthread_mutex_destroy(&stdev->lock);
    free(device);
    stdev = NULL;

exit:
    pthread_mutex_unlock(&stdev_init_lock);
    ALOGD("%s: Exit device=%p cnt=%d ", __func__, stdev, stdev_ref_cnt);
    return 0;
}

static int stdev_open(const hw_module_t* module, const char* name,
                     hw_device_t** device)
{
    int status = 0;
    char audio_hal_lib[100];

    ALOGD("%s: Enter", __func__);

    if (strcmp(name, SOUND_TRIGGER_HARDWARE_INTERFACE) != 0) {
        ALOGE("%s: ERROR. wrong interface", __func__);
        status = -EINVAL;
        goto exit;
    }

    pthread_mutex_lock(&stdev_init_lock);
    if (stdev_ref_cnt != 0) {
        *device = &stdev->device.common;
        stdev_ref_cnt++;
        ALOGD("%s: returning existing stdev instance, exit", __func__);
        pthread_mutex_unlock(&stdev_init_lock);
        return status;
    }

    stdev = calloc(1, sizeof(struct sound_trigger_device));
    if (!stdev) {
        ALOGE("%s: ERROR. stdev alloc failed", __func__);
        status = -ENOMEM;
        goto exit;
    }

    stdev->platform = platform_stdev_init(stdev);
    if (!stdev->platform) {
        ALOGE("%s: ERROR. platform init failed", __func__);
        status = -ENODEV;
        goto exit;
    }

    snprintf(audio_hal_lib, sizeof(audio_hal_lib),
             "/system/lib/hw/audio.primary.%s.so", XSTR(SOUND_TRIGGER_PLATFORM));
    stdev->audio_hal_handle = dlopen(audio_hal_lib, RTLD_NOW);
    if (!stdev->audio_hal_handle){
        ALOGE("%s: ERROR. dlopen failed for %s", __func__, audio_hal_lib);
        status = -ENODEV;
        goto exit_1;
    }

    stdev->audio_hal_cb = (audio_hw_call_back_t)dlsym(stdev->audio_hal_handle,
                                              "audio_hw_call_back");
    if (!stdev->audio_hal_cb){
        ALOGE("%s: ERROR. dlsym error %s for audio_hw_call_back", __func__, dlerror());
        status = -ENODEV;
        goto exit_1;
    }

    stdev->ape_pcm_use_cases =
        calloc(stdev->max_ape_sessions, sizeof(struct use_case_info));

    if (!stdev->ape_pcm_use_cases) {
        ALOGE("%s: ERROR. Mem alloc failed for ape use cases", __func__);
        status = -ENODEV;
        goto exit_1;
    }

    stdev->cpe_pcm_use_cases =
        calloc(stdev->max_cpe_sessions, sizeof(struct use_case_info));

    if (!stdev->cpe_pcm_use_cases) {
        ALOGE("%s: ERROR. Mem alloc failed for cpe use cases", __func__);
        status = -ENODEV;
        goto exit_1;
    }
    stdev->device.common.tag = HARDWARE_DEVICE_TAG;
    stdev->device.common.version = SOUND_TRIGGER_DEVICE_API_VERSION_1_0;
    stdev->device.common.module = (struct hw_module_t *) module;
    stdev->device.common.close = stdev_close;
    stdev->device.get_properties = stdev_get_properties;
    stdev->device.load_sound_model = stdev_load_sound_model;
    stdev->device.unload_sound_model = stdev_unload_sound_model;
    stdev->device.start_recognition = stdev_start_recognition;
    stdev->device.stop_recognition = stdev_stop_recognition;
    stdev->session_id = 1;
    stdev->exec_mode = ST_DEVICE_EXEC_MODE_NONE;

    pthread_mutex_init(&stdev->lock, (const pthread_mutexattr_t *) NULL);
    list_init(&stdev->sound_model_list);

    *device = &stdev->device.common;
    stdev_ref_cnt++;
    pthread_mutex_unlock(&stdev_init_lock);

    return 0;

exit_1:
    if (stdev->cpe_pcm_use_cases)
        free(stdev->cpe_pcm_use_cases);

    if (stdev->ape_pcm_use_cases)
        free(stdev->ape_pcm_use_cases);

    if(stdev->audio_hal_handle)
        dlclose(stdev->audio_hal_handle);

    if(stdev->platform)
       platform_stdev_deinit(stdev->platform);

exit:

    if(stdev)
        free(stdev);
    stdev = NULL;
    *device = NULL;

    pthread_mutex_unlock(&stdev_init_lock);
    ALOGD("%s: Exit status %d", __func__, status);
    return status;
}

/* Audio hal calls this callback for notifying Subsystem restart,
   lab stop and concurrency events */
void sound_trigger_hw_call_back(audio_event_type_t event,
                               audio_event_info_t* config)
{
    struct listnode *p_ses_node;
    sound_trigger_session_t *p_ses;

    if (!stdev)
        return;

    switch (event) {
    case AUDIO_EVENT_CAPTURE_DEVICE_INACTIVE:
    case AUDIO_EVENT_CAPTURE_DEVICE_ACTIVE:
    case AUDIO_EVENT_PLAYBACK_STREAM_INACTIVE:
    case AUDIO_EVENT_PLAYBACK_STREAM_ACTIVE:
        handle_audio_concurrency(event);
        break;
    case AUDIO_EVENT_STOP_LAB:
        if(!config || !config->u.ses_info.pcm) {
            ALOGE("%s: NULL params for stop lab", __func__);
            break;
        }
        pthread_mutex_lock(&stdev->lock);
        list_for_each(p_ses_node, &stdev->sound_model_list) {
            p_ses = node_to_item(p_ses_node, sound_trigger_session_t, list_node);
            if (p_ses->pcm == config->u.ses_info.pcm) {
                p_ses->fptrs->stop_buffering(p_ses);
                break;
            }
        }
        pthread_mutex_unlock(&stdev->lock);
        break;
    case AUDIO_EVENT_SSR:
        if (!config) {
            ALOGE("%s: NULL config for SSR", __func__);
            break;
        }

        if ((stdev->run_on_ape &&
             (config->u.status == SND_CARD_STATUS_OFFLINE)) ||
            (!stdev->run_on_ape &&
             (config->u.status == CPE_STATUS_OFFLINE)))
            handle_ssr_offline();

        if ((stdev->run_on_ape &&
             (config->u.status == SND_CARD_STATUS_ONLINE)) ||
            (!stdev->run_on_ape &&
             (config->u.status == CPE_STATUS_ONLINE)))
            handle_ssr_online();

        break;
    case AUDIO_EVENT_NUM_ST_SESSIONS:
        if (!config) {
            ALOGE("%s: NULL config for AUDIO_EVENT_NUM_ST_SESSIONS", __func__);
            break;
        }
        pthread_mutex_lock(&stdev->lock);
        ALOGV("%s: num sessions configured %d", __func__, config->u.value);
        stdev->num_sessions_configured = config->u.value;
        pthread_mutex_unlock(&stdev->lock);
        break;
    default:
        ALOGW("%s: Unknown event %d", __func__, event);
        break;
    }
}

static struct hw_module_methods_t hal_module_methods = {
    .open = stdev_open,
};

struct sound_trigger_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = SOUND_TRIGGER_MODULE_API_VERSION_1_0,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = SOUND_TRIGGER_HARDWARE_MODULE_ID,
        .name = "Sound trigger HAL",
        .author = "QUALCOMM Technologies, Inc",
        .methods = &hal_module_methods,
    },
};
