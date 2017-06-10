/*===========================================================================
    FILE:           acdb_parser.c

    OVERVIEW:       This file is the parser for the Acdb file. This is an
                    optimization from the previous parser to not use so
                    much ARM9 heap at any given time. This implementation
                    will use the stack from the context of the caller.

    DEPENDENCIES:   None

                    Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb_includes.h"
#include "acdb_parser.h"
#include "acdb_init.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

#define ACDB_PARSE_FAILURE -10
#define ACDB_GENERAL_FAILURE -1
#define ACDB_SUCCESS 0

// Warning: This will define how much ARM9 heap the parser will use. The
// assumption is that there will be no call to getNextAcdbData (...) with
// more bytes than what is specified below. If there is, then there may be
// an issue in the current implementation (getNextAcdbData will not
// repeatedly query the FS for data. It assumes only one query is necessary
// to satisfy any request).
//#define ACDB_READ_BUFFER_SIZE 8192
#define ACDB_READ_BUFFER_SIZE 4096
//#define ACDB_READ_BUFFER_SIZE 1024
//#define ACDB_READ_BUFFER_SIZE 512

#define ACDB_FCC_RIFF   0x46464952
#define ACDB_FCC_LIST   0x5453494C
#define ACDB_FCC_ACDB   0x42444341
#define ACDB_FCC_VERS   0x53524556
#define ACDB_FCC_DDB    0x20424444
#define ACDB_FCC_DIOC   0x434F4944
#define ACDB_FCC_HDR    0x20524448
#define ACDB_FCC_DATE   0x45544144
#define ACDB_FCC_OEMI   0x494D454F
#define ACDB_FCC_DLST   0x54534C44
#define ACDB_FCC_DINF   0x464E4944
#define ACDB_FCC_DIN2   0x324E4944
#define ACDB_FCC_TGTV   0x56544754
#define ACDB_FCC_THDR   0x52444854
#define ACDB_FCC_TDEF   0x46454454
#define ACDB_FCC_VPTB   0x42545056
#define ACDB_FCC_VPIO   0x4F495056
#define ACDB_FCC_VSTB   0x42545356
#define ACDB_FCC_VSIO   0x4F495356
#define ACDB_FCC_APTB   0x42545041
#define ACDB_FCC_APIO   0x4F495041
#define ACDB_FCC_ASTB   0x42545341
#define ACDB_FCC_ASIO   0x4F495341
#define ACDB_FCC_VVTB   0x42545656
#define ACDB_FCC_VVIO   0x4F495656
#define ACDB_FCC_AVTB   0x42545641
#define ACDB_FCC_AVIO   0x4F495641
#define ACDB_FCC_ADPTB  0x42544441
#define ACDB_FCC_ADPIO  0x4F494441
#define ACDB_FCC_ADATB  0x42544E41
#define ACDB_FCC_ADAIO  0x4F494E41
#define ACDB_FCC_AFTB   0x42544641
#define ACDB_FCC_AFIO   0x4F494641
#define ACDB_FCC_GBTB   0x32544247
#define ACDB_FCC_GBIO   0x32444247
#define ACDB_FCC_AFT2   0x32544641
#define ACDB_FCC_AFI2   0x32494641

#define ACDB_FIELD_SIZE          4

#define ACDB_MAX_BUFFER_SIZE 2048

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

typedef struct _AcdbParseContext
{
   void                    *pHandle;
   AcdbParseCBFuncPtrType  pfnCallback;

   uint32_t nFileOffset;
   uint32_t nFileSize;

   // Memory used to manage the file i/o as we read the ACDB file a segment
   // at a time.
   uint8_t *pCache;
   uint32_t nCacheOffset;
   uint32_t nCacheEnd;

   // Memory malloced once and re-used whenever the parser needs to deliver
   // content back to it's client through the callback.
   uint8_t *pBuffer;
   uint32_t nBufferSize;

   uint8_t bExitParser;
   uint8_t bParseForTargetVersionOnly;
} AcdbParseContext;


typedef struct _AcdbRiffHeader
{
   uint32_t id;
   uint32_t length;
   uint32_t type;
} AcdbRiffHeader;

typedef struct _AcdbChunkHeader
{
   uint32_t id;
   uint32_t length;
} AcdbChunkHeader;

/* ---------------------------------------------------------------------------
 * Global Data Definitions
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Static Variable Definitions
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 *--------------------------------------------------------------------------- */

