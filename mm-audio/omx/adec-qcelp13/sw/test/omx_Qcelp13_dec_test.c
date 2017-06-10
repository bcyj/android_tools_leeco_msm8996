

/*
An Open max test application for 13K component ....
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <time.h>
#include <linux/ioctl.h>
#include "OMX_Core.h"
#include "OMX_Component.h"
#ifdef AUDIOV2
#include "control.h"
#endif
#include <signal.h>
#include <stdint.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <linux/msm_audio.h>

uint32_t samplerate = 8000;
uint32_t channels = 1;
uint32_t pcmplayback = 0;
uint32_t tunnel      = 0;
uint32_t filewrite   = 0;
#ifdef _DEBUG

#define DEBUG_PRINT(args...) printf("%s:%d ", __FUNCTION__, __LINE__); \
    printf(args)

#define DEBUG_PRINT_ERROR(args...) printf("%s:%d ", __FUNCTION__, __LINE__); \
    printf(args)

#else

#define DEBUG_PRINT
#define DEBUG_PRINT_ERROR

#endif



#define PCM_PLAYBACK /* To write the pcm decoded data to the msm_pcm device for playback*/

extern OMX_U8 frameFormat;

#ifdef PCM_PLAYBACK
int m_pcmdrv_fd;
#endif  // PCM_PLAYBACK


/************************************************************************/
/*                #DEFINES                                 */
/************************************************************************/
#define false 0
#define true 1

#define CONFIG_VERSION_SIZE(param) \
    param.nVersion.nVersion = CURRENT_OMX_SPEC_VERSION;\
    param.nSize = sizeof(param);

#define FAILED(result) (result != OMX_ErrorNone)

#define SUCCEEDED(result) (result == OMX_ErrorNone)

/************************************************************************/
/*                   GLOBAL DECLARATIONS                     */
/************************************************************************/

pthread_mutex_t lock;
pthread_cond_t cond;
pthread_mutex_t elock;
pthread_cond_t econd;
pthread_cond_t fcond;
FILE * inputBufferFile;
FILE * outputBufferFile;
OMX_PARAM_PORTDEFINITIONTYPE inputportFmt;
OMX_PARAM_PORTDEFINITIONTYPE outputportFmt;

OMX_AUDIO_PARAM_QCELP13TYPE Qcelp13param;

OMX_PORT_PARAM_TYPE portParam;
OMX_ERRORTYPE error;



/* http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

static int bFileclose = 0;

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
/*                    GLOBAL INIT                             */
/************************************************************************/

int input_buf_cnt = 0;
int output_buf_cnt = 0;
int used_ip_buf_cnt = 0;
volatile int event_is_done = 0;
volatile int ebd_event_is_done = 0;
volatile int fbd_event_is_done = 0;
int ebd_cnt;
int bOutputEosReached = 0;
int bInputEosReached = 0;
#ifdef AUDIOV2
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

int chunksize =0;
OMX_U8* pBuffer_tmp = NULL;
int timeStampLfile = 0;
int timestampInterval = 100;
//* OMX Spec Version supported by the wrappers. Version = 1.1 */
const OMX_U32 CURRENT_OMX_SPEC_VERSION = 0x00000101;
OMX_COMPONENTTYPE* Qcelp13_dec_handle = 0;

OMX_BUFFERHEADERTYPE  **pInputBufHdrs = NULL;
OMX_BUFFERHEADERTYPE  **pOutputBufHdrs = NULL;

/************************************************************************/
/*                   GLOBAL FUNC DECL                        */
/************************************************************************/
int Init_Decoder(OMX_STRING audio_component);
int Play_Decoder();

OMX_STRING aud_comp;

/**************************************************************************/
/*                STATIC DECLARATIONS                       */
/**************************************************************************/

static int open_audio_file ();
static int Read_Buffer(OMX_BUFFERHEADERTYPE  *pBufHdr );
static OMX_ERRORTYPE Allocate_Buffer ( OMX_COMPONENTTYPE *Qcelp13_dec_handle,
                                      OMX_BUFFERHEADERTYPE  ***pBufHdrs,
                                      OMX_U32 nPortIndex,
                                      long bufCntMin, long bufSize);


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
/*=============================================================================
FUNCTION:
  wait_for_event

DESCRIPTION:
  Waits for events from other threads

INPUT/OUTPUT PARAMETERS:
  None

RETURN VALUE:
..None

Dependency:
  event_complete() should be called to exit from this function

SIDE EFFECTS:
   None
=============================================================================*/
void wait_for_event(void)
{
    pthread_mutex_lock(&lock);
    DEBUG_PRINT("%s: event_is_done=%d", __FUNCTION__, event_is_done);
    while (event_is_done == 0)
    {
        pthread_cond_wait(&cond, &lock);
    }
    event_is_done = 0;
    pthread_mutex_unlock(&lock);
}

/*=============================================================================
FUNCTION:
  event_complete

DESCRIPTION:
  function to indicate a thread which is waiting for an event

INPUT/OUTPUT PARAMETERS:
  None

RETURN VALUE:
..None

Dependency:
  wait_for_event should be called

SIDE EFFECTS:
   None
=============================================================================*/
void event_complete(void )
{
    pthread_mutex_lock(&lock);
    if (event_is_done == 0)
    {
        event_is_done = 1;
        pthread_cond_broadcast(&cond);
    }
    pthread_mutex_unlock(&lock);
}

