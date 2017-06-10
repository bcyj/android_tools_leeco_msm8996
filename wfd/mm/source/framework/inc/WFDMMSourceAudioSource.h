/* =======================================================================
                              WFDMMSourceAudioSource.h
DESCRIPTION

Header file for WFDMMSourceAudioSource.cpp having relevant declarations

Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/framework/inc/WFDMMSourceAudioSource.h#2 $
$DateTime: 2012/02/10 05:45:30 $
$Change:$
========================================================================== */

#ifndef __WFD_MM_AUDIO_SOURCE_H__
#define __WFD_MM_AUDIO_SOURCE_H__

/*========================================================================
 *                    INCLUDE FILES FOR MODULE
 *========================================================================*/
#include "WFDMMSourceComDef.h"
#include "MMMalloc.h"
#include "OMX_Audio.h"
#include "MMThread.h"
#include "MMSignal.h"
#include "WFD_HdcpCP.h"
#include "WFDMMIonMemory.h"

extern "C" {
#include <tinyalsa/asoundlib.h>
}
//Temp definition of macro to avoid Makefilechanges
#ifdef AAC_DUMP_ENABLE
#define AUDIO_DUMP_ENABLE
#endif
/*========================================================================
 *                     CLASS F/W DECLARATIONS
 *========================================================================*/

class SignalQueue;   // forward declaration
class WFDMMSourceAudioEncode;

/*!
 *@brief Audio Enc Config structure.
 */

typedef struct AudioEncConfigType
{
   OMX_AUDIO_CODINGTYPE   eCoding;
   OMX_S32                nSamplingFrequency;
   OMX_S32                nBitrate;
   OMX_S8                 nNumChannels;
   OMX_S16                nBlockAlign;
   OMX_S16                nBitsPerSample;
   OMX_S32                nAudioInBufCount;

}AudioEncConfigType;

/*!
 *@brief Dumping types for audio
 */
typedef enum dump_type
{
    AAC_DUMP,
    WAV_DUMP
}AudioDump;

/**
* @brief Delivers YUV data in two different modes.
*
* In live mode, buffers are pre-populated with YUV data at the time
* of configuration. The source will loop through and deliver the
* pre-populated buffers throughout the life of the session. Frames will be
* delivered at the configured frame rate. This mode is useful for gathering
* performance statistics as no file reads are performed at run-time.
*
* In  non-live mode, buffers are populated with YUV data at run time.
* Buffers are delivered downstream as they become available. Timestamps
* are based on the configured frame rate, not on the system clock.
*
*/
class AudioSource
{
public:

    /**
    * @brief Frame callback for YUV buffer deliver
    */
    typedef void (*FrameDeliveryFnType) (OMX_BUFFERHEADERTYPE* pBuffer, void *);


    /**
    * @brief Constructor
    */
    AudioSource();

    /**
    * @brief Destructor
    */
    ~AudioSource();


    /**
    * @brief Configures the source
    *
    * @param[in] nFrames The number of frames to to play.
    * @param[in] nFramerate The frame rate to emulate.
    * @param[in] nFrameWidth The frame width.
    * @param[in] nFrameHeight The frame height.
    * @param[in] nBuffers The number of buffers allocated for the session.
    * @param[in] pFrameDeliverFn Frame delivery callback.
    * @param[in] pFileName Name of the file to read from
    * @param[in] nDVSXOffset Smaller frame pixel offset in the x direction
    * @param[in] nDVSYOffset Smaller frame pixel offset in the y direction
    * @param[in] bEnableLiveMode If true will encode in real time.
    *
    * @return    OMX_ERRORTYPE
    */
    OMX_ERRORTYPE Configure( AudioEncConfigType *pConfig,
                             FrameDeliveryFnType pFrameDeliverFn,
                             eventHandlerType  pEventHandler,
                             OMX_STRING pFileName,
                             OMX_U32  nModuleId,
                             void *appData
                           );


    /**
    * @brief Starts the delivery of buffers.
    *
    * All buffers should be registered through the SetFreeBuffer function
    * prior to calling this function.
    *
    * Source will continue to deliver buffers at the specified
    * rate until the specified number of frames have been delivered.
    *
    * Note that Source will not deliver buffers unless it has ownership
    * of atleast 1 buffer. If Source does not have ownership of buffer when
    * timer expires, it will block until a buffer is given to the component.
    * Free buffers should be given to the Source through SetFreeBuffer
    * function.
    * @param[in]    None
    *
    * @return       OMX_ERRORTYPE
    */
    OMX_ERRORTYPE Start();

    /**
    * @brief Wait for the thread to finish.
    *
    * @param[in]    None
    *
    * @return       OMX_ERRORTYPE
    * Function will block until the Source has delivered all frames.
    */
    OMX_ERRORTYPE Finish();

    /**
    * @brief Gives ownership of the buffer to the source.
    *
    * All bufffers must be registered with the Source prior
    * to invoking Start. This will give initial ownership to
    * the source.
    *
    * After Start is called, buffers must be given ownership
    * when YUV buffers are free.
    *
    * @param pBuffer Pointer to the buffer
    * @return       OMX_ERRORTYPE
    **************************************************************************/
    OMX_ERRORTYPE SetFreeBuffer(OMX_BUFFERHEADERTYPE* pBuffer);

    /**
    * @brief Get number of Output buffers required by the port.
    *
    *
    * @param nPortIndex[in] - Port number
    * @param nBuffers[rout] - Number of buffers
....*
    * @AUDIO_DUMP_ENABLEreturn OMX Error Codes.
    */
    OMX_ERRORTYPE GetOutNumBuffers(OMX_U32 nPortIndex, OMX_U32 *nBuffers);

