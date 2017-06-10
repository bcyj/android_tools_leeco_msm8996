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

static int set_params(struct pcm *pcm, int path)
{
     struct snd_pcm_hw_params *params;
     struct snd_pcm_sw_params *sparams;

     unsigned long periodSize, bufferSize, reqBuffSize;
     unsigned int periodTime, bufferTime;
     unsigned int requestedRate = pcm->rate;

     params = (struct snd_pcm_hw_params*) calloc(1, sizeof(struct snd_pcm_hw_params));
     if (!params) {
          fprintf(stderr, "failed to allocate ALSA hardware parameters!");
          return -ENOMEM;
     }

     param_init(params);

     param_set_mask(params, SNDRV_PCM_HW_PARAM_ACCESS,
                    (pcm->flags & PCM_MMAP)? SNDRV_PCM_ACCESS_MMAP_INTERLEAVED : SNDRV_PCM_ACCESS_RW_INTERLEAVED);
     param_set_mask(params, SNDRV_PCM_HW_PARAM_FORMAT,
                    SNDRV_PCM_FORMAT_S16_LE);
     param_set_mask(params, SNDRV_PCM_HW_PARAM_SUBFORMAT,
                    SNDRV_PCM_SUBFORMAT_STD);
     param_set_min(params, SNDRV_PCM_HW_PARAM_PERIOD_TIME, 1000);
     param_set_int(params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, 16);
     param_set_int(params, SNDRV_PCM_HW_PARAM_FRAME_BITS,
                    pcm->channels - 1 ? 32 : 16);
     param_set_int(params, SNDRV_PCM_HW_PARAM_CHANNELS,
                    pcm->channels);
     param_set_int(params, SNDRV_PCM_HW_PARAM_RATE, pcm->rate);

     param_set_hw_refine(pcm, params);

     if (param_set_hw_params(pcm, params)) {
         fprintf(stderr, "cannot set hw params");
         return -1;
     }
     if (1)
          param_dump(params);

     pcm->buffer_size = pcm_buffer_size(params);
     pcm->period_size = pcm_period_size(params);
     pcm->period_cnt = pcm->buffer_size/pcm->period_size;
     if (1) {
        fprintf (stderr,"period_size (%d)", pcm->period_size);
        fprintf (stderr," buffer_size (%d)", pcm->buffer_size);
        fprintf (stderr," period_cnt  (%d)\n", pcm->period_cnt);
     }
     sparams = (struct snd_pcm_sw_params*) calloc(1, sizeof(struct snd_pcm_sw_params));
     if (!sparams) {
         fprintf(stderr, "failed to allocate ALSA software parameters!\n");
         return -ENOMEM;
     }
    sparams->tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
    sparams->period_step = 1;
    sparams->avail_min = (pcm->flags & PCM_MONO) ? pcm->period_size/2 : pcm->period_size/4;

    /* start after at least two periods are prefilled */
    if (path == PATH_RX)
	sparams->start_threshold = (pcm->flags & PCM_MONO) ? pcm->period_size : pcm->period_size/2;
    else
	sparams->start_threshold = 1;
    sparams->stop_threshold = (pcm->flags & PCM_MONO) ? pcm->buffer_size/2 : pcm->buffer_size/4;
    sparams->xfer_align = (pcm->flags & PCM_MONO) ? pcm->period_size/2 : pcm->period_size/4; /* needed for old kernels */
    sparams->silence_size = 0;
    sparams->silence_threshold = 0;

    if (param_set_sw_params(pcm, sparams)) {
         fprintf(stderr, "cannot set sw params");
         return -1;
    }
    if (1) {
        fprintf (stderr,"avail_min (%ul)\n", sparams->avail_min);
        fprintf (stderr,"start_threshold (%ul)\n", sparams->start_threshold);
        fprintf (stderr,"stop_threshold (%ul)\n", sparams->stop_threshold);
        fprintf (stderr,"xfer_align (%ul)\n", sparams->xfer_align);
    }
    return 0;

}