/*=============================================================================
FUNCTION:
  EventHandler

DESCRIPTION:
  Callback function which handles the events from the OMX Component

INPUT/OUTPUT PARAMETERS:
  [IN] hComponent
  [IN] pAppData
  [IN] eEvent
  [IN] nData1
  [IN] nData2
  [IN] pEventData

RETURN VALUE:
  OMX_ERRORTYPE

Dependency:
  None

  SIDE EFFECTS:
   None
=============================================================================*/
OMX_ERRORTYPE EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
                           OMX_IN OMX_PTR pAppData,
                           OMX_IN OMX_EVENTTYPE eEvent,
                           OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2,
                           OMX_IN OMX_PTR pEventData)
{
    DEBUG_PRINT("Function %s \n", __FUNCTION__);
    (void)hComponent;
    (void)pAppData;
    (void)pEventData;
    switch(eEvent) {
        case OMX_EventCmdComplete:
            DEBUG_PRINT("*********************************************\n");
            DEBUG_PRINT("\n OMX_EventCmdComplete \n");
            DEBUG_PRINT("*********************************************\n");
            if(OMX_CommandPortDisable == (OMX_COMMANDTYPE)nData1)
            {
                DEBUG_PRINT("******************************************\n");
                DEBUG_PRINT("Recieved DISABLE Event Command Complete[%u]\n",nData2);
                DEBUG_PRINT("******************************************\n");
            }
            else if(OMX_CommandPortEnable == (OMX_COMMANDTYPE)nData1)
            {
                DEBUG_PRINT("*********************************************\n");
                DEBUG_PRINT("Recieved ENABLE Event Command Complete[%u]\n",nData2);
                DEBUG_PRINT("*********************************************\n");
            }
            else if(OMX_CommandFlush== (OMX_COMMANDTYPE)nData1)
            {
                DEBUG_PRINT("*********************************************\n");
                DEBUG_PRINT("Recieved FLUSH Event Command Complete[%u]\n",nData2);
                DEBUG_PRINT("*********************************************\n");
            }
            event_complete();
            break;
        case OMX_EventError:
            DEBUG_PRINT("*********************************************\n");
            DEBUG_PRINT("\n OMX_EventError \n");
            DEBUG_PRINT("*********************************************\n");
            break;

        case OMX_EventPortSettingsChanged:
            DEBUG_PRINT("*********************************************\n");
            DEBUG_PRINT("\n OMX_EventPortSettingsChanged \n");
            DEBUG_PRINT("*********************************************\n");
            event_complete();
            break;

        case OMX_EventBufferFlag:
            DEBUG_PRINT("*********************************************\n");
            DEBUG_PRINT("\n OMX_Bufferflag \n");
            DEBUG_PRINT("*********************************************\n");
            event_complete();
            break;
        default:
            DEBUG_PRINT("\n Unknown Event \n");
            break;
    }
    return OMX_ErrorNone;
}

/*=============================================================================
FUNCTION:
  FillBufferDone

DESCRIPTION:
  Callback function which is called by the OMX component to indicate buffer is
  filled with decoded data.

INPUT/OUTPUT PARAMETERS:
  [INOUT] pBuffer
  [IN] hComponent
  [IN] pAppData

RETURN VALUE:
  OMX_ERRORTYPE

Dependency:
   None

SIDE EFFECTS:
   None
=============================================================================*/

OMX_ERRORTYPE FillBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                              OMX_IN OMX_PTR pAppData,
                              OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    int bytes_writen = 0;
    static int count = 0;
    static int pcm_buf_size = 4800;
    static int pcm_buf_count = 2;
    struct msm_audio_config drv_pcm_config;
   (void)pAppData;
    if(count == 0 && pcmplayback)
    {
        DEBUG_PRINT(" open pcm device \n");
        if (m_pcmdrv_fd < 0)
        {
            DEBUG_PRINT("Cannot open audio device\n");
            return -1;
        }
        else
        {
            DEBUG_PRINT("Open pcm device successfull\n");
            DEBUG_PRINT("Configure Driver for PCM playback \n");
            ioctl(m_pcmdrv_fd, AUDIO_GET_CONFIG, &drv_pcm_config);
            DEBUG_PRINT("drv_pcm_config.buffer_count %d \n", drv_pcm_config.buffer_count);
            DEBUG_PRINT("drv_pcm_config.buffer_size %d \n", drv_pcm_config.buffer_size);
            drv_pcm_config.sample_rate = samplerate; //SAMPLE_RATE; //m_adec_param.nSampleRate;
            drv_pcm_config.channel_count = channels;  /* 1-> mono 2-> stereo*/
            ioctl(m_pcmdrv_fd, AUDIO_SET_CONFIG, &drv_pcm_config);
            DEBUG_PRINT("Configure Driver for PCM playback \n");
            ioctl(m_pcmdrv_fd, AUDIO_GET_CONFIG, &drv_pcm_config);
            DEBUG_PRINT("drv_pcm_config.buffer_count %d \n", drv_pcm_config.buffer_count);
            DEBUG_PRINT("drv_pcm_config.buffer_size %d \n", drv_pcm_config.buffer_size);
            pcm_buf_size = drv_pcm_config.buffer_size;
            pcm_buf_count = drv_pcm_config.buffer_count;
            DEBUG_PRINT("AUDIO_START called for PCM \n");
            ioctl(m_pcmdrv_fd, AUDIO_START, 0);

        }
    }
    DEBUG_PRINT(" FillBufferDone #%d size %u\n", count++,pBuffer->nFilledLen);

    if(bOutputEosReached)
    {
        return OMX_ErrorNone;
    }

    if((tunnel == 0) && (filewrite == 1))
    {
        bytes_writen =
            fwrite(pBuffer->pBuffer,1,pBuffer->nFilledLen,outputBufferFile);
        DEBUG_PRINT(" FillBufferDone size writen to file  %d\n",bytes_writen);
        totaldatalen += bytes_writen ;
    }

