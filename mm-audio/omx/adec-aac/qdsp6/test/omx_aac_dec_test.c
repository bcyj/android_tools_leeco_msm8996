/*
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */


/*
    An Open max test application ....
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/ioctl.h>
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "QOMX_AudioExtensions.h"
#include "QOMX_AudioIndexExtensions.h"
#ifdef AUDIOV2
#include "control.h"
#endif
#include "pthread.h"
#include <signal.h>

#include <stdint.h>
#include <linux/ioctl.h>
#include <linux/msm_audio.h>

#ifdef MSM_ALSA
#include <omx_alsa_utils.h>
#endif

#define SAMPLE_RATE 16000
#define STEREO      2
uint32_t samplerate = 16000;
uint32_t channels = 2;
uint32_t pcmplayback = 1;
uint32_t tunnel      = 0;
uint32_t filewrite   = 0;
uint32_t configbufsize = 0;
uint32_t raw_config_bytes = 0;
int dual_mono_mode = OMX_AUDIO_DUAL_MONO_MODE_INVALID;
uint32_t sbr_ps_enabled = 0; //if 0, then both not enabled. if 1 only sbr enabled, if 2 both enabled.
uint32_t first_read = 1;
uint32_t aac_channels = 2;
uint32_t force_multiaac_codec = 0;
uint32_t devid = 1; // Default Device-Speaker
uint32_t volumelevel = 30;
uint32_t mix_coeff = 0; //Default to ISO coeff. ISO = 0, ARIB = 1.

char *devtble[]={"handset_rx","speaker_stereo_rx","headset_stereo_rx"};

#define DEBUG_PRINT printf
uint32_t flushinprogress = 0;
uint32_t bsac = 0;
int start_done = 0;



int m_pcmdrv_fd;

#ifdef MSM_ALSA
struct mixer *mxr;
struct pcm *pcmCntxt;
#endif /*MSM_ALSA*/

/************************************************************************/
/*                #DEFINES                            */
/************************************************************************/
#define false 0
#define true 1

#define CONFIG_VERSION_SIZE(param) \
    param.nVersion.nVersion = CURRENT_OMX_SPEC_VERSION;\
    param.nSize = sizeof(param);

#define FAILED(result) (result != OMX_ErrorNone)

#define SUCCEEDED(result) (result == OMX_ErrorNone)

/************************************************************************/
/*                GLOBAL DECLARATIONS                     */
/************************************************************************/

pthread_mutex_t lock;
pthread_cond_t cond;
pthread_mutex_t elock;
pthread_cond_t econd;
pthread_mutex_t lock1;
pthread_mutexattr_t lock1_attr;
pthread_cond_t fcond;
pthread_mutex_t etb_lock;
pthread_mutex_t etb_lock1;
pthread_cond_t etb_cond;
pthread_mutexattr_t etb_lock_attr;
FILE * inputBufferFile;
FILE * outputBufferFile;
FILE * logFile = NULL;
OMX_PARAM_PORTDEFINITIONTYPE inputportFmt;
OMX_PARAM_PORTDEFINITIONTYPE outputportFmt;
OMX_AUDIO_PARAM_AACPROFILETYPE aacparam;
QOMX_AUDIO_STREAM_INFO_DATA streaminfoparam;
QOMX_AUDIO_CONFIG_DUALMONOTYPE dualmonoconfig;
OMX_PORT_PARAM_TYPE portParam;
OMX_ERRORTYPE error;
int bReconfigureOutputPort = 0;


#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

static int bFileclose = 0;
//typedef unsigned int uint32_t;
//typedef unsigned short int uint16_t;

struct wav_header {
  uint32_t riff_id;
  uint32_t riff_sz;
  uint32_t riff_fmt;
  uint32_t fmt_id;
  uint32_t fmt_sz;
  uint16_t audio_format;
  uint16_t num_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;       /* sample_rate * num_channels * bps / 8 */
  uint16_t block_align;     /* num_channels * bps / 8 */
  uint16_t bits_per_sample;
  uint32_t data_id;
  uint32_t data_sz;
};

static unsigned totaldatalen = 0;
/************************************************************************/
/*                GLOBAL INIT                    */
/************************************************************************/

unsigned int input_buf_cnt = 0;
unsigned int output_buf_cnt = 0;
int used_ip_buf_cnt = 0;
volatile int event_is_done = 0;
volatile int ebd_event_is_done = 0;
volatile int fbd_event_is_done = 0;
volatile int etb_event_is_done = 0;
int ebd_cnt;
int bOutputEosReached = 0;
int bInputEosReached = 0;
int bEosOnInputBuf = 0;
int bEosOnOutputBuf = 0;
static int etb_done = 0;
#ifdef AUDIOV2
unsigned short session_id;
unsigned short session_id_hpcm;
int device_id;
int control = 0;
const char *device="handset_rx";
int devmgr_fd;
#endif
int bFlushing = false;
int bPause    = false;
const char *in_filename;
const char out_filename[512];
const char *log_filename = NULL;
OMX_U8* pBuffer_tmp = NULL;
int timeStampLfile = 0;
int timestampInterval = 100;

//* OMX Spec Version supported by the wrappers. Version = 1.1 */
const OMX_U32 CURRENT_OMX_SPEC_VERSION = 0x00000101;
OMX_COMPONENTTYPE* aac_dec_handle = 0;

OMX_BUFFERHEADERTYPE  **pInputBufHdrs = NULL;
OMX_BUFFERHEADERTYPE  **pOutputBufHdrs = NULL;

/************************************************************************/
/*                GLOBAL FUNC DECL                        */
/************************************************************************/
int Init_Decoder(char*);
int Play_Decoder();
void process_portreconfig();
OMX_STRING aud_comp;

/**************************************************************************/
/*                STATIC DECLARATIONS                       */
/**************************************************************************/

static int open_audio_file ();
static size_t Read_Buffer(OMX_BUFFERHEADERTYPE  *pBufHdr );
static OMX_ERRORTYPE Allocate_Buffer ( OMX_COMPONENTTYPE *aac_dec_handle,
                                       OMX_BUFFERHEADERTYPE  ***pBufHdrs,
                                       OMX_U32 nPortIndex,
                                       unsigned int bufCntMin, unsigned int bufSize);


static OMX_ERRORTYPE EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
                                  OMX_IN OMX_PTR pAppData,
                                  OMX_IN OMX_EVENTTYPE eEvent,
                                  OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2,
                                  OMX_IN OMX_PTR pEventData);
static OMX_ERRORTYPE EmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_PTR pAppData,
                                     OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

static OMX_ERRORTYPE FillBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_PTR pAppData,
                                     OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);
static void write_devctlcmd(int fd, const void *buf, int param);
static void Display_Help(void);

#ifdef MSM_ALSA
#define PS (int)pcmCntxt->period_size
#endif /*MSM_ALSA*/

