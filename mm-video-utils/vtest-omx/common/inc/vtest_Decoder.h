/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential

Copyright (c) 2010 The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of The Linux Foundation nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--------------------------------------------------------------------------*/

#ifndef _VTEST_DECODER_H
#define _VTEST_DECODER_H

#include "OMX_Core.h"
#include "OMX_Component.h"
#include "OMX_QCOMExtns.h"
#include "vtest_ComDef.h"
#include "vtest_SignalQueue.h"
#include "vtest_Thread.h"
#include "vtest_ISource.h"
#include "vtest_BufferManager.h"

namespace vtest {

/**
* @brief Delivers YUV data from the MDP for testing WFD.
*
*/
class Decoder : virtual public ISource {

public:
    Decoder(CodecConfigType *pConfig);
    ~Decoder();

    virtual PortBufferCapability GetBufferRequirements(OMX_U32 ePortIndex);
    virtual OMX_ERRORTYPE Start();
    virtual OMX_ERRORTYPE Stop();
    virtual OMX_ERRORTYPE Configure(CodecConfigType *pConfig,
            BufferManager *pBufManager, ISource *pSource,ISource *pSink);
    virtual OMX_ERRORTYPE SetBuffer(BufferInfo *pBufferInfo, ISource *pSource);
    virtual OMX_ERRORTYPE AllocateBuffers(BufferInfo **pBuffers,
            OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
            OMX_U32 nBufferSize, OMX_U32 ePortIndex, OMX_U32 nBufferUsage = 0);
    virtual OMX_ERRORTYPE UseBuffers(BufferInfo **pBuffers,
            OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
            OMX_U32 nBufferSize, OMX_U32 ePortIndex);

private:
    Decoder();
    virtual OMX_ERRORTYPE ThreadRun(OMX_PTR pThreadData);
    virtual OMX_ERRORTYPE FreeBuffer(
            BufferInfo *pBuffer, OMX_U32 ePortIndex);

    /**
     * @brief Set the encoder state
     *
     * This method can be asynchronous or synchronous. If asynchonous,
     * WaitState can be called to wait for the corresponding state
     * transition to complete.
     *
     * @param eState The state to enter
     * @param bSynchronous If OMX_TRUE, synchronously wait for
     *                     the state transition to complete
     */
    OMX_ERRORTYPE SetState(OMX_STATETYPE eState, OMX_BOOL bSynchronous);

    /**
     * @brief Wait for the corresponding state transition to complete
     *
     * @param eState The state to wait for
     */
    OMX_ERRORTYPE WaitState(OMX_STATETYPE eState);

    OMX_ERRORTYPE HandleOutputPortSettingsChange(OMX_U32 nData2);
    OMX_ERRORTYPE PortReconfigOutput();

    OMX_ERRORTYPE DumpCodecInfo();

    OMX_ERRORTYPE SetNativeWindowEnable();
    OMX_ERRORTYPE GetGraphicBufferUsage(OMX_U32 *nBufferUsage);

    static OMX_ERRORTYPE EventCallback(
            OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_PTR pAppData,
            OMX_IN OMX_EVENTTYPE eEvent, OMX_IN OMX_U32 nData1,
            OMX_IN OMX_U32 nData2, OMX_IN OMX_PTR pEventData);
    static OMX_ERRORTYPE EmptyDoneCallback(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData, OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);
    static OMX_ERRORTYPE FillDoneCallback(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData, OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);

    OMX_ERRORTYPE GetComponentRole(OMX_BOOL bSecureSession,
            FileType *eFileType, OMX_STRING *role);
    OMX_ERRORTYPE Flush(OMX_U32 nPortIndex);

private:
    OMX_BOOL m_bInputEOSReached;
    OMX_BOOL m_bSecureSession;
    OMX_BOOL m_bPortReconfig;
    SignalQueue *m_pSignalQueue;
    SignalQueue *m_pFreeBufferQueue;
    OMX_U32 m_nFrames;
    OMX_U32 m_nFramerate;
    OMX_U32 m_nFrameWidth;
    OMX_U32 m_nFrameHeight;
    OMX_U32 m_nInputBufferCount;
    OMX_U32 m_nInputBufferSize;
    OMX_U32 m_nOutputBufferCount;
    OMX_U32 m_nOutputBufferSize;
    OMX_U32 m_nOriginalOutputBufferCount;
    OMX_U32 m_nOutputBufferUsage;
    OMX_HANDLETYPE m_hDecoder;
    OMX_STATETYPE m_eState;
    OMX_STATETYPE m_eStatePending;
    OMX_VIDEO_CODINGTYPE m_eCodec;
    OMX_PORT_PARAM_TYPE m_sPortParam;
    OMX_PARAM_PORTDEFINITIONTYPE m_sPortFmt;
    PlaybackModeType m_ePlaybackMode;
};

} // namespace vtest

#endif // #ifndef _VTEST_DECODER_H