#ifdef PCM_PLAYBACK
    if(pcmplayback && pBuffer->nFilledLen)
    {
             if (write(m_pcmdrv_fd, pBuffer->pBuffer, pBuffer->nFilledLen ) !=
               (ssize_t)pBuffer->nFilledLen)
            {
                DEBUG_PRINT("FillBufferDone: Write data to PCM failed\n");
                return OMX_ErrorNone;
            }
		DEBUG_PRINT(" FillBufferDone: writing data to pcm device for play succesfull \n");
    }
#endif   // PCM_PLAYBACK


    if(pBuffer->nFlags != OMX_BUFFERFLAG_EOS)
    {
        DEBUG_PRINT(" FBD calling FTB");
        OMX_FillThisBuffer(hComponent,pBuffer);
    }
    else
    {
        DEBUG_PRINT(" FBD EOS REACHED...........\n");
        bOutputEosReached = true;
        event_complete();
        return OMX_ErrorNone;
    }
    return OMX_ErrorNone;
}

/*=============================================================================
FUNCTION:
  EmptyBufferDone

DESCRIPTION:
  Callback function which is called by the OMX component to indicate that buffer
  with encoded data is read and given to the driver/adsp for decoding

INPUT/OUTPUT PARAMETERS:
  [INOUT] pBuffer
  [IN] hComponent
  [IN] pAppData

RETURN VALUE:
OMX_ERRORTYPE

Dependency:
   None

SIDE EFFECTS:
   None
=============================================================================*/
OMX_ERRORTYPE EmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                              OMX_IN OMX_PTR pAppData,
                              OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
   int readBytes =0;
   (void)pAppData;

    DEBUG_PRINT("\nFunction %s cnt[%d]\n", __FUNCTION__, ebd_cnt);
    ebd_cnt++;
    used_ip_buf_cnt--;
    if(bInputEosReached)
    {
        DEBUG_PRINT("\n*********************************************\n");
        DEBUG_PRINT("   EBD::EOS on input port\n ");
        DEBUG_PRINT("   TBD:::De Init the open max here....!!!\n");
        DEBUG_PRINT("*********************************************\n");
        if(tunnel)
        {
            event_complete();
        }
        return OMX_ErrorNone;
    }
    else if (bFlushing == true)
    {
        DEBUG_PRINT("omx_Qcelp13_adec_test: bFlushing is set to TRUE used_ip_buf_cnt=%d\n",used_ip_buf_cnt);
        if (used_ip_buf_cnt == 0)
        {
            bFlushing = false;
        }
        else
        {
            DEBUG_PRINT("omx_Qcelp13_adec_test: more buffer to come back\n");
            return OMX_ErrorNone;
        }
    }
    readBytes = Read_Buffer(pBuffer);
    if(readBytes > 0)
    {
        pBuffer->nFilledLen = readBytes;
        used_ip_buf_cnt++;
        OMX_EmptyThisBuffer(hComponent,pBuffer);
    }
    else
    {
        pBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
        bInputEosReached = true;
        pBuffer->nFilledLen = 0;
        OMX_EmptyThisBuffer(hComponent,pBuffer);
        DEBUG_PRINT("EBD..Either EOS or Some Error while reading file\n");
    }
    return OMX_ErrorNone;
}

/*=============================================================================
FUNCTION:
  signal_handler

DESCRIPTION:
  Function which is used to handle signals.

INPUT/OUTPUT PARAMETERS:
  [IN] sig_id

RETURN VALUE:
..None

Dependency:
..None

SIDE EFFECTS:
   None
=============================================================================*/
void signal_handler(int sig_id)
{
    /* Flush */
    if (sig_id == SIGUSR1)
    {
        DEBUG_PRINT("%s Initiate flushing\n", __FUNCTION__);
        bFlushing = true;
        OMX_SendCommand(Qcelp13_dec_handle, OMX_CommandFlush, OMX_ALL, NULL);
    }
    else if (sig_id == SIGUSR2)
    {
        if (bPause == true)
        {
            DEBUG_PRINT("%s resume playback\n", __FUNCTION__);
            bPause = false;
            OMX_SendCommand(Qcelp13_dec_handle,
                OMX_CommandStateSet, OMX_StateExecuting, NULL);
        }
        else
        {
            DEBUG_PRINT("%s pause playback\n", __FUNCTION__);
            bPause = true;
            OMX_SendCommand(Qcelp13_dec_handle,
                OMX_CommandStateSet, OMX_StatePause, NULL);
        }
    }
}

