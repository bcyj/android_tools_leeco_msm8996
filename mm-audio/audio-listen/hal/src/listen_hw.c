/* listen_hw.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_TAG "listen_hw"
#define LOG_NDEBUG 0
/* #define LOG_NDDEBUG 0 */
#include <cutils/log.h>
#include <cutils/str_parms.h>
#include <cutils/sched_policy.h>
#include <cutils/properties.h>
#include <system/thread_defs.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <dlfcn.h>
#include <hardware/audio.h>
#include "listen_platform.h"
#include "listen_hw.h"

static int route_open_session_ape(audio_listen_session_t *p_ses);
static int route_close_session_ape(audio_listen_session_t *p_ses);
static int route_reg_sm_ape (audio_listen_session_t *p_ses,
                              listen_sound_model_params_t *params);
static int route_dereg_sm_ape  (audio_listen_session_t* p_ses);
static int route_start_ape(audio_listen_session_t* p_ses);
static int route_stop_ape(audio_listen_session_t* p_ses);
static int route_read_buffer_ape(audio_listen_session_t* p_ses,
                                 uint8_t *p_read_buf,
                                 uint32_t *p_bytes);
static int route_stop_buffering_ape(audio_listen_session_t* p_ses);

static int route_open_session_cpe(audio_listen_session_t *p_ses);
static int route_close_session_cpe(audio_listen_session_t *p_ses);
static int route_reg_sm_cpe (audio_listen_session_t *p_ses,
                              listen_sound_model_params_t *params);
static int route_dereg_sm_cpe(audio_listen_session_t* p_ses);
static int route_start_cpe(audio_listen_session_t* p_ses);
static int route_stop_cpe(audio_listen_session_t* p_ses);
static int route_read_buffer_cpe(audio_listen_session_t* p_ses,
                                 uint8_t *p_read_buf,
                                 uint32_t *p_bytes);
static int route_stop_buffering_cpe(audio_listen_session_t* p_ses);
void get_num_of_active_sessions(int *cpe_ses, int *ape_ses);

static struct pcm_config ape_pcm_config_listen = {
    .channels = LISTEN_DEFAULT_CHANNEL_MODE,
    .rate = LISTEN_DEFAULT_SAMPLING_RATE,
    .period_size = LISTEN_APE_PERIOD_SIZE,
    .period_count = LISTEN_APE_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

static struct pcm_config cpe_pcm_config_listen = {
    .channels = LISTEN_DEFAULT_CHANNEL_MODE,
    .rate = LISTEN_DEFAULT_SAMPLING_RATE,
    .period_size = LISTEN_CPE_PERIOD_SIZE,
    .period_count = LISTEN_CPE_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct listen_session_fptrs ape_fptrs = {
    .open = route_open_session_ape,
    .close = route_close_session_ape,
    .reg_sm = route_reg_sm_ape ,
    .dereg_sm = route_dereg_sm_ape  ,
    .start = route_start_ape,
    .stop = route_stop_ape,
    .read_buffer = route_read_buffer_ape,
    .stop_buffering = route_stop_buffering_ape,
};

struct listen_session_fptrs cpe_fptrs = {
    .open = route_open_session_cpe,
    .close = route_close_session_cpe,
    .reg_sm = route_reg_sm_cpe ,
    .dereg_sm = route_dereg_sm_cpe  ,
    .start = route_start_cpe,
    .stop = route_stop_cpe,
    .read_buffer = route_read_buffer_cpe,
    .stop_buffering = route_stop_buffering_cpe,
};

static audio_listen_hardware_t *g_listen_hal = NULL;
static pthread_mutex_t listen_hw_init_lock;
/* count of number of listen hal clients */
static unsigned int listen_hw_ref_cnt = 0;

static int listen_get_hw_type(void)
{
    int dev_id;
    int i = 0, j = 0;
    char buf[100], comp_string[USECASE_STRING_SIZE];
    FILE *fp;

    fp = fopen(PCM_DEVICE_FILE, "r");

    if (!fp) {
        ALOGV("%s: %s file open failed", __func__, PCM_DEVICE_FILE);
        return -ENODEV;
    }
    snprintf(comp_string, USECASE_STRING_SIZE, "Listen %d Audio Service", i+1);

    while(1) {
        fgets(buf, 100, fp);
        if (feof(fp))
            break;
        sscanf(&buf[3],"%d", &dev_id);
        if((strstr(buf, comp_string)) &&
           (i < g_listen_hal->max_ape_sessions)) {

            g_listen_hal->hw_type |= LISTEN_HW_APE;
            snprintf(g_listen_hal->ape_pcm_use_cases[i].use_case,
                     USECASE_STRING_SIZE, "listen-voice-wakeup-%d", i+1);
            g_listen_hal->ape_pcm_use_cases[i++].pcm_id = dev_id;
            snprintf(comp_string, USECASE_STRING_SIZE, "Listen %d Audio Service", i+1);
        }
        else if(strstr(buf, "CPE Listen") &&
                (j < g_listen_hal->max_cpe_sessions)) {

            g_listen_hal->hw_type |= LISTEN_HW_CPE;
            g_listen_hal->cpe_pcm_use_cases[j++].pcm_id = dev_id;
        }
    }
    fclose(fp);

    if (!CHECK_BIT(g_listen_hal->hw_type, LISTEN_HW_APE|LISTEN_HW_CPE)) {
        ALOGV("%s: ERROR. No Listen device present", __func__);
        return -ENODEV;
    }
    if (g_listen_hal->max_ape_sessions > i)
        g_listen_hal->max_ape_sessions = i;
    if (g_listen_hal->max_cpe_sessions > j)
        g_listen_hal->max_cpe_sessions = j;

    return 0;
}

static void listen_select_device(void)
{
    g_listen_hal->cur_device = LISTEN_DEVICE_HANDSET_MIC;

    ALOGV("%s: current device (%d)", __func__,g_listen_hal->cur_device);
}

static int listen_hw_init(unsigned int snd_card,
                          struct audio_route *audio_route)
{
    int retry_num = 0;
    int status = 0;

    ALOGV("%s: Enter", __func__);

    /* Initialize the parameters */
    g_listen_hal->snd_card = snd_card;
    g_listen_hal->audio_route = audio_route;

    g_listen_hal->mixer = mixer_open(g_listen_hal->snd_card);

    while (!g_listen_hal->mixer && retry_num < RETRY_NUMBER) {
        usleep(RETRY_US);
        g_listen_hal->mixer = mixer_open(g_listen_hal->snd_card);
        retry_num++;
    }

    if (!g_listen_hal->mixer) {
        ALOGE("%s: ERROR. Unable to open the mixer, aborting", __func__);
        return -ENODEV;
    }

    g_listen_hal->platform = platform_listen_init(g_listen_hal);
    if (!g_listen_hal->platform) {
        ALOGE("%s: ERROR. platform init failed", __func__);
        status = -ENODEV;
        goto error;
    }

    g_listen_hal->ape_pcm_use_cases =
        calloc(g_listen_hal->max_ape_sessions, sizeof(struct use_case_info));
    if (!g_listen_hal->ape_pcm_use_cases) {
        ALOGE("%s: ERROR. Mem alloc failed for ape use cases", __func__);
        status = -ENODEV;
        goto error;
    }
    g_listen_hal->cpe_pcm_use_cases =
        calloc(g_listen_hal->max_cpe_sessions, sizeof(struct use_case_info));
    if (!g_listen_hal->cpe_pcm_use_cases) {
        ALOGE("%s: ERROR. Mem alloc failed for cpe use cases", __func__);
        status = -ENODEV;
        goto error;
    }

    pthread_mutex_init(&g_listen_hal->lock, (const pthread_mutexattr_t *) NULL);
    pthread_mutex_init(&g_listen_hal->list_lock, (const pthread_mutexattr_t *) NULL);

    listen_select_device();
    list_init(&g_listen_hal->ses_list);

    ALOGV("%s: Exit", __func__);
    return status;

error:
    if (g_listen_hal->mixer)
        mixer_close(g_listen_hal->mixer);

    if (g_listen_hal->ape_pcm_use_cases)
        free(g_listen_hal->ape_pcm_use_cases);

    if (g_listen_hal->cpe_pcm_use_cases)
        free(g_listen_hal->cpe_pcm_use_cases);

    ALOGV("%s: Exit, status=%d", __func__, status);
    return status;
}

static void listen_enable_device(bool enable, listen_device_t listen_device)
{
    char device_name[DEVICE_NAME_MAX_SIZE] = {0};

    if (listen_device < LISTEN_DEVICE_MIN ||
            listen_device >= LISTEN_DEVICE_MAX) {
        ALOGE("%s: Invalid listen device %d", __func__, listen_device);
        return;
    }

    if(platform_listen_get_device_name(g_listen_hal->platform, listen_device, device_name) < 0 ) {
        ALOGE("%s: Invalid listen device returned", __func__);
        return;
    }

    if (enable) {
        ALOGV("%s: enable listen device = %s", __func__, device_name);
        audio_route_apply_path(g_listen_hal->audio_route, device_name);
    } else {
        ALOGV("%s: diable listen device = %s", __func__,device_name);
        audio_route_reset_path(g_listen_hal->audio_route, device_name);
    }

    audio_route_update_mixer(g_listen_hal->audio_route);
}

static int set_listen_detection(audio_listen_session_t *p_ses, bool enable)
{
    int  ret = 0;

    if (CHECK_STATE(p_ses->op_state, SES_SM_REGD)) {
        if (enable) {
            ret = p_ses->lfptrs->start(p_ses);
        } else {
            ret = p_ses->lfptrs->stop(p_ses);
        }
    }
    return ret;
}

/* Called when SSR is complete.
 * Re-enable all ListenDriver session active before SSR.
 */
static void handle_snd_card_online(void)
{
    struct listnode *p_ses_node;
    audio_listen_session_t *p_ses;
    int status = 0;

    ALOGV("%s: Enter", __func__);

    pthread_mutex_lock(&g_listen_hal->lock);
    if(g_listen_hal->actual_mad_state == g_listen_hal->reqd_mad_state) {
        pthread_mutex_unlock(&g_listen_hal->lock);
        ALOGV("%s: Exit", __func__);
        return;
    }
    g_listen_hal->actual_mad_state = MAD_ENABLED;
    /* Reset the DAPM cached mixer control for MADONOFF switch as the kernel
       is not reseting this mixer control when codec resets */
    listen_enable_device(false, g_listen_hal->cur_device);
    listen_enable_device(true, g_listen_hal->cur_device);

    /* reload and reactive each previously active session */
    pthread_mutex_lock(&g_listen_hal->list_lock);
    /* list locked while looping thru each session */
    list_for_each(p_ses_node, &g_listen_hal->ses_list) {
        p_ses = node_to_item(p_ses_node, audio_listen_session_t, list_node);

        pthread_mutex_lock(&p_ses->lock);
        /* In case of ADSP SSR for CPE session, the codec hwmad resets to
           connect to ADSP. So connect hwmad back to CPE */
        if(p_ses->exec_mode == LISTEN_EXEC_MODE_CPE)
            platform_listen_connect_mad(g_listen_hal->platform, true, p_ses);

        status = p_ses->lfptrs->open(p_ses);
        if (status){
            SET_STATE(p_ses->op_state, SES_IN_BAD_STATE);
            goto cleanup;
        }
        status = p_ses->lfptrs->reg_sm(p_ses, &p_ses->reg_params);
        if (status){
            SET_STATE(p_ses->op_state, SES_IN_BAD_STATE);
            goto cleanup;
        }
        status = p_ses->lfptrs->start(p_ses);
        if (status)
            SET_STATE(p_ses->op_state, SES_IN_BAD_STATE);

        cleanup:
            CLEAR_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS);
            pthread_cond_signal(&p_ses->ssr_cond);
            pthread_mutex_unlock(&p_ses->lock);
    }
    pthread_mutex_unlock(&g_listen_hal->list_lock);

