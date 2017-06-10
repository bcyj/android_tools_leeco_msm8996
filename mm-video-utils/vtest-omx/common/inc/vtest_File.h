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

#ifndef _VTEST_FILE_H
#define _VTEST_FILE_H

#include "OMX_Core.h"

namespace vtest {

/**
 * @brief Class for reading from and writing to files
 */
class File {

public:

    /**
     * @brief Constructor
     */
    File();

    /**
     * @brief Destructor
     */
    ~File();

    /**
     * @brief Opens the file in read or write mode
     *
     * @param pFileName The name of the file
     * @param bReadOnly Set to OMX_TRUE for read access, OMX_FALSE for write access.
     */
    OMX_ERRORTYPE Open(OMX_STRING pFileName, OMX_BOOL bReadOnly);

    /**
     * @brief Reads the file. Only valid in read mode.
     *
     * @param pBuffer The buffer to read into
     * @param nBytes The number of bytes to read
     * @param pBytesRead The number of bytes actually read (output)
     */
    OMX_ERRORTYPE Read(OMX_U8 *pBuffer, OMX_S32 nWidth,
                       OMX_S32 nHeight, OMX_S32 *pBytesRead, OMX_U32 nConfig);
    /**
     * @brief Writes to the file. Only valid in write mode.
     *
     * @param pBuffer The buffer to write from
     * @param nBytes The number of bytes to write
     * @param pBytesWritten The number of bytes actually written (output)
     */
    OMX_ERRORTYPE Write(OMX_U8 *pBuffer, OMX_S32 nBytes, OMX_S32 *pBytesWritten);

    /**
     * @brief Reposition the file pointer.
     *
     * @param nBytes The number of bytes from the start of file
     */
    OMX_ERRORTYPE SeekStart(OMX_S32 nBytes);

    /**
     * @brief Closes the file
     */
    OMX_ERRORTYPE Close();

private:
    void *m_pFile;
    OMX_BOOL m_bReadOnly;
};

}

#endif // #ifndef _VTEST_FILE_H