/*=============================================================================
FUNCTION:
  main

DESCRIPTION:
  Entry point of the test application

INPUT/OUTPUT PARAMETERS:
  [IN] argc
  [IN] argv

RETURN VALUE:
  0

Dependency:
  None

SIDE EFFECTS:
   None
=============================================================================*/
int main(int argc, char **argv)
{
    int bufCnt=0;
    OMX_ERRORTYPE result;
    struct sigaction sa;

    struct wav_header hdr;
    int bytes_writen = 0;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &signal_handler;
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    pthread_cond_init(&cond, 0);
    pthread_mutex_init(&lock, 0);

    if (argc >= 4)
    {
        in_filename = argv[1];
        pcmplayback = atoi(argv[2]);
        tunnel  = atoi(argv[3]);
        filewrite = atoi(argv[4]);
        if (tunnel == 1)
        {
            pcmplayback = 0; /* This feature holds good only for non tunnel mode*/
            filewrite = 0;  /* File write not supported in tunnel mode */
        }
    }
    else
    {
        DEBUG_PRINT(" invalid format: \n");
        DEBUG_PRINT("ex: ./mm-adec-omxQcelp13k 13KINPUTFILE PCMPLAYBACK TUNNEL FILEWRITE\n");
        DEBUG_PRINT( "PCMPLAYBACK = 1 (ENABLES PCM PLAYBACK IN NON TUNNEL MODE) \n");
        DEBUG_PRINT( "PCMPLAYBACK = 0 (DISABLES PCM PLAYBACK IN NON TUNNEL MODE) \n");
        DEBUG_PRINT( "TUNNEL = 1 (DECODED QCELP13 SAMPLES IS PLAYED BACK)\n");
        DEBUG_PRINT( "TUNNEL = 0 (DECODED QCELP13 SAMPLES IS LOOPED BACK TO THE USER APP)\n");
        DEBUG_PRINT( "FILEWRITE = 1 (ENABLES PCM FILEWRITE IN NON TUNNEL MODE) \n");
        DEBUG_PRINT( "FILEWRITE = 0 (DISABLES PCM FILEWRITE IN NON TUNNEL MODE) \n");
        return 0;
    }


    if(tunnel == 0)
    {
        aud_comp = "OMX.qcom.audio.decoder.Qcelp13";
    }
    else
    {
        aud_comp = "OMX.qcom.audio.decoder.tunneled.Qcelp13";
    }
    DEBUG_PRINT(" OMX test app : aud_comp = %s\n",aud_comp);
#ifdef AUDIOV2
    m_pcmdrv_fd = open("/dev/msm_pcm_out", O_RDWR);
    ioctl(m_pcmdrv_fd, AUDIO_GET_SESSION_ID, &session_id_hpcm);
    devmgr_fd = open("/data/omx_devmgr", O_WRONLY);
    if(devmgr_fd >= 0)
    {
        //control = 0;
	write_devctlcmd(devmgr_fd, "-cmd=register_session_rx -sid=", session_id_hpcm);
	printf("devmgr_fd = %d\n",devmgr_fd);
    }
    else
    {
    control = msm_mixer_open("/dev/snd/controlC0", 0);
    if(control < 0)
    printf("ERROR opening the device\n");
    device_id = msm_get_device(device);
    DEBUG_PRINT ("\ndevice_id = %d\n",device_id);
    DEBUG_PRINT("\nsession_id = %d\n",session_id_hpcm);
    if (msm_en_device(device_id, 1))
    {
        perror("could not enable device\n");
        return -1;
    }
    if (msm_route_stream(1,session_id_hpcm,device_id, 1))
    {
        perror("could not set stream routing\n");
        return -1;
        }
    }
#endif
    if(Init_Decoder(aud_comp)!= 0x00)
    {
        DEBUG_PRINT("Decoder Init failed\n");
        return -1;
    }

    if(Play_Decoder() != 0x00)
    {
        DEBUG_PRINT("Play_Decoder failed\n");
        return -1;
    }

    // Wait till EOS is reached...
    wait_for_event();
    if(bOutputEosReached || (tunnel && bInputEosReached))
    {
#ifdef PCM_PLAYBACK
        if(pcmplayback == 1)
        {
            sleep(5); //slepp is introduced as driver doesnt support IOCTL STOP
            if(m_pcmdrv_fd >= 0)
            {
                close(m_pcmdrv_fd);
                m_pcmdrv_fd = -1;
                DEBUG_PRINT(" PCM device closed succesfully \n");
            }
            else
            {
                DEBUG_PRINT(" PCM device close failure \n");
            }
        }
#endif // PCM_PLAYBACK

        if((tunnel == 0) && (filewrite == 1))
        {
            hdr.riff_id = ID_RIFF;
            hdr.riff_sz = 0;
            hdr.riff_fmt = ID_WAVE;
            hdr.fmt_id = ID_FMT;
            hdr.fmt_sz = 16;
            hdr.audio_format = FORMAT_PCM;
            hdr.num_channels = channels;
            hdr.sample_rate = samplerate;
            hdr.byte_rate = hdr.sample_rate * hdr.num_channels * 2;
            hdr.block_align = hdr.num_channels * 2;
            hdr.bits_per_sample = 16;
            hdr.data_id = ID_DATA;
            hdr.data_sz = 0;

            DEBUG_PRINT("output file closed and EOS reached total decoded data length %d\n",totaldatalen);
            hdr.data_sz = totaldatalen;
            hdr.riff_sz = totaldatalen + 8 + 16 + 8;
            fseek(outputBufferFile, 0L , SEEK_SET);
            bytes_writen = fwrite(&hdr,1,sizeof(hdr),outputBufferFile);
            if (bytes_writen <= 0)
            {
                DEBUG_PRINT("Invalid Wav header write failed\n");
            }
            bFileclose = 1;
            fclose(outputBufferFile);
        }
        DEBUG_PRINT("\nMoving the decoder to idle state \n");
        OMX_SendCommand(Qcelp13_dec_handle,
            OMX_CommandStateSet, OMX_StateIdle,0);
        wait_for_event();
        DEBUG_PRINT("\nMoving the decoder to loaded state \n");
        OMX_SendCommand(Qcelp13_dec_handle,
            OMX_CommandStateSet, OMX_StateLoaded,0);
        DEBUG_PRINT("\nFillBufferDone: Deallocating i/p buffers \n");
        for(bufCnt=0; bufCnt < input_buf_cnt; ++bufCnt)
        {
            OMX_FreeBuffer(Qcelp13_dec_handle, 0, pInputBufHdrs[bufCnt]);
        }
        if(tunnel == 0)
        {
            DEBUG_PRINT("\nFillBufferDone: Deallocating o/p buffers \n");
            for(bufCnt=0; bufCnt < output_buf_cnt; ++bufCnt)
            {
                OMX_FreeBuffer(Qcelp13_dec_handle, 1, pOutputBufHdrs[bufCnt]);
            }
        }
        ebd_cnt=0;
        bInputEosReached = false;
        wait_for_event();
        ebd_cnt=0;
        bOutputEosReached = false;
        result = OMX_FreeHandle(Qcelp13_dec_handle);
        if (result != OMX_ErrorNone)
        {
            DEBUG_PRINT("\nOMX_FreeHandle error. Error code: %d\n", result);
        }
        Qcelp13_dec_handle = NULL;
#ifdef AUDIOV2
	if(devmgr_fd >= 0)
	{
            write_devctlcmd(devmgr_fd, "-cmd=unregister_session_rx -sid=", session_id_hpcm);
	    close(devmgr_fd);
	}
        else
        {
            if (msm_route_stream(1,session_id_hpcm,device_id, 0))
            {
                DEBUG_PRINT("\ncould not set stream routing\n");
                return -1;
            }
            if (msm_en_device(device_id, 0))
            {
                DEBUG_PRINT("\ncould not enable device\n");
                return -1;
            }
            msm_mixer_close();
        }
#endif
        /* Deinit OpenMAX */
        OMX_Deinit();
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&lock);

        DEBUG_PRINT("*****************************************\n");
        DEBUG_PRINT("******...TEST COMPLETED...***************\n");
        DEBUG_PRINT("*****************************************\n");
    }
    return 0;
}

