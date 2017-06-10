/* listen_platform.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_TAG "listen_platform"
#define LOG_NDDEBUG 0

#include <cutils/log.h>
#include <errno.h>
#include <stdio.h>
#include <dlfcn.h>
#include <expat.h>
#include "listen_platform.h"

#define PLATFORM_XML_PATH  "/system/etc/listen_platform_info.xml"
#define LIB_ACDB_LOADER "libacdbloader.so"
#define BUF_SIZE 1024

typedef void (*acdb_loader_send_listen_cal_t)(int, int);

struct listen_device_index
listen_device_name_idx[LISTEN_EXEC_MODE_MAX][LISTEN_DEVICE_MAX] = {
   {
       {"DEVICE_HANDSET_APE_ACDB_ID", LISTEN_DEVICE_HANDSET_MIC},
   },
   {
       {"DEVICE_HANDSET_CPE_ACDB_ID", LISTEN_DEVICE_HANDSET_MIC},
   },
};

static const char * const listen_device_table[LISTEN_DEVICE_MAX] = {
    [LISTEN_DEVICE_NONE] = "none",
    [LISTEN_DEVICE_HANDSET_MIC] = "listen-handset-mic",
};

/* ACDB IDs for each device for both CDSP and ADSP */
static int acdb_device_table[LISTEN_EXEC_MODE_MAX][LISTEN_DEVICE_MAX] = {
    {
      [LISTEN_DEVICE_NONE] = -1,
      [LISTEN_DEVICE_HANDSET_MIC] = DEVICE_HANDSET_APE_ACDB_ID,
    },
    {
      [LISTEN_DEVICE_NONE] = -1,
      [LISTEN_DEVICE_HANDSET_MIC] = DEVICE_HANDSET_CPE_ACDB_ID,
    }
};

struct platform_data {
    audio_listen_hardware_t *listen_hal;
    void *acdb_handle;
    acdb_loader_send_listen_cal_t acdb_loader_send_listen_cal;
};

int platform_listen_set_acdb_id(void *userdata, const char* device, int acdb_id)
{
    int i, j;
    int ret = 0;
    int dev_idx = LISTEN_DEVICE_NONE;

    if (device == NULL) {
       ALOGE("%s: device name is NULL", __func__);
       ret = -ENODEV;
       goto done;
    }

    for (i = 0; i < LISTEN_EXEC_MODE_MAX; i++) {
        for (j = 0; j < LISTEN_DEVICE_MAX; i++) {
           if(strcmp(listen_device_name_idx[i][j].name, device) == 0)
               dev_idx = listen_device_name_idx[i][j].index;
               break;
        }
        if (dev_idx != LISTEN_DEVICE_NONE)
            break;
    }
    if (dev_idx == LISTEN_DEVICE_NONE) {
       ALOGE("%s: Could not find index for device name = %s",
               __func__, device);
       ret = -ENODEV;
       goto done;
    }

    acdb_device_table[i][dev_idx] = acdb_id;

done:
    return ret;

}

void platform_listen_set_default_config(struct platform_data *platform)
{
    audio_listen_hardware_t *listen_hal = platform->listen_hal;

    listen_hal->run_on_ape =  true;
    listen_hal->max_ape_sessions = 1;
    listen_hal->avail_ape_kw = 1;
    listen_hal->avail_ape_kw_user_pairs = 1;
    listen_hal->max_cpe_sessions = 1;
    listen_hal->avail_cpe_kw = 1;
    listen_hal->avail_cpe_kw_user_pairs= 1;
    listen_hal->debug_dump_lab_data = false;
    listen_hal->rx_conc_max_st_ses = UINT_MAX;
}

