#ifdef __cplusplus
extern "C" {
#endif

/**

  @file   audio_ftm_dispatch.c
  @brief  AUDIO FTM Dispatcher
====================================================================================================
Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

====================================================================================================
                                        INCLUDE FILES
==================================================================================================*/

#include "event.h"
#include "msg.h"
#include "log.h"

#include "diag_lsm.h"
#include "diagpkt.h"
#include "diagcmd.h"
#include "diag.h"
#include <errno.h>
//#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

/* FTM-specific interfaces and definitions */
#include "DALSYS_common.h"
#include "ftm_audio_dispatch.h"
#include "ftm_audio_diag_dispatch.h"

/* Audio FTM driver */
#include "audio_ftm_driver.h"
#include "audio_ftm_diag_mem.h"

/*==================================================================================================
                                     Test Path IDs
==================================================================================================*/


/*==================================================================================================
                                     LOCAL CONSTANTS
==================================================================================================*/

#define RECORD_ONE_SECOND  50     /* frame numbers for 8K PCM capture in one second */
#define RECORD_MAX_BUF_SIZE    16000  /* Buffer allocated for collecting record data */

/*==================================================================================================
                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/
typedef union
{
  AUD_FTM_TONE_PLAY_PARAM_T  tone_play_param;
  AUD_FTM_PCM_LP_PARAM_T     pcm_lp_param;
  AUD_FTM_AFE_LP_PARAM_T     afe_lp_param;
  AUD_FTM_PCM_REC_PARAM_T    pcm_rec_param;
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


/*==================================================================================================
                                        LOCAL MACROS
==================================================================================================*/

/*******************************************************************
                     Tone Playback
********************************************************************/

#define FTM_CASE_TONE_PLAY_HANDSET              0
#define FTM_CASE_TONE_PLAY_HEADSET_L            1
#define FTM_CASE_TONE_PLAY_HEADSET_R            2
#define FTM_CASE_TONE_PLAY_SPK_L                    3
#define FTM_CASE_TONE_PLAY_SPK_R                    4

#define FTM_CASE_TONE_PLAY_MI2S_SD0          5
#define FTM_CASE_TONE_PLAY_MI2S_SD1       6
#define FTM_CASE_TONE_PLAY_MI2S_SD2       7
#define FTM_CASE_TONE_PLAY_MI2S_SD3       8

#define FTM_CASE_TONE_PLAY_FM                          9
#define FTM_CASE_TONE_PLAY_BT                          10

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
#define FTM_CASE_PCM_REC_FM                           33   /* Record 2 channels 48K stereo FM into efile */


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

/*---- utility help functions ----*/
static DALBOOL find_match_path(int32 test_case,
    AUDIO_FTM_INPUT_PATH_ENUM_T *in, AUDIO_FTM_OUTPUT_PATH_ENUM_T *out,
    DALBOOL *bTonePlay, DALBOOL *bAFE_lp, DALBOOL *bAdie_lp, DALBOOL *bPCM_rec, uint16 *chnl);

static DALBOOL aud_ftm_common_start();
static DALBOOL aud_ftm_common_stop();

static DALBOOL ftm_audio_start_pcm_play(AUDIO_FTM_PCM_RATE_T  rate, AUDIO_FTM_CHN_T  channel);
static DALBOOL ftm_audio_stop_pcm_play();

static DALBOOL ftm_audio_stop_pcm_capture();

/*==================================================================================================
                                     LOCAL FUNCTIONS
==================================================================================================*/


/*================================================================
  @brief check if any active session exist

  @param  void

  @return: TRUE - has active session; otherwise no active session
==================================================================*/
static DALBOOL ftm_audio_checkif_active_session()
{
   DALBOOL ret;
     int   instances;

   instances=DALSYS_atomic_read(&g_total_ftm_instances);
     ret= (instances > 0)? TRUE:FALSE;
   return ret;
}

/*================================================================
  @brief Find out the matched path(test case) and get the path configure information

  @param  test_case: Input - FTM test case enumeration
          in:        Output - input device ID
          out:       Output - output device ID
          bTonePlay: Output - TRUE: TonePlay is allowed; else not
          bAFE_lp:   Output - TRUE: AFE  loopback is allowed; else not
          bAdie_lp:  Output - TRUE: Adie loopback is allowed; else not
          bPCM_rec:  Output - TRUE: PCM record is allowed; else not

  @return: TRUE - one matching is found; otherwise no matching is found
==================================================================*/

static DALBOOL find_match_path(int32 test_case,
    AUDIO_FTM_INPUT_PATH_ENUM_T *in, AUDIO_FTM_OUTPUT_PATH_ENUM_T *out,
    DALBOOL *bTonePlay, DALBOOL *bAFE_lp, DALBOOL *bAdie_lp, DALBOOL *bPCM_rec, uint16 *channel)
{
  DALBOOL ret=FALSE;
  int16  i, size;
  AUD_FTM_PATH_MATRIX_T *po;

  size=sizeof(aud_ftm_loopback_path_matrix)/sizeof(AUD_FTM_PATH_MATRIX_T);

  for(i=0; i<size; i++)
  {
    po=&aud_ftm_loopback_path_matrix[i];
    if( po->ftm_case == test_case)
      break;
  }
  if(i >= size) return FALSE;

  *in=po->iPath;
  *out=po->oPath;
  *bTonePlay=po->bALLOW_TONE_PLAY;
  *bAFE_lp=po->bALLOW_AFE_LOOPBACK;
    *bAdie_lp=po->bALLOW_ADIE_LOOPBACK;
  *bPCM_rec=po->bALLOW_PCM_REC;
    *channel=po->channel;

  return TRUE;
}

/*================================================================
  @brief validate the path and cache the path setting
  @param  test_case: Input - test cases enumerated number

  @return: TRUE - user selected paths are valid; otherwise non-valid
==================================================================*/
static DALBOOL ftm_audio_set_path(
  word test_case
)
{
     uint32 ret;
     uint32  inpath,outpath;
         uint16  chnl;
     DALBOOL bAdieLP_Allowed, bAfeLP_Allowed,bTonePlay_Allowed,bPCM_Rec_Allowed;

     ret=find_match_path(test_case,&inpath,&outpath,&bTonePlay_Allowed,&bAfeLP_Allowed,&bAdieLP_Allowed,&bPCM_Rec_Allowed,&chnl);
     if(ret == TRUE)
     {
       g_current_path.iPath=inpath;
       g_current_path.oPath=outpath;
             g_current_path.bALLOW_TONE_PLAY=bTonePlay_Allowed;
       g_current_path.bALLOW_AFE_LOOPBACK=bAfeLP_Allowed;
       g_current_path.bALLOW_ADIE_LOOPBACK=bAdieLP_Allowed;
       g_current_path.bALLOW_PCM_REC=bPCM_Rec_Allowed;
             g_current_path.ftm_case=test_case;
             g_current_path.channel=chnl;
     }
         else
         {
       DALSYS_Log_Err("Invalid Test Case, please select right audio path\n");
         }

     return ret;
}


/*================================================================
  @brief  cache the device volume setting if no active session;
          change device volume if active session is available

  @param  new_vol: Input - new volume

  @return: TRUE - volume is changed in success; otherwise failure
==================================================================*/
static DALBOOL ftm_audio_set_volume(
  uint8 new_vol
)
{
  AUDIO_FTM_STS_T res;

    if(new_vol >= VOL_MAX)  new_vol=VOL_MAX;
    g_volume_level=(uint16)new_vol;  /*cache the new volume setting */

    if(ftm_audio_checkif_active_session() == TRUE)
      /* apply new volume to active device  */
    {
    res=Audio_FTM_IOControl(
    g_active_ftm_driver_open_hdl,
    IOCTL_AUDIO_FTM_CHG_VOL,
    (uint8 *)&g_volume_level,
    sizeof(g_volume_level),
    NULL,
    0,
    NULL);

    if(AUDIO_FTM_SUCCESS != res)  return FALSE;
    }

  return TRUE;
}

/*================================================================
  @brief  a common subroutine for starting a FTM test operation.
          This function extracts the common part in test operation
          START process.

  @param  void

  @return: TRUE - success; otherwise failure
==================================================================*/
static DALBOOL aud_ftm_common_start()
{
    AUDIO_FTM_STS_T res;

    res=Audio_FTM_Open(
    g_active_ftm_driver_attach_hdl,
    AUD_FTM_ACCESS_MODE_CTRL,
    AUD_FTM_SHAREMODE_EXCLU,
    &g_active_ftm_driver_open_hdl);

    if((res != AUDIO_FTM_SUCCESS) || (g_active_ftm_driver_open_hdl == NULL))
    {
      g_active_ftm_driver_open_hdl=NULL;
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
      return FALSE;
    }

    AddRef(&g_total_ftm_instances);

    if(ftm_audio_set_volume((uint16)g_volume_level) != TRUE)
    {
      return FALSE;
    }

    return TRUE;
}

/*================================================================
  @brief  a common subroutine for stopping a FTM test operation.
          This function extracts the common part in test operation
          STOP process.

  @param  void

  @return: TRUE - success; otherwise failure
==================================================================*/
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
    return FALSE;
  }

  res=Audio_FTM_Close(g_active_ftm_driver_open_hdl);

  if(res != AUDIO_FTM_SUCCESS)
  {
    return FALSE;
  }

  g_active_ftm_driver_open_hdl=NULL;

  res=Audio_FTM_Detach(g_active_ftm_driver_attach_hdl);

  if(res != AUDIO_FTM_SUCCESS)
  {
    return FALSE;
  }

  g_active_ftm_driver_attach_hdl=NULL;
  Release(&g_total_ftm_instances);
    return TRUE;
}

