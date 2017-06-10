/*=========================================================================
*//** @file omx_alsa_utils.c
This module contains the API to configure ALSA for OMX Test App

Copyright (c) 2012 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*=====================================================================*/
/*=========================================================================
                            Edit History

$Header:
when       who     what, where, why
--------   ---     -------------------------------------------------------
=========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <omx_alsa_utils.h>
#include "OMX_Core.h"
#include <ctype.h>

#include <utils/Log.h>
#include "common_log.h"

#include "alsa_audio.h"
#include "msm8960_use_cases.h"




/*=========================================================================
Static Functions
=========================================================================*/


static int  speaker_volume(FILE *logfilefd, struct mixer *mxr, int vol);
static struct mixer_ctl *get_ctl(struct mixer *m, char *name);
static int speaker_enable(FILE *logfilefd, struct mixer *mxr, int enable, int vol);
static int msg_printf(FILE *fd, const char *format, ...);
static void set_audiodevice( uint32_t devId,int enable, int vol);




struct snd_use_case_mgr_t *mUcMgr;


/**
@brief This function enable and disable pcm session
@ParamIn-logfilefd can be paased as a NULL, if logging is not required.
@val=1 enable
@val=0 disable
*/
int alsa_enable_pcm_session(FILE *logfilefd, int val, struct alsa_config_param *alsaConfigData, struct mixer **mxr, struct pcm **pcmCntxt)

{
    int ret = 0;

    if(!val && !*pcmCntxt){
        msg_printf(stderr,"No Active session to close \n");
        return 0;
    }else if(val && *pcmCntxt){
        msg_printf(stderr,"Session Active new session not needed\n");
        return 0;
    }


    if(!val)
    {
        msg_printf(logfilefd,"closing down PCM session and Mixer\n");
        if(pcm_close(*pcmCntxt)){
            msg_printf(stderr,"pcm_close(0x%08X) failed\n",(unsigned int )*pcmCntxt);
           return -1;
        }
        *pcmCntxt= NULL;
    }
    set_audiodevice(alsaConfigData->devid, val, alsaConfigData->volume);
    if(val){
        int flags = PCM_NMMAP;

        msg_printf(logfilefd,"Opening PCM session\n");
        if (alsaConfigData->channels == 1)
            flags |= PCM_MONO;
        else
            flags |= PCM_STEREO;

        *pcmCntxt = pcm_open(flags, "hw:0,0");
        if (!pcm_ready(*pcmCntxt)) {
            pcm_close(*pcmCntxt);
            msg_printf(stderr,"pcm_ready(0x%08X) failed\n",(unsigned int )*pcmCntxt);
            return -1;
        }
    }


    return ret;
}

int alsa_set_params(FILE *logfilefd, int channels, int samplingrate, struct pcm *pcmCntxt)
{
    struct snd_pcm_hw_params *params;
    struct snd_pcm_sw_params *sparams;

    unsigned long periodSize, bufferSize;
    unsigned int periodTime, bufferTime;

    params = (struct snd_pcm_hw_params*) calloc(1, sizeof(struct snd_pcm_hw_params));
    if (!params) {
      msg_printf(stderr, "failed to allocate ALSA hardware parameters!");
      return -1;
    }

    param_init(params);

    param_set_mask(params, SNDRV_PCM_HW_PARAM_ACCESS,
                (pcmCntxt->flags & PCM_MMAP)? SNDRV_PCM_ACCESS_MMAP_INTERLEAVED : SNDRV_PCM_ACCESS_RW_INTERLEAVED);
    param_set_mask(params, SNDRV_PCM_HW_PARAM_FORMAT,
                SNDRV_PCM_FORMAT_S16_LE);
    param_set_mask(params, SNDRV_PCM_HW_PARAM_SUBFORMAT,
                SNDRV_PCM_SUBFORMAT_STD);
    param_set_min(params, SNDRV_PCM_HW_PARAM_PERIOD_TIME, 1000);
    param_set_int(params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, 16);
    param_set_int(params, SNDRV_PCM_HW_PARAM_FRAME_BITS,
                channels - 1 ? 32 : 16);
    param_set_int(params, SNDRV_PCM_HW_PARAM_CHANNELS, channels);
    param_set_int(params, SNDRV_PCM_HW_PARAM_RATE, samplingrate);

    param_set_hw_refine(pcmCntxt, params);

    if (param_set_hw_params(pcmCntxt, params)) {
         msg_printf(stderr, "cannot set hw params \n");
         return -1;
    }
    param_dump(params);

    pcmCntxt->buffer_size = pcm_buffer_size(params);
    pcmCntxt->period_size = pcm_period_size(params);
    pcmCntxt->period_cnt = pcmCntxt->buffer_size/pcmCntxt->period_size;
    msg_printf (logfilefd,"period_size (%d)\n", pcmCntxt->period_size);
    msg_printf (logfilefd,"buffer_size (%d)\n", pcmCntxt->buffer_size);
    msg_printf (logfilefd,"period_cnt  (%d)\n", pcmCntxt->period_cnt);

    sparams = (struct snd_pcm_sw_params*) calloc(1, sizeof(struct snd_pcm_sw_params));
    if (!sparams) {
        msg_printf(stderr, "failed to allocate ALSA software parameters!\n");
        return -1;
    }
    sparams->tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
    sparams->period_step = 1;
    sparams->avail_min = (channels) ? pcmCntxt->period_size/2 : pcmCntxt->period_size/4;

    sparams->start_threshold = 1;
    sparams->stop_threshold = pcmCntxt->buffer_size;
    sparams->xfer_align = (channels) ? pcmCntxt->period_size/2 : pcmCntxt->period_size/4; /* needed for old kernels */;
    sparams->silence_size = 0;
    sparams->silence_threshold = 0;

    if (param_set_sw_params(pcmCntxt, sparams)) {
         msg_printf(stderr, "cannot set sw params");
         return -1;
    }
    msg_printf (logfilefd,"avail_min (%d)\n", (int)sparams->avail_min);
    msg_printf (logfilefd,"start_threshold (%d)\n",(int) sparams->start_threshold);
    msg_printf (logfilefd,"stop_threshold (%d)\n", (int)sparams->stop_threshold);
    msg_printf (logfilefd,"xfer_align (%d)\n", (int)sparams->xfer_align);

    if (pcm_prepare(pcmCntxt)) {
       msg_printf(stderr,"pcm_prepare failed\n");
       pcm_close(pcmCntxt);
       return -1;
    }
return 0;
}