    if (g_listen_hal->mad_cb_func) {
        ALOGV("%s: send LISTEN_EVENT_STARTED",__func__);
        g_listen_hal->mad_cb_func(LISTEN_EVENT_STARTED, NULL, NULL);
    }
    pthread_mutex_unlock(&g_listen_hal->lock);
    ALOGV("%s: Exit", __func__);
}

/* Called when SSR is started.
 * Disable all currently active ListenDriver sessions.
 */
static void handle_snd_card_offline(void)
{
    struct listnode *p_ses_node;
    audio_listen_session_t *p_ses;

    ALOGV("%s: Enter", __func__);

    pthread_mutex_lock(&g_listen_hal->lock);
    if(g_listen_hal->actual_mad_state == MAD_DISABLED) {
        pthread_mutex_unlock(&g_listen_hal->lock);
        ALOGV("%s: Exit", __func__);
        return;
    }

    g_listen_hal->actual_mad_state = MAD_DISABLED;
    if (g_listen_hal->mad_cb_func) {
        ALOGV("%s: send LISTEN_EVENT_STOPPED",__func__);
        g_listen_hal->mad_cb_func(LISTEN_EVENT_STOPPED, NULL, NULL);
    }

    /* teardown each active session */
    pthread_mutex_lock(&g_listen_hal->list_lock);
    /* list locked while looping thru each session */
    list_for_each(p_ses_node, &g_listen_hal->ses_list) {
        p_ses = node_to_item(p_ses_node, audio_listen_session_t, list_node);
        pthread_mutex_lock(&p_ses->lock);
        SET_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS);
        /* No need to check the states as these calls may return failures
        due to DSP is down and we ignore those failures. These calls
        are required to clean listen kernel driver states */
        /* TODO: Consider defining single ListenDriver API to clean up session;
         *       One call that does stop, deregisterSM, & close */
        if (p_ses->reg_params.enable_buffering)
            p_ses->lfptrs->stop_buffering(p_ses);
        p_ses->lfptrs->stop(p_ses);
        p_ses->lfptrs->dereg_sm(p_ses);
        p_ses->lfptrs->close(p_ses);
        /* Clear other states as the above calls may fail and doesn't
           reset the states */
        p_ses->op_state = SES_SSR_IN_PROGRESS;
        pthread_mutex_unlock(&p_ses->lock);
    }
    pthread_mutex_unlock(&g_listen_hal->list_lock);
    pthread_mutex_unlock(&g_listen_hal->lock);
    ALOGV("%s: Exit", __func__);
}

static int handle_set_parameters(const char *kvpairs)
{
    int status = 0;
    struct str_parms *params;
    struct listnode *node;
    audio_listen_session_t *p_ses;
    char *str;
    char value[32];
    char *snd_card_status;
    int ret;
    ALOGV("%s: Enter kvpairs=%s", __func__, kvpairs);

    params = str_parms_create_str(kvpairs);

    if(!params) {
        ALOGV("%s: Error, str_parms returned NULL", __func__);
        return -ENOMEM;
    }

    ret = str_parms_get_str(params, AUDIO_PARAMETER_KEY_MAD, value,
            sizeof(value));
    if (ret > 0) {
        pthread_mutex_lock(&g_listen_hal->lock);
        if (strcmp(value, AUDIO_PARAMETER_VALUE_MAD_ON) == 0)
            g_listen_hal->reqd_mad_state = MAD_ENABLED;
        else
            g_listen_hal->reqd_mad_state = MAD_DISABLED;

        /*  Rx concurrency allows mad to be enabled, but limits number
            of active sessions, hence handled in open listen session call*/
        if ((g_listen_hal->actual_mad_state == g_listen_hal->reqd_mad_state) ||
             (g_listen_hal->tx_concurrency_active > 0)) {
            ALOGV("%s: Exit, mad state = %d tx_concurrency = %d", __func__,
                g_listen_hal->actual_mad_state, g_listen_hal->tx_concurrency_active);
            pthread_mutex_unlock(&g_listen_hal->lock);
            goto handled_mad_key;
        }

        /* if mad device needs to be enabled, do this before sessions are started */
        if (g_listen_hal->reqd_mad_state) {
            g_listen_hal->actual_mad_state = MAD_ENABLED;
            listen_enable_device(true, g_listen_hal->cur_device);
        }

        /* start or stop all active sessions */
        pthread_mutex_lock(&g_listen_hal->list_lock);
        /* list locked while looping thru each session */
        list_for_each(node, &g_listen_hal->ses_list) {
            p_ses = node_to_item(node, audio_listen_session_t, list_node);
            pthread_mutex_lock(&p_ses->lock);
            ret = set_listen_detection(p_ses, g_listen_hal->reqd_mad_state);
            if (ret)
                ALOGE("%s: ERROR. set_listen_detection failed", __func__);
            pthread_mutex_unlock(&p_ses->lock);
        }
        pthread_mutex_unlock(&g_listen_hal->list_lock);

        /* if mad device needs to be disabled, do this after sessions are stopped */
        if(!g_listen_hal->reqd_mad_state) {
            g_listen_hal->actual_mad_state = MAD_DISABLED;
            listen_enable_device(false, g_listen_hal->cur_device);
        }

        pthread_mutex_unlock(&g_listen_hal->lock);
        goto handled_mad_key;
    }

    ret = str_parms_get_str(params, AUDIO_PARAMETER_KEY_SND_CARD_STATUS, value,
                            sizeof(value));
    if (ret > 0) {
        /* Ignore ADSP SSR event for session running on CPE, we will get CPE
           SSR event in which teardown and bringup the CPE session */
        if(!g_listen_hal->run_on_ape)
            goto handled_mad_key;

        snd_card_status = value + 2;

        if (strcmp(snd_card_status, "OFFLINE") == 0)
            handle_snd_card_offline();
        else if (strcmp(snd_card_status, "ONLINE") == 0)
            handle_snd_card_online();
        else
            ALOGE("%s: ERROR. unknown snd_card_status", __func__);
    }

    ret = str_parms_get_str(params, "CPE_STATUS", value, sizeof(value));
    if (ret > 0) {
        /* Ignore CPE SSR event for session running on ADSP, we will get ADSP
           SSR event in which teardown and bringup the ADSP session */
        if(g_listen_hal->run_on_ape)
            goto handled_mad_key;

        if (strstr(value, "OFFLINE"))
            handle_snd_card_offline();
        else if (strstr(value, "ONLINE"))
            handle_snd_card_online();
        else
            ALOGE("%s: ERROR. unknown cpe status", __func__);
    }

handled_mad_key:
    str_parms_destroy(params);
    ALOGV("%s: Exit",__func__);
    return status;
}

static char* handle_get_parameters(const char *kvpairs)
{
    char *str;
    char value[32] = {0};
    int ret;
    struct str_parms *query;
    struct str_parms *reply;

    ALOGV("%s: Enter, kvpairs - %s", __func__, kvpairs);

    query = str_parms_create_str(kvpairs);
    if(!query) {
        ALOGV("%s: Error, str_parms returned query NULL", __func__);
        return NULL;
    }
    reply = str_parms_create();
    if(!reply) {
        ALOGV("%s: Error, str_parms returned reply NULL", __func__);
        str_parms_destroy(query);
        return NULL;
    }

    ret = str_parms_get_str(query, AUDIO_PARAMETER_KEY_MAD, value,
            sizeof(value));
    if (ret > 0) {
        pthread_mutex_lock(&g_listen_hal->lock);
        if (g_listen_hal->actual_mad_state == MAD_ENABLED)
            strlcpy(value, AUDIO_PARAMETER_VALUE_MAD_ON, sizeof(value));
        else
            strlcpy(value, AUDIO_PARAMETER_VALUE_MAD_OFF, sizeof(value));
        pthread_mutex_unlock(&g_listen_hal->lock);

        str_parms_add_str(reply, AUDIO_PARAMETER_KEY_MAD, value);
    }

    str = str_parms_to_str(reply);
    str_parms_destroy(query);
    str_parms_destroy(reply);

    ALOGV("%s: Exit returns - %s", __func__, str);
    return str;
}

static void handle_notify_event(event_type_t event_type)
{
    struct listnode *p_ses_node;
    audio_listen_session_t *p_ses;
    bool concurrency_active = false;
    int status = 0;
    int cpe_ses = 0, ape_ses = 0;
    ALOGV("%s: Enter, event type = %d", __func__, event_type);

    pthread_mutex_lock(&g_listen_hal->lock);
    platform_listen_update_concurrency(g_listen_hal->platform, event_type);
    ALOGD("%s: tx_active %d, rx_active %d", __func__,
        g_listen_hal->tx_concurrency_active,
        g_listen_hal->rx_concurrency_active);

    if (g_listen_hal->tx_concurrency_active > 0 ||
        g_listen_hal->rx_concurrency_active > 0)
        concurrency_active = true;

    if ((g_listen_hal->reqd_mad_state == MAD_DISABLED) &&
        (g_listen_hal->actual_mad_state == MAD_DISABLED))
        goto exit;

    if ((!concurrency_active) &&
        (g_listen_hal->actual_mad_state == MAD_DISABLED)) {
        g_listen_hal->actual_mad_state = MAD_ENABLED;
        listen_enable_device(true, g_listen_hal->cur_device);

        pthread_mutex_lock(&g_listen_hal->list_lock);
        /* list locked while looping thru and restarting each session */
        list_for_each(p_ses_node, &g_listen_hal->ses_list) {
            p_ses = node_to_item(p_ses_node, audio_listen_session_t, list_node);
            pthread_mutex_lock(&p_ses->lock);
            if (g_listen_hal->sw_mad) {
                status = p_ses->lfptrs->open(p_ses);
                if (!status)
                    status = p_ses->lfptrs->reg_sm(p_ses, &p_ses->reg_params);
                if (!status)
                    status = p_ses->lfptrs->start(p_ses);
                CLEAR_STATE(p_ses->op_state, SES_CONCURRENCY_ACTIVE);
            } else
                status = p_ses->lfptrs->start(p_ses);
            if (status)
                SET_STATE(p_ses->op_state, SES_IN_BAD_STATE);
            pthread_mutex_unlock(&p_ses->lock);
        }
        pthread_mutex_unlock(&g_listen_hal->list_lock);

        if (g_listen_hal->mad_cb_func)
            g_listen_hal->mad_cb_func(LISTEN_EVENT_STARTED, NULL, NULL);
    } else if ((concurrency_active) &&
               (g_listen_hal->actual_mad_state == MAD_ENABLED)){
        get_num_of_active_sessions(&cpe_ses, &ape_ses);
        ALOGI("%s: ape %d, cpe %d", __func__, ape_ses, cpe_ses);
        if ((g_listen_hal->tx_concurrency_active > 0) ||
            (ape_ses > g_listen_hal->rx_conc_max_st_ses ||
             cpe_ses > g_listen_hal->rx_conc_max_st_ses)) {
            pthread_mutex_lock(&g_listen_hal->list_lock);
            /* list locked while looping thru and stop each session */
            list_for_each(p_ses_node, &g_listen_hal->ses_list) {
                p_ses = node_to_item(p_ses_node, audio_listen_session_t, list_node);
                pthread_mutex_lock(&p_ses->lock);
                if (g_listen_hal->sw_mad) {
                    SET_STATE(p_ses->op_state, SES_CONCURRENCY_ACTIVE);
                    if (p_ses->reg_params.enable_buffering)
                        status = p_ses->lfptrs->stop_buffering(p_ses);
                    if (!status)
                        status = p_ses->lfptrs->stop(p_ses);
                    if (!status)
                        status = p_ses->lfptrs->dereg_sm(p_ses);
                    if (!status)
                        status = p_ses->lfptrs->close(p_ses);
                } else {
                    if (p_ses->reg_params.enable_buffering)
                        status = p_ses->lfptrs->stop_buffering(p_ses);
                    if (!status)
                        status = p_ses->lfptrs->stop(p_ses);
                }
                if (status)
                    SET_STATE(p_ses->op_state, SES_IN_BAD_STATE);
                pthread_mutex_unlock(&p_ses->lock);
            }
            pthread_mutex_unlock(&g_listen_hal->list_lock);

            g_listen_hal->actual_mad_state = MAD_DISABLED;
            listen_enable_device(false, g_listen_hal->cur_device);

            if (g_listen_hal->mad_cb_func)
                g_listen_hal->mad_cb_func(LISTEN_EVENT_STOPPED, NULL, NULL);
        }
    }

exit:
    pthread_mutex_unlock(&g_listen_hal->lock);
    ALOGV("%s: Exit", __func__);
}

