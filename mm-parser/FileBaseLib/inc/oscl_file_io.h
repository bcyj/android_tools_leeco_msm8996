#ifndef __OSCL_FILE_IO_H__
#define __OSCL_FILE_IO_H__
/* =======================================================================
                               oscl_file_io.h
DESCRIPTION
*
    This header file contains OS specific defines and re-defines necessary for
    the File Format library to compile and run on different OS's.
*

* WARNING because this file defines the macro _T(x), this file must be  *
* included AFTER standard compiler header files that could potentially  *
* define this macro also depending on the compiler.                     *

Copyright 2011-2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/oscl_file_io.h#26 $
$DateTime: 2012/11/27 20:01:35 $
$Change: 3077641 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <stdio.h>
#include "parserdatadef.h"
#include "parserinternaldefs.h"
extern "C"
{
  #ifdef USE_EFS_FILEIO
    #include "fs_public.h"
    #include "fs_stdio.h"
    #include "fs_sys_types.h"
  #endif
}
#define FILE_HANDLE_INVALD (void *)(-1)

#ifdef __cplusplus
  #include "filesourcestring.h"
#endif //__cplusplus

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  #include "DataSourcePort.h"
#endif

#define FEATURE_FILESOURCE_FILE_IO_CACHING

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define MAX_FILENAME_LEN QTV_MAX_FILENAME_BYTES /* 256 */ /* use same macro as everywhere else */
#define MAX_MODE_LEN 16

#ifdef FEATURE_FILESOURCE_FILE_IO_CACHING
#define FILE_READ_BUFFER_DEFAULT_SIZE 1024
#endif //FEATURE_FILESOURCE_FILE_IO_CACHING

class IxStream;
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* File I/O */
/* OS dependant file descriptor structure */
typedef struct _BUF_FILE
{
  unsigned char * pBuf;
  uint64 bufSize;
  uint64 curPos;
} BUF_FILE;

typedef struct _PULL_FILE
{
  uint64 curPos;
  uint64 trackId;
} PULL_FILE;

typedef enum _INPUT_STREAM_TYPE
{
   LOCAL_FILE     = 1,
   DRM_IXSTREAM,
   ISTREAM_PORT
}INPUT_STREAM_TYPE;

typedef struct _OSCL_FILE
{
  BUF_FILE        memBuf;
  bool            bBufValid;
  bool            bPullBufValid;
  PULL_FILE       pullBuf;
  MM_HANDLE    videoHandle;
#ifdef FEATURE_FILESOURCE_FILE_IO_CACHING
  uint8           *readBuffer;
  int64           readBuffPos; /*The next byte available to be read
                                 from the buffer after fileReadPos */
  int64           fileReadPos; /*Offset of the last byte read from the file */
  int64           fileSize;
  int64           readBuffSize;  /*size of the the valid cache*/
  bool            cacheValid;   /* cache is valid or not */
#endif //FEATURE_FILESOURCE_FILE_IO_CACHING

  IxStream* inputStream;
  INPUT_STREAM_TYPE StreamType;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  video::iStreamPort* pStreamPort;
#endif
  MM_HANDLE pCriticalSection;
} OSCL_FILE;

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
MACRO MYOBJ

ARGS
  xx_obj - this is the xx argument

DESCRIPTION:
  Complete description of what this macro does
========================================================================== */

/* =======================================================================
**                        Function Declarations
** ======================================================================= */

/* File I/O functions */
#ifdef __cplusplus

#ifdef FEATURE_FILESOURCE_DRM_DCF
/* ======================================================================
FUNCTION
  OSCL_FileOpen

DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

========================================================================== */
OSCL_FILE * OSCL_FileOpen(IxStream* inputStream);
#endif