void wait_for_event(void)
{
    pthread_mutex_lock(&lock);
    fprintf(logFile,"%s: event_is_done=%d\n", __FUNCTION__, event_is_done);
    while (event_is_done == 0) {
        pthread_cond_wait(&cond, &lock);
    }
    event_is_done = 0;
    pthread_mutex_unlock(&lock);
}

void event_complete(void )
{
    pthread_mutex_lock(&lock);
    if (event_is_done == 0) {
        event_is_done = 1;
        pthread_cond_broadcast(&cond);
    }
    pthread_mutex_unlock(&lock);
}

void etb_wait_for_event(void)
{
    pthread_mutex_lock(&etb_lock1);
    fprintf(logFile,"%s: etb_event_is_done=%d", __FUNCTION__, etb_event_is_done);
    while (etb_event_is_done == 0) {
        pthread_cond_wait(&etb_cond, &etb_lock1);
    }
    etb_event_is_done = 0;
    pthread_mutex_unlock(&etb_lock1);
}

void etb_event_complete(void )
{
    pthread_mutex_lock(&etb_lock1);
    if (etb_event_is_done == 0) {
        etb_event_is_done = 1;
        pthread_cond_broadcast(&etb_cond);
    }
    pthread_mutex_unlock(&etb_lock1);
}



OMX_ERRORTYPE EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
                           OMX_IN OMX_PTR pAppData,
                           OMX_IN OMX_EVENTTYPE eEvent,
                           OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2,
                           OMX_IN OMX_PTR pEventData)
{
    fprintf(logFile,"Function %s \n", __FUNCTION__);

   unsigned int bufCnt=0;
   (void)hComponent;
   (void)pAppData;
   (void)pEventData;
    switch(eEvent) {
        case OMX_EventCmdComplete:
            fprintf(logFile,"*********************************************\n");
            fprintf(logFile,"\n OMX_EventCmdComplete \n");
            fprintf(logFile,"*********************************************\n");
            if(OMX_CommandPortDisable == (OMX_COMMANDTYPE)nData1)
            {
                fprintf(logFile,"******************************************\n");
                fprintf(logFile,"Recieved DISABLE Event Command Complete[%u]\n",nData2);
                fprintf(logFile,"******************************************\n");
            }
            else if(OMX_CommandPortEnable == (OMX_COMMANDTYPE)nData1)
            {
                fprintf(logFile,"*********************************************\n");
                fprintf(logFile,"Recieved ENABLE Event Command Complete[%u]\n",nData2);
                fprintf(logFile,"*********************************************\n");
            }
            else if(OMX_CommandFlush== (OMX_COMMANDTYPE)nData1)
            {
                fprintf(logFile,"*********************************************\n");
                fprintf(logFile,"Recieved FLUSH Event Command Complete[%u]\n",nData2);
                fprintf(logFile,"*********************************************\n");
            }
        event_complete();
        break;
        case OMX_EventError:
            fprintf(logFile,"*********************************************\n");
            fprintf(logFile,"\n OMX_EventError \n");
            fprintf(logFile,"*********************************************\n");
            if(OMX_ErrorInvalidState == (OMX_ERRORTYPE)nData1)
            {
               fprintf(logFile,"\n OMX_ErrorInvalidState \n");
               for(bufCnt=0; bufCnt < input_buf_cnt; ++bufCnt)
               {
                  OMX_FreeBuffer(aac_dec_handle, 0, pInputBufHdrs[bufCnt]);
               }
               if(tunnel == 0)
               {
                   for(bufCnt=0; bufCnt < output_buf_cnt; ++bufCnt)
                   {
                     OMX_FreeBuffer(aac_dec_handle, 1, pOutputBufHdrs[bufCnt]);
                   }
               }

               fprintf(logFile,"*********************************************\n");
               fprintf(logFile,"\n Component Deinitialized \n");
               fprintf(logFile,"*********************************************\n");
               exit(0);
            }
            else if(OMX_ErrorComponentSuspended == (OMX_ERRORTYPE)nData1)
            {
               fprintf(logFile,"*********************************************\n");
               fprintf(logFile,"\n Component Received Suspend Event \n");
               fprintf(logFile,"*********************************************\n");
            }
            break;


        case OMX_EventPortSettingsChanged:
            if(tunnel == 0)
            {
                bReconfigureOutputPort = 1;
            fprintf(logFile,"*********************************************\n");
            fprintf(logFile,"\n OMX_EventPortSettingsChanged \n");
            fprintf(logFile,"*********************************************\n");
            event_complete();
            }
        break;
         case OMX_EventBufferFlag:
             fprintf(logFile,"\n *********************************************\n");
             fprintf(logFile,"\n OMX_EventBufferFlag \n");
             fprintf(logFile,"\n *********************************************\n");
             if(tunnel)
             {
                 bInputEosReached = true;
             }
             else
             {
                 bOutputEosReached = true;
             }
             event_complete();
             break;

       case OMX_EventComponentResumed:
           fprintf(logFile,"*********************************************\n");
           fprintf(logFile,"\n Component Received Suspend Event \n");
           fprintf(logFile,"*********************************************\n");
           break;

       default:
            fprintf(logFile,"\n Unknown Event \n");
            break;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE FillBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                              OMX_IN OMX_PTR pAppData,
                              OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
   OMX_U32 i=0;
   size_t bytes_writen = 0;
   static int count = 0;
   static int copy_done = 0;
   static unsigned int length_filled = 0;
   static unsigned int spill_length = 0;
   static unsigned int  pcm_buf_size = 4800;
   static int unsigned pcm_buf_count = 2;
   struct msm_audio_config drv_pcm_config;
   OMX_AUDIO_PARAM_PCMMODETYPE OutputPcmParam;
   (void)pAppData;
   static int splbytes = 0;

   if(flushinprogress == 1)
   {
       fprintf(logFile," FillBufferDone: flush is in progress so hold the buffers\n");
       return OMX_ErrorNone;
   }

   if(count == 0 ){
        CONFIG_VERSION_SIZE(OutputPcmParam);
        /* Port for which the Client needs to obtain info */
        OutputPcmParam.nPortIndex = 1;

        OMX_GetParameter(aac_dec_handle,OMX_IndexParamAudioPcm,
            &OutputPcmParam);
        channels= OutputPcmParam.nChannels;
        samplerate= OutputPcmParam.nSamplingRate;

        fprintf(logFile,"\n\nPCM config sampling rate = %d channel count = %d\n",
            (int)OutputPcmParam.nSamplingRate,(int)OutputPcmParam.nChannels);
    }

   if(count == 0 && pcmplayback)
   {
       fprintf(logFile," open pcm device \n");

#ifdef MSM_ALSA
       {
           struct alsa_config_param alsaConfigData = {channels, devid, volumelevel};
           if(alsa_init_audiohardware(logFile) < 0){
                fprintf(stderr,"alsa_init_audiohardware failed \n");
                return -1;
           }
           if(alsa_enable_pcm_session(logFile, 1, &alsaConfigData, &mxr, &pcmCntxt) < 0)
                return -1;
           if (alsa_set_params(logFile, OutputPcmParam.nChannels,OutputPcmParam.nSamplingRate, pcmCntxt) < 0){
                fprintf(stderr,"set PCM config failed \n");
                return -1;
           }

           pBuffer_tmp= (OMX_U8*)malloc(PS);
           if (pBuffer_tmp == NULL)
           {
              fprintf(stderr,"temp buffer malloc failed \n");
              return -1;
           }
       }
#endif /*MSM_ALSA*/
   }
   fprintf(logFile," FillBufferDone #%d size %u Flag 0x%x\n",
        count++,pBuffer->nFilledLen, (unsigned int)pBuffer->nFlags);
   if(bEosOnOutputBuf)
       return OMX_ErrorNone;
   if((tunnel == 0) && (filewrite == 1))
   {
       bytes_writen =
       fwrite(pBuffer->pBuffer + pBuffer->nOffset,1,pBuffer->nFilledLen,outputBufferFile);
       fprintf(logFile," FillBufferDone size writen to file  %zu\n",bytes_writen);
       totaldatalen = totaldatalen + (unsigned int)bytes_writen;
   }

   if(pcmplayback && pBuffer->nFilledLen)
   {

#ifdef MSM_ALSA
        /* framer logic to update ALSA in period size chuncks*/
        if(!alsa_framer_update_pcm(logFile, pcmCntxt, pBuffer,&splbytes,(char *)pBuffer_tmp))
            fprintf(stderr,"framer_update_pcm failed ");
        if(!start_done)start_done= 1;
#endif /*MSM_ALSA*/
        fprintf(logFile," FillBufferDone: writing data to pcm device for play succesfull \n");
   }


    if((pBuffer->nFlags & OMX_BUFFERFLAG_EOS) != OMX_BUFFERFLAG_EOS)
    {
        fprintf(logFile," FBD calling FTB");
        OMX_FillThisBuffer(hComponent,pBuffer);
    }
    else
    {
       fprintf(logFile," FBD EOS REACHED...........\n");
       bEosOnOutputBuf = true;
#ifdef MSM_ALSA
       if((tunnel == 0) && (filewrite == 1) && splbytes)
       {
          fprintf(logFile," Flushing left over bytes(%d) to ALSA\n",bytes_writen);
          if(splbytes < PS)
              memset((pBuffer_tmp+splbytes),0,(PS-splbytes));
          else
              fprintf(logFile," This cannot be, framer logic goofed up\n");

          if(pcm_write(pcmCntxt,pBuffer_tmp,PS)){
              fprintf(logFile," Alsa PCM write Failed\n");
              return -1;
          }
       }
#endif /*MSM_ALSA*/

    }
    return OMX_ErrorNone;

}


OMX_ERRORTYPE EmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                              OMX_IN OMX_PTR pAppData,
                              OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    size_t readBytes =0;
    (void)pAppData;

    fprintf(logFile,"\nFunction %s cnt[%d], used_ip_buf_cnt[%d]\n", __FUNCTION__, ebd_cnt,used_ip_buf_cnt);
    fprintf(logFile,"\nFunction %s %p %u\n", __FUNCTION__, pBuffer,pBuffer->nFilledLen);
    ebd_cnt++;
    used_ip_buf_cnt--;
    pthread_mutex_lock(&etb_lock);
    if(!etb_done)
    {
        fprintf(logFile,"\n*********************************************\n");
        fprintf(logFile,"Wait till first set of buffers are given to component\n");
        fprintf(logFile,"\n*********************************************\n");
        etb_done++;
        pthread_mutex_unlock(&etb_lock);
        etb_wait_for_event();
    }
    else
    {
        pthread_mutex_unlock(&etb_lock);
    }


    if(bEosOnInputBuf)
    {
        fprintf(logFile,"\n*********************************************\n");
        fprintf(logFile,"   EBD::EOS on input port\n ");
        fprintf(logFile,"*********************************************\n");
        return OMX_ErrorNone;
    }else if (bFlushing == true) {
      fprintf(logFile,"omx_aac_adec_test: bFlushing is set to TRUE used_ip_buf_cnt=%d\n",used_ip_buf_cnt);
      if (used_ip_buf_cnt == 0) {
        //fseek(inputBufferFile, 0, 0);
        bFlushing = false;
      } else {
        fprintf(logFile,"omx_aac_adec_test: more buffer to come back used_ip_buf_cnt=%d\n",used_ip_buf_cnt);
        return OMX_ErrorNone;
      }
    }

    if((readBytes = Read_Buffer(pBuffer)) > 0) {
        pBuffer->nFilledLen = (OMX_U32)readBytes;
        used_ip_buf_cnt++;
        OMX_EmptyThisBuffer(hComponent,pBuffer);
    }
    else{
        pBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
            used_ip_buf_cnt++;
            //bInputEosReached = true;
            bEosOnInputBuf = true;
        pBuffer->nFilledLen = 0;
        OMX_EmptyThisBuffer(hComponent,pBuffer);
        fprintf(logFile,"EBD..Either EOS or Some Error while reading file\n");
    }
    return OMX_ErrorNone;
}