static void debug_dump_lab_buffer
(
   audio_listen_session_t* p_ses,
   int type,
   unsigned char* buffer,
   int copy_bytes
)
{
    int ret;
    char file_name[100];
    struct stat st = {0};

    /*do "adb shell chmod 777 /data" before dumping data */
    switch(type) {
    case DUMP_INIT:
        ALOGV("%s: DUMP_INIT", __func__);
        if (stat("/data/LHAL", &st) == -1)
            mkdir("/data/LHAL", S_IRWXU | S_IRWXG | S_IRWXO);
        snprintf(file_name, sizeof(file_name), "/data/LHAL/listen_lab_data_%x.bin", (int)p_ses);
        p_ses->dump_fptr = fopen(file_name, "wb");
        if (!p_ses->dump_fptr)
            ALOGE("%s: FILE OPEN ERROR filename=%s errno=%d. Try chmod 777 /data/",
                      __func__, file_name, errno);
        break;
    case DUMP_DEINIT:
        ALOGV("%s: DUMP_DEINIT", __func__);
        if (p_ses->dump_fptr)
            fclose(p_ses->dump_fptr);
        break;
    case DUMP_DATA:
        ALOGV("%s: DUMP_DATA", __func__);
        if (p_ses->dump_fptr) {
            ret = fwrite(buffer, 1, copy_bytes, p_ses->dump_fptr);
            if (ret!=copy_bytes)
                ALOGV("%s: fwrite error: ret=%d, copy_bytes=%d", __func__,ret,copy_bytes);
        }
        break;
    default:
        break;
    }
}

static void process_event(audio_listen_session_t *p_ses,
                          struct snd_lsm_event_status *params)
{
    listen_event_enum_t event_type = LISTEN_ERROR;
    listen_event_data_t payload;
    int i  = 0;

    if (!params) {
        ALOGE("%s: ERROR.[%p] params NULL", __func__, p_ses);
        return;
    }

    ALOGV("%s: param->status %d", __func__, params->status);
    switch (params->status){
    case LSM_VOICE_WAKEUP_STATUS_RUNNING:
        return;
    case LSM_VOICE_WAKEUP_STATUS_DETECTED:
        event_type = LISTEN_EVENT_DETECT_SUCCESS;
        if (p_ses->reg_params.enable_buffering) {
            pthread_mutex_lock(&p_ses->lock);
            SET_STATE(p_ses->op_state, SES_BUFFERING);
            pthread_mutex_unlock(&p_ses->lock);
        }
        break;
    case LSM_VOICE_WAKEUP_STATUS_END_SPEECH:
    case LSM_VOICE_WAKEUP_STATUS_REJECTED:
        event_type = LISTEN_EVENT_DETECT_FAILED;
        break;
    default:
        ALOGV("%s: ERROR. Unknown status %d", __func__, params->status);
        return;
    }
    payload.event_detect.status = params->status;
    payload.event_detect.p_data = (uint8_t *)&params->payload;
    payload.event_detect.size = params->payload_size;

    ALOGV("%s:[%p]-%d Sending event %d to client ", __func__, p_ses, p_ses->pcm_id, event_type);
    p_ses->ses_cb_func(event_type, &payload, p_ses->client_data);
}

static void* event_thread_entry(void *p_data)
{
    audio_listen_session_t *p_ses;
    struct snd_lsm_event_status *params;
    int ret;
    void *ret_ptr = NULL;
    unsigned int payload_alloc_size = 0;
    char name[50];

    ALOGV("%s:[%p] Enter",__func__, p_data);

    if (p_data == NULL) {
        ALOGE("%s: ERROR. session data NULL", __func__);
        return NULL;
    }
    p_ses = p_data;

    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_DEFAULT);
    snprintf(name, sizeof(name), "Listen HAL Session Event Thread %d", p_ses->pcm_id);
    prctl(PR_SET_NAME, (unsigned long)name, 0, 0, 0);

    params = (struct snd_lsm_event_status*)
            malloc(sizeof(*params) + LISTEN_MAX_EVNT_PAYLOAD_SIZE);
    if (params == NULL) {
        ALOGE("%s: ERROR. insufficient memory for payload exiting thread", __func__);
        p_ses->event_thread_alive = false;
        return NULL;
    }
    payload_alloc_size = LISTEN_MAX_EVNT_PAYLOAD_SIZE;

    while (!p_ses->kill_event_thread) {
        params->payload_size = payload_alloc_size;
        ret = pcm_ioctl(p_ses->pcm, SNDRV_LSM_EVENT_STATUS, params);
        if (ret < 0) {
            if (errno == ENOMEM) {
                payload_alloc_size = payload_alloc_size << 1;
                ret_ptr = realloc(params, sizeof(*params) + payload_alloc_size);

                if (ret_ptr == NULL) {
                    ALOGE("%s: ERROR. NOT enough memory for"
                            " payload. exiting thread", __func__);
                    p_ses->kill_event_thread = true;
                    break;
                }
                else {
                    params = (struct snd_lsm_event_status*)ret_ptr;
                    continue;
                }
            } else {
                ALOGE("%s: ERROR. SNDRV_LSM_EVENT_STATUS  failed",__func__);
                p_ses->kill_event_thread = true;
                break;
            }
        }
        if (p_ses->kill_event_thread == true)
            break;

        if(p_ses->ses_cb_func != NULL)
            process_event(p_ses, params);
    }
    if (params)
        free(params);

    p_ses->event_thread_alive = false;
    ALOGV("%s:[%p] Exit", __func__, p_ses);
    return NULL;
}

/* When called the first time, this create and initializes ListenHAL structures/state
 * When call subsequent times, just increments reference count
 */
int create_listen_hw(unsigned int snd_card, struct audio_route *audio_route)
{
    int ret;

    ALOGV("%s: Enter. snd_card = %u. audio_route = %p",
            __func__, snd_card,  audio_route);

    pthread_mutex_lock(&listen_hw_init_lock);
    if (listen_hw_ref_cnt != 0){
        listen_hw_ref_cnt++;
        ALOGV("%s: listen hw already created", __func__);
        ALOGV("%s: Exit", __func__);
        pthread_mutex_unlock(&listen_hw_init_lock);
        return 0;
    }

    if (!audio_route) {
        ALOGE("%s: ERROR. audio_route = %p",
                __func__, audio_route);
        pthread_mutex_unlock(&listen_hw_init_lock);
        return -EINVAL;
    }

    g_listen_hal = calloc(1, sizeof(audio_listen_hardware_t));
    if (g_listen_hal == NULL) {
        ALOGE("%s: ERROR. Failed to create listen hardware", __func__);
        ret = -ENOMEM;
    } else {
        ret = listen_hw_init(snd_card, audio_route);
        if (ret) {
            ALOGE("%s: ERROR. Failed listen hardware init", __func__);
            free(g_listen_hal);
            g_listen_hal = NULL;
        } else
            listen_hw_ref_cnt++;
    }

    pthread_mutex_unlock(&listen_hw_init_lock);
    ALOGV("%s: Exit", __func__);
    return ret;
}

/* When called when there is only one client using ListenHAL structures/state are cleaned up
 * Otherwise, just decrement reference count
 */
void destroy_listen_hw()
{
    ALOGE("%s: Enter hw=%p", __func__, g_listen_hal);

    pthread_mutex_lock(&listen_hw_init_lock);
    if (!g_listen_hal || (--listen_hw_ref_cnt != 0)) {
        pthread_mutex_unlock(&listen_hw_init_lock);
        return;
    }

    if (g_listen_hal->mixer)
        mixer_close(g_listen_hal->mixer);

    platform_listen_deinit(g_listen_hal->platform);

    pthread_mutex_destroy(&g_listen_hal->lock);
    pthread_mutex_destroy(&g_listen_hal->list_lock);

    free(g_listen_hal);
    g_listen_hal = NULL;

    pthread_mutex_unlock(&listen_hw_init_lock);

    ALOGV("%s: Exit", __func__);
}

void get_num_of_active_sessions(int *cpe_ses, int *ape_ses)
{
    struct listnode *p_ses_node;
    audio_listen_session_t *p_ses;

    *cpe_ses = *ape_ses = 0;

    pthread_mutex_lock(&g_listen_hal->list_lock);
    list_for_each(p_ses_node, &g_listen_hal->ses_list) {
        p_ses = node_to_item(p_ses_node, audio_listen_session_t, list_node);
        if(p_ses->exec_mode == LISTEN_EXEC_MODE_CPE)
            (*cpe_ses)++;
        else
            (*ape_ses)++;
    }
    pthread_mutex_unlock(&g_listen_hal->list_lock);
}

/* based on what listen resources are currently executing
 * and requested SoundModel desired to be registered,
 * determine which codec this session should be openned in
 */
static int determine_exec_mode(audio_listen_session_t *p_ses,
                               listen_sound_model_params_t *params,
                               listen_exec_mode_t *p_exec_mode)
{
    int status = 0;
    int cpe_ses = 0, ape_ses = 0;

    ALOGV("%s: Enter - KW=%d UP=%d AKW=%d AKWUP=%d CKW=%d CKWUP=%d lab=%d", __func__,
          params->num_keywords, params->num_user_pairings,
          g_listen_hal->avail_ape_kw, g_listen_hal->avail_ape_kw_user_pairs,
          g_listen_hal->avail_cpe_kw, g_listen_hal->avail_cpe_kw_user_pairs,
          params->enable_buffering);

    *p_exec_mode = LISTEN_EXEC_MODE_INVALID;
    get_num_of_active_sessions(&cpe_ses, &ape_ses);
    /* Run on CPE only when all the conditions are satisfied
        1. If CPE service is present
        2. If the available sessions doesn't exceed max supported
        3. If number of KW+UP doesn't exceed the max supported
        4. If the app id is of SVA2.0
   */
    if ((g_listen_hal->run_on_ape == false) &&
        CHECK_BIT(g_listen_hal->hw_type, LISTEN_HW_CPE) &&
        (cpe_ses < g_listen_hal->max_cpe_sessions) &&
        (p_ses->app_type == LISTEN_APP_TYPE_SVA_V2) &&
        (params->num_keywords <= g_listen_hal->avail_cpe_kw) &&
        (params->num_user_pairings <= g_listen_hal->avail_cpe_kw_user_pairs)) {

        *p_exec_mode = LISTEN_EXEC_MODE_CPE;
    }
    else if ((p_ses->app_type == LISTEN_APP_TYPE_SVA) ||
             ((g_listen_hal->run_on_ape == true) &&
              (ape_ses < g_listen_hal->max_ape_sessions) &&
              (params->num_keywords <= g_listen_hal->avail_ape_kw) &&
              (params->num_user_pairings <=
               g_listen_hal->avail_ape_kw_user_pairs))) {
        g_listen_hal->run_on_ape = true;
        *p_exec_mode = LISTEN_EXEC_MODE_APE;
    }
    else {
        status = -EINVAL;
    }

    ALOGV("%s: Exit, exec_mode=%d", __func__, *p_exec_mode);
    return status;
}

