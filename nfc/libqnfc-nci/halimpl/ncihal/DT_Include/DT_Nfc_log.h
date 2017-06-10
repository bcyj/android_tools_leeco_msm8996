/*
 * Copyright (C) 2010 NXP Semiconductors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/******************************************************************************

Copyright (c) 2013, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/
/*
 * \file  nfc.h
 * \brief
 *
 * Project: NFC-NCI
 * $Aliases: NFC_NCI
 *
 */
#ifndef NFC_H
#define NFC_H

/** \defgroup grp_nci_DT DT NCI Component
 *
 *\note:
 *
 */

#define LOGD    ALOGD
#define LOGE    ALOGE
#define LOGW    ALOGW
#define LOGI    ALOGI
#define NCI_DEBUG ALOGW
#define NCI_PRINT ALOGW

#define ALOG_NFC(priority, ...) __android_log_print(priority, LOG_TAG, __VA_ARGS__)
#define NFC_MSG(where, threshold, level, str, ...)      nfc_DbgStringLevel(where, threshold, level, str,##__VA_ARGS__)

#ifdef NFC_CUSTOMINTEGRATION
#include <nfcCustomInt.h>
#else
#include <memory.h>

/**< Message Type */
#ifdef WIN32
#define ONFC_MESSAGE_BASE  LNFC_MESSAGE_BASE
#endif




/** Define 10 Levels of Logging **/
typedef enum LogLevel{
    NFC_NO_LOGGING = 0,
    NFC_ERROR,
    NFC_WARNING,
    NFC_INFO,
    NFC_DEBUG,
    NFC_VERBOSE,
    NFC_VERBOSE_1,
    NFC_VERBOSE_2,
    NFC_VERBOSE_3,
    NFC_VERBOSE_4,
    NFC_VERBOSE_5,
    NFC_ALL_LOGGING
}eLogLevel_t;

/** Set where output is to be directed **/
/** NFC_TO_LOGGER is Logcat in case of ANDROID over ADB Interface.
 *  It will be something different depending on OS. **/
/** NFC_TO_CONSOLE is the returned directly to console application
 *  for ANDROID and could be directed elsewhere for differenr
 *  OS. **/
typedef enum OutputDirected{
    NFC_TO_LOGGER,
    NFC_TO_FILE,
    NFC_TO_LOGGER_PLUS_FILE,
    NFC_TO_CONSOLE,
    NFC_TO_LOGGER_PLUS_CONSOLE,
    NFC_TO_CONSOLE_PLUS_FILE,
    NFC_TO_ALL
}eOutputDirected_t;

typedef enum NFCLogError{
    NFC_LOG_OK,
    NFC_ERROR_OPEN_APPEND_FILE,
    NFC_INVALID_LOG_TYPE,
    NFC_INVALID_DIRECTION
}eNFCLogError_t;
/*!
 * \ingroup grp_nci_DT
 * \brief Print string
 *
 * Outputs given string to debug port.
 *
 * \param[in] pString pointer to buffer content to be displayed.
 *
 * \retval None
 */
void nfc_DbgString(const char *pString);

/*!
 * \ingroup grp_nci_DT
 * \brief Allocates some memory
 *
 * \param[in] Size   Size, in uint8_t, to be allocated
 *
 * \retval NON-NULL value:  The memory was successfully allocated ;
 * the return value points to the allocated memory location
 * \retval NULL:            The operation was not successful,
 * certainly because of insufficient resources.
 *
 */
extern void * nfc_GetMemory(uint32_t Size);
/*!
 * \ingroup grp_nci_DT
 * \brief Allocates more memory
 *
 * \param[in] pPrevMem   Pointer to the previously allocated memory which
 * you need to expand.
 * \param[in] Size       Size, in uint8_t, the new size to be allocated,
 * if it's 0 the nothing is done and the old pointer is returned.
 *
 * \retval NON-NULL value:  The memory was successfully allocated ;
 * the return value points to the allocated memory location
 * \retval NULL:            The operation was not successful,
 * certainly because of insufficient resources.
 *
 */
extern void *nfc_GetMoreMemory(void *pPrevMem, uint32_t Size);
/*!
 * \ingroup grp_nci_DT
 * \brief This API allows to free already allocated memory.
 * \param[in] pMem  Pointer to the memory block to deallocated
 * \retval None
 */
void   nfc_FreeMemory(void * pMem);
/*!
 * \ingroup grp_nci_DT
 * \brief Compares the values stored in the source memory with the
 * values stored in the destination memory.
 *
 * \param[in] src   Pointer to the Source Memory
 * \param[in] dest  Pointer to the Destination Memory
 * \param[in] n     Number of bytes to be compared.
 *
 * \retval Zero value:        The comparison was successful,
                    Both the memory areas contain the identical values.
 * \retval Non-Zero Value:    The comparison failed, both the memory
 *                  areas are non-identical.
 *
 */
int nfc_MemCompare(void *src, void *dest, unsigned int n);
/*!
 * \ingroup grp_nci_DT
 * \brief Copy the memory pointed by src to the memory location pointed by dest
 *
 * \param[in] src   Pointer to the Source Memory
 * \param[in] dest  Pointer to the Destination Memory
 * \param[in] n     Number of bytes
 *
 * \retval the pointer to the destination
 *
 */
void* nfc_MemCpy(void *dest, void *src, unsigned int n);



#endif
#endif /*  nfc_H  */