void signal_handler(int sig_id) {

  /* Flush */


   if (sig_id == SIGUSR1) {
    fprintf(logFile,"%s Initiate flushing\n", __FUNCTION__);
    bFlushing = true;
    OMX_SendCommand(aac_dec_handle, OMX_CommandFlush, OMX_ALL, NULL);
  } else if (sig_id == SIGUSR2) {
    if (bPause == true) {
      fprintf(logFile,"%s resume playback\n", __FUNCTION__);
      bPause = false;
      OMX_SendCommand(aac_dec_handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    } else {
      fprintf(logFile,"%s pause playback\n", __FUNCTION__);
      bPause = true;
      OMX_SendCommand(aac_dec_handle, OMX_CommandStateSet, OMX_StatePause, NULL);
    }
  }
}

int main(int argc, char **argv)
{
    unsigned int bufCnt=0;
    int opt = 0;
    int option_index = 0;
    OMX_ERRORTYPE result;
    time_t logFileTime ;
    struct sigaction sa;
    int ret = 0;


    struct wav_header hdr;
    size_t bytes_writen = 0;

    struct option long_options[] =
    {
        /* These options set a flag. */
        {"sample-rate",     required_argument,    0, 'r'},
        {"inchannel",       required_argument,    0, 'm'},
        {"pcm-play",        required_argument,    0, 'p'},
        {"tunnel",          no_argument,          0, 't'},
        {"dump-pcm",        no_argument,          0, 'd'},
        {"sbr-ps",          required_argument,    0, 's'},
        {"bsac",            required_argument,    0, 'b'},
        {"configbufsize",   required_argument,    0, 'c'},
        {"raw-config-bytes",required_argument,    0, 'R'},
        {"outchannel",      required_argument,    0, 'n'},
        {"force-multiaac",  no_argument,          0, 'f'},
        {"dual-mono",       required_argument,    0, 'u'},
        {"out-devid",       required_argument,    0, 'i'},
        {"vol",             required_argument,    0, 'v'},
        {"logfile",         required_argument,    0, 'l'},
        {"mix_coeff",       required_argument,    0, 'a'},
        {"help",            no_argument,          0, 'h'},
        {0, 0, 0, 0}
    };

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &signal_handler;
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    pthread_cond_init(&cond, 0);
    pthread_mutex_init(&lock, 0);

    pthread_cond_init(&etb_cond, 0);
    pthread_mutex_init(&etb_lock, 0);
    pthread_mutex_init(&etb_lock1, 0);
    pthread_mutexattr_init(&lock1_attr);
    pthread_mutex_init(&lock1, &lock1_attr);

    while ((opt = getopt_long(argc, argv, "-r:m:p:tds:b:c:R:n:fu:i:v:l:a:h", long_options, &option_index)) != -1)
        {
            switch (opt) {
            case 'r':
                samplerate = (uint32_t)atoi(optarg);
                break;
            case 'm':;
                channels = (uint32_t)atoi(optarg);
                break;
            case 'p':;
                pcmplayback = (uint32_t)atoi(optarg);
                break;
            case 't':
                tunnel = 1;
                break;
            case 'd':
                filewrite = 1;
                break;
            case 's':
                sbr_ps_enabled = (uint32_t)atoi(optarg);
            case 'b':
                bsac = (uint32_t)atoi(optarg);
                break;
            case 'c':
                configbufsize = (uint32_t)atoi(optarg);
                break;
            case 'R':
                raw_config_bytes = (uint32_t)atoi(optarg);
                break;
            case 'n':
                aac_channels = (uint32_t)atoi(optarg);
                break;
            case 'f':
                force_multiaac_codec = 1;
                break;
            case 'u':
                dual_mono_mode = atoi(optarg);
                break;
            case 'i':
                devid = (uint32_t)atoi(optarg);
                break;
            case 'v':
                volumelevel = (uint32_t)atoi(optarg);
                break;
            case 'l':{
                if(logFile != NULL)
                    fclose(logFile);

                if((!strcmp("stdout",optarg)) || (atoi(optarg)== 1))
                    logFile = stdout;
                else
                    {
                        log_filename = optarg;
                        if((logFile = fopen(log_filename,"wb"))== NULL)
                            fprintf(stderr, "Cannot open log file %s",log_filename);
                    }
                break;
                }
            case 'a':
                mix_coeff = (uint32_t)atoi(optarg);
                break;
            case 'h':{
                Display_Help();
                goto EXIT;
                break;
                }
         }
    }



    if(logFile == NULL)
        logFile = fopen("/dev/null","w");
        if(logFile == NULL)
        {
           fprintf(stderr, "/dev/null file can not be opened EXIT");
           goto EXIT;
        }

    in_filename = argv[1];
    if(!in_filename)
    {
        fprintf(stderr, "Provide the file name to be decoded, for help use -h or --help option \n");
        goto EXIT;
    }

    if (tunnel == 1) {
        pcmplayback = 0; /* This feature holds good only for non tunnel mode*/
        if(filewrite)
            fprintf(stderr, "PCM dump notpossible in Tunnel mode, \
              overriding it\n");
        filewrite = 0;  /* File write not supported in tunnel mode */
    }


    time(&logFileTime);
    fprintf(logFile,"*****************************************\n");
    fprintf(logFile,"File to be decoded is %s \n",in_filename);
    fprintf(logFile,"Log File Time Stamp %s\n",ctime(&logFileTime));
    fprintf(logFile,"*****************************************\n");

    if(force_multiaac_codec == 1){
            aud_comp = "OMX.qcom.audio.decoder.multiaac";
            fprintf(logFile,"Using multiaac aac Codec\n");
    }
    else if(aac_channels > 2)
    {
        if(tunnel == 0)
        {
            aud_comp = "OMX.qcom.audio.decoder.multiaac";
        }
        else
        {
            aud_comp = "OMX.qcom.audio.decoder.tunneled.multiaac";
        }
     }
     else
     {
         if(tunnel == 0)
         {
             aud_comp = "OMX.qcom.audio.decoder.aac";
         }
         else
         {
             aud_comp = "OMX.qcom.audio.decoder.tunneled.aac";
         }
      }

    if(Init_Decoder(aud_comp)!= 0x00)
    {
        fprintf(logFile,"Decoder Init failed\n");
        fprintf(stderr,"Decoder Init failed\n");
        goto EXIT;
    }

    if(Play_Decoder() != 0x00)
    {
        fprintf(logFile,"Play_Decoder failed\n");
        fprintf(stderr,"Play_Decoder failed\n");
        goto EXIT;
    }

    // Wait till EOS is reached...

    fprintf(logFile,"before wait_for_event\n");
    if(bReconfigureOutputPort)
    {
        wait_for_event();
    }
    if(bOutputEosReached || (tunnel && bInputEosReached))
    {
        if((tunnel == 0)&& (filewrite == 1))
        {
            hdr.riff_id = ID_RIFF;
            hdr.riff_sz = 0;
            hdr.riff_fmt = ID_WAVE;
            hdr.fmt_id = ID_FMT;
            hdr.fmt_sz = 16;
            hdr.audio_format = FORMAT_PCM;
            hdr.num_channels = (uint16_t)channels;//2;
            hdr.sample_rate = samplerate; //SAMPLE_RATE;  //44100;
            hdr.byte_rate = hdr.sample_rate * hdr.num_channels * 2;
            hdr.block_align = (uint16_t)(hdr.num_channels * 2);
            hdr.bits_per_sample = 16;
            hdr.data_id = ID_DATA;
            hdr.data_sz = 0;

            fprintf(logFile,"output file closed and EOS reached total \
                decoded data length %d\n",totaldatalen);
            hdr.data_sz = totaldatalen;
            hdr.riff_sz = totaldatalen + 8 + 16 + 8;
            fseek(outputBufferFile, 0L , SEEK_SET);
            bytes_writen = fwrite(&hdr,1,sizeof(hdr),outputBufferFile);
            if (bytes_writen <= 0) {
                fprintf(logFile,"Invalid Wav header write failed\n");
            }
            bFileclose = 1;
            fclose(outputBufferFile);
        }
        /************************************************************************************/
        fprintf(logFile,"\nMoving the decoder to idle state \n");
        OMX_SendCommand(aac_dec_handle, OMX_CommandStateSet, OMX_StateIdle,0);
        wait_for_event();
        fprintf(logFile,"\nMoving the decoder to loaded state \n");
        OMX_SendCommand(aac_dec_handle, OMX_CommandStateSet, OMX_StateLoaded,0);

        fprintf(logFile,"\nFillBufferDone: Deallocating i/p buffers \n");
        for(bufCnt=0; bufCnt < input_buf_cnt; ++bufCnt) {
            OMX_FreeBuffer(aac_dec_handle, 0, pInputBufHdrs[bufCnt]);
        }

        if(tunnel == 0)
        {
            fprintf(logFile,"\nFillBufferDone: Deallocating o/p buffers \n");
            for(bufCnt=0; bufCnt < output_buf_cnt; ++bufCnt) {
            OMX_FreeBuffer(aac_dec_handle, 1, pOutputBufHdrs[bufCnt]);
            }
        }


        ebd_cnt=0;
            wait_for_event();
            ebd_cnt=0;
        bOutputEosReached = false;
        bInputEosReached = false;
            bEosOnInputBuf = 0;
            bEosOnOutputBuf = 0;
        result = OMX_FreeHandle(aac_dec_handle);
        if (result != OMX_ErrorNone) {
            fprintf(logFile,"\nOMX_FreeHandle error. Error code: %d\n", result);
        }
        aac_dec_handle = NULL;

        if(pcmplayback){
#ifdef MSM_ALSA
            struct alsa_config_param alsaConfigData = {channels, devid, volumelevel};
            ret = alsa_enable_pcm_session(logFile, 0, &alsaConfigData, &mxr, &pcmCntxt);
            if(ret)
                fprintf(stderr,"\nFailure in tearing down the PCM session\n");
            if(alsa_deinit_audiohardware(logFile) < 0)
                fprintf(stderr,"\nFailure in closing down the UCM \n");

#endif /* MSM_ALSA */
        }

        /* Deinit OpenMAX */

        OMX_Deinit();

        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&lock);
        etb_done = 0;
            bReconfigureOutputPort = 0;
        fprintf(logFile,"*****************************************\n");
        fprintf(logFile,"******...TEST COMPLETED...***************\n");
        fprintf(logFile,"*****************************************\n");
EXIT:
        if (pBuffer_tmp)
        {
            free(pBuffer_tmp);
            pBuffer_tmp =NULL;
        }
        if(logFile != NULL)
            fclose(logFile);
    }
    return 0;
}