static void destroy_event_thread(audio_listen_session_t *p_ses)
{
    int ret;

    ALOGV("%s:[%p] Enter", __func__, p_ses);

    if (!p_ses->event_thread_alive){
        ALOGV("%s:[%p] -Exit", __func__, p_ses);
        return;
    }

    p_ses->kill_event_thread = true;

    if ( pcm_ioctl(p_ses->pcm, SNDRV_LSM_ABORT_EVENT) < 0 )
        ALOGE("%s: ERROR. ABORT_EVENT failed", __func__);

    ret = pthread_join(p_ses->event_thread, NULL);
    if (ret)
        ALOGE("%s: ERROR. pthread_join returned %d", __func__, ret);

    ALOGV("%s:[%p] Exit", __func__, p_ses);
}

static void create_event_thread(audio_listen_session_t *p_ses)
{
    pthread_attr_t attr;
    ALOGV("%s:[%p] Enter", __func__, p_ses);

    p_ses->kill_event_thread = false;
    p_ses->event_thread_alive = true;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&p_ses->event_thread, &attr, event_thread_entry, p_ses);

    ALOGV("%s:[%p] Exit", __func__, p_ses);
}

static int cpe_get_pcm_device_id(audio_listen_session_t *p_ses)
{
    int i, ret = -1;

    if (!CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS|SES_CONCURRENCY_ACTIVE))
        pthread_mutex_lock(&g_listen_hal->lock);

    for (i = 0; i < g_listen_hal->max_cpe_sessions; i++) {
        if(!g_listen_hal->cpe_pcm_use_cases[i].active) {
            g_listen_hal->cpe_pcm_use_cases[i].active = true;
            ret = g_listen_hal->cpe_pcm_use_cases[i].pcm_id;
            p_ses->use_case_idx = i;
            break;
        }
    }
    if (ret < 0) {
        ALOGE("%s: ERROR. no free pcm device available", __func__);
    }
    if (!CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS|SES_CONCURRENCY_ACTIVE))
        pthread_mutex_unlock(&g_listen_hal->lock);

    return ret;
}

static void cpe_free_pcm_device_id(audio_listen_session_t *p_ses)
{
    int i;

    if (!CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS|SES_CONCURRENCY_ACTIVE))
        pthread_mutex_lock(&g_listen_hal->lock);

    for (i = 0; i < g_listen_hal->max_cpe_sessions; i++) {
        if(g_listen_hal->cpe_pcm_use_cases[i].pcm_id == p_ses->pcm_id) {
            g_listen_hal->cpe_pcm_use_cases[i].active = false;
            break;
        }
    }
    if (!CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS|SES_CONCURRENCY_ACTIVE))
        pthread_mutex_unlock(&g_listen_hal->lock);
}

static int cpe_close_device(audio_listen_session_t *p_ses)
{
    ALOGV("%s:[%p] Enter", __func__, p_ses);
    if (p_ses->pcm) {
        ALOGV("%s: closing pcm %p", __func__, p_ses->pcm);
        pcm_close(p_ses->pcm);
        p_ses->pcm = NULL;
    } else
        ALOGV("%s: pcm %p already closed", __func__, p_ses->pcm);

    ALOGV("%s:[%p] Exit", __func__, p_ses);
    return 0;
}

static int cpe_open_device(audio_listen_session_t *p_ses)
{
    int status = 0;
    int retry_num = 0;
    struct snd_lsm_session_data ses_data;

    ALOGV("%s:[%p] Enter. pcm device = %u",__func__, p_ses, p_ses->pcm_id);

    p_ses->pcm = pcm_open(g_listen_hal->snd_card, p_ses->pcm_id,
                          PCM_IN, &p_ses->config);
    if (!p_ses->pcm) {
        ALOGE("%s: ERROR. pcm_open failed", __func__);
        status = -ENODEV;
        goto error;
    }
    if (!pcm_is_ready(p_ses->pcm)) {
        ALOGE("%s: ERROR. pcm_is_ready failed err=%s", __func__,
              pcm_get_error(p_ses->pcm));
        status = -ENODEV;
        goto error;
    }

    status = pcm_start(p_ses->pcm);
    while (status && (retry_num < RETRY_NUMBER)) {
        usleep(RETRY_US);
        retry_num++;
        ALOGV("%s: pcm_start retrying..status %d errno %d, retry cnt %d",
              __func__, status, errno, retry_num);
        status = pcm_start(p_ses->pcm);
    }
    if (status) {
        ALOGE("%s: ERROR. pcm_start failed, returned status %d",__func__, status);
        goto error;
    }

    ALOGV("%s:[%p] Exit", __func__, p_ses);
    return status;

error:
    cpe_close_device(p_ses);
    return status;
}

static int cpe_open_session(audio_listen_session_t *p_ses)
{
    int status = 0;

    p_ses->pcm_id = cpe_get_pcm_device_id(p_ses);
    if(p_ses->pcm_id < 0)
        return -ENODEV;

    status = platform_listen_send_calibration(g_listen_hal->platform,
                                              g_listen_hal->cur_device,
                                              p_ses->exec_mode,
                                              LSM_VOICE_WAKEUP_APP_ID);
    if (status) {
        cpe_free_pcm_device_id(p_ses);
        return status;
    }
    p_ses->config = cpe_pcm_config_listen;
    status = cpe_open_device(p_ses);
    if (status) {
        ALOGE("%s: Device open failed", __func__);
        cpe_free_pcm_device_id(p_ses);
    }

    return status;
}

static int cpe_close_session(audio_listen_session_t *p_ses)
{
    int status = 0;
    if (!p_ses->pcm) {
        ALOGV("%s:[%p] pcm NULL", __func__, p_ses);
        return status;
    }

    cpe_close_device(p_ses) ;
    cpe_free_pcm_device_id(p_ses);
    return status;
}

static int cpe_start(audio_listen_session_t* p_ses)
{
    int status = 0;

    ALOGV("%s:[%p] Enter", __func__, p_ses);
    if (!p_ses->pcm) {
        ALOGV("%s:[%p] pcm NULL", __func__, p_ses);
        return status;
    }
    if (pcm_ioctl(p_ses->pcm, SNDRV_LSM_START)) {
        ALOGE("%s: ERROR. SNDRV_LSM_START failed", __func__);
        status = -errno;
    }
    ALOGV("%s:[%p] Exit, status=%d", __func__, p_ses, status);
    return status;
}

static int cpe_stop(audio_listen_session_t* p_ses)
{
    int status = 0;

    ALOGV("%s:[%p] Enter", __func__, p_ses);
    if (!p_ses->pcm) {
        ALOGV("%s:[%p] pcm NULL", __func__, p_ses);
        return status;
    }
    if (pcm_ioctl(p_ses->pcm, SNDRV_LSM_STOP) < 0) {
        ALOGE("%s: ERROR. SNDDRV_LSM_STOP failed", __func__);
        status = -errno;
    }
    ALOGV("%s:[%p] Exit, status=%d", __func__, p_ses, status);
    return status;
}

static int cpe_read_buffer(audio_listen_session_t* p_ses,
                           uint8_t *p_buf,
                           uint32_t *p_bytes)
{
    int status = 0;
    int read_bytes = 0, total_bytes = 0;

    ALOGV("%s:[%p] Enter *p_bytes %d", __func__, p_ses, *p_bytes);

    read_bytes = pcm_frames_to_bytes(p_ses->pcm, p_ses->config.period_size);
    total_bytes = *p_bytes;

    while(total_bytes > 0) {
        read_bytes = (total_bytes < read_bytes)?total_bytes:read_bytes;
        /* call pcm_read with not more than period size */
        status = pcm_read(p_ses->pcm, p_buf, read_bytes);

        pthread_mutex_lock(&p_ses->lock);
        /* in case of SSR and concurrency the buffering can be stopped */
        if (!CHECK_STATE(p_ses->op_state, SES_BUFFERING)) {
            ALOGE("%s: SSR stopped lab op_state=0x%x status=%d",
                  __func__, p_ses->op_state, status);
            /* pcm_read would have filled the complete buffer */
            if (!status)
                total_bytes -= read_bytes;
            status = -EBUSY;
        }
        pthread_mutex_unlock(&p_ses->lock);

        if (status) {
            ALOGE("%s:[%p] pcm_read error %d total_bytes %d read_bytes %d",
                            __func__, p_ses, status, total_bytes, read_bytes);
            break;
        }
        p_buf += read_bytes;
        total_bytes -= read_bytes;
    }
    *p_bytes -= total_bytes;

    if(g_listen_hal->debug_dump_lab_data)
        debug_dump_lab_buffer(p_ses, DUMP_DATA, p_buf, *p_bytes);

    ALOGV("%s:[%p] Exit, status=%d, *p_bytes=%d",
                __func__, p_ses, status, *p_bytes);
    return status;
}

static int cpe_stop_buffering(audio_listen_session_t* p_ses)
{
    int status = 0;

    ALOGV("%s:[%p] Enter", __func__, p_ses);
    if (!p_ses->pcm) {
        ALOGV("%s:[%p] pcm NULL", __func__, p_ses);
        return status;
    }
    if (pcm_ioctl(p_ses->pcm, SNDRV_LSM_STOP_LAB) < 0) {
        ALOGE("%s: ERROR. SNDRV_LSM_STOP_BUFFERING failed", __func__);
        return -errno;
    }
    if (pcm_ioctl(p_ses->pcm, SNDRV_PCM_IOCTL_RESET) < 0) {
        ALOGE("%s: ERROR. SNDRV_PCM_IOCTL_RESET failed", __func__);
        status = -errno;
    }
    ALOGV("%s:[%p] Exit, status=%d", __func__, p_ses, status);
    return status;
}

static int cpe_reg_sm_v2(audio_listen_session_t *p_ses,
                          listen_sound_model_params_t *p_reg_params)
{
    int status = 0;
    unsigned int buf_en;
    struct snd_lsm_sound_model_v2   lsm_params_v2;
    ALOGV("%s:[%p] Enter pcm=%p", __func__, p_ses, p_ses->pcm);

    lsm_params_v2.data = p_reg_params->sound_model_data->p_data;
    lsm_params_v2.data_size = p_reg_params->sound_model_data->size;
    lsm_params_v2.num_confidence_levels =
         p_reg_params->num_keywords + p_reg_params->num_user_pairings;
    lsm_params_v2.confidence_level = p_reg_params->min_conf_levels;
    lsm_params_v2.detect_failure = p_reg_params->detect_failure;

    if(p_reg_params->detection_mode == LISTEN_MODE_KEYWORD_ONLY_DETECTION)
        lsm_params_v2.detection_mode = LSM_MODE_KEYWORD_ONLY_DETECTION;
    else if (p_reg_params->detection_mode == LISTEN_MODE_USER_KEYWORD_DETECTION)
        lsm_params_v2.detection_mode = LSM_MODE_USER_KEYWORD_DETECTION;
    else {
        ALOGV("%s: Unknown detection mode..Setting keyword only", __func__);
        lsm_params_v2.detection_mode = LSM_MODE_KEYWORD_ONLY_DETECTION;
    }

    /* Store the soundmodel params to send again when SSR happens */
    if(!p_ses->reg_params.sound_model_data) {
        memcpy(&p_ses->reg_params, p_reg_params, sizeof(*p_reg_params));
        p_ses->reg_params.sound_model_data = (listen_sound_model_data_t*)malloc(sizeof(listen_sound_model_data_t));
        if (!p_ses->reg_params.sound_model_data) {
            ALOGE("%s: ERROR. lsm_params_v2 alloc failed, size=%d", __func__,
                       p_reg_params->sound_model_data->size);
            status = -ENOMEM;
            goto error;
        }

        p_ses->reg_params.sound_model_data->p_data = (uint8_t *)malloc(p_reg_params->sound_model_data->size * sizeof(uint8_t));
        if (!p_ses->reg_params.sound_model_data->p_data) {
            ALOGE("%s: lsm_params_v2 data alloc failed, size=%d", __func__,
                       p_reg_params->sound_model_data->size);
            status = -ENOMEM;
            goto error;
        }

        p_ses->reg_params.sound_model_data->size = p_reg_params->sound_model_data->size;
        memcpy(p_ses->reg_params.sound_model_data->p_data, p_reg_params->sound_model_data->p_data, p_reg_params->sound_model_data->size);
    }
    ALOGV("%s: data=%p, dataSize=%d, num_confidence_levels=%d, "
               "confidence_levels ptr=%p, detect_failure=%d, detection_mode=%d",
                __func__, lsm_params_v2.data, lsm_params_v2.data_size,
                lsm_params_v2.num_confidence_levels, lsm_params_v2.confidence_level,
                lsm_params_v2.detect_failure, lsm_params_v2.detection_mode);

    if(pcm_ioctl(p_ses->pcm, SNDRV_LSM_REG_SND_MODEL_V2, &lsm_params_v2) < 0)
    {
        ALOGE("%s: ERROR. SNDRV_LSM_REG_SND_MODEL_V2 failed", __func__);
        status = -errno;
        goto error;
    }
    buf_en = p_reg_params->enable_buffering ? 1:0;
    if(pcm_ioctl(p_ses->pcm, SNDRV_LSM_LAB_CONTROL, &buf_en) < 0)
    {
        ALOGE("%s: ERROR. SNDRV_LSM_LAB_CONTROL failed", __func__);
        status = -errno;
        goto error;
    }


    ALOGV("%s:[%p] Exit status=%d", __func__, p_ses, status);
    return 0;

error:
    if(p_ses->reg_params.sound_model_data) {
        if(p_ses->reg_params.sound_model_data->p_data) {
            free(p_ses->reg_params.sound_model_data->p_data);
            p_ses->reg_params.sound_model_data->p_data = NULL;
        }
        free(p_ses->reg_params.sound_model_data);
        p_ses->reg_params.sound_model_data = NULL;
    }
    ALOGV("%s:[%p] Exit status=%d", __func__, p_ses, status);
    return status;
}

