#ifdef __cplusplus
extern "C" {
#endif

/**

  @file   audio_ftm_dispatch.c
  @brief  AUDIO FTM Dispatcher
====================================================================================================
Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

**/

#include "event.h"
#include "msg.h"
#include "log.h"

#include "diag_lsm.h"
#include "diagpkt.h"
#include "diagcmd.h"
#include "diag.h"
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <alsa_audio.h>
#include <alsa_ucm.h>
#include <cutils/properties.h>

/* FTM-specific interfaces and definitions */
#include "DALSYS_common.h"
#include "ftm_audio_dispatch.h"
#include "ftm_audio_diag_dispatch.h"

/* Audio FTM driver */
#include "audio_ftm_driver.h"
#include "audio_ftm_diag_mem.h"

#include "audio_ftm_hw_drv.h"

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

#define RECORD_ONE_SECOND  50     /* frame numbers for 8K PCM capture in one second */
#define RECORD_MAX_BUF_SIZE    16000  /* Buffer allocated for collecting record data */

typedef union
{
  AUD_FTM_TONE_PLAY_PARAM_T  tone_play_param;
  AUD_FTM_PCM_LP_PARAM_T     pcm_lp_param;
  AUD_FTM_AFE_LP_PARAM_T     afe_lp_param;
  AUD_FTM_PCM_REC_PARAM_T    pcm_rec_param;
  AUD_FTM_PCM_PLAY_PARAM_T    pcm_play_param;
}AUD_FTM_DISPATCH_PARAM_T;


#define VOL_MAX 100
#define VOL_DEFAULT 7

typedef struct
{
    int32                        ftm_case;
    AUDIO_FTM_INPUT_PATH_ENUM_T  iPath;
    AUDIO_FTM_OUTPUT_PATH_ENUM_T oPath;
    DALBOOL bALLOW_TONE_PLAY;
    DALBOOL bALLOW_AFE_LOOPBACK;
    DALBOOL bALLOW_ADIE_LOOPBACK;
    DALBOOL bALLOW_PCM_REC;
    uint16  channel;
}AUD_FTM_PATH_MATRIX_T;

struct ftm_tc_device ftm_tc_devices[50];

static DALBOOL taiko_i2s = FALSE;
struct test_params {
  FILE *fp;
  int test_case;
  int enable;
  int volume;
  int fl;
  int fh;
  int duration;
  char *filename;
  int type_of_test;
};
extern int g_config_test;
static int  g_deviceplayback = -1;
static int  g_devicecapture = -1;

struct mixer_ctl *get_ctl(struct mixer *m, char *name)
{
    char *p;
    unsigned idx = 0;

    if (isdigit(name[0]))
        return mixer_get_nth_control(m, atoi(name) - 1);

    p = strrchr(name, '#');
    if (p) {
        *p++ = 0;
        idx = atoi(p);
    }

    return mixer_get_control(m, name, idx);
}

static int parse(struct test_params *commands)
{
    FILE *fp;
    char *p, array[100], *pmode;
    char tc[10];
    char en[] = "enable";
    char dis[] = "disable";
    int len = 0, found = 0, len2 = 0;
    struct mixer *mxr = NULL;
    struct mixer_ctl *ctl = 0;
    int ret, vol_found = 0;
    char *temp;
    int params, sublen;


    if (!commands || !commands->fp) {
        printf("\n Ivalid params");
        return -1;
    }
    fp = commands->fp;
    snprintf(tc, sizeof(tc), "tc %d", commands->test_case);
    pmode = (commands->enable)?en:dis;

    /* Find the test case */
    while((p = fgets(array, sizeof(array), fp))) {
        len = strnlen(p,sizeof(array));
        p[len-1] = '\0';
        len--;
        if (!strncmp(p, tc, sizeof(tc))) {
             /* Differentiate between tc 2 and tc 28 */
             len2 = strnlen(tc, sizeof(tc));
             if (len == len2)
                 found = 1;
             break;
        }
    }
    if (!found) {
        printf("\n Invalid Testcase %s", tc);
        return -1;
    }

    printf("\nFound %s %d seq %s", tc, found, pmode);
    strlcpy(tc, "tc", sizeof(tc));

    /* Find enable or disable commands*/
    found = 0;
    commands->type_of_test = -1;
    while((p = fgets(array, sizeof(array), fp))) {
        len = strnlen(p,sizeof(array));
        p[len-1] = '\0';
        len--;
        /* Comment added print comment */
        if (!strncmp(&p[0], "#", 1)) {
            printf("\n %s", p);
        } else if (strstr(p, "Rxdevice")) {
            temp = NULL;
            if ((temp = strstr(p,":"))) {
                sublen = temp - p;
                len = len - sublen - 1;
                temp++;
                temp[len] = '\0';
                if (*temp >= '0' || *temp <= '9')
                    g_deviceplayback = atoi(temp);
                else
                    g_deviceplayback = -1;
            }
        } else if (strstr(p, "Txdevice")) {
            temp = NULL;
            if ((temp = strstr(p,":"))) {
                sublen = temp - p;
                len = len - sublen - 1;
                temp++;
                temp[len] = '\0';
                if (*temp >= '0' || *temp <= '9')
                    g_devicecapture = atoi(temp);
                else
                    g_devicecapture = -1;
            }
        } else if (strstr(p, tc) && !strstr(p, ":")) {
            break;
        } else if (!strncmp(&p[0], "!", 1)) {
            if (!strncmp(&p[1], "Playback", len)) {
                commands->type_of_test = PATH_RX;
            } else if (!strncmp(&p[1], "Capture", len)) {
                commands->type_of_test = PATH_TX;
            } else if (!strncmp(&p[1], "AfeLoop", len)) {
                commands->type_of_test = PATH_AFE_LB;
            } else if (!strncmp(&p[1], "CodecLoop", len)) {
                commands->type_of_test = PATH_ADIE_LB;
            } else if (!strncmp(&p[1], "FM", len)) {
                commands->type_of_test = PATH_FM;
            } else
                commands->type_of_test = -1;
        } else if (strstr(p, pmode)) {
            found = 1;
            break;
        }
    }
    if (!found || commands->type_of_test == -1) {
        printf("\n Sequence for %s not found", pmode);
        return -1;
    } else {
        printf("\n Sequence for %s found", pmode);
    }
    pmode = (!commands->enable)?en:dis;
    mxr = mixer_open("/dev/snd/controlC0");
    if (!mxr) {
        printf("\nOpening mixer control failed");
        return -1;
    }
    while((p = fgets(array, sizeof(array), fp))) {
        len = strnlen(p,sizeof(array));
        p[len-1] = '\0';
        len--;
        if ( ((strstr(p, tc) || strstr(p, pmode)) && (!strstr(p, ":"))) ) {
        break;
        } else {
            if (len) {
                char ctlname[100];
                char ctlval[100];
                temp = strstr(p,":");
                if (temp) {
                    sublen = temp - p;
                    memcpy(ctlname, p, sublen);
                    ctlname[sublen] = '\0';
                    ctl = get_ctl(mxr, ctlname);
                    if (!ctl) {
                        printf("%s Failed to get %s\n", ctlname);
                        break;
                    }
                    sublen = len - sublen;
                    sublen--;
                    temp++;
                    memcpy(ctlval, temp, sublen);
                    ctlval[sublen] = '\0';
                    int val = -1;
                    while(sublen > 0) {
                        if (*temp == ' ') {
                            temp++;
                            sublen--;
                        }else if (*temp >= '0' && *temp <= '9') {
                            val = 1;
                            break;
                        } else {
                            val = 0;
                            break;
                        }
                    }
                    if (val < 0) {
                        printf("\n In valid param for val");
                        return -EINVAL;
                    } else if (!val) {
                        printf("\n Set %s %s", ctlname, ctlval);
                        ret = mixer_ctl_select(ctl, ctlval);
                    } else {
                        val =  atoi(temp);
                        if (strstr(ctlname, "Volume")) {
                            val = commands->volume;
                        }
                        printf("\n Set %s %d", ctlname, val);
                        ret = mixer_ctl_set(ctl, val);
                    }
                }
        }
      }
    }
    mixer_close(mxr);
    return ret;
}

// Test on CDP for analog mic1.
int tc_tx_handset_mic1_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC6");
    ctl = get_ctl(mxr, "DEC6 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "DEC6 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 20);
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 1);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_handset_mic1_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC6 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_handset_mic2_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    // MIC2 is set to headset on MTP
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 20);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_handset_mic2_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_handset_earphone_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "RX1");
    ctl = get_ctl(mxr, "DAC1 Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 0);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_handset_earphone_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_headset_mic_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 1);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC5");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC5 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "DEC5 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 20);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_headset_mic_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC5 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_headset_spkr_mono_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_headset_spkr_mono_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_headset_spkr_stereo_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");

    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 0);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 0);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_headset_spkr_stereo_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_headset_spkr_mono_diff_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX5 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX6 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");
    ctl = get_ctl(mxr, "RX6 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT4 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX4 CHAIN INVERT Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX6 CHAIN INVERT Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_headset_spkr_mono_diff_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%",vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_headset_spkr_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 0);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_headset_spkr_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_headset_spkr_left_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 0);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_headset_spkr_left_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_spkr_phone_mic_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "DMIC1");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_spkr_phone_mic_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_spkr_phone_stereo_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 1);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    // BUG in the mixer lib. should set to 2 channels
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX5 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");
    ctl = get_ctl(mxr, "RX6 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT4 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_spkr_phone_stereo_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_spkr_phone_mono_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX5 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");
    ctl = get_ctl(mxr, "RX6 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT4 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_spkr_phone_mono_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "RX3 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    ctl = get_ctl(mxr, "RX5 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_spkr_phone_left_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_spkr_phone_left_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "RX3 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_spkr_phone_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX5 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX6 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_spkr_phone_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);

    ctl = get_ctl(mxr, "RX4 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

//TODO: check this
int tc_rx_lineout_right_diff_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX5 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX6 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX6 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");

    ctl = get_ctl(mxr, "RX4 CHAIN INVERT Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX6 CHAIN INVERT Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT4 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_lineout_right_diff_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

//TODO: check this
int tc_rx_lineout_left_diff_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");
    ctl = get_ctl(mxr, "RX4 CHAIN INVERT Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX6 CHAIN INVERT Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT4 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_lineout_left_diff_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_linein_left_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC5");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC5 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_linein_left_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC2 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_linein_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;
    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC6");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC6 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_linein_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_audio_input3_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC4");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC4 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC3");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC3 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "MICBIAS3 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 1);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_audio_input3_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC4 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_audio_input4_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC3");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC3 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC4");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC4 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "MICBIAS4 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 1);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_audio_input4_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC3 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_lineout5_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX7 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "LINEOUT5 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "On");
    else
        ret = mixer_ctl_select(ctl, "Off");
    ctl = get_ctl(mxr, "LINEOUT5 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_lineout5_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic1_left_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    // MIC BIAS will vary by platform.
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic1_left_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic1_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic1_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC2 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

// broadside
int tc_tx_dmic1_left_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIM TX8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic1_left_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "DEC2 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic2_left_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    // This varies by platform.
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic2_left_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC2 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic2_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    // This varies by platform.
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic2_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic2_left_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "SLIM TX8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    // This varies by platform.
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic2_left_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "DEC2 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_analog_input1_analog_input3_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");

    // input 1
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC6");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC6 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (!ctl)
        goto mixer_err;
    ctl = get_ctl(mxr, "ADC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);

    // input 3
    ctl = get_ctl(mxr, "SLIM TX8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC4");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC4 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC3");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (!ctl)
        goto mixer_err;
    ctl = get_ctl(mxr, "MICBIAS3 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "ADC3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_analog_input1_analog_input3_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC2 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

//TODO (handset mic + headset mic)
int tc_tx_analog_input1_analog_input2_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    // input 1
    ctl = get_ctl(mxr, "SLIM TX8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC6");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC6 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (!ctl)
        goto mixer_err;
    ctl = get_ctl(mxr, "ADC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);

    // analog input 2
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC5");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC5 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (!ctl)
        goto mixer_err;
    ctl = get_ctl(mxr, "ADC2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_analog_input1_analog_input2_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC6 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "DEC5 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_fm_stereo_input_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer INTERNAL_FM_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Switch");
    if (!ctl) {
        /* kernel 3.0 command for hostless mode on
         * SLIMBUS_0_RX path
         */
        ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
        if (!ctl)
            goto mixer_err;
    }
    ret = mixer_ctl_set(ctl, val);
    // FM Tx is device "hw:0,6".  Check /proc/asound/pcm.
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_fm_stereo_input_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC2 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

// FM Rx is device "hw:0,5".  Check /proc/asound/pcm.
int tc_rx_fm_stereo_output_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer INTERNAL_FM_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Switch");
    if (!ctl) {
        /* kernel 3.0 command for hostless mode on
         * SLIMBUS_0_RX path
         */
        ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
        if (!ctl)
            goto mixer_err;
    }
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

/*
   Must be able to set true 0dB to pass FM tests.
*/
int tc_rx_fm_stereo_output_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

//TODO
int tc_tx_bt_sco_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer INTERNAL_BT_SCO_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    // BT SCO Tx is device "hw:0,?".  Check /proc/asound/pcm.
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_bt_sco_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    return 0;
}

int tc_rx_bt_sco_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "INTERNAL_BT_SCO_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    // BT SCO Rx is device "hw:0,4".  Check /proc/asound/pcm.
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_bt_sco_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    return 0;
}

int tc_lb_afe_handset_mic_handset_earphone_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DAC1 Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC6");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC6 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Switch");
    if (!ctl) {
        /* kernel 3.0 command for hostless mode on
         * SLIMBUS_0_RX path
         */
        ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
        if (!ctl)
            goto mixer_err;
    }

    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_afe_handset_mic_handset_earphone_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_afe_headset_mic_headset_l_r_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
        if (!ctl)
      goto mixer_err;
        if (enable)
      ret = mixer_ctl_select(ctl, "One");

    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC5");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC5 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC5 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 0);
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 0);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 0);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Switch");
    if (!ctl) {
        /* kernel 3.0 command for hostless mode on
         * SLIMBUS_0_RX path
         */
        ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
        if (!ctl)
            goto mixer_err;
    }
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_afe_headset_mic_headset_l_r_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;

    printf("%s(%d)\n",__func__,vol);

    ctl = get_ctl(mxr, "DEC5 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 80);

    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_afe_spkr_phone_mic_spkr_phone_l_r_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
    ret = mixer_ctl_set(ctl, 50);

    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");
    ctl = get_ctl(mxr, "RX5 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX6 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 50);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 50);
    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 50);
    ctl = get_ctl(mxr, "LINEOUT4 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 50);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Switch");
    if (!ctl) {
        /* kernel 3.0 command for hostless mode on
         * SLIMBUS_0_RX path
         */
        ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
        if (!ctl)
            goto mixer_err;
    }
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_afe_spkr_phone_mic_spkr_phone_l_r_vol(struct mixer *mxr, int vol)
{
    printf("%s(%d)\n",__func__,vol);
    return 0;
}

int tc_lb_adie_handset_mic_handset_earphone_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "DEC6 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC6 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 0);
    ctl = get_ctl(mxr, "ADC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "IIR1 INP1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC6");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "DAC1 Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 0);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_handset_mic_handset_earphone_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    char decvol[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    snprintf(decvol, 4, "%d%%", 65);
    ctl = get_ctl(mxr, "DEC6 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, decvol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_headset_mic_headset_l_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    // headset mic
    ctl = get_ctl(mxr, "DEC8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);

    // loopback
    ctl = get_ctl(mxr, "IIR1 INP1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC8");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    // headset spkr left
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
        ret = mixer_ctl_set(ctl, 0);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_headset_mic_headset_l_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return 0;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_headset_mic_headset_r_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    // headset mic
    ctl = get_ctl(mxr, "DEC8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "ADC2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    else
        ret = mixer_ctl_set(ctl, 0);
    // loopback
    ctl = get_ctl(mxr, "IIR1 INP1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC8");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    // headset spkr right
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 0);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_headset_mic_headset_r_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return 0;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_spkr_phone_mic_spkr_phone_l_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    // spkr phone mic
    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    // loopback
    ctl = get_ctl(mxr, "IIR1 INP1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    // spkr phone left
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 0);
    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 0);
    ctl = get_ctl(mxr, "RX3 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 50);
    ctl = get_ctl(mxr, "RX4 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 50);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_spkr_phone_mic_spkr_phone_l_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_spkr_phone_mic_spkr_phone_r_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    // spkr phone mic
    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    // loopback
    ctl = get_ctl(mxr, "IIR1 INP1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX5 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "RX6 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "CIC_OUT");

    ctl = get_ctl(mxr, "RX5 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 50);
    else
        ret = mixer_ctl_set(ctl, 0);
    ctl = get_ctl(mxr, "RX6 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 50);
    else
        ret = mixer_ctl_set(ctl, 0);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_spkr_phone_mic_spkr_phone_r_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    char pct[5];
    printf("%s(%d)\n",__func__,vol);
    snprintf(pct, 4, "%d%%", vol);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    ctl = get_ctl(mxr, "LINEOUT4 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, pct);
    return 0;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}


static struct ftm_tc_device ftm_tc_devices_tabla[] = {
{
    NULL,
    NULL,
    PATH_NULL,
    50
},
{
    tc_tx_handset_mic1_en,
    tc_tx_handset_mic1_vol,
    PATH_TX,
    50
},
{
    tc_tx_handset_mic2_en,
    tc_tx_handset_mic2_vol,
    PATH_TX,
    0
},
{
    tc_rx_handset_earphone_en,
    tc_rx_handset_earphone_vol,
    PATH_RX,
    50
},
{
    tc_tx_headset_mic_en,
    tc_tx_headset_mic_vol,
    PATH_TX,
    50
},
{
    tc_rx_headset_spkr_mono_en,
    tc_rx_headset_spkr_mono_vol,
    PATH_RX,
    50
},
{
    tc_rx_headset_spkr_stereo_en,
    tc_rx_headset_spkr_stereo_vol,
    PATH_RX,
    50
},
{
    tc_rx_headset_spkr_mono_diff_en,
    tc_rx_headset_spkr_mono_diff_vol,
    PATH_RX,
    50
},
{
    tc_rx_headset_spkr_left_en,
    tc_rx_headset_spkr_left_vol,
    PATH_RX,
    50
},
{
    tc_rx_headset_spkr_right_en,
    tc_rx_headset_spkr_right_vol,
    PATH_RX,
    100
},
{
    tc_tx_spkr_phone_mic_en,
    tc_tx_spkr_phone_mic_vol,
    PATH_TX,
    50
},
{
    tc_rx_spkr_phone_stereo_en,
    tc_rx_spkr_phone_stereo_vol,
    PATH_RX,
    50
},
{
    tc_rx_spkr_phone_mono_en,
    tc_rx_spkr_phone_mono_vol,
    PATH_RX,
    50
},
{
    tc_rx_spkr_phone_left_en,
    tc_rx_spkr_phone_left_vol,
    PATH_RX,
    50
},
{
    tc_rx_spkr_phone_right_en,
    tc_rx_spkr_phone_right_vol,
    PATH_RX,
    50
},
{
    tc_rx_lineout_right_diff_en,
    tc_rx_lineout_right_diff_vol,
    PATH_RX,
    50
},
{
    tc_rx_lineout_left_diff_en,
    tc_rx_lineout_left_diff_vol,
    PATH_RX,
    50
},
{
    tc_tx_linein_left_en,
    tc_tx_linein_left_vol,
    PATH_TX,
    50
},
{
    tc_tx_linein_right_en,
    tc_tx_linein_right_vol,
    PATH_TX,
    50
},
{
    tc_tx_audio_input3_en,
    tc_tx_audio_input3_vol,
    PATH_TX,
    50
},
{
    tc_tx_audio_input4_en,
    tc_tx_audio_input4_vol,
    PATH_TX,
    50
},
{
    tc_rx_lineout5_en,
    tc_rx_lineout5_vol,
    PATH_RX,
    50
},
{
    tc_tx_dmic1_left_en,
    tc_tx_dmic1_left_vol,
    PATH_TX,
    50
},
{
    tc_tx_dmic1_right_en,
    tc_tx_dmic1_right_vol,
    PATH_TX,
    50
},
{
    tc_tx_dmic1_left_right_en,
    tc_tx_dmic1_left_right_vol,
    PATH_TX,
    50
},
{
    tc_tx_dmic2_left_en,
    tc_tx_dmic2_left_vol,
    PATH_TX,
    50
},
{
    tc_tx_dmic2_right_en,
    tc_tx_dmic2_right_vol,
    PATH_TX,
    50
},
{
    tc_tx_dmic2_left_right_en,
    tc_tx_dmic2_left_right_vol,
    PATH_TX,
    50
},
{
    tc_tx_analog_input1_analog_input3_en,
    tc_tx_analog_input1_analog_input3_vol,
    PATH_TX,
    50
},
{
    tc_tx_analog_input1_analog_input2_en,
    tc_tx_analog_input1_analog_input2_vol,
    PATH_TX,
    50
},
{
    tc_tx_fm_stereo_input_en,
    tc_tx_fm_stereo_input_vol,
    PATH_FM,
    50
},
{
    tc_rx_fm_stereo_output_en,
    tc_rx_fm_stereo_output_vol,
    PATH_FM,
    100
},
{
    tc_tx_bt_sco_en,
    tc_tx_bt_sco_vol,
    PATH_TX,
    50
},
{
    tc_rx_bt_sco_en,
    tc_rx_bt_sco_vol,
    PATH_RX,
    50
},
{
    tc_lb_afe_handset_mic_handset_earphone_en,
    tc_lb_afe_handset_mic_handset_earphone_vol,
    PATH_AFE_LB,
    50
},
{
    tc_lb_afe_headset_mic_headset_l_r_en,
    tc_lb_afe_headset_mic_headset_l_r_vol,
    PATH_AFE_LB,
    50
},
{
    tc_lb_afe_spkr_phone_mic_spkr_phone_l_r_en,
    tc_lb_afe_spkr_phone_mic_spkr_phone_l_r_vol,
    PATH_AFE_LB,
    50
},
// these are same device
{
    // vol 0
    tc_lb_afe_headset_mic_headset_l_r_en,
    tc_lb_afe_headset_mic_headset_l_r_vol,
    PATH_AFE_LB,
    0
},
{
    // vol 25
    tc_lb_afe_headset_mic_headset_l_r_en,
    tc_lb_afe_headset_mic_headset_l_r_vol,
    PATH_AFE_LB,
    25
},
{
    // vol 75
    tc_lb_afe_headset_mic_headset_l_r_en,
    tc_lb_afe_headset_mic_headset_l_r_vol,
    PATH_AFE_LB,
    75
},
{
    tc_lb_adie_handset_mic_handset_earphone_en,
    tc_lb_adie_handset_mic_handset_earphone_vol,
    PATH_ADIE_LB,
    50
},
{
    tc_lb_adie_headset_mic_headset_l_en,
    tc_lb_adie_headset_mic_headset_l_vol,
    PATH_ADIE_LB,
    50
},
{
    tc_lb_adie_headset_mic_headset_r_en,
    tc_lb_adie_headset_mic_headset_r_vol,
    PATH_ADIE_LB,
    50
},
{
    tc_lb_adie_spkr_phone_mic_spkr_phone_l_en,
    tc_lb_adie_spkr_phone_mic_spkr_phone_l_vol,
    PATH_ADIE_LB,
    50
},
{
    tc_lb_adie_spkr_phone_mic_spkr_phone_r_en,
    tc_lb_adie_spkr_phone_mic_spkr_phone_r_vol,
    PATH_ADIE_LB,
    50
},
};

int tc_tx_handset_mic_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    // MIC2 is set to headset on MTP
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 20);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_handset_mic_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_handset_earphone_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "RX1");
    ctl = get_ctl(mxr, "EAR PA Gain");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "POS_2_DB");
        ctl = get_ctl(mxr, "DAC1 MUX");
        if (!ctl)
                goto mixer_err;
        ret = mixer_ctl_select(ctl, "RX1");
    ctl = get_ctl(mxr, "DAC1 Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_handset_earphone_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_headset_mic_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 1);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 20);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_headset_mic_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_headset_spkr_mono_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "DAC4 MUX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "ON");
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 80);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 0);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_headset_spkr_mono_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_spkr_mono_diff_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DAC2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DAC3 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "INV_RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 100);
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "PMIC SPK Gain");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "POS_20_DB");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_spkr_mono_diff_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic1_left_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    // MIC BIAS will vary by platform.
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic1_left_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic1_right_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic1_right_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