/*=============================================================================
FUNCTION:
  Init_Decoder

DESCRIPTION:
  Function which initializes the OMX decoder component

INPUT/OUTPUT PARAMETERS:
  [IN] audio_component

RETURN VALUE:
  -1 Failure
  0 Success

Dependency:
   qvp_mmap_buffers should be called

SIDE EFFECTS:
   None
=============================================================================*/
int Init_Decoder(OMX_STRING audio_component)
{
    DEBUG_PRINT("Inside %s \n", __FUNCTION__);
    OMX_ERRORTYPE omxresult;
    OMX_U32 total = 0;
    OMX_U8** audCompNames = NULL;
    typedef OMX_U8* OMX_U8_PTR;
    OMX_STRING role ="audio_decoder.Qcelp13";


    static OMX_CALLBACKTYPE call_back = {
        &EventHandler,&EmptyBufferDone,&FillBufferDone
    };

  OMX_U32 i = 0;

    DEBUG_PRINT("Inside Play_Decoder - channels = %d\n", channels);
    DEBUG_PRINT("Inside Play_Decoder - pcmplayback = %d\n", pcmplayback);
    DEBUG_PRINT("Inside Play_Decoder - tunnel = %d\n", tunnel);

    /* Init. the OpenMAX Core */
    DEBUG_PRINT("\nInitializing OpenMAX Core....\n");
    omxresult = OMX_Init();

    if(OMX_ErrorNone != omxresult)
    {
        DEBUG_PRINT("\n Failed to Init OpenMAX core");
        return -1;
    }
    else
    {
        DEBUG_PRINT("\nOpenMAX Core Init Done\n");
    }


    OMX_STRING audiocomponent ="OMX.qcom.audio.decoder.Qcelp13";
    int componentfound = false;

    if(tunnel == 1)
    {
        audiocomponent = "OMX.qcom.audio.decoder.tunneled.Qcelp13";
    }
    /* Query for audio decoders*/
    DEBUG_PRINT("Qcelp13_test: Before entering OMX_GetComponentOfRole");
    OMX_GetComponentsOfRole(role, &total, 0);
    DEBUG_PRINT("\nTotal components of role=%s :%u", role, total);

    if(total)
    {
        DEBUG_PRINT("Total number of components = %u\n", total);
        /* Allocate memory for pointers to component name */
        audCompNames = (OMX_U8**)malloc((sizeof(OMX_U8))*total);

        if(audCompNames == NULL)
        {
            return -1;
        }

        for (i = 0; i < total; ++i)
        {
            audCompNames[i] =
                (OMX_U8*)malloc(sizeof(OMX_U8)*OMX_MAX_STRINGNAME_SIZE);
            if(audCompNames[i] == NULL)
            {
                while (i > 0)
                {
                    free(audCompNames[--i]);
                }
                free(audCompNames);
                audCompNames = NULL;
                return -1;
            }
            memset(&audCompNames[i],0,sizeof(audCompNames[i]));
        }
        DEBUG_PRINT("Before calling OMX_GetComponentsOfRole()\n");
        OMX_GetComponentsOfRole(role, &total, audCompNames);
        DEBUG_PRINT("\nComponents of Role:%s\n", role);
        for (i = 0; i < total; ++i)
        {
           //DEBUG_PRINT("\n Found Component[%s]\n",audCompNames[i]);
           // if(strcmp(audiocomponent,audCompNames[i]))
            {
                componentfound = true;
                audio_component = audiocomponent;
                DEBUG_PRINT("\n audiocomponent = %s\n",audiocomponent);
		    }
        }
    }
    else
    {
        DEBUG_PRINT("No components found with Role:%s", role);
    }
    if(componentfound == false)
    {
        DEBUG_PRINT("\n Not found audiocomponent = %s\n",audiocomponent);
        if (audCompNames != NULL) {
            free(audCompNames);
            audCompNames = NULL;
        }
        return -1;
    }
    omxresult = OMX_GetHandle((OMX_HANDLETYPE*)(&Qcelp13_dec_handle),
        audio_component, NULL, &call_back);
    if (FAILED(omxresult))
    {
        DEBUG_PRINT("\nFailed to Load the component OMX.qcom.audio.decoder.Qcelp13\n");
        if (audCompNames != NULL) {
            free(audCompNames);
            audCompNames = NULL;
        }
        return -1;
    }
    else
    {
        if (audCompNames != NULL) {
           DEBUG_PRINT("\nComponent %s is in LOADED state\n", audCompNames[i-1]);
        }
    }

    /* Get the port information */
    CONFIG_VERSION_SIZE(portParam);
    omxresult = OMX_GetParameter(Qcelp13_dec_handle, OMX_IndexParamAudioInit,
        (OMX_PTR)&portParam);

    if(FAILED(omxresult))
    {
        DEBUG_PRINT("\nFailed to get Port Param\n");
        if (audCompNames != NULL) {
            free(audCompNames);
            audCompNames = NULL;
        }
        return -1;
    }
    else
    {
        DEBUG_PRINT("\nportParam.nPorts:%u\n", portParam.nPorts);
        DEBUG_PRINT("\nportParam.nStartPortNumber:%u\n",
            portParam.nStartPortNumber);
    }
    if (audCompNames != NULL) {
        free(audCompNames);
        audCompNames = NULL;
    }
    return 0;
}

