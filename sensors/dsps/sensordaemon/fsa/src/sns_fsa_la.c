/*============================================================================
  @file sns_fsa_la.c

  @brief
  This is the implementation of the Sensors file system abstraction for
  Linux Android.

  <br><br>

  DEPENDENCIES: This uses file access interfaces defined in stdio.h.

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================

  INCLUDE FILES

  ============================================================================*/
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <limits.h>
#include <stdlib.h>
#include <libgen.h>

#include "sensor1.h"
#include "sns_common.h"
#include "sns_fsa.h"
#include "sns_main.h"
#include "sns_debug_str.h"

/*============================================================================
 *
  Function Definitions

  ============================================================================*/

/*============================================================================

  FUNCTION:   sns_fsa_open

  ============================================================================*/
void* sns_fsa_open( const char* pathname, const char* mode )
{
  FILE* fptr = NULL;
  int fd;
  int err;
  char *resolved_path = malloc( PATH_MAX );
  char *resolved_dirname_path = malloc( PATH_MAX );
  char *dir = malloc( PATH_MAX );
  char *dir_name;

  if( NULL == resolved_path || NULL == resolved_dirname_path || NULL == dir )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_FSA, "malloc failed" );
    goto cleanup;
  }

  if( NULL == pathname || NULL == mode )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_FSA, "NULL input ptrs" );
    goto cleanup;
  }

  strlcpy( dir, pathname, PATH_MAX );
  dir_name = dirname( dir );

  /* check for existence of directory */
  if( NULL == realpath( dir_name, resolved_dirname_path ) )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_FSA, "realpath failed for directory name%i", errno );
    goto cleanup;
  }
  else if( ( 0 != ( err = strncmp( dir_name, resolved_dirname_path, PATH_MAX ) ) ) )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_FSA, "invalid directory path %i", err );
    goto cleanup;
  }

  /* check for existence of file */
  if( NULL == realpath( pathname, resolved_path ) )
  {
    if( errno != ENOENT )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_FSA, "realpath failed for pathname %i", errno );
      goto cleanup;
    }
  }
  else if( 0 != ( err = strncmp( pathname, resolved_path, PATH_MAX ) ) )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_FSA, "invalid file path %i", err );
    goto cleanup;
  }

  fptr = fopen( pathname, mode );
  if( NULL != fptr )
  {
    sns_fsa_flush( (void*)fptr );
  }

  /* In case of creating a new file, open the directory and force a sync */
  if( NULL == dir_name )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_FSA, "dirname failed" );
  }
  else
  {
    fd = open( dir_name, O_DIRECTORY );
    if( 0 <= fd )
    {
      fsync( fd );
      close( fd );
    }
  }

cleanup:
  free( resolved_path );
  free( resolved_dirname_path );
  free( dir );
  return( (void*)fptr );
}

/*============================================================================

  FUNCTION:   sns_fsa_seek

  ============================================================================*/
sns_err_code_e sns_fsa_seek( void* file_handle, int64_t offset, int32_t origin )
{
  int err = SNS_SUCCESS;

  err = fseek( (FILE*)file_handle, offset, SEEK_SET );

  SNS_PRINTF_STRING_LOW_3( SNS_DBG_MOD_APPS_FSA,
                           "sns_fsa_seek: offset %d, origin %d, err: %d",
                           offset, origin, err );
  if( 0 != err )
  {
    return SNS_ERR_FAILED;
  }
  else
  {
    return SNS_SUCCESS;
  }
}

/*============================================================================

  FUNCTION:   sns_fsa_read

  ============================================================================*/
sns_err_code_e sns_fsa_read( void* file_handle, uint8_t* buffer_ptr,
                             uint32_t num_bytes, uint32_t* bytes_read_ptr )
{
  size_t cnt;
  int    err;

  cnt = fread( (void*)buffer_ptr , sizeof(uint8_t), (size_t)num_bytes,
               (FILE*)file_handle );
  err = ferror( (FILE*)file_handle );

  SNS_PRINTF_STRING_LOW_2( SNS_DBG_MOD_APPS_FSA,
                           "sns_fsa_read: bytes read %d, err: %d", cnt, err );

  /* The bionic version of fread() returns EOF even though it is defined
     as (-1). This does not match the type of the return value, size_t
     which is unsigned. */
  if( ( 0 == cnt ) || ( (size_t)EOF == cnt ) || ( 0 != err ) )
  {
    *bytes_read_ptr = 0;
    return SNS_ERR_FAILED;
  }
  else
  {
    *bytes_read_ptr = (uint32_t)cnt;
    return SNS_SUCCESS;
  }
}

/*============================================================================

  FUNCTION:   sns_fsa_write

  ============================================================================*/