/*================================================================
  @brief  start Adie PCM loopback.

  @param  void

  @return: TRUE - success; otherwise failure
==================================================================*/
static DALBOOL ftm_audio_start_adie_loopback()
{
  AUDIO_FTM_STS_T res;
  AUD_FTM_DISPATCH_PARAM_T ftm_param; //={0};
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
    return FALSE;
  }

  return aud_ftm_common_start();
}

/*================================================================
  @brief  stop Adie PCM loopback.

  @param  void

  @return: TRUE - success; otherwise failure
==================================================================*/
static DALBOOL ftm_audio_stop_adie_loopback()
{
   return aud_ftm_common_stop();
}

/*================================================================
  @brief  start Tone Playback.

  @param  lo_freq: Input - low frequency
          hi_freq: Input - high frequency

  @return: TRUE - success; otherwise failure
==================================================================*/
static DALBOOL ftm_audio_start_tone(uint16  lo_freq, uint16  hi_freq)
{
  AUDIO_FTM_STS_T res;

  AUD_FTM_DISPATCH_PARAM_T ftm_param; //={0};
   memset(&ftm_param,0,sizeof(ftm_param));

  ftm_param.tone_play_param.path.inpath=g_current_path.iPath;
  ftm_param.tone_play_param.path.outpath=g_current_path.oPath;
  ftm_param.tone_play_param.gain=(uint16)g_volume_level;
  ftm_param.tone_play_param.dtmf_hi=hi_freq;
  ftm_param.tone_play_param.dtmf_low=lo_freq;
    ftm_param.tone_play_param.channel=g_current_path.channel;

  res=Audio_FTM_Attach(
    AUDIO_FTM_METHOD_TONE_PLAY,
    &ftm_param,
    &g_active_ftm_driver_attach_hdl);
   if(res != AUDIO_FTM_SUCCESS)
  {
    return FALSE;
  }

  return aud_ftm_common_start();
}

