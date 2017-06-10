#ifndef _SNS_FSA_H_
#define _SNS_FSA_H_
/*============================================================================
  @file sns_fsa.h

  @brief
  File System Abstraction layer for sensors.

  This is a common header file; however, each target OS will have
  a specific implementation.

  <br><br>

  Copyright (c) 2010-2012 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
  ============================================================================*/

/*============================================================================
  EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header:
  $DateTime:

  when       who    what, where, why
  ---------- --- -----------------------------------------------------------
  2011-10-19 gju Added flush.
  2011-06-21 jtl Added stat.
  2010-11-30 jh  Initial revision.
  ============================================================================*/


/*============================================================================
  INCLUDES
  ============================================================================*/
#include <stdint.h>
#include "sns_common.h"

/*============================================================================
  DEFINITIONS AND TYPES
  ============================================================================*/

/*! Meta-data about files */
typedef struct sns_fsa_stat_s {
  int64_t size; /*< Size of the file, in bytes */
} sns_fsa_stat_s;

/*============================================================================
  FUNCTION PROTOTYPES
  ============================================================================*/

/*============================================================================

  FUNCTION:   sns_fsa_open

  ============================================================================*/
/*!
  @brief Opens a file

  @param[i] pathname: A pointer to a string specifying the path name of the file
  @param[i] mode: A pointer to a string specifying the capabilities allowed
            after file is opened:

  @return Handle to the opened file

  @sideeffects
  File will be opened for access specified by mode.
*/
/*============================================================================*/
void* sns_fsa_open( const char* pathname, const char* mode );

/*============================================================================

  FUNCTION:   sns_fsa_seek

  ============================================================================*/
/*!
  @brief Set the file pointer to new location specified

  @return
  SNS_ERR_FAILED if operation failed; SNS_SUCCESS otherwise

  @sideeffects
  File pointer will be set to new location.
*/
/*============================================================================*/
sns_err_code_e sns_fsa_seek( void* file_handle, int64_t offset, int32_t origin );

/*============================================================================

  FUNCTION:   sns_fsa_read

  ============================================================================*/
/*!
  @brief Reads from a file

  @return
  SNS_ERR_FAILED if operation failed; SNS_SUCCESS otherwise

  @sideeffects
  Output buffer will be updated.
*/
/*============================================================================*/
sns_err_code_e sns_fsa_read( void* file_handle, uint8_t* buffer_ptr,
                             uint32_t num_bytes, uint32_t* bytes_read_ptr );

/*============================================================================

  FUNCTION:   sns_fsa_write

  ============================================================================*/
/*!
  @brief Writes to a file

  @return
  SNS_ERR_FAILED if operation failed; SNS_SUCCESS otherwise

  @sideeffects
  File will be updated.
*/
/*============================================================================*/
sns_err_code_e sns_fsa_write( void const *file_handle, uint8_t const *buffer_ptr,
                              uint32_t num_bytes, uint32_t* bytes_written_ptr );

/*============================================================================

  FUNCTION:   sns_fsa_close

  ============================================================================*/
/*!
  @brief Closes a file

  @return
  SNS_ERR_FAILED if operation failed; SNS_SUCCESS otherwise

  @sideeffects
  File will be closed
*/
/*============================================================================*/
sns_err_code_e sns_fsa_close ( void* file_handle );

/*============================================================================

  FUNCTION:   sns_fsa_stat

  ============================================================================*/
/*!
  @brief Returns metadata about a file

  @param[i] pathname: A pointer to a string specifying the path name of the file
  @param[o] fsa_stat: Pointer to a stat structure to fill in

  @return
  SNS_ERR_FAILED if operation failed; SNS_SUCCESS otherwise

*/
/*============================================================================*/
sns_err_code_e sns_fsa_stat( const char* pathname, sns_fsa_stat_s *fsa_stat );

/*============================================================================

  FUNCTION:   sns_fsa_flush

  ============================================================================*/
/*!
  @brief Writes unwritten data in the output buffer to the file.

  @return
  SNS_ERR_FAILED if operation failed; SNS_SUCCESS otherwise

*/
/*============================================================================*/
sns_err_code_e sns_fsa_flush( void* file_handle );

/*============================================================================

  FUNCTION:   sns_fsa_mkdir

  ============================================================================*/
/*!
  @brief Creates a new directory with read/write permissions only to current
    user/group.

  @param[i] pathname: A pointer to a string specifying the path name.

  @return
  SNS_ERR_FAILED if operation failed; SNS_SUCCESS otherwise

*/
/*============================================================================*/
sns_err_code_e sns_fsa_mkdir( const char* pathname );

/*============================================================================

  FUNCTION:   sns_fsa_chown

  ============================================================================*/
/*!
  @brief Changes the owner for the specified file

  @param[i] pathname: A pointer to a string specifying the path name.
  @param[i] usr: String containing the username for the new owner. If NULL,
    will set to the permission-level of the daemon.

  @return
  SNS_ERR_FAILED if operation failed; SNS_SUCCESS otherwise

*/
/*============================================================================*/
sns_err_code_e sns_fsa_chown( const char* pathname, const char* usr );

/*============================================================================

  FUNCTION:   sns_fsa_chgrp

  ============================================================================*/
/*!
  @brief Changes the group for the specified file

  @param[i] pathname: A pointer to a string specifying the path name.
  @param[i] grp: String containing the group name for the new group. If NULL,
    will set to the permission-level of the daemon.

  @return
  SNS_ERR_FAILED if operation failed; SNS_SUCCESS otherwise

*/
/*============================================================================*/
sns_err_code_e sns_fsa_chgrp( const char* pathname, const char* grp );

#endif /* _SNS_FSA_H_ */