static int32_t InitAcdbContext (AcdbParseContext* pContext)
{
   int32_t result = ACDB_GENERAL_FAILURE;
   memset(pContext, 0, sizeof (AcdbParseContext));

   pContext->pCache = ACDB_MALLOC (ACDB_READ_BUFFER_SIZE);
   if (pContext->pCache)
   {
      memset (pContext->pCache, 0, ACDB_READ_BUFFER_SIZE);

      pContext->pBuffer = ACDB_MALLOC (ACDB_MAX_BUFFER_SIZE);
      if (pContext->pBuffer)
      {
         memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
         pContext->nBufferSize = ACDB_MAX_BUFFER_SIZE;

      result = ACDB_SUCCESS;
   }
   }

   return result;
}

static void DeinitAcdbContext (AcdbParseContext* pContext)
{
   if (pContext->pCache != NULL)
   {
      ACDB_MEM_FREE (pContext->pCache);
      pContext->pCache = NULL;
   }

   if (pContext->pBuffer != NULL)
   {
      ACDB_MEM_FREE (pContext->pBuffer);
      pContext->pBuffer = NULL;
   }
}

static int32_t getNextAcdbData (AcdbParseContext* pContext,
                              uint8_t* pDst,
                              uint32_t bytesToRead)
{
   int32_t result = ACDB_GENERAL_FAILURE;
   AcdbParseGetDataType data;
   uint32_t bytesLeft = pContext->nCacheEnd - pContext->nCacheOffset;

   // There are three use cases:
   // 1) If all the data requested exists in the buffer
   // 2) If half of the data exists in the buffer
   // 3) If no data exists in the buffer
   if (bytesLeft == 0)
   {
      data.pBuffer = pContext->pCache;
      data.position = pContext->nFileOffset;
      data.bytesToRead = ACDB_READ_BUFFER_SIZE;
      data.bytesRead = 0;
      result = pContext->pfnCallback (ACDB_GET_DATA, pContext->pHandle, &data);
      if (result != ACDB_SUCCESS )
      {
         //error
      }
      else if ( data.bytesToRead != data.bytesRead )
      {
         memcpy (pDst, pContext->pCache, bytesToRead);

         pContext->nCacheOffset = bytesToRead;
         pContext->nCacheEnd = data.bytesRead;
         pContext->nFileOffset += data.bytesRead;
         result = ACDB_SUCCESS;
      }
      else
      {
         memcpy (pDst, pContext->pCache, bytesToRead);

         pContext->nCacheOffset = bytesToRead;
         pContext->nCacheEnd = data.bytesRead;
         pContext->nFileOffset += data.bytesRead;
         result = ACDB_SUCCESS;
      }
   }
   else if (bytesLeft < bytesToRead)
   {
      // copy the remainder of the existing buffer into the destination.
      memcpy (pDst, pContext->pCache + pContext->nCacheOffset, bytesLeft);

      // read the entire data buffer.
      data.pBuffer = pContext->pCache;
      data.position = pContext->nFileOffset;
      data.bytesToRead = ACDB_READ_BUFFER_SIZE;
      data.bytesRead = 0;
      result = pContext->pfnCallback (ACDB_GET_DATA, pContext->pHandle, &data);
      if (result != ACDB_SUCCESS)
      {
         //error
      }
      else if (data.bytesToRead != data.bytesRead)
      {
         // copy the reminder of the requested buffer into the destination.
         memcpy (pDst + bytesLeft, pContext->pCache, bytesToRead - bytesLeft);

         pContext->nCacheOffset = bytesToRead - bytesLeft;
         pContext->nCacheEnd = data.bytesRead;
         pContext->nFileOffset += data.bytesRead;
         result = ACDB_SUCCESS;
      }
      else
      {
         // copy the reminder of the requested buffer into the destination.
         memcpy (pDst + bytesLeft, pContext->pCache, bytesToRead - bytesLeft);

         pContext->nCacheOffset = bytesToRead - bytesLeft;
         pContext->nCacheEnd = data.bytesRead;
         pContext->nFileOffset += data.bytesRead;
         result = ACDB_SUCCESS;
      }
   }
   else
   {
      // copy the requested bytes into the destination buffer.
      memcpy (pDst, pContext->pCache + pContext->nCacheOffset, bytesToRead);
      pContext->nCacheOffset += bytesToRead;
      result = ACDB_SUCCESS;
   }

   return result;
}