/*================================================================
  @brief  Stop Tone Playback.

  @param  void

  @return: TRUE - success; otherwise failure
==================================================================*/
static DALBOOL ftm_audio_stop_tone()
{
   return aud_ftm_common_stop();
}

/*================================================================
  @brief  start AFE loopback

  @param  void

  @return: TRUE - success; otherwise failure
==================================================================*/
static DALBOOL ftm_audio_start_dsp_loopback()
{
  AUDIO_FTM_STS_T res;
  AUD_FTM_DISPATCH_PARAM_T ftm_param; //={0};
   memset(&ftm_param,0,sizeof(ftm_param));

  ftm_param.afe_lp_param.path.inpath=g_current_path.iPath;
  ftm_param.afe_lp_param.path.outpath=g_current_path.oPath;
  ftm_param.afe_lp_param.gain=(uint16)g_volume_level;
  ftm_param.afe_lp_param.channel=g_current_path.channel;

  res=Audio_FTM_Attach(
    AUDIO_FTM_METHOD_AFE_LOOPBACK,
    &ftm_param,
    &g_active_ftm_driver_attach_hdl);
  if(res != AUDIO_FTM_SUCCESS)
  {
    return FALSE;
  }

  return aud_ftm_common_start();
}

/*================================================================
  @brief  stop DSP loopback

  @param  void

  @return: TRUE - success; otherwise failure
==================================================================*/
static DALBOOL ftm_audio_stop_dsp_loopback()
{
   return aud_ftm_common_stop();
}


