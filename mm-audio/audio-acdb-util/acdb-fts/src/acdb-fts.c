/*
 * Copyright (c) 2014 QUALCOMM Technologies, Inc. All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 *
 */

#define LOG_TAG "ACDB-FTS"

#define LOG_NDDEBUG 0

#include <utils/Log.h>
#include <stdio.h>
#include <pthread.h>
#include "acph.h"

#define MAX_FILES	100

struct acdb_fts {
    FILE *file_handles[MAX_FILES];
    pthread_mutex_t lock;
};

static struct acdb_fts *fts_info;


static int process_open_request(struct  ACPH_FTS_CMD_OPEN_FILE_req *open_req,
                         struct ACPH_FTS_CMD_OPEN_FILE_resp *resp,
                         uint32_t *resp_len,
                         uint32_t *resp_buf_bytes_filled)
{
    int i = 0;

    for (; i < MAX_FILES; i++)
        if (fts_info->file_handles[i] == NULL)
            break;

    if (i >= MAX_FILES) {
        ALOGE("%s: Max number of files reached %d",
             __func__, MAX_FILES);
        return ACPH_FAILURE;
    }

    fts_info->file_handles[i] = fopen(open_req->fileName, "wb");
    if (!fts_info->file_handles[i]) {
        ALOGE("%s: failed to open file %s (errno = %d).",
             __func__, open_req->fileName, errno);
        return ACPH_ERR_INVALID_FILE_PATH;
    }

    if (*resp_len < sizeof(struct ACPH_FTS_CMD_OPEN_FILE_resp)) {
        ALOGE("%s: can't update the response buffer. len = %d", __func__, *resp_len);
        fclose(fts_info->file_handles[i]);
        fts_info->file_handles[i] = NULL;
        return ACPH_FAILURE;
    }
    ALOGD("%s: handle %p", __func__, fts_info->file_handles[i]);

    resp->file_handle_id = i;
    *resp_buf_bytes_filled = sizeof(struct ACPH_FTS_CMD_OPEN_FILE_resp);
    return ACPH_SUCCESS;
}

static int process_write_request(struct  ACPH_FTS_CMD_WRITE_FILE_DATA_req *write_req,
                                uint32_t tot_msg_len)
{
    uint8_t * data_ptr;

    if ((write_req->file_handle_id >= MAX_FILES) ||
        (write_req->file_handle_id < 0)) {
        ALOGE("%s: invalid file handle %p.", __func__, write_req->file_handle_id);
        return ACPH_FAILURE;
    }

    if (fts_info->file_handles[write_req->file_handle_id] == NULL) {
        ALOGE("%s: invalid file handle %p:%p.", __func__, write_req->file_handle_id,
             fts_info->file_handles[write_req->file_handle_id]);
        return ACPH_FAILURE;
    }

    if (tot_msg_len !=
        (write_req->data_length + sizeof(struct  ACPH_FTS_CMD_WRITE_FILE_DATA_req))) {
        ALOGE("%s: invalid date lenth. Total length = %d, Data length = %d", __func__,
             tot_msg_len,
             write_req->data_length);
        return ACPH_FAILURE;
    }

    data_ptr = (uint8_t*)write_req;
    data_ptr += sizeof(struct  ACPH_FTS_CMD_WRITE_FILE_DATA_req);

    if (fwrite(data_ptr, 1, write_req->data_length,
        fts_info->file_handles[write_req->file_handle_id])
        != write_req->data_length) {
        ALOGE("%s: fwrite failure (errno %d).", __func__, errno);
        return ACPH_FAILURE;
    }

    ALOGD("%s: write %d bytes of data.", __func__, write_req->data_length);

    return ACPH_SUCCESS;
}

static int process_close_request(struct  ACPH_FTS_CMD_CLOSE_FILE_req *close_req)
{
    if ((close_req->file_handle_id >= MAX_FILES) ||
        (close_req->file_handle_id < 0)) {
        ALOGE("%s: invalid file handle %p.", __func__, close_req->file_handle_id);
        return ACPH_FAILURE;
    }
    if (fts_info->file_handles[close_req->file_handle_id] == NULL) {
        ALOGE("%s: invalid file handle %p:%p.",
             __func__, close_req->file_handle_id,
             fts_info->file_handles[close_req->file_handle_id]);
        return ACPH_FAILURE;
    }

    if (fclose(fts_info->file_handles[close_req->file_handle_id]) != 0) {
        ALOGE("%s: fail to close the file %p (errno %d).", __func__,
             close_req->file_handle_id,
             errno);
        return ACPH_FAILURE;
    }

    fts_info->file_handles[close_req->file_handle_id] = NULL;

    return ACPH_SUCCESS;
}

static int32_t acdb_fts_callback(uint16_t cmd,
                            uint8_t *req_buf_ptr,
                            uint32_t req_buf_len,
                            uint8_t *resp_buf_ptr,
                            uint32_t resp_buf_len,
                            uint32_t *resp_buf_bytes_filled)
{
    int ret = ACPH_SUCCESS;

    pthread_mutex_lock(&fts_info->lock);

    switch(cmd) {
    case ACPH_FTS_CMD_OPEN_FILE:
        ret = process_open_request((struct ACPH_FTS_CMD_OPEN_FILE_req *)req_buf_ptr,
                                   (struct ACPH_FTS_CMD_OPEN_FILE_resp *)resp_buf_ptr,
                                   &resp_buf_len,
                                   resp_buf_bytes_filled);
        break;
    case ACPH_FTS_CMD_WRITE_FILE_DATA:
        ret = process_write_request((struct  ACPH_FTS_CMD_WRITE_FILE_DATA_req *)req_buf_ptr,
                                    req_buf_len);
        break;
    case ACPH_FTS_CMD_CLOSE_FILE:
        ret = process_close_request((struct  ACPH_FTS_CMD_CLOSE_FILE_req *)req_buf_ptr);
        break;
    default:
        ALOGE("%s: unknown command %d.", __func__, cmd);
        ret = ACPH_FAILURE;
        break;
    };

    pthread_mutex_unlock(&fts_info->lock);
    return ret;
}

int acdb_fts_init(void)
{
    int ret;

    /* Initialize the FTS resources */
    fts_info = calloc(1, sizeof(struct acdb_fts));
    if (fts_info == NULL) {
        ALOGE("%s: memory allocation failed.", __func__);
        ret = -ENOMEM;
        goto err_ret;
    }

    pthread_mutex_init(&fts_info->lock, NULL);

    /* Register call back */
    ret = acph_register_command(ACPH_FILE_TRANSFER_REG_SERVICEID, acdb_fts_callback);
    if (ret < 0) {
        ALOGE("%s: acph register failed error = %d\n", __func__, ret);
        goto err_ret;
    }

    return 0;

err_ret:
    free (fts_info);
    return ret;
}