void platform_listen_set_config(void *userdata, const char* param, const char* value)
{
    struct platform_data *platform = userdata;
    audio_listen_hardware_t *listen_hal;

    if (!platform) {
        ALOGE("%s: platform data NULL", __func__);
        return;
    }
    if (!platform->listen_hal) {
        ALOGE("%s: platform listen_hal data NULL", __func__);
        return;
    }
    listen_hal = platform->listen_hal;
    if (!strcmp(param, "execution_type")) {
        if(!strcmp(value, "CPE"))
            listen_hal->run_on_ape =  false;
    }
    else if (!strcmp(param, "max_ape_sessions")) {
        listen_hal->max_ape_sessions = atoi(value);
    }
    else if (!strcmp(param, "max_cpe_sessions")) {
        listen_hal->max_cpe_sessions = atoi(value);
    }
    else if (!strcmp(param, "max_cpe_keywords")) {
        listen_hal->avail_cpe_kw= atoi(value);
    }
    else if (!strcmp(param, "max_cpe_kw_user_pairs")) {
        listen_hal->avail_cpe_kw_user_pairs= atoi(value);
    }
    else if (!strcmp(param, "max_ape_keywords")) {
        listen_hal->avail_ape_kw = atoi(value);
    }
    else if (!strcmp(param, "max_ape_kw_user_pairs")) {
        listen_hal->avail_ape_kw_user_pairs = atoi(value);
    }
    else if (!strcmp(param, "dump_lab_data")) {
        if (!strcmp(value, "true"))
            listen_hal->debug_dump_lab_data = true;
    }
    else if (!strcmp(param, "rx_concurrency_disabled")) {
        listen_hal->rx_concurrency_disabled =
           (0 == strncasecmp(value, "true", 4))? true:false;
        ALOGD("%s:rx_concurrency_disabled = %d",
                 __func__, listen_hal->rx_concurrency_disabled);
    }
    else if (!strcmp(param, "sw_mad")) {
        listen_hal->sw_mad =
           (0 == strncasecmp(value, "true", 4))? true:false;
    }
    else if (!strcmp(param, "rx_conc_max_st_ses")) {
        listen_hal->rx_conc_max_st_ses = atoi(value);
        ALOGD("%s:rx_conc_max_st_ses = %d",
                 __func__, listen_hal->rx_conc_max_st_ses);
    }
    else
        ALOGD("%s: unknown config param, ignoring..", __func__);
}

static void start_tag(void *userdata, const XML_Char *tag_name,
                      const XML_Char **attr)
{
    int ret;

    if (!strcmp(tag_name, "device")) {
        if (strcmp(attr[0], "name") || strcmp(attr[2], "acdb_id")) {
            ALOGE("%s: 'name' or 'acdb_id' not found!", __func__);
            goto done;
        }
        ret = platform_listen_set_acdb_id(userdata, (const char *)attr[1], atoi((const char *)attr[3]));
        if (ret < 0) {
            ALOGE("%s: Device %s in platform xml not found, no ACDB ID set!",
                  __func__, attr[1]);
            goto done;
        }
    }
    if (!strcmp(tag_name, "ctrl")) {
        if (strcmp(attr[0], "name") || strcmp(attr[2], "value")) {
            ALOGE("%s: 'name' or 'value' not found!", __func__);
            goto done;
        }
        platform_listen_set_config(userdata, (const char *)attr[1], (const char *)attr[3]);
    }
done:
    return;
}

static void end_tag(void *userdata, const XML_Char *tag_name)
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

void *platform_listen_init(struct audio_listen_hardware *listen_hal)
{
    int ret = 0;
    struct platform_data *my_data = NULL;

    my_data = calloc(1, sizeof(struct platform_data));

    if (!my_data || !listen_hal) {
        ALOGE("%s: ERROR. NULL param", __func__);
        if(my_data)
            free(my_data);
        return NULL;
    }
    my_data->listen_hal = listen_hal;

    my_data->acdb_handle = dlopen(LIB_ACDB_LOADER, RTLD_NOW);
    if (my_data->acdb_handle == NULL) {
        ALOGE("%s: ERROR. dlopen failed for %s", __func__, LIB_ACDB_LOADER);
        goto cleanup;
    }

    my_data->acdb_loader_send_listen_cal = (acdb_loader_send_listen_cal_t)
              dlsym(my_data->acdb_handle, "acdb_loader_send_listen_cal");

    if (my_data->acdb_loader_send_listen_cal == NULL) {
       ALOGE("%s: ERROR. dlsym Error:%s acdb_loader_send_listen_cal", __func__,
               dlerror());
       goto cleanup;
    }

    platform_listen_set_default_config(my_data);

    platform_parse_info(my_data, PLATFORM_XML_PATH);

    return my_data;

cleanup:
    if (my_data->acdb_handle)
        dlclose(my_data->acdb_handle);

    free(my_data);
    return NULL;
}

