/* voice_svc_client.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

//#define LOG_NDEBUG 0
//#define LOG_NDDEBUG 0

#ifdef _ANDROID_
#include <utils/Log.h>
#include <cutils/properties.h>
#include "common_log.h"
#else
#define LOGI(...)      fprintf(stdout,__VA_ARGS__)
#define LOGE(...)      fprintf(stderr,__VA_ARGS__)
#define LOGV(...)      fprintf(stderr,__VA_ARGS__)
#define LOGD(...)      fprintf(stderr,__VA_ARGS__)
#endif /* _ANDROID_ */

#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sound/voice_svc.h>
#include "voice_svc_client.h"

#define DRIVER "/dev/voice_svc"

/* seting the size to 512 as currently IMS client uses 300 for the payload
 * size. With 512 there will be additional space for the structure fields
 * and also some expansion space for payload.
 */
#define MAX_APR_DATA_SIZE 512
#define NO_ERROR 0

enum response_loop_state
{
    VOICE_SVC_STOP = 0,
    VOICE_SVC_RUN
};

struct voice_svc_to_handle_type
{
    uint32_t svc_handle;
    char svc_name[MAX_APR_SERVICE_NAME_LEN];
};

static struct voice_svc_to_handle_type svc_to_handle_table[] =
{
    {VOICE_SVC_CVS, "CVS"},
    {VOICE_SVC_MVM, "MVM"},
};

struct voice_svc_prtd
{
    int fd;
    voice_svc_recv_response_fn cb;
    uint32_t state;
    pthread_t responseThread;
    pthread_mutex_t lock;
};

static struct voice_svc_prtd prtd;

void* voice_svc_response_loop(void* t);

static int voice_svc_write_cmd(void *req, uint32_t msg_type, uint16_t count)
{
    int ret = 0;
    struct voice_svc_write_msg *data = NULL;

    data = (struct voice_svc_write_msg *)malloc(
           sizeof(struct voice_svc_write_msg) + count);

    if (data == NULL) {
        ALOGE("%s: malloc failed", __func__);

        ret = -ENOMEM;
        goto done;
    }

    data->msg_type = msg_type;
    memcpy(((uint32_t *)data) + 1, req, count);

    ret =  write(prtd.fd, data, sizeof(struct voice_svc_write_msg) + count);

    ALOGV("%s: write returned: %d", __func__, ret);

done:
    if (data)
        free(data);

    return ret;
}

static int voice_svc_get_name(enum voice_svc svc_handle, char *svc_name)
{
    int ret = 0;
    uint32_t i = 0;
    uint32_t svc_count = sizeof(svc_to_handle_table) /
                         sizeof(struct voice_svc_to_handle_type);

    for (i = 0; i < svc_count; i++) {
        if (svc_handle == svc_to_handle_table[i].svc_handle) {
            strlcpy(svc_name, svc_to_handle_table[i].svc_name,
                    MAX_APR_SERVICE_NAME_LEN);
            ALOGV("%s: service: %s", __func__, svc_name);

            return 0;
        }
    }

    ALOGE("%s: Invalid service %d", __func__, svc_handle);
    ret = -EINVAL;

    return ret;
}

static int voice_svc_get_handle(char *svc_name, uint32_t *svc_handle)
{
    int ret = 0;
    uint32_t i = 0;
    uint32_t svc_count = sizeof(svc_to_handle_table) /
                         sizeof(struct voice_svc_to_handle_type);

    for (i = 0; i < svc_count; i++) {
        if (!strcmp(svc_name, svc_to_handle_table[i].svc_name)) {
            *svc_handle = svc_to_handle_table[i].svc_handle;
            ALOGV("%s: service handle: %d", __func__, *svc_handle);

            return 0;
        }
    }

    ALOGE("%s: Invalid service %s", __func__, svc_name);

    ret = -EINVAL;

    return ret;
}

int voice_svc_register(enum voice_svc svc, uint32_t src_port,
                       uint8_t reg_flag, uint32_t *svc_handle)
{
    int ret = 0;
    uint32_t i = 0;
    struct voice_svc_register apr_reg;

    ALOGV("%s: svc: %d, src_port: %d", __func__, svc, src_port);

    pthread_mutex_lock(&prtd.lock);

    if (!prtd.state) {
        ALOGE("%s: voice_svc_client not initialized yet", __func__);

        ret = -EINVAL;
        goto done;
    }

    memset(apr_reg.svc_name, 0, MAX_APR_SERVICE_NAME_LEN);

    ret = voice_svc_get_name(svc, apr_reg.svc_name);
    if (ret) {
        ALOGE("%s: Could not get service name", __func__);

        goto done;
    }

    apr_reg.src_port = src_port;
    apr_reg.reg_flag = reg_flag;

    if ((ret = voice_svc_write_cmd(&apr_reg, MSG_REGISTER,
        sizeof(apr_reg))) < 0) {
        ALOGE("%s: Failed to register service %s; error: %d", __func__,
              apr_reg.svc_name, errno);

        goto done;
    }

    ret = voice_svc_get_handle(apr_reg.svc_name, svc_handle);
    if (ret) {
        ALOGE("%s: Could not get service handle", __func__);

        goto done;
    }

    ALOGV("%s: Registered service %s", __func__, apr_reg.svc_name);

    ret = NO_ERROR;

done:
    pthread_mutex_unlock(&prtd.lock);
    return ret;
}

