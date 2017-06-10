#ifndef __WFD_MM_SINK_VIDEO_SOURCE_H__
#define __WFD_MM_SINK_VIDEO_SOURCE_H__
/*==============================================================================
*       WFDMMSinkMediaSource.h
*
*  DESCRIPTION:
*       Class declaration for WFDMM Sink MediaSource. Connects RTP decoder and
*       parser and provides Audio and Video samples to
*       the media framework.
*
*
*  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
03/28/2013         SK            InitialDraft
================================================================================
*/

/*==============================================================================
**               Includes and Public Data Declarations
**==============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/
//#include "AEEStd.h"
#include <linux/msm_ion.h>
#include "OMX_Core.h"
#include "MMCriticalSection.h"
#include "WFDMMSinkCommon.h"
#include "filesource.h"
#include "GraphicBuffer.h"
#include "wdsm_mm_interface.h"
#include "RTPStreamPort.h"

typedef bool (*decryptCbType) (void* handle, int trackId, int input, int output,
                               int size, char* pIV, int IVSize);

typedef void (*avInfoCbType) (void* handle, avInfoType *pInfo);

typedef struct mediaSourceConfig
{
    decryptCbType pFnDecrypt;
    RTPStreamPort *pRTPStreamPort;
    uint32 nLocalIP;
    uint32 nPeerIP;
    unsigned short rtpPort;
    unsigned short rtcpPortLocal;
    unsigned short rtcpPortRemote;
    int    nRtpSock;
    int    nRtcpSock;
    WFD_audio_type eAudioFmt;
    int32  nFrameDropMode;
    bool   bIsTCP;
    bool   bHasVideo;
    bool   bHasAudio;
    bool   bSecure;
}mediaSourceConfigType;

class  WFDMMThreads;
class  RTPStreamPort;
class  SignalQueue;

class WFDMMSinkMediaSource
{
public:
    WFDMMSinkMediaSource(int moduleId,
                         WFDMMSinkHandlerFnType pFnHandler,
                         WFDMMSinkFBDType       pFnFBD,
                         avInfoCbType           pFnAVInfo,
                         void* clientData);

    ~WFDMMSinkMediaSource();

    OMX_ERRORTYPE Configure(mediaSourceConfigType *pCfg);

    OMX_ERRORTYPE Deinit();

    OMX_ERRORTYPE setFreeBuffer(
        int trackId,
        OMX_BUFFERHEADERTYPE *pBuffer
    );

    OMX_ERRORTYPE Start();

    OMX_ERRORTYPE Stop();

    void setFlushTimeStamp(uint64 nTS);

    void streamPlay(bool);

    void streamPause(void);

    void videoFrameBufferInfo(videoFrameInfoType frameInfo);
private:
    int state(int state, bool get_true_set_false);

    OMX_ERRORTYPE createResources();

    bool createThreadsAndQueues();

    static void VideoThreadEntry(void* pThis,
                          unsigned int nSignal);
    void VideoThread(unsigned int nSignal);

    static void AudioThreadEntry(void* pThis,
                          unsigned int nSignal);
    void AudioThread(unsigned int nSignal);

    bool configureDataSource();

    bool configureHDCPResources();

    bool configureParser();

    static void fileSourceCallback
    (
        FileSourceCallBackStatus status,
        void *pThis
    );

    void fileSourceCallbackHandler
    (
        FileSourceCallBackStatus status
    );

    int fetchVideoSample
    (
        uint32
    );

    int fetchVideoSampleSecure
    (
        uint32
    );

    int fetchAudioSample
    (
        uint32
    );

    int fetchAudioSampleSecure
    (
        uint32
    );

    bool allocateAudioBuffers();
    bool deallocateAudioBuffers();

    bool AllocateIonBufs();
    OMX_ERRORTYPE deinitialize();
    bool deallocateIonBufs();

    void InitData();
    void SetMediaBaseTime(uint64 nStartTime);
    bool ProximityCheck(uint64 nTestVal, uint64 nValA, uint64 nValB);
    void CheckBaseTimeChange(uint32);

    void CheckAndReqIDR();

    WFDMMSinkHandlerFnType mpFnHandler;
    WFDMMSinkFBDType       mpFnFBD;
    avInfoCbType           mpFnAVInfoCb;
    uint32 mVideoTrackId;
    uint32 mAudioTrackId;
    uint8 *mpVideoFmtBlk;
    uint32 mnVideoFmtBlkSize;
    uint8 *mpAudioFmtBlk;
    uint32 mnAudioFmtBlkSize;
    uint32 mnAudioMaxBufferSize;
    uint32 mnVideoMaxBufferSize;
    uint32 mnVideoTimescale;
    uint32 mnAudioTimescale;
    uint64 mnActualBaseTime;
    SignalQueue *mpAudioQ;
    SignalQueue *mpVideoQ;
    WFDMMThreads *mpVideoThread;
    WFDMMThreads *mpAudioThread;
    RTPStreamPort *mpRTPStreamPort;
    mediaSourceConfigType mCfg;
    MM_HANDLE mhCritSect;
    int      mnVideoFrameDropMode;
    FileSource  *mpFileSource;
    uint64   mnFlushTimeStamp;
    uint32   mnCurrVideoTime;
    uint64   mnMediaBaseTime;
    uint64   mnNewBaseTime;
    uint64   mnOldBaseTime;
    int32    meState;
    int32    mnTracks;
    int32    mIonFd;
    int32    mAudioBufFd;
    int32    mAudioBufSize;
    int32    mVideoBufSize;
    ion_user_handle_t mAudioIonHandle;
    int32    mVideoBufFd;
    ion_user_handle_t mVideoIonHandle;
    uint8   *mAudioBufPtr;
    uint8   *mVideoBufPtr;
    uint8   *mVideoHeapPtr;
    int      mnModuleId;
    void*    mClientData;
    bool     mbMediaTimeSet;
    bool     mbFlushInProgress;
    bool     mbPaused;
    bool     mbSinkIDRRequest;
    videoFrameInfoType meFrameTypeInfo;
    bool     mbMediaBaseTimeSet;
    bool     mbAudioStarted;
    bool     mbVideoStarted;
    bool     mbVideoTimeReset;
    bool     mbAudioTimeReset;
    bool     mbCorruptedFrame;
    uint64   mnLastIDRReqTime;
};
#endif /*__WFD_MM_SINK_VIDEO_SOURCE_H__*/
