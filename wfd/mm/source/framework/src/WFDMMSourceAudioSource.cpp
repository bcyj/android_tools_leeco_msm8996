/*=======================================================================
                              WFDMMSourceAudioSource.cpp
DESCRIPTION

This module is for WFD source implementation
Interacts with PCM driver and other audio encoders and muxer

Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/framework/src/WFDMMSourceAudioSource.cpp#2 $
$DateTime: 2012/02/10 05:45:30 $
$Change:$
========================================================================== */
/*========================================================================
 *                    INCLUDE FILES FOR MODULE
 *========================================================================*/

#include <sys/ioctl.h>
#include <sys/time.h>
#include <utils/Log.h>
#include <errno.h>
#include <threads.h>
#include <utils/String8.h>
#include "WFDMMSourceSignalQueue.h"
#include "WFDMMSourceAudioSource.h"
#include "WFDMMSourceAudioEncode.h"
#include "QOMX_VideoExtensions.h"
#include "MMMalloc.h"
#include "MMMemory.h"
#include "MMTimer.h"
#include "MMDebugMsg.h"
#include "wfd_cfg_parser.h"
#include"media/AudioParameter.h"
#include"media/AudioSystem.h"
#include "AudioSystemLegacy.h"
#ifndef WFD_ICS
#include "common_log.h"
#endif
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <linux/msm_ion.h>
#include <fcntl.h>
#include <cutils/properties.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "WFDMMSourceAudioSource"


/*========================================================================
 *                    CONSTANT DECLARATION
 *========================================================================*/

#define AUDIO_ION_BUF_SIZE 16384 // page size in linux which has to be the minimum size for mmap
#define LPCM_PRIVATE_STREAM_HEADER 4
#define AUDIO_BITS_PER_SAMPLE 16  // for both 44.1 and 48 Khz
#define WFD_MM_SOURCE_THREAD_STACK_SIZE 16384
#define WFD_MM_AUDIO_SOURCE_THREAD_PRIORITY -19

#define PCM_HW_PARAM_PERIOD_SIZE 480
//#define AAC_HW_PARAM_PERIOD_SIZE 512
#define AAC_HW_PARAM_PERIOD_SIZE 1536

#define OMX_AUDIO_PCM_FRAME 0x01000000
#define PCM_AUDIO_SAMPLE_SIZE  PCM_HW_PARAM_PERIOD_SIZE * 4

#define WFD_TIME_NSEC_IN_MSEC 1000000
#define WFD_TIME_USEC_IN_MSEC 1000
#define WFD_TIME_MSEC_IN_SEC 1000
#define WFD_TIME_NSEC_IN_USEC 1000

#define WFD_TIME_USEC_IN_SEC 1000000
#define WFD_TIME_NSEC_IN_USEC 1000

#define WFD_TIME_NSEC_IN_SEC 1000000000

inline bool IS_PROXY_IN_USE() {
  if(((android::AudioSystem::getDeviceConnectionState((audio_devices_t)
  AUDIO_DEVICE_OUT_BLUETOOTH_A2DP,""))
      == AUDIO_POLICY_DEVICE_STATE_AVAILABLE) ||
  ((android::AudioSystem::getDeviceConnectionState((audio_devices_t)
  AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES,""))
   == AUDIO_POLICY_DEVICE_STATE_AVAILABLE) ||
  ((android::AudioSystem::getDeviceConnectionState((audio_devices_t)
  AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER,""))
   == AUDIO_POLICY_DEVICE_STATE_AVAILABLE)||
  ((android::AudioSystem::getDeviceConnectionState((audio_devices_t)
  AUDIO_DEVICE_OUT_USB_ACCESSORY,""))
   == AUDIO_POLICY_DEVICE_STATE_AVAILABLE) ||
  ((android::AudioSystem::getDeviceConnectionState((audio_devices_t)
  AUDIO_DEVICE_OUT_USB_DEVICE,""))
   == AUDIO_POLICY_DEVICE_STATE_AVAILABLE) ||
  ((android::AudioSystem::getDeviceConnectionState((audio_devices_t)
  AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET,""))
   == AUDIO_POLICY_DEVICE_STATE_AVAILABLE) ||
  ((android::AudioSystem::getDeviceConnectionState((audio_devices_t)
  AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET,""))
   == AUDIO_POLICY_DEVICE_STATE_AVAILABLE)
 )
  {
    return OMX_TRUE;
  }
  else
  {
    return OMX_FALSE;
  }
}

inline OMX_BOOL IS_DUMP_ENABLE(AudioDump format) {
    int ret = 0;
    char szTemp[PROPERTY_VALUE_MAX];
    switch(format)
    {
        case AAC_DUMP:
            ret = property_get("persist.debug.wfd.dumpaac",szTemp,NULL);
            if (ret <= 0 )
            {
                MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_ERROR,"Failed to read prop %d %s value", ret, szTemp);
                return OMX_FALSE;
            }
            break;
        case WAV_DUMP:
            ret = property_get("persist.debug.wfd.dumpwav",szTemp,NULL);
            if (ret <= 0 )
            {
                MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_ERROR,"Failed to read prop %d %s value", ret, szTemp);
                return OMX_FALSE;
            }
            break;
        default:
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"!!!Invalid format type for audio dump");
            return OMX_FALSE;
    }
    if(strcmp(szTemp,"1")==0)
    {
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_DEBUG,"IS_DUMP_ENABLE OMX_TRUE for %d", format);
        return OMX_TRUE;
    }
    else
    {
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_DEBUG,"IS_DUMP_ENABLE OMX_FALSE for %d", format);
        return OMX_FALSE;
    }
}

const OMX_U32 AUDIO_BUFFER_QUEUE_SIZE = 30; // changed to 30 from 100 during audio encryption feature

const int nAudioSourceThreadPriority = 99;

const OMX_U32 AudioSource::AUDIO_SOURCE_PLAY_EVENT= 1;
const OMX_U32 AudioSource::AUDIO_SOURCE_PAUSE_EVENT= 2;
const OMX_U32 AudioSource::AUDIO_SOURCE_EXIT_EVENT= 5;

/**!
 * @brief Helper macro to set private/internal flag
 */
#define FLAG_SET(_pCtx_, _f_) (_pCtx_)->nFlags  |= (_f_)

/**!
 * @brief Helper macro to check if a flag is set or not
 */
#define FLAG_ISSET(_pCtx_, _f_) (((_pCtx_)->nFlags & (_f_)) ? OMX_TRUE : OMX_FALSE)

/**!
 * @brief Helper macro to clear a private/internal flag
 */
#define FLAG_CLEAR(_pCtx_, _f_) (_pCtx_)->

#ifdef WFD_DUMP_AUDIO_DATA
#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
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
    uint16_t block_align;      /* num_channels * bps / 8 */
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};

#endif

/*==============================================================================

         FUNCTION:         AudioSource Constructor

         DESCRIPTION:
*//**       @brief         This is the WFDMMAudioSource class constructor - initializes
                           the class members.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None


*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:

*//*==========================================================================*/
  AudioSource::AudioSource()
 :
    m_pHdcpHandle(NULL),
    m_nFramesRegistered(0),
    m_pBufferQueue(new SignalQueue(MAX_BUFFER_ASSUME, sizeof(OMX_BUFFERHEADERTYPE*))),
    m_AudioSourceThreadHandle(NULL),
    m_bStarted(OMX_FALSE),
    m_pFrameDeliverFn(NULL),
    m_pInputBuffers(NULL),
    m_AudiosignalQ(NULL),
    bRunning(OMX_FALSE),
    m_bEncryptFlag(OMX_FALSE),
    m_nSamplingFreq(0),
    m_nAudiodevice(0),
    m_nAudioBufsize(0),
    m_pAudioDataBfr(NULL),
    m_pAudioEncoder(NULL),
    m_WFDAudioSourcePlaySignal(NULL),
    m_WFDAudioSourcePauseSignal(NULL),
    m_WFDAudioSourceExitSignal(NULL)
  {
      m_nDownStreamFrames = 0;
      m_nAudioDelay = 0;
      m_nBuffers = AUDIO_BUFFER_QUEUE_SIZE;
      m_eAudioSrcState = WFDMM_AUDIOSOURCE_STATE_INIT;
      m_pHDCPInputBuffers = NULL;
      m_nDropFrames = 0;
#ifdef AUDIO_DUMP_ENABLE
      m_fpAudio = NULL;
      m_bDumpAudio = IS_DUMP_ENABLE(AAC_DUMP);
#endif
#ifdef WFD_DUMP_AUDIO_DATA
      m_fpWav = NULL;
      m_bDumpWav = IS_DUMP_ENABLE(WAV_DUMP);
#endif
      memset((void*)&m_PcmConfig,0x0,sizeof(struct pcm_config));
      return;
  }

/*============================================================================

         FUNCTION:         Audio Source Destructor

         DESCRIPTION:
*//**       @brief         This is the WFDMMAudioSource class Destructor
                           De-initializes the class members.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None


*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:

*//*==========================================================================*/
  AudioSource::~AudioSource()
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_ERRORTYPE threadResult;

    MM_Signal_Set(m_WFDAudioSourceExitSignal);
    int exitCode = 0;
    // wait for thread to exit
    MM_Thread_Join( m_AudioSourceThreadHandle, &exitCode );

    if(m_pAudioDataBfr)
    {
      MM_Free(m_pAudioDataBfr);
      m_pAudioDataBfr = NULL;
    }

    if(m_pAudioEncoder)
    {
      MM_Delete(m_pAudioEncoder);
      m_pAudioEncoder = NULL;
    }

    if(m_eAudioCodingType == OMX_AUDIO_CodingAAC ||
       m_eAudioCodingType == OMX_AUDIO_CodingAC3)
    {
#ifdef AUDIO_DUMP_ENABLE
      if (m_fpAudio)
      {
        fclose(m_fpAudio);
      }
#endif
    }