int voice_svc_send_request(uint32_t svc_handle, struct voice_svc_data *svc_data)
{
    int ret = 0;
    uint32_t i = 0;
    struct voice_svc_cmd_request *apr_req = NULL;
    int size;

    ALOGV("%s", __func__);

    pthread_mutex_lock(&prtd.lock);

    if (!prtd.state) {
        ALOGE("%s: voice_svc_client not initialized yet", __func__);

        ret = -EINVAL;
        goto done;
    }

    if (svc_data == NULL) {
        ALOGE("%s: svc_data is NULL", __func__);

        ret = -EINVAL;
        goto done;
    }

    size = sizeof(struct voice_svc_cmd_request) + svc_data->payload_size;
    ALOGV("%s: payload_size: %d; struct_size: %d; total: %d", __func__,
          svc_data->payload_size, sizeof(struct voice_svc_cmd_request), size);

    apr_req = (struct voice_svc_cmd_request*)malloc(size);
    if (apr_req == NULL) {
        ALOGE("%s: malloc failed", __func__);

        ret = -ENOMEM;
        goto done;
    }

    ret = voice_svc_get_name(svc_handle, apr_req->svc_name);
    if (ret) {
        ALOGE("%s: Could not get service name", __func__);

        goto done;
    }

    apr_req->src_port = svc_data->src_port;
    apr_req->dest_port = svc_data->dest_port;
    apr_req->token = svc_data->token;
    apr_req->opcode = svc_data->opcode;
    apr_req->payload_size = svc_data->payload_size;
    memcpy(apr_req->payload, svc_data->payload, apr_req->payload_size);

    if ((ret = voice_svc_write_cmd(apr_req, MSG_REQUEST, size)) < 0) {
        ALOGE("%s: Failed to send command 0x%x; error: %d", __func__,
              svc_data->opcode, errno);

        goto done;
    }

    ret = NO_ERROR;

done:
    if (apr_req != NULL) {
        free(apr_req);
    }

    pthread_mutex_unlock(&prtd.lock);
    return ret;
}

void* voice_svc_response_loop(void* t)
{
    int ret = 0;
    struct voice_svc_cmd_response *resp;
    uint32_t *ptr = NULL;

    resp = malloc(sizeof(struct voice_svc_data) + MAX_APR_DATA_SIZE);
    if (resp == NULL) {
        ALOGE("%s: malloc failed", __func__);

        return NULL;
    }

    while (prtd.state) {
        pthread_mutex_lock(&prtd.lock);

        if (prtd.fd == -1 || !prtd.state) {
            ALOGV("%s: Driver seems to be stopped", __func__);

            pthread_mutex_unlock(&prtd.lock);
            break;
        }

        pthread_mutex_unlock(&prtd.lock);

        if ((ret = read(prtd.fd, resp, sizeof(struct voice_svc_data) +
            MAX_APR_DATA_SIZE)) <= 0) {
            ALOGV("%s: read failed %d", __func__, ret);
        } else {
            ptr = (uint32_t*)resp->payload;
            ALOGV("%s: opcode: 0x%x; size: %d; payload: 0x%x 0x%x", __func__,
                  resp->opcode, resp->payload_size, ptr[0], ptr[1]);

            pthread_mutex_lock(&prtd.lock);

            if (prtd.cb != NULL)
                prtd.cb((struct voice_svc_data*)resp);
            else
                ALOGE("%s: callback function is NULL", __func__);

            pthread_mutex_unlock(&prtd.lock);
        }
    }

    ALOGV("%s: Exiting response loop", __func__);

    free(resp);
    return NULL;
}

void voice_svc_deinit()
{
    int ret = 0;

    ALOGV("%s", __func__);

    pthread_mutex_lock(&prtd.lock);

    if (!prtd.state) {
        ALOGE("%s: voice_svc_client not initialized yet", __func__);

        pthread_mutex_unlock(&prtd.lock);
        return;
    }

    ret = close(prtd.fd);
    prtd.fd = -1;
    prtd.cb = NULL;
    prtd.state = VOICE_SVC_STOP;

    pthread_mutex_unlock(&prtd.lock);

    pthread_mutex_destroy(&prtd.lock);

    if (ret)
        ALOGE("%s: Failed to close: %d", __func__, ret);
}

int voice_svc_init(voice_svc_recv_response_fn callback)
{
    int ret = 0;

    ALOGV("%s", __func__);

    pthread_mutex_init(&prtd.lock, NULL);
    pthread_mutex_lock(&prtd.lock);

    if (prtd.state) {
        ALOGE("%s: voice_svc has already been initialized", __func__);

        ret = -EINVAL;
        goto done;
    }

    if (callback == NULL) {
        ALOGE("%s: callback is NULL", __func__);

        ret = -EINVAL;
        goto done;
    }

    prtd.cb = callback;

    prtd.fd = open(DRIVER, O_RDWR);
    if (prtd.fd == -1) {
        ALOGE("%s: open() failed: %d", __func__, errno);

        prtd.cb = NULL;
        ret = -errno;
        goto done;
    }

    prtd.state = VOICE_SVC_RUN;

    ret = pthread_create(&prtd.responseThread, NULL,
			 voice_svc_response_loop, NULL);

    if (ret) {
        ALOGE("%s: Failed to start response thread %d", __func__, ret);

        pthread_mutex_unlock(&prtd.lock);
        voice_svc_deinit();
        return ret;
    }

done:
    pthread_mutex_unlock(&prtd.lock);
    return ret;
}
