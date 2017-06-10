/* sound_trigger_platform.c
 *
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_TAG "sound_trigger_platform"
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

#include <cutils/log.h>
#include <dlfcn.h>
#include <expat.h>
#include "sound_trigger_platform.h"

typedef int  (*acdb_loader_init_v2_t)(const char *, const char *, int);
typedef void (*acdb_loader_deallocate_t)(void);
typedef void (*acdb_loader_send_listen_cal_t)(int, int);

struct st_device_index
st_device_name_idx[ST_DEVICE_EXEC_MODE_MAX][ST_DEVICE_MAX] = {
   {
       {"DEVICE_HANDSET_APE_ACDB_ID", ST_DEVICE_HANDSET_MIC},
   },
   {
       {"DEVICE_HANDSET_CPE_ACDB_ID", ST_DEVICE_HANDSET_MIC},
   },
};

static const char * const st_device_table[ST_DEVICE_MAX] = {
    [ST_DEVICE_NONE] = "none",
    [ST_DEVICE_HANDSET_MIC] = "listen-handset-mic",
};

/* ACDB IDs for each device for both CDSP and ADSP */
static int acdb_device_table[ST_DEVICE_EXEC_MODE_MAX][ST_DEVICE_MAX] = {
    {
      [ST_DEVICE_NONE] = -1,
      [ST_DEVICE_HANDSET_MIC] = DEVICE_HANDSET_APE_ACDB_ID,
    },
    {
      [ST_DEVICE_NONE] = -1,
      [ST_DEVICE_HANDSET_MIC] = DEVICE_HANDSET_CPE_ACDB_ID,
    }
};

struct platform_data {
    sound_trigger_device_t *stdev;
    void *acdb_handle;
    acdb_loader_send_listen_cal_t acdb_send_cal;
    acdb_loader_deallocate_t acdb_deinit;
};

int platform_stdev_set_acdb_id(void *userdata __unused, const char* device, int acdb_id)
{
    int i, j;
    int ret = 0;
    int dev_idx = ST_DEVICE_NONE;

    if (device == NULL) {
       ALOGE("%s: device name is NULL", __func__);
       ret = -ENODEV;
       goto done;
    }

    for (i = 0; i < ST_DEVICE_EXEC_MODE_MAX; i++) {
        for (j = 0; j < ST_DEVICE_MAX; i++) {
           if(strcmp(st_device_name_idx[i][j].name, device) == 0)
               dev_idx = st_device_name_idx[i][j].index;
               break;
        }
        if (dev_idx != ST_DEVICE_NONE)
            break;
    }
    if (dev_idx == ST_DEVICE_NONE) {
       ALOGE("%s: Could not find index for device name = %s",
               __func__, device);
       ret = -ENODEV;
       goto done;
    }

    acdb_device_table[i][dev_idx] = acdb_id;

done:
    return ret;

}

void platform_stdev_set_default_config(struct platform_data *platform)
{
    sound_trigger_device_t *stdev = platform->stdev;

    stdev->run_on_ape =  true;
    stdev->max_ape_sessions = 1;
    stdev->avail_ape_phrases = 1;
    stdev->avail_ape_users = 1;
    stdev->max_cpe_sessions = 1;
    stdev->avail_cpe_phrases  = 1;
    stdev->avail_cpe_users = 1;
    stdev->rx_conc_max_st_ses = UINT_MAX;
}

void platform_stdev_set_config(void *userdata, const char* param, const char* value)
{
    struct platform_data *platform = userdata;
    sound_trigger_device_t *stdev;

    if (!platform) {
        ALOGE("%s: platform data NULL", __func__);
        return;
    }
    if (!platform->stdev) {
        ALOGE("%s: platform stdev data NULL", __func__);
        return;
    }
    stdev = platform->stdev;
    if (!strcmp(param, "execution_type")) {
        if(!strcmp(value, "CPE"))
            stdev->run_on_ape =  false;
    }
    else if (!strcmp(param, "enable_transition")) {
        stdev->transition_enabled =
           (0 == strncasecmp(value, "true", 4))? true : false;
    }
    else if (!strcmp(param, "max_ape_sessions")) {
        stdev->max_ape_sessions = atoi(value);
    }
    else if (!strcmp(param, "max_cpe_sessions")) {
        stdev->max_cpe_sessions = atoi(value);
    }
    else if (!strcmp(param, "max_cpe_phrases")) {
        stdev->avail_cpe_phrases = atoi(value);
    }
    else if (!strcmp(param, "max_cpe_users")) {
        stdev->avail_cpe_users = atoi(value);
    }
    else if (!strcmp(param, "max_ape_phrases")) {
        stdev->avail_ape_phrases = atoi(value);
    }
    else if (!strcmp(param, "max_ape_users")) {
        stdev->avail_ape_users = atoi(value);
    }
    else if (!strcmp(param, "rx_concurrency_disabled")) {
        stdev->rx_concurrency_disabled =
           (0 == strncasecmp(value, "true", 4))? true:false;
        ALOGD("%s:rx_concurrency_disabled = %d",
                 __func__, stdev->rx_concurrency_disabled);
    }
    else if (!strcmp(param, "rx_conc_max_st_ses")) {
        stdev->rx_conc_max_st_ses = atoi(value);
        ALOGD("%s:rx_conc_max_st_ses = %d",
                 __func__, stdev->rx_conc_max_st_ses);
    }
    else if (!strcmp(param, "enable_failure_detection")) {
        stdev->detect_failure =
           (0 == strncasecmp(value, "true", 4))? true:false;
    }
    else
        ALOGD("%s: unknown config param, ignoring..", __func__);
}