int Init_Decoder(OMX_STRING audio_component)
{
    fprintf(logFile,"Inside %s \n", __FUNCTION__);
    OMX_ERRORTYPE omxresult;
    OMX_U32 total = 0;
    typedef OMX_U8* OMX_U8_PTR;
    OMX_STRING role ="audio_decoder";

    static OMX_CALLBACKTYPE call_back = {
        &EventHandler,&EmptyBufferDone,&FillBufferDone
    };

    /* Init. the OpenMAX Core */
    fprintf(logFile,"\nInitializing OpenMAX Core....\n");
    omxresult = OMX_Init();

    if(OMX_ErrorNone != omxresult) {
        fprintf(logFile,"\n Failed to Init OpenMAX core");
        fprintf(stderr,"\n Failed to Init OpenMAX core");
          return -1;
    }
    else {
        fprintf(logFile,"\nOpenMAX Core Init Done\n");
    }

    /* Query for audio decoders*/
    fprintf(logFile,"Aac_test: Before entering OMX_GetComponentOfRole");
    OMX_GetComponentsOfRole(role, &total, 0);
    fprintf(logFile,"\nTotal components of role=%s :%u", role, total);


    omxresult = OMX_GetHandle((OMX_HANDLETYPE*)(&aac_dec_handle),
                        (OMX_STRING)audio_component, NULL, &call_back);
    if (FAILED(omxresult)) {
        fprintf(logFile,"\nFailed to Load the component:%s\n", audio_component);
        fprintf(stderr,"\nFailed to Load the component:%s\n", audio_component);
    return -1;
    }
    else
    {
        fprintf(logFile,"\nComponent %s is in LOADED state\n", audio_component);
    }

    /* Get the port information */
    CONFIG_VERSION_SIZE(portParam);
    omxresult = OMX_GetParameter(aac_dec_handle, OMX_IndexParamAudioInit,
                                (OMX_PTR)&portParam);

    if(FAILED(omxresult)) {
        fprintf(stderr,"\nFailed to get Port Param\n");
        fprintf(logFile,"\nFailed to get Port Param\n");
    return -1;
    }
    else
    {
        fprintf(logFile,"\nportParam.nPorts:%u\n", portParam.nPorts);
        fprintf(logFile,"\nportParam.nStartPortNumber:%u\n",
            portParam.nStartPortNumber);
    }
    return 0;
}