static int cpe_reg_sm(audio_listen_session_t *p_ses,
                       listen_sound_model_params_t *p_params)
{
    int status = 0;
    listen_app_type_enum_t          app_type;

    ALOGV("%s:[%p] Enter", __func__, p_ses);
    if (!p_ses->pcm) {
        ALOGV("%s:[%p] pcm NULL", __func__, p_ses);
        return status;
    }
    if (!p_params || !p_params->sound_model_data ||
        !p_params->sound_model_data->p_data) {
        ALOGE("%s: params NULL", __func__);
        status = -EINVAL;
        goto error;
    }

    status = cpe_reg_sm_v2(p_ses, p_params);

    if (status != 0) {
        goto error;
    }
    ALOGV("%s:[%p] Exit", __func__, p_ses);
    return 0;  // %%% Nothing to clean up??

error:
    ALOGV("%s:[%p] Exit status=%d", __func__, p_ses, status);
    return status;
}

static int cpe_dereg_sm(audio_listen_session_t *p_ses)
{
    ALOGV("%s:[%p] Enter", __func__, p_ses);
    listen_sound_model_data_t* p_sm_data = NULL;
    if (!p_ses->pcm) {
        ALOGV("%s:[%p] pcm NULL", __func__, p_ses);
        return 0;
    }
    if (pcm_ioctl(p_ses->pcm, SNDRV_LSM_DEREG_SND_MODEL) < 0) {
        ALOGE("%s: ERROR. SNDRV_LSM_DEREG_SND_MODEL failed", __func__);
        return -errno;
    }
    if (!CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS|SES_CONCURRENCY_ACTIVE)) {
        if(p_ses->reg_params.sound_model_data) {
            if(p_ses->reg_params.sound_model_data->p_data) {
                free(p_ses->reg_params.sound_model_data->p_data);
                p_ses->reg_params.sound_model_data->p_data = NULL;
            }
            free(p_ses->reg_params.sound_model_data);
            p_ses->reg_params.sound_model_data = NULL;
        }
    }

    ALOGV("%s:[%p] Exit", __func__, p_ses);
    return 0;
}

static int ape_get_pcm_device_id(audio_listen_session_t *p_ses)
{
    int i, ret = -1;

    if (!CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS|SES_CONCURRENCY_ACTIVE))
        pthread_mutex_lock(&g_listen_hal->lock);

    for (i = 0; i < g_listen_hal->max_ape_sessions; i++) {
        if(!g_listen_hal->ape_pcm_use_cases[i].active) {
            g_listen_hal->ape_pcm_use_cases[i].active = true;
            ret = g_listen_hal->ape_pcm_use_cases[i].pcm_id;
            p_ses->use_case_idx = i;
            break;
        }
    }
    if (ret < 0) {
        ALOGE("%s: ERROR. no free pcm device available", __func__);
    }
    ALOGV("%s: return pcm_id=%d", __func__, ret);
    if (!CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS|SES_CONCURRENCY_ACTIVE))
        pthread_mutex_unlock(&g_listen_hal->lock);

    return ret;
}

static void ape_free_pcm_device_id(audio_listen_session_t *p_ses)
{
    int i;

    if (!CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS|SES_CONCURRENCY_ACTIVE))
        pthread_mutex_lock(&g_listen_hal->lock);

    for (i = 0; i < g_listen_hal->max_ape_sessions; i++) {
        if(g_listen_hal->ape_pcm_use_cases[i].pcm_id == p_ses->pcm_id) {
            g_listen_hal->ape_pcm_use_cases[i].active = false;
            break;
        }
    }
    if (!CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS|SES_CONCURRENCY_ACTIVE))
        pthread_mutex_unlock(&g_listen_hal->lock);
}

static void ape_listen_enable_use_case(bool enable, audio_listen_session_t *p_ses)
{
    if (enable) {
        ALOGV("%s: enable use case = %s, app_type = %u", __func__,
            g_listen_hal->ape_pcm_use_cases[p_ses->use_case_idx].use_case,
            p_ses->app_type);

        audio_route_apply_path(g_listen_hal->audio_route,
            g_listen_hal->ape_pcm_use_cases[p_ses->use_case_idx].use_case);
        audio_route_update_mixer(g_listen_hal->audio_route);

    } else {
        ALOGV("%s: disable use case = %s", __func__,
              g_listen_hal->ape_pcm_use_cases[p_ses->use_case_idx].use_case);

        audio_route_reset_path(g_listen_hal->audio_route,
            g_listen_hal->ape_pcm_use_cases[p_ses->use_case_idx].use_case);
        audio_route_update_mixer(g_listen_hal->audio_route);
    }
}

static int ape_close_device(audio_listen_session_t *p_ses)
{
    ALOGV("%s:[%p] Enter", __func__, p_ses);
    if (p_ses->pcm) {
        ALOGV("%s: closing pcm %p", __func__, p_ses->pcm);
        pcm_close(p_ses->pcm);
        p_ses->pcm = NULL;
    } else
        ALOGV("%s: pcm %p already closed", __func__, p_ses->pcm);

    ALOGV("%s:[%p] Exit", __func__, p_ses);
    return 0;
}

static int ape_open_device(audio_listen_session_t *p_ses)
{
    int status = 0;
    int retry_num = 0;
    struct snd_lsm_session_data ses_data;

    ALOGV("%s:[%p] Enter. pcm device = %u",__func__, p_ses, p_ses->pcm_id);

    p_ses->pcm = pcm_open(g_listen_hal->snd_card, p_ses->pcm_id,
                          PCM_IN, &p_ses->config);
    if (!p_ses->pcm) {
        ALOGE("%s: ERROR. pcm_open failed", __func__);
        status = -ENODEV;
        goto error;
    }
    if (!pcm_is_ready(p_ses->pcm)) {
        ALOGE("%s: ERROR. pcm_is_ready failed err=%s", __func__,
              pcm_get_error(p_ses->pcm));
        status = -ENODEV;
        goto error;
    }

    ses_data.app_id = (p_ses->app_type == LISTEN_APP_TYPE_SVA)?
                    LSM_VOICE_WAKEUP_APP_ID:LSM_VOICE_WAKEUP_APP_ID_V2;

    if(pcm_ioctl(p_ses->pcm, SNDRV_LSM_SET_SESSION_DATA, &ses_data) < 0)
    {
        ALOGE("%s: ERROR. SNDRV_LSM_SET_SESSION_DATA failed", __func__);
        status = -ENODEV;
        goto error;
    }

    status = pcm_start(p_ses->pcm);
    while (status && (retry_num < RETRY_NUMBER)) {
        usleep(RETRY_US);
        retry_num++;
        ALOGV("%s: pcm_start retrying..status %d errno %d, retry cnt %d",
              __func__, status, errno, retry_num);
        status = pcm_start(p_ses->pcm);
    }
    if (status) {
        ALOGE("%s: ERROR. pcm_start failed, returned status %d",__func__, status);
        goto error;
    }

    ALOGV("%s:[%p] Exit", __func__, p_ses);
    return status;

error:
    ape_close_device(p_ses);
    return status;
}

static int ape_open_session(audio_listen_session_t *p_ses)
{
    int status = 0;

    p_ses->pcm_id = ape_get_pcm_device_id(p_ses);
    if(p_ses->pcm_id < 0)
        return -ENODEV;

    status = platform_listen_send_calibration(g_listen_hal->platform,
                                              g_listen_hal->cur_device,
                                              p_ses->exec_mode,
                                              LSM_VOICE_WAKEUP_APP_ID);
    if (status) {
        ape_free_pcm_device_id(p_ses);
        return  -ENODEV;
    }

    ape_listen_enable_use_case(true, p_ses);

    p_ses->config = ape_pcm_config_listen;
    status = ape_open_device(p_ses);
    if (status) {
        ALOGE("%s: Device open failed", __func__);
        ape_listen_enable_use_case(false, p_ses);
        ape_free_pcm_device_id(p_ses);
    }

    return status;
}

static int ape_close_session(audio_listen_session_t *p_ses)
{
    int status = 0;

    if (!p_ses->pcm) {
        ALOGV("%s:[%p] pcm NULL", __func__, p_ses);
        return status;
    }

    ape_close_device(p_ses) ;
    ape_listen_enable_use_case(false, p_ses);
    ape_free_pcm_device_id(p_ses);

    return status;
}

static int ape_start(audio_listen_session_t* p_ses)
{
    int status = 0;

    ALOGV("%s:[%p] Enter", __func__, p_ses);
    if (!p_ses->pcm) {
        ALOGV("%s:[%p] pcm NULL", __func__, p_ses);
        return status;
    }
    if (pcm_ioctl(p_ses->pcm, SNDRV_LSM_START)) {
        ALOGE("%s: ERROR. SNDRV_LSM_START failed", __func__);
        status = -errno;
    }
    ALOGV("%s:[%p] Exit, status=%d", __func__, p_ses, status);
    return status;
}

static int ape_stop(audio_listen_session_t* p_ses)
{
    int status = 0;

    ALOGV("%s:[%p] Enter", __func__, p_ses);
    if (!p_ses->pcm) {
        ALOGV("%s:[%p] pcm NULL", __func__, p_ses);
        return status;
    }
    if (pcm_ioctl(p_ses->pcm, SNDRV_LSM_STOP) < 0) {
        ALOGE("%s: ERROR. SNDDRV_LSM_STOP failed", __func__);
        status = -errno;
    }
    ALOGV("%s:[%p] Exit, status=%d", __func__, p_ses, status);
    return status;
}

