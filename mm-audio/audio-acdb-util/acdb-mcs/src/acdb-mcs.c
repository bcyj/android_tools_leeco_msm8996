/*
** Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
** Qualcomm Technologies Proprietary and Confidential.
**
** Copyright 2011, The Android Open Source Project
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of The Android Open Source Project nor the names of
**       its contributors may be used to endorse or promote products derived
**       from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY The Android Open Source Project ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL The Android Open Source Project BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
** DAMAGE.
*/

#define LOG_TAG "ACDB-MCS"
#define LOG_NDDEBUG 0

#include <utils/Log.h>
#ifdef _TINY_ALSA_LIB_
#include <tinyalsa/asoundlib.h>
#include "mcs-tinyalsa-wrapper.h"
#else
#include <alsa_audio.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <linux/msm_audio.h>
#include "acph.h"
#include "acdb-loader.h"
#include "mcs-route-ctrl.h"


#define PCM_PLAY_PERIOD_SIZE 1024
#define PCM_PLAY_PERIOD_COUNT 8
#define PCM_REC_PERIOD_SIZE 1024
#define PCM_REC_PERIOD_COUNT 4
#define SND_CARD_NUM 0

struct mcs_pb_ctxt {
    unsigned int cur_state;
    struct ACPH_MCS_CMD_PLAY_req cur_param;
    pthread_t thread_hdl;
    FILE    *file_hdl;
    struct pcm *pcm_hdl;
    int pcm_id;
};

struct mcs_rec_ctxt {
    unsigned int cur_state;
    struct ACPH_MCS_CMD_REC_req cur_param;
    pthread_t thread_hdl;
    FILE    *file_hdl;
    struct pcm *pcm_hdl;
    int pcm_id;
};


struct acdb_mcs{
    struct mcs_pb_ctxt* pb_ctxt;
    struct mcs_rec_ctxt* rec_ctxt;
    void* route_ctrl_hdl;

    /* MUTEX */
    pthread_mutex_t lock;
};

static struct acdb_mcs *mcs_info;

enum {
    MCS_STATE_IDLE,
    MCS_STATE_RUNNING,
    MCS_STATE_STOPPING,
};

enum {
    MCS_START,
    MCS_STOP,
};

static int check_param(struct pcm_params *params, unsigned int param,
                    unsigned int value, char *param_name,
                    char *param_unit)
{
    unsigned int min;
    unsigned int max;
    int is_within_bounds = 1;

    min = pcm_params_get_min(params, param);
    if (value < min) {
        ALOGE("%s is %u%s, device only supports >= %u%s\n", param_name, value,
                param_unit, min, param_unit);
        is_within_bounds = 0;
    }

    max = pcm_params_get_max(params, param);
    if (value > max) {
        ALOGE("%s is %u%s, device only supports <= %u%s\n", param_name, value,
                param_unit, max, param_unit);
        is_within_bounds = 0;
    }

    return is_within_bounds;
}

static int sample_is_playable(unsigned int card, unsigned int device,
                        unsigned int channels, unsigned int rate,
                        unsigned int bits, unsigned int period_size,
                        unsigned int period_count)
{
    struct pcm_params *params;
    int can_play;

    params = pcm_params_get(card, device, PCM_OUT);
    if (params == NULL) {
        ALOGE("%s: Unable to open PCM device %u.\n", __func__, device);
        return 0;
    }

    can_play = check_param(params, PCM_PARAM_RATE, rate, "Sample rate", "Hz");
    can_play &= check_param(params, PCM_PARAM_CHANNELS, channels, "Sample",
                            " channels");
    can_play &= check_param(params, PCM_PARAM_SAMPLE_BITS, bits, "Bitrate",
                            " bits");
    can_play &= check_param(params, PCM_PARAM_PERIOD_SIZE, period_size,
                            "Period size", "Hz");
    can_play &= check_param(params, PCM_PARAM_PERIODS, period_count,
                            "Period count", "Hz");

    pcm_params_free(params);

    return can_play;
}

static void mcs_play_close(struct mcs_pb_ctxt * ctxt)
{
    pcm_close(ctxt->pcm_hdl);
    fclose(ctxt->file_hdl);
}