static void start_tag(void *userdata, const XML_Char *tag_name,
                      const XML_Char **attr)
{
    int ret;

    if (!strcmp(tag_name, "device")) {
        if (strcmp(attr[0], "name") || strcmp(attr[2], "value")) {
            ALOGE("%s: 'name' or 'value' not found! for device tag", __func__);
            goto done;
        }
        ret = platform_stdev_set_acdb_id(userdata, (const char *)attr[1], atoi((const char *)attr[3]));
        if (ret < 0) {
            ALOGE("%s: Device %s in platform xml not found, no ACDB ID set!",
                  __func__, attr[1]);
            goto done;
        }
    }
    if (!strcmp(tag_name, "ctrl")) {
        if (strcmp(attr[0], "name") || strcmp(attr[2], "value")) {
            ALOGE("%s: 'name' or 'value' not found! for ctrl tag", __func__);
            goto done;
        }
        platform_stdev_set_config(userdata, (const char *)attr[1], (const char *)attr[3]);
    }
done:
    return;
}

static void end_tag(void *userdata __unused, const XML_Char *tag_name __unused)
{
}

static int platform_parse_info(struct platform_data *platform, const char *filename)
{
    XML_Parser      parser;
    FILE            *file;
    int             ret = 0;
    int             bytes_read;
    void            *buf;

    file = fopen(filename, "r");
    if (!file) {
        ALOGD("%s: Failed to open %s, using defaults", __func__, filename);
        ret = -ENODEV;
        goto done;
    }

    parser = XML_ParserCreate(NULL);
    if (!parser) {
        ALOGE("%s: Failed to create XML parser!", __func__);
        ret = -ENODEV;
        goto err_close_file;
    }

    XML_SetUserData(parser, platform);

    XML_SetElementHandler(parser, start_tag, end_tag);

    while (1) {
        buf = XML_GetBuffer(parser, BUF_SIZE);
        if (buf == NULL) {
            ALOGE("%s: XML_GetBuffer failed", __func__);
            ret = -ENOMEM;
            goto err_free_parser;
        }

        bytes_read = fread(buf, 1, BUF_SIZE, file);
        if (bytes_read < 0) {
            ALOGE("%s: fread failed, bytes read = %d", __func__, bytes_read);
             ret = bytes_read;
            goto err_free_parser;
        }

        if (XML_ParseBuffer(parser, bytes_read,
                            bytes_read == 0) == XML_STATUS_ERROR) {
            ALOGE("%s: XML_ParseBuffer failed, for %s",
                __func__, filename);
            ret = -EINVAL;
            goto err_free_parser;
        }

        if (bytes_read == 0)
            break;
    }

err_free_parser:
    XML_ParserFree(parser);
err_close_file:
    fclose(file);
done:
    return ret;
}

static void query_stdev_platform(sound_trigger_device_t *stdev,
                                 const char *snd_card_name,
                                 char *mixer_path_xml)
{
    if (strstr(snd_card_name, "msm8939-tapan")) {
        strlcpy(mixer_path_xml, MIXER_PATH_XML_WCD9306,
                        sizeof(MIXER_PATH_XML_WCD9306));
    } else {
        strlcpy(mixer_path_xml, MIXER_PATH_XML,
                         sizeof(MIXER_PATH_XML));
    }

    if ((strstr(snd_card_name, "msm8939") ||
        strstr(snd_card_name, "msm8909") ||
        strstr(snd_card_name, "msm8x16")) &&
        !strstr(snd_card_name, "msm8939-tomtom")) {
        stdev->sw_mad = true;
    }
}