// broadside
int tc_tx_dmic1_left_right_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIM TX2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic1_left_right_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic2_left_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    // This varies by platform.
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic2_left_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic2_right_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC4");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    // This varies by platform.
    ctl = get_ctl(mxr, "MICBIAS4 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic2_right_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic2_left_right_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "SLIM TX2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    // This varies by platform.
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_dmic2_left_right_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "DEC2 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_analog_input1_analog_input3_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");

    // input 1
    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (!ctl)
        goto mixer_err;
    ctl = get_ctl(mxr, "ADC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
        ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
        if (!ctl)
                goto mixer_err;
        ret = mixer_ctl_set(ctl, val);

    // input 3
    ctl = get_ctl(mxr, "SLIM TX2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC3");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (!ctl)
        goto mixer_err;
    ctl = get_ctl(mxr, "MICBIAS2 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "ADC3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_analog_input1_analog_input3_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

//TODO (handset mic + headset mic)
int tc_tx_analog_input1_analog_input2_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    // input 1
    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (!ctl)
        goto mixer_err;
    ctl = get_ctl(mxr, "ADC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
        ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_set(ctl, 1);

    // analog input 2
    ctl = get_ctl(mxr, "SLIM TX2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (!ctl)
        goto mixer_err;
        ctl = get_ctl(mxr, "MICBIAS2 CAPLESS Switch");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_set(ctl, 1);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_analog_input1_analog_input2_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "DEC2 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_fm_stereo_input_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DAC4 MUX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "ON");
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer INTERNAL_FM_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Switch");
    if (!ctl) {
        /* kernel 3.0 command for hostless mode on
         * SLIMBUS_0_RX path
         */
        ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
        if (!ctl)
            goto mixer_err;
    }
    ret = mixer_ctl_set(ctl, val);
    // FM Tx is device "hw:0,6".  Check /proc/asound/pcm.
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_fm_stereo_input_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX3 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

// FM Rx is device "hw:0,5".  Check /proc/asound/pcm.
int tc_rx_fm_stereo_output_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer INTERNAL_FM_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Switch");
    if (!ctl) {
        /* kernel 3.0 command for hostless mode on
         * SLIMBUS_0_RX path
         */
        ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
        if (!ctl)
            goto mixer_err;
    }
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DAC4 MUX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "ON");
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

/*
   Must be able to set true 0dB to pass FM tests.
*/
int tc_rx_fm_stereo_output_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

//TODO
int tc_tx_bt_sco_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer INTERNAL_BT_SCO_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    // BT SCO Tx is device "hw:0,?".  Check /proc/asound/pcm.
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_tx_bt_sco_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    return 0;
}

int tc_rx_bt_sco_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "INTERNAL_BT_SCO_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    // BT SCO Rx is device "hw:0,4".  Check /proc/asound/pcm.
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_rx_bt_sco_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    return 0;
}

int tc_lb_afe_handset_mic_handset_earphone_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
        ctl = get_ctl(mxr, "DAC1 MUX");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_select(ctl, "RX1");
    ctl = get_ctl(mxr, "EAR PA Gain");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "POS_2_DB");
    ctl = get_ctl(mxr, "DAC1 Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Switch");
    if (!ctl) {
        /* kernel 3.0 command for hostless mode on
         * SLIMBUS_0_RX path
         */
        ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
        if (!ctl)
            goto mixer_err;
    }

    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_afe_handset_mic_handset_earphone_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_afe_headset_mic_headset_l_r_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
        if (!ctl)
      goto mixer_err;
        if (enable)
      ret = mixer_ctl_select(ctl, "One");

    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DAC4 MUX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "ON");
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX3 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Switch");
    if (!ctl) {
        /* kernel 3.0 command for hostless mode on
         * SLIMBUS_0_RX path
         */
        ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
        if (!ctl)
            goto mixer_err;
    }
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_afe_headset_mic_headset_l_r_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;

    printf("%s(%d)\n",__func__,vol);

    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX3 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_afe_spkr_phone_mic_spkr_phone_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "SLIM TX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DAC3 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "INV_RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DAC2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 67);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
    ret = mixer_ctl_set(ctl, 67);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Switch");
    if (!ctl) {
        /* kernel 3.0 command for hostless mode on
         * SLIMBUS_0_RX path
         */
        ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
        if (!ctl)
            goto mixer_err;
    }
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_afe_spkr_phone_mic_spkr_phone_vol_sitar(struct mixer *mxr, int vol)
{
    struct mixer_ctl *ctl = 0;
    uint32 ret = 0;
    printf("%s(%d)\n",__func__,vol);
        ctl = get_ctl(mxr, "RX1 Digital Volume");
        if (!ctl)
                goto mixer_err;
        ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_handset_mic_handset_earphone_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "IIR1 INP1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

        ctl = get_ctl(mxr, "EAR PA Gain");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_select(ctl, "POS_2_DB");

        ctl = get_ctl(mxr, "DAC1 MUX");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_select(ctl, "RX1");
        else
                ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "DAC1 Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 0);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_handset_mic_handset_earphone_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_headset_mic_headset_l_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    // headset mic
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
        ctl = get_ctl(mxr, "ADC2 Volume");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_set(ctl, 1);
        else
                ret = mixer_ctl_set(ctl, 0);

    // loopback
    ctl = get_ctl(mxr, "IIR1 INP1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX2 MIX1 INP2");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    // headset spkr left
    ctl = get_ctl(mxr, "RX3 MIX1 INP2");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "DAC4 MUX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "ON");
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_headset_mic_headset_l_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
        ctl = get_ctl(mxr, "RX3 Digital Volume");
        if (!ctl)
                goto mixer_err;
        ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_spkr_phone_mic_spkr_phone_l_en_sitar(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    // spkr phone mic
        ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
        if (!ctl)
                goto mixer_err;
        ret = mixer_ctl_set(ctl, val);

    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "DMIC1");

    // loopback
    ctl = get_ctl(mxr, "IIR1 INP1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    // spkr phone left
    ctl = get_ctl(mxr, "RX1 MIX1 INP2");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
        ctl = get_ctl(mxr, "DAC2 MUX");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_select(ctl, "RX1");
        else
                ret = mixer_ctl_select(ctl, "ZERO");
        ctl = get_ctl(mxr, "DAC3 MUX");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_select(ctl, "INV_RX1");
        else
                ret = mixer_ctl_select(ctl, "ZERO");
        ctl = get_ctl(mxr, "LINEOUT1 Volume");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_set(ctl, 100);
        ctl = get_ctl(mxr, "LINEOUT2 Volume");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_set(ctl, 100);
        ctl = get_ctl(mxr, "RX1 Digital Volume");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_set(ctl, 70);
        ctl = get_ctl(mxr, "PMIC SPK Gain");
        if (!ctl)
                goto mixer_err;
        if (enable)
                ret = mixer_ctl_select(ctl, "POS_20_DB");

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int tc_lb_adie_spkr_phone_mic_spkr_phone_l_vol_sitar(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

struct ftm_tc_device ftm_tc_devices_sitar[] = {
{
    NULL,
    NULL,
    PATH_NULL,
    50
},
{
    tc_tx_handset_mic_en_sitar,
    tc_tx_handset_mic_vol_sitar,
    PATH_TX,
    100
},
{
    tc_rx_handset_earphone_en_sitar,
    tc_rx_handset_earphone_vol_sitar,
    PATH_RX,
    100
},
{
    tc_tx_headset_mic_en_sitar,
    tc_tx_headset_mic_vol_sitar,
    PATH_TX,
    100
},
{
    tc_rx_headset_spkr_mono_en_sitar,
    tc_rx_headset_spkr_mono_vol_sitar,
    PATH_RX,
    50
},
{
    tc_rx_spkr_mono_diff_en_sitar,
    tc_rx_spkr_mono_diff_vol_sitar,
    PATH_RX,
    100
},
{
    tc_tx_dmic1_left_en_sitar,
    tc_tx_dmic1_left_vol_sitar,
    PATH_TX,
    100
},
{
    tc_tx_dmic1_right_en_sitar,
    tc_tx_dmic1_right_vol_sitar,
    PATH_TX,
    50
},
{
    tc_tx_dmic1_left_right_en_sitar,
    tc_tx_dmic1_left_right_vol_sitar,
    PATH_TX,
    50
},
{
    tc_tx_dmic2_left_en_sitar,
    tc_tx_dmic2_left_vol_sitar,
    PATH_TX,
    50
},
{
    tc_tx_dmic2_right_en_sitar,
    tc_tx_dmic2_right_vol_sitar,
    PATH_TX,
    50
},
{
    tc_tx_dmic2_left_right_en_sitar,
    tc_tx_dmic2_left_right_vol_sitar,
    PATH_TX,
    50
},
{
    tc_tx_analog_input1_analog_input3_en_sitar,
    tc_tx_analog_input1_analog_input3_vol_sitar,
    PATH_TX,
    50
},
{
    tc_tx_analog_input1_analog_input2_en_sitar,
    tc_tx_analog_input1_analog_input2_vol_sitar,
    PATH_TX,
    50
},
{
    tc_tx_fm_stereo_input_en_sitar,
    tc_tx_fm_stereo_input_vol_sitar,
    PATH_FM,
    50
},
{
    tc_rx_fm_stereo_output_en_sitar,
    tc_rx_fm_stereo_output_vol_sitar,
    PATH_FM,
    100
},
{
    tc_tx_bt_sco_en_sitar,
    tc_tx_bt_sco_vol_sitar,
    PATH_TX,
    50
},
{
    tc_rx_bt_sco_en_sitar,
    tc_rx_bt_sco_vol_sitar,
    PATH_RX,
    50
},
{
    tc_lb_afe_handset_mic_handset_earphone_en_sitar,
    tc_lb_afe_handset_mic_handset_earphone_vol_sitar,
    PATH_AFE_LB,
    70
},
{
    tc_lb_afe_headset_mic_headset_l_r_en_sitar,
    tc_lb_afe_headset_mic_headset_l_r_vol_sitar,
    PATH_AFE_LB,
    70
},
{
    tc_lb_afe_spkr_phone_mic_spkr_phone_en_sitar,
    tc_lb_afe_spkr_phone_mic_spkr_phone_vol_sitar,
    PATH_AFE_LB,
    70
},
// these are same device
{
    // vol 0
    tc_lb_afe_headset_mic_headset_l_r_en_sitar,
    tc_lb_afe_headset_mic_headset_l_r_vol_sitar,
    PATH_AFE_LB,
    0
},
{
    // vol 25
    tc_lb_afe_headset_mic_headset_l_r_en_sitar,
    tc_lb_afe_headset_mic_headset_l_r_vol_sitar,
    PATH_AFE_LB,
    25
},
{
    // vol 75
    tc_lb_afe_headset_mic_headset_l_r_en_sitar,
    tc_lb_afe_headset_mic_headset_l_r_vol_sitar,
    PATH_AFE_LB,
    75
},
/*ADIE loopback*/
{
    tc_lb_adie_handset_mic_handset_earphone_en_sitar,
    tc_lb_adie_handset_mic_handset_earphone_vol_sitar,
    PATH_ADIE_LB,
    70
},
{
    tc_lb_adie_headset_mic_headset_l_en_sitar,
    tc_lb_adie_headset_mic_headset_l_vol_sitar,
    PATH_ADIE_LB,
    70
},
{
    tc_lb_adie_headset_mic_headset_l_en_sitar,
    tc_lb_adie_headset_mic_headset_l_vol_sitar,
    PATH_ADIE_LB,
    70
},
{
    tc_lb_adie_spkr_phone_mic_spkr_phone_l_en_sitar,
    tc_lb_adie_spkr_phone_mic_spkr_phone_l_vol_sitar,
    PATH_ADIE_LB,
    70
},
};

int taiko_tc_tx_handset_mic1_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX7");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MultiMedia1 Mixer PRI_MI2S_TX");
    else
        ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;

    if (taiko_i2s != TRUE) {
        if (enable)
            ret = mixer_ctl_select(ctl, "ADC1");
        else
            ret = mixer_ctl_select(ctl, "ZERO");
        ctl = get_ctl(mxr, "ADC1 Volume");
        if (!ctl)
            goto mixer_err;
        if (enable)
            ret = mixer_ctl_set(ctl, 100);
    } else {
        if (enable)
            ret = mixer_ctl_select(ctl, "DMIC1");
        else
            ret = mixer_ctl_select(ctl, "ZERO");
    }
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_handset_mic1_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_handset_mic2_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    // MIC2 is set to headset on MTP
    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX7");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MultiMedia1 Mixer PRI_MI2S_TX");
    else
        ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable) {
        if (taiko_i2s == TRUE) {
            ret = mixer_ctl_select(ctl, "DEC8");
            ctl = get_ctl(mxr, "DEC8 MUX");
        } else {
            ret = mixer_ctl_select(ctl, "DEC7");
            ctl = get_ctl(mxr, "DEC7 MUX");
        }
    }
    if (!ctl)
        goto mixer_err;
    if (enable) {
        if (taiko_i2s == TRUE)
            ret = mixer_ctl_select(ctl, "DMIC2");
        else
            ret = mixer_ctl_select(ctl, "DMIC1");
    }
    if (taiko_i2s != TRUE) {
        ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
        if (!ctl)
            goto mixer_err;
        ret = mixer_ctl_set(ctl, val);
    }
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_handset_mic2_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_handset_earphone_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM RX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "PRI_MI2S_RX Audio Mixer MultiMedia1");
    else
        ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_RX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "RX1");

    ctl = get_ctl(mxr, "CLASS_H_DSM MUX");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_HPHL_RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "DAC1 Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_handset_earphone_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_headset_mic_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX7");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MultiMedia1 Mixer PRI_MI2S_TX");
    else
        ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 1);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_TX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC8");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_headset_mic_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "DEC5 Volume");
    else
        ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_headset_spkr_mono_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM RX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "PRI_MI2S_RX Audio Mixer MultiMedia1");
    else
        ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_RX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "CLASS_H_DSM MUX");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_HPHL_RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_headset_spkr_mono_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_headset_spkr_stereo_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM RX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIM RX2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "PRI_MI2S_RX Audio Mixer MultiMedia1");
    else
        ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_RX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");

    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "CLASS_H_DSM MUX");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_HPHL_RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_headset_spkr_stereo_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}


