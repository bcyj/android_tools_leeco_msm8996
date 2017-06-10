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

#include "vtest_ComDef.h"
#include "vtest_Debug.h"
#include "vtest_File.h"
#include "vt_file.h"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
File::File()
    : m_pFile(NULL),
      m_bReadOnly(OMX_TRUE) {
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
File::~File() {
    if (m_pFile) vt_file_close(m_pFile);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE File::Open(OMX_STRING pFileName, OMX_BOOL bReadOnly) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (pFileName != NULL) {
        if (m_pFile == NULL) {
            int ret;
            m_bReadOnly = bReadOnly;
            if (bReadOnly == OMX_TRUE) {
                ret = vt_file_open(&m_pFile, (char *)pFileName, 1);
            } else {
                ret = vt_file_open(&m_pFile, (char *)pFileName, 0);
            }

            if (ret != 0) {
                VTEST_MSG_ERROR("Unable to open file");
                result = OMX_ErrorUndefined;
            }
        } else {
            VTEST_MSG_ERROR("File is already open");
            result = OMX_ErrorUndefined;
        }
    } else {
        VTEST_MSG_ERROR("Null param");
        result = OMX_ErrorBadParameter;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE File::Read(OMX_U8 *pBuffer, OMX_S32 nWidth, OMX_S32 nHeight,
        OMX_S32 *pBytesRead, OMX_U32 nConfig) {

    int nBytes = nWidth;
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (m_bReadOnly == OMX_TRUE) {
        if (pBuffer != NULL) {
            if (nBytes > 0) {
                if (pBytesRead != NULL) {
                    *pBytesRead = (OMX_S32)vt_file_read(m_pFile, (void *)pBuffer, (int)nWidth, (int)nHeight, (int)nConfig);
                } else {
                    VTEST_MSG_ERROR("Null param");
                    result = OMX_ErrorBadParameter;
                }
            } else {
                VTEST_MSG_ERROR("Bytes must be > 0");
                result = OMX_ErrorBadParameter;
            }
        } else {
            VTEST_MSG_ERROR("Null param");
            result = OMX_ErrorBadParameter;
        }
    } else {
        VTEST_MSG_ERROR("File is open for writing");
        result = OMX_ErrorUndefined;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE File::Write(OMX_U8 * pBuffer, OMX_S32 nBytes, OMX_S32 *pBytesWritten) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (m_bReadOnly == OMX_FALSE) {
        if (pBuffer != NULL) {
            if (nBytes > 0) {
                if (pBytesWritten != NULL) {
                    *pBytesWritten = vt_file_write(m_pFile, (void *)pBuffer, (int)nBytes);
                } else {
                    VTEST_MSG_ERROR("Null param");
                    result = OMX_ErrorBadParameter;
                }
            } else {
                VTEST_MSG_ERROR("Bytes must be > 0");
                result = OMX_ErrorBadParameter;
            }
        } else {
            VTEST_MSG_ERROR("Null param");
            result = OMX_ErrorBadParameter;
        }
    } else {
        VTEST_MSG_ERROR("File is open for reading");
        result = OMX_ErrorUndefined;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE File::SeekStart(OMX_S32 nBytes) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (nBytes >= 0) {
        if (vt_file_seek_start(m_pFile, (int)nBytes) != 0) {
            VTEST_MSG_ERROR("failed to seek");
            result = OMX_ErrorUndefined;
        }
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE File::Close() {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (m_pFile != NULL) {
        vt_file_close(m_pFile);
        m_pFile = NULL;
    } else {
        VTEST_MSG_ERROR("File was already closed");
        result = OMX_ErrorUndefined;
    }
    return result;
}
} // namespace vtest