void *platform_stdev_init(sound_trigger_device_t *stdev)
{
    int ret = 0, retry_num = 0;
    struct platform_data *my_data = NULL;
    const char *snd_card_name = NULL;
    acdb_loader_init_v2_t acdb_init;
    char mixer_path_xml[100];

    ALOGI("%s: Enter", __func__);
    my_data = calloc(1, sizeof(struct platform_data));

    if (!my_data || !stdev) {
        ALOGE("%s: ERROR. NULL param", __func__);
        if(my_data)
            free(my_data);
        return NULL;
    }
    my_data->stdev = stdev;

    stdev->mixer = mixer_open(SOUND_CARD);
    while (!stdev->mixer && retry_num < MIXER_OPEN_MAX_NUM_RETRY) {
        usleep(RETRY_US);
        stdev->mixer = mixer_open(SOUND_CARD);
        retry_num++;
    }

    if (!stdev->mixer) {
        ALOGE("%s: ERROR. Unable to open the mixer, aborting", __func__);
        goto cleanup;
    }

    snd_card_name = mixer_get_name(stdev->mixer);

    query_stdev_platform(stdev, snd_card_name, mixer_path_xml);
    stdev->audio_route = audio_route_init(SOUND_CARD, mixer_path_xml);
    if (!stdev->audio_route) {
        ALOGE("%s: ERROR. Failed to init audio route controls, aborting.",
                __func__);
        goto cleanup;
    }

    my_data->acdb_handle = dlopen(LIB_ACDB_LOADER, RTLD_NOW);
    if (my_data->acdb_handle == NULL) {
        ALOGE("%s: ERROR. dlopen failed for %s", __func__, LIB_ACDB_LOADER);
        goto cleanup;
    }

    acdb_init = (acdb_loader_init_v2_t)dlsym(my_data->acdb_handle,
                                              "acdb_loader_init_v2");
    if (acdb_init == NULL) {
        ALOGE("%s: dlsym error %s for acdb_loader_init_v2", __func__, dlerror());
        goto cleanup;
    }

    my_data->acdb_deinit = (acdb_loader_deallocate_t)dlsym(my_data->acdb_handle,
                                           "acdb_loader_deallocate_ACDB");
    if (my_data->acdb_deinit == NULL) {
        ALOGE("%s: dlsym error %s for acdb_loader_deallocate_ACDB", __func__, dlerror());
        goto cleanup;
    }

    my_data->acdb_send_cal = (acdb_loader_send_listen_cal_t)
              dlsym(my_data->acdb_handle, "acdb_loader_send_listen_cal");

    if (my_data->acdb_send_cal == NULL) {
       ALOGE("%s: ERROR. dlsym Error:%s acdb_loader_send_listen_cal", __func__,
               dlerror());
       goto cleanup;
    }

    ALOGI("%s: acdb_init: %s", __func__, snd_card_name);
    ret = acdb_init(snd_card_name, NULL, 0);
    if (ret) {
        ALOGE("%s: ERROR. acdb_loader_init_v2 failed status %d", __func__, ret);
        goto cleanup;
    }

    platform_stdev_set_default_config(my_data);

    platform_parse_info(my_data, PLATFORM_PATH_XML);

    return my_data;

cleanup:
    if (my_data->acdb_handle)
        dlclose(my_data->acdb_handle);

    if (stdev->audio_route)
        audio_route_free(stdev->audio_route);

    if (stdev->mixer)
        mixer_close(stdev->mixer);

    free(my_data);
    return NULL;
}

void platform_stdev_deinit(void *platform)
{
    struct platform_data *my_data = (struct platform_data *)platform;

    ALOGI("%s: Enter", __func__);
    if (my_data) {
        my_data->acdb_deinit();
        dlclose(my_data->acdb_handle);
        audio_route_free(my_data->stdev->audio_route);
        mixer_close(my_data->stdev->mixer);
        free(my_data);
    }
}

int platform_stdev_get_device_name
(
   void *platform,
   audio_devices_t device,
   char *device_name
)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    st_device_t st_device = ST_DEVICE_NONE;

    if(device == AUDIO_DEVICE_NONE)
        ALOGV("%s: device none",__func__);

    st_device = ST_DEVICE_HANDSET_MIC;//TODO

    if (st_device != ST_DEVICE_NONE) {
        strlcpy(device_name, st_device_table[st_device], DEVICE_NAME_MAX_SIZE);
    } else {
        strlcpy(device_name, "", DEVICE_NAME_MAX_SIZE);
        return -EINVAL;
    }
    return 0;
}

int platform_stdev_get_device
(
   void *platform,
   audio_devices_t device
)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    st_device_t st_device = ST_DEVICE_NONE;

    if(device == AUDIO_DEVICE_NONE)
        ALOGV("%s: device none",__func__);

    st_device = ST_DEVICE_HANDSET_MIC;//TODO

    return st_device;
}

