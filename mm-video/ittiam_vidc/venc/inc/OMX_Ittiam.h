/*
 * Copyright (c) 2005 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 */
/******************************************************************************
*
*                                 OMX Ittiam
*
*                     ITTIAM SYSTEMS PVT LTD, BANGALORE
*                             COPYRIGHT(C) 2011
*
*  This program is proprietary to ittiam systems pvt. ltd.,and is protected
*  under indian copyright act as an unpublished work.its use and disclosure
*  is limited by the terms and conditions of a license agreement.it may not
*  be copied or otherwise  reproduced or disclosed  to persons  outside the
*  licensee's   organization  except  in  accordance   with  the  terms and
*  conditions  of such  an agreement. all copies and reproductions shall be
*  the property of ittiam systems pvt. ltd.  and  must bear this  notice in
*  its entirety.
*
******************************************************************************/

#ifndef OMX_ITTIAM_H
#define OMX_ITTIAM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 *  D E C L A R A T I O N S
 */
#define OMX_NOPORT 0xFFFFFFFE
#define OMX_TIMEOUT_SEC 4             // Timeout value in Seconds
#define OMX_TIMEOUT_MSEC 50         // Timeout value in Seconds
#define OMX_MAX_TIMEOUTS 40         // Count of Maximum number of times the component can time out
#define OMX_OSAL_Malloc malloc
#define OMX_OSAL_Free free

#undef LOG_NDDEBUG
#define LOG_NDDEBUG 0
#undef LOG_TAG
#define LOG_TAG "OMX_VENC_SW"

#include <utils/Log.h>

#define ITTIAM_LOG ALOGD
#define ITTIAM_ERROR ALOGE

// #define DEBUG_INFO

#ifdef DEBUG_INFO
#define ITTIAM_DEBUG ITTIAM_LOG
#else
#define ITTIAM_DEBUG(...)
#endif

#define ITTIAM_DEBUG ALOGV
#define INPUT_DUMP_PATH "/data/media/ittiam/input.raw"

// Color Format definitions
#ifdef ISExynos

#define IOMX_COLOR_FORMATYUV420PackedSemiPlanar32m      0x0
#define IOMX_COLOR_FormatYVU420SemiPlanar               0x0

#else

/* For 4.2.2 Nexus 4 use the following */
//#define IOMX_COLOR_FORMATYUV420PackedSemiPlanar32m              OMX_QCOM_COLOR_FormatYVU420SemiPlanar
//#define IOMX_COLOR_FormatYVU420SemiPlanar                       OMX_QCOM_COLOR_FormatYVU420SemiPlanar

/* For 4.1.2 BSP use this */
#define IOMX_COLOR_FormatAndroidOpaque							QOMX_COLOR_FormatAndroidOpaque
#define IOMX_COLOR_FormatYVU420PackedSemiPlanar32m4ka           QOMX_COLOR_FormatYVU420PackedSemiPlanar32m4ka
#define IOMX_COLOR_FORMATYUV420PackedSemiPlanar32m              QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m
#define IOMX_COLOR_FormatYVU420SemiPlanar                                 QOMX_COLOR_FormatYVU420SemiPlanar

#endif

/*
 *     D E F I N I T I O N S
 */

typedef struct _BufferList BufferList;


/*
 * The main structure for buffer management.
 *
 *   pBufHdr     - An array of pointers to buffer headers.
 *                 The size of the array is set dynamically using the nBufferCountActual value
 *                   send by the client.
 *   nListEnd    - Marker to the boundary of the array. This points to the last index of the
 *                   pBufHdr array.
 *   nSizeOfList - Count of valid data in the list.
 *   nAllocSize  - Size of the allocated list. This is equal to (nListEnd + 1) in most of
 *                   the times. When the list is freed this is decremented and at that
 *                   time the value is not equal to (nListEnd + 1). This is because
 *                   the list is not freed from the end and hence we cannot decrement
 *                   nListEnd each time we free an element in the list. When nAllocSize is zero,
 *                   the list is completely freed and the other paramaters of the list are
 *                   initialized.
 *                 If the client crashes before freeing up the buffers, this parameter is
 *                   checked (for nonzero value) to see if there are still elements on the list.
 *                   If yes, then the remaining elements are freed.
 *    nWritePos  - The position where the next buffer would be written. The value is incremented
 *                   after the write. It is wrapped around when it is greater than nListEnd.
 *    nReadPos   - The position from where the next buffer would be read. The value is incremented
 *                   after the read. It is wrapped around when it is greater than nListEnd.
 *    eDir       - Type of BufferList.
 *                            OMX_DirInput  =  Input  Buffer List
 *                           OMX_DirOutput  =  Output Buffer List
 */
struct _BufferList{
   OMX_BUFFERHEADERTYPE **pBufHdr;
   OMX_BUFFERHEADERTYPE **pBufHdr_dyn;
   OMX_U32 nListEnd;
   OMX_U32 nSizeOfList;
   OMX_U32 nAllocSize;
   OMX_U32 nWritePos;
   OMX_U32 nReadPos;
   OMX_BOOL *bBufOwner;
   OMX_DIRTYPE eDir;
};

