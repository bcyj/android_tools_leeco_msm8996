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

#ifndef _VTEST_ENCODER_FILE_SOURCE_H
#define _VTEST_ENCODER_FILE_SOURCE_H

#include "OMX_Core.h"
#include "vtest_Debug.h"
#include "vtest_ComDef.h"
#include "vtest_ISource.h"
#include "vtest_File.h"
#include "vtest_Thread.h"
#include "vtest_SignalQueue.h"
#include "vtest_BufferManager.h"
#include "vtest_Crypto.h"

namespace vtest {
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
class EncoderFileSource : virtual public ISource {

public:
    EncoderFileSource(Crypto *pCrypto);
    ~EncoderFileSource();

    virtual PortBufferCapability GetBufferRequirements(OMX_U32 ePortIndex);
    virtual OMX_ERRORTYPE Configure(CodecConfigType *pConfig,
            BufferManager *pBufManager, ISource *pSource, ISource *pSink);
    virtual OMX_ERRORTYPE SetBuffer(BufferInfo *pBuffers, ISource *pSource);
    virtual OMX_ERRORTYPE AllocateBuffers(BufferInfo **pBuffers,
            OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
            OMX_U32 nBufferSize, OMX_U32 ePortIndex, OMX_U32 nBufferUsage = 0);
    /**
     * @brief Changes the frame rate
     *
     * The frame rate will take effect immediately.
     *
     * @param nFramerate The new frame rate.
     */
    virtual OMX_ERRORTYPE ChangeFrameRate(OMX_S32 nFramerate);

private:
    EncoderFileSource();
    virtual OMX_ERRORTYPE ThreadRun(OMX_PTR pThreadData);
    OMX_ERRORTYPE FillBuffer(OMX_BUFFERHEADERTYPE *pBuffer, OMX_S32 nFrameBytes);
    OMX_ERRORTYPE AllocateBuffer(BufferInfo *pBuffer,
            OMX_U32 ePortIndex, OMX_U32 nBufferSize);
    virtual OMX_ERRORTYPE FreeBuffer(
            BufferInfo *pBuffer, OMX_U32 ePortIndex);

private:
    OMX_S32 m_nFrames;
    OMX_S32 m_nFramesRegistered;
    OMX_S32 m_nFramerate;
    OMX_S32 m_nFrameWidth;
    OMX_S32 m_nFrameHeight;
    OMX_S32 m_nBuffers;
    OMX_S32 m_nDVSXOffset;
    OMX_S32 m_nDVSYOffset;
    File *m_pFile;
    OMX_BOOL m_bIsProfileMode;
    OMX_BOOL m_bSecureSession;
    OMX_BOOL m_bIsVTPath;
    OMX_U8 *m_pInCopyBuf;
    OMX_S32 m_nInCopyBufSize;
    Crypto *m_pCrypto;
    OMX_BOOL m_bMetaMode;
    YuvColorSpace m_eYuvColorSpace;
};

} // namespace vtest

#endif // #ifndef _VTEST_ENCODER_FILE_SOURCE_H