static int play_loopback(void* ctxt)
{
	struct pcm *pcm;
	unsigned flags;

	printf("%s\n",__func__);
	flags = PCM_OUT | PCM_MONO;

	// We should get the sub-device dynamically from /proc/asound/pcm.
#ifndef ANDROID
	pcm = pcm_open(flags, "hw:0,4");
#else
	char temp[20];
	strlcpy(temp, "hw:0,5", sizeof(temp));
	if (g_playback_device >= 0)
		snprintf(temp, sizeof(temp), "hw:0,%d",g_playback_device);
	printf("%s device %s\n",__func__, temp);
	pcm = pcm_open(flags, temp);
#endif

	if (!pcm_ready(pcm)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		pcm_close(pcm);
		return -EBADFD;
	}

	pcm->channels = 1;
	pcm->rate = 48000;
	pcm->flags = flags;

	if (set_params(pcm, PATH_RX)) {
		DALSYS_Log_Err("params setting failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	if (pcm_prepare(pcm)) {
		DALSYS_Log_Err("pcm_prepare failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	if (ioctl(pcm->fd, SNDRV_PCM_IOCTL_START)) {
		DALSYS_Log_Err("%s: start ioctl for hostless mode playback failed\n", __func__);
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

	printf("%s\n",__func__);
	g_loopback_run = 1;
	flags = PCM_IN | PCM_MONO;

	// We should get the sub-device dynamically from /proc/asound/pcm.
#ifndef ANDROID
	pcm = pcm_open(flags, "hw:0,4");
#else
	char temp[20];
	strlcpy(temp, "hw:0,5", sizeof(temp));
	if (g_capture_device >= 0)
		snprintf(temp, sizeof(temp), "hw:0,%d",g_capture_device);
	printf("%s device %s\n",__func__, temp);
	pcm = pcm_open(flags, temp);
#endif

	if (!pcm_ready(pcm)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		pcm_close(pcm);
		return -EBADFD;
	}

	pcm->channels = 1;
	pcm->rate = 48000;
	pcm->flags = flags;

	if (set_params(pcm, PATH_TX)) {
		DALSYS_Log_Err("params setting failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	if (pcm_prepare(pcm)) {
		DALSYS_Log_Err("pcm_prepare failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	if (ioctl(pcm->fd, SNDRV_PCM_IOCTL_START)) {
		DALSYS_Log_Err("%s: start ioctl for hostless mode record failed\n", __func__);
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
	char temp[20];

	strlcpy(temp, "hw:0,5", sizeof(temp));
	if (fm_playback >= 0)
		snprintf(temp, sizeof(temp), "hw:0,%d",fm_playback);
	printf("%s pcm device %s\n",__func__, temp);
	flags = PCM_OUT | DEBUG_ON;

	// We should get the sub-device dynamically from /proc/asound/pcm.
	pcm = pcm_open(flags, temp);

	if (!pcm_ready(pcm)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		pcm_close(pcm);
		return -EBADFD;
	}

	pcm->channels = 2;
	pcm->rate = 48000;
	pcm->flags = flags;

	if (set_params(pcm, PATH_RX)) {
		DALSYS_Log_Err("params setting failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	if (pcm_prepare(pcm)) {
		DALSYS_Log_Err("pcm_prepare failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	if (ioctl(pcm->fd, SNDRV_PCM_IOCTL_START)) {
		DALSYS_Log_Err("%s: start ioctl for hostless mode playback failed\n", __func__);
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
	char temp[20];

	strlcpy(temp, "hw:0,6", sizeof(temp));
	if (fm_capture >= 0)
		snprintf(temp, sizeof(temp), "hw:0,%d",fm_capture);
	printf("%s pcm device %s\n",__func__, temp);
	g_fm_run = 1;
	flags = PCM_IN | DEBUG_ON;

	// We should get the sub-device dynamically from /proc/asound/pcm.
	pcm = pcm_open(flags, temp);

	if (!pcm_ready(pcm)) {
		DALSYS_Log_Err("pcm_ready() failed\n");
		pcm_close(pcm);
		return -EBADFD;
	}

	pcm->channels = 2;
	pcm->rate = 48000;
	pcm->flags = flags;

	if (set_params(pcm, PATH_TX)) {
		DALSYS_Log_Err("params setting failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	if (pcm_prepare(pcm)) {
		DALSYS_Log_Err("pcm_prepare failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	if (ioctl(pcm->fd, SNDRV_PCM_IOCTL_START)) {
		DALSYS_Log_Err("%s: start ioctl for hostless mode record failed\n", __func__);
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

	m = mixer_open("/dev/snd/controlC0");
	if (!m){
		DALSYS_Log_Err("mixer_open() failed\n");
		return -EINVAL;
	}

	if (!enable)
		g_loopback_run = 0;

	if (afe) {
		ctl = get_ctl(m, "SLIMBUS_0_RX Port Mixer SLIM_0_TX");
		if (!ctl) {
			mixer_close(m);
			return -EINVAL;
		}

		if (enable)
			ret = mixer_ctl_set(ctl, 1);
		else {
			ret = mixer_ctl_set(ctl, 0);
		}
	}

	mixer_close(m);

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
		const char *device, unsigned *running)
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

	if (channels == 1)
		flags |= PCM_MONO;
	else
		flags |= PCM_STEREO;

	pcm = pcm_open(flags, device);
	if (!pcm_ready(pcm)) {
		pcm_close(pcm);
		goto fail;
	}
	pcm->channels = channels;
	pcm->rate = rate;
	pcm->flags = flags;
	if (set_params(pcm, PATH_TX)) {
	        fprintf(stderr, "params setting failed\n");
		pcm_close(pcm);
		return -EINVAL;
	}

	bufsize = pcm->period_size;
	if (pcm_prepare(pcm)) {
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
	fprintf(stderr, "pcm error: %s\n", pcm_error(pcm));
	return -1;
}

int audio_ftm_hw_rec_wav(const char *fg, const char *device, int rate, int ch, const char *fn, unsigned *running)
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
	flag = PCM_NMMAP;
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
		unsigned flags = pDevCtxt->read_write_flag | PCM_NMMAP;

		if (pDevCtxt->numChannels == 1)
			flags |= PCM_MONO;
		else
			flags |= PCM_STEREO;

		flags |= DEBUG_ON;
		if (pDevCtxt->playbackdevice < 0) {
			pDevCtxt->pcm = pcm_open(flags, "hw:0,0");
		} else {
			char temp[10];
			snprintf(temp, sizeof(temp), "hw:0,%d",
			pDevCtxt->playbackdevice);
			printf("\n%s: Device Name is %s", __func__, temp);
			pDevCtxt->pcm = pcm_open(flags, temp);
		}
		if (pDevCtxt->pcm < 0) {
			printf("\n %s: error opening device %s", __func__, strerror(errno));
			return AUDIO_FTM_ERROR;
		}
		g_pcm = pDevCtxt->pcm;

		if (!pcm_ready(pDevCtxt->pcm)) {
			pcm_close(pDevCtxt->pcm);
			DALSYS_Log_Err("pcm_ready(0x%p) failed\n",pDevCtxt->pcm);
			return AUDIO_FTM_ERROR;
		}
		pDevCtxt->pcm->channels = pDevCtxt->numChannels;
		pDevCtxt->pcm->rate = pDevCtxt->sampleRate;
		pDevCtxt->pcm->flags = flags;
		if (set_params(pDevCtxt->pcm, PATH_RX)) {
			DALSYS_Log_Err("params setting failed\n");
			pcm_close(pDevCtxt->pcm);
			return AUDIO_FTM_ERROR;
		}

		// DTMF tone generation will be mono or stereo depending on channels used.
		config.channel_count = pDevCtxt->numChannels;
		config.sample_rate = pDevCtxt->sampleRate;
		config.bits = pDevCtxt->bitWidth;
		config.buffer_size = pDevCtxt->pcm->period_size;
		pDevCtxt->rx_buf_size = config.buffer_size;

		if (pcm_prepare(pDevCtxt->pcm)) {
			DALSYS_Log_Err("pcm_prepare failed\n");
			pcm_close(pDevCtxt->pcm);
			return AUDIO_FTM_ERROR;
		}
	}

	if ((pDevCtxt->read_write_flag == PCM_IN) && (pDevCtxt->bLoopbackCase != TRUE))
	{
		unsigned flags = PCM_IN | PCM_NMMAP | DEBUG_ON;
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