void* ComponentThread(void* pThreadData);


/*
 * Enumeration for the commands processed by the component
 */

typedef enum ThrCmdType
{
    SetState,
    Flush,
    DisablePort,
    EnablePort,
    MarkBuf,
    StopThread,
    FillBuf,
    EmptyBuf
} ThrCmdType;


/*
 *     M A C R O S
 */



/*
 * Initializes a data structure using a pointer to the structure.
 * The initialization of OMX structures always sets up the nSize and nVersion fields
 *   of the structure.
 */
#define OMX_CONF_INIT_STRUCT_PTR(_s_, _name_)   \
    memset((_s_), 0x0, sizeof(_name_)); \
    (_s_)->nSize = sizeof(_name_);      \
    (_s_)->nVersion.s.nVersionMajor = 0x1;  \
    (_s_)->nVersion.s.nVersionMinor = 0x0;  \
    (_s_)->nVersion.s.nRevision = 0x0;      \
    (_s_)->nVersion.s.nStep = 0x0



/*
 * Checking for version compliance.
 * If the nSize of the OMX structure is not set, raises bad parameter error.
 * In case of version mismatch, raises a version mismatch error.
 */
#define OMX_CONF_CHK_VERSION(_s_, _name_, _e_)              \
    if((_s_)->nSize != sizeof(_name_)) _e_ = OMX_ErrorBadParameter; \
    if(((_s_)->nVersion.s.nVersionMajor != 0x1)||           \
       ((_s_)->nVersion.s.nVersionMinor != 0x0)||           \
       ((_s_)->nVersion.s.nRevision != 0x0)||               \
       ((_s_)->nVersion.s.nStep != 0x0)) _e_ = OMX_ErrorVersionMismatch;\
    if(_e_ != OMX_ErrorNone) goto OMX_CONF_CMD_BAIL;



/*
 * Checking paramaters for non-NULL values.
 * The macro takes three parameters because inside the code the highest
 *   number of parameters passed for checking in a single instance is three.
 * In case one or two parameters are passed, the ramaining parameters
 *   are set to 1 (or a nonzero value).
 */
#define OMX_CONF_CHECK_CMD(_ptr1, _ptr2, _ptr3) \
{                       \
    if(!_ptr1 || !_ptr2 || !_ptr3){     \
        eError = OMX_ErrorBadParameter;     \
    goto OMX_CONF_CMD_BAIL;         \
    }                       \
}



/*
 * Redirects control flow in an error situation.
 * The OMX_CONF_CMD_BAIL label is defined inside the calling function.
 */
#define OMX_CONF_BAIL_IF_ERROR(_eError)     \
{                       \
    if(_eError != OMX_ErrorNone)        \
        goto OMX_CONF_CMD_BAIL;         \
}



/*
 * Sets error type and redirects control flow to error handling and cleanup section
 */
#define OMX_CONF_SET_ERROR_BAIL(_eError, _eCode)\
{                       \
    _eError = _eCode;               \
    goto OMX_CONF_CMD_BAIL;         \
}




/*
 * Allocates a new entry in a BufferList.
 * Finds the position where memory has to be allocated.
 * Actual allocation happens in the caller function.
 */
#define ListAllocate(_pH, _nIndex)              \
   if ((WORD32)_pH.nListEnd == -1){                     \
      _pH.nListEnd = 0;                         \
      _pH.nWritePos = 0;                        \
      }                                         \
   else                                         \
   _pH.nListEnd++;                              \
   _pH.nAllocSize++;                            \
   _nIndex = _pH.nListEnd




/*
 * Sets an entry in the BufferList.
 * The entry set is a BufferHeader.
 * The nWritePos value is incremented after the write.
 * It is wrapped around when it is greater than nListEnd.
 */
#define ListSetEntry(_pH, _pB)                  \
   if (_pH.nSizeOfList < (_pH.nListEnd + 1)){   \
      _pH.nSizeOfList++;                        \
      _pH.pBufHdr_dyn[_pH.nWritePos++] = _pB;       \
      if ((WORD32)_pH.nReadPos == -1)                   \
         _pH.nReadPos = 0;                      \
      if (_pH.nWritePos > _pH.nListEnd)         \
         _pH.nWritePos = 0;                     \
      }




/*
 * Gets an entry from the BufferList
 * The entry is a BufferHeader
 * The nReadPos value is incremented after the read.
 * It is wrapped around when it is greater than nListEnd.
 */
#define ListGetEntry(_pH, _pB)                  \
   if (_pH.nSizeOfList > 0){                    \
      _pH.nSizeOfList--;                        \
      _pB = _pH.pBufHdr_dyn[_pH.nReadPos++];        \
      if (_pH.nReadPos > _pH.nListEnd)          \
         _pH.nReadPos = 0;                      \
      }



/*
 * Flushes all entries from the BufferList structure.
 * The nSizeOfList gives the number of valid entries in the list.
 * The nReadPos value is incremented after the read.
 * It is wrapped around when it is greater than nListEnd.
 */
