#ifndef MMFILE_H
#define MMFILE_H
/*===========================================================================
                          M M    W r a p p e r
                        f o r   F i l e   S e r v i c e s

*//** @file MMFile.h
  This file defines the interfaces the support file operations like open, read,
  write, and seek.

Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/platform/OSAbstraction/main/latest/inc/MMFile.h#5 $

when       who         what, where, why
--------   ---         -------------------------------------------------------
07/02/08   gkapalli    Created file.

============================================================================*/

/* =======================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

#include <sys/types.h>
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
#ifndef _MM_HANDLE
typedef void* MM_HANDLE;
#define _MM_HANDLE
#endif


/*
 * wide character data type, can hold unicode character
 */
#define MM_WCHAR unsigned short
/*
 * The file name is limited to MM_MAX_PATH characters
 */
#define MM_MAX_PATH MAX_PATH

/*
 * List of seek options are,
 *      MM_FILE_SEEK_BEG - seek from the begnining
 *      MM_FILE_SEEK_CUR - seek from the current position
 *      MM_FILE_SEEK_END - seek from the end of the file
 */
#define MM_FILE_SEEK_BEG 0
#define MM_FILE_SEEK_CUR 1
#define MM_FILE_SEEK_END 2

/*
 * List of file open modes are,
 *      MM_FILE_CREATE_R - open for read
 *      MM_FILE_CREATE_R_PLUS - open for read and write
 *      MM_FILE_CREATE_W - truncate or create file for write
 *      MM_FILE_CREATE_W_PLUS - truncate or create for read and write
 *      MM_FILE_CREATE_A - open+seek_to_the_end or create file for write
 */
#define MM_FILE_CREATE_R      0
#define MM_FILE_CREATE_R_PLUS 1
#define MM_FILE_CREATE_W      2
#define MM_FILE_CREATE_W_PLUS 3
#define MM_FILE_CREATE_A      4

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/*
 * Creates/Opens a file
 *
 *
 * @param[in] pFilePath - Name and path of the file to act on
 * @param[in] nMode -  mode to be used to create a file (MM_FILE_CREATE_*)
 * @param[out] pHandle - returns a reference to the file handle
 *
 * @return return value 0 is success else failure
 */
int MM_File_Create
(
  const char   *pFilePath,
  int           nMode,
  MM_HANDLE *pHandle
);

/*
 * Creates/Opens a file path given in WCHAR format
 *
 *
 * @param[in] pFilePath - Name and path of the file to act on in WCHAR
 * @param[in] nMode -  mode to be used to create a file (MM_FILE_CREATE_*)
 * @param[out] pHandle - returns a reference to the file handle
 *
 * @return return value 0 is success else failure
 */
int MM_File_CreateW
(
  MM_WCHAR *pFilePath,
  int   nMode,
  MM_HANDLE *pHandle
);

/*
 * Releases the resources associated with the file handle
 *
 * @param[in] handle - reference to the file handle
 *
 * @return zero value on success else failure
 */
int MM_File_Release
(
  MM_HANDLE handle
);

/*
 * Reads data from a file into the buffer
 *
 * @param[in] handle - reference to the file handle
 * @param[in] pBuffer - pointer to the buffer to which data may be copied
 * @param[in] nSize -  number of bytes to read, assumes pBuffer is big enough to hold nSize
 * @param[out] pBytesRead -  number of bytes read into pBuffer
 *
 * @return return value 0 is success else failure
 */
int MM_File_Read
(
  MM_HANDLE handle,
  char *pBuffer,
  ssize_t nSize,
  ssize_t *pnBytesRead
);

/*
 * Writes data from the buffer into the file
 *
 * @param[in] handle - reference to the file handle
 * @param[in] pBuffer - pointer to the buffer that contains the data
 * @param[in] nSize -  size of pBuffer that needs to be written
 * @param[out] pBytesWritten -  number of bytes written into the file
 *
 * @return return value 1 is success else failure
 */
int MM_File_Write
(
  MM_HANDLE handle,
  char *pBuffer,
  ssize_t nSize,
  ssize_t *pnBytesWritten
);