static int ape_reg_sm_v1(audio_listen_session_t *p_ses,
                          listen_sound_model_params_t *p_reg_params)
{
    int status = 0;
    struct snd_lsm_sound_model      lsm_params;
    int num_conf_levels = 0;
    ALOGV("%s:[%p] Enter pcm=%p", __func__, p_ses, p_ses->pcm);

    num_conf_levels = p_reg_params->num_keywords + p_reg_params->num_user_pairings;
    if (num_conf_levels != 2) {
        ALOGE("%s: params (V1) 2 confidence levels expected to be set", __func__);
        status = -EINVAL;
        goto error;
    }
    lsm_params.data = p_reg_params->sound_model_data->p_data;
    lsm_params.data_size = p_reg_params->sound_model_data->size;
    lsm_params.min_keyw_confidence = p_reg_params->min_conf_levels[0];
    lsm_params.min_user_confidence = p_reg_params->min_conf_levels[1];
    lsm_params.detect_failure = p_reg_params->detect_failure;

    if(p_reg_params->detection_mode == LISTEN_MODE_KEYWORD_ONLY_DETECTION)
        lsm_params.detection_mode = LSM_MODE_KEYWORD_ONLY_DETECTION;
    else if (p_reg_params->detection_mode == LISTEN_MODE_USER_KEYWORD_DETECTION)
        lsm_params.detection_mode = LSM_MODE_USER_KEYWORD_DETECTION;
    else {
        ALOGV("%s: Unknown detection mode..Setting keyword only", __func__);
        lsm_params.detection_mode = LSM_MODE_KEYWORD_ONLY_DETECTION;
    }

    /* Store the soundmodel params to send again when SSR happens */
    if(!p_ses->reg_params.sound_model_data) {
        memcpy(&p_ses->reg_params, p_reg_params, sizeof(listen_sound_model_params_t));
        p_ses->reg_params.sound_model_data = (listen_sound_model_data_t*)malloc(sizeof(listen_sound_model_data_t));
        if (!p_ses->reg_params.sound_model_data) {
            ALOGE("%s: ERROR. lsm_params alloc failed, size=%d", __func__,
                       p_reg_params->sound_model_data->size);
            status = -ENOMEM;
            goto error;
        }

        p_ses->reg_params.sound_model_data->p_data = (uint8_t *)malloc(p_reg_params->sound_model_data->size * sizeof(uint8_t));
        if (!p_ses->reg_params.sound_model_data->p_data) {
        ALOGE("%s: lsm_params data alloc failed, size=%d", __func__,
                       p_reg_params->sound_model_data->size);
        status = -ENOMEM;
        goto error;
        }
        p_ses->reg_params.sound_model_data->size = p_reg_params->sound_model_data->size;
        memcpy(p_ses->reg_params.sound_model_data->p_data, p_reg_params->sound_model_data->p_data, p_reg_params->sound_model_data->size);
        p_ses->reg_params.enable_buffering = false;
    }

    ALOGV("%s: data=%p, data_size=%d, min_keyw_confidence=%d, "
                "min_user_confidence=%d, detect_failure=%d, detection_mode=%d",
                __func__, lsm_params.data, lsm_params.data_size,
                lsm_params.min_keyw_confidence, lsm_params.min_user_confidence,
                lsm_params.detect_failure, lsm_params.detection_mode);

    if(pcm_ioctl(p_ses->pcm, SNDRV_LSM_REG_SND_MODEL, &lsm_params) < 0)
    {
        ALOGE("%s: ERROR. SNDRV_LSM_REG_SND_MODEL failed", __func__);
        status = -errno;
        goto error;
    }

error:
    if(p_ses->reg_params.sound_model_data) {
        if(p_ses->reg_params.sound_model_data->p_data) {
            free(p_ses->reg_params.sound_model_data->p_data);
            p_ses->reg_params.sound_model_data->p_data = NULL;
        }
          free(p_ses->reg_params.sound_model_data);
          p_ses->reg_params.sound_model_data = NULL;
    }
    ALOGV("%s:[%p] Exit status=%d", __func__, p_ses, status);
    return status;
}


static int ape_reg_sm_v2(audio_listen_session_t *p_ses,
                          listen_sound_model_params_t *p_reg_params)
{
    int status = 0;
    unsigned int buf_en;
    struct snd_lsm_sound_model_v2   lsm_params_v2;
    ALOGV("%s:[%p] Enter pcm=%p", __func__, p_ses, p_ses->pcm);

    lsm_params_v2.data = p_reg_params->sound_model_data->p_data;
    lsm_params_v2.data_size = p_reg_params->sound_model_data->size;
    lsm_params_v2.num_confidence_levels =
          p_reg_params->num_keywords + p_reg_params->num_user_pairings;
    lsm_params_v2.confidence_level = p_reg_params->min_conf_levels;
    lsm_params_v2.detect_failure = p_reg_params->detect_failure;

    if(p_reg_params->detection_mode == LISTEN_MODE_KEYWORD_ONLY_DETECTION)
        lsm_params_v2.detection_mode = LSM_MODE_KEYWORD_ONLY_DETECTION;
    else if (p_reg_params->detection_mode == LISTEN_MODE_USER_KEYWORD_DETECTION)
        lsm_params_v2.detection_mode = LSM_MODE_USER_KEYWORD_DETECTION;
    else {
        ALOGV("%s: Unknown detection mode..Setting keyword only", __func__);
        lsm_params_v2.detection_mode = LSM_MODE_KEYWORD_ONLY_DETECTION;
    }

    /* Store the soundmodel params to send again when SSR happens */
    if(!p_ses->reg_params.sound_model_data) {
        memcpy(&p_ses->reg_params, p_reg_params, sizeof(*p_reg_params));
        p_ses->reg_params.sound_model_data = (listen_sound_model_data_t*)malloc(sizeof(listen_sound_model_data_t));
        if (!p_ses->reg_params.sound_model_data) {
            ALOGE("%s: ERROR. lsm_params_v2 alloc failed, size=%d", __func__,
                       p_reg_params->sound_model_data->size);
            status = -ENOMEM;
            goto error;
        }

        p_ses->reg_params.sound_model_data->p_data = (uint8_t *)malloc(p_reg_params->sound_model_data->size * sizeof(uint8_t));
        if (!p_ses->reg_params.sound_model_data->p_data) {
            ALOGE("%s: lsm_params_v2 data alloc failed, size=%d", __func__,
                        p_reg_params->sound_model_data->size);
            status = -ENOMEM;
            goto error;
        }

        p_ses->reg_params.sound_model_data->size = p_reg_params->sound_model_data->size;
        memcpy(p_ses->reg_params.sound_model_data->p_data, p_reg_params->sound_model_data->p_data, p_reg_params->sound_model_data->size);
    }
    ALOGV("%s: data=%p, dataSize=%d, num_confidence_levels=%d, "
        "confidence_levels ptr=%p, detect_failure=%d, detection_mode=%d",
        __func__, lsm_params_v2.data, lsm_params_v2.data_size,
        lsm_params_v2.num_confidence_levels, lsm_params_v2.confidence_level,
        lsm_params_v2.detect_failure, lsm_params_v2.detection_mode);

    if(pcm_ioctl(p_ses->pcm, SNDRV_LSM_REG_SND_MODEL_V2, &lsm_params_v2) < 0)
    {
        ALOGE("%s: ERROR. SNDRV_LSM_REG_SND_MODEL_V2 failed", __func__);
        status = -errno;
        goto error;
    }
    buf_en = p_reg_params->enable_buffering ? 1:0;
    if(pcm_ioctl(p_ses->pcm, SNDRV_LSM_LAB_CONTROL, &buf_en) < 0)
    {
        ALOGE("%s: ERROR. SNDRV_LSM_LAB_CONTROL failed", __func__);
        status = -errno;
        goto error;
    }
    ALOGV("%s:[%p] Exit status=%d", __func__, p_ses, status);
    return 0;

error:
    if(p_ses->reg_params.sound_model_data) {
        if(p_ses->reg_params.sound_model_data->p_data) {
            free(p_ses->reg_params.sound_model_data->p_data);
            p_ses->reg_params.sound_model_data->p_data = NULL;
        }
        free(p_ses->reg_params.sound_model_data);
        p_ses->reg_params.sound_model_data = NULL;
    }
    ALOGV("%s:[%p] Exit status=%d", __func__, p_ses, status);
    return status;
}

static int ape_reg_sm(audio_listen_session_t *p_ses,
                       listen_sound_model_params_t *p_params)
{
    int status = 0;
    listen_app_type_enum_t app_type;

    ALOGV("%s:[%p] Enter", __func__, p_ses);
    if (!p_ses->pcm) {
        ALOGV("%s:[%p] pcm NULL", __func__, p_ses);
        return status;
    }
    if (!p_params || !p_params->sound_model_data ||
        !p_params->sound_model_data->p_data) {
        ALOGE("%s: params NULL", __func__);
        status = -EINVAL;
        goto error;
    }

    app_type = (listen_app_type_enum_t)p_ses->app_type;
    if ( app_type == LISTEN_APP_TYPE_SVA )
        status = ape_reg_sm_v1(p_ses, p_params);
    else
        status = ape_reg_sm_v2(p_ses, p_params);

    if (status != 0) {
        goto error;
    }
    ALOGV("%s:[%p] Exit", __func__, p_ses);
    return 0;

error:
    ALOGV("%s:[%p] Exit status=%d", __func__, p_ses, status);
    return status;
}

static int ape_dereg_sm(audio_listen_session_t *p_ses)
{
    ALOGV("%s:[%p] Enter", __func__, p_ses);
    if (!p_ses->pcm) {
        ALOGV("%s:[%p] pcm NULL", __func__, p_ses);
        return 0;
    }
    if (pcm_ioctl(p_ses->pcm, SNDRV_LSM_DEREG_SND_MODEL) < 0) {
        ALOGE("%s: ERROR. SNDRV_LSM_DEREG_SND_MODEL failed", __func__);
        return -errno;
    }
    if (!CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS|SES_CONCURRENCY_ACTIVE)) {
        if(p_ses->reg_params.sound_model_data) {
            if(p_ses->reg_params.sound_model_data->p_data) {
                free(p_ses->reg_params.sound_model_data->p_data);
                p_ses->reg_params.sound_model_data->p_data = NULL;
            }
            free(p_ses->reg_params.sound_model_data);
            p_ses->reg_params.sound_model_data = NULL;
        }
    }
    ALOGV("%s:[%p] Exit", __func__, p_ses);
    return 0;
}

static int ape_read_buffer(audio_listen_session_t* p_ses,
                           uint8_t *p_buf,
                           uint32_t *p_bytes)
{
    int status = 0;
    int read_bytes = 0, total_bytes = 0;

    ALOGV("%s:[%p] Enter *p_bytes %d", __func__, p_ses, *p_bytes);

    read_bytes = pcm_frames_to_bytes(p_ses->pcm, p_ses->config.period_size);
    total_bytes = *p_bytes;

    while(total_bytes > 0) {
        read_bytes = (total_bytes < read_bytes)?total_bytes:read_bytes;
        /* call pcm_read with not more than period size */
        status = pcm_read(p_ses->pcm, p_buf, read_bytes);

        pthread_mutex_lock(&p_ses->lock);
        /* in case of SSR and concurrency the buffering can be stopped */
        if (!CHECK_STATE(p_ses->op_state, SES_BUFFERING)) {
            ALOGE("%s: SSR stopped lab op_state=0x%x status=%d",
                  __func__, p_ses->op_state, status);
            /* pcm_read would have filled the complete buffer */
            if (!status)
                total_bytes -= read_bytes;
            status = -EBUSY;
        }
        pthread_mutex_unlock(&p_ses->lock);

        if (status) {
            ALOGE("%s:[%p] pcm_read error %d total_bytes %d read_bytes %d",
                            __func__, p_ses, status, total_bytes, read_bytes);
            break;
        }
        p_buf += read_bytes;
        total_bytes -= read_bytes;
    }
    *p_bytes -= total_bytes;

    if(g_listen_hal->debug_dump_lab_data)
        debug_dump_lab_buffer(p_ses, DUMP_DATA, p_buf, *p_bytes);

    ALOGV("%s:[%p] Exit, status=%d, *p_bytes=%d",
                __func__, p_ses, status, *p_bytes);
    return status;
}