int taiko_tc_rx_headset_spkr_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM RX2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "PRI_MI2S_RX Audio Mixer MultiMedia1");
    else
        ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_RX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_headset_spkr_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_class_d_spkr(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM RX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "PRI_MI2S_RX Audio Mixer MultiMedia1");
    else
        ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 1);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_RX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX7 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "RX7 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_class_d_spkr_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX7 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_headset_spkr_left_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM RX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "PRI_MI2S_RX Audio Mixer MultiMedia1");
    else
        ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_RX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "CLASS_H_DSM MUX");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_HPHL_RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_headset_spkr_left_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_spkr_phone_mic_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX7");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MultiMedia1 Mixer PRI_MI2S_TX");
    else
        ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "DMIC1");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_spkr_phone_mic_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_spkr_phone_stereo_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM RX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIM RX2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 1);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    // BUG in the mixer lib. should set to 2 channels
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX5 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX6 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT4 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_spkr_phone_stereo_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX3 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX4 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);

    ctl = get_ctl(mxr, "RX5 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX6 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_spkr_phone_mono_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM RX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX5 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX6 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT4 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_spkr_phone_mono_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX3 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX5 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_spkr_phone_left_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM RX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_spkr_phone_left_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX3 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);

    ctl = get_ctl(mxr, "RX4 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_spkr_phone_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM RX2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX5 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX6 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT4 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_spkr_phone_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);

    ctl = get_ctl(mxr, "RX5 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);

    ctl = get_ctl(mxr, "RX6 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_lineout_right_diff_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX5 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX6 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX6 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "LINEOUT3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT4 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_lineout_right_diff_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX5 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX6 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_lineout_left_diff_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 DSM MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_INV");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_lineout_left_diff_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX3 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX4 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_linein_left_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC8");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC5");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC5 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_linein_left_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_linein_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;
    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC6");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC6 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_linein_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_audio_input3_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC9");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC9 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC3");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC3 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "MICBIAS3 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 1);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_audio_input3_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC9 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_audio_input4_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC10");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC10 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC4");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "ADC4 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "MICBIAS4 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 1);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_audio_input4_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC10 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}


int taiko_tc_tx_dmic1_left_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX7");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MultiMedia1 Mixer PRI_MI2S_TX");
    else
        ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_TX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (taiko_i2s != TRUE) {
        // MIC BIAS will vary by platform.
        ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
        if (!ctl)
            goto mixer_err;
        ret = mixer_ctl_set(ctl, val);
    } else {
        ctl = get_ctl(mxr, "DEC7 Volume");
        if (!ctl)
            goto mixer_err;
        if (enable)
            ret = mixer_ctl_set(ctl, 68);
    }
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_dmic1_left_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_dmic1_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX7");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MultiMedia1 Mixer PRI_MI2S_TX");
    else
        ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_TX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC8");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (taiko_i2s != TRUE) {
        ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
        if (!ctl)
            goto mixer_err;
        ret = mixer_ctl_set(ctl, val);
    } else {
        ctl = get_ctl(mxr, "DEC8 Volume");
        if (!ctl)
            goto mixer_err;
        if (enable)
            ret = mixer_ctl_set(ctl, 68);
    }
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_dmic1_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