int Play_Decoder()
{
    unsigned int i;
    size_t Size=0;
    fprintf(logFile,"Inside %s \n", __FUNCTION__);
    OMX_ERRORTYPE ret;
    OMX_INDEXTYPE index;
#ifdef __LP64__
    fprintf(logFile,"sizeof[%ld]\n", sizeof(OMX_BUFFERHEADERTYPE));
#else
    fprintf(logFile,"sizeof[%d]\n", sizeof(OMX_BUFFERHEADERTYPE));
#endif
    /* open the i/p and o/p files based on the video file format passed */
    if(open_audio_file()) {
        fprintf(logFile,"\n Returning -1 \n");
    return -1;
    }

    /*  Configuration of Input Port definition */

    /* Query the decoder input min buf requirements */
    CONFIG_VERSION_SIZE(inputportFmt);

    /* Port for which the Client needs to obtain info */
    inputportFmt.nPortIndex = portParam.nStartPortNumber;

    OMX_GetParameter(aac_dec_handle,OMX_IndexParamPortDefinition,&inputportFmt);
    inputportFmt.nBufferCountActual = inputportFmt.nBufferCountMin + 3;
    fprintf(logFile,"\nDec: Input Buffer Count %u\n", inputportFmt.nBufferCountMin);
    fprintf(logFile,"\nDec: Input Buffer Size %u\n", inputportFmt.nBufferSize);

    if(OMX_DirInput != inputportFmt.eDir) {
        fprintf(logFile,"\nDec: Expect Input Port\n");
    return -1;
    }


    OMX_SetParameter(aac_dec_handle,OMX_IndexParamPortDefinition,&inputportFmt);
    OMX_GetExtensionIndex(aac_dec_handle,"OMX.Qualcomm.index.audio.sessionId",&index);
    OMX_GetParameter(aac_dec_handle,index,&streaminfoparam);

    if(tunnel == 0)
    {
        /*  Configuration of Ouput Port definition */
        /* Query the decoder outport's min buf requirements */
        CONFIG_VERSION_SIZE(outputportFmt);
        /* Port for which the Client needs to obtain info */
        outputportFmt.nPortIndex = portParam.nStartPortNumber + 1;

        OMX_GetParameter(aac_dec_handle,OMX_IndexParamPortDefinition,&outputportFmt);
        fprintf(logFile,"\nDec: Output Buffer Count %u\n", outputportFmt.nBufferCountMin);
        fprintf(logFile,"\nDec: Output Buffer Size %u\n", outputportFmt.nBufferSize);

        if(OMX_DirOutput != outputportFmt.eDir) {
            fprintf(logFile,"\nDec: Expect Output Port\n");
        return -1;
        }

        outputportFmt.nBufferCountActual = outputportFmt.nBufferCountMin;
        OMX_SetParameter(aac_dec_handle,OMX_IndexParamPortDefinition,&outputportFmt);
    }

    CONFIG_VERSION_SIZE(aacparam);


    aacparam.nPortIndex   =  0;
    aacparam.nChannels    =  aac_channels; /* 1-> mono 2-> stereo 6-> AAC 5.1*/
    aacparam.nBitRate     =  samplerate; //SAMPLE_RATE;
    aacparam.nSampleRate  =  samplerate; //SAMPLE_RATE;
    aacparam.eChannelMode =  OMX_AUDIO_ChannelModeStereo;
    if (sbr_ps_enabled == 0 )
        aacparam.eAACProfile = OMX_AUDIO_AACObjectLC;
    else if (sbr_ps_enabled == 1 )
        aacparam.eAACProfile = OMX_AUDIO_AACObjectHE;
    else if (sbr_ps_enabled == 2 )
        aacparam.eAACProfile = OMX_AUDIO_AACObjectHE_PS;
    aacparam.eAACStreamFormat    =  OMX_AUDIO_AACStreamFormatMP2ADTS;
    OMX_SetParameter(aac_dec_handle,OMX_IndexParamAudioAac,&aacparam);

    if ((dual_mono_mode >= OMX_AUDIO_DUAL_MONO_MODE_FL_FR) &&
        (dual_mono_mode <= OMX_AUDIO_DUAL_MONO_MODE_FL_SR))
    {
        CONFIG_VERSION_SIZE(dualmonoconfig);
        dualmonoconfig.nPortIndex = 0;
        dualmonoconfig.eChannelConfig = dual_mono_mode;
        fprintf(logFile,"Configure DualMono: size, version,"
                    "portindex, channelconfig = %d %d %d\n",
                    dualmonoconfig.nSize,dualmonoconfig.nPortIndex,
                    dualmonoconfig.eChannelConfig);
        OMX_GetExtensionIndex(aac_dec_handle,"OMX.Qualcomm.index.audio.dualmono",&index);
        OMX_SetConfig(aac_dec_handle,index,&dualmonoconfig);
    }

    fprintf(logFile,"\n mix_coeff = %u\n", mix_coeff);
    OMX_GetExtensionIndex(aac_dec_handle,"OMX.Qualcomm.index.audio.aac_sel_mix_coef",&index);
    OMX_SetConfig(aac_dec_handle,index,&mix_coeff);

    fprintf(logFile,"\nOMX_SendCommand Decoder -> IDLE\n");
    OMX_SendCommand(aac_dec_handle, OMX_CommandStateSet, OMX_StateIdle,0);
    /* wait_for_event(); should not wait here event complete status will
       not come until enough buffer are allocated */

    input_buf_cnt = inputportFmt.nBufferCountActual;
    fprintf(logFile,"Transition to Idle State succesful...\n");
    /* Allocate buffer on decoder's i/p port */
    error = Allocate_Buffer(aac_dec_handle, &pInputBufHdrs, inputportFmt.nPortIndex,
                            input_buf_cnt, inputportFmt.nBufferSize);
    if (error != OMX_ErrorNone) {
        fprintf(logFile,"\nOMX_AllocateBuffer Input buffer error\n");
    return -1;
    } else if (pInputBufHdrs == NULL) {
        fprintf(logFile,"\npInputBufHdrs is NULL\\n");
        return -1;
    } else {
        fprintf(logFile,"\nOMX_AllocateBuffer Input buffer success\n");
    }

    if(tunnel == 0)
    {
        output_buf_cnt = outputportFmt.nBufferCountActual; // outputportFmt.nBufferCountMin ;

        /* Allocate buffer on decoder's O/Pp port */
        error = Allocate_Buffer(aac_dec_handle, &pOutputBufHdrs, outputportFmt.nPortIndex,
                                output_buf_cnt, outputportFmt.nBufferSize);
        if (error != OMX_ErrorNone || !pOutputBufHdrs) {
            fprintf(logFile,"\nOMX_AllocateBuffer Output buffer error\n");
            fprintf(stderr,"\nOMX_AllocateBuffer Output buffer error\n");
        return -1;
        }
        else {
            fprintf(logFile,"\nOMX_AllocateBuffer Output buffer success\n");
        }
    }

    wait_for_event();


    if (tunnel == 1)
    {
        fprintf(logFile,"\nOMX_SendCommand to enable TUNNEL MODE during IDLE\n");
        OMX_SendCommand(aac_dec_handle, OMX_CommandPortDisable,1,0);
        wait_for_event();
    }

    fprintf(logFile,"\nOMX_SendCommand Decoder -> Executing\n");
    OMX_SendCommand(aac_dec_handle, OMX_CommandStateSet, OMX_StateExecuting,0);
    wait_for_event();

    if(tunnel == 0)
    {
        fprintf(logFile," Start sending OMX_FILLthisbuffer\n");
        for(i=0; i < output_buf_cnt; i++) {
            fprintf(logFile,"\nOMX_FillThisBuffer on output buf no.%d\n",i);
            pOutputBufHdrs[i]->nOutputPortIndex = 1;
            pOutputBufHdrs[i]->nFlags = 0;
            ret = OMX_FillThisBuffer(aac_dec_handle, pOutputBufHdrs[i]);
            if (OMX_ErrorNone != ret) {
                fprintf(logFile,"OMX_FillThisBuffer failed with result %d\n", ret);
                fprintf(stderr,"OMX_FillThisBuffer failed with result %d\n", ret);
            }
            else {
                fprintf(logFile,"OMX_FillThisBuffer success!\n");
            }
        }
    }


    fprintf(logFile," Start sending OMX_emptythisbuffer\n");
    for (i = 0;i < input_buf_cnt;i++) {
        fprintf(logFile,"\nOMX_EmptyThisBuffer on Input buf no.%d\n",i);
        pInputBufHdrs[i]->nInputPortIndex = 0;
        Size = Read_Buffer(pInputBufHdrs[i]);
        if(Size <=0 ){
          fprintf(logFile,"NO DATA READ\n");
          //bInputEosReached = true;
          bEosOnInputBuf = true;
          pInputBufHdrs[i]->nFlags= OMX_BUFFERFLAG_EOS;
        }
        pInputBufHdrs[i]->nFilledLen = (OMX_U32)Size;
        pInputBufHdrs[i]->nInputPortIndex = 0;
        used_ip_buf_cnt++;
        ret = OMX_EmptyThisBuffer(aac_dec_handle, pInputBufHdrs[i]);
        if (OMX_ErrorNone != ret) {
            fprintf(logFile,"OMX_EmptyThisBuffer failed with result %d\n", ret);
            fprintf(stderr,"OMX_EmptyThisBuffer failed with result %d\n", ret);
        }
        else {
            fprintf(logFile,"OMX_EmptyThisBuffer success!\n");
        }
        if(Size <=0 ){
            break;//eos reached
        }
    }
    pthread_mutex_lock(&etb_lock);
    if(etb_done)
    {
        fprintf(logFile,"Component is waiting for EBD to be released.\n");
        etb_event_complete();
    }
    else
    {
        fprintf(logFile,"\n****************************\n");
        fprintf(logFile,"EBD not yet happened ...\n");
        fprintf(logFile,"\n****************************\n");
        etb_done++;
    }
    pthread_mutex_unlock(&etb_lock);
    while(1)
    {
        wait_for_event();
        if(bOutputEosReached || (tunnel && bInputEosReached))
        {
            bReconfigureOutputPort = 0;
            fprintf(logFile, "bOutputEosReached || (tunnel && bInputEosReached breaking\n");
            break;
        }
        else
        {
            if(tunnel == 0 && bReconfigureOutputPort)
            process_portreconfig();
        }
    }
    return 0;
}