static int ape_stop_buffering(audio_listen_session_t* p_ses)
{
    int status = 0;

    ALOGV("%s:[%p] Enter", __func__, p_ses);
    if (!p_ses->pcm) {
        ALOGV("%s:[%p] pcm NULL", __func__, p_ses);
        return status;
    }
    if (pcm_ioctl(p_ses->pcm, SNDRV_LSM_STOP_LAB) < 0) {
        ALOGE("%s: ERROR. SNDRV_LSM_STOP_BUFFERING failed", __func__);
        status = -errno;
    }
    ALOGV("%s:[%p] Exit, status=%d", __func__, p_ses, status);
    return status;
}

static int route_open_session_cpe(audio_listen_session_t *p_ses)
{
    int status = 0;

    if (CHECK_STATE(p_ses->op_state, SES_OPENED )) {
        ALOGD("%s: session %p already openned", __func__, p_ses);
        return status;
    }
    status = cpe_open_session(p_ses);
    if (!status) {
        create_event_thread(p_ses);
        SET_STATE(p_ses->op_state, SES_OPENED );
    }
    return status;
}

static int route_close_session_cpe(audio_listen_session_t *p_ses)
{
    int status = 0;

    if (!CHECK_STATE(p_ses->op_state, SES_OPENED )) {
        ALOGD("%s: session %p already closed", __func__, p_ses);
        return status;
    }
    destroy_event_thread(p_ses);
    status = cpe_close_session(p_ses);
    if (!status) {
        CLEAR_STATE(p_ses->op_state, SES_OPENED );
    }

    return status;
}

static int route_reg_sm_cpe(audio_listen_session_t *p_ses,
                             listen_sound_model_params_t *params)
{
    int status = 0;
    if (CHECK_STATE(p_ses->op_state, SES_SM_REGD)) {
        ALOGV("%s: session %p already registered", __func__, p_ses);
        return status;
    }
    status = cpe_reg_sm(p_ses, params);
    if (!status) {
        SET_STATE(p_ses->op_state, SES_SM_REGD);
    }
    return status;
}

static int route_dereg_sm_cpe(audio_listen_session_t* p_ses)
{
    int status = 0;
    if (!CHECK_STATE(p_ses->op_state, SES_SM_REGD)) {
        ALOGD("%s: session %p already deregistered", __func__, p_ses);
        return status;
    }
    status = cpe_dereg_sm(p_ses);
    if (!status) {
        CLEAR_STATE(p_ses->op_state, SES_SM_REGD);
    }
    return status;
}

static int route_start_cpe(audio_listen_session_t* p_ses)
{
    int status = 0;
    if (CHECK_STATE(p_ses->op_state, SES_STARTED )) {
        ALOGD("%s: session %p already started", __func__, p_ses);
        return status;
    }
    status = cpe_start(p_ses);
    if (!status) {
        SET_STATE(p_ses->op_state, SES_STARTED );
    }
    return status;
}

static int route_stop_cpe(audio_listen_session_t* p_ses)
{
    int status = 0;
    if (!CHECK_STATE(p_ses->op_state, SES_STARTED)) {
        ALOGV("%s: session %p already stopped", __func__, p_ses);
        return status;
    }
    status = cpe_stop(p_ses);
    if (!status) {
        CLEAR_STATE(p_ses->op_state, SES_STARTED);
    }
    return status;
}

static int route_read_buffer_cpe(audio_listen_session_t* p_ses,
                                 uint8_t *p_buf,
                                 uint32_t *p_bytes)
{
    int status = 0;
    if (!CHECK_STATE(p_ses->op_state, SES_BUFFERING)) {
        ALOGE("%s:[%p] buffering not in progress", __func__, p_ses);
        return -ENOEXEC;
    }
    status = cpe_read_buffer(p_ses, p_buf, p_bytes);
    return status;
}

static int route_stop_buffering_cpe(audio_listen_session_t* p_ses)
{
    int status = 0;
    if (!CHECK_STATE(p_ses->op_state, SES_BUFFERING)) {
        ALOGV("%s:[%p] no buffering in progress", __func__, p_ses);
        return status;
    }
    status = cpe_stop_buffering(p_ses);
    if (!status)
        CLEAR_STATE(p_ses->op_state, SES_BUFFERING);
    return status;
}

static int route_open_session_ape(audio_listen_session_t *p_ses)
{
    int status = 0;

    if (CHECK_STATE(p_ses->op_state, SES_OPENED )) {
        ALOGD("%s: session %p already openned", __func__, p_ses);
        return status;
    }
    status = ape_open_session(p_ses);
    if (!status) {
        create_event_thread(p_ses);
        SET_STATE(p_ses->op_state, SES_OPENED );
    }
    return status;
}

static int route_close_session_ape(audio_listen_session_t *p_ses)
{
    int status = 0;

    if (!CHECK_STATE(p_ses->op_state, SES_OPENED )) {
        ALOGD("%s: session %p already closed", __func__, p_ses);
        return status;
    }
    destroy_event_thread(p_ses);
    status = ape_close_session(p_ses);
    if (!status) {
        CLEAR_STATE(p_ses->op_state, SES_OPENED );
    }

    return status;
}

static int route_reg_sm_ape(audio_listen_session_t *p_ses,
                              listen_sound_model_params_t *params)
{
    int status = 0;
    if (CHECK_STATE(p_ses->op_state, SES_SM_REGD)) {
        ALOGD("%s: session %p already registered", __func__, p_ses);
        return status;
    }
    status = ape_reg_sm(p_ses, params);
    if (!status) {
        SET_STATE(p_ses->op_state, SES_SM_REGD);
    }
    return status;
}

static int route_dereg_sm_ape(audio_listen_session_t* p_ses)
{
    int status = 0;
    if (!CHECK_STATE(p_ses->op_state, SES_SM_REGD)) {
        ALOGD("%s: session %p already deregistered", __func__, p_ses);
        return status;
    }
    status = ape_dereg_sm(p_ses);
    if (!status) {
        CLEAR_STATE(p_ses->op_state, SES_SM_REGD);
    }
    return status;
}

static int route_start_ape(audio_listen_session_t* p_ses)
{
    int status = 0;
    if (CHECK_STATE(p_ses->op_state, SES_STARTED )) {
        ALOGD("%s: session %p already started", __func__, p_ses);
        return status;
    }
    status = ape_start(p_ses);
    if (!status) {
        SET_STATE(p_ses->op_state, SES_STARTED );
    }
    return status;
}

static int route_stop_ape(audio_listen_session_t* p_ses)
{
    int status = 0;
    if (!CHECK_STATE(p_ses->op_state, SES_STARTED)) {
        ALOGD("%s: session %p already stopped", __func__, p_ses);
        return status;
    }
    status = ape_stop(p_ses);
    if (!status) {
        CLEAR_STATE(p_ses->op_state, SES_STARTED);
    }
    return status;
}

static int route_read_buffer_ape(audio_listen_session_t* p_ses,
                                 uint8_t *p_buf,
                                 uint32_t *p_bytes)
{
    int status = 0;

    if (!CHECK_STATE(p_ses->op_state, SES_BUFFERING)) {
        ALOGE("%s:[%p] buffering not in progress", __func__, p_ses);
        return -ENOEXEC;
    }
    status = ape_read_buffer(p_ses, p_buf, p_bytes);
    return status;
}

static int route_stop_buffering_ape(audio_listen_session_t* p_ses)
{
    int status = 0;

    if (!CHECK_STATE(p_ses->op_state, SES_BUFFERING)) {
        ALOGV("%s:[%p] no buffering in progress", __func__, p_ses);
        return status;
    }
    status = ape_stop_buffering(p_ses);
    if (!status)
        CLEAR_STATE(p_ses->op_state, SES_BUFFERING);
    return status;
}

int register_sound_model(listen_session_t* p_handle,
                         listen_sound_model_params_t *params)
{
    int status = 0;
    audio_listen_session_t *p_ses;
    listen_exec_mode_t exec_mode;

    p_ses = (audio_listen_session_t *)(p_handle);
    ALOGV("%s:[%p] Enter", __func__, p_ses);

    if (!g_listen_hal) {
        ALOGE("%s: ERROR. listen hardware is not created", __func__);
        return -ENODEV;
    }
    if (!p_ses) {
        ALOGE("%s: session handle is NULL", __func__);
        return -EINVAL;
    }

    pthread_mutex_lock(&g_listen_hal->lock);
    status = determine_exec_mode(p_ses, params, &exec_mode);
    if (status) {
        pthread_mutex_unlock(&g_listen_hal->lock);
        ALOGE("%s: ERROR. capabilities exceeded, force failure", __func__);
        pthread_mutex_unlock(&g_listen_hal->lock);
        return status;
    }

    p_ses->exec_mode = exec_mode;
    p_ses->num_keywords = params->num_keywords;
    p_ses->num_user_pairings = params->num_user_pairings;

    if (LISTEN_EXEC_MODE_CPE == exec_mode) {
        ALOGV("%s: CPE mode", __func__);
        p_ses->lfptrs = &cpe_fptrs;
        g_listen_hal->avail_cpe_kw -= p_ses->num_keywords;
        g_listen_hal->avail_cpe_kw_user_pairs -= p_ses->num_user_pairings;
    } else {
        ALOGV("%s: APE mode", __func__);
        p_ses->lfptrs = &ape_fptrs;
        g_listen_hal->avail_ape_kw -= p_ses->num_keywords;
        g_listen_hal->avail_ape_kw_user_pairs -= p_ses->num_user_pairings;
    }
    pthread_mutex_unlock(&g_listen_hal->lock);

    pthread_mutex_lock(&p_ses->lock);
    if (CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS)) {
        /* Wait till SSR completes */
        ALOGV("%s: waiting on SSR completion", __func__);
        pthread_cond_wait(&p_ses->ssr_cond, &p_ses->lock);
    }

    platform_listen_connect_mad(g_listen_hal->platform, true, p_ses);
    status = p_ses->lfptrs->open(p_ses);
    if (!status) {
        status = p_ses->lfptrs->reg_sm(p_ses, params);
        if (!status)
            status = p_ses->lfptrs->start(p_ses);
        else
            p_ses->lfptrs->close(p_ses);
    }
    pthread_mutex_unlock(&p_ses->lock);

    pthread_mutex_lock(&g_listen_hal->list_lock);
    list_add_tail(&g_listen_hal->ses_list, &p_ses->list_node);
    pthread_mutex_unlock(&g_listen_hal->list_lock);

    ALOGV("%s:[%p] Exit status=%d", __func__, p_ses, status);
    return status;
}

int deregister_sound_model(listen_session_t* p_handle)
{
    int status = 0;
    audio_listen_session_t *p_ses;

    p_ses = (audio_listen_session_t *)(p_handle);
    ALOGV("%s:[%p] Enter", __func__, p_ses);

    if (!g_listen_hal) {
        ALOGE("%s: ERROR. listen hardware is not created", __func__);
        return -ENODEV;
    }
    if (!p_ses || !p_ses->lfptrs) {
        ALOGE("%s: session handle is NULL", __func__);
        return -EINVAL;
    }

    pthread_mutex_lock(&p_ses->lock);
    if (CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS)) {
        /* Wait till SSR completes */
        ALOGE("%s: waiting on SSR completion", __func__);
        pthread_cond_wait(&p_ses->ssr_cond, &p_ses->lock);
    }
    /* Session may have been in bad state after SSR */
    if (CHECK_STATE(p_ses->op_state, SES_IN_BAD_STATE)) {
        ALOGE("%s: ERROR. session in bad state", __func__);
        goto error;
    }
    status = p_ses->lfptrs->stop_buffering(p_ses);
    if (!status) {
        status = p_ses->lfptrs->stop(p_ses);
        if (!status) {
            status = p_ses->lfptrs->dereg_sm(p_ses);
            if (!status)
                status = p_ses->lfptrs->close(p_ses);
        }
    }
    if (status)
        goto error;
    pthread_mutex_unlock(&p_ses->lock);

    pthread_mutex_lock(&g_listen_hal->list_lock);
    list_remove(&p_ses->list_node);
    pthread_mutex_unlock(&g_listen_hal->list_lock);

    pthread_mutex_lock(&g_listen_hal->lock);
    if (LISTEN_EXEC_MODE_CPE == p_ses->exec_mode) {
        g_listen_hal->avail_cpe_kw += p_ses->num_keywords;
        g_listen_hal->avail_cpe_kw_user_pairs += p_ses->num_user_pairings;
    }
    else {
        g_listen_hal->avail_ape_kw += p_ses->num_keywords;
        g_listen_hal->avail_ape_kw_user_pairs += p_ses->num_user_pairings;
    }
    p_ses->num_keywords = 0;
    p_ses->num_user_pairings = 0;
    pthread_mutex_unlock(&g_listen_hal->lock);

    ALOGV("%s:[%p] Exit status=%d", __func__, p_ses, status);
    return status;