// broadside
int taiko_tc_tx_dmic1_left_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX7");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX8");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MultiMedia1 Mixer PRI_MI2S_TX");
    else
        ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIM TX8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC8");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (taiko_i2s != TRUE) {
        ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
        if (!ctl)
            goto mixer_err;
        ret = mixer_ctl_set(ctl, val);
    }
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_TX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_dmic1_left_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_dmic2_left_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX7");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MultiMedia1 Mixer PRI_MI2S_TX");
    else
        ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC8");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (taiko_i2s != TRUE) {
        // This varies by platform.
        ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
        if (!ctl)
            goto mixer_err;
        ret = mixer_ctl_set(ctl, val);
    }
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_TX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_dmic2_left_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_dmic2_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX7");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MultiMedia1 Mixer PRI_MI2S_TX");
    else
        ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    // This varies by platform.
    if (taiko_i2s != TRUE) {
        ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
        if (!ctl)
            goto mixer_err;
        ret = mixer_ctl_set(ctl, val);
    }
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_TX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_dmic2_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_dmic2_left_right_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX7");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX8");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MultiMedia1 Mixer PRI_MI2S_TX");
    else
        ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "SLIM TX8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC8");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (taiko_i2s != TRUE) {
    // This varies by platform.
        ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
        if (!ctl)
            goto mixer_err;
        ret = mixer_ctl_set(ctl, val);
    }
    if (taiko_i2s == TRUE)
        ctl = get_ctl(mxr, "MI2S_TX Channels");
    else
        ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_dmic2_left_right_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_analog_input1_analog_input3_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");

    // input 1
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (!ctl)
        goto mixer_err;
    ctl = get_ctl(mxr, "ADC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);

    // input 3
    ctl = get_ctl(mxr, "SLIM TX8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC9");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC9 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC3");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (!ctl)
        goto mixer_err;
    ctl = get_ctl(mxr, "MICBIAS3 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "ADC3 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_analog_input1_analog_input3_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);

    ctl = get_ctl(mxr, "DEC9 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

//TODO (handset mic + headset mic)
int taiko_tc_tx_analog_input1_analog_input2_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    // input 1
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (!ctl)
        goto mixer_err;
    ctl = get_ctl(mxr, "ADC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);

    // analog input 2
    ctl = get_ctl(mxr, "SLIM TX8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC8");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    if (!ctl)
        goto mixer_err;
    ctl = get_ctl(mxr, "ADC2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_analog_input1_analog_input2_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_fm_stereo_input_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM RX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIM RX2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "CLASS_H_DSM MUX");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_HPHL_RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer INTERNAL_FM_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Switch");
    if (!ctl) {
        /* kernel 3.0 command for hostless mode on
         * SLIMBUS_0_RX path
         */
        ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
        if (!ctl)
            goto mixer_err;
    }
    ret = mixer_ctl_set(ctl, val);
    // FM Tx is device "hw:0,6".  Check /proc/asound/pcm.
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_fm_stereo_input_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);

    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);

    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

// FM Rx is device "hw:0,5".  Check /proc/asound/pcm.
int taiko_tc_rx_fm_stereo_output_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer INTERNAL_FM_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Switch");
    if (!ctl) {
        /* kernel 3.0 command for hostless mode on
         * SLIMBUS_0_RX path
         */
        ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
        if (!ctl)
            goto mixer_err;
    }
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM RX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIM RX2 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "CLASS_H_DSM MUX");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_HPHL_RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "Two");
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

/*
   Must be able to set true 0dB to pass FM tests.
*/
int taiko_tc_rx_fm_stereo_output_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

//TODO
int taiko_tc_tx_bt_sco_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "MultiMedia1 Mixer INTERNAL_BT_SCO_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    // BT SCO Tx is device "hw:0,?".  Check /proc/asound/pcm.
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_tx_bt_sco_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    return 0;
}

int taiko_tc_rx_bt_sco_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "INTERNAL_BT_SCO_RX Audio Mixer MultiMedia1");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    // BT SCO Rx is device "hw:0,4".  Check /proc/asound/pcm.
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_rx_bt_sco_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    return 0;
}