#ifdef WFD_DUMP_AUDIO_DATA
    if(m_fpWav)
    {
        fclose(m_fpWav);
    }
#endif
    m_bStarted = OMX_FALSE;

    if (m_pBufferQueue != NULL)
    {
      delete m_pBufferQueue;
    }

    if(m_AudioSourceThreadHandle)
    {
      MM_Thread_Release( m_AudioSourceThreadHandle );
      m_AudioSourceThreadHandle = NULL;
    }

    //releasing audio thread signals
    if(m_WFDAudioSourcePlaySignal != NULL)
    {
      MM_Signal_Release(m_WFDAudioSourcePlaySignal);
    }
    if(m_WFDAudioSourcePauseSignal != NULL)
    {
      MM_Signal_Release(m_WFDAudioSourcePauseSignal);
    }
    if(m_WFDAudioSourceExitSignal != NULL)
    {
      MM_Signal_Release(m_WFDAudioSourceExitSignal);
    }
    if(m_AudiosignalQ != NULL)
    {
      MM_SignalQ_Release(m_AudiosignalQ);
    }

    for (int i = 0; i < m_nBuffers; i ++)
    {
      int ret = 0;
      struct ion_handle_data handle_data;

       if((m_pHdcpHandle != NULL) &&(m_pHDCPInputBuffers != NULL))
       {
        if(m_pHDCPInputBuffers[i].start != NULL)
        {
          if (munmap(m_pHDCPInputBuffers[i].start, m_pHDCPInputBuffers[i].length) == -1)
          {
             MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                  "AudioSource::error in munmap = %d", i);
          }
          m_pHDCPInputBuffers[i].start = NULL;
          m_pHDCPInputBuffers[i].length = 0;
        }

        if(m_pHDCPInputBuffers[i].handle != 0)
        {
          handle_data.handle = m_pHDCPInputBuffers[i].handle;
          ret = ioctl(ionfd, ION_IOC_FREE, &handle_data);
          m_pHDCPInputBuffers[i].handle = 0;
        }
        if(m_pHDCPInputBuffers[i].fd > 0)
        {
          close(m_pHDCPInputBuffers[i].fd);
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                    "AudioSource::closing hdcp ion fd = %d ret type = %d \n", m_pHDCPInputBuffers[i].fd, ret);
          m_pHDCPInputBuffers[i].fd = -1;
        }
      }
      if(m_pInputBuffers)
      {
       OMX_BUFFERHEADERTYPE * pHeader = m_pInputBuffers[i];
       if(pHeader != NULL)
       {
        if(pHeader->pBuffer != NULL && !m_bEncryptFlag)
        {
         MM_Free(pHeader->pBuffer);
         pHeader->pBuffer = NULL;
        }
        MM_Free(pHeader);
        pHeader = NULL;
       }
     }
    }
    if(ionfd > 0)
    {
      close(ionfd);
      ionfd = -1;
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "AudioSource::closing ion fd = %d",ionfd);
    }
    if(m_pHDCPInputBuffers)
    {
      MM_Free(m_pHDCPInputBuffers);
      m_pHDCPInputBuffers = NULL;
    }
    if(m_pInputBuffers != NULL)
    {
       MM_Delete_Array(m_pInputBuffers);
    }

   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "AudioSource::audio thread finished");
  }

/*==============================================================================

         FUNCTION:         Configure

         DESCRIPTION:
*//**       @brief         This is the WFDMMAudioSource configure function for
                           Buffer Configuration.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pConfig  -  WFD Config type
                           pFrameDeliverFn  - Frame Delivery Function
                           pFileName - Filename
                           appData - App data


*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:

*//*==========================================================================*/
  OMX_ERRORTYPE AudioSource::Configure( AudioEncConfigType *pConfig,
      FrameDeliveryFnType pFrameDeliverFn,
      eventHandlerType pEventHandlerFn,
      OMX_STRING pFileName,
      OMX_U32 nModuleId,
      void *appData)
  {
      (void)pConfig;
      (void)pFileName;
      OMX_ERRORTYPE result = OMX_ErrorNone;
      OMX_U16 debug = 1;
      unsigned int uBufferSize = 0;
      unsigned int uAudioIonBufSize = AUDIO_ION_BUF_SIZE;
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "AudioSource::Configure AudioSource");
      int tempAudioDelay = 0;
      getCfgItem(AUDIO_AV_SYNC_DEL_KEY,&tempAudioDelay);
      {
          m_nAudioDelay = tempAudioDelay;
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
              "AudioSource:: Audio delay %d", m_nAudioDelay);
      }

      if (pConfig->nAudioInBufCount > 0 &&
          pConfig->nAudioInBufCount <= MAX_BUFFER_ASSUME &&
          pFrameDeliverFn != NULL &&
          pEventHandlerFn != NULL)
      {
          m_pEventHandlerFn = pEventHandlerFn;
          m_nModuleId       = nModuleId;
          m_nBuffers = pConfig->nAudioInBufCount;
          m_pFrameDeliverFn = pFrameDeliverFn;
          m_appData = appData;
          m_eAudioCodingType =     pConfig->eCoding;
          m_PcmConfig.channels = (unsigned int) pConfig->nNumChannels;
          m_nSamplingFreq = (OMX_U32) pConfig->nSamplingFrequency;
          m_PcmConfig.rate = (unsigned int) pConfig->nSamplingFrequency;
          m_nBitRate = (OMX_U32)pConfig->nBitrate;
          m_nAudiodevice = 8;
          //m_nAudioflags |= PCM_NMMAP;
          m_eAudioSrcState = WFDMM_AUDIOSOURCE_STATE_IDLE;
          m_nTimeErrAdjust = 0;
          m_nTimeErrAdjustCnt = 0;
          m_nTimeErrAdjustCntr = 0;
          m_nAudioParamPeriodSize = AAC_HW_PARAM_PERIOD_SIZE;//Default to AAC

          bKeepGoing = OMX_TRUE;

          result = CreateResources();


          if( m_eAudioCodingType == OMX_AUDIO_CodingPCM )
          {
              uBufferSize = PCM_AUDIO_SAMPLE_SIZE + 4;
              m_nFrameTimeUnit = ((PCM_HW_PARAM_PERIOD_SIZE) *
                                         WFD_TIME_NSEC_IN_MSEC)/m_nSamplingFreq;
          }
          else
          {
              uBufferSize = m_nAudioParamPeriodSize * 2 * m_PcmConfig.channels;
              m_nFrameTimeUnit = (m_nAudioParamPeriodSize *
                         WFD_TIME_NSEC_IN_MSEC + (m_nSamplingFreq/2))/m_nSamplingFreq;
              //Downstream components work on ms timescale
              m_nFrameTimeUnit = (m_nFrameTimeUnit / WFD_TIME_USEC_IN_MSEC) * WFD_TIME_USEC_IN_MSEC;
              m_nAudioBufsize = m_nAudioParamPeriodSize * m_PcmConfig.channels * 2;
              //Calculate error adjustment
              m_nTimeErrAdjust = (OMX_S64)WFD_TIME_USEC_IN_MSEC * 1;
              m_nTimeErrAdjustCnt = 3;
          }

          /**---------------------------------------------------------------------
                 Decision to encrypt audio or not is made by application
                 or user based on the WFD config file
                ------------------------------------------------------------------------
            */
          if( ( NULL != m_pHdcpHandle ) && ( HDCP_STATUS_SUCCESS == m_pHdcpHandle->m_eHdcpSessionStatus ) )
          {
              int32 nEncryptAudioDecision = 0;
              int32 nRetVal = -1;

              m_bEncryptFlag = OMX_FALSE;
              int tempEncryptAudio = 0;
              nRetVal = getCfgItem(ENCRYPT_AUDIO_DECISION_KEY,&tempEncryptAudio);
              if(nRetVal == 0)
              {
                nEncryptAudioDecision = tempEncryptAudio;
                m_bEncryptFlag = nEncryptAudioDecision ? OMX_TRUE : OMX_FALSE;
              }
              MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                                 "AudioSource::Configure: Audio Encryption Flag m_bEncryptFlag= %d",
                                 m_bEncryptFlag);
           }

          if( m_bEncryptFlag &&
            m_pHdcpHandle &&
           (HDCP_STATUS_SUCCESS == m_pHdcpHandle->m_eHdcpSessionStatus))
          {

            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "AudioSource::HDCP Session - m_bEncrypt flag = %d "\
              "HDCP session status = %d",m_bEncryptFlag,m_pHdcpHandle->m_eHdcpSessionStatus);

            /* Open ION device*/
            ionfd = open("/dev/ion",  O_RDONLY);
            if (ionfd < 0)
            {
              MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                           "AudioSource::Failed to open ion device = %d\n", ionfd);
              return OMX_ErrorInsufficientResources;
            }

            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                         "AudioSource::open ion device = %d\n", ionfd);

            m_pHDCPInputBuffers = (struct buffer *) calloc(m_nBuffers, sizeof(*m_pHDCPInputBuffers));
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                        "AudioSource::m_nBuffers = %ld\n",m_nBuffers);
            if (!m_pHDCPInputBuffers)
            {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                         "AudioSource::Could not allocate buffers\n");
              return OMX_ErrorInsufficientResources;
            }
            uAudioIonBufSize = (uBufferSize>AUDIO_ION_BUF_SIZE)?uBufferSize:AUDIO_ION_BUF_SIZE;
            /* Allocate ION Non Secure memory buffers */
            for (unsigned int i = 0; i < (m_nBuffers); i++)
            {
                // hack to test ion buff
                                        //4k - page size
                inputmemfd = allocate_ion_mem(uAudioIonBufSize, &(m_pHDCPInputBuffers[i].handle),ionfd,ION_QSECOM_HEAP_ID, OMX_FALSE);
                if(inputmemfd < 0)
                {
                   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                            "AudioSource::Failed to allocate ion memory\n");
                   return OMX_ErrorInsufficientResources;
                }
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"AudioSource::memfd = %d ", inputmemfd);

                m_pHDCPInputBuffers[i].start = (unsigned char *)
                    mmap(NULL, uAudioIonBufSize, PROT_READ | PROT_WRITE, MAP_SHARED, inputmemfd, 0);

                m_pHDCPInputBuffers[i].length = uAudioIonBufSize;//uBufferSize;
                m_pHDCPInputBuffers[i].fd = inputmemfd;
                m_pHDCPInputBuffers[i].offset = 0;
                m_pHDCPInputBuffers[i].index = i;
                MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_MEDIUM,
                    "AudioSource::allocated buffer(%p) of size = %d, fd = %d errno = %d\n",
                    m_pHDCPInputBuffers[i].start, m_pHDCPInputBuffers[i].length, inputmemfd,errno);

                if ((m_pHDCPInputBuffers[i].start == MAP_FAILED))
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR ,
                                "AudioSource::Could not allocate pmem buffers\n");
                    return OMX_ErrorInsufficientResources;
                }
            } // for loop
          } // Encryption enabled if condition

            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Allocate Encoder output Buffers");
            m_pInputBuffers = MM_New_Array(OMX_BUFFERHEADERTYPE*, m_nBuffers);
            for (int i = 0; i < m_nBuffers; i ++)
            {
                OMX_BUFFERHEADERTYPE * pHeader = (OMX_BUFFERHEADERTYPE*)
                    MM_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
                m_pInputBuffers[i] = pHeader;

                if(m_bEncryptFlag &&
                   m_pHdcpHandle &&
                  (HDCP_STATUS_SUCCESS == m_pHdcpHandle->m_eHdcpSessionStatus))
                  {
                    // HDCP session
                    m_pInputBuffers[i]->pBuffer = (OMX_U8 *) m_pHDCPInputBuffers[i].start;
                    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "AudioSource : HDCP: Buffer Addr = %p Ion Addr = %p",
                                 m_pInputBuffers[i]->pBuffer,(OMX_U8 *) m_pHDCPInputBuffers[i].start);
                  }
                 else // Non HDCP Session
                 {
                   OMX_U8 * pBuffer = (OMX_U8*)MM_Malloc(uBufferSize);
                   m_pInputBuffers[i]->pBuffer = pBuffer;
                   MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH, "AudioSource : NonHDCP Buffer Addr = %p Buffer+4 = %p Size = %d",
                                     m_pInputBuffers[i]->pBuffer,(m_pInputBuffers[i]->pBuffer+LPCM_PRIVATE_STREAM_HEADER),uBufferSize);
                 }
                  m_pInputBuffers[i]->nAllocLen = uAudioIonBufSize;
            }
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "AudioSource::Configure completed");
      }
      else
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "AudioSource::bad params");
          result = OMX_ErrorBadParameter;
      }
