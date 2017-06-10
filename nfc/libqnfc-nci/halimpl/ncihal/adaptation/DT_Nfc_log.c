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
/**
 * \file DT_Nfc_log.c
 *
 */

#define DEBUG

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <DT_Nfc_log.h>

#ifdef  ANDROID
#define LOG_TAG "NFC-NCI"

#include <utils/Log.h>

#endif

#ifdef  DEBUG
#define MAX_PRINT_BUFSIZE   (0x450U)
#endif

#define MAX_STRING_LENGTH   2000
#define MAX_TYPE_LENGTH     5

void nfc_Mgt_Recovery();
/*!
 * \brief Allocates memory.
 *        This function attempts to allocate \a size bytes on the heap and
 *        returns a pointer to the allocated block.
 *
 * \param size size of the memory block to be allocated on the heap.
 *
 * \return pointer to allocated memory block or NULL in case of error.
 */
void *nfc_GetMemory(uint32_t size)
{
    void *pMem = (void *)malloc(size);
    return pMem;
}

void *nfc_GetMoreMemory(void *pPrevMem, uint32_t Size)
{
    void *pMem = realloc( pPrevMem, (size_t) Size);
    if (pMem != NULL)
        memset(pMem, 0, Size);

    return pMem;
}

/*!
 * \brief Frees allocated memory block.
 *        This function deallocates memory region pointed to by \a pMem.
 *
 * \param pMem pointer to memory block to be freed.
 */
void nfc_FreeMemory(void *pMem)
{
    if(NULL !=  pMem)
        free(pMem);
}

void nfc_DbgString(const char *pString)
{
#ifdef DEBUG

    if(pString != NULL){

        ALOGD("%s", pString);
    }

#endif

}