    /**
    * @brief Get number of buffers required by the port.
    *
    * @param None

    * @return OMX_BUFFERHEADERTYPE
    */
    OMX_BUFFERHEADERTYPE** GetBuffers();

    /* HDCP handle */
    CWFD_HdcpCp* m_pHdcpHandle;
private:
    /**
    * @brief Audio Time Handler
    *
    * @param None
    *
    * @return None
    */
    static OMX_ERRORTYPE Audio_Handler (void *);
     /**
    * @brief configure Audio proxy device
    *
    * @param AudioSource
    *
    * @return OMX_ERRORTYPE
    */
    OMX_ERRORTYPE ConfigureAudioProxyDevice();

    /**
    * @brief sleep for duration equalent to one audio frame
    *
    * @param AudioSource
    *
    * @return None
    */
    void AudioSource_sleep_for_audioframe_interval(unsigned long);

    OMX_ERRORTYPE  FillHdcpAudioExtraData(
                               OMX_BUFFERHEADERTYPE *pBufHdr,
                               OMX_U8* pucPESPvtHeader,
                               OMX_U32 nPortIndex);



    static int SourceThreadEntry(OMX_PTR pThreadData);
    OMX_ERRORTYPE SourceThread();
    OMX_ERRORTYPE ConfigureAACEncoder();
    OMX_ERRORTYPE EncodeAAC(void* pIn, void* pOut);
    OMX_ERRORTYPE CreateResources();
    OMX_ERRORTYPE ConfigureAudioComponents();
    static int    ProxyMonitorEntry(void* pThis);
    int           ProxyMonitor();
    OMX_BOOL      GetProxyAvailability();

    typedef enum WFDAudioSourceStates
    {
        WFDMM_AUDIOSOURCE_STATE_INIT = 0,
        WFDMM_AUDIOSOURCE_STATE_IDLE,
        WFDMM_AUDIOSOURCE_STATE_PLAY,
        WFDMM_AUDIOSOURCE_STATE_PLAYING,
        WFDMM_AUDIOSOURCE_STATE_STOPPING,
        WFDMM_AUDIOSOURCE_STATE_UNDEFINED
    }WFDAudioSourceStatesType;

    WFDAudioSourceStatesType m_eAudioSrcState;
    eventHandlerType m_pEventHandlerFn;
    OMX_S32 m_nFramesRegistered;
    OMX_S32 m_nBuffers;
    SignalQueue* m_pBufferQueue;
    MM_HANDLE m_AudioSourceThreadHandle;
    OMX_BOOL m_bStarted;
    FrameDeliveryFnType m_pFrameDeliverFn;
    OMX_BUFFERHEADERTYPE** m_pInputBuffers;
    OMX_U32 m_nModuleId;
    void *m_appData;
    int inputmemfd;
    int outputmemfd;
    int m_fd;
    int ionfd;
    struct buffer *m_pHDCPInputBuffers;
    MM_HANDLE m_AudiosignalQ;
    OMX_U64 m_nBaseTime;
    OMX_U64 m_nCurrTime;
    OMX_BOOL m_bBaseTimeTaken;
    OMX_U32 m_nFrameTimeUnit;
    OMX_S64 m_nTimeErrAdjust;
    OMX_U32 m_nTimeErrAdjustCnt;
    OMX_U32 m_nTimeErrAdjustCntr;
    OMX_U32 m_bTimeReset;
    OMX_BOOL bKeepGoing;
    OMX_BOOL bRunning;
    OMX_BOOL m_bEncryptFlag;
    OMX_BOOL m_bFlushPending;
    OMX_BUFFERHEADERTYPE* pBufferHdr   ;
    OMX_AUDIO_CODINGTYPE     m_eAudioCodingType;
    OMX_U32 m_nSamplingFreq; /* audio sampling frequency*/
    OMX_U32 m_nBitRate;      /* Number of audio channels */
    unsigned int m_nAudiodevice;  /* Device from where audio shall be captured */
    OMX_U32 m_nAudioBufsize; /* m_nAudioDataBfr buffer size */
    OMX_U8 *m_pAudioDataBfr; /* Buffer to be read data from the memory
                              * map where driver has written it */
    WFDMMSourceAudioEncode *m_pAudioEncoder;
    OMX_U32 m_nAudioParamPeriodSize;

    OMX_S32 m_nAudioDelay;
    static const OMX_U32 AUDIO_SOURCE_PLAY_EVENT;
    static const OMX_U32 AUDIO_SOURCE_PAUSE_EVENT;
    static const OMX_U32 AUDIO_SOURCE_EXIT_EVENT;
    MM_HANDLE m_WFDAudioSourcePlaySignal;
    MM_HANDLE m_WFDAudioSourcePauseSignal;
    MM_HANDLE m_WFDAudioSourceExitSignal;
    MM_HANDLE m_ProxyMonitorThread;
    OMX_U8    m_nDropFrames;
    struct pcm *m_pPcmHandle;
    struct pcm_config m_PcmConfig;
    OMX_BOOL m_bIsProxyAvailabile;
    OMX_BOOL m_bIsProxyOpened;
    OMX_U32   m_nDownStreamFrames;
#ifdef AUDIO_DUMP_ENABLE
    FILE *m_fpAudio;
    OMX_BOOL m_bDumpAudio;
#endif
#ifdef WFD_DUMP_AUDIO_DATA
    FILE *m_fpWav;
    OMX_BOOL m_bDumpWav;
#endif
  };


#endif // #ifndef __WFD_MM_AUDIO_SOURCE_H__