#ifdef WFD_DUMP_AUDIO_DATA
      if(m_bDumpWav)
      {
          m_fpWav= fopen("/data/media/audiodump.wav","wb");
          if(!m_fpWav)
          {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                            "AudioSource::audiodump fopen failed");
              m_bDumpWav = OMX_FALSE;
          }
          else
          {
              wav_header hdr;
              hdr.riff_id = ID_RIFF;
              hdr.riff_fmt = ID_WAVE;
              hdr.fmt_id = ID_FMT;
              hdr.fmt_sz = 16;
              hdr.audio_format = 1;
              hdr.num_channels = (uint16_t)m_PcmConfig.channels;
              hdr.sample_rate = 48000;
              hdr.bits_per_sample = 16;
              hdr.byte_rate = (48000 * m_PcmConfig.channels * hdr.bits_per_sample) / 8;
              hdr.block_align = (uint16_t)(( hdr.bits_per_sample * m_PcmConfig.channels ) / 8);
              hdr.data_id = ID_DATA;
              hdr.data_sz = 2147483648LL;
              hdr.riff_sz = hdr.data_sz + 44 - 8;
              fwrite(&hdr,1, sizeof(hdr),m_fpWav);
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                            "AudioSource::Writing  wav header");
          }
      }
#endif
#ifdef AUDIO_DUMP_ENABLE
      if(m_bDumpAudio && (m_eAudioCodingType == OMX_AUDIO_CodingAAC ||
                        m_eAudioCodingType == OMX_AUDIO_CodingAC3))
      {
        m_fpAudio = fopen("/data/media/aacdump.aac","wb");
        if(!m_fpAudio)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                          "AudioSource:aacdump fopen failed");
            m_bDumpAudio = OMX_FALSE;
        }
      }
#endif

      return result;
  }

/*=============================================================================

         FUNCTION:         CreateResources

         DESCRIPTION:
*//**       @brief         This is the WFDMMAudioSource Create Resource
                           function for creating all EVENT.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*=========================================================================*/
  OMX_ERRORTYPE AudioSource::CreateResources()
  {

      OMX_ERRORTYPE result = OMX_ErrorNone;

      /**----------------------------------------------------------------------
      Create the signal Q for the Audio source thread to wait on.
      -------------------------------------------------------------------------
      */
      if ( 0 != MM_SignalQ_Create( &m_AudiosignalQ ) )
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
              "AudioSource:: m_AudiosignalQ creation failed");
          result = OMX_ErrorInsufficientResources;
          return result;
      }

      if ( (0 != MM_Signal_Create( m_AudiosignalQ,
          (void *) &AUDIO_SOURCE_PLAY_EVENT,
          NULL,
          &m_WFDAudioSourcePlaySignal ) ) )
      {
          result = OMX_ErrorInsufficientResources;
          return result;
      }
      if ( (0 != MM_Signal_Create( m_AudiosignalQ,
          (void *) &AUDIO_SOURCE_PAUSE_EVENT,
          NULL,
          &m_WFDAudioSourcePauseSignal ) ) )
      {
          result = OMX_ErrorInsufficientResources;
          return result;
      }
      if ( (0 != MM_Signal_Create( m_AudiosignalQ,
          (void *) &AUDIO_SOURCE_EXIT_EVENT,
          NULL,
          &m_WFDAudioSourceExitSignal ) ) )
      {
          result = OMX_ErrorInsufficientResources;
          return result;
      }

     /**----------------------------------------------------------------------
      Creation of Audio source thread
      -------------------------------------------------------------------------
      */
      if( (0!= MM_Thread_CreateEx(nAudioSourceThreadPriority,
          0,
          AudioSource::SourceThreadEntry,
          this,
          WFD_MM_SOURCE_THREAD_STACK_SIZE,
          "WFDAudioSourceThread",
          &m_AudioSourceThreadHandle)))

      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
              "AudioSource::thread creation failed");
          result = OMX_ErrorInsufficientResources;
          return result;
      }

      if(m_eAudioCodingType == OMX_AUDIO_CodingAAC ||
                    m_eAudioCodingType == OMX_AUDIO_CodingAC3)
      {
          result = ConfigureAudioComponents();
      }
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                  "AudioSource::Configure completed");
      return result;
  }

/*=============================================================================

         FUNCTION:         Start

         DESCRIPTION:
*//**       @brief         This function used for setting the play signal.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None


*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:

*//*=========================================================================*/
  OMX_ERRORTYPE AudioSource::Start()
  {
      OMX_ERRORTYPE result = OMX_ErrorNone;
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "AudioSource::Start");
      // make sure we've been configured
      if (m_nBuffers > 0)
      {
          m_bStarted = OMX_TRUE;
          bRunning = OMX_TRUE;
          bKeepGoing = OMX_TRUE;
          m_eAudioSrcState = WFDMM_AUDIOSOURCE_STATE_PLAYING;
          MM_Signal_Set(m_WFDAudioSourcePlaySignal);
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "AudioSource::Start signal set");
      }
      else
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                        "AudioSource::source has not been configured");
          result = OMX_ErrorUndefined;
      }
      return result;
  }

/*=============================================================================

         FUNCTION:         Finish

         DESCRIPTION:
*//**       @brief         This is the WFDMMAudioSource Finish function
                          used for stopping the audio source
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None


*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:

*//*========================================================================*/
  OMX_ERRORTYPE AudioSource::Finish()
  {
      OMX_ERRORTYPE result = OMX_ErrorNone;
      int timeoutCnt=5000;/*wait counter*/
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "AudioSource:: AudioSource::Finish called");
      m_eAudioSrcState = WFDMM_AUDIOSOURCE_STATE_STOPPING;
      if (m_bStarted == OMX_TRUE)
      {
          bKeepGoing = OMX_FALSE;
          MM_Signal_Set(m_WFDAudioSourcePauseSignal);
          while((m_eAudioSrcState != WFDMM_AUDIOSOURCE_STATE_IDLE)&&timeoutCnt)
          {
              MM_Timer_Sleep(4);/*20sec*/
              timeoutCnt--;
              if(timeoutCnt == 0)
              {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"AudioSource::Finish timed out");
              }
          }

          m_bStarted = OMX_FALSE;
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "AudioSource::m_WFDAudioSourcePauseSignal");
      }
      else
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                        "AudioSource::already stopped");
          result = OMX_ErrorIncorrectStateTransition;
      }
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    " AudioSource::Finish completed");
      return result;
  }

