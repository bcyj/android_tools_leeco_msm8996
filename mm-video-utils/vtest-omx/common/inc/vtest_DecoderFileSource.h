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

#ifndef _VTEST_DECODER_FILE_SOURCE_H
#define _VTEST_DECODER_FILE_SOURCE_H

#include "OMX_Core.h"
#include "vtest_ISource.h"
#include "vtest_SignalQueue.h"
#include "vtest_File.h"
#include "vtest_Thread.h"
#include "vtest_Crypto.h"

namespace vtest {

//typedef int (*OMX_ReadBuffer)(OMX_BUFFERHEADERTYPE* pBuffer);

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
class DecoderFileSource : virtual public ISource {

public:
    DecoderFileSource(Crypto *pCrypto);
    ~DecoderFileSource();

    virtual PortBufferCapability GetBufferRequirements(OMX_U32 ePortIndex);
    virtual OMX_ERRORTYPE Configure(CodecConfigType *pConfig,
            BufferManager *pBufManager, ISource *pSource, ISource *pSink);
    virtual OMX_ERRORTYPE SetBuffer(BufferInfo *pBuffer, ISource *pSource);
    /**
     * @brief Changes the frame rate
     *
     * The frame rate will take effect immediately.
     *
     * @param nFramerate The new frame rate.
     */
    virtual OMX_ERRORTYPE ChangeFrameRate(OMX_S32 nFramerate);

private:
    DecoderFileSource();
    virtual OMX_ERRORTYPE ThreadRun(OMX_PTR pThreadData);
    typedef int (DecoderFileSource::*OMX_ReadBuffer)(OMX_BUFFERHEADERTYPE *pBuffer);

    int ReadBufferFromDATFile(OMX_BUFFERHEADERTYPE *pBuffer);
    int SecureReadBufferFromDATFile(OMX_BUFFERHEADERTYPE *pBuffer);
    int ReadBufferArbitraryBytes(OMX_BUFFERHEADERTYPE *pBuffer);
    int ReadBufferFromVopStartCodeFile(OMX_BUFFERHEADERTYPE *pBuffer);
    int ReadBufferFromMpeg2StartCode(OMX_BUFFERHEADERTYPE *pBuffer);
    int ReadBufferFromSizeNal(OMX_BUFFERHEADERTYPE *pBuffer);
    int ReadBufferFromRCVFileSeqLayer(OMX_BUFFERHEADERTYPE *pBuffer);
    int ReadBufferFromRCVFile(OMX_BUFFERHEADERTYPE *pBuffer);
    int ReadBufferFromVC1File(OMX_BUFFERHEADERTYPE *pBuffer);
    int ReadBufferFromDivX456File(OMX_BUFFERHEADERTYPE *pBuffer);
    int ReadBufferFromDivX311File(OMX_BUFFERHEADERTYPE *pBuffer);
    int ReadBufferFromVP8File(OMX_BUFFERHEADERTYPE *pBuffer);

private:
    OMX_U32 m_nFramerate;
    OMX_U32 m_nFrameWidth;
    OMX_U32 m_nFrameHeight;
    OMX_U32 m_nBuffers;
    File *m_pFile;
    int m_nFileFd;
    OMX_U32 m_bIsProfileMode;
    OMX_BOOL m_bSecureSession;
    OMX_VIDEO_CODINGTYPE m_eCodec;
    FileType m_eFileType;
    OMX_ReadBuffer m_fReadBuffer;
    OMX_U32 m_nRcv_v1;
    OMX_U32 m_nVc1_bHdrFlag;
    Crypto *m_pCrypto;
};

} // namespace vtest

#endif // #ifndef _VTEST_FILE_SOURCE_H