static OMX_ERRORTYPE Allocate_Buffer ( OMX_COMPONENTTYPE *avc_dec_handle,
                                       OMX_BUFFERHEADERTYPE  ***pBufHdrs,
                                       OMX_U32 nPortIndex,
                                       unsigned int bufCntMin, unsigned int bufSize)
{
    fprintf(logFile,"Inside %s \n", __FUNCTION__);
    OMX_ERRORTYPE error=OMX_ErrorNone;
    unsigned int bufCnt=0;
    (void)avc_dec_handle;
    *pBufHdrs= (OMX_BUFFERHEADERTYPE **)
                   malloc(sizeof(OMX_BUFFERHEADERTYPE*)*bufCntMin);

    for(bufCnt=0; bufCnt < bufCntMin; ++bufCnt) {
        fprintf(logFile,"\n OMX_AllocateBuffer No %u \n", bufCnt);
        error = OMX_AllocateBuffer(aac_dec_handle, &((*pBufHdrs)[bufCnt]),
                                   nPortIndex, NULL, bufSize);
    }
    if (error != OMX_ErrorNone){
        free (*pBufHdrs);
    }

    return error;
}

static size_t Read_Buffer (OMX_BUFFERHEADERTYPE  *pBufHdr )
{
    size_t bytes_read=0;
    pBufHdr->nFilledLen = 0;
    pBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
    fprintf(logFile,"\n Length : %u, buffer address : %p\n", pBufHdr->nAllocLen, pBufHdr->pBuffer);
    if(bsac && configbufsize)
    {
        bytes_read = fread(pBufHdr->pBuffer, 1, configbufsize, inputBufferFile);
        configbufsize = 0;
        bsac = 0;
    } else if ((first_read == 1) && (raw_config_bytes > 0))
    {
        bytes_read = fread(pBufHdr->pBuffer, 1, raw_config_bytes, inputBufferFile);
        pBufHdr->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;
        fprintf(logFile,"\nraw_given = %d, Bytes read for first read is =%zu\n",
                 raw_config_bytes, bytes_read);
        first_read = 0;
    }
    else
    {
        size_t read_bytes = (pBufHdr->nAllocLen >  4096) ? 4096 : pBufHdr->nAllocLen;
        bytes_read = fread(pBufHdr->pBuffer, 1, read_bytes , inputBufferFile);
    }
    pBufHdr->nFilledLen = (OMX_U32)bytes_read;
    if(bytes_read == 0)
    {
       pBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
       fprintf(logFile,"\nBytes read zero\n");
    }
    else
    {
       pBufHdr->nFlags = pBufHdr->nFlags & (unsigned)~OMX_BUFFERFLAG_EOS;
       fprintf(logFile,"\nBytes read is Non zero=%zu\n",bytes_read);
    }
    return bytes_read;;
}