int taiko_tc_lb_afe_handset_mic_handset_earphone_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);
    ctl = get_ctl(mxr, "SLIM RX1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "CLASS_H_DSM MUX");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_HPHL_RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "DAC1 Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX7");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
    if (!ctl)
        goto mixer_err;

    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_lb_afe_handset_mic_handset_earphone_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_lb_afe_headset_mic_headset_l_r_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "AIF1_CAP Mixer SLIM TX7");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
        if (!ctl)
      goto mixer_err;
        if (enable)
          ret = mixer_ctl_select(ctl, "One");

    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC8");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 0);


    ctl = get_ctl(mxr, "SLIM RX1 MUX");
        if (!ctl)
      goto mixer_err;
        if (enable)
      ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
      ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIM RX2 MUX");
        if (!ctl)
      goto mixer_err;
        if (enable)
      ret = mixer_ctl_select(ctl, "AIF1_PB");
    else
      ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
        if (!ctl)
      goto mixer_err;
        if (enable)
      ret = mixer_ctl_select(ctl, "Two");

    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "CLASS_H_DSM MUX");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_HPHL_RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);

    ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
    if (!ctl)
        goto mixer_err;

    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_lb_afe_headset_mic_headset_l_r_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;

    printf("%s(%d)\n",__func__,vol);

    ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, 68);

    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_lb_afe_spkr_phone_mic_spkr_phone_l_r_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "SLIM_0_TX Channels");
        if (!ctl)
      goto mixer_err;
        if (enable)
          ret = mixer_ctl_select(ctl, "One");

    ctl = get_ctl(mxr, "SLIM TX7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC1");
    ctl = get_ctl(mxr, "DEC1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DMIC1");
    ctl = get_ctl(mxr, "MICBIAS1 CAPLESS Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "DEC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
    ret = mixer_ctl_set(ctl, 68);

    ctl = get_ctl(mxr, "SLIM_0_RX Channels");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "One");
    ctl = get_ctl(mxr, "RX3 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX4 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "LINEOUT1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 66);
    ctl = get_ctl(mxr, "LINEOUT2 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
    ret = mixer_ctl_set(ctl, 66);

    ctl = get_ctl(mxr, "SLIMBUS_DL_HL Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    ctl = get_ctl(mxr, "SLIMBUS_0_RX Port Mixer SLIM_0_TX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_lb_afe_spkr_phone_mic_spkr_phone_l_r_vol(struct mixer *mxr, int vol)
{
    printf("%s(%d)\n",__func__,vol);
    return 0;
}

int taiko_tc_lb_adie_handset_mic_handset_earphone_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    ctl = get_ctl(mxr, "ADC1 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "IIR1 INP1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "CLASS_H_DSM MUX");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_HPHL_RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "DAC1 Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_lb_adie_handset_mic_handset_earphone_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_lb_adie_headset_mic_headset_l_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    // headset mic
    ctl = get_ctl(mxr, "DEC8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);

    // loopback
    ctl = get_ctl(mxr, "IIR1 INP1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC8");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "RX1 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "CLASS_H_DSM MUX");
    if (!ctl)
        goto mixer_err;

    if (enable)
        ret = mixer_ctl_select(ctl, "DSM_HPHL_RX1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "HPHL DAC Switch");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, val);
    ctl = get_ctl(mxr, "HPHL Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_lb_adie_headset_mic_headset_l_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);

    ctl = get_ctl(mxr, "RX1 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return 0;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_lb_adie_headset_mic_headset_r_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    // headset mic
    ctl = get_ctl(mxr, "DEC8 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "ADC2");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "DEC8 Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);

    // loopback
    ctl = get_ctl(mxr, "IIR1 INP1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC8");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    // headset spkr right
    ctl = get_ctl(mxr, "RX2 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");
    ctl = get_ctl(mxr, "HPHR Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 68);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_lb_adie_headset_mic_headset_r_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "RX2 Digital Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return 0;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_lb_adie_spkr_phone_mic_spkr_phone_l_en(struct mixer *mxr, int enable)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    int val = enable ? 1 : 0;

    printf("%s(%d)\n",__func__,enable);

    // spkr phone mic
    ctl = get_ctl(mxr, "DEC7 MUX");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_select(ctl, "DMIC1");

    // loopback
    ctl = get_ctl(mxr, "IIR1 INP1 MUX");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "DEC7");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    // spkr phone left
    ctl = get_ctl(mxr, "RX7 MIX1 INP1");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_select(ctl, "IIR1");
    else
        ret = mixer_ctl_select(ctl, "ZERO");

    ctl = get_ctl(mxr, "SPK Volume");
    if (!ctl)
        goto mixer_err;
    if (enable)
        ret = mixer_ctl_set(ctl, 100);

    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}

int taiko_tc_lb_adie_spkr_phone_mic_spkr_phone_l_vol(struct mixer *mxr, int vol)
{
    uint32 ret = 0;
    struct mixer_ctl *ctl = 0;
    printf("%s(%d)\n",__func__,vol);
    ctl = get_ctl(mxr, "DEC7 Volume");
    if (!ctl)
        goto mixer_err;
    ret = mixer_ctl_set(ctl, vol);
    return ret;
mixer_err:
    DALSYS_Log_Err("%s: get_ctl() failed\n",__func__);
    return -1;
}



static struct ftm_tc_device ftm_tc_devices_taiko[] = {
{
    NULL,
    NULL,
    PATH_NULL,
    68
},
{
    taiko_tc_tx_handset_mic1_en,
    taiko_tc_tx_handset_mic1_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_handset_mic2_en,
    taiko_tc_tx_handset_mic2_vol,
    PATH_TX,
    68
},
{
    taiko_tc_rx_handset_earphone_en,
    taiko_tc_rx_handset_earphone_vol,
    PATH_RX,
    68
},
{
    taiko_tc_tx_headset_mic_en,
    taiko_tc_tx_headset_mic_vol,
    PATH_TX,

},
{
    taiko_tc_rx_headset_spkr_mono_en,
    taiko_tc_rx_headset_spkr_mono_vol,
    PATH_RX,
    68
},
{
    taiko_tc_rx_headset_spkr_stereo_en,
    taiko_tc_rx_headset_spkr_stereo_vol,
    PATH_RX,
    68
},
{
    taiko_tc_rx_headset_spkr_left_en,
    taiko_tc_rx_headset_spkr_left_vol,
    PATH_RX,
    68
},
{
    taiko_tc_rx_headset_spkr_right_en,
    taiko_tc_rx_headset_spkr_right_vol,
    PATH_RX,
    100
},
{
    taiko_tc_rx_class_d_spkr,
    taiko_tc_rx_class_d_spkr_vol,
    PATH_RX,
    50
},

{
    taiko_tc_tx_spkr_phone_mic_en,
    taiko_tc_tx_spkr_phone_mic_vol,
    PATH_TX,
    68
},
{
    taiko_tc_rx_spkr_phone_stereo_en,
    taiko_tc_rx_spkr_phone_stereo_vol,
    PATH_RX,
    68
},
{
    taiko_tc_rx_spkr_phone_mono_en,
    taiko_tc_rx_spkr_phone_mono_vol,
    PATH_RX,
    68
},
{
    taiko_tc_rx_spkr_phone_left_en,
    taiko_tc_rx_spkr_phone_left_vol,
    PATH_RX,
    68
},
{
    taiko_tc_rx_spkr_phone_right_en,
    taiko_tc_rx_spkr_phone_right_vol,
    PATH_RX,
    68
},
{
    taiko_tc_rx_lineout_right_diff_en,
    taiko_tc_rx_lineout_right_diff_vol,
    PATH_RX,
    68
},
{
    taiko_tc_rx_lineout_left_diff_en,
    taiko_tc_rx_lineout_left_diff_vol,
    PATH_RX,
    68
},
{
    taiko_tc_tx_linein_left_en,
    taiko_tc_tx_linein_left_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_linein_right_en,
    taiko_tc_tx_linein_right_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_audio_input3_en,
    taiko_tc_tx_audio_input3_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_audio_input4_en,
    taiko_tc_tx_audio_input4_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_dmic1_left_en,
    taiko_tc_tx_dmic1_left_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_dmic1_right_en,
    taiko_tc_tx_dmic1_right_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_dmic1_left_right_en,
    taiko_tc_tx_dmic1_left_right_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_dmic2_left_en,
    taiko_tc_tx_dmic2_left_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_dmic2_right_en,
    taiko_tc_tx_dmic2_right_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_dmic2_left_right_en,
    taiko_tc_tx_dmic2_left_right_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_analog_input1_analog_input3_en,
    taiko_tc_tx_analog_input1_analog_input3_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_analog_input1_analog_input2_en,
    taiko_tc_tx_analog_input1_analog_input2_vol,
    PATH_TX,
    68
},
{
    taiko_tc_tx_fm_stereo_input_en,
    taiko_tc_tx_fm_stereo_input_vol,
    PATH_FM,
    68
},
{
    taiko_tc_rx_fm_stereo_output_en,
    taiko_tc_rx_fm_stereo_output_vol,
    PATH_FM,
    100
},
{
    taiko_tc_tx_bt_sco_en,
    taiko_tc_tx_bt_sco_vol,
    PATH_TX,
    68
},
{
    taiko_tc_rx_bt_sco_en,
    taiko_tc_rx_bt_sco_vol,
    PATH_RX,
    68
},
{
    taiko_tc_lb_afe_handset_mic_handset_earphone_en,
    taiko_tc_lb_afe_handset_mic_handset_earphone_vol,
    PATH_AFE_LB,
    68
},
{
    taiko_tc_lb_afe_headset_mic_headset_l_r_en,
    taiko_tc_lb_afe_headset_mic_headset_l_r_vol,
    PATH_AFE_LB,
    68
},
{
    taiko_tc_lb_afe_spkr_phone_mic_spkr_phone_l_r_en,
    taiko_tc_lb_afe_spkr_phone_mic_spkr_phone_l_r_vol,
    PATH_AFE_LB,
    68
},
// these are same device
{
    // vol 0
    taiko_tc_lb_afe_headset_mic_headset_l_r_en,
    taiko_tc_lb_afe_headset_mic_headset_l_r_vol,
    PATH_AFE_LB,
    0
},
{
    // vol 25
    taiko_tc_lb_afe_headset_mic_headset_l_r_en,
    taiko_tc_lb_afe_headset_mic_headset_l_r_vol,
    PATH_AFE_LB,
    25
},
{
    // vol 75
    taiko_tc_lb_afe_headset_mic_headset_l_r_en,
    taiko_tc_lb_afe_headset_mic_headset_l_r_vol,
    PATH_AFE_LB,
    75
},
{
    taiko_tc_lb_adie_handset_mic_handset_earphone_en,
    taiko_tc_lb_adie_handset_mic_handset_earphone_vol,
    PATH_ADIE_LB,
    68
},
{
    taiko_tc_lb_adie_headset_mic_headset_l_en,
    taiko_tc_lb_adie_headset_mic_headset_l_vol,
    PATH_ADIE_LB,
    68
},
{
    taiko_tc_lb_adie_headset_mic_headset_r_en,
    taiko_tc_lb_adie_headset_mic_headset_r_vol,
    PATH_ADIE_LB,
    68
},
{
    taiko_tc_lb_adie_spkr_phone_mic_spkr_phone_l_en,
    taiko_tc_lb_adie_spkr_phone_mic_spkr_phone_l_vol,
    PATH_ADIE_LB,
    68
},
};

static struct ftm_tc_device ftm_tc_devices_taiko_i2s[] = {
{
    NULL,
    NULL,
    PATH_NULL,
    68
},
/* TC =1 */
{
    taiko_tc_tx_handset_mic1_en,
    taiko_tc_tx_handset_mic1_vol,
    PATH_TX,
    68
},
/* TC =2 */
{
    taiko_tc_tx_handset_mic2_en,
    taiko_tc_tx_handset_mic2_vol,
    PATH_TX,
    68
},
/* TC =3 */
{
    taiko_tc_rx_handset_earphone_en,
    taiko_tc_rx_handset_earphone_vol,
    PATH_RX,
    68
},
/* TC =4 */
{
    taiko_tc_tx_headset_mic_en,
    taiko_tc_tx_headset_mic_vol,
    PATH_TX,

},
/* TC =5 */
{
    taiko_tc_rx_headset_spkr_mono_en,
    taiko_tc_rx_headset_spkr_mono_vol,
    PATH_RX,
    68
},
/* TC =6 */
{
    taiko_tc_rx_headset_spkr_stereo_en,
    taiko_tc_rx_headset_spkr_stereo_vol,
    PATH_RX,
    68
},
/* TC = 7 */
{
    taiko_tc_rx_headset_spkr_left_en,
    taiko_tc_rx_headset_spkr_left_vol,
    PATH_RX,
    68
},
/* TC = 8 */
{
    taiko_tc_rx_headset_spkr_right_en,
    taiko_tc_rx_headset_spkr_right_vol,
    PATH_RX,
    100
},
/* TC = 9 */
{
    taiko_tc_rx_class_d_spkr,
    taiko_tc_rx_class_d_spkr_vol,
    PATH_RX,
    50
},
/* TC = 10 */
{
    taiko_tc_tx_spkr_phone_mic_en,
    taiko_tc_tx_spkr_phone_mic_vol,
    PATH_TX,
    68
},
/* TC = 11 */
{
    taiko_tc_tx_dmic1_left_en,
    taiko_tc_tx_dmic1_left_vol,
    PATH_TX,
    68
},
/* TC =12 */
{
    taiko_tc_tx_dmic1_right_en,
    taiko_tc_tx_dmic1_right_vol,
    PATH_TX,
    68
},
/* TC = 13 */
{
    taiko_tc_tx_dmic1_left_right_en,
    taiko_tc_tx_dmic1_left_right_vol,
    PATH_TX,
    68
},
/* TC = 14 */
{
    taiko_tc_tx_dmic2_left_en,
    taiko_tc_tx_dmic2_left_vol,
    PATH_TX,
    68
},
/* TC = 15 */
{
    taiko_tc_tx_dmic2_right_en,
    taiko_tc_tx_dmic2_right_vol,
    PATH_TX,
    68
},
/* TC = 16 */
{
    taiko_tc_tx_dmic2_left_right_en,
    taiko_tc_tx_dmic2_left_right_vol,
    PATH_TX,
    68
},
/* TC =  17 */
{
    taiko_tc_lb_adie_handset_mic_handset_earphone_en,
    taiko_tc_lb_adie_handset_mic_handset_earphone_vol,
    PATH_ADIE_LB,
    68
},
/* TC = 18 */
{
    taiko_tc_lb_adie_headset_mic_headset_l_en,
    taiko_tc_lb_adie_headset_mic_headset_l_vol,
    PATH_ADIE_LB,
    68
},
/* TC = 19 */
{
    taiko_tc_lb_adie_headset_mic_headset_r_en,
    taiko_tc_lb_adie_headset_mic_headset_r_vol,
    PATH_ADIE_LB,
    68
},
/* TC = 20 */
{
    taiko_tc_lb_adie_spkr_phone_mic_spkr_phone_l_en,
    taiko_tc_lb_adie_spkr_phone_mic_spkr_phone_l_vol,
    PATH_ADIE_LB,
    68
},
};

/*******************************************************************
                     Tone Playback
********************************************************************/
#define FTM_CASE_TONE_PLAY_HANDSET            0
#define FTM_CASE_TONE_PLAY_HEADSET_L            1
#define FTM_CASE_TONE_PLAY_HEADSET_R            2
#define FTM_CASE_TONE_PLAY_SPK_L                    3
#define FTM_CASE_TONE_PLAY_SPK_R                    4

#define FTM_CASE_TONE_PLAY_MI2S_SD0            5
#define FTM_CASE_TONE_PLAY_MI2S_SD1       6
#define FTM_CASE_TONE_PLAY_MI2S_SD2       7
#define FTM_CASE_TONE_PLAY_MI2S_SD3       8

#define FTM_CASE_TONE_PLAY_FM        9
#define FTM_CASE_TONE_PLAY_BT        10

/*******************************************************************
                     ADIE loopback
********************************************************************/
#define FTM_CASE_ADIE_LP_MIC1_HANDSET                                20
#define FTM_CASE_ADIE_LP_HEADSET_MIC_HEADSET_L            21
#define FTM_CASE_ADIE_LP_HEADSET_MIC_HEADSET_R            22
#define FTM_CASE_ADIE_LP_SPK_MIC_SPK_L                            23
#define FTM_CASE_ADIE_LP_SPK_MIC_SPK_R                            24

/*******************************************************************
                     PCM RECORD
********************************************************************/
#define FTM_CASE_PCM_REC_HANDSET_MIC                30
#define FTM_CASE_PCM_REC_HEADSET_MIC                31
#define FTM_CASE_PCM_REC_SPK_MIC                        32
#define FTM_CASE_PCM_REC_FM    33   /* Record 2 channels 48K stereo FM into efile */


/*******************************************************************
                     AFE loopback
********************************************************************/

/*------------ Mono cases --------------*/
#define FTM_CASE_AFE_LP_MIC1_HANDSET        40   /* handset mic -> handset spk */
#define FTM_CASE_AFE_LP_MIC1_HPH_MONO_DIFF  41   /* handset mic -> headset mono differential output */
#define FTM_CASE_AFE_LP_MIC1_HPH_L_R        42   /* handset mic -> HPH L+R simulated stereo */
#define FTM_CASE_AFE_LP_MIC1_SPK_L_R        43   /* handset mic -> speaker phone L+R simulated stereo */
#define FTM_CASE_AFE_LP_MIC1_LINEOUT_DIFF   44      /* handset mic -> lineoutput differenttial */

#define FTM_CASE_AFE_LP_MIC2_HPH_DIFF       45   /* headset mic -> headset mono differential */
#define FTM_CASE_AFE_LP_MIC2_HPH_L_R        46   /* headset mic -> headset L+R simulated stereo */
#define FTM_CASE_AFE_LP_MIC2_SPK_L_R        47   /* headset mic -> speaker phone L+R simulated stereo */
#define FTM_CASE_AFE_LP_MIC2_LINEOUT_DIFF   48   /* headset mic -> lineoutput differenttial */
#define FTM_CASE_AFE_LP_MIC2_HANDSET        49   /* headset mic -> handset earphone */

#define FTM_CASE_AFE_LP_AUXIN_HPH_DIFF      50   /* AuxIn -> headset mono differential */
#define FTM_CASE_AFE_LP_AUXIN_HPH_L_R       51   /* AuxIn -> headset L+R simulated stereo */
#define FTM_CASE_AFE_LP_AUXIN_SPK_L_R       52   /* AuxIn -> speaker phone L+R simulated stereo */
#define FTM_CASE_AFE_LP_AUXIN_LINEOUT_DIFF  53   /* AuxIn -> lineoutput differenttial */
#define FTM_CASE_AFE_LP_AUXIN_AUXOUT        54   /* AuxIn -> Auxout */
#define FTM_CASE_AFE_LP_AUXIN_HANDSET       55   /* AuxIn -> handset earphone */

#define FTM_CASE_AFE_LP_LineInLeft_HPH_DIFF      56   /* LineIn left -> headset mono differential */
#define FTM_CASE_AFE_LP_LineInLeft_HPH_L_R       57   /* LineIn left -> headset L+R simulated stereo */
#define FTM_CASE_AFE_LP_LineInLeft_SPK_L_R       58   /* LineIn left -> speaker phone L+R simulated stereo */
#define FTM_CASE_AFE_LP_LineInLeft_LINEOUT_DIFF  59   /* LineIn left -> lineoutput differenttial */
#define FTM_CASE_AFE_LP_LineInLeft_HANDSET       60   /* LineIn left -> handset earphone */

/*-----------  Analog Stereo cases ------------*/
#define FTM_CASE_AFE_LP_MIC1_TO_HPH_L_AND_AUXIN_TO_HPH_R          61 /* Handset Mic -> Headset Left; AuxIn -> headset Right */
#define FTM_CASE_AFE_LP_MIC1_TO_LINEOUT_L_AND_AUXIN_TO_LINEOUT_R  62 /* Handset Mic -> Lineout left; AuxIn -> lineout right */
#define FTM_CASE_AFE_LP_MIC1_TO_LINEOUT_L_AND_MIC2_TO_LINEOUT_R   63 /* Handset Mic ->Lienout left; Headset Mic -> lineout right */
#define FTM_CASE_AFE_LP_MIC1_TO_HPH_L_AND_MIC2_TO_HPH_R           64 /* Handset Mic ->headset Left; Headset Mic -> Headset right */

/*********  Mono Digital *********/
#define FTM_CASE_AFE_LP_L_DMIC_HANDSET         65  /* Digital Mic (AuxIn) left -> Handset */
#define FTM_CASE_AFE_LP_L_DMIC_HPH_L           66  /* Digital Mic (AuxIn) left -> Headset Left*/
#define FTM_CASE_AFE_LP_L_DMIC_SPK_L           67  /* Digital Mic (AuxIn) left -> Speaker phone Left*/
#define FTM_CASE_AFE_LP_L_DMIC_AUXOUT          68  /* Digital Mic (AuxIn) left -> AuxOut*/

/*------------  Stereo Digital cases ------------*/
#define FTM_CASE_AFE_LP_DMIC_L_TO_HPH_L_AND_DMIC_R_TO_HPH_R          69 /* Digital Mic left -> headset left; digital Mic right -> headset right */
#define FTM_CASE_AFE_LP_DMIC_L_TO_LINEOUT_L_AND_DMIC_R_TO_LINEOUT_R  70  /* Digital Mic left -> Lineout left; digital Mic right -> Lineout right */

/*------------  FM cases ------------*/
#define FTM_CASE_AFE_LP_FM_LOOPBACK_TO_HEADSET   71   /* FM loopback: FM -> Headset  */
#define FTM_CASE_AFE_LP_FM_LOOPBACK_TO_SPK       72   /* FM loopback: FM -> speaker phone  */

/*------------  BT cases ------------*/
#define FTM_CASE_BT_LOOPBACK_TO_HANDSET  73    /* Handset Tx->BT Rx; BT Tx-> Handset Rx */

#define FTM_CASE_PCM_ANALOG_INPUT_5    74

/*==================================================================================================
                                      LOCAL VARIABLES
==================================================================================================*/
/*---- Global variables for FTM dispatcher ----*/
static AUDIO_FTM_DRIVER_HANDLE_T  g_active_ftm_driver_attach_hdl=NULL;
static AUDIO_FTM_CLIENT_HANDLE_T  g_active_ftm_driver_open_hdl=NULL;

static DAL_ATOMIC                 g_total_ftm_instances=0;
static uint16                     g_volume_level=(uint16)VOL_DEFAULT;
static AUD_FTM_PATH_MATRIX_T      g_current_path=
    {FTM_CASE_TONE_PLAY_HANDSET,   AUDIO_FTM_IN_INVALID, AUDIO_FTM_OUT_HANDSET,   TRUE,FALSE,FALSE,FALSE};

static DALBOOL                    g_bRecord_to_file=FALSE;
uint8                             *g_PCM_rec_buf=NULL;
uint32                            ftm_pcm_buffer_size;

static AUD_FTM_PATH_MATRIX_T aud_ftm_loopback_path_matrix[]=
{
/* Tone Play */
  {FTM_CASE_TONE_PLAY_HANDSET,   AUDIO_FTM_IN_INVALID, AUDIO_FTM_OUT_HANDSET,   TRUE,FALSE,FALSE,FALSE,1},
  {FTM_CASE_TONE_PLAY_HEADSET_L, AUDIO_FTM_IN_INVALID, AUDIO_FTM_OUT_HEADSET_L, TRUE,FALSE,FALSE,FALSE,1},
  {FTM_CASE_TONE_PLAY_HEADSET_R, AUDIO_FTM_IN_INVALID, AUDIO_FTM_OUT_HEADSET_R, TRUE,FALSE,FALSE,FALSE,1},
  {FTM_CASE_TONE_PLAY_SPK_L,     AUDIO_FTM_IN_INVALID, AUDIO_FTM_OUT_SPEAKER1_L,TRUE,FALSE,FALSE,FALSE,1},
  {FTM_CASE_TONE_PLAY_SPK_R,     AUDIO_FTM_IN_INVALID, AUDIO_FTM_OUT_SPEAKER1_R,TRUE,FALSE,FALSE,FALSE,1},
  {FTM_CASE_TONE_PLAY_MI2S_SD0,  AUDIO_FTM_IN_INVALID, AUDIO_FTM_OUT_MI2S_SD0,  TRUE,FALSE,FALSE,FALSE,2},
  {FTM_CASE_TONE_PLAY_MI2S_SD1,  AUDIO_FTM_IN_INVALID, AUDIO_FTM_OUT_MI2S_SD1,  TRUE,FALSE,FALSE,FALSE,2},
  {FTM_CASE_TONE_PLAY_MI2S_SD2,  AUDIO_FTM_IN_INVALID, AUDIO_FTM_OUT_MI2S_SD2,  TRUE,FALSE,FALSE,FALSE,2},
  {FTM_CASE_TONE_PLAY_MI2S_SD3,  AUDIO_FTM_IN_INVALID, AUDIO_FTM_OUT_MI2S_SD3,  TRUE,FALSE,FALSE,FALSE,2},
  {FTM_CASE_TONE_PLAY_FM,        AUDIO_FTM_IN_INVALID, AUDIO_FTM_OUT_FM,        TRUE,FALSE,FALSE,FALSE,2},
  {FTM_CASE_TONE_PLAY_BT,        AUDIO_FTM_IN_INVALID, AUDIO_FTM_OUT_BT,        TRUE,FALSE,FALSE,FALSE,1},

/* ADIE loopback */
  {FTM_CASE_ADIE_LP_MIC1_HANDSET,          AUDIO_FTM_IN_HANDSET_MIC_ADIE_LP, AUDIO_FTM_OUT_HANDSET_ADIE_LP,    FALSE,FALSE,TRUE,FALSE,1},
  {FTM_CASE_ADIE_LP_HEADSET_MIC_HEADSET_L, AUDIO_FTM_IN_HEADSET_MIC_ADIE_LP, AUDIO_FTM_OUT_HEADSET_L_ADIE_LP,  FALSE,FALSE,TRUE,FALSE,1},
  {FTM_CASE_ADIE_LP_HEADSET_MIC_HEADSET_R, AUDIO_FTM_IN_HEADSET_MIC_ADIE_LP, AUDIO_FTM_OUT_HEADSET_R_ADIE_LP,  FALSE,FALSE,TRUE,FALSE,1},
  {FTM_CASE_ADIE_LP_SPK_MIC_SPK_L,         AUDIO_FTM_IN_HANDSET_MIC_ADIE_LP, AUDIO_FTM_OUT_SPK_L_ADIE_LP,FALSE,FALSE,TRUE,FALSE,1},
  {FTM_CASE_ADIE_LP_SPK_MIC_SPK_R,         AUDIO_FTM_IN_HANDSET_MIC_ADIE_LP, AUDIO_FTM_OUT_SPK_R_ADIE_LP,FALSE,FALSE,TRUE,FALSE,1},

/* PCM Record */
  {FTM_CASE_PCM_REC_HANDSET_MIC,   AUDIO_FTM_IN_HANDSET_MIC1, AUDIO_FTM_OUT_INVALID,   FALSE,FALSE,FALSE,TRUE,1},
  {FTM_CASE_PCM_REC_HEADSET_MIC,   AUDIO_FTM_IN_HEADSET_MIC,  AUDIO_FTM_OUT_INVALID,   FALSE,FALSE,FALSE,TRUE,1},
  {FTM_CASE_PCM_REC_SPK_MIC,       AUDIO_FTM_IN_SPEAKER_MIC,  AUDIO_FTM_OUT_INVALID,   FALSE,FALSE,FALSE,TRUE,1},
  {FTM_CASE_PCM_REC_FM,            AUDIO_FTM_IN_FM,           AUDIO_FTM_OUT_INVALID,   FALSE,FALSE,FALSE,TRUE,2},

/* AFE Loopback */
/*------------ Handset Mono cases --------------*/
  {FTM_CASE_AFE_LP_MIC1_HANDSET,          AUDIO_FTM_IN_HANDSET_MIC1, AUDIO_FTM_OUT_HANDSET,                 FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_MIC1_HPH_MONO_DIFF,    AUDIO_FTM_IN_HANDSET_MIC1, AUDIO_FTM_OUT_HEADSET_MONO_DIFF_L_R,   FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_MIC1_HPH_L_R,          AUDIO_FTM_IN_HANDSET_MIC1, AUDIO_FTM_OUT_HEADSET_MONO_L_R,        FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_MIC1_SPK_L_R,          AUDIO_FTM_IN_HANDSET_MIC1, AUDIO_FTM_OUT_SPEAKER1_MONO_L_R,       FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_MIC1_LINEOUT_DIFF,     AUDIO_FTM_IN_HANDSET_MIC1, AUDIO_FTM_OUT_SPEAKER1_MONO_DIFF_L_R,  FALSE,TRUE,FALSE,FALSE,1},

/*------------ Headset Mono cases --------------*/
  {FTM_CASE_AFE_LP_MIC2_HPH_DIFF,         AUDIO_FTM_IN_HEADSET_MIC, AUDIO_FTM_OUT_HEADSET_MONO_DIFF_L_R, FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_MIC2_HPH_L_R,          AUDIO_FTM_IN_HEADSET_MIC, AUDIO_FTM_OUT_HEADSET_MONO_L_R,      FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_MIC2_SPK_L_R,          AUDIO_FTM_IN_HEADSET_MIC, AUDIO_FTM_OUT_SPEAKER1_MONO_L_R,     FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_MIC2_LINEOUT_DIFF,     AUDIO_FTM_IN_HEADSET_MIC, AUDIO_FTM_OUT_SPEAKER1_MONO_DIFF_L_R,FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_MIC2_HANDSET,          AUDIO_FTM_IN_HEADSET_MIC, AUDIO_FTM_OUT_HANDSET,               FALSE,TRUE,FALSE,FALSE,1},

/*------------ AuxIn Mono cases --------------*/
  {FTM_CASE_AFE_LP_AUXIN_HPH_DIFF,        AUDIO_FTM_IN_AUXIN, AUDIO_FTM_OUT_HEADSET_MONO_DIFF_L_R, FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_AUXIN_HPH_L_R,         AUDIO_FTM_IN_AUXIN, AUDIO_FTM_OUT_HEADSET_MONO_L_R,      FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_AUXIN_SPK_L_R,         AUDIO_FTM_IN_AUXIN, AUDIO_FTM_OUT_SPEAKER1_MONO_L_R,     FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_AUXIN_LINEOUT_DIFF,    AUDIO_FTM_IN_AUXIN, AUDIO_FTM_OUT_SPEAKER1_MONO_DIFF_L_R,FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_AUXIN_AUXOUT,          AUDIO_FTM_IN_AUXIN, AUDIO_FTM_OUT_AUXOUT,                FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_AUXIN_HANDSET,         AUDIO_FTM_IN_AUXIN, AUDIO_FTM_OUT_HANDSET,               FALSE,TRUE,FALSE,FALSE,1},

/*------------ LineIn Mono cases --------------*/
  {FTM_CASE_AFE_LP_LineInLeft_HPH_DIFF,     AUDIO_FTM_IN_LINEIN_L, AUDIO_FTM_OUT_HEADSET_MONO_DIFF_L_R, FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_LineInLeft_HPH_L_R,      AUDIO_FTM_IN_LINEIN_L, AUDIO_FTM_OUT_HEADSET_MONO_L_R,      FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_LineInLeft_SPK_L_R,      AUDIO_FTM_IN_LINEIN_L, AUDIO_FTM_OUT_SPEAKER1_MONO_L_R,     FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_LineInLeft_LINEOUT_DIFF, AUDIO_FTM_IN_LINEIN_L, AUDIO_FTM_OUT_SPEAKER1_MONO_DIFF_L_R,FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_LineInLeft_HANDSET,      AUDIO_FTM_IN_LINEIN_L, AUDIO_FTM_OUT_HANDSET,               FALSE,TRUE,FALSE,FALSE,1},

/*-----------  Analog Stereo cases ------------*/
  {FTM_CASE_AFE_LP_MIC1_TO_HPH_L_AND_AUXIN_TO_HPH_R,          AUDIO_FTM_IN_MIC1_AUXIN, AUDIO_FTM_OUT_HEADSET_STEREO_L_R,   FALSE,TRUE,FALSE,FALSE,2},
  {FTM_CASE_AFE_LP_MIC1_TO_LINEOUT_L_AND_AUXIN_TO_LINEOUT_R,  AUDIO_FTM_IN_MIC1_AUXIN, AUDIO_FTM_OUT_SPEAKER1_STEREO_L_R,  FALSE,TRUE,FALSE,FALSE,2},
  {FTM_CASE_AFE_LP_MIC1_TO_LINEOUT_L_AND_MIC2_TO_LINEOUT_R,   AUDIO_FTM_IN_MIC1_MIC2,  AUDIO_FTM_OUT_SPEAKER1_STEREO_L_R,  FALSE,TRUE,FALSE,FALSE,2},
  {FTM_CASE_AFE_LP_MIC1_TO_HPH_L_AND_MIC2_TO_HPH_R,           AUDIO_FTM_IN_MIC1_MIC2,  AUDIO_FTM_OUT_HEADSET_STEREO_L_R,   FALSE,TRUE,FALSE,FALSE,2},

/*-----------  Mono Digital cases -----------*/
  {FTM_CASE_AFE_LP_L_DMIC_HANDSET,        AUDIO_FTM_IN_DMIC1, AUDIO_FTM_OUT_HANDSET,   FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_L_DMIC_HPH_L,          AUDIO_FTM_IN_DMIC1, AUDIO_FTM_OUT_HEADSET_L, FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_L_DMIC_SPK_L,          AUDIO_FTM_IN_DMIC1, AUDIO_FTM_OUT_SPEAKER1_L,FALSE,TRUE,FALSE,FALSE,1},
  {FTM_CASE_AFE_LP_L_DMIC_AUXOUT,         AUDIO_FTM_IN_DMIC1, AUDIO_FTM_OUT_AUXOUT,    FALSE,TRUE,FALSE,FALSE,1},

/*------------  Stereo Digital cases ------------*/
  {FTM_CASE_AFE_LP_DMIC_L_TO_HPH_L_AND_DMIC_R_TO_HPH_R,         AUDIO_FTM_IN_DMIC1_DMIC2, AUDIO_FTM_OUT_HEADSET_STEREO_L_R,  FALSE,TRUE,FALSE,FALSE,2},
  {FTM_CASE_AFE_LP_DMIC_L_TO_LINEOUT_L_AND_DMIC_R_TO_LINEOUT_R, AUDIO_FTM_IN_DMIC1_DMIC2, AUDIO_FTM_OUT_SPEAKER1_STEREO_L_R, FALSE,TRUE,FALSE,FALSE,2},

/*------------  FM cases ------------*/
  {FTM_CASE_AFE_LP_FM_LOOPBACK_TO_HEADSET,      AUDIO_FTM_IN_FM, AUDIO_FTM_OUT_HEADSET_STEREO_L_R,    FALSE,TRUE,FALSE,FALSE,2},
  {FTM_CASE_AFE_LP_FM_LOOPBACK_TO_SPK,          AUDIO_FTM_IN_FM, AUDIO_FTM_OUT_SPEAKER1_STEREO_L_R,   FALSE,TRUE,FALSE,FALSE,2},

/*------------  BT cases ------------*/
  {FTM_CASE_BT_LOOPBACK_TO_HANDSET,             AUDIO_FTM_IN_MIC1_BT, AUDIO_FTM_OUT_HANDSET_BT,       FALSE,TRUE,FALSE,FALSE,1}

};

static char  *pcm_capture_file   = "/data/ftm_pcm_record.wav";
snd_use_case_mgr_t *uc_mgr;

/*==================================================================================================
                                     GLOBAL VARIABLES
==================================================================================================*/


/*==================================================================================================
                                 LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

static DALBOOL ftm_audio_set_path( word device);
static DALBOOL ftm_audio_set_volume(uint8 new_vol);

static DALBOOL ftm_audio_start_adie_loopback();
static DALBOOL ftm_audio_stop_adie_loopback();

static DALBOOL ftm_audio_start_tone(uint16  lo_freq, uint16  hi_freq);
static DALBOOL ftm_audio_stop_tone();

static DALBOOL ftm_audio_start_dsp_loopback();
static DALBOOL ftm_audio_stop_dsp_loopback();

static DALBOOL aud_ftm_common_start();
static DALBOOL aud_ftm_common_stop();

static DALBOOL ftm_audio_start_pcm_play(AUDIO_FTM_PCM_RATE_T  rate, AUDIO_FTM_CHN_T  channel);
static DALBOOL ftm_audio_stop_pcm_play();

static DALBOOL ftm_audio_stop_pcm_capture();
static DALBOOL ftm_audio_start_file_play(struct test_params *params);
static DALBOOL ftm_audio_stop_file_play();

static DALBOOL ftm_audio_checkif_active_session()
{
   DALBOOL ret;
     int   instances;

   instances=DALSYS_atomic_read(&g_total_ftm_instances);
     ret= (instances > 0)? TRUE:FALSE;
   return ret;
}

/*
snd_use_case_set(uc_mgr, "_verb", SND_USE_CASE_VERB_HIFI);
snd_use_case_set(uc_mgr, "_enadev", SND_USE_CASE_DEV_SPEAKER);
snd_use_case_set(uc_mgr, "_swdev/Speaker", SND_USE_CASE_DEV_HEADSET);
snd_use_case_set(uc_mgr, "_verb", SND_USE_CASE_VERB_INACTIVE);
snd_use_case_set(uc_mgr, "_enadev", SND_USE_CASE_DEV_HEADSET);
*/

static DALBOOL ftm_audio_set_path(
  word test_case
)
{
    uint32 ret;
    struct mixer *mxr = 0;
    struct mixer_ctl *ctl = 0;
    //static char* curr_uc_rx = SND_USE_CASE_DEV_EARPIECE;
    //static char* curr_uc_tx = SND_USE_CASE_DEV_HANDSET;
    //snd_use_case_mgr_open(&uc_mgr, "snd_soc_msm");
    //snd_use_case_set(uc_mgr, "_verb", SND_USE_CASE_VERB_HIFI);

    printf("%s: %d\n", __func__, __LINE__);
    if (!g_config_test) {
        mxr = mixer_open("/dev/snd/controlC0");
        if (!mxr) {
            DALSYS_Log_Err("Mixer_open() failed \n");
            return FALSE;
    }
    }
    if ((test_case >= AUDIO_FTM_TC_NUM_DEVS)||(test_case == 0))
    {
        DALSYS_Log_Err("%s: invalid test case device (%d)\n", __func__,test_case);
                if (mxr != NULL)
           mixer_close(mxr);
        return FALSE;
    }

    if (!g_config_test) {
        if (ftm_tc_devices[test_case].path_type == PATH_RX) {
            if (g_curr_rx_device >= 0)
                ftm_tc_devices[g_curr_rx_device].enable(mxr, 0);
        }
        else if(ftm_tc_devices[test_case].path_type == PATH_TX) {
            if (g_curr_tx_device >= 0)
                ftm_tc_devices[g_curr_tx_device].enable(mxr, 0);
        }
        if (g_curr_afe_lb_device >= 0)
            ftm_tc_devices[g_curr_afe_lb_device].enable(mxr, 0);

        if (g_curr_adie_lb_device >= 0)
            ftm_tc_devices[g_curr_adie_lb_device].enable(mxr, 0);

        if (g_curr_fm_device >= 0)
            ftm_tc_devices[g_curr_fm_device].enable(mxr, 0);
        //g_curr_alsa_device_name =
    }

    sleep(1);

    if (g_config_test || !ftm_tc_devices[test_case].enable(mxr, 1)) {
        switch(ftm_tc_devices[test_case].path_type)
        {
        case PATH_RX:
            g_curr_rx_device = test_case;
            break;
        case PATH_TX:
            g_curr_tx_device = test_case;
            break;
        case PATH_AFE_LB:
            g_curr_afe_lb_device = test_case;
            break;
        case PATH_ADIE_LB:
            g_curr_adie_lb_device = test_case;
            break;
        case PATH_FM:
            g_curr_fm_device = test_case;
            break;
        default:
            DALSYS_Log_Err("%s: invalid device path (%d)\n",__func__,ftm_tc_devices[test_case].path_type);
        }
    } else {
        DALSYS_Log_Err("%s: failed to set device path\n",__func__);
    }
    g_curr_device = test_case;

    sleep(1);
    if (!g_config_test)
        mixer_close(mxr);
    //snd_use_case_mgr_close(uc_mgr);

    return TRUE;
}


static DALBOOL ftm_audio_set_volume(
  uint8 new_vol
)
{
    struct mixer *mxr = 0;

    printf("%s: %d\n",__func__,__LINE__);

    mxr = mixer_open("/dev/snd/controlC0");
    if (!mxr){
        DALSYS_Log_Err("mixer_open() failed\n");
        return FALSE;
    }

    if (g_curr_device == AUDIO_FTM_TC_RX_FM_STEREO_OUTPUT)
        ftm_tc_devices[AUDIO_FTM_TC_RX_FM_STEREO_OUTPUT].set_volume(mxr, new_vol);
    else if (g_curr_rx_device >= 0)
        ftm_tc_devices[g_curr_rx_device].set_volume(mxr, new_vol);

    mixer_close(mxr);

    return TRUE;
}

static DALBOOL aud_ftm_common_start()
{
    AUDIO_FTM_STS_T res;

    res=Audio_FTM_Open(
        g_active_ftm_driver_attach_hdl,
        AUD_FTM_ACCESS_MODE_CTRL,
        AUD_FTM_SHAREMODE_EXCLU,
        &g_active_ftm_driver_open_hdl);

    if((res != AUDIO_FTM_SUCCESS) || (g_active_ftm_driver_open_hdl == NULL)) {
        g_active_ftm_driver_open_hdl=NULL;
        printf("%s: open failed\n",__func__);
        return FALSE;
    }

    res=Audio_FTM_IOControl(
        g_active_ftm_driver_open_hdl,
        IOCTL_AUDIO_FTM_START,
        NULL,
        0,
        NULL,
        0,
        NULL);

    if(res != AUDIO_FTM_SUCCESS)
    {
        printf("%s: ioctl failed\n",__func__);
        return FALSE;
    }

    AddRef(&g_total_ftm_instances);
    return TRUE;
}

static DALBOOL aud_ftm_common_stop()
{
    AUDIO_FTM_STS_T res;

    res=Audio_FTM_IOControl(
        g_active_ftm_driver_open_hdl,
        IOCTL_AUDIO_FTM_STOP,
        NULL,
        0,
        NULL,
        0,
        NULL);

    if(res != AUDIO_FTM_SUCCESS)
    {
        printf("%s: stop failed\n",__func__);
        return FALSE;
    }

    printf("%s: %d\n",__func__,__LINE__);
    res=Audio_FTM_Close(g_active_ftm_driver_open_hdl);

    if(res != AUDIO_FTM_SUCCESS)
    {
        return FALSE;
    }

    g_active_ftm_driver_open_hdl = NULL;

    res=Audio_FTM_Detach(g_active_ftm_driver_attach_hdl);

    if(res != AUDIO_FTM_SUCCESS)
    {
        printf("%s: detach failed\n",__func__);
        return FALSE;
    }

    g_active_ftm_driver_attach_hdl = NULL;
    Release(&g_total_ftm_instances);
    return TRUE;
}

static DALBOOL ftm_audio_start_adie_loopback()
{
    AUDIO_FTM_STS_T res;
    AUD_FTM_DISPATCH_PARAM_T ftm_param;
    memset(&ftm_param,0,sizeof(ftm_param));

    ftm_param.pcm_lp_param.path.inpath=g_current_path.iPath;
    ftm_param.pcm_lp_param.path.outpath=g_current_path.oPath;
    ftm_param.pcm_lp_param.gain=(uint16)g_volume_level;

    res=Audio_FTM_Attach(
        AUDIO_FTM_METHOD_PCM_LOOPBACK,
        &ftm_param,
        &g_active_ftm_driver_attach_hdl);
    if(res != AUDIO_FTM_SUCCESS)
    {
        printf("%s: attach failed res = %d\n",__func__,res);
        return FALSE;
    }

    return aud_ftm_common_start();
}

static DALBOOL ftm_audio_stop_adie_loopback()
{
    return aud_ftm_common_stop();
}

static DALBOOL ftm_audio_start_tone(uint16  lo_freq, uint16  hi_freq)
{
    AUDIO_FTM_STS_T res;

    AUD_FTM_DISPATCH_PARAM_T ftm_param;
    memset(&ftm_param,0,sizeof(ftm_param));

    ftm_param.tone_play_param.path.inpath=g_current_path.iPath;
    ftm_param.tone_play_param.path.outpath=g_current_path.oPath;
    ftm_param.tone_play_param.gain=(uint16)g_volume_level;
    ftm_param.tone_play_param.dtmf_hi=hi_freq;
    ftm_param.tone_play_param.dtmf_low=lo_freq;
    ftm_param.tone_play_param.channel=g_current_path.channel;
    ftm_param.tone_play_param.device=g_deviceplayback;

    res = Audio_FTM_Attach(
        AUDIO_FTM_METHOD_TONE_PLAY,
        &ftm_param,
        &g_active_ftm_driver_attach_hdl);
    if (res != AUDIO_FTM_SUCCESS) {
        printf("%s: attach failed res = %d\n",__func__,res);
        return FALSE;
    }

    return aud_ftm_common_start();
}

static DALBOOL ftm_audio_stop_tone()
{
    return aud_ftm_common_stop();
}

static DALBOOL ftm_audio_start_dsp_loopback()
{
    AUDIO_FTM_STS_T res;
    AUD_FTM_DISPATCH_PARAM_T ftm_param;
    memset(&ftm_param,0,sizeof(ftm_param));

    ftm_param.afe_lp_param.path.inpath=g_current_path.iPath;
    ftm_param.afe_lp_param.path.outpath=g_current_path.oPath;
    ftm_param.afe_lp_param.gain=(uint16)g_volume_level;
    ftm_param.afe_lp_param.channel=g_current_path.channel;
    ftm_param.afe_lp_param.capturedevice=g_devicecapture;
    ftm_param.afe_lp_param.playbackdevice=g_deviceplayback;
    printf("\n %s: cdevice %d pdevice %d", __func__,
               g_devicecapture, g_deviceplayback);
    res=Audio_FTM_Attach(
        AUDIO_FTM_METHOD_AFE_LOOPBACK,
        &ftm_param,
        &g_active_ftm_driver_attach_hdl);
    if(res != AUDIO_FTM_SUCCESS)
    {
        printf("%s: attach failed res = %d\n",__func__,res);
        return FALSE;
    }

    return aud_ftm_common_start();
}

static DALBOOL ftm_audio_stop_dsp_loopback()
{
    return aud_ftm_common_stop();
}

static DALBOOL ftm_audio_start_pcm_capture(uint8 seconds)
{
  AUDIO_FTM_STS_T res;
  AUD_FTM_DISPATCH_PARAM_T ftm_param;

  ftm_param.pcm_rec_param.path.inpath=g_current_path.iPath;
  ftm_param.pcm_rec_param.path.outpath=g_current_path.oPath;
  ftm_param.pcm_rec_param.gain=(uint16)g_volume_level;
  ftm_param.pcm_rec_param.channel=g_current_path.channel;
  ftm_param.pcm_rec_param.device=g_devicecapture;

  /* one frame per 20ms, 160 samples will be generated */
//  ftm_param.pcm_rec_param.size=no_of_pcm_buffers*160*sizeof(uint16);
//    ftm_pcm_buffer_size=ftm_param.pcm_rec_param.size;

  /* collect data into flash e-file */
  ftm_param.pcm_rec_param.pFileName=pcm_capture_file;
  ftm_param.pcm_rec_param.pData_buf=NULL;
  g_bRecord_to_file=TRUE;

  res=Audio_FTM_Attach(
    AUDIO_FTM_METHOD_PCM_CAPTURE,
    &ftm_param,
    &g_active_ftm_driver_attach_hdl);
  if(res != AUDIO_FTM_SUCCESS)
  {
        printf("%s: attach failed res = %d\n",__func__,res);
    return FALSE;
  }

//  return aud_ftm_common_start();
  aud_ftm_common_start();
  sleep(seconds);   /* record 2 seconds */
  return ftm_audio_stop_pcm_capture();
}

static DALBOOL ftm_audio_stop_pcm_capture()
{
   return aud_ftm_common_stop();
}

static DALBOOL ftm_audio_start_file_play(struct test_params *params)
{
    AUDIO_FTM_STS_T res;
    if (!params) {
        printf("\n%s Invalid params %d", __func__, __LINE__);
        return FALSE;
    }
    AUD_FTM_DISPATCH_PARAM_T ftm_param;
    memset(&ftm_param,0,sizeof(ftm_param));
    ftm_param.pcm_play_param.path.inpath=g_current_path.iPath;
    ftm_param.pcm_play_param.path.outpath=g_current_path.oPath;
    ftm_param.pcm_play_param.channel = 2;
    ftm_param.pcm_play_param.gain =(uint16)g_volume_level;
    ftm_param.pcm_play_param.pFileName = params->filename;
    ftm_param.pcm_play_param.device = g_deviceplayback;
    printf("%s: call attach res = %d\n",__func__,res);
    res = Audio_FTM_Attach(
        AUDIO_FTM_METHOD_PCM_PLAY,
        &ftm_param,
        &g_active_ftm_driver_attach_hdl);
    if (res != AUDIO_FTM_SUCCESS) {
        printf("%s: attach failed res = %d\n",__func__,res);
        return FALSE;
    }
    return aud_ftm_common_start();
}
static DALBOOL ftm_audio_stop_file_play()
{
    return aud_ftm_common_stop();
}

static int test_ftm_tone_play_comm(uint32 path, struct test_params *params)
{
    ftm_audio_pkt_type  cmd;
    ftm_audio_response_type *pkt = NULL;
    int result = 0;
    struct mixer *mxr = 0;

    if (ftm_tc_devices[path].path_type != PATH_RX) {
        printf("error: trying to play tone on non-Rx device\n");
        return -1;
    }

    if (!ftm_audio_set_path(path)) {
        result = 1;
        goto done;
    }
    if (!params) {
        if (!ftm_audio_start_tone(300, 2000)) {
            result = 2;
            goto done;
        }
    } else {
        if (params->filename) {
            printf("\n Playback of file %s", params->filename);
            ftm_audio_start_file_play(params);
        } else if (!ftm_audio_start_tone(params->fl, params->fh)) {
            result = 2;
            goto done;
        }
    }

    if (!params) {
        mxr = mixer_open("/dev/snd/controlC0");
        if (!mxr){
            printf("error: failed to open mixer\n");
            result = 3;
            goto done;
        }

        if (ftm_tc_devices[path].set_volume(mxr, ftm_tc_devices[path].tc_vol)) {
            result = 4;
            goto done;
        }

        mixer_close(mxr);
    }
    if (!params)
        sleep(3);
    else {
        if (params)
            sleep(params->duration);
    }
    if (!params || (params && !params->filename)) {
        printf("\n Stop the tone now");
        if (!ftm_audio_stop_tone()) {
            result = 5;
            goto done;
        }
    } else if (params && params->filename) {
        ftm_audio_stop_file_play();
    }
done:
    return result;
}

int start_ftm_pcm_fm_comm(uint32 path)
{
    ftm_audio_pkt_type  cmd;
    struct mixer *mxr = 0;
    int result = 0;

    if (!ftm_audio_set_path(path)) {
        result = 1;
        goto done;
    }

    mxr = mixer_open("/dev/snd/controlC0");
    if (!mxr) {
        printf("error: failed to open mixer\n");
        result = 3;
        goto done;
    }

    audio_ftm_fm_hostless_en(1);

    mixer_close(mxr);
done:
    return result;
}

int stop_ftm_pcm_fm_comm()
{
    audio_ftm_fm_hostless_en(0);
    return 0;
}

int test_ftm_pcm_fm_comm(uint32 path, struct test_params *params)
{
    struct mixer *mxr = 0;
    int result = 0;
    char g_fm_stop[128];

    if (!ftm_audio_set_path(path)) {
        result = 1;
        goto done;
    }
    if (!params) {
        mxr = mixer_open("/dev/snd/controlC0");
        if (!mxr) {
            printf("error: failed to open mixer\n");
            result = 3;
            goto done;
        }

        if (ftm_tc_devices[path].set_volume(mxr, ftm_tc_devices[path].tc_vol)) {
            result = 4;
            goto done;
        }
    }
    if (params)
        audio_ftm_fm_device_set(g_devicecapture, g_deviceplayback);
    audio_ftm_fm_hostless_en(1);

    /* for MDM property_get API not available. Retain old change
       for it. For 8960, it runs till user stops by setting
       prop ftm.fm_stop to true.
         */
#ifdef _ANDROID_
    //FM runs until setprop is done for ftm.fm_stop to true
    do {
        property_get("ftm.fm_stop", g_fm_stop, "0");
    } while (strcmp("true", g_fm_stop) != 0);
    //reset the prop so that next time FM runs until this prop is set
    property_set("ftm.fm_stop", "0");
#else
    sleep(60);
#endif

    audio_ftm_fm_hostless_en(0);
    if (!params) {
        mixer_close(mxr);
    } else
      audio_ftm_fm_device_set(-1, -1);

done:
    return result;
}

int test_ftm_afe_loopback_comm(uint32 path,  struct test_params *params)
{
    ftm_audio_pkt_type  cmd;
    struct mixer *mxr = 0;
    int result = 0;

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_SET_PATH;
    cmd.audio_params.device=path;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_DSP_LOOPBACK;
    cmd.audio_params.on_off = ON;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    if (!params) {
        mxr = mixer_open("/dev/snd/controlC0");
        if (!mxr) {
            printf("error: failed to open mixer\n");
            result = 3;
            goto done;
        }

        if (ftm_tc_devices[path].set_volume(mxr, ftm_tc_devices[path].tc_vol)) {
            result = 4;
            goto done;
        }

        mixer_close(mxr);
    }
    if (!params)
        sleep(30);
    else
        sleep(params->duration);

    cmd.audio_params.on_off = OFF;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);
    sleep(1);
done:
    return result;
}

int test_ftm_adie_loopback_comm(uint32 path, struct test_params *params)
{
    ftm_audio_pkt_type  cmd;
    struct mixer *mxr = 0;
    int result = 0;

    if(ftm_audio_set_path(path) != TRUE)
    {
        result = FTM_AUDIO_DEVICE_NOT_PRESENT;
    }

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_PCM_LOOPBACK;
    cmd.audio_params.on_off = ON;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    if (!params) {
        mxr = mixer_open("/dev/snd/controlC0");
        if (!mxr){
            printf("error: failed to open mixer\n");
            result = 3;
            goto done;
        }

        if (ftm_tc_devices[path].set_volume(mxr, ftm_tc_devices[path].tc_vol)) {
            result = 4;
            goto done;
        }

        mixer_close(mxr);
    }
    if (!params) {
        sleep(30);
    } else {
        sleep(params->duration);
    }

    cmd.audio_params.on_off = OFF;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);
    sleep(1);
done:
    return result;
}

void test_ftm_audio_volume(int32 vol1, int32 vol2, int32 vol3)
{

    ftm_audio_pkt_type  cmd;

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_SET_PATH;
    cmd.audio_params.device=FTM_CASE_TONE_PLAY_HANDSET;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_TONES_PLAY;
    cmd.audio_params.tone_params.lo_freq=300;
    cmd.audio_params.tone_params.hi_freq=2000;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);
    sleep(5);

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_SET_VOLUME;
    cmd.audio_params.volume=vol1;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);
    sleep(5);

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_SET_VOLUME;
    cmd.audio_params.volume=vol2;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);
    sleep(5);

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_SET_VOLUME;
    cmd.audio_params.volume=vol3;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);
    sleep(5);

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_TONES_STOP;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);
    sleep(2);
}


