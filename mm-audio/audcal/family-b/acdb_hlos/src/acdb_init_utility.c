/*===========================================================================
    FILE:           acdb_init_utility.c

    OVERVIEW:       This file contains the acdb init utility functions
                    implemented specifically in the win32 environment.

    DEPENDENCIES:   None

                    Copyright (c) 2012 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/*===========================================================================
    EDIT HISTORY FOR MODULE

    This section contains comments describing changes made to the module.
    Notice that changes are listed in reverse chronological order. Please
    use ISO format for dates.

    $Header: //source/qcom/qct/multimedia2/Audio/audcal3/acdb_hlos/rel/2.5/src/acdb_init_utility.c#1 $

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2010-07-08  vmn     Initial revision.

========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */

#include "acdb_init_utility.h"
#include "acdb_init.h"
#include "acdb_command.h"
//modified for QNX build

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
extern int32_t g_persistenceStatus;

int32_t AcdbGetFileData (const char* pFilename, uint8_t **pBfr, uint32_t *pFileSize)
{
   int32_t result = ACDB_UTILITY_INIT_SUCCESS;
   FILE *fp = NULL;
   uint32_t nBytesRead = 0;

   if (pFilename == NULL || pFileSize == NULL || pBfr == NULL)
   {
      return ACDB_UTILITY_INIT_FAILURE;
   }
   else
   {
	  fp = fopen(pFilename, "rb");
      if (fp == NULL)
      {
         return ACDB_UTILITY_INIT_FAILURE;
      }

      fseek (fp, 0, SEEK_END);
      *pFileSize = ftell (fp);
      fseek (fp, 0, SEEK_SET);

	  *pBfr = (uint8_t *)malloc(*pFileSize);
	  if(*pBfr != NULL)
	  {
		  nBytesRead = (uint32_t) fread (*pBfr,sizeof (uint8_t),(size_t)*pFileSize,fp);
	  }
	  else
	  {
		  result = ACDB_UTILITY_INIT_FAILURE;
	  }
	  if(nBytesRead != *pFileSize)
	  {
		  result = ACDB_UTILITY_INIT_FAILURE;
		  if(*pBfr != NULL)
		  {
			 free(*pBfr);
		  }
	  }
	  fclose (fp);
   }
   return result;
}

void  AcdbFreeFileData (void *pBfr)
{
    if(pBfr != NULL)
	{
		free(pBfr);
		pBfr = NULL;
	}
}

int32_t AcdbIsPersistenceSupported(void)
{
   return g_persistenceStatus;
}

char *GetDeltaFileName(const char* pFilename, uint32_t fileNameLen)
{
   char *pDeltaFileName = NULL;
   char *pDeltaFileExtn = "delta";
   uint32_t deltaFileNameSize = 0;

   deltaFileNameSize = fileNameLen + strlen(pDeltaFileExtn) + 1;
   pDeltaFileName = (char *)malloc(deltaFileNameSize);
   if (pDeltaFileName == NULL)
   {
       return NULL;
   }

   strlcpy(pDeltaFileName, pFilename, deltaFileNameSize);
   strlcat(pDeltaFileName, pDeltaFileExtn, deltaFileNameSize);

   return pDeltaFileName;
}

int32_t AcdbIsDeltaFileAvailable (const char* pFilename, uint32_t fileNameLen, uint32_t* pDeltaFileDataLen)
{
   FILE *fp = NULL;
   char *pDeltaFileName = NULL;

   *pDeltaFileDataLen = 0;
   if(pFilename == NULL || fileNameLen == 0)
   {
      return ACDB_UTILITY_INIT_FAILURE;
   }
   else
   {
      pDeltaFileName = GetDeltaFileName(pFilename, fileNameLen);
      if (pDeltaFileName == NULL)
      {
         return ACDB_UTILITY_INIT_FAILURE;
      }

      fp = fopen( pDeltaFileName, "rb");
      if (fp == NULL)
      {
         return ACDB_UTILITY_INIT_FAILURE;
      }

      fseek (fp, 0, SEEK_END);
      *pDeltaFileDataLen = ftell (fp);
      fclose (fp);
   }

   return ACDB_UTILITY_INIT_SUCCESS;
}

int32_t AcdbGetDeltaFileData (const char* pFilename, uint32_t fileNameLen, uint8_t *pFileBfr, uint32_t nFileBfrSize)
{
   int32_t result = ACDB_UTILITY_INIT_SUCCESS;
   FILE *fp = NULL;
   uint32_t nBytesRead = 0;
   uint32_t fileSize = 0;
   char *pDeltaFileName = NULL;

   if (pFilename == NULL)
   {
      return ACDB_UTILITY_INIT_FAILURE;
   }
   else
   {
      pDeltaFileName = GetDeltaFileName(pFilename, fileNameLen);
      if (pDeltaFileName == NULL)
      {
         return ACDB_UTILITY_INIT_FAILURE;
      }

      fp = fopen (pDeltaFileName, "rb");
      if (fp == NULL)
      {
         return ACDB_UTILITY_INIT_FAILURE;
      }

      fseek (fp, 0, SEEK_END);
      fileSize = ftell (fp);
      fseek (fp, 0, SEEK_SET);

	  if(pFileBfr != NULL && fileSize == nFileBfrSize)
	  {
		  nBytesRead = (uint32_t) fread (pFileBfr,sizeof (uint8_t),(size_t)fileSize,fp);
	  }
	  else
	  {
		  result = ACDB_UTILITY_INIT_FAILURE;
	  }

	  if(nBytesRead != fileSize)
	  {
		  result = ACDB_UTILITY_INIT_FAILURE;
		  if(pFileBfr != NULL)
		  {
			 free(pFileBfr);
		  }
	  }
	  fclose (fp);
   }
   return result;
}

int32_t AcdbWriteDeltaFileData (const char* pFilename, uint32_t fileNameLen, uint8_t *pFileBfr, uint32_t nFileBfrSize)
{
   int32_t result = ACDB_UTILITY_INIT_SUCCESS;
   FILE *fp = NULL;
   uint32_t nBytesWrite = 0;
   char *pDeltaFileName = NULL;

   pDeltaFileName = GetDeltaFileName(pFilename, fileNameLen);
   if (pDeltaFileName == NULL)
   {
       return ACDB_UTILITY_INIT_FAILURE;
   }

   fp = fopen (pDeltaFileName, "wb");
   if (fp == NULL)
   {
      return ACDB_UTILITY_INIT_FAILURE;
   }

   if(pFileBfr != NULL && nFileBfrSize != 0)
   {
      nBytesWrite = fwrite(pFileBfr,sizeof(uint8_t),(size_t)nFileBfrSize, fp);
      if(nBytesWrite != nFileBfrSize)
      {
         result = ACDB_UTILITY_INIT_FAILURE;
      }
   }
   else
   {
      result = ACDB_UTILITY_INIT_FAILURE;
   }

   fclose (fp);

   return result;
}

int32_t AcdbDeleteDeltaFileData (const char* pFilename, uint32_t fileNameLen)
{
   int32_t result = ACDB_UTILITY_INIT_SUCCESS;
   int32_t delFileResult = 0;
   char *pDeltaFileName = NULL;

   pDeltaFileName = GetDeltaFileName(pFilename, fileNameLen);
   if (pDeltaFileName == NULL)
   {
      return ACDB_UTILITY_INIT_FAILURE;
   }

   if(access(pDeltaFileName, 0) != -1)
   {
      delFileResult = remove(pDeltaFileName);
      if(delFileResult != 0)
      {
         result = ACDB_UTILITY_INIT_FAILURE;
      }
   }

   return result;
}
