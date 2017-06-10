/*

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*/
#ifdef __cplusplus
extern "C" {
#endif

#include "audio_ftm_hw_drv.h"

#include <stdio.h>
#include "linux/msm_audio.h"
#ifdef ANDROID
/* definitions for Android logging */
#include <utils/Log.h>
#include <cutils/properties.h>
#else /* ANDROID */
#define strlcat g_strlcat
#define strlcpy g_strlcpy
#define ALOGI(...)      fprintf(stdout, __VA_ARGS__)
#define ALOGE(...)      fprintf(stderr, __VA_ARGS__)
#define ALOGV(...)      fprintf(stderr, __VA_ARGS__)
#define ALOGD(...)      fprintf(stderr, __VA_ARGS__)
#endif /* ANDROID */

static DALBOOL    g_bDriverInitialized=FALSE;   //  whether it has been inited or not
#define SOUND_CARD_NUM 0
#define DEEP_BUFFER_OUTPUT_PERIOD_SIZE 960
#define DEEP_BUFFER_OUTPUT_PERIOD_COUNT 8
#define DEFAULT_OUTPUT_SAMPLING_RATE 48000
#define LOW_LATENCY_OUTPUT_PERIOD_SIZE 240
#define LOW_LATENCY_OUTPUT_PERIOD_COUNT 2


static struct pcm_config pb_config = {
	.channels = 2,
	.rate = DEFAULT_OUTPUT_SAMPLING_RATE,
	.period_size = DEEP_BUFFER_OUTPUT_PERIOD_SIZE,
	.period_count = DEEP_BUFFER_OUTPUT_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = DEEP_BUFFER_OUTPUT_PERIOD_SIZE / 4,
	.stop_threshold = INT_MAX,
	.avail_min = DEEP_BUFFER_OUTPUT_PERIOD_SIZE / 4,
};

static struct pcm_config pcm_config_low_latency = {
	.channels = 2,
	.rate = DEFAULT_OUTPUT_SAMPLING_RATE,
	.period_size = LOW_LATENCY_OUTPUT_PERIOD_SIZE,
	.period_count = LOW_LATENCY_OUTPUT_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = LOW_LATENCY_OUTPUT_PERIOD_SIZE / 4,
	.stop_threshold = INT_MAX,
	.avail_min = LOW_LATENCY_OUTPUT_PERIOD_SIZE / 4,
};

static struct pcm_config cp_config = {
	.channels = 2,
	.period_count = 2,
	.format = PCM_FORMAT_S16_LE,
};
static struct pcm_config pcm_config_fm = {
	.channels = 2,
	.rate = 48000,
	.period_size = 256,
	.period_count = 4,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
	.avail_min = 0,
};
volatile int g_loopback_run = 0;
volatile int g_fm_run = 0;

int g_curr_rx_device = -1;
int g_curr_tx_device = -1;
int g_curr_afe_lb_device = -1;
int g_curr_adie_lb_device = -1;
int g_curr_fm_device = -1;
int g_curr_device = -1;
static int g_playback_device = -1, g_capture_device = -1;
static int fm_capture = -1, fm_playback = -1;
const char *g_curr_alsa_device_name = AUDIO_FTM_ALSA_DEVICE_MM1;

static struct pcm *g_pcm = 0;
#define AUDIO_CAPTURE_PERIOD_DURATION_MSEC 20

 size_t get_input_buffer_size(uint32_t sample_rate,
                                    int channel_count)
{
    size_t size = 0;

    size = (sample_rate * AUDIO_CAPTURE_PERIOD_DURATION_MSEC) / 1000;
    /* ToDo: should use frame_size computed based on the format and
       channel_count here. */
    size *= sizeof(short) * channel_count;

    /* make sure the size is multiple of 64 */
    size += 0x3f;
    size &= ~0x3f;

    return size;
}

static int play_loopback(void* ctxt)
{
	struct pcm *pcm;
	unsigned flags;

	printf("%s\n",__func__);
	flags = PCM_OUT;

	// We should get the sub-device dynamically from /proc/asound/pcm.
#ifndef ANDROID
	pcm = pcm_open(flags, "hw:0,4");
#else
	int device_num = 0;
	if (g_playback_device >= 0)
		device_num = g_playback_device;
	pcm_config_low_latency.rate = 48000;
	pcm_config_low_latency.channels = 1;
	pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &pcm_config_low_latency);
#endif

	if (!pcm_is_ready(pcm)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		return -EBADFD;
	}
	if (pcm_start(pcm)) {
		DALSYS_Log_Err("pcm_start failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}
	while(g_loopback_run);
	pcm_close(pcm);
	return 0;
}

static int rec_loopback(void* ctxt)
{
	struct pcm *pcm;
	unsigned flags;
	size_t bufsize;

	printf("%s\n",__func__);
	g_loopback_run = 1;
	flags = PCM_IN;

	// We should get the sub-device dynamically from /proc/asound/pcm.
#ifndef ANDROID
	pcm = pcm_open(flags, "hw:0,4");
#else
	int device_num = 0;
	if (g_capture_device >= 0)
		device_num = g_capture_device;
	cp_config.rate = 48000;
	cp_config.channels = 1;
	bufsize = get_input_buffer_size(cp_config.rate,cp_config.channels);
	cp_config.period_size = bufsize/(cp_config.channels * 2);

	pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &cp_config);
#endif

	if (!pcm_is_ready(pcm)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		return -EBADFD;
	}
	if (pcm_start(pcm)) {
		DALSYS_Log_Err("pcm_prepare failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}
	while(g_loopback_run);

	pcm_close(pcm);

	return 0;
}

static int play_fm(void* ctxt)
{
	struct pcm *pcm;
	unsigned flags;
	int device_num = 5;

	if (fm_playback >= 0)
		device_num = fm_playback;
	flags = PCM_OUT;

	// We should get the sub-device dynamically from /proc/asound/pcm.
	pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &pcm_config_fm);

	if (!pcm_is_ready(pcm)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		return -EBADFD;
	}

	if (pcm_start(pcm)) {
		DALSYS_Log_Err("pcm_prepare failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	while(g_fm_run);

	pcm_close(pcm);

	return 0;
}

static int rec_fm(void* ctxt)
{
	struct pcm *pcm;
	unsigned flags;
	int device_num = 6;
	size_t bufsize;
	if (fm_capture >= 0)
		device_num = fm_capture;
	g_fm_run = 1;
	flags = PCM_IN;

	// We should get the sub-device dynamically from /proc/asound/pcm.
	pcm = pcm_open(SOUND_CARD_NUM, device_num, flags, &pcm_config_fm);

	if (!pcm_is_ready(pcm)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		return -EBADFD;
	}

	if (pcm_start(pcm)) {
		DALSYS_Log_Err("pcm_prepare failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	while(g_fm_run);

	pcm_close(pcm);

	return 0;
}

int audio_ftm_hw_loopback_en(int enable, int afe)
{
	struct mixer *m = 0;
	struct mixer_ctl *ctl = 0;
	int ret = 0;
	pthread_t lb_play_thread;
	pthread_t lb_rec_thread;

	printf("%s(%d.%d)\n",__func__,enable,afe);

	if (enable && afe) {
		g_loopback_run = 1;

		ret = pthread_create(&lb_play_thread, NULL, play_loopback, NULL);
		if (ret != 0)
		{
			DALSYS_Log_Err("Failed to create lb play thread\n");
			return AUDIO_FTM_ERROR;
		}
		ret = pthread_create(&lb_rec_thread, NULL, rec_loopback, NULL);
		if (ret != 0)
		{
			DALSYS_Log_Err("Failed to create lb rec thread\n");
			return AUDIO_FTM_ERROR;
		}
	}

	if (!enable)
	 g_loopback_run = 0;

	return ret;
}

void audio_ftm_fm_device_set(int capture , int playback)
{
	fm_capture = capture;
	fm_playback = playback;
}
int audio_ftm_fm_hostless_en(int enable)
{
	struct mixer *m = 0;
	struct mixer_ctl *ctl = 0;
	int ret = 0;
	pthread_t fm_play_thread;
	pthread_t fm_rec_thread;

	printf("%s(%d)\n",__func__,enable);

	if (enable) {
		g_fm_run = 1;

		ret = pthread_create(&fm_play_thread, NULL, play_fm, NULL);
		if (ret != 0)
		{
			DALSYS_Log_Err("Failed to create FM play thread\n");
			return AUDIO_FTM_ERROR;
		}
		ret = pthread_create(&fm_rec_thread, NULL, rec_fm, NULL);
		if (ret != 0)
		{
			DALSYS_Log_Err("Failed to create FM rec thread\n");
			return AUDIO_FTM_ERROR;
		}
	}

	if (!enable)
		g_fm_run = 0;

	return ret;
}

AUDIO_FTM_STS_T aud_ftm_hw_init
(
    Aud_FTM_DevCtxt_T  *pCtxt,
    Aud_FTM_HW_INIT_PARAM_T *pInitParam
)
{
	AUDIO_FTM_STS_T res;
	static DALBOOL bIsDBInitialized;
	char *pDevName;

	bIsDBInitialized = FALSE;
	res= AUDIO_FTM_SUCCESS;

	g_bDriverInitialized=TRUE;

	pCtxt->sampleRate=pInitParam->rate;
	pCtxt->bitWidth=pInitParam->width;
	pCtxt->numChannels = 1;
	if (pInitParam->channel == 2) {
		pCtxt->numChannels=pInitParam->channel;
	}
	pCtxt->gain=pInitParam->gain;
	pCtxt->rx_buf_size=0;
	pCtxt->tx_buf_size=0;
	pCtxt->bLoopbackCase=pInitParam->bLoopbackCase;
	pCtxt->m_loopback_type=pInitParam->m_loopback_type;
	if(pCtxt->bLoopbackCase == TRUE)
	{
		if( (pCtxt->rx_dev_id < 0) || (pCtxt->tx_dev_id < 0) )
			return AUDIO_FTM_ERROR;
	}

	if((pInitParam->inpath !=AUDIO_FTM_IN_INVALID) && (pCtxt->tx_dev_id < 0))
		return AUDIO_FTM_ERROR;

	if((pInitParam->outpath !=AUDIO_FTM_OUT_INVALID) && (pCtxt->rx_dev_id < 0))
		return AUDIO_FTM_ERROR;

	pCtxt->m_state=AUDIO_FTM_HW_DRV_INITIALIZED;

	return AUDIO_FTM_SUCCESS;
}


#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
static struct wav_header hdr;
static int fd;

struct wav_header {
	uint32 riff_id;
	uint32 riff_sz;
	uint32 riff_fmt;
	uint32 fmt_id;
	uint32 fmt_sz;
	uint16 audio_format;
	uint16 num_channels;
	uint32 sample_rate;
	uint32 byte_rate;	  /* sample_rate * num_channels * bps / 8 */
	uint16 block_align;	  /* num_channels * bps / 8 */
	uint16 bits_per_sample;
	uint32 data_id;
	uint32 data_sz;
};

int record_file(unsigned rate, unsigned channels, int fd, unsigned count,  unsigned flags,
		int device, unsigned *running)
{
	unsigned avail, xfer, bufsize;
	int r;
	int nfds = 1;
	static int start = 0;
	struct snd_xferi x;
	long frames;
	unsigned offset = 0;
	struct pcm *pcm;
	char *data;

	flags |= PCM_IN;
	cp_config.rate = rate;
	cp_config.channels = channels;
	bufsize = get_input_buffer_size(rate,channels);
	cp_config.period_size = bufsize/(channels * 2);
	printf("%s: period size %d buf size %d\n",__func__,cp_config.period_size, bufsize);
	pcm = pcm_open(SOUND_CARD_NUM, device, flags, &cp_config);

	if (!pcm_is_ready(pcm)) {
		goto fail;
	}

	if (pcm_start(pcm)) {
		fprintf(stderr, "failed in pcm_prepare\n");
		pcm_close(pcm);
		return -1;
	}

	data = calloc(1, bufsize);
	if (!data) {
		fprintf(stderr, "could not allocate %d bytes\n", bufsize);
		pcm_close(pcm);
		return -ENOMEM;
	}

	while (!pcm_read(pcm, data, bufsize)) {
		if (write(fd, data, bufsize) != bufsize) {
			fprintf(stderr, "could not write %d bytes\n", bufsize);
			close(fd);
			free(data);
			pcm_close(pcm);
			return -1;
		}
		hdr.data_sz += bufsize;
		//printf("*running = %d\n", *running);
		if (*running == 0) {
			printf("done\n");
			break;
		}
	}

	sleep(1);
	close(fd);
	free(data);
	pcm_close(pcm);
	return hdr.data_sz;

fail:
	fprintf(stderr, "pcm error: %s\n", pcm_get_error(pcm));
	return -1;
}

int audio_ftm_hw_rec_wav(const char *fg, int device, int rate, int ch, const char *fn, unsigned *running)
{
	unsigned flag = 0;
	int i = 0;
	fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if (fd < 0) {
		fprintf(stderr, "cannot open '%s'\n", fn);
		return -EBADFD;
	}
	memset(&hdr, 0, sizeof(struct wav_header));
	hdr.riff_id = ID_RIFF;
	hdr.riff_fmt = ID_WAVE;
	hdr.fmt_id = ID_FMT;
	hdr.fmt_sz = 16;
	hdr.audio_format = 1;
	hdr.num_channels = ch;
	hdr.sample_rate = rate;
	hdr.bits_per_sample = 16;
	hdr.byte_rate = (rate * ch * hdr.bits_per_sample) / 8;
	hdr.block_align = ( hdr.bits_per_sample * ch ) / 8;
	hdr.data_id = ID_DATA;
	hdr.data_sz = 2147483648LL;
	hdr.riff_sz = hdr.data_sz + 44 - 8;

	if (write(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
		fprintf(stderr, "cannot write header\n");
		return -1;
	}
	flag = 0;
	return record_file(hdr.sample_rate, hdr.num_channels, fd, hdr.data_sz, flag, device, running);
}

AUDIO_FTM_STS_T
audio_ftm_hw_open(Aud_FTM_DevCtxt_T  *pDevCtxt)
{
	AUDIO_FTM_STS_T  ret;
	int dev_fd;
	uint16  session_id;
	struct msm_audio_config config;

	if( pDevCtxt->m_state != AUDIO_FTM_HW_DRV_INITIALIZED)
	{
		DALSYS_Log_Err("Fail: Open must be run after initialization only\n");
		return AUDIO_FTM_ERROR;
	}

	if ((pDevCtxt->read_write_flag == PCM_OUT) && (pDevCtxt->bLoopbackCase != TRUE))
	{
		unsigned flags = pDevCtxt->read_write_flag;
		pb_config.rate = pDevCtxt->sampleRate;
		pb_config.channels = pDevCtxt->numChannels;
		printf("%s: flags %d rate %d channels %d", __func__, flags, pb_config.rate, pb_config.channels);

		if (pDevCtxt->playbackdevice < 0) {
			pDevCtxt->pcm = pcm_open(SOUND_CARD_NUM, 0, flags, &pb_config);
		} else {
			pDevCtxt->pcm = pcm_open(SOUND_CARD_NUM, pDevCtxt->playbackdevice, flags, &pb_config);
		}
		if (pDevCtxt->pcm < 0) {
			printf("\n %s: error opening device %s", __func__, strerror(errno));
			return AUDIO_FTM_ERROR;
		}
		g_pcm = pDevCtxt->pcm;

		if (!pcm_is_ready(pDevCtxt->pcm)) {
			DALSYS_Log_Err("pcm_ready(0x%p) failed\n",pDevCtxt->pcm);
			return AUDIO_FTM_ERROR;
		}

		// DTMF tone generation will be mono or stereo depending on channels used.
		config.channel_count = pDevCtxt->numChannels;
		config.sample_rate = pDevCtxt->sampleRate;
		config.bits = pDevCtxt->bitWidth;
		config.buffer_size = pcm_get_buffer_size(pDevCtxt->pcm)/pb_config.period_count;
		pDevCtxt->rx_buf_size = config.buffer_size;

		if (pcm_start(pDevCtxt->pcm)) {
			DALSYS_Log_Err("pcm_prepare failed\n");
			pcm_close(pDevCtxt->pcm);
			return AUDIO_FTM_ERROR;
		}
	}

	if ((pDevCtxt->read_write_flag == PCM_IN) && (pDevCtxt->bLoopbackCase != TRUE))
	{
		unsigned flags = PCM_IN;
	}

	if(pDevCtxt->bLoopbackCase == TRUE)
	{
		if((pDevCtxt->tx_dev_id == -ENODEV) || (pDevCtxt->rx_dev_id == -ENODEV))
		{
			DALSYS_Log_Err("At least one device ID is invalid for loopback\n");
			return AUDIO_FTM_ERROR;
		}
	}

	pDevCtxt->m_state=AUDIO_FTM_HW_DRV_OPENED;

	return AUDIO_FTM_SUCCESS;
}

AUDIO_FTM_STS_T
audio_ftm_hw_close(Aud_FTM_DevCtxt_T  *pDevCtxt)
{
	AUDIO_FTM_STS_T  ret;

	ret = AUDIO_FTM_SUCCESS;

	if(pDevCtxt == NULL)  return AUDIO_FTM_ERR_INVALID_PARAM;

	pDevCtxt->m_state=AUDIO_FTM_HW_DRV_CLOSED;

	return ret;
}

AUDIO_FTM_STS_T
audio_ftm_hw_write(
    Aud_FTM_DevCtxt_T  *pDevCtxt,     /* Input: handle to hw driver */
    void *pBuf,                       /* Input: buffer pointer containing data for writing */
    uint32 nSamples                   /* Input: Samples */
)
{
	uint32  len;

	len = nSamples*(pDevCtxt->bitWidth/AUDIO_FTM_BIT_WIDTH_8) *
		(pDevCtxt->numChannels);
	int ret = 0;
	ret = pcm_write(g_pcm, pBuf, len);
	if (ret) {
	//if (pcm_write(pDevCtxt->pcm, pBuf, len)){
		printf("\npcm_write() error\n %d error %s", ret, strerror(errno));
		return AUDIO_FTM_ERROR;
	}

	return AUDIO_FTM_SUCCESS;
}

AUDIO_FTM_STS_T
audio_ftm_hw_iocontrol(
    Aud_FTM_DevCtxt_T  *pDevCtxt,     /* Input: handle to hw driver */
    uint32 dwIOCode,                  /* Input: IOControl command */
    uint8 * pBufIn,                   /* Input: buffer pointer for input parameters */
    uint32 dwLenIn,                   /* Input: parameter length */
    uint8 * pBufOut,                  /* Output: buffer pointer for outputs */
    uint32 dwLenOut,                  /* Input: expected output length */
    uint32 *pActualOutLen             /* Output: actual output length */
)
{
	struct mixer *mixer = 0;
	struct mixer_ctl *ctl = 0;
	AUDIO_FTM_STS_T   ret=AUDIO_FTM_SUCCESS;

	if((pDevCtxt->m_state == AUDIO_FTM_HW_DRV_OPENED) ||
		(pDevCtxt->m_state == AUDIO_FTM_HW_DRV_STARTED) ||
		(pDevCtxt->m_state == AUDIO_FTM_HW_DRV_STOPPED))
	{
		switch(dwIOCode)
		{
		case IOCTL_AUDIO_FTM_START:
		{
			AFEDevAudIfDirType dir;

			if(pBufIn == NULL)
			{
				DALSYS_Log_Err("pBufIn is NULL, IOCTL_AUDIO_FTM_START failed\n");
				ret=AUDIO_FTM_ERROR;
				break;
			}

			dir= *(AFEDevAudIfDirType*)pBufIn;
			printf("\n %s: capturedevice %d playback %d",
				   __func__, pDevCtxt->capturedevice, pDevCtxt->playbackdevice);
			g_playback_device = pDevCtxt->playbackdevice;
			g_capture_device = pDevCtxt->capturedevice;
			if((pDevCtxt->bLoopbackCase == TRUE)
				&& (pDevCtxt->tx_dev_id != -ENODEV)
				&& (pDevCtxt->rx_dev_id != -ENODEV))
			{
				if(pDevCtxt->m_loopback_type == AUDIO_FTM_AFE_LOOPBACK)
				{
					audio_ftm_hw_loopback_en(1, 1);
				}
				else if(pDevCtxt->m_loopback_type == AUDIO_FTM_ADIE_LOOPBACK)
				{
					audio_ftm_hw_loopback_en(1, 0);
				}
			} else if (g_curr_rx_device == AUDIO_FTM_TC_RX_FM_STEREO_OUTPUT) {
				audio_ftm_fm_hostless_en(1);
			}

			if(ret == AUDIO_FTM_SUCCESS)
				pDevCtxt->m_state = AUDIO_FTM_HW_DRV_STARTED;
		}
		break;

		case IOCTL_AUDIO_FTM_STOP:
		{
			AFEDevAudIfDirType dir;

			if(pBufIn == NULL)
			{
				DALSYS_Log_Err("pBufIn is NULL, IOCTL_AUDIO_FTM_STOP failed\n");
				ret=AUDIO_FTM_ERROR;
				break;
			}

			if (ftm_tc_devices[g_curr_device].path_type == PATH_RX) {
				DALSYS_Log_Err("\n PCM Close called");
				pcm_close(pDevCtxt->pcm);
			} else {
				DALSYS_Log_Err("\n PCM Close not called");
			}

			dir= *(AFEDevAudIfDirType*)pBufIn;

			if((pDevCtxt->bLoopbackCase == TRUE)
				&& (pDevCtxt->tx_dev_id != -ENODEV)
				&& (pDevCtxt->rx_dev_id != -ENODEV)) {
				if(pDevCtxt->m_loopback_type == AUDIO_FTM_AFE_LOOPBACK)
				{
					audio_ftm_hw_loopback_en(0, 1);
				}
				else if(pDevCtxt->m_loopback_type == AUDIO_FTM_ADIE_LOOPBACK)
				{
					audio_ftm_hw_loopback_en(0, 0);
				}
			} else if (g_curr_rx_device == AUDIO_FTM_TC_RX_FM_STEREO_OUTPUT) {
				audio_ftm_fm_hostless_en(0);
			}

			if(ret == AUDIO_FTM_SUCCESS)
				pDevCtxt->m_state = AUDIO_FTM_HW_DRV_STOPPED;
		}
		break;

		case IOCTL_AUDIO_FTM_RX_DEV_BUF_SIZE:
		{
			if(pBufOut == NULL)
			{
				DALSYS_Log_Err("pBufOut is NULL, IOCTL_AUDIO_FTM_RX_DEV_BUF_SIZE failed\n");
				ret=AUDIO_FTM_ERROR;
				break;
			}
			// buffer size will be 8192 bytes
			*((uint32 *)pBufOut) = pDevCtxt->rx_buf_size;
		}
		break;

		case IOCTL_AUDIO_FTM_TX_DEV_BUF_SIZE:
		{
			if(pBufOut == NULL)
			{
				DALSYS_Log_Err("pBufOut is NULL, IOCTL_AUDIO_FTM_TX_DEV_BUF_SIZE failed\n");
				ret=AUDIO_FTM_ERROR;
				break;
			}
			*((uint32 *)pBufOut)=pDevCtxt->tx_buf_size;
		}
		break;

		default:
			DALSYS_Log_Err("this operation is not supportted\n");
			ret= AUDIO_FTM_ERROR;
			break;
		}
	}
	else
	{
		DALSYS_Log_Err("this operation cannot be done when the driver is not in active state\n");
		ret= AUDIO_FTM_ERROR;
	}

	return ret;
}

AUDIO_FTM_STS_T
aud_ftm_hw_deinit(Aud_FTM_DevCtxt_T  *pDevCtxt)
{
	g_bDriverInitialized = FALSE;

	pDevCtxt->m_state=AUDIO_FTM_HW_DRV_UN_INITIALIZED;

	return AUDIO_FTM_SUCCESS;
}

#ifdef __cplusplus
}
#endif