/*==============================================================================

         FUNCTION:         SetFreeBuffer

         DESCRIPTION:
*//**       @brief         This is the WFDMMAudioSource function- used for
                           pushing the buffer into the Queue.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pBufferHdr - Buffer Header


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
  OMX_ERRORTYPE AudioSource::SetFreeBuffer(OMX_BUFFERHEADERTYPE* pBufferHdr)
  {
      OMX_ERRORTYPE result = OMX_ErrorNone;

      if (pBufferHdr != NULL && pBufferHdr->pBuffer != NULL)
      {
          int index = 0;
          // if we have not started then the client is registering buffers
          if (m_bStarted == OMX_FALSE)
          {
              ++m_nFramesRegistered;
          }

          if(m_nDownStreamFrames)
          {
            m_nDownStreamFrames--;
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
               "SetBuffer: Passing audio buffer from MUXER DownStreamBuf = %lu",
                                              (m_nDownStreamFrames));
          }

          if( m_pHdcpHandle && m_bEncryptFlag )
          {
            // find the fd corresponding to the buffer
            for(index = 0; index < m_nBuffers; index++)
            {
              MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM, "SetFreeBuffer Index =%d  Buffer Addr = %p Ion Addr = %p",
                           index, pBufferHdr->pBuffer,(OMX_U8 *) m_pHDCPInputBuffers[index].start);
              if( pBufferHdr->pBuffer == m_pHDCPInputBuffers[index].start )
              {
                break;
              }
             }
            }

          if(m_bStarted == OMX_TRUE)
          {
            if( m_pHdcpHandle && m_bEncryptFlag && index < m_nBuffers )
            {
              m_pInputBuffers[index]->pBuffer =  (OMX_U8 *) m_pHDCPInputBuffers[index].start;
              result = m_pBufferQueue->Push(&(m_pInputBuffers[index]),
              sizeof(OMX_BUFFERHEADERTYPE**));
            }
            else
            {
              result = m_pBufferQueue->Push(&pBufferHdr,
              sizeof(OMX_BUFFERHEADERTYPE**));
            }
          }
      }
      else
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "AudioSource::bad params");
          result = OMX_ErrorBadParameter;
      }
      return result;
  }

/*==============================================================================

         FUNCTION:         GetOutNumBuffers

         DESCRIPTION:
*//**       @brief         This is the WFDMMAudioSource function used for
                           getting the number of buffer.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         nPortIndex  -  Port index
                           nBuffers  - number of buffer


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
  OMX_ERRORTYPE AudioSource::GetOutNumBuffers(
      OMX_U32 nPortIndex, OMX_U32* nBuffers)
  {
      if(nPortIndex != 0)
      {
          return OMX_ErrorBadPortIndex;
      }

      *nBuffers = m_nBuffers;

      return OMX_ErrorNone;
  }
/*==============================================================================

         FUNCTION:         Audio_Handler

         DESCRIPTION:
*//**       @brief         This is the WFDMMAudioSource Function used for
                           Handling the audio buffer
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         audioHandle  - Audio Source Handle


*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:

*//*==========================================================================*/

  OMX_ERRORTYPE AudioSource::Audio_Handler (void * audioHandle)
  {
      AudioSource *pThis;
      OMX_BOOL bDataAvailable = OMX_FALSE;
      OMX_BOOL bInsertSilence = OMX_FALSE;
      pThis = (AudioSource *)audioHandle;

      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,"AudioSource::Audio_Handler");
      if(audioHandle == NULL)
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                        "AudioSource::AudioHandle is NULL");
          return OMX_ErrorBadParameter;
      }
      if (pThis && (pThis->bKeepGoing == OMX_FALSE))
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                        "AudioSource::bKeepGoing is false");

          if(pThis->m_eAudioSrcState == WFDMM_AUDIOSOURCE_STATE_STOPPING ||
                 pThis->m_eAudioSrcState == WFDMM_AUDIOSOURCE_STATE_IDLE)
          {
              return OMX_ErrorNone;
          }

          return OMX_ErrorInvalidState;
      }
      OMX_ERRORTYPE result = OMX_ErrorNone;
      result = pThis->m_pBufferQueue->Pop(&pThis->pBufferHdr,
          sizeof(pThis->pBufferHdr),
          0);

      if(result != OMX_ErrorNone)
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                      "Audio_Handler : Currently MUX is holding all the buffers continue calling pcm_read");
      }

      if( OMX_AUDIO_CodingPCM == pThis->m_eAudioCodingType )
      {
        int index = 0;
        memset(pThis->pBufferHdr->pBuffer,0,LPCM_PRIVATE_STREAM_HEADER);
        //sub_Stream_id
        pThis->pBufferHdr->pBuffer[index] |= 0xA0;
        index += 1;

        //number_of_frame_headers
        pThis->pBufferHdr->pBuffer[index] |= 6;
        index += 1;

        //audio emphasis flag 1
        //audio mute flag     1
        //reserved            1
        //audio frame number  5
        pThis->pBufferHdr->pBuffer[index] = 0;
        index += 1;

        //quantization word length 2

        pThis->pBufferHdr->pBuffer[index] = (uint8)(AUDIO_BITS_PER_SAMPLE == 16 ? 0 :
                                   AUDIO_BITS_PER_SAMPLE == 20 ? 1 :
                                   AUDIO_BITS_PER_SAMPLE == 24 ? 2 :
                                  0) << 6;
        // audio sampling frequency  2
        OMX_U32 sampling_rate = pThis->m_nSamplingFreq;
        OMX_U8 tempSamplingRate = 0;
        tempSamplingRate = (OMX_U8)(((sampling_rate == 48000 ? 2 :
                                    sampling_rate == 44100 ? 1 :
                                   0) & 0x7) << 3);
        pThis->pBufferHdr->pBuffer[index] |= tempSamplingRate;
        //reserved
        // num channels  3
        pThis->pBufferHdr->pBuffer[index] |= (uint8)((2 -1) & 0x7);

        index += 1;

        MM_MSG_PRIO5(MM_GENERAL, MM_PRIO_MEDIUM,"Audio_Handler:: After Filling LPCM Private stream header dump"\
                            "Byte 1 : %x \n" \
                            "Byte 2 : %x \n" \
                            "Byte 3 : %x \n" \
                            "Byte 4 : %x \n" \
                            "index = %d",
                            pThis->pBufferHdr->pBuffer[0],
                            pThis->pBufferHdr->pBuffer[1],
                            pThis->pBufferHdr->pBuffer[2],
                            pThis->pBufferHdr->pBuffer[3],
                            index);
       }
      if (pThis->bKeepGoing == OMX_TRUE )
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                        "Audio_Handler: calling pcm_read");
          if(pThis->m_bFlushPending == OMX_TRUE && OMX_TRUE == pThis->m_bIsProxyOpened)
          {
            OMX_U64 nTime = 0;
            OMX_U64 nPrevTime = 0;
            struct timespec tempTime;
            clock_gettime(CLOCK_MONOTONIC, &tempTime);
            nTime = ((OMX_U64)tempTime.tv_sec * WFD_TIME_NSEC_IN_MSEC)
                             + ((OMX_U64)tempTime.tv_nsec / WFD_TIME_NSEC_IN_USEC);
            while(pThis->bKeepGoing)
            {
              if(0 != pcm_read(pThis->m_pPcmHandle, pThis->m_pAudioDataBfr,
                          (unsigned int)pThis->m_nAudioBufsize))
              {
                pThis->m_bFlushPending = OMX_FALSE;
                pThis->m_nBaseTime = nTime;
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "PCRM ERROR while FLushing old frames");
                break;
              }
              nPrevTime = nTime;
              clock_gettime(CLOCK_MONOTONIC, &tempTime);
              nTime = ((OMX_U64)tempTime.tv_sec * WFD_TIME_NSEC_IN_MSEC) +
                                            ((OMX_U64)tempTime.tv_nsec / WFD_TIME_NSEC_IN_USEC);
              if(nTime - nPrevTime >= pThis->m_nFrameTimeUnit- (1 *WFD_TIME_USEC_IN_MSEC))
              {
                pThis->m_bFlushPending = OMX_FALSE;
                bDataAvailable = OMX_TRUE;
                pThis->m_nBaseTime = nTime;
                break;
              }
              else
              {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "FLushing old frames");
              }
            }
          }

         if( OMX_TRUE == pThis->m_bIsProxyOpened &&
              OMX_FALSE == pThis->m_bIsProxyAvailabile)
         {
           // Got a request to close proxy device in middle of WFD session.
           // This could be scanerio like BT A2DP concorrency where BT also
           // will read data from proxy port and WFD has to close proxy device.
           pcm_close(pThis->m_pPcmHandle);
           pThis->m_pPcmHandle = NULL;
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                     "Audio_Handler: Closing proxy device in middle of session");
           pThis->m_bIsProxyOpened = OMX_FALSE;
         }

         if(OMX_FALSE == pThis->m_bIsProxyOpened)
         {
           OMX_U64 nTimeTaken_ns = 0;
           OMX_U64 nTimeBefore = 0;
           OMX_U64 nTimeAfter = 0;

           struct timespec tempTimeGetparam;
           clock_gettime(CLOCK_MONOTONIC, &tempTimeGetparam);
           nTimeBefore = ((OMX_U64)tempTimeGetparam.tv_sec *
              WFD_TIME_NSEC_IN_SEC)+ ((OMX_U64)tempTimeGetparam.tv_nsec);
           if(OMX_TRUE == pThis->m_bIsProxyAvailabile &&
              !IS_PROXY_IN_USE())
           {
             OMX_ERRORTYPE eRetval;
             // got proxy device open request in middle of session
             // open device and start reading audio from proxy port
             MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                "Audio_Handler: Opening proxy device in middle of session");
              //Check if proxy device is available to be opened
             eRetval = pThis->ConfigureAudioProxyDevice();
             if(OMX_ErrorNone != eRetval)
             {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "Audio_Handler: Failed in Opening Proxy");
               bInsertSilence = OMX_TRUE;
             }
             else
             {
               if(pThis->m_pEventHandlerFn)
               {
                 pThis->m_pEventHandlerFn(pThis->m_appData, pThis->m_nModuleId,
                                         WFDMMSRC_AUDIO_PROXY_OPEN_DONE, OMX_ErrorNone, 0);
               }
             }
           }
           else
           {
              bInsertSilence = OMX_TRUE;
           }
           if(OMX_TRUE == bInsertSilence)
           {
             // Currently proxy port is not open, Keep on inseriting silence
             // frames until we dont get request to open proxy port again.
             MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
               "Proxy device closed: inserting silence ");
             clock_gettime(CLOCK_MONOTONIC, &tempTimeGetparam);
             nTimeAfter = ((OMX_U64)tempTimeGetparam.tv_sec *
              WFD_TIME_NSEC_IN_SEC)+ ((OMX_U64)tempTimeGetparam.tv_nsec);
             nTimeTaken_ns = nTimeAfter - nTimeBefore;
             if(pThis->m_eAudioCodingType == OMX_AUDIO_CodingPCM)
             {
               memset(pThis->pBufferHdr->pBuffer+LPCM_PRIVATE_STREAM_HEADER,
                  0, pThis->m_nAudioBufsize);
             }
             else
             {
               memset(pThis->m_pAudioDataBfr,0, pThis->m_nAudioBufsize);
             }
             pThis->AudioSource_sleep_for_audioframe_interval(nTimeTaken_ns);
           }
         }
         else
         {
           if (pThis->m_pPcmHandle != NULL && bDataAvailable == OMX_FALSE &&
              0 != pcm_read(pThis->m_pPcmHandle, pThis->m_pAudioDataBfr,
                          (unsigned int ) pThis->m_nAudioBufsize))
           {
             if(result == OMX_ErrorNone)
             {
               if(pThis->m_eAudioCodingType == OMX_AUDIO_CodingPCM)
               {
                 memset(pThis->pBufferHdr->pBuffer+LPCM_PRIVATE_STREAM_HEADER,
                   0, pThis->m_nAudioBufsize);
               }
               else
               {
                 memset(pThis->m_pAudioDataBfr,0, pThis->m_nAudioBufsize);
               }
             }
             // If pcm_read fails, re-initializing pcm driver to
             // recover from the underrun error. Insert silence
             // for this itteration, the next read will be successful.
             MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"Recovering from pcm_read: error");
             pThis->AudioSource_sleep_for_audioframe_interval(0);

             if(pThis->m_pPcmHandle)
             {
               OMX_ERRORTYPE Retval;
               pcm_close(pThis->m_pPcmHandle);
               pThis->m_pPcmHandle = NULL;
               pThis->m_bIsProxyOpened = OMX_FALSE;
               Retval = pThis->ConfigureAudioProxyDevice();
               if(OMX_ErrorNone != Retval)
               {
                  return Retval;
               }
             }
             else
             {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                "AudioSource::SourceThread:PCM read fail. Recovery done by HAL");
             }
             //WHen PCM reopen happens above steps are not
             // deterministic in time taken. Therefore it
             //is only possible to read actual timestamp
             OMX_U64 nTime = 0;
             OMX_U64 nPrevTime = 0;
             struct timespec tempTime;
             clock_gettime(CLOCK_MONOTONIC, &tempTime);
             nTime = ((OMX_U64)tempTime.tv_sec * WFD_TIME_NSEC_IN_MSEC)
                             + ((OMX_U64)tempTime.tv_nsec / WFD_TIME_NSEC_IN_USEC);
             pThis->m_nBaseTime = nTime;
             pThis->m_bBaseTimeTaken = OMX_FALSE;
             pThis->m_nTimeErrAdjustCntr = 0;
           }
         }
         if(result == OMX_ErrorNone)
         {
            if(!pThis->m_bBaseTimeTaken)
            {
              pThis->m_nCurrTime = pThis->m_nBaseTime;
              pThis->m_bBaseTimeTaken = OMX_TRUE;
            }
            else
            {
              pThis->m_nCurrTime += pThis->m_nFrameTimeUnit;
              if(pThis->m_nTimeErrAdjustCnt)
              {
                 pThis->m_nTimeErrAdjustCntr++;
                 if(pThis->m_nTimeErrAdjustCntr >= pThis->m_nTimeErrAdjustCnt)
                 {
                   pThis->m_nCurrTime += pThis->m_nTimeErrAdjust;
                   pThis->m_nTimeErrAdjustCntr = 0;
                 }
              }
            }
  #ifdef WFD_DUMP_AUDIO_DATA
            if(pThis->m_bDumpWav)
            {
              if (pThis->m_fpWav)
              {
                  fwrite(pThis->m_pAudioDataBfr, 1, pThis->m_nAudioBufsize, pThis->m_fpWav);
              }
            }
  #endif

            if(pThis->m_eAudioCodingType == OMX_AUDIO_CodingPCM)
            {

                if(OMX_FALSE == bInsertSilence)
                {
                  /**---------------------------------------------------------------
                  Swap bytes in LPCM and copy. We need LPCM in network byte order
                  ------------------------------------------------------------------
                  */
                  OMX_U32  nCopyCount = pThis->m_nAudioBufsize;

                  OMX_U16 *pDst       = (OMX_U16*)(pThis->pBufferHdr->pBuffer+LPCM_PRIVATE_STREAM_HEADER);
                  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"Audio_Handler:: Destination addr = %p",pDst);
                  OMX_U16 *pSrc       = (OMX_U16*)pThis->m_pAudioDataBfr;
                  OMX_U16  nVal       = 0;

                  if(nCopyCount % 2)
                  {
                      /**-----------------------------------------------------------
                      odd number of bytes is not acceptable here.
                      --------------------------------------------------------------
                      */
                      nCopyCount--;
                  }
                  for(unsigned int count = 0; count < (nCopyCount >> 1); count++)
                  {
                      nVal = *pSrc++;
                      int temp = ((nVal >> 8) | (nVal << 8));
                      *pDst++ = (uint16)temp;
                  }
                }
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Audio_Handler:Read %lu PCM bytes"\
                                                       " from Driver", pThis->m_nAudioBufsize);
                pThis->pBufferHdr->nFilledLen = pThis->m_nAudioBufsize + LPCM_PRIVATE_STREAM_HEADER;
                pThis->pBufferHdr->nFlags = OMX_AUDIO_PCM_FRAME;
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "Audio_Handler: LPCM Filled length after swap = %lu "\
                                     , pThis->pBufferHdr->nFilledLen);
            }
            else if(pThis->m_eAudioCodingType == OMX_AUDIO_CodingAAC ||
                    pThis->m_eAudioCodingType == OMX_AUDIO_CodingAC3 )
            {
                unsigned long lTime;
                MM_Time_GetTime(&lTime);
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                             "Audio_Handler:Start Audio encoding @ %lu",
                              lTime);

                pThis->pBufferHdr->nFilledLen =  0;
                pThis->pBufferHdr->nFlags = 0x00;

                result = pThis->m_pAudioEncoder->Encode(pThis->m_pAudioDataBfr,
                                                        pThis->m_nAudioBufsize,
                                                 pThis->pBufferHdr->pBuffer,
                                                 pThis->pBufferHdr->nAllocLen,
                                                 (uint32*)&pThis->pBufferHdr->nFilledLen);

                if(result != OMX_ErrorNone)
                {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                   "Audio_Handler: Audio Encoder Error");
                  pThis->m_pBufferQueue->Push(&pThis->pBufferHdr,
                      sizeof(pThis->pBufferHdr));
                  return OMX_ErrorUndefined;
                }
                MM_Time_GetTime(&lTime);
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "Audio_Handler\
                  : Complete Audio encoding @ = %lu",lTime);
  #ifdef AUDIO_DUMP_ENABLE
                if(pThis->m_bDumpAudio)
                {
                    if (pThis->m_fpAudio)
                    {
                        fwrite(pThis->pBufferHdr->pBuffer, 1,
                           pThis->pBufferHdr->nFilledLen, pThis->m_fpAudio);
                    }
                }
  #endif
            }//if(pThis->m_eAudioCodingType == OMX_AUDIO_CodingAAC)
        }//if(result == OMXErrorNone)
      }//if (pThis->bKeepGoing == OMX_TRUE )