sns_err_code_e sns_fsa_write( void const *file_handle, uint8_t const *buffer_ptr,
                              uint32_t num_bytes, uint32_t* bytes_written_ptr )
{
  size_t cnt;
  int    err;

  cnt = fwrite( (void*)buffer_ptr, sizeof(uint8_t), (size_t)num_bytes,
                (FILE*)file_handle );
  err = ferror( (FILE*)file_handle );

  if(( cnt != num_bytes ) || ( 0 != err ))
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_FSA, "fwrite failed, %d", err );
    *bytes_written_ptr = 0;
    return SNS_ERR_FAILED;
  }
  else
  {
    *bytes_written_ptr = (uint32_t)cnt;
    return SNS_SUCCESS;
  }
}

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
sns_err_code_e sns_fsa_close ( void* file_handle )
{
  int32_t err = SNS_SUCCESS;

  /* TODO: fclose is not working for some reason; leaving file open for now */
  UNREFERENCED_PARAMETER(file_handle);
  /* err = fclose( (FILE*)file_handle ); */
  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_FSA, "skipping fclose" );
  if( 0 != err )
  {
    return SNS_ERR_FAILED;
  }
  else
  {
    return SNS_SUCCESS;
  }
}

/*============================================================================

  FUNCTION:   sns_fsa_stat

  ============================================================================*/
sns_err_code_e sns_fsa_stat( const char* pathname, sns_fsa_stat_s *fsa_stat )
{
  sns_err_code_e err = SNS_ERR_FAILED;
  struct stat linux_stat;

  if( pathname != NULL &&
      fsa_stat != NULL &&
      0 == stat( pathname, &linux_stat ) )
  {
    fsa_stat->size = linux_stat.st_size;
    err = SNS_SUCCESS;
  }

  return err;
}

/*============================================================================

  FUNCTION:   sns_fsa_flush

  ============================================================================*/
sns_err_code_e sns_fsa_flush( void* file_handle )
{
  int err;
  int file_descriptor;

  if( NULL == file_handle ) {
    return SNS_ERR_BAD_PTR;
  }

  err = fflush( (FILE*)file_handle );
  if ( 0 != err )
  {
    return SNS_ERR_FAILED;
  }
  else
  {
    file_descriptor = fileno( (FILE*)file_handle );
    err = fdatasync( file_descriptor );
    if( 0 == err )
    {
      return SNS_SUCCESS;
    }
    else
    {
      return SNS_ERR_FAILED;
    }
  }
}

/*============================================================================

  FUNCTION:   sns_fsa_mkdir

  ============================================================================*/
sns_err_code_e sns_fsa_mkdir( const char* pathname )
{
  int err;

  err = mkdir( pathname, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
  if( 0 != err )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_FSA, "mkdir failed, %i",
                               errno );
    return SNS_ERR_FAILED;
  }

  return SNS_SUCCESS;
}

/*============================================================================

  FUNCTION:   sns_fsa_chown

  ============================================================================*/
sns_err_code_e sns_fsa_chown( const char* pathname, const char* usr )
{
  int err;
  struct passwd *sensors_pwent;
  sns_err_code_e rv = SNS_SUCCESS;

  if( NULL != usr )
  {
    sensors_pwent = getpwnam( usr );
  }
  else
  {
    sensors_pwent = getpwnam( SNS_USERNAME );
    if( NULL == sensors_pwent )
    {
      sensors_pwent = getpwnam( USERNAME_NOBODY );
    }
  }

  if( NULL == sensors_pwent )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_FSA, "Unable to find user" );
    rv = SNS_ERR_FAILED;
  }
  else
  {
    err = chown( pathname, sensors_pwent->pw_uid, -1 );
    if( 0 != err )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_FSA, "chown failed, %i", errno );
      rv = SNS_ERR_FAILED;
    }
  }
  return rv;
}

/*============================================================================

  FUNCTION:   sns_fsa_chgrp

  ============================================================================*/
sns_err_code_e sns_fsa_chgrp( const char* pathname, const char* grp )
{
  int err;
  struct group *sensors_grent;
  sns_err_code_e rv = SNS_SUCCESS;

  if( NULL != grp )
  {
    sensors_grent = getgrnam( grp );
  }
  else
  {
    sensors_grent = getgrnam( SNS_GROUPNAME );
    if( NULL == sensors_grent )
    {
      sensors_grent = getgrnam( GROUPNAME_NOBODY );
    }
  }

  if( NULL == sensors_grent )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_FSA, "Unable to find group" );
    rv = SNS_ERR_FAILED;
  }
  else
  {
    err = chown( pathname, -1, sensors_grent->gr_gid );
    if( 0 != err )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_FSA, "chown failed, %i", errno );
      rv = SNS_ERR_FAILED;
    }
  }
  return rv;
}