/*=============================================================================
FUNCTION:
  Play_Decoder

DESCRIPTION:
  Function which configure the PCM driver and initiates the playback

INPUT/OUTPUT PARAMETERS:
  None

RETURN VALUE:
  -1 Failure
  0 Success

Dependency:
  None

SIDE EFFECTS:
   None
=============================================================================*/
int Play_Decoder()
{
    int i;
    int Size=0;
    DEBUG_PRINT("Inside %s \n", __FUNCTION__);
    OMX_ERRORTYPE ret;
    DEBUG_PRINT("sizeof[%lu]\n", sizeof(OMX_BUFFERHEADERTYPE));

    /* open the i/p and o/p files based on the video file format passed */
    if(open_audio_file())
    {
        DEBUG_PRINT("\n Returning -1");
        return -1;
    }
    /* Query the decoder input min buf requirements */
    CONFIG_VERSION_SIZE(inputportFmt);

    /* Port for which the Client needs to obtain info */
    inputportFmt.nPortIndex = portParam.nStartPortNumber;

    OMX_GetParameter(Qcelp13_dec_handle,
        OMX_IndexParamPortDefinition,&inputportFmt);
    DEBUG_PRINT ("\nDec: Input Buffer Count %u\n", inputportFmt.nBufferCountMin);
    DEBUG_PRINT ("\nDec: Input Buffer Size %u\n", inputportFmt.nBufferSize);

    if(OMX_DirInput != inputportFmt.eDir)
    {
        DEBUG_PRINT ("\nDec: Expect Input Port\n");
        return -1;
    }
   inputportFmt.nBufferCountActual = inputportFmt.nBufferCountMin + 5;
   OMX_SetParameter(Qcelp13_dec_handle,OMX_IndexParamPortDefinition,&inputportFmt);

    if(tunnel == 0)
    {
        /* Query the decoder outport's min buf requirements */
        CONFIG_VERSION_SIZE(outputportFmt);
        /* Port for which the Client needs to obtain info */
        outputportFmt.nPortIndex = portParam.nStartPortNumber + 1;

        OMX_GetParameter(Qcelp13_dec_handle,
            OMX_IndexParamPortDefinition,&outputportFmt);
        DEBUG_PRINT ("\nDec: Output Buffer Count %u\n", outputportFmt.nBufferCountMin);
        DEBUG_PRINT ("\nDec: Output Buffer Size %u\n", outputportFmt.nBufferSize);

        if(OMX_DirOutput != outputportFmt.eDir)
        {
            DEBUG_PRINT ("\nDec: Expect Output Port\n");
            return -1;
        }
    outputportFmt.nBufferCountActual = outputportFmt.nBufferCountMin;
    //outputportFmt.nBufferCountActual = outputportFmt.nBufferCountMin+7;
    OMX_SetParameter(Qcelp13_dec_handle,OMX_IndexParamPortDefinition,&outputportFmt);
    }

    CONFIG_VERSION_SIZE(Qcelp13param);

    Qcelp13param.nPortIndex   =  0;
    Qcelp13param.nChannels    =  channels;  /* 1-> mono */

    OMX_SetParameter(Qcelp13_dec_handle,
        OMX_IndexParamAudioQcelp13,&Qcelp13param);

    DEBUG_PRINT ("\nOMX_SendCommand Decoder -> IDLE\n");
    OMX_SendCommand(Qcelp13_dec_handle, OMX_CommandStateSet, OMX_StateIdle,0);

    input_buf_cnt = inputportFmt.nBufferCountMin + 5;
    DEBUG_PRINT("Transition to Idle State succesful...\n");
    /* Allocate buffer on decoder's i/p port */
    error = Allocate_Buffer(Qcelp13_dec_handle, &pInputBufHdrs,
        inputportFmt.nPortIndex, input_buf_cnt, inputportFmt.nBufferSize);
    if (error != OMX_ErrorNone || !pInputBufHdrs)
    {
        DEBUG_PRINT ("\nOMX_AllocateBuffer Input buffer error\n");
        return -1;
    }
    else
    {
        DEBUG_PRINT ("\nOMX_AllocateBuffer Input buffer success\n");
    }

    if(tunnel == 0)
    {
        output_buf_cnt = outputportFmt.nBufferCountMin ;
        /* Allocate buffer on decoder's O/Pp port */
        error = Allocate_Buffer(Qcelp13_dec_handle, &pOutputBufHdrs,
            outputportFmt.nPortIndex, output_buf_cnt,
            outputportFmt.nBufferSize);
        if (error != OMX_ErrorNone || !pOutputBufHdrs)
        {
            DEBUG_PRINT ("\nOMX_AllocateBuffer Output buffer error\n");
            return -1;
        }
        else
        {
            DEBUG_PRINT ("\nOMX_AllocateBuffer Output buffer success\n");
        }
    }

    wait_for_event();

    if (tunnel == 1)
    {
        DEBUG_PRINT ("\nOMX_SendCommand to enable TUNNEL MODE during IDLE\n");
        OMX_SendCommand(Qcelp13_dec_handle, OMX_CommandPortDisable,1,0);
        wait_for_event();
    }

    DEBUG_PRINT ("\nOMX_SendCommand Decoder -> Executing\n");
    OMX_SendCommand(Qcelp13_dec_handle,
        OMX_CommandStateSet, OMX_StateExecuting,0);
    wait_for_event();

    if(tunnel == 0)
    {
        DEBUG_PRINT(" Start sending OMX_FILLthisbuffer\n");

        for(i=0; i < output_buf_cnt; i++)
        {
            DEBUG_PRINT ("\nOMX_FillThisBuffer on output buf no.%d\n",i);
            pOutputBufHdrs[i]->nOutputPortIndex = 1;
            pOutputBufHdrs[i]->nFlags &= ~OMX_BUFFERFLAG_EOS;
            ret = OMX_FillThisBuffer(Qcelp13_dec_handle, pOutputBufHdrs[i]);
            if (OMX_ErrorNone != ret)
            {
                DEBUG_PRINT("OMX_FillThisBuffer failed with result %d\n", ret);
            }
            else
            {
                DEBUG_PRINT("OMX_FillThisBuffer success!\n");
            }
        }
    }
    DEBUG_PRINT(" Start sending OMX_emptythisbuffer\n");
    for (i = 0;i < input_buf_cnt;i++)
    {
        DEBUG_PRINT ("\nOMX_EmptyThisBuffer on Input buf no.%d\n",i);
        pInputBufHdrs[i]->nInputPortIndex = 0;
        Size = Read_Buffer(pInputBufHdrs[i]);
        if(Size <=0 )
        {
            DEBUG_PRINT("NO DATA READ\n");
        }
        pInputBufHdrs[i]->nFilledLen = Size;
        pInputBufHdrs[i]->nInputPortIndex = 0;
        used_ip_buf_cnt++;
        ret = OMX_EmptyThisBuffer(Qcelp13_dec_handle, pInputBufHdrs[i]);
        if (OMX_ErrorNone != ret)
        {
            DEBUG_PRINT("OMX_EmptyThisBuffer failed with result %d\n", ret);
        }
        else
        {
            DEBUG_PRINT("OMX_EmptyThisBuffer success!\n");
        }
    }
    return 0;
}