int alsa_framer_update_pcm(FILE *logfilefd, struct pcm *pcmCntxt, OMX_BUFFERHEADERTYPE* pbuff,int *spilledOverBytes,char* tempbuff)
{
    /* framer logic to update ALSA in period size chuncks*/
    int sbytes = 0,rbytes = 0;
    int PS = (int)pcmCntxt->period_size;
    unsigned char *inbuf = pbuff->pBuffer + pbuff->nOffset;
    int fl = pbuff->nFilledLen;

    msg_printf(logfilefd," Period Size (%d)\n",PS);
    msg_printf(logfilefd," Bytes spilled over from last frame (%d)\n",*spilledOverBytes);
    msg_printf(logfilefd," FilledLength of current buffer (%d)\n",fl);

    do{
       if(*spilledOverBytes + fl < PS){
           memcpy((tempbuff+ *spilledOverBytes),inbuf,fl);
           *spilledOverBytes +=fl;
           break;
       }

       memcpy((tempbuff+ *spilledOverBytes),(inbuf+sbytes),(PS-*spilledOverBytes));
       if(pcm_write(pcmCntxt,tempbuff,PS)){
           msg_printf(stderr," Alsa PCM write Failed\n");
           return -1;
       }

       sbytes += (PS - *spilledOverBytes);
       if(*spilledOverBytes) *spilledOverBytes = 0;
       rbytes = fl - sbytes;
       msg_printf(logfilefd," Bytes sent in current buffer(%d)\n \
           Bytes remaining in current buffer(%d)\n",sbytes,rbytes);

    }while(rbytes >= PS);

    if(rbytes){
       memcpy(tempbuff,inbuf+sbytes,rbytes);
       *spilledOverBytes = rbytes;
       msg_printf(logfilefd," Bytes spilled over (%d)\n",*spilledOverBytes);
    }
    return 1;

}