#define ListFlushEntries(_pH, _pC)              \
    while (_pH.nSizeOfList > 0){                \
       _pH.nSizeOfList--;                       \
       if (_pH.eDir == OMX_DirInput) {           \
          _pC->pCallbacks->EmptyBufferDone(_pC->hSelf,_pC->pAppData,_pH.pBufHdr_dyn[_pH.nReadPos++]);}\
       else if (_pH.eDir == OMX_DirOutput){      \
                pVidEnc->NumFBD++; \
          _pC->pCallbacks->FillBufferDone(_pC->hSelf,_pC->pAppData,_pH.pBufHdr_dyn[_pH.nReadPos++]);}\
       if (_pH.nReadPos > _pH.nListEnd)         \
          _pH.nReadPos = 0;                     \
      }


/*
 * Frees the memory allocated for BufferList entries
 *   by comparing with client supplied buffer header.
 * The nAllocSize value gives the number of allocated (i.e. not free'd) entries in the list.
 * When nAllocSize is zero, the list is completely freed
 *   and the other paramaters of the list are initialized.
 */
#define ListFreeBuffer(_pH, _pB, _pP, _nIndex)                       \
    for (_nIndex = 0; _nIndex <= _pH.nListEnd; _nIndex++){           \
        if (_pH.pBufHdr[_nIndex] == _pB){                            \
           _pH.nAllocSize--;                                         \
           if (_pH.pBufHdr[_nIndex]){                                \
              if (_pH.pBufHdr[_nIndex]->pBuffer){                    \
                 if (_pH.bBufOwner[_nIndex])                         \
                    OMX_OSAL_Free(_pH.pBufHdr[_nIndex]->pBuffer);    \
                 _pH.pBufHdr[_nIndex]->pBuffer = NULL;}              \
              OMX_BUFFERHEADERTYPE *bufhdr = (OMX_BUFFERHEADERTYPE *)_pH.pBufHdr[_nIndex]; \
              OMX_OSAL_Free(bufhdr);                                 \
              _pH.pBufHdr[_nIndex] = NULL;                           \
              }                                                      \
           if (_pH.nAllocSize == 0){                                 \
              _pH.nWritePos = -1;                                    \
              _pH.nReadPos = -1;                                     \
              _pH.nListEnd = -1;                                     \
              _pH.nSizeOfList = 0;                                   \
              _pP->bPopulated = OMX_FALSE;                           \
              }                                                      \
           break;                                                    \
           }                                                         \
        }


/*
 * Frees the memory allocated for BufferList entries.
 * This is called in case the client crashes suddenly before freeing all the component buffers.
 * The nAllocSize parameter is
 *   checked (for nonzero value) to see if there are still elements on the list.
 * If yes, then the remaining elements are freed.
 */
#define ListFreeAllBuffers(_pH, _nIndex)                             \
    for (_nIndex = 0; _nIndex <= _pH.nListEnd; _nIndex++){           \
        if (_pH.pBufHdr[_nIndex]){                                   \
           _pH.nAllocSize--;                                         \
           if (_pH.pBufHdr[_nIndex]->pBuffer){                       \
              if (_pH.bBufOwner[_nIndex])                            \
                 OMX_OSAL_Free(_pH.pBufHdr[_nIndex]->pBuffer);       \
              _pH.pBufHdr[_nIndex]->pBuffer = NULL;}                 \
           OMX_BUFFERHEADERTYPE *bufhdr = (OMX_BUFFERHEADERTYPE *)_pH.pBufHdr[_nIndex]; \
           OMX_OSAL_Free(bufhdr);                                    \
           _pH.pBufHdr[_nIndex] = NULL;                              \
           if (_pH.nAllocSize == 0){                                 \
              _pH.nWritePos = -1;                                    \
              _pH.nReadPos = -1;                                     \
              _pH.nListEnd = -1;                                     \
              _pH.nSizeOfList = 0;                                   \
              break;                             \
              }                                                      \
           }                                                         \
        }



/*
 * Loads the parameters of the buffer header.
 * When the list has nBufferCountActual elements allocated
 *   then the bPopulated value of port definition is set to true.
 */
#define LoadBufferHeader(_pList, _pBufHdr, _pAppPrivate, _nSizeBytes, _nPortIndex,    \
                                                            _ppBufHdr, _pPortDef)     \
    _pBufHdr->nAllocLen = _nSizeBytes;                                                \
    _pBufHdr->pAppPrivate = _pAppPrivate;                                             \
    if (_pList.eDir == OMX_DirInput){                                                 \
       _pBufHdr->nInputPortIndex = _nPortIndex;                                       \
       _pBufHdr->nOutputPortIndex = OMX_NOPORT;                                       \
       }                                                                              \
    else{                                                                             \
       _pBufHdr->nInputPortIndex = OMX_NOPORT;                                        \
       _pBufHdr->nOutputPortIndex = _nPortIndex;                                      \
       }                                                                              \
    _ppBufHdr = _pBufHdr;                                                             \
    if (_pList.nListEnd == (_pPortDef->nBufferCountActual - 1))                       \
       _pPortDef->bPopulated = OMX_TRUE

static void millisleep(int milliseconds)
{
      usleep(milliseconds * 1000);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* OMX_ITTIAM_H */