/* Encrypt Audio Elementary Stream if supported in config file */
// 1. Call HDCP Encrypt API
// 2. Append Private Data as Extra Data to the end of Elementary Stream Buffer


      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "Audio_Handler: "\
                 "bEncrypt Audio = %d",pThis->m_bEncryptFlag);

      MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
          "Audio_Handler::HDCP Session Status(%d) Filled Length = %lu",
          pThis->m_pHdcpHandle->m_eHdcpSessionStatus, pThis->pBufferHdr->nFilledLen);

      if( pThis->m_pHdcpHandle && pThis->m_bEncryptFlag && result == OMX_ErrorNone &&
          pThis->m_pHdcpHandle->m_eHdcpSessionStatus == HDCP_STATUS_SUCCESS )
      {
        int index = 0;
        unsigned char ucAudioPESPvtData[PES_PVT_DATA_LEN] = {0};
        // find the fd corresponding to the buffer
        for(index = 0; index < pThis->m_nBuffers; index++)
        {
          MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM, "Audio_handler :Index =%d  Buffer Addr = %p Ion Addr = %p",
                       index, pThis->pBufferHdr->pBuffer,(OMX_U8 *) pThis->m_pHDCPInputBuffers[index].start);
          if( pThis->pBufferHdr->pBuffer == pThis->m_pHDCPInputBuffers[index].start )
          {
            break;
          }
        }
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "Audio_Handler: "\
                      "Fd = %d Index = %d",pThis->m_pHDCPInputBuffers[index].fd,index);
        // H/w lib
        unsigned long ulStatus = pThis->m_pHdcpHandle->WFD_HdcpDataEncrypt(STREAM_AUDIO ,
                                                    (unsigned char*)ucAudioPESPvtData,
                                                    (OMX_U8 *)(int64)pThis->m_pHDCPInputBuffers[index].fd, // clear data
                                                    (OMX_U8 *)(int64)pThis->m_pHDCPInputBuffers[index].fd, // Encrypted data data
                                                    pThis->pBufferHdr->nFilledLen);
          if( ulStatus != 0)
          {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"Error in HDCP Encryption = %lu ",ulStatus);
            if( ulStatus == QC_HDCP_CP_ErrorCipherDisabled)
            {
              if(false == pThis->m_pHdcpHandle->proceedWithHDCP())
              {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "Cipher enablement wait timed out");
                return OMX_ErrorInsufficientResources;
              }
              else
              {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                    "Cipher is still not enabled");
                //Do not report error yet
                return OMX_ErrorNone;
              }
            }
            return OMX_ErrorBadParameter;
          }
        for ( int idx = 0; idx < PES_PVT_DATA_LEN; idx++)
        {
          MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
                      "Audio_handler : Encrypt PayloadLen[%lu] PES_PVTData[%d]:%x",
                      pThis->pBufferHdr->nFilledLen,
                      idx,
                      ucAudioPESPvtData[idx]);
        }
        // Fill PESPvtData at end of the encrypted buffer, as an extra data
        pThis->FillHdcpAudioExtraData( pThis->pBufferHdr, ucAudioPESPvtData, 0);
      }/* Audio Encryption Code Ends */
      if (result == OMX_ErrorNone)
      {
          if (pThis->pBufferHdr != NULL )
          {
              if (pThis->pBufferHdr->nFilledLen)
              {
                  OMX_TICKS lTime;

                  struct timespec tempTime;
                  clock_gettime(CLOCK_MONOTONIC, &tempTime);
                  lTime =(OMX_TICKS)(((unsigned long long)tempTime.tv_sec * WFD_TIME_NSEC_IN_MSEC)
                                   + ((unsigned long long)tempTime.tv_nsec / WFD_TIME_NSEC_IN_USEC));
                  OMX_TICKS nTimeStamp = lTime;

                  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "Audio_Handler:" \
                                              "  AUDIO TIMESTAMP IS %lld ms",nTimeStamp);
                  //If there is a drift in timestamp correct it from time
                  //to time. ALl these because audio driver can't give
                  //timestamp at any cost

                  OMX_U64 nDriftCutoff = 0;

                  if(pThis->m_eAudioCodingType == OMX_AUDIO_CodingAAC ||
                     pThis->m_eAudioCodingType == OMX_AUDIO_CodingAC3)
                  {
                      nDriftCutoff = 5 * pThis->m_nFrameTimeUnit;
                  }
                  else
                  {
                      nDriftCutoff = 10 * pThis->m_nFrameTimeUnit;
                  }
                  if((OMX_U64)nTimeStamp > (OMX_U64)pThis->m_nCurrTime &&
                     (OMX_U64)nTimeStamp - (OMX_U64)pThis->m_nCurrTime > nDriftCutoff)
                  {
                     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                                  "Audio_Handler: AUDIO TIMESTAMP IS %lld ms",nTimeStamp);
                     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                                  "Audio_Handler: AUDIO TIMESTAMP ACTUAL IS %lld ms",
                                  pThis->m_nCurrTime);
                     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                  "Audio_Handler: AUDIO TIMESTAMP drift exceeds threshold");
                     int nTotalDrift = (int)(((OMX_U64)nTimeStamp - pThis->m_nCurrTime));

                     //Always Add multiple of frame units
                     pThis->m_nCurrTime +=
                                  ((nTotalDrift / pThis->m_nFrameTimeUnit) * pThis->m_nFrameTimeUnit);
                     if(pThis->m_nTimeErrAdjustCnt && pThis->m_nFrameTimeUnit)
                     {
                         //When rounding errors are expected see if correction needed.
                         int64 numFrames = nTotalDrift/pThis->m_nFrameTimeUnit;
                         if(pThis->m_nTimeErrAdjustCntr + numFrames >= pThis->m_nTimeErrAdjustCnt)
                         {
                             pThis->m_nCurrTime += pThis->m_nTimeErrAdjust;
                         }
                         pThis->m_nTimeErrAdjustCntr =
                             (pThis->m_nTimeErrAdjustCntr + numFrames) % pThis->m_nTimeErrAdjustCnt;
                     }
                     pThis->m_bFlushPending = OMX_TRUE;
                     pThis->m_nDropFrames = 0;
                  }
                  else if(pThis->m_nDropFrames)
                  {
                     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                     "Frame dropped to adjust audio time and realtime progress mismatch");
                     pThis->m_nDropFrames--;
                     pThis->m_nCurrTime -= pThis->m_nFrameTimeUnit;
                     pThis->pBufferHdr->nFilledLen = 0;

                     if((OMX_U64)pThis->m_nCurrTime > (OMX_U64)nTimeStamp)
                     {
                         pThis->m_nDropFrames = 1;
                     }
                  }
                  else if((OMX_U64)pThis->m_nCurrTime > (OMX_U64)nTimeStamp)
                  {
                     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                                  "Audio_Handler: AUDIO TIMESTAMP IS %lld ms",nTimeStamp);
                     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                                  "Audio_Handler: AUDIO TIMESTAMP ACTUAL IS %lld ms",
                                  pThis->m_nCurrTime);
                     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                  "Audio_Handler: AUDIO TIMESTAMP drift exceeds threshold. Drop");
                     pThis->pBufferHdr->nFilledLen = 0;
                     pThis->m_nCurrTime -= pThis->m_nFrameTimeUnit;

                     //Now we are ahead of realtime and may need to drop a couple more
                     //packets. This may happen if we get more samples than expected from DSP
                     //pThis->m_nDropFrames = 1;

                  }

                  pThis->pBufferHdr->nTimeStamp = pThis->m_nCurrTime;
                  pThis->pBufferHdr->nOffset = 0;
                  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "Audio_Handler: "\
                              "Passing audio data to MUXER @ nTimeStamp = %lld",
                                              (pThis->pBufferHdr->nTimeStamp));
                  pThis->pBufferHdr->nTimeStamp += pThis->m_nAudioDelay;
                  pThis->m_nDownStreamFrames++;
                  pThis->m_pFrameDeliverFn(pThis->pBufferHdr, pThis->m_appData);
              }
              else
              {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                   "Audio_Handler: Audio Encode returned 0 bytes, pushing buffer");
                  pThis->m_pBufferQueue->Push(&pThis->pBufferHdr,
                      sizeof(pThis->pBufferHdr));
              }
          }
          else
          {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                         "Audio_Handler: Buffer is null");
          }
      }
      return OMX_ErrorNone;
  }