/*================================================================
  @brief  start PCM capture

  @param  no_of_pcm_buffers: Input - number of frames to capture
                             for 8K PCM, one frame is generated per 20ms


  @return: TRUE - success; otherwise failure
==================================================================*/
static DALBOOL ftm_audio_start_pcm_capture(uint8 seconds)
{
  AUDIO_FTM_STS_T res;
  AUD_FTM_DISPATCH_PARAM_T ftm_param={0};

  ftm_param.pcm_rec_param.path.inpath=g_current_path.iPath;
  ftm_param.pcm_rec_param.path.outpath=g_current_path.oPath;
  ftm_param.pcm_rec_param.gain=(uint16)g_volume_level;
  ftm_param.pcm_rec_param.channel=g_current_path.channel;

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
    return FALSE;
  }

//  return aud_ftm_common_start();
  aud_ftm_common_start();
  sleep(seconds);   /* record 2 seconds */
  return ftm_audio_stop_pcm_capture();
}

static ftm_audio_response_type * ftm_audio_get_pcm( ftm_audio_pkt_type *cmd_ptr )
{
 /* data was captured into flash file already */
     return NULL;
}

static DALBOOL ftm_audio_stop_pcm_capture()
{
   return aud_ftm_common_stop();
}


/*==================================================================================================
 ***************************************************************************************************
                                       GLOBAL FUNCTIONS
 ***************************************************************************************************
==================================================================================================*/

/*****************   Test functions for debug purpose (Start) ****************/
static void test_ftm_tone_play_comm( uint32 path)
{

    ftm_audio_pkt_type  cmd;

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_SET_PATH;
    cmd.audio_params.device=path;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_TONES_PLAY;
    cmd.audio_params.tone_params.lo_freq=300;
    cmd.audio_params.tone_params.hi_freq=2000;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    sleep(3);

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_TONES_STOP;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);
    sleep(1);
}


void test_ftm_audio_tone_play(int32 tone_case)
{
    switch(tone_case)
    {
    case 0:
        test_ftm_tone_play_comm(FTM_CASE_TONE_PLAY_HANDSET);
        break;

    case 1:
        test_ftm_tone_play_comm(FTM_CASE_TONE_PLAY_HEADSET_L);
        break;

    case 2:
        test_ftm_tone_play_comm(FTM_CASE_TONE_PLAY_HEADSET_R);
        break;

    case 3:
        test_ftm_tone_play_comm(FTM_CASE_TONE_PLAY_SPK_L);
        break;

    case 4:
        test_ftm_tone_play_comm(FTM_CASE_TONE_PLAY_SPK_R);
        break;
    }

}

void test_ftm_afe_loopback_comm(uint32 path)
{
  ftm_audio_pkt_type  cmd;

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_SET_PATH;
  cmd.audio_params.device=path;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_DSP_LOOPBACK;
  cmd.audio_params.on_off = ON;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    sleep(30*1);

  cmd.audio_params.on_off = OFF;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);
    sleep(1);

}


void test_ftm_audio_afe_loopback( int32 play_case)
{
    switch(play_case)
    {
    case 0:
        test_ftm_afe_loopback_comm(FTM_CASE_AFE_LP_MIC1_HANDSET);
        break;

    case 1:
        test_ftm_afe_loopback_comm(FTM_CASE_AFE_LP_MIC2_HPH_L_R);
        break;

    case 2:
        test_ftm_afe_loopback_comm(FTM_CASE_AFE_LP_MIC1_SPK_L_R);
        break;

    }
}

void test_ftm_adie_loopback_comm(uint32 path)
{
  ftm_audio_pkt_type  cmd;

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_SET_PATH;
  cmd.audio_params.device=path;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_PCM_LOOPBACK;
  cmd.audio_params.on_off = ON;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    sleep(30*1);

  cmd.audio_params.on_off = OFF;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);
    sleep(1);

}