/*=============================================================================
FUNCTION:
  Allocate_Buffer

DESCRIPTION:
  Function which is used to allocate input and output buffers

INPUT/OUTPUT PARAMETERS:
  [INOUT] pBufHdrs
  [IN] avc_dec_handle
  [IN] bufCntMin
  [IN] bufSize
  [IN] nPortIndex

RETURN VALUE:
  OMX_ERRORTYPE

Dependency:
  None

SIDE EFFECTS:
  None
=============================================================================*/
static OMX_ERRORTYPE Allocate_Buffer ( OMX_COMPONENTTYPE *avc_dec_handle,
                                       OMX_BUFFERHEADERTYPE  ***pBufHdrs,
                                       OMX_U32 nPortIndex,
                                       long bufCntMin, long bufSize)
{
    DEBUG_PRINT("Inside %s \n", __FUNCTION__);
    OMX_ERRORTYPE error=OMX_ErrorNone;
    long bufCnt=0;
    (void)avc_dec_handle;
    *pBufHdrs= (OMX_BUFFERHEADERTYPE **)
      malloc(sizeof(OMX_BUFFERHEADERTYPE*)*bufCntMin);

    for(bufCnt=0; bufCnt < bufCntMin; ++bufCnt)
    {
        DEBUG_PRINT("\n OMX_AllocateBuffer No %lu \n", bufCnt);
        error = OMX_AllocateBuffer(Qcelp13_dec_handle, &((*pBufHdrs)[bufCnt]),
            nPortIndex, NULL, bufSize);
    }
    return error;
}

/*=============================================================================
FUNCTION:
  Read_Buffer

DESCRIPTION:
  Function which is used to read encoded data from the input file

INPUT/OUTPUT PARAMETERS:
  [IN] pBufHdr

RETURN VALUE:
  Number of bytes read

Dependency:
  None

SIDE EFFECTS:
  None
=============================================================================*/
static int Read_Buffer (OMX_BUFFERHEADERTYPE  *pBufHdr )
{
    int bytes_read = 0;
    static int totalbytes_read =0;

    DEBUG_PRINT ("\nInside Read_Buffer\n");

    pBufHdr->nFilledLen = 0;
    pBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;

    bytes_read = fread(pBufHdr->pBuffer, 1, pBufHdr->nAllocLen ,
        inputBufferFile);
    totalbytes_read += bytes_read;
    DEBUG_PRINT ("\bytes_read = %d\n",bytes_read);
    DEBUG_PRINT ("\totalbytes_read = %d\n",totalbytes_read);
    pBufHdr->nFilledLen = bytes_read;
    if( bytes_read == 0 || (totalbytes_read > chunksize))
    {
        pBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
        pBufHdr->nTimeStamp = timeStampLfile;
        timeStampLfile += timestampInterval;
        DEBUG_PRINT ("\nBytes read zero\n");
    }
    else
    {
        pBufHdr->nTimeStamp = timeStampLfile;
        timeStampLfile += timestampInterval;
        pBufHdr->nFlags &= ~OMX_BUFFERFLAG_EOS;
        DEBUG_PRINT ("\nBytes read is Non zero\n");
    }
    return bytes_read;
}

