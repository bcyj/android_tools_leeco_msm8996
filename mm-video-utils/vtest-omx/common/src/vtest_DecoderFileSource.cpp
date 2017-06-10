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

#include <fcntl.h>
#include "venc/inc/omx_video_common.h"
#include "vtest_ComDef.h"
#include "vtest_Debug.h"
#include "vtest_Thread.h"
#include "vtest_SignalQueue.h"
#include "vtest_Sleeper.h"
#include "vtest_File.h"
#include "vtest_Time.h"
#include "vtest_DecoderFileSource.h"
#include "vtest_ISource.h"

#define VOP_START_CODE 0x000001B6
#define SHORT_HEADER_START_CODE 0x00008000
#define MPEG2_FRAME_START_CODE 0x00000100
#define MPEG2_SEQ_START_CODE 0x000001B3
#define VC1_START_CODE  0x00000100
#define VC1_FRAME_START_CODE  0x0000010D
#define VC1_FRAME_FIELD_CODE  0x0000010C
#define VC1_SEQUENCE_START_CODE 0x0000010F
#define VC1_ENTRY_POINT_START_CODE 0x0000010E
#define NUMBER_OF_ARBITRARYBYTES_READ  (4 * 1024)
#define SIZE_NAL_FIELD_MAX  4
#define MAX_NO_B_FRMS 3 // Number of non-b-frames packed in each buffer
#define N_PREV_FRMS_B 1 // Number of previous non-b-frames packed
    // with a set of consecutive b-frames
#define FRM_ARRAY_SIZE (MAX_NO_B_FRMS + N_PREV_FRMS_B)