void test_ftm_audio_adie_loopback( int32 lp_case)
{
    switch(lp_case)
    {
        case 0:
            test_ftm_adie_loopback_comm(FTM_CASE_ADIE_LP_MIC1_HANDSET);
            break;

        case 1:
            test_ftm_adie_loopback_comm(FTM_CASE_ADIE_LP_HEADSET_MIC_HEADSET_L);
            break;

        case 2:
            test_ftm_adie_loopback_comm(FTM_CASE_ADIE_LP_HEADSET_MIC_HEADSET_R);
            break;

        case 3:
            test_ftm_adie_loopback_comm(FTM_CASE_ADIE_LP_SPK_MIC_SPK_L);
            break;

        case 4:
            test_ftm_adie_loopback_comm(FTM_CASE_ADIE_LP_SPK_MIC_SPK_R);
            break;

    }
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


void test_ftm_pcm_record_comm(uint32 path)
{
    ftm_audio_pkt_type  cmd;

    cmd.composite_header.ftm_hdr.cmd_id = FTM_AUDIO_SET_PATH;
    cmd.audio_params.device = path;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    ftm_audio_start_pcm_capture(2);

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_PCM_ENABLE;

    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    sleep(2);

    cmd.composite_header.ftm_hdr.cmd_id=FTM_AUDIO_PCM_DISABLE;
    ftm_audio_dispatch((PACKED void *)&cmd, 0);

    sleep(1);
}

void test_ftm_pcm_record( int32 lp_case)
{
    switch(lp_case)
    {
        case 0:
            test_ftm_pcm_record_comm(FTM_CASE_PCM_REC_HANDSET_MIC);
            break;

        case 1:
            test_ftm_pcm_record_comm(FTM_CASE_PCM_REC_SPK_MIC);
            break;

    }

}

/*****************  Test functions for debug purpose (End) ****************/


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

    if(( cmd_ptr->audio_params.on_off == ON ) && (ftm_audio_checkif_active_session() == FALSE)
       && (g_current_path.bALLOW_ADIE_LOOPBACK))
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
        result = FTM_AUDIO_COMMAND_FAILURE;
    }
    break;

  case FTM_AUDIO_TONES_PLAY:

       printf("\n Tone Play Start: h_freq=%d  l_freq=%d\n", cmd_ptr->audio_params.tone_params.hi_freq,
            cmd_ptr->audio_params.tone_params.lo_freq);

        if(!ftm_audio_checkif_active_session() && g_current_path.bALLOW_TONE_PLAY)
    {
      if(!ftm_audio_start_tone(
        cmd_ptr->audio_params.tone_params.lo_freq,
        cmd_ptr->audio_params.tone_params.hi_freq))
      {
        result = FTM_AUDIO_COMMAND_FAILURE;
      }
    }
    break;

  case FTM_AUDIO_TONES_STOP:

  printf("\n Tone play stop \n");

    if(ftm_audio_checkif_active_session())
    {
      if(!ftm_audio_stop_tone())
      {
        result = FTM_AUDIO_COMMAND_FAILURE;
      }
    }
    break;

  case FTM_AUDIO_DSP_LOOPBACK:

  printf("\n AFE loopback to status: %d\n",cmd_ptr->audio_params.on_off);

    if(( cmd_ptr->audio_params.on_off == ON ) && (!ftm_audio_checkif_active_session())
      && (g_current_path.bALLOW_AFE_LOOPBACK))
    {
      ftm_audio_start_dsp_loopback();
    }
    else if(( cmd_ptr->audio_params.on_off == OFF ) && (ftm_audio_checkif_active_session()))
    {
      ftm_audio_stop_dsp_loopback();
    }
    break;

case FTM_AUDIO_NS_CONTROL:   /* used for PCM capture in LA, record PCM data is not sent to PC any more, it is in target /data/*/

    if(( cmd_ptr->audio_params.on_off == ON ) && !ftm_audio_checkif_active_session() && g_current_path.bALLOW_PCM_REC)
    {
      printf("\n PCM capture enabling \n");

      if(!ftm_audio_start_pcm_capture(2))  /* record 2 seconds to file */
        {
        result = FTM_AUDIO_COMMAND_FAILURE;
        }
        else
         printf("Recorded file is at /data/ftm_pcm_record.wav\n");
    }
    else if( cmd_ptr->audio_params.on_off == OFF )
    {
       printf("\n PCM capture is disabled \n");  /* record was stopped in ftm_audio_start_pcm_capture() */
    }
    else
        result = FTM_AUDIO_COMMAND_FAILURE;

    break;

case  FTM_AUDIO_PCM_ENABLE:
case  FTM_AUDIO_GET_PCM_SAMPLES:
case  FTM_AUDIO_PCM_DISABLE:

  printf("\n Click \"Noise Suppresion Enable\" instead, this key is not used for PCM Record \n");
       result = FTM_AUDIO_COMMAND_FAILURE;
    break;

  default:
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
} /* end ftm_audio_dispatch */




/*================================================================================================*/


#ifdef __cplusplus
}
#endif