/*==============================================================================

         FUNCTION:         AudioSource_sleep_for_audioframe_interval

         DESCRIPTION:
*//**       @brief         This function is used to add sleep equal to one audio frame interval in ns
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pAudioSrcHandle  - Audio Source Handle


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/

void AudioSource::AudioSource_sleep_for_audioframe_interval(unsigned long drift)
{
  unsigned long nSleepTime=0;
  int nResult = 1;
  int periodSize;
  if(OMX_AUDIO_CodingPCM == m_eAudioCodingType)
  {
    periodSize = PCM_HW_PARAM_PERIOD_SIZE;
  }
  else
  {
    periodSize = m_nAudioParamPeriodSize;
  }
  nSleepTime =  (periodSize * WFD_TIME_NSEC_IN_MSEC/
                 m_nSamplingFreq);// sleep in ms
  nSleepTime *= WFD_TIME_NSEC_IN_USEC; //Sleep time in ns
  if(nSleepTime>drift)
  {
    nSleepTime -= drift;
    struct timespec t;
    t.tv_sec = nSleepTime / WFD_TIME_NSEC_IN_SEC;
    t.tv_nsec = nSleepTime % WFD_TIME_NSEC_IN_SEC;
    nResult = nanosleep(&t, NULL);
    if(nResult == 0)
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
            "AudioSource_sleep_for_audioframe_interval = %lu ns",nSleepTime);
    }
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
            "AudioSource_sleep_for_audioframe_interval drift time is more than sleep duration");
  }
}

/*==============================================================================

         FUNCTION:         ConfigureAudioProxyDevice

         DESCRIPTION:
*//**       @brief         This is the WFDMMAudioSource Function used for
                           opening and configuring audio proxy port
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pAudioSrcHandle  - Audio Source Handle


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/

OMX_ERRORTYPE AudioSource::ConfigureAudioProxyDevice()
{
  OMX_ERRORTYPE Retval = OMX_ErrorNone;

  //Notify AudioFlinger that PROXY out is multichannel and can handle unmixed
  //multichannel output
  android::AudioParameter param = android::AudioParameter();
  param.addInt(android::String8("wfd_channel_cap"), m_PcmConfig.channels);
  android::AudioSystem::setParameters(0, param.toString());
  m_PcmConfig.format = PCM_FORMAT_S16_LE;
  m_PcmConfig.period_count = 32;
  if(m_eAudioCodingType == OMX_AUDIO_CodingPCM)
  {
    m_PcmConfig.period_size = PCM_HW_PARAM_PERIOD_SIZE;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                  "AudioSource::Audio codec is LPCM");
  }
  else
  {
    m_PcmConfig.period_size = m_nAudioParamPeriodSize;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "AudioSource::Audio codec is AAC");
  }
  unsigned int flags = PCM_IN;
  m_pPcmHandle = pcm_open(0 /*card */, m_nAudiodevice,flags, &m_PcmConfig);

  if(m_pPcmHandle == NULL)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                 "AudioSource::SourceThread: pcm_open failed\n");
    return OMX_ErrorDynamicResourcesUnavailable;
  }

  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
             "AudioSource:: channels = %d freq= %d",
             m_PcmConfig.channels,
             m_PcmConfig.rate);


  m_nAudioBufsize = m_PcmConfig.period_size * 2 * m_PcmConfig.channels;

  m_bIsProxyOpened = OMX_TRUE;
  return Retval;
}