/*
 * Reposition the file pointer in an open file
 *
 * @param[in] handle - reference to the file handle
 * @param[in] nOffset - offset based on nWhence; may be negative
 * @param[in] nWhence -  flags to be used when acting on a file
 *
 * @return return value 0 is success else failure
 */
int MM_File_Seek
(
  MM_HANDLE handle,
  long nOffset,
  int nWhence
);

/*
 * Reposition the file pointer in an open file. Extended to support
 * 64 bit nOffset
 *
 * @param[in] handle - reference to the file handle
 * @param[in] nOffset - offset based on nWhence; may be negative
 * @param[in] nWhence -  flags to be used when acting on a file
 *
 * @return return value 0 is success else failure
 */
int MM_File_SeekEx
(
  MM_HANDLE handle,
  long long nOffset,
  int nWhence
);
/*
 * Returns the current file size
 *
 * @param[in] handle - reference to the file handle
 * @param[out] dwSize - returns the file size on success
 *
 * @return return value 1 is success else failure
 */
int MM_File_GetSize
(
  MM_HANDLE handle,
  unsigned long *pnSize
);
/*
 * Returns the current file size. Extended to support 64 bit
 *
 * @param[in] handle - reference to the file handle
 * @param[out] dwSize - returns the file size on success
 *
 * @return return value 1 is success else failure
 */
int MM_File_GetSizeEx
(
  MM_HANDLE handle,
  unsigned long long *pnSize
);

/*
 * Truncates the file to the length specified
 *
 * @param[in] handle - reference to the file handle
 * @param[in] nLength - the length to which the file needs to be truncated
 *
 * @return return value 0 is success else failure
 */
int MM_File_Truncate
(
  MM_HANDLE handle,
  long nLength
);

/*
 * Copies an existing file to a new file, fails if there is file by same
 * name at the new location.
 *
 * @param[in] pExistingFile - Pointer to a null-terminated string that
 *                            specifies the name of an existing file
 * @param[in] pNewFilePath - Pointer to a null-terminated string that specifies
 *                           the name of the new file.
 *
 * @return return value 0 is success else failure
 */
int MM_File_Copy
(
  const char* pExistingFile,
  const char *pNewFile
);

/*
 * Copies an existing file to a new file, fails if there is file by same
 * name at the new location.
 *
 * @param[in] pExistingFile - Pointer to a null-terminated string that
 *                            specifies the name of an existing file
 * @param[in] pNewFilePath - Pointer to a null-terminated string that specifies
 *                           the name of the new file.
 *
 * @return return value 0 is success else failure
 */
int MM_File_Move
(
  const char* pExistingFile,
  const char *pNewFile
);

/*
 * Deletes an existing file.
 *
 * @param[in] pFile - Pointer to a null-terminated string that specifies the
 *                    name of the file file
 *
 * @return return value 0 is success else failure
 */
int MM_File_Delete
(
  const char* pFile
);

/*
 * Gets the free space in the disk pointed by the path
 *
 * @param[in] pPath - Pointer to a null-terminated string that specifies the
 *                    path
 * @param[out] pFreeSpace - updates the value with the avaliable free space
 *
 * @return return value 0 is success else failure
 */
int MM_File_GetFreeSpace
(
  const char* pPath,
  unsigned long long *pFreeSpace
);

/*
 * Get the current file position
 *
 * @param[in] handle - Reference to the file handle
 * @param[out] filePos - Pointer to the file position
 *
 * @return return value 0 is success else failure
 */
int MM_File_GetCurrentPosition
(
  MM_HANDLE handle,
  unsigned long* filePos
);

/*
 * Get the current file position, extended support for 64 bit
 *
 * @param[in] handle - Reference to the file handle
 * @param[out] filePos - Pointer to the file position
 *
 * @return return value 0 is success else failure
 */
int MM_File_GetCurrentPositionEx
(
  MM_HANDLE handle,
  unsigned long long* filePos
);
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif // MMFILE_H
