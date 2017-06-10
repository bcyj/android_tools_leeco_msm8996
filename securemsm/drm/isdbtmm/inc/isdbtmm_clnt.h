#ifndef ISDBTMM_CLNT_H
#define ISDBTMM_CLNT_H

/** @file isdbtmm_clnt.h
 * @brief
 * This file contains the definitions of the constants, data structures
 * and interfaces to the ISDB-Tmm BKM client
 */
/*===========================================================================
  Copyright (c) 2012 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/27/13   rz     Handling deprecated mkdir() and rmdir() in SFS
01/16/12   ib     Initial Version.

===========================================================================*/

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
/** @addtogroup OEMCrypto_Clnt
  @} */

#ifdef __cplusplus
extern "C"
{
#endif

#include "comdef.h"

/**
  Initialize the ISDB-Tmm BKM module.
  - Loads the TZ image
  - Open the FS services

  @return

  0        - Success.
  non 0    - Failure .


  @dependencies
  Must be called prior to invoking any of the other BKM APIs


  @sideeffects
  None
*/
long BKMCL_Initialize();


/**
  Teardown the ISDB-Tmm BKM module.
  - Unload the TZ image
  - Closes the FS services

  @return
  0      - Success.
  non 0  - Failure.


  @dependencies
  Must be called for cleaning up the BKM module.
  No other BKM APIs (other than KMCL_Initialize) should be called after BKMCL_Uninitialize.
  To resume BKM operation after this API has been called, the calling application should call
  BKMCL_Initialize() first.

  @sideeffects
  None
*/
long BKMCL_Uninitialize();


/**
  Open a file in the ISDB-Tmm SFS folder.

  @return
  Positive value of file descriptor   - Success.
  (-1)    - Failure.

  @param[in]	  *fileName         Pointer to the file name
  @param[in]	  mode              Bitmask field that is used to specify file
                                    modes.
                                - O_RDONLY - Open for read-only access.
                                - O_RDWR   - Open for read-write access.
                                - O_CREAT - Create the file if it does not exist.
                                - O_TRUNC - Truncate the file to zero size after opening.
                                - O_APPEND - Write operations occur at the end of the file.


  @dependencies
  None

  @sideeffects
  None
*/
long BKMCL_Open( uint8* fileName, int32 mode );


/**
  Close a file in the ISDB-Tmm SFS folder.

  @return
  0       - Success.
  (-1)    - Failure.

  @param[in]	  fd         File descriptor of the file to close

  @dependencies
  None

  @sideeffects
  None
*/
long BKMCL_Close( int fd );


/**
  Remove a file from the ISDB-Tmm SFS folder.

  @return
  0       - Success.
  (-1)    - Failure.

  @param[in]	  *fileName         Pointer to the file name to remove

  @dependencies
  None

  @sideeffects
  None
*/
long BKMCL_Remove( uint8* fileName );



/**
  Read from a file in the ISDB-Tmm SFS folder.

  @return
  Number of bytes that were read       - Success.
  (-1)    - Failure.

  @param[in]	  fd         File descriptor of the file to read from
  @param[in]	  buf        Buffer where to copy the read data
  @param[in]	  count      Number of bytes to read

  @dependencies
  None

  @sideeffects
  None
*/
long BKMCL_Read( int fd, uint8* buf, uint32 count);



/**
  Write to a file in the ISDB-Tmm SFS folder.

  @return
  Number of bytes that were written       - Success.
  (-1)    - Failure.

  @param[in]	  fd         File descriptor of the file to write to
  @param[in]	  buf        Buffer of data to write
  @param[in]	  count      Number of bytes to write

  @dependencies
  None

  @sideeffects
  None
*/
long BKMCL_Write( int fd, uint8* buf, uint32 count);


/**
  Creates a new folder in the ISDB-Tmm SFS folder.

  @return
  0       - Success.
  (-1)    - Failure.
  (-2)    - Deprecated.

  @param[in]	  *dirName         Pointer to the file name

  @dependencies
  None

  @sideeffects
  None
*/
long BKMCL_MkDir( uint8* dirName );


/**

  Removed a  directory from the ISDB-Tmm SFS folder tree.

  @return
  0       - Success.
  (-1)    - Failure.
  (-2)    - Deprecated.

  @param[in]	  *dirName         Pointer to the directory name

  @dependencies
  None

  @sideeffects
  None

*/
long BKMCL_RmDir( uint8* dirName );


/**
  Retreive the size of an open file in the ISDB-Tmm SFS folder.

  @return
  0       - Success.
  (-1)    - Failure.


  @param[in]	  fd         File descriptor of the file
  @param[out]	  size       File size in bytes

  @dependencies
  None

  @sideeffects
  None
*/
long BKMCL_GetSize( int fd, uint32* size);


/**
  Seek in a file in the ISDB-Tmm SFS folder.

  @return

  Current file position       - Success.
  (-1)                        - Failure.

  @param[in]	  fd         File descriptor of the file to seek in
  @param[in]	  offset     File offset to seek in bytes.
  @param[in]	  whence     Indicates start, end, or current position.
                    - 0 = SEEK_SET - Start of the file.
                    - 1 = SEEK_CUR - Current file point.
                    - 2 = SEEK_END - End of the file.


  @dependencies

  Caller must include unistd.h in order to use WHENCE values defined.

  @sideeffects
  None

*/
long BKMCL_Seek( int fd, int32 offset, int whence);


#ifdef __cplusplus
}
#endif

#endif //ISDBTMM_CLNT_H