void platform_listen_deinit(void *platform)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    free(platform);
}

int platform_listen_get_device_name
(
   void *platform,
   listen_device_t listen_device,
   char *device_name
)
{
    struct platform_data *my_data = (struct platform_data *)platform;

    if (listen_device >= LISTEN_DEVICE_MIN && listen_device < LISTEN_DEVICE_MAX) {
        strlcpy(device_name, listen_device_table[listen_device], DEVICE_NAME_MAX_SIZE);
    } else {
        strlcpy(device_name, "", DEVICE_NAME_MAX_SIZE);
        return -EINVAL;
    }
    return 0;
}

int platform_listen_send_calibration
(
   void *platform,
   listen_device_t listen_device,
   listen_exec_mode_t exec_mode,
   int app_id
)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    int acdb_id;

    acdb_id = acdb_device_table[exec_mode][listen_device];
    if (acdb_id < 0) {
        ALOGE("%s: Could not find acdb id for device(%d)",
              __func__, listen_device);
        return -EINVAL;
    }
    if (my_data->acdb_loader_send_listen_cal) {
        ALOGD("%s: sending calibration for device(%d) acdb_id(%d)",
              __func__, listen_device, acdb_id);
        my_data->acdb_loader_send_listen_cal(acdb_id, app_id);
    }
    return 0;
}

void platform_listen_update_concurrency
(
   void *platform,
   event_type_t event_type
)
{
    struct platform_data *my_data;
    audio_listen_hardware_t *listen_hal;

    if (!platform) {
        ALOGE("%s: platform data is NULL", __func__);
        return;
    }
    my_data = (struct platform_data *)platform;
    if (!my_data->listen_hal) {
        ALOGE("%s: platform listen_hal data is NULL", __func__);
        return;
    }
    listen_hal = my_data->listen_hal;

    if (event_type == AUDIO_DEVICE_IN_ACTIVE)
        listen_hal->tx_concurrency_active++;
    else if ((event_type == AUDIO_DEVICE_IN_INACTIVE) &&
        (listen_hal->tx_concurrency_active > 0))
        listen_hal->tx_concurrency_active--;
    else if (listen_hal->rx_concurrency_disabled == true) {
        if (event_type == AUDIO_STREAM_OUT_ACTIVE)
            listen_hal->rx_concurrency_active++;
        else if ((event_type == AUDIO_STREAM_OUT_INACTIVE) &&
            (listen_hal->rx_concurrency_active > 0))
            listen_hal->rx_concurrency_active--;
    }
}

int platform_listen_connect_mad
(
   void *platform,
   bool en,
   audio_listen_session_t *p_ses
)
{
#ifndef PLATFORM_APQ8084
    return 0;
#else
    /* This mixer control is only valid for CPE supported codec */
    struct platform_data *my_data = (struct platform_data *)platform;
    audio_listen_hardware_t *listen_hal = my_data->listen_hal;
    int status = 0;
    struct mixer_ctl *ctl = NULL;
    const char *mixer_ctl_name = "MAD_SEL MUX";

    ctl = mixer_get_ctl_by_name(listen_hal->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: ERROR. Could not get ctl for mixer cmd - %s",
        __func__, mixer_ctl_name);
        return -EINVAL;
    }
    if(p_ses->exec_mode == LISTEN_EXEC_MODE_CPE)
        status = mixer_ctl_set_enum_by_string(ctl, "SPE");
    else
        status = mixer_ctl_set_enum_by_string(ctl, "MSM");

    if (status)
        ALOGE("%s: ERROR. Mixer ctl set failed", __func__);

    return status;
#endif
}