int test_ftm_pcm_record_comm(uint32 path, struct test_params *params)
{
    ftm_audio_pkt_type  cmd;
    int result = 0;
    struct mixer *mxr = 0;

    cmd.composite_header.ftm_hdr.cmd_id = FTM_AUDIO_SET_PATH;
    cmd.audio_params.device = path;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    if (!params) {
        mxr = mixer_open("/dev/snd/controlC0");
        if (!mxr){
            printf("error: failed to open mixer\n");
            result = 3;
            goto done;
        }

        if (ftm_tc_devices[path].set_volume(mxr, ftm_tc_devices[path].tc_vol)) {
            result = 4;
            goto done;
        }
        mixer_close(mxr);
    }

    if (!params)
        ftm_audio_start_pcm_capture(2);
    else
        ftm_audio_start_pcm_capture(params->duration);
done:
    return result;
}

/**
  @brief Main FTM audio command handler.

  This function can be considered the main() of FTM audio functionality.
  It receives FTM commands from the FTM main dispatcher (PC FTM tool ->
  DIAG -> PC USB -> Phone USB -> DIAG -> FTM -> here) and engages various
  audio system drivers to perform actions corresponding to the FTM audio
  command sent.

  @param cmd_ptr The FTM audio DIAG packet structure.
  @return The DIAG response packet.
*/
PACKED void * ftm_audio_dispatch( PACKED void * request, uint16 length )
{
    ftm_audio_pkt_type * cmd_ptr = (ftm_audio_pkt_type *)request;
    char  result;
    ftm_audio_response_type * new_pkt=NULL;
    AUDIO_FTM_OUTPUT_PATH_ENUM_T x;
    result=FTM_AUDIO_COMMAND_SUCCESS;

    switch( cmd_ptr->composite_header.ftm_hdr.cmd_id )
    {
    case FTM_AUDIO_SET_PATH:
        printf("\n Set Path to device: %d\n", cmd_ptr->audio_params.device);

        if (cmd_ptr->audio_params.device == AUDIO_FTM_TC_RX_FM_STEREO_OUTPUT)
        {
            start_ftm_pcm_fm_comm(cmd_ptr->audio_params.device);
            break;
        }
        if( ftm_audio_set_path(cmd_ptr->audio_params.device) != TRUE)
        {
            result = FTM_AUDIO_DEVICE_NOT_PRESENT;
        }
        break;

    case FTM_AUDIO_SET_VOLUME:

        printf("\n Set Volume to: %d\n",cmd_ptr->audio_params.volume);

        if(ftm_audio_set_volume(cmd_ptr->audio_params.volume) != TRUE)
        {
            result = FTM_AUDIO_COMMAND_FAILURE;
        }
        break;

    case FTM_AUDIO_PCM_LOOPBACK:

        printf("\n PCM loopback to status: %d\n",cmd_ptr->audio_params.on_off );

        if (g_curr_device <= 0)
            break;

        if(( cmd_ptr->audio_params.on_off == ON ) && (ftm_audio_checkif_active_session() == FALSE)
            && (ftm_tc_devices[g_curr_device].path_type == PATH_ADIE_LB))
        {
            if( ftm_audio_start_adie_loopback() == FALSE )
            {
                result = FTM_AUDIO_COMMAND_FAILURE;
            }
        }
        else if(( cmd_ptr->audio_params.on_off == OFF ) && (ftm_audio_checkif_active_session()== TRUE))
        {
            if( ftm_audio_stop_adie_loopback()== FALSE )
            {
                result = FTM_AUDIO_COMMAND_FAILURE;
            }
        }
        else
        {
            printf("%s: failed\n",__func__);
            result = FTM_AUDIO_COMMAND_FAILURE;
        }
        break;

    case FTM_AUDIO_TONES_PLAY:

        printf("\n Tone Play Start: h_freq=%d  l_freq=%d\n", cmd_ptr->audio_params.tone_params.hi_freq,
            cmd_ptr->audio_params.tone_params.lo_freq);
        if(!ftm_audio_checkif_active_session() && g_current_path.bALLOW_TONE_PLAY)
        {
            printf("%s: ->ftm_audio_start_tone()\n",__func__);
            if(!ftm_audio_start_tone(
                cmd_ptr->audio_params.tone_params.lo_freq,
                cmd_ptr->audio_params.tone_params.hi_freq))
            {
                DALSYS_Log_Err("%s: failed\n",__func__);
                result = FTM_AUDIO_COMMAND_FAILURE;
            }
        }
        break;

    case FTM_AUDIO_TONES_STOP:

        printf("\n Tone play stop \n");
        if (g_curr_device == AUDIO_FTM_TC_RX_FM_STEREO_OUTPUT)
        {
            struct mixer *mxr = 0;
            mxr = mixer_open("/dev/snd/controlC0");
            if (!mxr){
                printf("%s: error: failed to open mixer\n",__func__);
                return -1;
            }
            stop_ftm_pcm_fm_comm();
            ftm_tc_devices[g_curr_device].enable(mxr, 0);
            mixer_close(mxr);
            break;
        }
        if(ftm_audio_checkif_active_session())
        {
            if(!ftm_audio_stop_tone())
            {
                DALSYS_Log_Err("%s: ftm_audio_stop_tone() failed\n",__func__);
                result = FTM_AUDIO_COMMAND_FAILURE;
            }
        }
        break;

    case FTM_AUDIO_DSP_LOOPBACK:

        printf("\n AFE loopback to status: %d\n",cmd_ptr->audio_params.on_off);

        if (g_curr_device <= 0) {
            printf("\n In correct device");
            break;
        }

        if(( cmd_ptr->audio_params.on_off == ON ) && (!ftm_audio_checkif_active_session())
            && (ftm_tc_devices[g_curr_device].path_type == PATH_AFE_LB))
        {
            ftm_audio_start_dsp_loopback();
        }
        else if(( cmd_ptr->audio_params.on_off == OFF ) && (ftm_audio_checkif_active_session()))
        {
            ftm_audio_stop_dsp_loopback();
        }
        break;

    case FTM_AUDIO_NS_CONTROL:
        /* used for PCM capture in LA, record PCM data is not sent to PC any more, it is in target /data/*/
        ftm_audio_start_pcm_capture(2);
        break;

    case FTM_AUDIO_PCM_ENABLE:
    case FTM_AUDIO_GET_PCM_SAMPLES:
    case FTM_AUDIO_PCM_DISABLE:

        printf("\n Click \"Noise Suppresion Enable\" instead, this key is not used for PCM Record \n");
        result = FTM_AUDIO_COMMAND_FAILURE;
        break;

    default:
        printf("%s: unknown command (%d)\n",__func__,cmd_ptr->composite_header.ftm_hdr.cmd_id);
        result=FTM_AUDIO_COMMAND_FAILURE;
        break;
    }

send_diag:

    if( new_pkt == NULL )
    {
        new_pkt = (ftm_audio_response_type *) audio_ftm_diagpkt_subsys_alloc(
            (diagpkt_subsys_id_type)DIAG_SUBSYS_FTM,
            FTM_AUDIO_C,
            sizeof(ftm_audio_response_type)
            );
    }

    if( new_pkt != NULL )
    {
        new_pkt->result=result;
    }

    return new_pkt;
}