static int open_audio_file ()
{
    int error_code = 0;
    struct wav_header hdr;
    size_t header_len = 0;
    memset(&hdr,0,sizeof(hdr));

    hdr.riff_id = ID_RIFF;
    hdr.riff_sz = 0;
    hdr.riff_fmt = ID_WAVE;
    hdr.fmt_id = ID_FMT;
    hdr.fmt_sz = 16;
    hdr.audio_format = FORMAT_PCM;
    hdr.num_channels = (uint16_t)channels;//2;
    hdr.sample_rate = samplerate; //SAMPLE_RATE;  //44100;
    hdr.byte_rate = hdr.sample_rate * hdr.num_channels * 2;
    hdr.block_align = (uint16_t)(hdr.num_channels * 2);
    hdr.bits_per_sample = 16;
    hdr.data_id = ID_DATA;
    hdr.data_sz = 0;


    inputBufferFile = fopen (in_filename, "rb");
    fprintf(logFile,"\n FILE DESCRIPTOR : %p\n", inputBufferFile );
    if (inputBufferFile == NULL) {
        fprintf(stderr,"\ni/p file %s could NOT be opened\n",
                                         in_filename);
        error_code = -1;
        return error_code;
    }

    if((tunnel == 0)&& (filewrite == 1))
    {

        strlcpy((char *)out_filename, in_filename, sizeof(out_filename));
        if ((strlen(in_filename) + strlen(".wav")) >= sizeof(out_filename))
        {
          fprintf(stderr, "PCMDump file name is too long, ignoring dump\n");
          return -1;
        }

        strlcat((char *)out_filename,".wav", sizeof(out_filename));

        fprintf(logFile,"Inside %s filename=%s -->%s\n", __FUNCTION__,
            in_filename,out_filename);
        fprintf(logFile,"output file is opened\n");
          outputBufferFile = fopen(out_filename,"wb");
        if (outputBufferFile == NULL) {
            fprintf(logFile,"\no/p file %s could NOT be opened\n",
                                             out_filename);
            fprintf(stderr,"\no/p file %s could NOT be opened\n",
                                             out_filename);
            error_code = -1;
            return error_code;
        }

        header_len = fwrite(&hdr,1,sizeof(hdr),outputBufferFile);


        if (header_len <= 0) {
            fprintf(logFile,"Invalid Wav header \n");
        }
        fprintf(logFile," Length og wav header is %zu \n",header_len );
    }
    return error_code;
}