/*==============================================================================

         FUNCTION:         ConfigureAudioComponents

         DESCRIPTION:
*//**       @brief         This is the WFDMMAudioSource function used for
                           configuring the audio track.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*========================================================================*/
  OMX_ERRORTYPE AudioSource::ConfigureAudioComponents()
  {
      OMX_ERRORTYPE result = OMX_ErrorNone;

      m_pAudioEncoder = WFDMMSourceAudioEncode::create(m_eAudioCodingType);

      if(!m_pAudioEncoder)
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Creating Audio encode failed");
          return OMX_ErrorInsufficientResources;
      }


      if(m_pAudioEncoder)
      {
          m_nAudioParamPeriodSize = m_pAudioEncoder->GetInputFrameSize();
      }

      audioConfigType sCfg;

      sCfg.eAudioType       = m_eAudioCodingType;
      sCfg.nBitsPerSample   = 16;
      sCfg.nChannels        = m_PcmConfig.channels;
      sCfg.nSampleRate      = m_PcmConfig.rate;
      sCfg.nBitrate         = m_nBitRate;
      sCfg.nSamplesPerFrame = m_nAudioParamPeriodSize;

      if(m_pAudioEncoder)
      {
          return m_pAudioEncoder->Configure(&sCfg);
      }

      return OMX_ErrorNone;
  }
/*==============================================================================

         FUNCTION:         SourceThreadEntry

         DESCRIPTION:
*//**       @brief         This is the WFDMMAudioSource Thread entry function
                           used for starting the Source thread.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pThreadData  -  Thread Data


*//*     RETURN VALUE:
*//**       @return
                           result

@par     SIDE EFFECTS:

*//*==========================================================================*/
  int AudioSource::SourceThreadEntry(OMX_PTR pThreadData)
  {
      OMX_ERRORTYPE result = OMX_ErrorBadParameter;
      if (pThreadData)
      {
          result = ((AudioSource*) pThreadData)->SourceThread();
      }
      return result;
  }



