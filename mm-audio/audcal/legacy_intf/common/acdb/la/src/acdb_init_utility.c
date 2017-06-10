/*===========================================================================
FILE:           acdb_init_utility.c

OVERVIEW:       This file contains the acdb init utility functions
implemented specifically for the LA environment.

DEPENDENCIES:   None

                    Copyright (c) 2010-2012 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* ---------------------------------------------------------------------------
* Include Files
*--------------------------------------------------------------------------- */

#include "acdb_includes.h"
#include "acdb_init_utility.h"
#include "acdb_init.h"

/* ---------------------------------------------------------------------------
* Preprocessor Definitions and Constants
*--------------------------------------------------------------------------- */
typedef int errno_t;

const char* AcdbInitDefaultPath = "/etc";
const char* AcdbInitDefaultFilename = "/etc/audio_cal.acdb";
const char* AcdbInitDefaultErrorFilename = "/etc/audio_cal_invalid.acdb";

/* ---------------------------------------------------------------------------
* Type Declarations
*--------------------------------------------------------------------------- */

typedef struct AcdbInitLAFileContext {
   int32_t fh;
} AcdbInitLAFileContext;

/* ---------------------------------------------------------------------------
* Global Data Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Static Variable Definitions
*--------------------------------------------------------------------------- */

#ifdef _ANDROID_
static mode_t ALLPERMS = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
#endif

/* ---------------------------------------------------------------------------
* Static Function Declarations and Definitions
*--------------------------------------------------------------------------- */
static int32_t AcdbMkdir(const char *path, mode_t mode)
{
   struct stat     st = {0};
   int             status = 0;

   /* check if directory exists or not*/
   if (mkdir(path, mode) != 0)
   {
      /* Directory does not exist */
      if (stat(path, &st) != 0)
      {
         status = -1;
      }
   }
   else if (!S_ISDIR(st.st_mode))
   {
      errno = ENOTDIR;
      status = -1;
   }

   return(status);
}

static int32_t AcdbMakepath(const char *path, mode_t mode)
{
   char           *pp;
   char           *sp;
   int             status;
   char           *copypath = strdup(path);

   status = 0;
   pp = copypath;
   while (status == 0 && pp && (sp = strchr(pp, '/')) != 0)
   {
      if (sp != pp)
      {
         /* Neither root nor double slash in path */
         *sp = '\0';
         status = AcdbMkdir(copypath, mode);
         *sp = '/';
      }
      pp = sp + 1;
   }
   if (status == 0)
   {
      status = AcdbMkdir(path, mode);
   }
   free(copypath);
   return (status);
}

/* ---------------------------------------------------------------------------
* Externalized Function Definitions
*--------------------------------------------------------------------------- */

int32_t AcdbInitGetAcdbDefaultFilename (const char** ppFilename)
{
   int32_t result = ACDB_INIT_SUCCESS;
   if (ppFilename == NULL)
   {
      result = ACDB_INIT_FAILURE;
      printf("ACDB init get default file failed\n");
   }
   else
   {
      *ppFilename = AcdbInitDefaultFilename;
   }
   return -1;
}

int32_t AcdbInitGetAcdbDefaultErrorFilename (const char** ppErrorFilename)
{
   int32_t result = ACDB_INIT_SUCCESS;
   if (ppErrorFilename == NULL)
   {
      result = ACDB_INIT_FAILURE;
      printf("ACDB init get default error file failed\n");
   }
   else
   {
      *ppErrorFilename = AcdbInitDefaultErrorFilename;
   }
   return result;
}

int32_t AcdbInitDoesPathExist (const char* pPath)
{
   int32_t result = ACDB_INIT_SUCCESS;

   if (pPath == NULL)
   {
      result = ACDB_INIT_FAILURE;
      printf("ACDB init check path failed\n");
   }
   else
   {
      errno_t err = access (pPath, 0);
      if (err != 0)
      {
         result = ACDB_INIT_FAILURE;
         printf("ACDB init access path failed\n");
      }
   }

   return result;
}