int alsa_init_audiohardware(FILE *logfilefd)
{
    FILE *fp;
    char soundCardInfo[200];
    int codec_rev = 2;
    msg_printf(logfilefd, "opening UCM manager");
    if((fp = fopen("/proc/asound/cards","r")) == NULL) {
        msg_printf(stderr,"Cannot open /proc/asound/cards file to get sound card info");
    } else {
        while((fgets(soundCardInfo, sizeof(soundCardInfo), fp) != NULL)) {
            msg_printf(logfilefd, "SoundCardInfo %s", soundCardInfo);
            if (strstr(soundCardInfo, "msm8960-tabla1x-snd-card")) {
                codec_rev = 1;
                break;
            }
            else if (strstr(soundCardInfo, "sm-snd-card")) {
                codec_rev = 2;
                break;
            }
            else if (strstr(soundCardInfo, "msm8930-sitar-snd-card")) {
                codec_rev = 3;
                break;
            } else if (strstr(soundCardInfo, "msm8974-taiko-mtp-snd-card")) {
                codec_rev = 40;
                break;
            } else if (strstr(soundCardInfo, "msm8974-taiko-cdp-snd-card")) {
                codec_rev = 41;
                break;
            } else if (strstr(soundCardInfo, "msm8974-taiko-fluid-snd-card")) {
                codec_rev = 42;
                break;
            } else if (strstr(soundCardInfo, "msm8974-taiko-liquid-snd-card")) {
                codec_rev = 43;
                break;
            } else if (strstr(soundCardInfo, "msm8610-wcd-snd-card")) {
                codec_rev = 43;//todo: change code_Rec for msm8610-wcd sound card
                break;
            } else if (strstr(soundCardInfo, "msm8x10-skuab-snd-card")) {
                codec_rev = 45;
                break;
            } else if (strstr(soundCardInfo, "msm8x10-skuaa-snd-card")) {
                codec_rev = 46;
                break;
            } else if (strstr(soundCardInfo, "msm8226-tapan-snd-card")) {
                codec_rev = 50;
                break;
            } else if (strstr(soundCardInfo, "msm8226-tapan-skuf-snd-card")) {
                codec_rev = 51;
                break;
            } else if (strstr(soundCardInfo, "msm8226-tapan9302-skuf-snd-card")) {
                codec_rev = 52;
                break;
            } else if (strstr(soundCardInfo, "apq8084-taiko-cdp-snd-card")) {
                codec_rev = 60;
                break;
            } else if (strstr(soundCardInfo, "apq8084-taiko-mtp-snd-card")) {
                codec_rev = 61;
                break;
            } else if (strstr(soundCardInfo, "apq8084-taiko-liquid-snd-card")) {
                codec_rev = 62;
                break;
            } else if (strstr(soundCardInfo, "msm8994-tomtom-mtp-snd-card")) {
                codec_rev = 70;
                break;
            } else if (strstr(soundCardInfo, "msm8994-tomtom-cdp-snd-card")) {
                codec_rev = 71;
                break;
            } else if (strstr(soundCardInfo, "msm8994-tomtom-stp-snd-card")) {
                codec_rev = 72;
                break;
            } else if (strstr(soundCardInfo, "msm8994-tomtom-liquid-snd-card")) {
                codec_rev = 73;
                break;
            }
        }
        fclose(fp);
    }
    if (codec_rev == 1) {
            msg_printf(logfilefd, "Detected tabla 1.x sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm");
    }
    else if (codec_rev == 3) {
            msg_printf(logfilefd, "Detected sitar 1.x sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_Sitar");
    } else if (codec_rev == 40) {
            msg_printf(logfilefd, "Detected taiko sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_Taiko");
    } else if (codec_rev == 41) {
            msg_printf(logfilefd, "Detected taiko sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_Taiko_CDP");
    } else if (codec_rev == 42) {
            msg_printf(logfilefd, "Detected taiko sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_Taiko_Fluid");
    } else if (codec_rev == 43) {
            msg_printf(logfilefd, "Detected taiko liquid sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_Taiko_liquid");
    } else if (codec_rev == 45) {
            msg_printf(logfilefd, "Detected msm8610-wcd skuab sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_8x10_wcd_skuab");
    } else if (codec_rev == 46) {
            msg_printf(logfilefd, "Detected msm8610-wcd skuaa sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_8x10_wcd_skuaa");
    } else if (codec_rev == 50) {
            msg_printf(logfilefd, "Detected tapan liquid sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_Tapan");
    } else if (codec_rev == 51) {
            msg_printf(logfilefd, "Detected tapan skuf sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_Tapan_SKUF");
    } else if ( codec_rev == 52) {
            msg_printf(logfilefd, "Detected tapan 9302 skuf sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_TapanLite_SKUF");
    } else if (codec_rev == 60) {
            msg_printf(logfilefd, "Detected taiko sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_Taiko_CDP");
    } else if (codec_rev == 61) {
            msg_printf(logfilefd, "Detected taiko sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_Taiko");
    } else if (codec_rev == 62) {
            msg_printf(logfilefd, "Detected taiko sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_Taiko_liquid");
    } else {
            msg_printf(logfilefd,"Detected tabla 2.x sound card");
            snd_use_case_mgr_open(&mUcMgr, "snd_soc_msm_2x");

    }

    if (mUcMgr < 0) {
        msg_printf(stderr, "Failed to open ucm instance: %d", errno);
        return -1;
    } else {
        msg_printf(logfilefd,"ucm instance opened: %u", (unsigned)mUcMgr);
    }
    return 0;
}

int alsa_deinit_audiohardware(FILE *logfilefd)
{
    if (mUcMgr != NULL) {
        msg_printf(logfilefd, "closing ucm instance: %u", (unsigned)mUcMgr);
        snd_use_case_mgr_close(mUcMgr);
    }
    return 0;
}

static void set_audiodevice( uint32_t devId,int enable, int vol)
{
    char *devtble[]={SND_USE_CASE_DEV_HANDSET,SND_USE_CASE_DEV_SPEAKER,
        SND_USE_CASE_DEV_HEADSET,SND_USE_CASE_DEV_HEADPHONES};
    snd_use_case_set(mUcMgr, "_verb", SND_USE_CASE_VERB_HIFI);
    if(devId <= 3 ) {
        if(enable)
            snd_use_case_set(mUcMgr, "_enadev", devtble[devId]);
        else
            snd_use_case_set(mUcMgr, "_disdev", devtble[devId]);
    }
    else{
        msg_printf(stderr,"Device(%d) Id %d\n",enable,devId);
        return -1;
    }
}


static int msg_printf(FILE *fd, const char *format, ...)
{
    if (fd == NULL)
        return 0;
    va_list argp;
    va_start(argp, format);
    vfprintf(fd, format, argp);
    va_end(argp);
    return 0;
}