/*==============================================================================

         FUNCTION:         SourceThread

         DESCRIPTION:
*//**       @brief         This is the WFDMMSource class function used for
                           running the source thread and maintaing its state.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None


*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
  OMX_ERRORTYPE AudioSource::SourceThread()
  {
      OMX_ERRORTYPE result = OMX_ErrorNone;
      OMX_U32 *pEvent = NULL;
      bRunning = OMX_TRUE;
      int tid = androidGetTid();
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "WFDD: AudioSource  priority b4 %d ", androidGetThreadPriority(tid));
      androidSetThreadPriority(0,WFD_MM_AUDIO_SOURCE_THREAD_PRIORITY);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,  "WFDD:AudioSource  priority after%d ", androidGetThreadPriority(tid));

      for ( int i =0; bRunning == OMX_TRUE; i++ )
      {
          if ( 0 == MM_SignalQ_Wait( m_AudiosignalQ, (void **) &pEvent ) )
          {
              switch ( *pEvent )
              {
              case AUDIO_SOURCE_PLAY_EVENT:
                  {
                      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                          "AudioSource:: AUDIO_SOURCE_PLAY_EVENT");
                      if(m_eAudioSrcState != WFDMM_AUDIOSOURCE_STATE_PLAYING)
                      {
                         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                           "AudioSource:: AUDIO_SOURCE_PLAY_EVENT when not in PLAYING");
                      }
                      m_eAudioSrcState = WFDMM_AUDIOSOURCE_STATE_PLAY;
                      OMX_BUFFERHEADERTYPE** ppAudBuffers = NULL;
                      ppAudBuffers = GetBuffers();
                      m_ProxyMonitorThread = NULL;
                      m_bIsProxyOpened  = OMX_FALSE;
                      m_bIsProxyAvailabile = OMX_FALSE;
                      m_bFlushPending = OMX_TRUE;
                      m_bTimeReset = OMX_FALSE;
                      m_nBaseTime = 0;
                      m_nCurrTime = 0;
                      m_bBaseTimeTaken = OMX_FALSE;
                      m_nDownStreamFrames = 0;
                      for (int i = 0; i < m_nBuffers; i++)
                      {
                          result = SetFreeBuffer(ppAudBuffers[i]);
                          if (result != OMX_ErrorNone)
                          {
                              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                  "WFDMMAudioSource::Play Error in passing\
                                   buffers to audio source");

                              if(m_pEventHandlerFn)
                              {
                                  m_pEventHandlerFn(m_appData, m_nModuleId,
                                                    WFDMMSRC_ERROR, OMX_ErrorInsufficientResources,0);
                              }

                              break;
                          }
                      }

                      //Before starting the capture session, check one for
                      //proxy availability. During runtime the proxy availability will be
                      //checked in another thread
                      m_bIsProxyAvailabile = GetProxyAvailability();

                      if(m_bIsProxyAvailabile == OMX_TRUE)
                      {
                          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "AudioSource Proxy Available Open");
                      }

                      //Start the thread for querying proxy status
                      int ret = MM_Thread_CreateEx(0,0,AudioSource::ProxyMonitorEntry,this,
                                                   WFD_MM_SOURCE_THREAD_STACK_SIZE,
                                                  "WFDAudioProxyMonitorThread",
                                                   &m_ProxyMonitorThread);
                      if(ret != 0)
                      {
                          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                      "AudioSource Failed to create ProxyMonitorThread");
                          if(m_pEventHandlerFn)
                          {
                              m_pEventHandlerFn(m_appData, m_nModuleId,
                                                WFDMMSRC_ERROR, OMX_ErrorInsufficientResources, 0);
                          }
                          break;
                      }

                      if(OMX_TRUE == m_bIsProxyAvailabile)
                      {
                          OMX_ERRORTYPE eErr = ConfigureAudioProxyDevice();
                          if(eErr != OMX_ErrorNone)
                          {
                              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                      "AudioSource Failed to Open Proxy");
                              if(m_pEventHandlerFn)
                              {
                                  m_pEventHandlerFn(m_appData, m_nModuleId,
                                                    WFDMMSRC_ERROR,
                                                    OMX_ErrorInsufficientResources,
                                                    0);
                              }
                              break;
                          }
                          if(m_pEventHandlerFn)
                          {
                              m_pEventHandlerFn(m_appData, m_nModuleId,
                                                WFDMMSRC_AUDIO_PROXY_OPEN_DONE,
                                                OMX_ErrorNone, 0);
                          }
                      }
                      m_pAudioDataBfr =  (OMX_U8*)MM_Malloc(m_nAudioBufsize);
                      if (!m_pAudioDataBfr)
                      {
                          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                             "AudioSource::SourceThread: could not allocate%ld bytes"
                             ,m_nAudioBufsize);
                          if(m_pEventHandlerFn)
                          {
                              m_pEventHandlerFn(m_appData, m_nModuleId,
                                                WFDMMSRC_ERROR, OMX_ErrorInsufficientResources , 0);
                          }
                          break;
                      }

                      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                          "AudioSource::SourceThread: allocated %ld bytes\n",
                           m_nAudioBufsize);

                      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                                  "AudioSource::AUDIO_SOURCE_PLAY_EVENT bReleaseTimer = false");

                      OMX_ERRORTYPE eError = OMX_ErrorNone;

                      while(m_eAudioSrcState == WFDMM_AUDIOSOURCE_STATE_PLAY
                            && eError == OMX_ErrorNone)
                      {
                          eError = Audio_Handler(this);
                      }
                      if(eError != OMX_ErrorNone)
                      {
                         if(m_pEventHandlerFn)
                         {
                             m_pEventHandlerFn(m_appData, m_nModuleId,
                                               WFDMMSRC_ERROR,
                                               OMX_ErrorResourcesLost, 0);
                         }
                      }

                  }
                  break;
              case AUDIO_SOURCE_PAUSE_EVENT:
                  {
                      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                  "AudioSource::AUDIO_SOURCE_PAUSE_EVENT bReleaseTimer = TRUE");

                      if(m_eAudioSrcState != WFDMM_AUDIOSOURCE_STATE_STOPPING)
                      {
                         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                           "AudioSource:: DEBUG:AUDIO_SOURCE_PAUSE_EVENT when not in STOPPING");
                      }
                      m_eAudioSrcState = WFDMM_AUDIOSOURCE_STATE_STOPPING;

                      OMX_ERRORTYPE result = OMX_ErrorNone;
                      OMX_S32 nSize = 0;

                      //Ideally checking Queue depth at this point is sufficient to determine
                      //all buffers are returned or not. But Audio Handler has some return
                      //statements which can return on error without queuing buffer back.
                      //It needs revisit. Checking downstream buffers is a more robust way
                      //to check downstream buffers at this point.
                      while(m_nDownStreamFrames)
                      {
                        MM_Timer_Sleep(1);
                        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                              "AudioSource:: AUDIO_SOURCE_PAUSE_EVENT pending buf is %lu ",
                              m_nDownStreamFrames);
                      }

                      nSize = m_pBufferQueue->GetSize();

                      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                              "AudioSource:: AUDIO_SOURCE_PAUSE_EVENT queue size is %ld ",
                              nSize);

                      if(m_pPcmHandle && (OMX_TRUE == m_bIsProxyOpened))
                      {
                        pcm_close(m_pPcmHandle);
                        m_pPcmHandle = NULL;
                      }
                      m_bIsProxyOpened = OMX_FALSE;
                      if(m_ProxyMonitorThread)
                      {
                          int exitCode = 0;
                          MM_Thread_Join(m_ProxyMonitorThread, &exitCode);
                          MM_Thread_Release(m_ProxyMonitorThread);
                          m_ProxyMonitorThread = NULL;
                      }

                      for (OMX_S32 i = 0; i < nSize; i++)
                      {
                          result = m_pBufferQueue->Pop(&pBufferHdr, sizeof(OMX_BUFFERHEADERTYPE**),0);
                      }

                      m_eAudioSrcState = WFDMM_AUDIOSOURCE_STATE_IDLE;
                  }
                  break;
              case AUDIO_SOURCE_EXIT_EVENT :
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                      "AudioSource:: AUDIO_SOURCE_EXIT_EVENT");
                  MM_Thread_Exit(m_AudioSourceThreadHandle, 0);
                  bRunning = OMX_FALSE;
                  break;
              }//switch
          }//if
      }//for
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                   "AudioSource::Source thread is exiting...");
      return result;
  }
/*==============================================================================

         FUNCTION:         GetBuffers

         DESCRIPTION:
*//**       @brief         This is the WFDMMSource Function used for
                           getting the input buffer.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None


*//*     RETURN VALUE:
*//**       @return
                           OMX_BUFFERHEADERTYPE

@par     SIDE EFFECTS:

*//*==========================================================================*/
  OMX_BUFFERHEADERTYPE** AudioSource::GetBuffers()
  {
    return m_pInputBuffers;
  }


  /*==============================================================================
           FUNCTION:         FillHdcpAudioExtraData

           DESCRIPTION:
  *//**       @brief         This funtion adds the extra data in case of
                             encryption enabled
  *//**

  @par     DEPENDENCIES:
                             None
  *//*
           PARAMETERS:
  *//**       @param         OMX_BUFFERHEADERTYPE pointer.
  *//**       @param         HDCP private data
  *//**       @param         Port Index.

  *//*     RETURN VALUE:
  *//**       @return
                             OMX_ERRORTYPE

  @par     SIDE EFFECTS:

  *//*==========================================================================*/

  OMX_ERRORTYPE  AudioSource::FillHdcpAudioExtraData(
                            OMX_BUFFERHEADERTYPE *pBufHdr,
                            OMX_U8* pucPESPvtHeader,
                            OMX_U32 nPortIndex)
  {
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    uint64 ulAddr;
    OMX_OTHER_EXTRADATATYPE *pHdcpCpExtraData;

    if( (NULL != pBufHdr ) && (NULL != pucPESPvtHeader))
    {
      MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                  "AudioSource::pBufHdr->pBuffer[%p] FilledLen[%lu]",
                  pBufHdr->pBuffer,
                  pBufHdr->nFilledLen);
      /* Skip encoded frame payload length filled by Audio driver */
      ulAddr = (uint64) ( pBufHdr->pBuffer) +  pBufHdr->nFilledLen;
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "AudioSource::ulAddr[%llu]", ulAddr);

      /* Aligned address to DWORD boundary */
      ulAddr = (ulAddr + 0x3) & (~0x3);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "AudioSource::Aligned ulAddr[%llu]", ulAddr);
      pHdcpCpExtraData = (OMX_OTHER_EXTRADATATYPE *)ulAddr;
      /* Update pBufHdr flag, to indicate that it carry extra data */
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "AudioSource::pHdcpCpExtraData[%p]", pHdcpCpExtraData);

      FLAG_SET(pBufHdr,OMX_BUFFERFLAG_EXTRADATA);
      /* Extra Data size = ExtraDataType*/
      pHdcpCpExtraData->nSize = sizeof(OMX_OTHER_EXTRADATATYPE) + sizeof(OMX_U8)* PES_PVT_DATA_LEN -4;
      pHdcpCpExtraData->nVersion = pBufHdr->nVersion;
      pHdcpCpExtraData->nPortIndex = nPortIndex;
      pHdcpCpExtraData->nDataSize = PES_PVT_DATA_LEN;
      MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM,
        "AudioSource::size[%lu] PortIndex[%lu] nDataSize[%lu]",
        pHdcpCpExtraData->nSize,
        pHdcpCpExtraData->nPortIndex,pHdcpCpExtraData->nDataSize );
      pHdcpCpExtraData->eType = (OMX_EXTRADATATYPE)QOMX_ExtraDataHDCPEncryptionInfo;
      /* Fill PES_PVT_DATA into data*/
      memcpy(pHdcpCpExtraData->data,pucPESPvtHeader, PES_PVT_DATA_LEN );
      /* Append OMX_ExtraDataNone */
      ulAddr += pHdcpCpExtraData->nSize;
      pHdcpCpExtraData = (OMX_OTHER_EXTRADATATYPE *)ulAddr;
      pHdcpCpExtraData->nSize = sizeof(OMX_OTHER_EXTRADATATYPE);
      pHdcpCpExtraData->nVersion = pBufHdr->nVersion;
      pHdcpCpExtraData->nPortIndex = nPortIndex;
      pHdcpCpExtraData->eType = OMX_ExtraDataNone;
      pHdcpCpExtraData->nDataSize = 0;
    }
    else
    {
      eError = OMX_ErrorBadParameter;
    }
    return eError;
  }

  /*==============================================================================
           FUNCTION:         ProxyMonitorEntry

           DESCRIPTION:
  *//**       @brief         Static thread entry for ProxyMonitor
  *//**

  @par     DEPENDENCIES:
                             None
  *//*
           PARAMETERS:
  *//**       @param         pThis - this pointer

  *//*     RETURN VALUE:
  *//**       @return
                             int

  @par     SIDE EFFECTS:

  *//*==========================================================================*/
  int AudioSource::ProxyMonitorEntry(void* pThis)
  {
      if(!pThis)
      {
          return 0;
      }
      AudioSource *pMe = (AudioSource*)pThis;

      return pMe->ProxyMonitor();
  }


  /*==============================================================================
           FUNCTION:         ProxyMonitor

           DESCRIPTION:
  *//**       @brief         This thread keeps monitoring audio proxy availability
  *//**

  @par     DEPENDENCIES:
                             None
  *//*
           PARAMETERS:
  *//**       @param         None

  *//*     RETURN VALUE:
  *//**       @return
                             int

  @par     SIDE EFFECTS:

  *//*==========================================================================*/
  int AudioSource::ProxyMonitor()
  {
      while(m_eAudioSrcState == WFDMM_AUDIOSOURCE_STATE_PLAY)
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                      "AudioSource Before Proxy GetAvailability");
          m_bIsProxyAvailabile = GetProxyAvailability();
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                      "AudioSource After Proxy GetAvailability");
          //Sleep for 100ms each time before getting the status. This will make
          // that we are not loading the CPU
          MM_Timer_Sleep(100);
      }
      MM_Thread_Exit(m_ProxyMonitorThread, 0);
      return 0;
  }


  /*==============================================================================
           FUNCTION:         GetProxyAvailability

           DESCRIPTION:
  *//**       @brief         Gets availability of proxy
  *//**

  @par     DEPENDENCIES:
                             None
  *//*
           PARAMETERS:
  *//**       @param         None

  *//*     RETURN VALUE:
  *//**       @return
                             Available or Not

  @par     SIDE EFFECTS:

  *//*==========================================================================*/
  OMX_BOOL AudioSource::GetProxyAvailability()
  {
      int value = 0;
      android::String8 output;
      android::String8 key ("can_open_proxy=0");
      //Query audio system API
      output= android::AudioSystem::getParameters(0,key);
      android::AudioParameter result(output);
      if(result.getInt(android::String8("can_open_proxy"),value)== android::NO_ERROR)
      {
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
             "GetProxyAvailability: found can_open_proxy :value = %x", value);

      }
      else
      {
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
             "GetProxyAvailability: cant find can_open_proxy :value = %x", value);
      }
      return value ? OMX_TRUE: OMX_FALSE;
  }