int platform_stdev_send_calibration
(
   void *platform,
   audio_devices_t device,
   st_exec_mode_t exec_mode,
   int app_id
)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    int acdb_id;
    st_device_t st_device;

    st_device = platform_stdev_get_device(platform, device);
    if (st_device == ST_DEVICE_NONE) {
        ALOGE("%s: Could not find valid device",__func__);
        return -EINVAL;
    }

    acdb_id = acdb_device_table[exec_mode][st_device];
    if (acdb_id < 0) {
        ALOGE("%s: Could not find acdb id for device(%d)",
              __func__, device);
        return -EINVAL;
    }
    if (my_data->acdb_send_cal) {
        ALOGD("%s: sending calibration for device(%d) acdb_id(%d)",
              __func__, st_device, acdb_id);
        my_data->acdb_send_cal(acdb_id, app_id);
    }
    return 0;
}

bool platform_stdev_check_and_update_concurrency
(
   void *platform,
   audio_event_type_t event_type,
   unsigned int num_sessions
)
{
    struct platform_data *my_data;
    sound_trigger_device_t *stdev;
    bool concurrency_ses_allowed = true;

    if (!platform) {
        ALOGE("%s: NULL platform", __func__);
        return false;
    }
    my_data = (struct platform_data *)platform;
    if (!my_data->stdev) {
        ALOGE("%s: platform stdev data is NULL", __func__);
        return false;
    }
    stdev = my_data->stdev;

    switch (event_type) {
    case AUDIO_EVENT_CAPTURE_DEVICE_ACTIVE:
        stdev->tx_concurrency_active++;
        break;
    case AUDIO_EVENT_CAPTURE_DEVICE_INACTIVE:
        if (stdev->tx_concurrency_active > 0)
            stdev->tx_concurrency_active--;
        break;
    case AUDIO_EVENT_PLAYBACK_STREAM_ACTIVE:
            stdev->rx_concurrency_active++;
        break;
    case AUDIO_EVENT_PLAYBACK_STREAM_INACTIVE:
        if (stdev->rx_concurrency_active > 0)
            stdev->rx_concurrency_active--;
        break;
    default:
        break;
    }
    if ((stdev->tx_concurrency_active > 0) ||
        (stdev->rx_concurrency_disabled &&
          stdev->rx_concurrency_active > 0 &&
          num_sessions > stdev->rx_conc_max_st_ses)) {
        concurrency_ses_allowed = false;
    }

    ALOGD("%s: concurrency active %d, tx %d, rx %d, concurrency session_allowed %d",
          __func__, stdev->audio_concurrency_active, stdev->tx_concurrency_active,
          stdev->rx_concurrency_active, concurrency_ses_allowed);
    return concurrency_ses_allowed;
}

bool platform_stdev_is_session_allowed
(
    void *platform,
    unsigned int num_sessions,
    bool sound_model_loaded
)
{
    struct platform_data *my_data;
    sound_trigger_device_t *stdev;
    bool session_allowed = true;

    if (!platform) {
        ALOGE("%s: NULL platform", __func__);
        return false;
    }
    my_data = (struct platform_data *)platform;
    if (!my_data->stdev) {
        ALOGE("%s: platform stdev data is NULL", __func__);
        return false;
    }
    stdev = my_data->stdev;

    if (!stdev->sw_mad) {
        /* hw_mad case only applicable only when sound_model is loaded */
        if (sound_model_loaded) {
            if (stdev->tx_concurrency_active > 0)
                session_allowed = false;
            stdev->audio_concurrency_active = session_allowed ? false: true;
        }
    } else {
        /* sw_mad case applicable only before sound_model is loaded */
        /* num_sessions does not reflect current session yet */
        if (!sound_model_loaded) {
            if ((stdev->tx_concurrency_active > 0) ||
                (stdev->rx_concurrency_disabled &&
                stdev->rx_concurrency_active > 0 &&
                (num_sessions + 1) > stdev->rx_conc_max_st_ses)) {
                session_allowed = false;
            } else {
                stdev->audio_concurrency_active = false;
            }
        }
    }

    ALOGD("%s: stdev->audio_concurrency_active %d session_allowed %d", __func__,
          stdev->audio_concurrency_active, session_allowed);
    return session_allowed;
}

int platform_stdev_connect_mad
(
   void *platform,
   bool is_ape
)
{
    /* This mixer control is only valid for CPE supported codec */
    struct platform_data *my_data = (struct platform_data *)platform;
    sound_trigger_device_t *stdev = my_data->stdev;
    int status = 0;
    struct mixer_ctl *ctl = NULL;
    const char *mixer_ctl_name = "MAD_SEL MUX";

    if (stdev->sw_mad)
        return 0;

    ctl = mixer_get_ctl_by_name(stdev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: ERROR. Could not get ctl for mixer cmd - %s",
        __func__, mixer_ctl_name);
        return -EINVAL;
    }
    if(is_ape)
        status = mixer_ctl_set_enum_by_string(ctl, "MSM");
    else
        status = mixer_ctl_set_enum_by_string(ctl, "SPE");

    if (status)
        ALOGE("%s: ERROR. Mixer ctl set failed", __func__);

    return status;
}