#undef LOG_TAG
#define LOG_TAG "VTEST_DECODER_FILE_SOURCE"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
DecoderFileSource::DecoderFileSource(Crypto *pCrypto)
    : ISource(),
      m_nFrameWidth(0),
      m_nFrameHeight(0),
      m_nBuffers(0),
      m_pFile(NULL),
      m_nFileFd(-1),
      m_bIsProfileMode(OMX_FALSE),
      m_bSecureSession(OMX_FALSE),
      m_eCodec(OMX_VIDEO_CodingUnused),
      m_eFileType(FILE_TYPE_ARBITRARY_BYTES),
      m_fReadBuffer(NULL),
      m_nRcv_v1(0),
      m_nVc1_bHdrFlag(1),
      m_pCrypto(pCrypto) {

    snprintf(m_pName, PROPERTY_FILENAME_MAX, "DecoderFileSource");
    VTEST_MSG_HIGH("%s created", Name());
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
DecoderFileSource::~DecoderFileSource() {

    if (m_pFile != NULL) {
        m_pFile->Close();
        delete m_pFile;
    }
    if (m_nFileFd != -1) {
        close(m_nFileFd);
        m_nFileFd = -1;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PortBufferCapability DecoderFileSource::GetBufferRequirements(OMX_U32 ePortIndex) {

    PortBufferCapability sBufCap;

    Mutex::Autolock autoLock(m_pLock);
    memset(&sBufCap, 0, sizeof(PortBufferCapability));

    if (ePortIndex == PORT_INDEX_OUT) {
        sBufCap.bAllocateBuffer = OMX_FALSE;
        sBufCap.bUseBuffer = OMX_TRUE;
        sBufCap.pSource = this;
        sBufCap.ePortIndex = ePortIndex;
        sBufCap.nMinBufferSize = 0x1000;
        sBufCap.nMinBufferCount = 1;
        sBufCap.nBufferUsage = 0;
        sBufCap.nExtraBufferCount = 0;
    } else {
        VTEST_MSG_ERROR("Error: invalid port selection");
    }
    return sBufCap;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE DecoderFileSource::Configure(CodecConfigType *pConfig,
        BufferManager *pBufManager, ISource *pSource, ISource *pSink) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    result = ISource::Configure(pConfig, pBufManager, pSource, pSink);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("DecoderFileSource configure failed");
        return result;
    }

    Mutex::Autolock autoLock(m_pLock);

    char *pFileName = pConfig->cInFileName;
    m_bSecureSession = pConfig->bSecureSession;
    m_nFramerate = pConfig->nFramerate;
    m_nFrameWidth = pConfig->nFrameWidth;
    m_nFrameHeight = pConfig->nFrameHeight;
    m_bIsProfileMode = pConfig->bProfileMode;
    m_eCodec = pConfig->eCodec;
    m_eFileType = pConfig->eFileType;

    if (pConfig->nFramerate > 0 &&
        pConfig->nFrameWidth > 0 &&
        pConfig->nFrameHeight > 0) {
        if (m_eFileType == FILE_TYPE_DAT_PER_AU) {
            if (!m_bSecureSession) {
                m_fReadBuffer = &DecoderFileSource::ReadBufferFromDATFile;
            } else {
                m_fReadBuffer = &DecoderFileSource::SecureReadBufferFromDATFile;
            }
        } else if (m_eFileType == FILE_TYPE_ARBITRARY_BYTES) {
            m_fReadBuffer = &DecoderFileSource::ReadBufferArbitraryBytes;
        } else if (m_eCodec == OMX_VIDEO_CodingAVC) {
            m_fReadBuffer = &DecoderFileSource::ReadBufferFromSizeNal;
        } else if ((m_eCodec == OMX_VIDEO_CodingH263) ||
                   (m_eCodec == OMX_VIDEO_CodingMPEG4)) {
            m_fReadBuffer = &DecoderFileSource::ReadBufferFromVopStartCodeFile;
        } else if (m_eCodec == OMX_VIDEO_CodingMPEG2) {
            m_fReadBuffer = &DecoderFileSource::ReadBufferFromMpeg2StartCode;
        } else if (m_eFileType == FILE_TYPE_DIVX_4_5_6) {
            m_fReadBuffer = &DecoderFileSource::ReadBufferFromDivX456File;
        } else if (m_eFileType == FILE_TYPE_DIVX_311) {
            m_fReadBuffer = &DecoderFileSource::ReadBufferFromDivX311File;
        } else if (m_eFileType == FILE_TYPE_RCV) {
            m_fReadBuffer = &DecoderFileSource::ReadBufferFromRCVFile;
        } else if (m_eFileType == FILE_TYPE_VC1) {
            m_fReadBuffer = &DecoderFileSource::ReadBufferFromVC1File;
        } else if (m_eFileType == FILE_TYPE_VP8) {
            m_fReadBuffer = &DecoderFileSource::ReadBufferFromVP8File;
        } else {
            VTEST_MSG_ERROR("Error: Unknown option for read");
            return OMX_ErrorBadParameter;
        }

        if (pFileName != NULL) {

            if ((m_nFileFd = open(pFileName, O_RDONLY | O_LARGEFILE)) == -1) {

                VTEST_MSG_ERROR("Error - i/p file %s could NOT be opened errno = %d",
                                pFileName, errno);
                return OMX_ErrorBadParameter;
            }
            /*
            m_pFile = new File();
            if (m_pFile != NULL) {
                result = m_pFile->Open(pFileName, OMX_TRUE);
                if (result != OMX_ErrorNone) {
                    VTEST_MSG_ERROR("Failed to open file");
                }
            } else {
                VTEST_MSG_ERROR("Failed to allocate file");
                result = OMX_ErrorInsufficientResources;
            }
            */
        }
    } else {
        VTEST_MSG_ERROR("bad params");
        result = OMX_ErrorBadParameter;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE DecoderFileSource::SetBuffer(
        BufferInfo *pBuffer, ISource *pSource) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    result = ISource::SetBuffer(pBuffer, pSource);
    if (result != OMX_ErrorNone) {
        return result;
    }

    VTEST_MSG_LOW("queue push (%p %p)", pBuffer->pHeaderIn, pBuffer->pHeaderOut);
    result = m_pBufferQueue->Push(&pBuffer, sizeof(BufferInfo**));
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE DecoderFileSource::ChangeFrameRate(OMX_S32 nFramerate) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    Mutex::Autolock autoLock(m_pLock);

    if (nFramerate > 0) {
        m_nFramerate = nFramerate;
    } else {
        VTEST_MSG_ERROR("bad frame rate");
        result = OMX_ErrorBadParameter;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE DecoderFileSource::ThreadRun(OMX_PTR pThreadData) {

    (void)pThreadData;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *pHeader = NULL;
    BufferInfo *pBuffer = NULL;
    OMX_TICKS nTimeStamp = 0;

    for (OMX_S32 i = 1; !m_bThreadStop; i++) {
        // Since frame rate can change at any time, let's make sure that we use
        // the same frame rate for the duration of this loop iteration
        OMX_S32 nFramerate = m_nFramerate;

        // If in live mode we deliver frames in a real-time fashion
        if (m_bIsProfileMode) {
            VTEST_MSG_LOW("delaying frame %u ms", (int)(1000 / nFramerate));
            Sleeper::Sleep(1000 / nFramerate);
        }

        result = m_pBufferQueue->Pop(&pBuffer, sizeof(pBuffer), 0);
        VTEST_MSG_LOW("queue pop %u (qsize %u)", (unsigned int)i, (unsigned int)m_pBufferQueue->GetSize());

        if ((pBuffer == NULL) || (result != OMX_ErrorNone)) {
            /* Can only happen if stop is called or someone else ran into an
             * error */
            VTEST_MSG_HIGH("Stopping thread");
            result = OMX_ErrorNone;
            continue;
        }

        pHeader = pBuffer->pHeaderOut;
        pHeader->nInputPortIndex = 0;
        pHeader->nOffset = 0;
        pHeader->nTimeStamp = 0;
        pHeader->nFlags = 0;

        OMX_S32 nBytesRead = 0;
        OMX_BOOL bWriteTimestamp = OMX_TRUE;
        OMX_BOOL bReadFrame = OMX_TRUE;

        if (i == 1) {

            if (m_eCodec == OMX_VIDEO_CodingVP8) {
                char temp[32] = {0};
                unsigned int bytes_read = 0;
                bytes_read = read(m_nFileFd, temp, 32);
                if (temp[0] == 'D' && temp[1] == 'K' && temp[2] == 'I' && temp[3] == 'F') {
                    /* Skip first 32 bytes */
                    VTEST_MSG_LOW("IVF header found, skipping first 32 bytes !!");
                } else {
                    VTEST_MSG_LOW("No IVF header found");
                    lseek(m_nFileFd, -32, SEEK_CUR);
                }
            } else if (((int)m_eCodec == (int)QOMX_VIDEO_CodingDivx) &&
                    (m_eFileType == FILE_TYPE_DIVX_311)) {
                char temp[8] = {0};
                unsigned int bytes_read = 0;
                VTEST_MSG_MEDIUM("Skipping first 8 bytes for Divx311 !!");
                /* Skip first 8 bytes */
                bytes_read = read(m_nFileFd, temp, 8);
            } else if (m_eCodec == OMX_VIDEO_CodingWMV) {
                if (m_eFileType == FILE_TYPE_RCV) {
                    nBytesRead = ReadBufferFromRCVFileSeqLayer(pHeader);
                    VTEST_MSG_HIGH("After Read_Buffer_From_RCV_File_Seq_Layer, bytes read %u",
                            (unsigned int)nBytesRead);
                    bWriteTimestamp = OMX_FALSE;
                } else if (m_eFileType == FILE_TYPE_VC1) {
                    m_nVc1_bHdrFlag = 1;
                    nBytesRead = (this->*m_fReadBuffer)(pHeader);
                    m_nVc1_bHdrFlag = 0;
                    VTEST_MSG_HIGH("After 1st Read_Buffer for VC1, bytes read %u",
                            (unsigned int)nBytesRead);
                    bWriteTimestamp = OMX_FALSE;
                }
                bReadFrame = OMX_FALSE;
            }
        }

        if (bReadFrame) {
            nBytesRead = (this->*m_fReadBuffer)(pHeader);
        }

        if (nBytesRead < 0) {
            VTEST_MSG_ERROR("Bytes read can never be negative. Erroring out");
            SetError();
        }

        pHeader->nFilledLen = nBytesRead;

        if (nBytesRead == 0) {
            pHeader->nFlags |= OMX_BUFFERFLAG_EOS;
            VTEST_MSG_HIGH("enable OMX_BUFFERFLAG_EOS on frame %u", (unsigned int)i);
            m_bThreadStop = OMX_TRUE;
        }

        if ((pHeader->nTimeStamp == 0) && bWriteTimestamp) {
            if (m_bIsProfileMode) {
                nTimeStamp = (OMX_TICKS)Time::GetTimeMicrosec();
            } else {
                nTimeStamp = nTimeStamp + (OMX_TICKS)(1000000 / nFramerate);
            }
            pHeader->nTimeStamp = nTimeStamp;
        }

        VTEST_MSG_MEDIUM("delivering frame %u, bytes %d", (unsigned int)i, (int)nBytesRead);
        m_pSink->SetBuffer(pBuffer, this);
    }

    //clean up
    while(m_pBufferQueue->GetSize() > 0) {
        VTEST_MSG_LOW("cleanup: q-wait (qsize %u)", (unsigned int)m_pBufferQueue->GetSize());
        m_pBufferQueue->Pop(&pBuffer, sizeof(BufferInfo **), 0);
        m_pSink->SetBuffer(pBuffer, this);
    }

    VTEST_MSG_HIGH("thread exiting...");
    return result;
}

int DecoderFileSource::ReadBufferFromDATFile(OMX_BUFFERHEADERTYPE *pBuffer) {

    long frameSize = 0;
    char temp_buffer[10];
    char temp_byte;
    int bytes_read = 0;
    int i = 0;
    unsigned char *read_buffer = NULL;
    char c = '1'; //initialize to anything except '\0'(0)
    char inputFrameSize[12] = { 0 };
    int count = 0; int cnt = 0;
    memset(temp_buffer, 0, sizeof(temp_buffer));

    VTEST_MSG_LOW("Inside %s ", __FUNCTION__);

    while (cnt < 10)
    /* Check the input file format, may result in infinite loop */ {
        VTEST_MSG_LOW("loop[%d] count[%d]", cnt, count);
        count = read(m_nFileFd, &inputFrameSize[cnt], 1);
        if (inputFrameSize[cnt] == '\0') {
            break;
        }
        cnt++;
    }
    inputFrameSize[cnt] = '\0';
    frameSize = atoi(inputFrameSize);
    pBuffer->nFilledLen = 0;

    /* get the frame length */
    lseek64(m_nFileFd, -1, SEEK_CUR);
    bytes_read = read(m_nFileFd, pBuffer->pBuffer, frameSize);

    VTEST_MSG_LOW("Actual frame Size [%ld] bytes_read using fread[%d]",
                  frameSize, bytes_read);

    if (bytes_read == 0 || bytes_read < frameSize) {
        VTEST_MSG_LOW("Bytes read Zero After Read frame Size ");
        return 0;
    }
    return bytes_read;
}

int DecoderFileSource::SecureReadBufferFromDATFile(OMX_BUFFERHEADERTYPE *pBuffer) {

    long frameSize = 0;
    char temp_buffer[10];
    char temp_byte;
    int bytes_read = 0;
    int i = 0;
    unsigned char *read_buffer = NULL;
    char c = '1'; //initialize to anything except '\0'(0)
    char inputFrameSize[12] = { 0 };
    int count = 0; int cnt = 0;
    OMX_U8 *temp_read_buffer = NULL;
    memset(temp_buffer, 0, sizeof(temp_buffer));
    OMX_U32 bytes_copied = 0;
    OMX_ERRORTYPE result = OMX_ErrorNone;

    VTEST_MSG_LOW("Inside %s ", __FUNCTION__);

    while (cnt < 10) {
        /* Check the input file format, may result in infinite loop */
        VTEST_MSG_LOW("loop[%d] count[%d]", cnt, count);
        count = read(m_nFileFd, &inputFrameSize[cnt], 1);
        if (inputFrameSize[cnt] == '\0') {
            break;
        }
        cnt++;
    }
    inputFrameSize[cnt] = '\0';
    frameSize = atoi(inputFrameSize);
    if (frameSize == 0) {
        VTEST_MSG_ERROR("Frame Size is 0");
        return 0;
    }
    pBuffer->nFilledLen = 0;

    if (!temp_read_buffer) {
        temp_read_buffer = (OMX_U8 *)malloc(sizeof(OMX_U8) * frameSize);
        if (!temp_read_buffer) {
            VTEST_MSG_ERROR("Out of memory: unable to allocate temp read buffer");
            return 0;
        }
    }

    /* get the frame length */
    lseek64(m_nFileFd, -1, SEEK_CUR);
    bytes_read = read(m_nFileFd, temp_read_buffer, frameSize);

    VTEST_MSG_LOW("Actual frame Size [%ld] bytes_read using fread[%d]",
                  frameSize, bytes_read);

    if (bytes_read == 0 || bytes_read < frameSize) {
        VTEST_MSG_LOW("Bytes read Zero After Read frame Size");
        result = OMX_ErrorUndefined;
    } else {

        result = m_pCrypto->Copy(SAMPLECLIENT_COPY_NONSECURE_TO_SECURE,
                temp_read_buffer, (unsigned long)pBuffer->pBuffer, bytes_read);

        if (result != OMX_ErrorNone) {
            VTEST_MSG_ERROR("OEMCrypto_Copy failed, result is %d", result);
        }
    }

    if (temp_read_buffer) {
        free(temp_read_buffer);
    }
    if (result != OMX_ErrorNone) {
        return 0;
    }
    return bytes_read;
}

int DecoderFileSource::ReadBufferArbitraryBytes(OMX_BUFFERHEADERTYPE *pBuffer) {

    int bytes_read = 0;
    VTEST_MSG_LOW("Inside %s", __FUNCTION__);
    bytes_read = read(m_nFileFd, pBuffer->pBuffer, NUMBER_OF_ARBITRARYBYTES_READ);
    if (bytes_read == 0) {
        VTEST_MSG_LOW("Bytes read Zero After Read frame Size");
    }
    return bytes_read;
}

int DecoderFileSource::ReadBufferFromVopStartCodeFile(OMX_BUFFERHEADERTYPE *pBuffer) {

    unsigned int readOffset = 0;
    int bytes_read = 0;
    unsigned int code = 0;
    pBuffer->nFilledLen = 0;
    static unsigned int header_code = 0;

    VTEST_MSG_LOW("Inside %s", __FUNCTION__);

    do {
        //Start codes are always byte aligned.
        bytes_read = read(m_nFileFd, &pBuffer->pBuffer[readOffset], 1);
        if (bytes_read == 0 || bytes_read == -1) {
            VTEST_MSG_LOW("Bytes read Zero");
            break;
        }
        code <<= 8;
        code |= (0x000000FF & pBuffer->pBuffer[readOffset]);
        //VOP start code comparision
        if (readOffset > 3) {
            if (!header_code) {
                if (VOP_START_CODE == code) {
                    header_code = VOP_START_CODE;
                } else if ((0xFFFFFC00 & code) == SHORT_HEADER_START_CODE) {
                    header_code = SHORT_HEADER_START_CODE;
                }
            }
            if ((header_code == VOP_START_CODE) && (code == VOP_START_CODE)) {
                //Seek backwards by 4
                lseek64(m_nFileFd, -4, SEEK_CUR);
                readOffset -= 3;
                break;
            } else if ((header_code == SHORT_HEADER_START_CODE) && (SHORT_HEADER_START_CODE == (code & 0xFFFFFC00))) {
                //Seek backwards by 4
                lseek64(m_nFileFd, -4, SEEK_CUR);
                readOffset -= 3;
                break;
            }
        }
        readOffset++;
    }
    while (1);

    return readOffset;
}

int DecoderFileSource::ReadBufferFromMpeg2StartCode(OMX_BUFFERHEADERTYPE *pBuffer) {

    unsigned int readOffset = 0;
    int bytesRead = 0;
    unsigned int code = 0;
    pBuffer->nFilledLen = 0;
    static unsigned int firstParse = true;
    unsigned int seenFrame = false;
    OMX_U8 *pTempBuffer = pBuffer->pBuffer;

    VTEST_MSG_LOW("Inside %s", __FUNCTION__);

    if (pBuffer->nAllocLen <= 0) {
        VTEST_MSG_ERROR("pBuffer Alloc len is 0");
        return 0;
    }

    if (m_bSecureSession) {
        pTempBuffer = new OMX_U8[pBuffer->nAllocLen];
        if (!pTempBuffer) {
            VTEST_MSG_ERROR("Out of memory: unable to allocate temp read buffer");
            return 0;
        }
    }

    /* Read one byte at a time. Construct the code every byte in order to
     * compare to the start codes. Keep looping until we've read in a complete
     * frame, which can be either just a picture start code + picture, or can
     * include the sequence header as well
     */
    while (1) {
        bytesRead = read(m_nFileFd, &pTempBuffer[readOffset], 1);

        /* Exit the loop if we can't read any more bytes */
        if (bytesRead == 0 || bytesRead == -1) {
            break;
        }

        /* Construct the code one byte at a time */
        code <<= 8;
        code |= (0x000000FF & pTempBuffer[readOffset]);

        /* Can't compare the code to MPEG2 start codes until we've read the
         * first four bytes
         */
        if (readOffset >= 3) {

            /* If this is the first time we're reading from the file, then we
             * need to throw away the system start code information at the
             * beginning. We can just look for the first sequence header.
             */
            if (firstParse) {
                if (code == MPEG2_SEQ_START_CODE) {
                    /* Seek back by 4 bytes and reset code so that we can skip
                     * down to the common case below.
                     */
                    lseek(m_nFileFd, -4, SEEK_CUR);
                    code = 0;
                    readOffset -= 3;
                    firstParse = false;
                    continue;
                }
            }

            /* If we have already parsed a frame and we see a sequence header, then
             * the sequence header is part of the next frame so we seek back and
             * break.
             */
            if (code == MPEG2_SEQ_START_CODE) {
                if (seenFrame) {
                    lseek(m_nFileFd, -4, SEEK_CUR);
                    readOffset -= 3;
                    break;
                }
                /* If we haven't seen a frame yet, then read in all the data until we
                 * either see another frame start code or sequence header start code.
                 */
            } else if (code == MPEG2_FRAME_START_CODE) {
                if (!seenFrame) {
                    seenFrame = true;
                } else {
                    lseek(m_nFileFd, -4, SEEK_CUR);
                    readOffset -= 3;
                    break;
                }
            }
        }
        readOffset++;
    }

    if (m_bSecureSession) {
        if (readOffset > 0) {
            OMX_ERRORTYPE result = m_pCrypto->Copy(SAMPLECLIENT_COPY_NONSECURE_TO_SECURE,
                    pTempBuffer, (unsigned long)pBuffer->pBuffer, readOffset);
            if (result != OMX_ErrorNone) {
                VTEST_MSG_ERROR("OEMCrypto_Copy failed, result is %d", result);
                readOffset = 0;
            }
        }
        if (pTempBuffer) {
            delete [] pTempBuffer;
        }
    }
    return readOffset;
}

int DecoderFileSource::ReadBufferFromSizeNal(OMX_BUFFERHEADERTYPE *pBuffer) {

    char temp_size[SIZE_NAL_FIELD_MAX];
    int i = 0;
    int j = 0;
    unsigned int size = 0;   // Need to make sure that uint32 has SIZE_NAL_FIELD_MAX (4) bytes
    int bytes_read = 0;
    int nalSize = 4;
    unsigned int *sptr = NULL;

    // read the "size_nal_field"-byte size field
    bytes_read = read(m_nFileFd, pBuffer->pBuffer + pBuffer->nOffset, nalSize);
    if (bytes_read == 0 || bytes_read == -1) {
        VTEST_MSG_LOW("Failed to read frame or it might be EOF");
        return 0;
    }

    for (i = 0; i < SIZE_NAL_FIELD_MAX - nalSize; i++) {
        temp_size[SIZE_NAL_FIELD_MAX - 1 - i] = 0;
    }

    /* Due to little endiannes, Reorder the size based on size_nal_field */
    for (j = 0; i < SIZE_NAL_FIELD_MAX; i++, j++) {
        temp_size[SIZE_NAL_FIELD_MAX - 1 - i] = pBuffer->pBuffer[pBuffer->nOffset + j];
    }
    size = (unsigned int)(*((unsigned int *)(void *)(temp_size)));

    // now read the data
    bytes_read = read(m_nFileFd, pBuffer->pBuffer + pBuffer->nOffset + nalSize, size);
    if ((unsigned int)bytes_read != size) {
        VTEST_MSG_ERROR("Failed to read frame");
    }
    return bytes_read + nalSize;
}

int DecoderFileSource::ReadBufferFromRCVFileSeqLayer(OMX_BUFFERHEADERTYPE *pBuffer) {

    unsigned int readOffset = 0, size_struct_C = 0;
    unsigned int startcode = 0;
    pBuffer->nFilledLen = 0;
    OMX_U8 *pTempBuffer = pBuffer->pBuffer;

    VTEST_MSG_LOW("Inside %s", __FUNCTION__);

    if (m_bSecureSession) {
        pTempBuffer = new OMX_U8[1024];
        if (!pTempBuffer) {
            VTEST_MSG_ERROR("Out of memory: unable to allocate temp read buffer");
            return 0;
        }
    }

    read(m_nFileFd, &startcode, 4);

    /* read size of struct C as it need not be 4 always*/
    read(m_nFileFd, &size_struct_C, 4);

    if ((startcode & 0xFF000000) == 0xC5000000) {

        VTEST_MSG_LOW("Read_Buffer_From_RCV_File_Seq_Layer size_struct_C: %d", size_struct_C);
        readOffset = read(m_nFileFd, pTempBuffer, size_struct_C);
        lseek64(m_nFileFd, 24, SEEK_CUR);
    } else if ((startcode & 0xFF000000) == 0x85000000) {
        // .RCV V1 file
        m_nRcv_v1 = 1;
        VTEST_MSG_LOW("Read_Buffer_From_RCV_File_Seq_Layer size_struct_C: %d", size_struct_C);
        readOffset = read(m_nFileFd, pTempBuffer, size_struct_C);
        lseek64(m_nFileFd, 8, SEEK_CUR);
    } else {
        VTEST_MSG_ERROR("Error: Unknown VC1 clip format %x", startcode);
    }

#if 0
    {
        int i=0;
        printf("Read_Buffer_From_RCV_File, length %d readOffset %d\n", readOffset, readOffset);
        for (i=0; i<36; i++){
            printf("0x%.2x ", pBuffer->pBuffer[i]);
            if (i%16 == 15) {
                printf("\n");
            }
        }
        printf("\n");
    }
#endif

    if (m_bSecureSession) {
        if (readOffset > 0) {
            OMX_ERRORTYPE result = m_pCrypto->Copy(SAMPLECLIENT_COPY_NONSECURE_TO_SECURE,
                    pTempBuffer, (unsigned long)pBuffer->pBuffer, readOffset);
            if (result != OMX_ErrorNone) {
                VTEST_MSG_ERROR("OEMCrypto_Copy failed, result is %d", result);
                readOffset = 0;
            }
        }
        if (pTempBuffer) {
            delete [] pTempBuffer;
        }
    }

    pBuffer->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;
    return readOffset;
}

int DecoderFileSource::ReadBufferFromRCVFile(OMX_BUFFERHEADERTYPE *pBuffer) {

    unsigned int readOffset = 0;
    unsigned int len = 0;
    unsigned int key = 0;
    OMX_U8 *pTempBuffer = pBuffer->pBuffer;

    VTEST_MSG_LOW("Inside %s \n", __FUNCTION__);

    VTEST_MSG_LOW("Read_Buffer_From_RCV_File - nOffset %u\n", (unsigned int)pBuffer->nOffset);

    if (pBuffer->nAllocLen <= 0) {
        VTEST_MSG_ERROR("pBuffer Alloc len is 0");
        return 0;
    }

    if (m_bSecureSession) {
        pTempBuffer = new OMX_U8[pBuffer->nAllocLen];
        if (!pTempBuffer) {
            VTEST_MSG_ERROR("Out of memory: unable to allocate temp read buffer");
            return 0;
        }
    }

    if (m_nRcv_v1) {
        /* for the case of RCV V1 format, the frame header is only of 4 bytes and has
           only the frame size information */
        readOffset = read(m_nFileFd, &len, 4);
        VTEST_MSG_LOW("Read_Buffer_From_RCV_File - framesize %d %x", len, len);

    } else {
        /* for a regular RCV file, 3 bytes comprise the frame size and 1 byte for key*/
        readOffset = read(m_nFileFd, &len, 3);
        VTEST_MSG_LOW("Read_Buffer_From_RCV_File - framesize %d %x", len, len);

        readOffset = read(m_nFileFd, &key, 1);
        if ((key & 0x80) == false) {
            VTEST_MSG_LOW("Read_Buffer_From_RCV_File - Non IDR frame key %x", key);
        }
    }

    if (!m_nRcv_v1) {
        /* There is timestamp field only for regular RCV format and not for RCV V1 format*/
        readOffset = read(m_nFileFd, &pBuffer->nTimeStamp, 4);
        VTEST_MSG_LOW("Read_Buffer_From_RCV_File - timeStamp %lld", pBuffer->nTimeStamp);
        pBuffer->nTimeStamp *= 1000;
    }

    if (len > pBuffer->nAllocLen) {
        VTEST_MSG_ERROR("Error in sufficient buffer framesize %u, alloclen %u noffset %u",
                len, (unsigned int)pBuffer->nAllocLen, (unsigned int)pBuffer->nOffset);
        readOffset = 0;
    } else {
        readOffset = read(m_nFileFd, pTempBuffer + pBuffer->nOffset, len);
    }

#if 0
    {
        int i=0;
        printf("Read_Buffer_From_RCV_File, length %d readOffset %d\n", len, readOffset);
        for (i=0; i<64; i++){
            printf("0x%.2x ", pBuffer->pBuffer[i]);
            if (i%16 == 15) {
                printf("\n");
            }
        }
        printf("\n");
    }
#endif

    if (m_bSecureSession) {
        if (readOffset > 0) {
            OMX_ERRORTYPE result = m_pCrypto->Copy(SAMPLECLIENT_COPY_NONSECURE_TO_SECURE,
                    pTempBuffer, (unsigned long)pBuffer->pBuffer, readOffset);
            if (result != OMX_ErrorNone) {
                VTEST_MSG_ERROR("OEMCrypto_Copy failed, result is %d", result);
                readOffset = 0;
            }
        }
        if (pTempBuffer) {
            delete [] pTempBuffer;
        }
    }

    if (readOffset != len) {
        VTEST_MSG_LOW("EOS reach or Reading error %d, %s", readOffset, strerror(errno));
        readOffset = 0;
    }
    return readOffset;
}

int DecoderFileSource::ReadBufferFromVC1File(OMX_BUFFERHEADERTYPE *pBuffer) {

    unsigned int readOffset = 0;
    int bytes_read = 0;
    unsigned int code = 0, total_bytes = 0;
    int startCode_cnt = 0;
    int bSEQflag = 0;
    int bEntryflag = 0;
    unsigned int SEQbytes = 0;
    int numStartcodes = 0;
    OMX_U8 *pTempBuffer = pBuffer->pBuffer;

    VTEST_MSG_LOW("Inside %s \n", __FUNCTION__);

    if (pBuffer->nAllocLen <= 0) {
        VTEST_MSG_ERROR("pBuffer Alloc len is 0");
        return 0;
    }

    if (m_bSecureSession) {
        pTempBuffer = new OMX_U8[pBuffer->nAllocLen];
        if (!pTempBuffer) {
            VTEST_MSG_ERROR("Out of memory: unable to allocate temp read buffer");
            return 0;
        }
    }

    numStartcodes = m_nVc1_bHdrFlag ? 1 : 2;

    do {
        if (total_bytes == pBuffer->nAllocLen) {
            VTEST_MSG_ERROR("Buffer overflow!");
            break;
        }
        //Start codes are always byte aligned.
        bytes_read = read(m_nFileFd, &pTempBuffer[readOffset], 1);

        if (!bytes_read) {
            VTEST_MSG_LOW("\n Bytes read Zero \n");
            break;
        }
        total_bytes++;
        code <<= 8;
        code |= (0x000000FF & pTempBuffer[readOffset]);

        if (!bSEQflag && (code == VC1_SEQUENCE_START_CODE)) {
            if (startCode_cnt) bSEQflag = 1;
        }

        if (!bEntryflag && (code == VC1_ENTRY_POINT_START_CODE)) {
            if (startCode_cnt) bEntryflag = 1;
        }

        if (code == VC1_FRAME_START_CODE || code == VC1_FRAME_FIELD_CODE) {
            startCode_cnt++;
        }

        //VOP start code comparision
        if (startCode_cnt == numStartcodes) {
            if (VC1_FRAME_START_CODE == (code & 0xFFFFFFFF) ||
                VC1_FRAME_FIELD_CODE == (code & 0xFFFFFFFF)) {
                //previous_vc1_au = 0;
                if (VC1_FRAME_FIELD_CODE == (code & 0xFFFFFFFF)) {
                    //previous_vc1_au = 1;
                }

                if (!m_nVc1_bHdrFlag && (bSEQflag || bEntryflag)) {
                    lseek(m_nFileFd, -(SEQbytes + 4), SEEK_CUR);
                    readOffset -= (SEQbytes + 3);
                } else {
                    //Seek backwards by 4
                    lseek64(m_nFileFd, -4, SEEK_CUR);
                    readOffset -= 3;
                }

                while (pTempBuffer[readOffset - 1] == 0) readOffset--;
                break;
            }
        }
        readOffset++;
        if (bSEQflag || bEntryflag) {
            SEQbytes++;
        }
    }
    while (1);

#if 0
    {
        int i=0;
        printf("Read_Buffer_From_VC1_File, readOffset %d\n", readOffset);
        for (i=0; i<64; i++){
            printf("0x%.2x ", pBuffer->pBuffer[i]);
            if (i%16 == 15) {
                printf("\n");
            }
        }
        printf("\n");
    }
#endif

    if (m_bSecureSession) {
        if (readOffset > 0) {
            OMX_ERRORTYPE result = m_pCrypto->Copy(SAMPLECLIENT_COPY_NONSECURE_TO_SECURE,
                    pTempBuffer, (unsigned long)pBuffer->pBuffer, readOffset);
            if (result != OMX_ErrorNone) {
                VTEST_MSG_ERROR("OEMCrypto_Copy failed, result is %d", result);
                readOffset = 0;
            }
        }
        if (pTempBuffer) {
            delete [] pTempBuffer;
        }
    }
    return readOffset;
}

int DecoderFileSource::ReadBufferFromDivX456File(OMX_BUFFERHEADERTYPE *pBuffer) {

    char *p_buffer = NULL;
    unsigned int offset_array[FRM_ARRAY_SIZE];
    int pckt_end_idx = 0, i, vop_set_cntr = 0;
    unsigned int read_code = 0, bytes_read, byte_pos = 0, frame_type;
    unsigned int b_frm_idx, b_frames_found = 0, byte_cntr;
    bool pckt_ready = false;
    char pckt_type[20];
    int pckd_frms = 0;
    static unsigned long long int total_bytes = 0;
    static unsigned long long int total_frames = 0;

    VTEST_MSG_LOW("Inside %s \n", __FUNCTION__);

    do {
        p_buffer = (char *)pBuffer->pBuffer + byte_pos;

        bytes_read = read(m_nFileFd, p_buffer, NUMBER_OF_ARBITRARYBYTES_READ);
        byte_pos += bytes_read;
        for (byte_cntr = 0; byte_cntr < bytes_read && !pckt_ready; byte_cntr++) {
            read_code <<= 8;
            ((char *)&read_code)[0] = p_buffer[byte_cntr];
            if (read_code == VOP_START_CODE) {
                if ((++byte_cntr) < bytes_read) {
                    frame_type = p_buffer[byte_cntr];
                    frame_type &= 0x000000C0;
                    switch (frame_type) {
                    case 0x00:
                        pckt_type[pckd_frms] = 'I'; break;
                    case 0x40:
                        pckt_type[pckd_frms] = 'P'; break;
                    case 0x80:
                        pckt_type[pckd_frms] = 'B'; break;
                    default:
                        pckt_type[pckd_frms] = 'X';
                    }
                    pckd_frms++;
                    offset_array[vop_set_cntr] = byte_pos - bytes_read + byte_cntr - 4;
                    if (frame_type == 0x80) { // B Frame found!
                        if (!b_frames_found) {
                            // Try to packet N_PREV_FRMS_B previous frames
                            // with the next consecutive B frames
                            i = N_PREV_FRMS_B;
                            while ((vop_set_cntr - i) < 0 && i > 0) i--;
                            b_frm_idx = vop_set_cntr - i;
                            if (b_frm_idx > 0) {
                                pckt_end_idx = b_frm_idx;
                                pckt_ready = true;
                                pckt_type[b_frm_idx] = '\0';
                                total_frames += b_frm_idx;
                            }
                        }
                        b_frames_found++;
                    } else if (b_frames_found) {
                        pckt_end_idx = vop_set_cntr;
                        pckt_ready = true;
                        pckt_type[pckd_frms - 1] = '\0';
                        total_frames += pckd_frms - 1;
                    } else if (vop_set_cntr == (FRM_ARRAY_SIZE - 1)) {
                        pckt_end_idx = MAX_NO_B_FRMS;
                        pckt_ready = true;
                        pckt_type[pckt_end_idx] = '\0';
                        total_frames += pckt_end_idx;
                    } else {
                        vop_set_cntr++;
                    }
                } else {
                    // The vop start code was found in the last 4 bytes,
                    // seek backwards by 4 to include this start code
                    // with the next buffer.
                    lseek64(m_nFileFd, -4, SEEK_CUR);
                    byte_pos -= 4;
                    pckd_frms--;
                }
            }
        }
        if (pckt_ready) {
            loff_t off = (byte_pos - offset_array[pckt_end_idx]);
            if (lseek64(m_nFileFd, -1LL * off, SEEK_CUR) == -1) {
                VTEST_MSG_ERROR("lseek64 with offset = %lld failed with errno %d"
                                ", current position =0x%llx", -1LL * off,
                                errno, (long long int)lseek64(m_nFileFd, 0, SEEK_CUR));
            }
        } else {
            char eofByte;
            int ret = read(m_nFileFd, &eofByte, 1);
            if (ret == 0) {
                offset_array[vop_set_cntr] = byte_pos;
                pckt_end_idx = vop_set_cntr;
                pckt_ready = true;
                pckt_type[pckd_frms] = '\0';
                total_frames += pckd_frms;
            } else if (ret == 1) {
                if (lseek64(m_nFileFd, -1, SEEK_CUR) == -1) {
                    VTEST_MSG_ERROR("lseek64 failed with errno = %d, "
                                    "current fileposition = %llx",
                                    errno,
                                    (long long int)lseek64(m_nFileFd, 0, SEEK_CUR));
                }
            } else {
                VTEST_MSG_ERROR("Error when checking for EOF");
            }
        }
    }
    while (!pckt_ready);
    pBuffer->nFilledLen = offset_array[pckt_end_idx];
    total_bytes += pBuffer->nFilledLen;
    VTEST_MSG_ERROR("[DivX] Packet: Type[%s] Size[%u] TB[%llx] NFrms[%lld]",
                    pckt_type, (unsigned int)pBuffer->nFilledLen, total_bytes, total_frames);
    return pBuffer->nFilledLen;
}

int DecoderFileSource::ReadBufferFromDivX311File(OMX_BUFFERHEADERTYPE *pBuffer) {

    char *p_buffer = NULL;
    unsigned int bytes_read = 0;
    int frame_size = 0;
    unsigned int num_bytes_size = 4;
    unsigned int n_offset = 0;

    VTEST_MSG_LOW("Inside %s \n", __FUNCTION__);

    if (pBuffer != NULL) {
        p_buffer = (char *)pBuffer->pBuffer + pBuffer->nOffset;
        n_offset = pBuffer->nOffset;
    } else {
        VTEST_MSG_ERROR("Read_Buffer_From_DivX_311_File: pBuffer is NULL");
        return 0;
    }

    if (p_buffer == NULL) {
        VTEST_MSG_ERROR("Read_Buffer_From_DivX_311_File: p_buffer is NULL");
        return 0;
    }

    //Read first frame based on size
    //DivX 311 frame - 4 byte header with size followed by the frame

    bytes_read = read(m_nFileFd, &frame_size, num_bytes_size);

    if (frame_size < 0) {

        // If we read the frame size as negative number it means that it could
        // be a corrupt frame. For DIVX311 this is fatal and we can't recover.
        // Hence return from here. The caller will set error and exit the test case.

        VTEST_MSG_ERROR("Frame size read as negative number. Returning from function");
        return frame_size;
    }

    VTEST_MSG_LOW("Read_Buffer_From_DivX_311_File: Frame size = %d", frame_size);
    n_offset += read(m_nFileFd, p_buffer, frame_size);

    //the packet is ready to be sent
    VTEST_MSG_LOW("Returning Read Buffer from Divx 311: Offset=[%d]", n_offset);
    return n_offset;
}

int DecoderFileSource::ReadBufferFromVP8File(OMX_BUFFERHEADERTYPE *pBuffer) {

    char *p_buffer = NULL;
    unsigned int bytes_read = 0;
    unsigned int frame_size = 0;
    unsigned long long time_stamp;
    unsigned int n_offset = 0;

    VTEST_MSG_LOW("Inside %s \n", __FUNCTION__);

    if (pBuffer != NULL) {
        p_buffer = (char *)pBuffer->pBuffer + pBuffer->nOffset;
        n_offset = pBuffer->nOffset;
    } else {
        VTEST_MSG_ERROR("Read_Buffer_From_VP8_File: pBuffer is NULL");
        return 0;
    }

    if (p_buffer == NULL) {
        VTEST_MSG_ERROR("Read_Buffer_From_VP8_File: p_buffer is NULL");
        return 0;
    }

    bytes_read = read(m_nFileFd, &frame_size, 4);
    bytes_read = read(m_nFileFd, &time_stamp, 8);
    n_offset += read(m_nFileFd, p_buffer, frame_size);
    pBuffer->nTimeStamp = time_stamp;
    return n_offset;
}

} // namespace vtest