static int mcs_play_prepare(struct mcs_pb_ctxt * ctxt)
{
    struct pcm_config config;

    ctxt->file_hdl = fopen(ctxt->cur_param.fileName, "rb");
    if (!ctxt->file_hdl) {
        ALOGE("%s: Unable to open file '%s' (%d)\n", __func__,
                ctxt->cur_param.fileName, errno);
        return errno;
    }
    config.channels = ctxt->cur_param.no_of_channels;
    config.rate = ctxt->cur_param.sample_rate;
    config.period_size = PCM_PLAY_PERIOD_SIZE;
    config.period_count = PCM_PLAY_PERIOD_COUNT;
    if (ctxt->cur_param.bit_width == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (ctxt->cur_param.bit_width == 24)
        config.format = PCM_FORMAT_S24_LE;
    else if (ctxt->cur_param.bit_width == 16)
        config.format = PCM_FORMAT_S16_LE;
    else
        return -EINVAL;

    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    if (!sample_is_playable(SND_CARD_NUM, ctxt->pcm_id,
                            config.channels, config.rate,
                            ctxt->cur_param.bit_width,
                            config.period_size,
                            config.period_count)) {
        fclose(ctxt->file_hdl);
        return -EINVAL;
    }

    ctxt->pcm_hdl = pcm_open(SND_CARD_NUM, ctxt->pcm_id, PCM_OUT, &config);
    if (!ctxt->pcm_hdl || !pcm_is_ready(ctxt->pcm_hdl)) {
        ALOGE("Unable to open PCM device %u (%s)\n",
                ctxt->pcm_id, pcm_get_error(ctxt->pcm_hdl));
        fclose(ctxt->file_hdl);
        return -ENODEV;
    }
    return 0;
}

static void mcs_play_raw_sample(struct mcs_pb_ctxt * ctxt)
{
    char *buffer;
    unsigned int size;
    unsigned int num_read;
    unsigned int total_bytes_to_play = 0;
    unsigned int bytes_count = 0;
    unsigned int replay = 0;

    if (ctxt->cur_param.play_dur_in_sec > 0) {
        total_bytes_to_play = ctxt->cur_param.play_dur_in_sec *
                ctxt->cur_param.sample_rate *
                pcm_frames_to_bytes(ctxt->pcm_hdl, 1);
        replay = 1;
    } else if (ctxt->cur_param.play_dur_in_sec < 0) {
        replay = 1;
    }

    size = pcm_frames_to_bytes(ctxt->pcm_hdl,
                                pcm_get_buffer_size(ctxt->pcm_hdl));
    buffer = malloc(size);
    if (!buffer) {
        ALOGE("%s: Unable to allocate %d bytes\n", __func__, size);
        goto err;
    }

    ALOGD("%s: Playing sample: %u ch, %u hz, %u bit\n", __func__,
            ctxt->cur_param.no_of_channels,
            ctxt->cur_param.sample_rate,
            ctxt->cur_param.bit_width);

    do {
        num_read = fread(buffer, 1, size, ctxt->file_hdl);
        if (num_read > 0) {
            if (pcm_write(ctxt->pcm_hdl, buffer, num_read)) {
                ALOGE ("Error playing sample\n");
                break;
            }
            bytes_count += num_read;
            if ((ctxt->cur_param.play_dur_in_sec > 0) &&
                (bytes_count >= total_bytes_to_play)) {
                ALOGD ("%d pcm data played\n", bytes_count);
                break;
            }
        } else {
            if ((feof(ctxt->file_hdl)) && (replay == 1)) {
                pcm_stop(ctxt->pcm_hdl);
                clearerr(ctxt->file_hdl);
                rewind(ctxt->file_hdl);
                num_read = size;
                pcm_start(ctxt->pcm_hdl);
            }
        }
    } while ((ctxt->cur_state == MCS_STATE_RUNNING) && num_read > 0);

err:
    ALOGD("%s: exiting thread", __func__);

    free(buffer);
    pthread_exit(0);
}

static int32_t process_playback_request(uint8_t cmd,
                                        struct mcs_pb_ctxt * ctxt,
                                        void * route_ctrl_hdl,
                                        struct ACPH_MCS_CMD_PLAY_req * param)
{
    int ret = 0;
    switch(cmd) {
    case MCS_START:
        if (ctxt->cur_state == MCS_STATE_IDLE) {
            memcpy(&ctxt->cur_param, param, sizeof(ctxt->cur_param));

            /* clear calibration data */
            acdb_loader_send_audio_cal_v2(ctxt->cur_param.rx_device_id,
                MSM_SNDDEV_CAP_RX, acdb_loader_get_default_app_type(),
                ctxt->cur_param.sample_rate);

            /* Setting up the device path */
            ret = mcs_route_ctrl_set_path(route_ctrl_hdl,
                                            ctxt->cur_param.rx_device_id,
                                            1, &ctxt->pcm_id);

            if (ret != 0) {
                ALOGE("%s: Failed to set the route %d.", __func__,
                        param->rx_device_id);
                break;
            }
            /* Prepare the playback session */
            ret = mcs_play_prepare(ctxt);

            if (ret == 0) {
                /* create a new thread to start the rendering*/
                ret = pthread_create(&ctxt->thread_hdl, NULL,
                                   (void *)mcs_play_raw_sample, (void *)ctxt);
                if (ret == 0) {
                    ctxt->cur_state = MCS_STATE_RUNNING;
                }
            }
        }
        break;

    case MCS_STOP:
        if (ctxt->cur_state != MCS_STATE_IDLE) {
            ctxt->cur_state = MCS_STATE_STOPPING;
            ret = pthread_join(ctxt->thread_hdl, NULL);
            mcs_play_close(ctxt);
            mcs_route_ctrl_set_path(route_ctrl_hdl,
                                    ctxt->cur_param.rx_device_id,
                                    0, &ctxt->pcm_id);
            ctxt->cur_state = MCS_STATE_IDLE;
        }
        break;

    default:
        ALOGE("%s:Invalid command %d", __func__, cmd);
        break;
    }

    return ret;
}


static int mcs_record_prepare(struct mcs_rec_ctxt * ctxt)
{
    struct pcm_config config;

    if (ctxt->cur_param.write_to_file == 1) {
        ctxt->file_hdl = fopen(ctxt->cur_param.fileName, "wb");
        if (!ctxt->file_hdl) {
            ALOGE("%s: Unable to create file '%s' (%d)\n", __func__,
                    ctxt->cur_param.fileName, errno);
            return -errno;
        }
        ALOGD("%s: open recording file %s", ctxt->cur_param.fileName);
    }
    config.channels = ctxt->cur_param.no_of_channels;
    config.rate = ctxt->cur_param.sample_rate;
    config.period_size = PCM_REC_PERIOD_SIZE;
    config.period_count = PCM_REC_PERIOD_COUNT;

    if (ctxt->cur_param.bit_width == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (ctxt->cur_param.bit_width == 24)
        config.format = PCM_FORMAT_S24_LE;
    else if (ctxt->cur_param.bit_width == 16)
        config.format = PCM_FORMAT_S16_LE;
    else
        return -EINVAL;

    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    ctxt->pcm_hdl = pcm_open(SND_CARD_NUM, ctxt->pcm_id, PCM_IN, &config);
    if (!ctxt->pcm_hdl || !pcm_is_ready(ctxt->pcm_hdl)) {
        ALOGE("Unable to open PCM device (%s)\n",
                pcm_get_error(ctxt->pcm_hdl));
        if (ctxt->cur_param.write_to_file == 1) {
            fclose(ctxt->file_hdl);
        }
        return -ENODEV;
    }

    return 0;
}

static void mcs_record_raw_sample(struct mcs_rec_ctxt * ctxt)
{
    char *buffer;
    unsigned int size;
    unsigned int bytes_count = 0;
    unsigned int total_bytes_to_record = 0;



    if (ctxt->cur_param.rec_dur_in_sec > 0) {
        total_bytes_to_record = ctxt->cur_param.rec_dur_in_sec *
                                ctxt->cur_param.sample_rate *
                                pcm_frames_to_bytes(ctxt->pcm_hdl, 1);
    }

    size = pcm_frames_to_bytes(ctxt->pcm_hdl,
                                pcm_get_buffer_size(ctxt->pcm_hdl));
    buffer = malloc(size);
    if (!buffer) {
        ALOGE("Unable to allocate %d bytes\n", size);
        goto err;
    }

    ALOGD("%s:Capturing sample: %u ch, %u hz, %u bit\n", __func__,
            ctxt->cur_param.no_of_channels, ctxt->cur_param.sample_rate,
            ctxt->cur_param.bit_width);

    while ((ctxt->cur_state == MCS_STATE_RUNNING) &&
           !pcm_read(ctxt->pcm_hdl, buffer, size)) {
        if (ctxt->cur_param.write_to_file == 1) {
            if (fwrite(buffer, 1, size, ctxt->file_hdl) != size) {
                ALOGE("Error capturing sample\n");
                break;
            }
        }
        bytes_count += size;
        if ((ctxt->cur_param.rec_dur_in_sec > 0) &&
            (bytes_count >= total_bytes_to_record)) {
            ALOGD ("%d pcm data recorded\n", bytes_count);
            break;
        }

    }

err:
    ALOGD("%s: exiting thread", __func__);
    free(buffer);
    pthread_exit(0);
}

static void mcs_record_close(struct mcs_rec_ctxt * ctxt)
{
    pcm_close(ctxt->pcm_hdl);
    if (ctxt->cur_param.write_to_file == 1) {
        fclose(ctxt->file_hdl);
    }
}

static int32_t process_recording_request(uint8_t cmd,
                                        struct mcs_rec_ctxt * ctxt,
                                        void * route_ctrl_hdl,
                                        struct ACPH_MCS_CMD_REC_req * param)
{
    int ret = 0;
    switch(cmd) {
    case MCS_START:
        if (ctxt->cur_state == MCS_STATE_IDLE) {
            memcpy(&ctxt->cur_param, param, sizeof(ctxt->cur_param));

            /* clear calibration data */
            acdb_loader_send_audio_cal_v2(ctxt->cur_param.tx_device_id,
                MSM_SNDDEV_CAP_TX, acdb_loader_get_default_app_type(),
                ctxt->cur_param.sample_rate);

            /* Setting up the device path */
            ret = mcs_route_ctrl_set_path(route_ctrl_hdl,
                                            ctxt->cur_param.tx_device_id,
                                            1, &ctxt->pcm_id);
            if (ret != 0) {
                ALOGE("%s: Failed to set the route %d.", __func__,
                        param->tx_device_id);
                break;
            }
            /* Prepare the recording session */
            ret = mcs_record_prepare(ctxt);

            if (ret == 0) {
                /* Start the caputring thread */
                ret = pthread_create(&ctxt->thread_hdl, NULL,
                                   (void *)mcs_record_raw_sample,
                                   (void *)ctxt);

                if (ret == 0) {
                    ctxt->cur_state = MCS_STATE_RUNNING;
                }
            }
        }
        break;

    case MCS_STOP:
        if (ctxt->cur_state != MCS_STATE_IDLE) {
            ctxt->cur_state = MCS_STATE_STOPPING;
            ret = pthread_join(ctxt->thread_hdl, NULL);
            mcs_record_close(ctxt);
            mcs_route_ctrl_set_path(route_ctrl_hdl,
                                    ctxt->cur_param.tx_device_id,
                                    0, &ctxt->pcm_id);
            ctxt->cur_state = MCS_STATE_IDLE;
        }
        break;

    default:
        ALOGE("%s:Invalid command %d", __func__, cmd);
        break;
    }

    return ret;
}

int32_t acdb_mcs_callback(uint16_t cmd, uint8_t *req_buf_ptr,
                            uint32_t req_buf_len, uint8_t *resp_buf_ptr,
                            uint32_t resp_buf_len,
                            uint32_t *resp_buf_bytes_filled)
{
    int ret = 0;

    pthread_mutex_lock(&mcs_info->lock);
    switch(cmd) {
    case ACPH_MCS_CMD_REC:
        ret = process_recording_request(MCS_START, mcs_info->rec_ctxt,
                                        mcs_info->route_ctrl_hdl,
                                        (ACPH_MCS_CMD_REC_req *) req_buf_ptr);
        break;

    case ACPH_MCS_CMD_PLAY:
        ret = process_playback_request(MCS_START, mcs_info->pb_ctxt,
                                        mcs_info->route_ctrl_hdl,
                                        (ACPH_MCS_CMD_PLAY_req *) req_buf_ptr);
        break;

    case ACPH_MCS_CMD_PLAY_REC:
        {
            ACPH_MCS_CMD_PLAY_REC_req *pdata = (ACPH_MCS_CMD_PLAY_REC_req *) req_buf_ptr;

            ret = process_playback_request(MCS_START, mcs_info->pb_ctxt,
                                            mcs_info->route_ctrl_hdl,
                                            &pdata->play_session);
            if (ret == 0) {
                ret = process_recording_request(MCS_START,
                                                 mcs_info->rec_ctxt,
                                                 mcs_info->route_ctrl_hdl,
                                                 &pdata->rec_session);
                if (ret != 0) {
                    ALOGE("Stop the playback session\n");
                    process_playback_request(MCS_STOP,
                                            mcs_info->pb_ctxt,
                                            mcs_info->route_ctrl_hdl,
                                            NULL);
                }
            }
        }
        break;

    case ACPH_MCS_CMD_STOP:
        ret = process_playback_request(MCS_STOP, mcs_info->pb_ctxt,
                                        mcs_info->route_ctrl_hdl, NULL);
        if (ret == 0)
            ret = process_recording_request(MCS_STOP, mcs_info->rec_ctxt,
                                            mcs_info->route_ctrl_hdl, NULL);
        break;

    default:
        ALOGE("%s: invalid command ID from ACPH: 0x%x\n", __func__, cmd);
        ret = ACPH_ERR_INVALID_COMMAND;
        break;
    }

    pthread_mutex_unlock(&mcs_info->lock);

    if (ret == 0)
        ret = ACPH_SUCCESS;
    else
        ret = ACPH_FAILURE;

    return ret;
}

int acdb_mcs_init(void)
{
    int ret;

    /* Initialize the MCS resources */
    mcs_info = calloc(1, sizeof(struct acdb_mcs));
    if (mcs_info == NULL) {
        ALOGE("%s: memory allocation failed.", __func__);
        ret = -ENOMEM;
        goto err_ret;
    }
    mcs_info->pb_ctxt = calloc(1, sizeof(struct mcs_pb_ctxt));
    if (mcs_info->pb_ctxt == NULL) {
        ALOGE("%s: memory allocation failed.", __func__);
        free(mcs_info);
        ret = -ENOMEM;
        goto err_ret;
    }
    mcs_info->pb_ctxt->cur_state = MCS_STATE_IDLE;

    mcs_info->rec_ctxt = calloc(1, sizeof(struct mcs_rec_ctxt));
    if (mcs_info->rec_ctxt == NULL) {
        ALOGE("%s: memory allocation failed.", __func__);
        ret = -ENOMEM;
        goto err_ret;
    }

    mcs_info->rec_ctxt->cur_state = MCS_STATE_IDLE;


    mcs_info->route_ctrl_hdl = mcs_route_ctrl_init(MIXER_CONTROL_FILE_PATH,
                                                    SND_CARD_NUM);

    if (mcs_info->route_ctrl_hdl == NULL) {
        ALOGE("%s: MCS routing control initialization failed.", __func__);
        ret = -ENODEV;
        goto err_ret;
    }

    pthread_mutex_init(&mcs_info->lock, NULL);

    /* Register call back */
    ret = acph_register_command(ACPH_MEDIA_CONTROL_REG_SERVICEID, acdb_mcs_callback);
    if (ret < 0) {
        ALOGE("ACDB MSC -> acph register failed error = %d\n", ret);
        goto err_ret;
    }
    return 0;

err_ret:
    if (mcs_info != NULL) {
        free(mcs_info->rec_ctxt);
        free(mcs_info->pb_ctxt);
        free (mcs_info);
    }
    return ret;
}