/*  Where do we want to direct ouput */
eNFCLogError_t nfc_LogOut(eOutputDirected_t eWhereTo, char *pStringIn, android_LogPriority eLogPriority)
{
    FILE *pfNfcLogFile = NULL;
    eNFCLogError_t NfcLogStatus;

    if( (eWhereTo == NFC_TO_FILE)||(eWhereTo == NFC_TO_CONSOLE_PLUS_FILE)||
         (eWhereTo == NFC_TO_LOGGER_PLUS_FILE)||(eWhereTo == NFC_TO_ALL) ){
        pfNfcLogFile = fopen ("/sdcard/NFCMsgLogFile.txt","a+");
        if (pfNfcLogFile == NULL){
            NfcLogStatus = NFC_ERROR_OPEN_APPEND_FILE;
            goto done;
        }
    }

    if (eLogPriority == ANDROID_LOG_UNKNOWN) {
        NfcLogStatus = NFC_INVALID_LOG_TYPE;
        goto done;
    }

    /* Direct output to requested place */
    switch (eWhereTo){
        case NFC_TO_LOGGER:
            ALOG_NFC(eLogPriority, "%s", pStringIn);
            break;

        case NFC_TO_FILE:
            fprintf(pfNfcLogFile, "%s", pStringIn);
            fclose(pfNfcLogFile);
            pfNfcLogFile = NULL;
            break;

        case NFC_TO_LOGGER_PLUS_FILE:
            ALOG_NFC(eLogPriority, "%s", pStringIn);
            fprintf(pfNfcLogFile, "%s", pStringIn);
            fclose(pfNfcLogFile);
            pfNfcLogFile = NULL;
            break;

        /** We can filter level from nfc_DbgStringLevel, however, output to console is always the same colour
        - Add/Append loglevel in text form ?*/
        case NFC_TO_CONSOLE:
            printf("%s", pStringIn);
            break;

        case NFC_TO_LOGGER_PLUS_CONSOLE:
            ALOG_NFC(eLogPriority, "%s", pStringIn);
            printf("%s", pStringIn);
            break;

        case NFC_TO_CONSOLE_PLUS_FILE:
            printf("%s", pStringIn);
            fprintf(pfNfcLogFile, "%s", pStringIn);
            fclose(pfNfcLogFile);
            pfNfcLogFile = NULL;
            break;

        /** To Logger (Logcat), console and File */
        case NFC_TO_ALL:
            ALOG_NFC(eLogPriority, "%s", pStringIn);
            fprintf(pfNfcLogFile, "%s", pStringIn);
            printf("%s", pStringIn);
            fclose(pfNfcLogFile);
            pfNfcLogFile = NULL;
            break;

        default:
            NfcLogStatus = NFC_INVALID_DIRECTION;
            goto done;
    }
    NfcLogStatus = NFC_LOG_OK;

done:
    if (pfNfcLogFile != NULL){
            fclose(pfNfcLogFile);
            pfNfcLogFile = NULL;
        }
    return NfcLogStatus;
}
/* New Debug method with Level */
void nfc_DbgStringLevel(eOutputDirected_t eWhereTo, eLogLevel_t eThreshHold, eLogLevel_t eLevel, const char *pStringIn, ...)
{
    char    TraceBuff[MAX_STRING_LENGTH],   *pStringOut,    *s;
    void    *pPtr;
    int     i,n;
    va_list arg;

#ifdef DEBUG
    if(pStringIn != NULL){
        va_start(arg, pStringIn);
        pStringOut = &TraceBuff[0];
        memset(pStringOut, 0, MAX_STRING_LENGTH);

        while (*pStringIn !='\0'){
            if (*pStringIn != '%') {
                memcpy(pStringOut, pStringIn, 1);
            }
            else{
                switch(*++pStringIn){
                    case 'c':
                        i = va_arg(arg, int);
                        n = snprintf(pStringOut, MAX_STRING_LENGTH, "%c", i);
                        pStringOut+=(n-1);
                        break;
                    case 'd':
                        i = va_arg(arg, int);
                        n = snprintf(pStringOut, MAX_STRING_LENGTH, "%d", i);
                        pStringOut+=(n-1);
                        break;
                    case 'x':
                        i = va_arg(arg, int);
                        n = snprintf(pStringOut, MAX_STRING_LENGTH, "%x", i);
                        pStringOut+=(n-1);
                        break;
                    case 'X':
                        i = va_arg(arg, int);
                        n = snprintf(pStringOut, MAX_STRING_LENGTH, "%x", i);
                        pStringOut+=(n-1);
                        break;
                    case 's':
                        s = va_arg(arg, char *);
                        n = snprintf(pStringOut, MAX_STRING_LENGTH, "%s", s);
                        pStringOut+= (n-1);
                        break;
                    case 'p':
                        pPtr = va_arg(arg, char *);
                        n = snprintf(pStringOut, MAX_STRING_LENGTH, "%p", pPtr);
                        pStringOut+= (n-1);
                        break;
                    default:
                        n = snprintf(pStringOut, MAX_STRING_LENGTH, "%c", '%');
                        pStringOut+= (n-1);
                        break;

                }
            }
        pStringIn++;
        pStringOut++;
        }

        va_end(arg);

        pStringOut = &TraceBuff[0];

        /** Now print string based on its level and where logging threshold is set  **/
        if (eThreshHold >= eLevel){

            if (eLevel != NFC_NO_LOGGING){
                switch(eLevel){
                    case NFC_ERROR:
                        /* Error */
                        nfc_LogOut(eWhereTo, pStringOut, ANDROID_LOG_ERROR);
                        break;
                    case NFC_WARNING:
                        /* Warning */
                        nfc_LogOut(eWhereTo, pStringOut, ANDROID_LOG_WARN);
                        break;
                    case NFC_INFO:
                        /* Information */
                        nfc_LogOut(eWhereTo, pStringOut, ANDROID_LOG_INFO);
                        break;
                    case NFC_DEBUG:
                        /* Debug */
                        nfc_LogOut(eWhereTo, pStringOut, ANDROID_LOG_DEBUG);
                        break;
                    case NFC_VERBOSE:
                        /* Verbose */
                        nfc_LogOut(eWhereTo, pStringOut, ANDROID_LOG_VERBOSE);
                        break;
                    default:
                        nfc_LogOut(eWhereTo, pStringOut, ANDROID_LOG_UNKNOWN);
                        break;              }
            }
        }
    }
#endif
}