error:
    pthread_mutex_unlock(&p_ses->lock);
    ALOGV("%s:[%p] ERROR. Exit status=%d", __func__, p_ses, status);
    return status;
}

int read_buffer(listen_session_t* p_handle,
                uint8_t *p_buf,
                uint32_t *p_bytes)
{
    int status = 0;
    audio_listen_session_t *p_ses;

    p_ses = (audio_listen_session_t *)(p_handle);
    ALOGV("%s:[%p] Enter p_buf=%p, p_bytes=%p", __func__, p_ses, p_buf, p_bytes);

    if (!g_listen_hal) {
        ALOGE("%s: ERROR. listen hardware is not created", __func__);
        return -ENODEV;
    }
    if (!p_ses || !p_ses->lfptrs || !p_buf || !p_bytes) {
        ALOGE("%s: ERROR. NULL param p_ses=%p, p_buf=%p, p_bytes=%p", __func__,
              p_ses, p_buf, p_bytes);
        return -EINVAL;
    }
    if (!*p_bytes) {
        ALOGE("%s: ERROR. zero bytes requested ", __func__);
        return -EINVAL;
    }

    pthread_mutex_lock(&p_ses->lock);
    if (CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS)||
        !CHECK_STATE(p_ses->op_state, SES_BUFFERING)) {
        ALOGE("%s: SSR wipes out LAB op_state=0x%x", __func__, p_ses->op_state);
        pthread_mutex_unlock(&p_ses->lock);
        *p_bytes = 0;
        return -EBUSY;
    }
    pthread_mutex_unlock(&p_ses->lock);

    status = p_ses->lfptrs->read_buffer(p_ses, p_buf, p_bytes);

    ALOGV("%s:[%p] Exit status=%d", __func__, p_ses, status);
    return status;
}

int stop_buffering(listen_session_t* p_handle)
{
    int status = 0;
    audio_listen_session_t *p_ses;

    p_ses = (audio_listen_session_t *)(p_handle);
    ALOGV("%s:[%p] Enter", __func__, p_ses);

    if (!g_listen_hal) {
        ALOGE("%s: ERROR. listen hardware is not created", __func__);
        return -ENODEV;
    }
    if (!p_ses || !p_ses->lfptrs) {
        ALOGE("%s: session handle is NULL", __func__);
        return -EINVAL;
    }

    pthread_mutex_lock(&p_ses->lock);
    if (CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS)) {
        ALOGE("%s: SSR terminated LAB", __func__);
        pthread_mutex_unlock(&p_ses->lock);
        return -EIO;
    }
    status = p_ses->lfptrs->stop_buffering(p_ses);
    pthread_mutex_unlock(&p_ses->lock);

    ALOGV("%s:[%p] Exit status=%d", __func__, p_ses, status);
    return status;
}

int set_session_observer(listen_session_t* p_handle,
                         listen_callback_t cb_func,
                         void *priv)
{
    audio_listen_session_t *p_ses;

    p_ses = (audio_listen_session_t *)p_handle;
    ALOGV("%s:[%p] Enter cb=%p", __func__, p_ses, cb_func);

    if (!g_listen_hal) {
        ALOGE("%s: ERROR. listen hardware is not created", __func__);
        return -ENODEV;
    }
    if (!p_handle) {
        ALOGE("%s: ERROR. NULL param handle=%p", __func__, p_handle);
        return -EINVAL;
    }

    p_ses->client_data = priv;
    p_ses->ses_cb_func = cb_func;

    ALOGV("%s:[%p] Exit", __func__, p_ses);
    return 0;
}

int open_listen_session(struct audio_hw_device *dev,
                        listen_open_params_t *params,
                        listen_session_t **pp_handle)
{
    audio_listen_session_t *p_ses = NULL;
    int status = 0;
    listen_exec_mode_t exec_mode;
    int cpe_ses = 0, ape_ses = 0;

    ALOGV("%s: Enter", __func__);

    *pp_handle = NULL;
    if (!g_listen_hal) {
        ALOGE("%s: ERROR. listen hardware is not created", __func__);
        status = -EINVAL;
        goto error;
    }
    if (!params) {
        ALOGE("%s: ERROR. listen_open_params NULL", __func__);
        status = -EINVAL;
        goto error;
    }

    if (params->app_id != LISTEN_APP_TYPE_SVA &&
        params->app_id != LISTEN_APP_TYPE_SVA_V2) {
        ALOGV("%s: ERROR. Invalid app_id=%d", __func__, params->app_id);
        status = -EINVAL;
        goto error;
    }

    /* check for rx concurrency */
    if (g_listen_hal->rx_concurrency_disabled) {
        pthread_mutex_lock(&g_listen_hal->lock);
        get_num_of_active_sessions(&cpe_ses, &ape_ses);
        if ((g_listen_hal->rx_concurrency_active > 0) &&
            (cpe_ses > 0 || ape_ses > 0)) {
            ALOGW("%s: session prevented by rx concurrency", __func__);
            status = -ENODEV;
            pthread_mutex_unlock(&g_listen_hal->lock);
            goto error;
        }
        pthread_mutex_unlock(&g_listen_hal->lock);
    }

    /* CPE takes time to become online, so parse for the pcm devices
       here instead during boot time */
    if (!CHECK_BIT(g_listen_hal->hw_type, LISTEN_HW_APE|LISTEN_HW_CPE)) {
        status = listen_get_hw_type();
        if (status)
            goto error;
    }

    p_ses = (audio_listen_session_t *)calloc(1, sizeof(audio_listen_session_t));
    if(!p_ses) {
        ALOGV("%s: ERROR. listen session alloc failed", __func__);
        status = -ENOMEM;
        goto error;
    }

    p_ses->app_type = params->app_id;
    p_ses->handle.register_sound_model = register_sound_model;
    p_ses->handle.deregister_sound_model = deregister_sound_model;
    p_ses->handle.set_session_observer = set_session_observer;
    p_ses->handle.read_buffer = read_buffer;
    p_ses->handle.stop_buffering = stop_buffering;

    pthread_mutex_init(&p_ses->lock, (const pthread_mutexattr_t *) NULL);

    *pp_handle = &p_ses->handle;

    if(g_listen_hal->debug_dump_lab_data)
        debug_dump_lab_buffer(p_ses, DUMP_INIT, 0, 0);

    ALOGV("%s: Exit ses=%p status=%d", __func__, *pp_handle, status);
    return status;

error:
    ALOGV("%s: ERROR. Exit status=%d", __func__, status);
    return status;
}

int close_listen_session(struct audio_hw_device *dev,
                           listen_session_t *p_handle)
{
    audio_listen_session_t *p_ses;
    struct listnode *p_ses_node;
    int status = 0;

    p_ses = (audio_listen_session_t *)(p_handle);
    ALOGV("%s:[%p] Enter", __func__, p_ses);

    if (!g_listen_hal) {
        ALOGE("%s: ERROR. listen hardware is not created", __func__);
        return -ENODEV;
    }
    if (!p_handle) {
        ALOGE("%s: ERROR. session handle is NULL", __func__);
        return -EINVAL;
    }
    if (!p_ses->lfptrs)
        goto cleanup1;

    pthread_mutex_lock(&p_ses->lock);
    if (CHECK_STATE(p_ses->op_state, SES_SSR_IN_PROGRESS)) {
        /* Wait till SSR completes */
        ALOGE("%s: waiting on SSR completion", __func__);
        pthread_cond_wait(&p_ses->ssr_cond, &p_ses->lock);
    }
    /* Session may have been in bad state after SSR */
    if (CHECK_STATE(p_ses->op_state, SES_IN_BAD_STATE)) {
        ALOGE("%s: ERROR. session in bad state", __func__);
        status = -ENODEV;
        goto cleanup;
    }
    /* Session may be in un-initialized state */
    if (p_ses->lfptrs == NULL) {
        ALOGE("%s: ERROR. session un-initialized", __func__);
        status = -ENODEV;
        pthread_mutex_unlock(&p_ses->lock);
        goto cleanup_uninit;
    }
    /* If client calls close session without deregister soundmodel,
       we will gracefully handle this by ourselves */
    status = p_ses->lfptrs->stop_buffering(p_ses);
    if (!status) {
        status = p_ses->lfptrs->stop(p_ses);
        if (!status) {
            status = p_ses->lfptrs->dereg_sm(p_ses);
            if (!status)
                status = p_ses->lfptrs->close(p_ses);
        }
    }

cleanup:
    pthread_mutex_unlock(&p_ses->lock);

    pthread_mutex_lock(&g_listen_hal->list_lock);
    list_for_each(p_ses_node, &g_listen_hal->ses_list) {
        /* session might have been already removed during deregister soundmodel,
           so check here */
        if (p_ses == node_to_item(p_ses_node, audio_listen_session_t, list_node)) {
            list_remove(&p_ses->list_node);
        }
    }
    pthread_mutex_unlock(&g_listen_hal->list_lock);

    if (LISTEN_EXEC_MODE_CPE == p_ses->exec_mode) {
        g_listen_hal->avail_cpe_kw += p_ses->num_keywords;
        g_listen_hal->avail_cpe_kw_user_pairs += p_ses->num_user_pairings;
    }
    else {
        g_listen_hal->avail_ape_kw += p_ses->num_keywords;
        g_listen_hal->avail_ape_kw_user_pairs += p_ses->num_user_pairings;
    }
cleanup_uninit:

cleanup1:
    pthread_mutex_destroy(&p_ses->lock);

    if(g_listen_hal->debug_dump_lab_data)
        debug_dump_lab_buffer(p_ses, DUMP_DEINIT, 0, 0);

    free(p_ses);

    ALOGV("%s:[%p] Exit status=%d", __func__, p_ses, status);
    return status;
}

int listen_hw_set_parameters(audio_hw_device_t *dev, const char *kvpairs)
{
    int ret = 0;
    ALOGV("%s: Enter", __func__);
    if (!g_listen_hal) {
        ALOGE("%s: ERROR. listen hardware is not created", __func__);
        return -ENODEV;
    }
    ret = handle_set_parameters(kvpairs);
    ALOGV("%s: Exit, ret=%d", __func__, ret);
    return ret;
}

char* listen_hw_get_parameters(audio_hw_device_t *dev, const char *kvpairs)
{
    char *str;
    ALOGV("%s: Enter", __func__);

    if (!g_listen_hal) {
        ALOGE("%s: ERROR. listen hardware is not created", __func__);
        return NULL;
    }
    str = handle_get_parameters(kvpairs);

    ALOGV("%s: Exit str=%s", __func__, str);
    return str;
}

void listen_hw_notify_event(event_type_t event_type)
{
    ALOGV("%s: Enter", __func__);

    if (!g_listen_hal) {
        ALOGW("%s: ERROR. listen hardware is not created", __func__);
        return;
    }
    handle_notify_event(event_type);
}

int set_mad_observer(audio_hw_device_t *dev, listen_callback_t cb_func)
{
    ALOGV("%s: Enter cb_func=%p", __func__, cb_func);

    if (!g_listen_hal) {
        ALOGE("%s: ERROR. listen hardware is not created", __func__);
        return -ENODEV;
    }

    g_listen_hal->mad_cb_func = cb_func;
    ALOGV("%s: Exit", __func__);
    return 0;
}