void process_portreconfig ( )
{
    unsigned int bufCnt,i=0;
    OMX_ERRORTYPE ret;
    struct msm_audio_config drv_pcm_config;
    //wait_for_event();
    fprintf(logFile,"************************************");
    fprintf(logFile,"RECIEVED EVENT PORT SETTINGS CHANGED EVENT\n");
    fprintf(logFile,"******************************************\n");

    // wait for port settings changed event


    fprintf(logFile,"************************************");
    fprintf(logFile,"NOW SENDING FLUSH CMD\n");
    fprintf(logFile,"******************************************\n");
    flushinprogress = 1;
    OMX_SendCommand(aac_dec_handle, OMX_CommandFlush, 1, 0);

    wait_for_event();
    fprintf(logFile,"************************************");
    fprintf(logFile,"RECIEVED FLUSH EVENT CMPL\n");
    fprintf(logFile,"******************************************\n");

    // Send DISABLE command
    OMX_SendCommand(aac_dec_handle, OMX_CommandPortDisable, 1, 0);

    fprintf(logFile,"******************************************\n");
    fprintf(logFile,"FREEING BUFFERS output_buf_cnt=%d\n",output_buf_cnt);
    fprintf(logFile,"******************************************\n");
    // Free output Buffer
    for(bufCnt=0; bufCnt < output_buf_cnt; ++bufCnt) {
        OMX_FreeBuffer(aac_dec_handle, 1, pOutputBufHdrs[bufCnt]);
    }

    // wait for Disable event to come back
    wait_for_event();
    fprintf(logFile,"******************************************\n");
    fprintf(logFile,"DISABLE EVENT RECD\n");
    fprintf(logFile,"******************************************\n");

        // Send Enable command
    OMX_SendCommand(aac_dec_handle, OMX_CommandPortEnable, 1, 0);
    flushinprogress = 0;
    // AllocateBuffers
    fprintf(logFile,"******************************************\n");
    fprintf(logFile,"ALLOC BUFFER AFTER PORT REENABLE");
    fprintf(logFile,"******************************************\n");
    /* Allocate buffer on decoder's o/p port */
    error = Allocate_Buffer(aac_dec_handle, &pOutputBufHdrs, outputportFmt.nPortIndex,
                            output_buf_cnt, outputportFmt.nBufferSize);
    if (error != OMX_ErrorNone || (pOutputBufHdrs == NULL)) {
        fprintf(logFile,"\nOMX_AllocateBuffer Output buffer error output_buf_cnt=%d\n",output_buf_cnt);
        return ;
    }
    else
    {
        fprintf(logFile,"\nOMX_AllocateBuffer Output buffer success output_buf_cnt=%d\n",output_buf_cnt);
    }

    fprintf(logFile,"******************************************\n");
    fprintf(logFile,"ENABLE EVENT HANDLER RECD\n");
    fprintf(logFile,"******************************************\n");
    // wait for enable event to come back
    wait_for_event();
    fprintf(logFile," Calling stop on pcm driver...\n");
    fprintf(logFile,"pcmplayback %d start_done %d\n", pcmplayback,start_done);

    OMX_GetParameter(aac_dec_handle,OMX_IndexParamAudioAac,
            &aacparam);
    channels= aacparam.nChannels;
    samplerate= aacparam.nSampleRate;
    fprintf(logFile,">>>>PortReconfigEvent<<< new samplerate =%u \
        channel = %u\n",aacparam.nSampleRate,aacparam.nChannels);


    if(pcmplayback && start_done)
    {
#ifdef MSM_ALSA
        struct alsa_config_param alsaConfigData = {channels, devid, volumelevel};
        alsa_enable_pcm_session(logFile, 0,&alsaConfigData, &mxr, &pcmCntxt);

        alsa_enable_pcm_session(logFile, 1,&alsaConfigData, &mxr, &pcmCntxt);

        alsa_set_params(logFile, aacparam.nChannels,aacparam.nSampleRate, pcmCntxt);

        //start_done = 0;
        bReconfigureOutputPort = 0;
#endif /*MSM_ALSA*/
    }


    fprintf(logFile,"******************************************\n");
    fprintf(logFile,"FTB after PORT RENABLE\n");
    fprintf(logFile,"******************************************\n");
    for(i=0; i < output_buf_cnt; i++) {
        fprintf(logFile,"\nOMX_FillThisBuffer on output buf no.%d\n",i);
        pOutputBufHdrs[i]->nOutputPortIndex = 1;
        //pOutputBufHdrs[i]->nFlags &= ~OMX_BUFFERFLAG_EOS;
        ret = OMX_FillThisBuffer(aac_dec_handle, pOutputBufHdrs[i]);
        if (OMX_ErrorNone != ret) {
            fprintf(logFile,"OMX_FillThisBuffer failed with result %d\n", ret);
        }
        else {
            fprintf(logFile,"OMX_FillThisBuffer success!\n");
    }
   }
}

static void write_devctlcmd(int fd, const void *buf, int param){
    size_t nbytes;
    ssize_t nbytesWritten;
    char cmdstr[128];
    snprintf(cmdstr, 128, "%s%d\n", (char *)buf, param);
    nbytes = strlen(cmdstr);
    nbytesWritten = write(fd, cmdstr, nbytes);

    if(nbytes != (size_t)nbytesWritten)
        fprintf(logFile,"Failed to write string %s to omx_devmgr\n",cmdstr);
}



static void Display_Help(void)
{
    printf(" \n Command \n");
    printf(" \n adb shell mm-adec-omxaac-test <file path>    - path of file to be decoded\n");
    printf(" \n Options\n");
    printf(" -r  --sample-rate <sampling rate>         - Required for raw aac stream\n\n");
    printf(" -m  --inchannel <Input channels>          - no of input channels\n\n");
    printf(" -p  --pcm-play <PCMPLAYBACK)              - PCM PLAYBACK DISABLE 0\n");
    printf("                                             PCM PLAYBACK ENABLE 1(default)\n\n");
    printf(" -t  --tunnel                              - Enable Tunnel Mode\n\n");
    printf(" -d  --dump-pcm                            - Enable Pcm dump \n\n");
    printf(" -s  --sbr-ps <SBRPS>                      - SBR PS DISABLED 0,SBR ENABLED 1\n");
    printf("                                             SBR AND PS ENABLED 2 \n\n");
    printf(" -b  --bsac <bsac>                         - bsac =1 for BSAC playback\n\n");
    printf(" -c  --configbufsize <CONFIGBUFFERSIZE>    - CONFIGBUFFERSIZE ONLY FOR BSAC\n");
    printf("                                             playback\n\n");
    printf(" -R  --raw-config-bytes <RAWCONFIGBYTES>   - RAWCONFIGBYTES = No. of codec \n");
    printf("                                             config bytes in the RAW fil\n\n");
    printf(" -n  --outchannel <out channels>           - No. of output channels 1,2,6\n");
    printf("                                            (for AAC 5.1)required for raw aac\n\n");
    printf(" -f  --force-multiaac                      - uses the multiaac Q6 codec by \n");
    printf("                                             default is q6 legacy aac decoder\n\n");
    printf(" -u  --dual-mono <DUALMONOMODE>            - DUALMONOMODE = Number indicating\n");
    printf("                                             the Dual Mono Mode(0, 1, 2, 3)\n");
    printf("                                             0- Ch1 to Left, Ch1 to Right\n");
    printf("                                             1- Ch2 to Left, Ch2 to Right\n");
    printf("                                             2- Ch2 to Left, Ch1 to Right\n");
    printf("                                             3- Ch1 to Left, Ch2 to Right\n\n");
    printf(" -i  --out-devid <DEVICE>                  - Device(0-handset, 1-speaker\n");
    printf("                                             2-headset, 3-headphone\n\n");
    printf(" -v  --volume level <VOL>                    0- Min Vol, 100 Max Vol\n\n");
    printf(" -l  --logfile <FILEPATH>                  - File path for debug msg, to print\n");
    printf("                                             on console use stdout or 1 \n");

}