int32_t AcdbInitRenameFile (const char* pOldFilename, const char* pNewFilename)
{
   int32_t result = ACDB_INIT_SUCCESS;

   if (pNewFilename == NULL)
   {
      result = ACDB_INIT_FAILURE;
      printf("ACDB init rename file failed with null filename\n");
   }
   else
   {
      // TODO: What is equivalent function in BMP?
      result = rename (pOldFilename, pNewFilename);
      if (result != 0)
      {
         result = ACDB_INIT_FAILURE;
         printf("ACDB init rename file failed\n");
      }
   }

   return result;
}

int32_t AcdbInitCreatePath ()
{
   int32_t result = ACDB_INIT_SUCCESS;

   result = AcdbMakepath (AcdbInitDefaultPath, ALLPERMS);
   if (result != 0)
   {
      result = ACDB_INIT_FAILURE;
      printf("ACDB init create path failed\n");
   }

   return result;
}

int32_t AcdbInitFileOpen (const char* pFilename, AcdbInitFileContext** ppFileContext)
{
   int32_t result = ACDB_INIT_SUCCESS;
   errno_t err = 0;
   AcdbInitLAFileContext* pContext = NULL;

   if (pFilename == NULL || ppFileContext == NULL)
   {
      result = ACDB_INIT_FAILURE;
      printf("ACDB init open file failed with null filename\n");
   }
   else
   {
      *ppFileContext = malloc (sizeof (AcdbInitLAFileContext));
      pContext = (AcdbInitLAFileContext*) *ppFileContext;
      if (pContext != NULL)
      {
         memset (pContext, 0, sizeof (AcdbInitLAFileContext));
      }
      else
      {
         result = ACDB_INIT_FAILURE;
         printf("ACDB init open file failed\n");
         return result;
      }

      pContext->fh = open (pFilename, O_RDONLY, ALLPERMS);
      if (pContext->fh < 0)
      {
         result = ACDB_INIT_FAILURE;
         free(pContext);
         printf("ACDB init open file failed\n");
      }
   }

   return result;
}

int32_t AcdbInitGetFileSize (AcdbInitFileContext *pFileContext, uint32_t *pSize)
{
   int32_t result = ACDB_INIT_SUCCESS;

   if (pFileContext == NULL || pSize == NULL)
   {
      result = ACDB_INIT_FAILURE;
      printf("ACDB init get file size failed with null filename\n");
   }
   else
   {
      AcdbInitLAFileContext* pContext = (AcdbInitLAFileContext*) pFileContext;

      struct stat sFileInfo;
      result = fstat(pContext->fh, &sFileInfo);
      if (0 == result)
      {
         *pSize = sFileInfo.st_size;
         printf("ACDB file size is: %llu\n", sFileInfo.st_size);
      }
      else
      {
         *pSize = 0;
         printf("ACDB init get file size failed\n");
      }
   }

   return result;
}

int32_t AcdbInitFileRead (AcdbInitFileContext *pFileContext,
                          uint8_t *pBfr,
                          uint32_t nBytesToRead,
                          uint32_t *pBytesRead)
{
   int32_t result = ACDB_INIT_SUCCESS;

   if (pFileContext == NULL || pBfr == NULL || pBytesRead == NULL)
   {
      result = ACDB_INIT_FAILURE;
      printf("ACDB init read file failed with null filename\n");
   }
   else
   {
      AcdbInitLAFileContext* pContext = (AcdbInitLAFileContext*) pFileContext;

      *pBytesRead = (uint32_t) read (pContext->fh, pBfr, nBytesToRead);
   }

   return result;
}

int32_t AcdbInitFileClose (AcdbInitFileContext *pFileContext)
{
   int32_t result = ACDB_INIT_SUCCESS;

   if (pFileContext == NULL)
   {
      result = ACDB_INIT_FAILURE;
      printf("ACDB init close file failed with null filename\n");
   }
   else
   {
      AcdbInitLAFileContext* pContext = (AcdbInitLAFileContext*) pFileContext;

      (void) close (pContext->fh);
      pContext->fh = NULL;

      // Free passed in pointer
      free(pContext);
   }

   return result;
}