/* ======================================================================
FUNCTION
  OSCL_FileOpen

DESCRIPTION
  To open the file and set the read cache buffer size with the given value
  or set with default size.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
OSCL_FILE *OSCL_FileOpen (
     const FILESOURCE_STRING                &filename,
     const OSCL_TCHAR                 *mode,
     int32 bufferSize       =         FILE_READ_BUFFER_DEFAULT_SIZE
);

/* ======================================================================
FUNCTION
  OSCL_FileOpen

DESCRIPTION
  To open the file and set the read cache buffer size with the given value
  or set with default size.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
OSCL_FILE *OSCL_FileOpen (const char *filename, const char *mode,
     int32 bufferSize       =         FILE_READ_BUFFER_DEFAULT_SIZE);



bool OSCL_FileRename (
    const char                  *old_filename,     /* Current name of file */
    const char                  *new_filename     /* New name of file */
);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
/* ======================================================================
FUNCTION
  OSCL_FileOpen

DESCRIPTION
  Support playback from StreamPort

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
OSCL_FILE * OSCL_FileOpen(video::iStreamPort* pPort);
#endif
/* ======================================================================
FUNCTION
  OSCL_FileDelete

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool OSCL_FileDelete(const FILESOURCE_STRING &filename);

/* ======================================================================
FUNCTION
  OSCL_FileDelete

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool OSCL_FileDelete(const char *filename);

/* ======================================================================
FUNCTION
  OSCL_FileOpen

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
OSCL_FILE * OSCL_FileOpen(unsigned char *pBuf, uint64 bufSize,uint32 trackId = 0);

/* ======================================================================
FUNCTION
  OSCL_IsFileOpen

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool OSCL_IsFileOpen(OSCL_FILE *fp);
#endif

/* ======================================================================
FUNCTION
  OSCL_FileRead

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32  OSCL_FileRead (void *buffer, uint32 size,
                       uint32 numelements,  OSCL_FILE *fp);

/* ======================================================================
FUNCTION
  OSCL_FileWrite

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32  OSCL_FileWrite (void *buffer, uint32 size,
                        uint32 numelements,  OSCL_FILE *fp);

/* ======================================================================
FUNCTION
  OSCL_FileSeek

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int64 OSCL_FileSeek (OSCL_FILE *fp,  uint64 offset, int origin);

/* ======================================================================
FUNCTION
  OSCL_FileSeekRead

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32 OSCL_FileSeekRead ( void *buffer, uint32 size, uint32 numelements,
                          OSCL_FILE *fp,  uint64 offset, int origin );

/* ======================================================================
FUNCTION
  OSCL_FileSeekWrite

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32 OSCL_FileSeekWrite (OSCL_FILE *fp, int origin, uint64 offset,
                           void *buffer, uint32 size, uint32 numelements);
/* ======================================================================
FUNCTION
  OSCL_FileTell

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint64 OSCL_FileTell (OSCL_FILE *fp,bool* bError=NULL);

/* ======================================================================
FUNCTION
  OSCL_FileClose

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int32 OSCL_FileClose (OSCL_FILE *fp);

/* ======================================================================
FUNCTION
  OSCL_FileFlush

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int32 OSCL_FileFlush (OSCL_FILE *fp);

/* ======================================================================
FUNCTION
  OSCL_CheckEndOfFile

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int32 OSCL_CheckEndOfFile (OSCL_FILE *fp);

/* ======================================================================
FUNCTION
  OSCL_GetFileError

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int32 OSCL_GetFileError (OSCL_FILE *fp);

/* ======================================================================
FUNCTION
  OSCL_FileExists

DESCRIPTION
  Find if a file with given (full path) name already exists

DEPENDENCIES
  NONE

RETURN VALUE
  TRUE if file already exists.

SIDE EFFECTS
  NONE

========================================================================== */
bool OSCL_FileExists(const FILESOURCE_STRING &filename);


/* ======================================================================
FUNCTION
  OSCL_GetEFSFileSysFreeSpace

DESCRIPTION
  Find if a file with given (full path) name already exists

DEPENDENCIES
  NONE

RETURN VALUE
  TRUE if file already exists.

SIDE EFFECTS
  NONE

========================================================================= */
uint64 OSCL_GetEFSFileSysFreeSpace (void);

/* ======================================================================
FUNCTION
  OSCL_GetFileSysFreeSpace

DESCRIPTION
  Checks the available space in the file system
  path provided and returns that value

DEPENDENCIES
  NONE

RETURN VALUE
  TRUE if file already exists.

SIDE EFFECTS
  NONE

========================================================================== */
int64 OSCL_GetFileSysFreeSpace (const char *filePath);

/* ======================================================================
FUNCTION
  OSCL_GetFileSysFreeSpace

DESCRIPTION
  Checks the available space in the file system
  path provided and returns that value

DEPENDENCIES
  NONE

RETURN VALUE
  TRUE if file already exists.

SIDE EFFECTS
  NONE

========================================================================== */

int64 OSCL_GetFileSysFreeSpace (const FILESOURCE_STRING &filePath);

/* ======================================================================
FUNCTION
  OSCL_FileExists

DESCRIPTION
  Find if a file with given (full path) name already exists

DEPENDENCIES
  NONE

RETURN VALUE
  TRUE if file already exists.

SIDE EFFECTS
  NONE

========================================================================== */
bool OSCL_FileList(void *buffer,unsigned char *dirName,unsigned long bufSize);

/* ======================================================================
FUNCTION
  OSCL_NumFiles

DESCRIPTION
  Returns number of files in a given directory on the EFS

DEPENDENCIES
  NONE

RETURN VALUE
  TRUE if number of files found

SIDE EFFECTS
  NONE

========================================================================== */
bool OSCL_NumFiles(const char *dirName,int32 *numFiles);

/* ======================================================================
FUNCTION
  OSCL_FileSize

DESCRIPTION
  Returns the size of the file

DEPENDENCIES
  NONE

RETURN VALUE
  The Size of the file

SIDE EFFECTS
  NONE

========================================================================== */
uint64 OSCL_FileSize( const FILESOURCE_STRING &filename );

/* ======================================================================
FUNCTION
  OSCL_FileSize

DESCRIPTION
  Returns the size of the file

DEPENDENCIES
  NONE

RETURN VALUE
  The Size of the file

SIDE EFFECTS
  NONE

========================================================================== */
uint64 OSCL_FileSize( const char *filename );


#endif // __File_Format_OS_H__