int execute_test_case(int test_case, int codec, FILE *fp, int vol, int fl,
    int fh, int duration, char *file_name)
{
    int result = 0;
    struct mixer *mxr = 0;
    struct test_params params, *paraminfo = NULL;

        fprintf(stderr, "size of ftm_tc_devices_tabla = %d\n",
            sizeof(ftm_tc_devices_tabla)/sizeof(ftm_tc_devices_tabla[0]));
    fprintf(stderr, "size of ftm_tc_devices_sitar = %d\n",
            sizeof(ftm_tc_devices_sitar)/sizeof(ftm_tc_devices_sitar[0]));
    fprintf(stderr, "size of ftm_tc_devices_taiko = %d\n",
        sizeof(ftm_tc_devices_taiko)/sizeof(ftm_tc_devices_taiko[0]));
    if (codec == CODEC_TABLA)
        memcpy(ftm_tc_devices, ftm_tc_devices_tabla,
        sizeof(ftm_tc_devices_tabla));
    else if (codec == CODEC_SITAR)
        memcpy(ftm_tc_devices, ftm_tc_devices_sitar,
        sizeof(ftm_tc_devices_sitar));
    else if (codec == CODEC_TAIKO)
        memcpy(ftm_tc_devices, ftm_tc_devices_taiko,
        sizeof(ftm_tc_devices_taiko));

    else if ((codec == CODEC_TAIKO_I2S) || (codec == CODEC_TOMTOM_I2S)) {
        memcpy(ftm_tc_devices, ftm_tc_devices_taiko_i2s,
        sizeof(ftm_tc_devices_taiko_i2s));
        taiko_i2s =  TRUE;
    } else if (!fp) {
        printf("\n Invalid test case config or unkown target");
        return -EINVAL;
    }
    g_devicecapture = g_deviceplayback = -1;
    if (fp){
        paraminfo = &params;
        params.duration = duration;
        params.volume = vol;
        params.enable = 1;
        params.fp = fp;
        params.fl = fl;
        params.fh = fh;
        params.test_case = test_case;
        params.filename = file_name;
        params.type_of_test = -1;
        printf("\n Call parse function to enable");
        parse(&params);
        if (params.type_of_test == -1 || params.type_of_test > PATH_FM ||
            params.type_of_test == PATH_NULL) {
            printf("\n Invalid Test case %d", __LINE__);
            return -EINVAL;
        }
        test_case = params.type_of_test;
        ftm_tc_devices[test_case].path_type = params.type_of_test;
        fseek(params.fp, 0 ,SEEK_SET);
    }

    switch (ftm_tc_devices[test_case].path_type)
    {
    case PATH_RX:
        result = test_ftm_tone_play_comm(test_case, paraminfo);
        break;
    case PATH_TX:
        result = test_ftm_pcm_record_comm(test_case, paraminfo);
        break;
    case PATH_FM:
        result = test_ftm_pcm_fm_comm(test_case, paraminfo);
        break;
    case PATH_AFE_LB:
        result = test_ftm_afe_loopback_comm(test_case, paraminfo);
        break;
    case PATH_ADIE_LB:
        result = test_ftm_adie_loopback_comm(test_case, paraminfo);
        break;
    }

    if (!fp) {
        mxr = mixer_open("/dev/snd/controlC0");
        if (!mxr){
            printf("error: failed to open mixer\n");
            return -1;
        }
        ftm_tc_devices[test_case].enable(mxr, 0);
        mixer_close(mxr);
        taiko_i2s = FALSE;
    } else {
        printf("\n Call parse function to disable");
        params.enable = 0;
        parse(&params);
    }
    return result;
}

#ifdef __cplusplus
}
#endif