static int32_t skipUnknownChunks (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint8_t ignore = 0;
   uint32_t i = 0;
   for (; i < pHdr->length; ++i)
   {
      (void) getNextAcdbData (pContext, &ignore, sizeof (uint8_t));
   }
   return result;
}

static int32_t skipUnknownListChunks (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint8_t ignore = 0;
   // Start ignoring bytes after the acdb list type id.
   uint32_t i = ACDB_FIELD_SIZE;
   for (; i < pHdr->length; ++i)
   {
      (void) getNextAcdbData (pContext, &ignore, sizeof (uint8_t));
   }
   return result;
}

static int32_t parseVersion (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   AcdbParseFileVersionType data;

   if (pHdr->length < 4 * sizeof (uint16_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.major, sizeof (uint16_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.minor, sizeof (uint16_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.build, sizeof (uint16_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.revision, sizeof (uint16_t));
      (void) pContext->pfnCallback (ACDB_FILE_VERSION, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseDateModified (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   AcdbParseDateModifiedType data;
   memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);

   data.dateStringLength = pHdr->length + 1;
   data.pDateString = (char *)pContext->pBuffer;
   (void) getNextAcdbData (pContext, (uint8_t*)data.pDateString, pHdr->length);
   (void) pContext->pfnCallback (ACDB_DATE_MODIFIED, pContext->pHandle, &data);

   return result;
}

static int32_t parseOemInformation (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   AcdbParseOemInformationType data;
   memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);

   data.oemInformationLength = pHdr->length + 1;
   data.pOemInformation = (char*) pContext->pBuffer;
   (void) getNextAcdbData (pContext, (uint8_t*)data.pOemInformation, pHdr->length);
   (void) pContext->pfnCallback (ACDB_OEM_INFORMATION, pContext->pHandle, &data);

   return result;
}

static int32_t parseTargetVersion (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   AcdbParseTargetVersionType data;

   if (pHdr->length < sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.targetversion, sizeof (uint32_t));
      (void) pContext->pfnCallback (ACDB_TARGET_VERSION, pContext->pHandle, &data);
   }

   if (pContext->bParseForTargetVersionOnly == TRUE)
   {
      pContext->bExitParser = TRUE;
   }

   return result;
}

static int32_t parseTableDefinition (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   AcdbParseTableDefinitionType data;
   uint32_t i = 0;

   memset (&data, 0, sizeof (data));

   if (pHdr->length < 3 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nTableId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nTableDataId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nTableIndexCount, sizeof (uint32_t));
      if (data.nTableIndexCount < ACDB_PARSE_INDEX_MAX_SIZE)
      {    
         for (i = 0; i < data.nTableIndexCount; ++i)
         {
            (void) getNextAcdbData (pContext, (uint8_t*) &data.nTableIndexType [i], sizeof (uint32_t));
         }

         (void) pContext->pfnCallback (ACDB_TABLE_DEFINITION, pContext->pHandle, &data);
      }
      else
      {
         ACDB_DEBUG_LOG("[ACDB parser]->parseTableDefinition->nTableIndexCount is Greater max size allowed\n");
         result = ACDB_PARSE_FAILURE;
      }
   }

   return result;
}

static int32_t parseTableHeader (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));
   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_TDEF:
         (void) parseTableDefinition (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseHeader (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;
   uint32_t listType = 0;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));
   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_DATE:
         (void) parseDateModified (pContext, &chunk);
         break;
      case ACDB_FCC_OEMI:
         (void) parseOemInformation (pContext, &chunk);
         break;
      case ACDB_FCC_TGTV:
         (void) parseTargetVersion (pContext, &chunk);
         break;
      case ACDB_FCC_LIST:
         (void) getNextAcdbData (pContext, (uint8_t*)&listType, ACDB_FIELD_SIZE);
         switch (listType)
         {
         case ACDB_FCC_THDR:
            (void) parseTableHeader (pContext, &chunk);
            break;
         default:
            (void) skipUnknownChunks (pContext, &chunk);
            break;
         }
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseDeviceInformation (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_GENERAL_FAILURE;

   AcdbParseDeviceInformationType data;
   memset (&data, 0, sizeof (data));
   if (pHdr->length < 2 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nDeviceId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nParamId, sizeof (uint32_t));

      memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
      data.nBufferSize = pHdr->length - 2*sizeof (uint32_t);
      data.pBuffer = pContext->pBuffer;
      (void) getNextAcdbData (pContext, data.pBuffer, data.nBufferSize);

      (void) pContext->pfnCallback (ACDB_DEVICE_INFORMATION, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseDeviceList (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));

   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_DINF:
         (void) parseDeviceInformation (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseVocProcCmnData (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_GENERAL_FAILURE;

   AcdbParseVocProcCmnDataType data;
   memset (&data, 0, sizeof (data));
   if (pHdr->length < 8 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nTxDeviceId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nRxDeviceId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nTxDeviceSampleRateId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nRxDeviceSampleRateId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nNetworkId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nVocProcSampleRateId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nModuleId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nParamId, sizeof (uint32_t));

      memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
      data.nBufferSize = pHdr->length - 8*sizeof (uint32_t);
      data.pBuffer = pContext->pBuffer;
      (void) getNextAcdbData (pContext, data.pBuffer, data.nBufferSize);

      (void) pContext->pfnCallback (ACDB_VOC_PROC_CMN_DATA, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseVocProcCmnTable (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));

   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_VPIO:
         (void) parseVocProcCmnData (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseVocProcStrmData (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_GENERAL_FAILURE;

   AcdbParseVocProcStreamDataType data;
   memset (&data, 0, sizeof (data));
   if (pHdr->length < 4 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nNetworkId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nVocProcSampleRateId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nModuleId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nParamId, sizeof (uint32_t));

      memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
      data.nBufferSize = pHdr->length - 4*sizeof (uint32_t);
      data.pBuffer = pContext->pBuffer;
      (void) getNextAcdbData (pContext, data.pBuffer, data.nBufferSize);

      (void) pContext->pfnCallback (ACDB_VOC_PROC_STRM_DATA, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseVocProcStrmTable (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));

   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_VSIO:
         (void) parseVocProcStrmData (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseAudProcCmnData (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_GENERAL_FAILURE;

   AcdbParseAudProcCmnDataType data;
   memset (&data, 0, sizeof (data));
   if (pHdr->length < 5 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nDeviceId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nDeviceSampleRateId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nApplicationTypeId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nModuleId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nParamId, sizeof (uint32_t));

      memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
      data.nBufferSize = pHdr->length - 5*sizeof (uint32_t);
      data.pBuffer = pContext->pBuffer;
      (void) getNextAcdbData (pContext, data.pBuffer, data.nBufferSize);

      (void) pContext->pfnCallback (ACDB_AUD_PROC_CMN_DATA, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseAudProcCmnTable (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));

   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_APIO:
         (void) parseAudProcCmnData (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseAudProcStrmData (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_GENERAL_FAILURE;

   AcdbParseAudProcStreamDataType data;
   memset (&data, 0, sizeof (data));
   if (pHdr->length < 4 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nDeviceId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nApplicationTypeId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nModuleId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nParamId, sizeof (uint32_t));

      memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
      data.nBufferSize = pHdr->length - 4*sizeof (uint32_t);
      data.pBuffer = pContext->pBuffer;
      (void) getNextAcdbData (pContext, data.pBuffer, data.nBufferSize);

      (void) pContext->pfnCallback (ACDB_AUD_PROC_STRM_DATA, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseAudProcStrmTable (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));

   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_ASIO:
         (void) parseAudProcStrmData (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseVocProcVolumeData (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_GENERAL_FAILURE;

   AcdbParseVocProcVolDataType data;
   memset (&data, 0, sizeof (data));
   if (pHdr->length < 7 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nTxDeviceId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nRxDeviceId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nNetworkId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nVocProcSampleRateId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nVolumeIndex, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nModuleId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nParamId, sizeof (uint32_t));

      memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
      data.nBufferSize = pHdr->length - 7*sizeof (uint32_t);
      data.pBuffer = pContext->pBuffer;
      (void) getNextAcdbData (pContext, data.pBuffer, data.nBufferSize);

      (void) pContext->pfnCallback (ACDB_VOC_PROC_VOL_DATA, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseVocProcVolumeTable (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));

   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_VVIO:
         (void) parseVocProcVolumeData (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseAudProcVolumeData (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_GENERAL_FAILURE;

   AcdbParseAudProcVolDataType data;
   memset (&data, 0, sizeof (data));
   if (pHdr->length < 5 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nDeviceId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nApplicationTypeId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nVolumeIndex, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nModuleId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nParamId, sizeof (uint32_t));

      memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
      data.nBufferSize = pHdr->length - 5*sizeof (uint32_t);
      data.pBuffer = pContext->pBuffer;
      (void) getNextAcdbData (pContext, data.pBuffer, data.nBufferSize);

      (void) pContext->pfnCallback (ACDB_AUD_PROC_VOL_DATA, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseAudProcVolumeTable (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));

   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_AVIO:
         (void) parseAudProcVolumeData (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseAdieProfileData (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_GENERAL_FAILURE;
   AcdbParseAdieProfileDataType data;
   memset (&data, 0, sizeof (data));
   if (pHdr->length < 4 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nCodecPathId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nFrequencyId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nOverSampleRateId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nParamId, sizeof (uint32_t));

      memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
      data.nBufferSize = pHdr->length - 4*sizeof (uint32_t);
      data.pBuffer = pContext->pBuffer;
      (void) getNextAcdbData (pContext, data.pBuffer, data.nBufferSize);

      (void) pContext->pfnCallback (ACDB_ADIE_PROFILE_DATA, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseAdieProfileTable (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));

   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_ADPIO:
         (void) parseAdieProfileData (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseAdieANCData (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_GENERAL_FAILURE;
   AcdbParseAdieANCDataType data;
   memset (&data, 0, sizeof (data));
   if (pHdr->length < 4 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nDeviceId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nFrequencyId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nOverSampleRateId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nParamId, sizeof (uint32_t));

      memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
      data.nBufferSize = pHdr->length - 4*sizeof (uint32_t);
      data.pBuffer = pContext->pBuffer;
      (void) getNextAcdbData (pContext, data.pBuffer, data.nBufferSize);

      (void) pContext->pfnCallback (ACDB_ADIE_ANC_CONFIG_DATA, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseAdieANCTable (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));

   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_ADAIO:
         (void) parseAdieANCData (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseAFEData (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_GENERAL_FAILURE;

   AcdbParseAfeDataType data;
   memset (&data, 0, sizeof (data));
   if (pHdr->length < 4 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nTxDeviceId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nRxDeviceId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nModuleId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nParamId, sizeof (uint32_t));

      memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
      data.nBufferSize = pHdr->length - 4*sizeof (uint32_t);
      data.pBuffer = pContext->pBuffer;
      (void) getNextAcdbData (pContext, data.pBuffer, data.nBufferSize);

      (void) pContext->pfnCallback (ACDB_AFE_DATA, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseAFETable (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));

   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_AFIO:
         (void) parseAFEData (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseGenericData (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_GENERAL_FAILURE;

   AcdbParseGlbTblDataType data;
   memset (&data, 0, sizeof (data));
   if (pHdr->length < 4 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nModuleId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nParamId, sizeof (uint32_t));

      memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
      data.nBufferSize = pHdr->length - 2*sizeof (uint32_t);
      data.pBuffer = pContext->pBuffer;
      (void) getNextAcdbData (pContext, data.pBuffer, data.nBufferSize);

      (void) pContext->pfnCallback (ACDB_GLBTBL_DATA, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseGenericTable (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));

   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_GBIO:
         (void) parseGenericData (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}

static int32_t parseAFECmnData (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_GENERAL_FAILURE;

   AcdbParseAfeCmnDataType data;
   memset (&data, 0, sizeof (data));
   if (pHdr->length < 4 * sizeof (uint32_t))
   {
      result = ACDB_GENERAL_FAILURE;
   }
   else
   {
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nDeviceId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nAfeSampleRateId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nModuleId, sizeof (uint32_t));
      (void) getNextAcdbData (pContext, (uint8_t*) &data.nParamId, sizeof (uint32_t));

      memset (pContext->pBuffer, 0, ACDB_MAX_BUFFER_SIZE);
      data.nBufferSize = pHdr->length - 4*sizeof (uint32_t);
      data.pBuffer = pContext->pBuffer;
      (void) getNextAcdbData (pContext, data.pBuffer, data.nBufferSize);

      (void) pContext->pfnCallback (ACDB_AFE_CMN_DATA, pContext->pHandle, &data);
   }

   return result;
}

static int32_t parseAFECmnTable (AcdbParseContext* pContext, AcdbChunkHeader* pHdr)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t offset = ACDB_FIELD_SIZE;
   uint32_t end = pHdr->length;

   AcdbChunkHeader   chunk;
   memset(&chunk,0,sizeof(AcdbChunkHeader));

   while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
   {
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
      offset += 2*ACDB_FIELD_SIZE;

      switch (chunk.id)
      {
      case ACDB_FCC_AFI2:
         (void) parseAFECmnData (pContext, &chunk);
         break;
      default:
         (void) skipUnknownChunks (pContext, &chunk);
         break;
      }

      offset += chunk.length;
      if ((chunk.length&1) != 0)
      {
         uint8_t ignore = 0;
         offset ++;
         (void) getNextAcdbData (pContext, &ignore, 1);
      }
   }

   return result;
}


static int32_t parseAcdbFile (AcdbParseContext* pContext)
{
   int32_t result = ACDB_SUCCESS;
   AcdbParseGetFileSizeType data;

   result = pContext->pfnCallback (ACDB_GET_FILE_SIZE, pContext->pHandle, &data);
   pContext->nFileSize = data.size;

   if (result != ACDB_SUCCESS)
   {
      return result;
   }

   if (result == 0)
   {
      uint32_t offset = 0;
      uint32_t end = pContext->nFileSize;

      AcdbRiffHeader hdr;
      memset(&hdr,0,sizeof(AcdbRiffHeader));

      (void) getNextAcdbData (pContext, (uint8_t*) &hdr.id, ACDB_FIELD_SIZE);
      (void) getNextAcdbData (pContext, (uint8_t*) &hdr.length, ACDB_FIELD_SIZE);
      
      if (hdr.length + 2*sizeof(uint32_t) != end)
      {
         ACDB_DEBUG_LOG("[ACDB Parser ERROR]->Actual file size and ACDB Data content size mismatch!\n"
                        "ACDB file in file system is likely corrupted\n");
         return ACDB_FAIL;
      }
      else
      {
         (void) getNextAcdbData (pContext, (uint8_t*) &hdr.type, ACDB_FIELD_SIZE);
         offset += 3*ACDB_FIELD_SIZE;
      }
      
      if (ACDB_FCC_RIFF == hdr.id && ACDB_FCC_ACDB == hdr.type)
      {
         AcdbChunkHeader   chunk;
         uint32_t            listType;

         memset(&chunk,0,sizeof(AcdbChunkHeader));
         while (offset < end && result == ACDB_SUCCESS && pContext->bExitParser == FALSE)
         {
            (void) getNextAcdbData (pContext, (uint8_t*)&chunk.id, ACDB_FIELD_SIZE);
            (void) getNextAcdbData (pContext, (uint8_t*)&chunk.length, ACDB_FIELD_SIZE);
            offset += 2*ACDB_FIELD_SIZE;

            switch (chunk.id)
            {
            case ACDB_FCC_VERS:
               (void) parseVersion (pContext, &chunk);
               break;
            case ACDB_FCC_LIST:
               (void) getNextAcdbData (pContext, (uint8_t*)&listType, ACDB_FIELD_SIZE);
               switch (listType)
               {
               case ACDB_FCC_HDR:
                  (void) parseHeader (pContext, &chunk);
                  break;
               case ACDB_FCC_DLST:
                  (void) parseDeviceList (pContext, &chunk);
                  break;
               case ACDB_FCC_VPTB:
                  (void) parseVocProcCmnTable (pContext, &chunk);
                  break;
               case ACDB_FCC_VSTB:
                  (void) parseVocProcStrmTable (pContext, &chunk);
                  break;
               case ACDB_FCC_APTB:
                  (void) parseAudProcCmnTable (pContext, &chunk);
                  break;
               case ACDB_FCC_ASTB:
                  (void) parseAudProcStrmTable (pContext, &chunk);
                  break;
               case ACDB_FCC_VVTB:
                  (void) parseVocProcVolumeTable (pContext, &chunk);
                  break;
               case ACDB_FCC_AVTB:
                  (void) parseAudProcVolumeTable (pContext, &chunk);
                  break;
               case ACDB_FCC_ADPTB:
                  (void) parseAdieProfileTable (pContext, &chunk);
                  break;
               case ACDB_FCC_ADATB:
                  (void) parseAdieANCTable (pContext, &chunk);
                  break;
               case ACDB_FCC_AFTB:
                  (void) parseAFETable (pContext, &chunk);
                  break;
               case ACDB_FCC_GBTB:
                  (void) parseGenericTable (pContext, &chunk);
                  break;
			   case ACDB_FCC_AFT2:
				  (void) parseAFECmnTable (pContext, &chunk);
				  break;
               default:
                  (void) skipUnknownListChunks (pContext, &chunk);
                  break;
               }
               break;
            default:
               (void) skipUnknownChunks (pContext, &chunk);
               break;
            }

            offset += chunk.length;
            if ((chunk.length&1) != 0)
            {
               uint8_t ignore = 0;
               offset ++;
               (void) getNextAcdbData (pContext, &ignore, 1);
            }
         }
      }
   }

   return result;
}

/* ---------------------------------------------------------------------------
 * Externalized Function Definitions
 *--------------------------------------------------------------------------- */

int32_t AcdbParse (void *pHandle, AcdbParseCBFuncPtrType pfnCallback)
{
   int32_t result = ACDB_SUCCESS;
   AcdbParseContext context;
   result = InitAcdbContext (&context);
   if (result == ACDB_SUCCESS)
   {
      context.pHandle = pHandle;
      context.pfnCallback = pfnCallback;
      context.bParseForTargetVersionOnly = FALSE;
      context.bExitParser = FALSE;

      result = parseAcdbFile (&context);      
   }
   DeinitAcdbContext (&context);

   return result;
}

int32_t AcdbParseTargetVersion (void *pHandle, AcdbParseCBFuncPtrType pfnCallback)
{
   int32_t result = ACDB_SUCCESS;
   AcdbParseContext context;
   result = InitAcdbContext (&context);
   if (result == ACDB_SUCCESS)
   {
      context.pHandle = pHandle;
      context.pfnCallback = pfnCallback;
      context.bParseForTargetVersionOnly = TRUE;
      context.bExitParser = FALSE;

      result = parseAcdbFile (&context);
   }   
   DeinitAcdbContext (&context);
   
   return result;
}