/*=============================================================================
FUNCTION:
  open_audio_file

DESCRIPTION:
  Function to open the input audio file

INPUT/OUTPUT PARAMETERS:
  None

RETURN VALUE:
  FALSE Failure
  TRUE Success

Dependency:


SIDE EFFECTS:
  None
=============================================================================*/
static int open_audio_file ()
{
    int error_code = 0;
    struct wav_header hdr;
    int header_len = 0;
    memset(&hdr,0,sizeof(hdr));
    hdr.riff_id = ID_RIFF;
    hdr.riff_sz = 0;
    hdr.riff_fmt = ID_WAVE;
    hdr.fmt_id = ID_FMT;
    hdr.fmt_sz = 16;
    hdr.audio_format = FORMAT_PCM;
    hdr.num_channels = channels;
    hdr.sample_rate = samplerate;
    hdr.byte_rate = hdr.sample_rate * hdr.num_channels * 2;
    hdr.block_align = hdr.num_channels * 2;
    hdr.bits_per_sample = 16;
    hdr.data_id = ID_DATA;
    hdr.data_sz = 0;
    OMX_U8  chunk_id[] = {'d', 'a', 't', 'a'};
    OMX_U8  parsebuffer[4];
    OMX_U8  parsefmtbuffer[10];
    int readerror = 0;
    int Size = 0;
    OMX_U8  parsechunksize[4];
    const OMX_U8 qcpsup_13k_guid1[] = {
        0x41,0x6D,0x7F,0x5E,
        0x15,0xB1,
        0xD0,0x11,
        0xBA,0x91,0x00,0x80,0x5F,0xB4,0xB9,0x7E
    };

    const OMX_U8 qcpsup_13k_guid2[] = {
        0x42,0x6D,0x7F,0x5E,
        0x15,0xB1,
        0xD0,0x11,
        0xBA,0x91,0x00,0x80,0x5F,0xB4,0xB9,0x7E
    };

    DEBUG_PRINT("Inside %s filename=%s\n", __FUNCTION__, in_filename);
    inputBufferFile = fopen (in_filename, "rb");
    if (inputBufferFile == NULL)
    {
        DEBUG_PRINT("\ni/p file %s could NOT be opened\n",
            in_filename);
        error_code = -1;
        return error_code;
    }

    /*Qcp parser code*/
    fseek(inputBufferFile, 22 , SEEK_CUR);
    Size = fread(parsefmtbuffer, 1, 10, inputBufferFile);
    if(Size <=0 )
    {
        DEBUG_PRINT("NO DATA READ Parser failed\n");
        error_code = -1;
        readerror  = 1;
    }
    if((strncmp((char *) parsefmtbuffer, (char *) qcpsup_13k_guid1, 10) == 0)
        ||(strncmp((char *) parsefmtbuffer, (char *) qcpsup_13k_guid2, 10) == 0))
    {
        DEBUG_PRINT("13kQCELP file format in qcp recognised\n");
    }
    else
    {
        DEBUG_PRINT("unknownsupported file format in qcp recognised\n");
        error_code = -1;
    }
    while(!readerror)
    {
        Size = fread(parsebuffer, 1, 4, inputBufferFile);
        if(Size <=0 )
        {
            DEBUG_PRINT("NO DATA READ Parser failed\n");
            error_code = -1;
            readerror  = 1;
            break;
        }

        if(strncmp((char *) chunk_id, (char *) parsebuffer, 4) == 0)
        {
            DEBUG_PRINT("Parser frame header success\n");
            Size = fread(parsechunksize, 1, 4 , inputBufferFile);
            chunksize = (parsechunksize[3] << 24) | (parsechunksize[2] << 16) |
                (parsechunksize[1] << 8)  | (parsechunksize[0]);
            DEBUG_PRINT("Data chunksize = %d\n",chunksize);
            break;
        }
        else
        {
            fseek(inputBufferFile, -3 , SEEK_CUR);
        }
    }

    if((tunnel == 0) && (filewrite == 1))
    {
        DEBUG_PRINT("output file is opened\n");
        outputBufferFile = fopen("Audio_Qcelp13.wav","wb");
      //  outputBufferFile = fopen(out_filename,"wb");
        if (outputBufferFile == NULL)
        {
            DEBUG_PRINT("\no/p file %s could NOT be opened\n",
                out_filename);
            error_code = -1;
            return error_code;
        }

        header_len = fwrite(&hdr,1,sizeof(hdr),outputBufferFile);


        if (header_len <= 0)
        {
            DEBUG_PRINT("Invalid Wav header \n");
        }
        DEBUG_PRINT(" Length og wav header is %d \n",header_len );
    }
    return error_code;
}

static void write_devctlcmd(int fd, const void *buf, int param){
	int nbytes, nbytesWritten;
	char cmdstr[128];
	snprintf(cmdstr, 128, "%s%d\n", (char *)buf, param);
	nbytes = strlen(cmdstr);
	nbytesWritten = write(fd, cmdstr, nbytes);

	if(nbytes != nbytesWritten)
		printf("Failed to write string \"%s\"to omx_devmgr\n",cmdstr);
}
