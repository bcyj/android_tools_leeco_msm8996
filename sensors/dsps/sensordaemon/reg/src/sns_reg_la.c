/*============================================================================
  @file sns_reg_la.c

  @brief Implementation of registry functions for Linux Android

  <br><br>

  DEPENDENCIES: This uses file system abstraction defined in sns_fsa.h.

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

#include "sensor1.h"
#include "sns_common.h"
#include "sns_debug_str.h"
#include "sns_fsa.h"
#include "sns_main.h"
#include "sns_memmgr.h"
#include "sns_reg.h"
#include "sns_reg_api_v02.h"
#include "sns_reg_priv.h"
#include "sns_reg_platform.h"

#include <stdio.h>
#include <sys/stat.h>

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/



/*============================================================================
 * Global Data Definitions
 ============================================================================*/
/**
 * OS-dependent pathname of the registry file
 */
static char sns_reg_filename[255] = SNS_REG_DATA_FILENAME;


/**
 * File handle for the registry file
 */
static void* reg_file_handle;

/*============================================================================
  Function Definitions and Documentation
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sns_reg_read

  ===========================================================================*/
/*!
  @brief Processs a registry read command

  @param[i] buffer_ptr: pointer to buffer where data read from file is stored
  @param[i] offset: offset from the beginning of the file where data should be
                    read
  @param[i] num_butes: number of bytes to be read

  @return
   - SNS_SUCCESS if read is successful.
   - All other values indicate an error has occurred.

  @sideeffects
  Reads data from the registry file.
  Data read from file will be written to memory pointed to by buffer_ptr.
*/
/*=========================================================================*/
sns_err_code_e
sns_reg_read( uint8_t* buffer_ptr, uint32_t offset,
              uint16_t num_bytes )
{
  sns_err_code_e err = SNS_ERR_FAILED;
  uint32_t        num_bytes_read = 0;
#ifdef SNS_REG_TEST
  uint16_t        i;
  uint8_t*        byte_ptr;
#endif

  SNS_PRINTF_STRING_LOW_2( SNS_MODULE_APPS_REG, "reg read: offset %d, num bytes: %d",
                           offset, num_bytes );

  if( NULL == reg_file_handle )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_REG,
        "reg_read: reg file not open" );
    return SNS_ERR_FAILED;
  }

  /* start at offset bytes from the beginning of file */
  err = sns_fsa_seek( reg_file_handle, (int64_t)offset, 0 );

  if( SNS_ERR_FAILED != err )
  {
    err = sns_fsa_read( reg_file_handle, buffer_ptr, (uint32_t)num_bytes,
                        &num_bytes_read );
  }

#ifdef SNS_REG_TEST
  /* display data read */
  SNS_PRINTF_STRING_LOW_2( SNS_MODULE_APPS_REG,
                           "sns_reg_read: offset %d, num bytes: %d",
                           offset, num_bytes );
  byte_ptr = buffer_ptr;
  for( i = 0; i < num_bytes; i++ )
  {
    SNS_PRINTF_STRING_LOW_1( SNS_MODULE_APPS_REG, "data: 0x%x", *(byte_ptr) );
    byte_ptr++;
  }
#endif

  if(( SNS_SUCCESS != err ) || ( num_bytes_read != num_bytes ))
  {
    SNS_PRINTF_STRING_ERROR_3( SNS_MODULE_APPS_REG,
                               "read error, err:%d, attempted:%d, read:%d",
                               err, num_bytes, num_bytes_read );
    err = SNS_ERR_FAILED;
  }

  return err;
}

/*============================================================================

  FUNCTION:   sns_reg_setperm

  ============================================================================*/
/*!
  @brief Set the permissions of the registry

  @return
  SNS_ERR_FAILED if operation failed; SNS_SUCCESS otherwise

*/
/*============================================================================*/
sns_err_code_e sns_reg_setperm( void )
{
  const char *pathname = sns_reg_filename;
  int mode = SNS_REG_FILE_MASK;

  sns_err_code_e err = SNS_SUCCESS;
  if( NULL == pathname )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_REG, "pathname is NULL" );
    err = SNS_ERR_FAILED;
  }
  else if( 0 != chmod( pathname, mode ) )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_REG, "chmod failed" );
    err = SNS_ERR_FAILED;
  }
  return err;
}

/*===========================================================================

  FUNCTION:   sns_reg_write

  ===========================================================================*/
/*!
  @brief Writes data to the registry file in response to a registry write
         command

  @param[i] buffer_ptr: pointer to buffer containing data to be written
  @param[i] offset: offset from the beginning of the file where data should be
                    written
  @param[i] num_butes: number of bytes to be written
  @param[i] do_flush: If a file flush should be done after the write.

  @return
   - SNS_SUCCESS if write is successful.
   - All other values indicate an error has occurred.

  @sideeffects
  Writes data to the registry file.
*/
/*=========================================================================*/
sns_err_code_e sns_reg_write( uint8_t const *buffer_ptr, uint32_t offset,
                              uint16_t num_bytes, bool do_flush )
{
  uint32_t        num_bytes_written;
  sns_err_code_e  err;
#ifdef SNS_REG_TEST
  uint16_t        i;
  uint8_t*        byte_ptr;
#endif

  SNS_PRINTF_STRING_LOW_2( SNS_MODULE_APPS_REG, "reg write: offset %d, num bytes: %d",
                     offset, num_bytes );

  if( NULL == reg_file_handle )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_REG,
        "reg write: reg file not open" );
    return SNS_ERR_FAILED;
  }

  /* start at offset bytes from the beginning of file */
  err = sns_fsa_seek( reg_file_handle, (int64_t)offset, 0 );

  if( SNS_SUCCESS == err )
  {
    SNS_PRINTF_STRING_LOW_0( SNS_MODULE_APPS_REG, "reg write: fsa_write");
    err = sns_fsa_write( reg_file_handle, buffer_ptr, (uint32_t)num_bytes,
                         &num_bytes_written );
  }
  if( true == do_flush )
  {
    err = sns_fsa_flush( reg_file_handle );
  }

#ifdef SNS_REG_TEST
  /* display data written */
  SNS_PRINTF_STRING_LOW_2( SNS_MODULE_APPS_REG,
                           "sns_reg_write: offset %d, num bytes: %d",
                           offset, num_bytes );
  byte_ptr = buffer_ptr;
  for( i = 0; i < num_bytes; i++ )
  {
    SNS_PRINTF_STRING_LOW_1( SNS_MODULE_APPS_REG, "data: 0x%x", *(byte_ptr) );
    byte_ptr++;
  }
#endif

  if( ( SNS_SUCCESS != err ) || ( num_bytes_written != num_bytes ) )
  {
    SNS_PRINTF_STRING_ERROR_3( SNS_MODULE_APPS_REG,
                       "write error, err:%d, attempted:%d, written:%d",
                       err, num_bytes, num_bytes_written );
    err = SNS_ERR_FAILED;
  }
  return err;
}

/*===========================================================================

  FUNCTION:   sns_reg_storage_init

  ===========================================================================*/
/*!
  @brief If registry file does not already exist, this function creates the
         registry file and writes default values to it. This function should be
         called only during system initialization.

  @return
   - SNS_SUCCESS if file initialization is successful.
   - All other values indicate an error has occurred.

  @sideeffects
  Registry file will be created and populated with default values if it does not
  already exist.
*/
/*=========================================================================*/
sns_err_code_e
sns_reg_storage_init( void )
{
  void*           group_ptr = NULL;
  uint16_t        group_count = sns_reg_get_group_count();
  int_fast16_t    i;
  sns_err_code_e  err = SNS_SUCCESS;
  sns_fsa_stat_s  stat;
  reg_file_handle = NULL;

#ifdef SNS_LA_SIM
  {
    /* For the LA Simulator, append the TID & a random number to the end of the filename */
    char postpend[80];
    snprintf( postpend, sizeof(postpend), ".%s", getlogin() );
    strncat( sns_reg_filename, postpend, sizeof(sns_reg_filename) );
  }
#endif /* SNS_LA_SIM */

  stat.size = 0;
  err = sns_fsa_stat( sns_reg_filename, &stat );

  if( err != SNS_SUCCESS || stat.size == 0 )
  { /* file does not exist, or is size 0 */
    SNS_PRINTF_STRING_LOW_0( SNS_MODULE_APPS_REG, "Create reg file" );
    reg_file_handle = sns_fsa_open( sns_reg_filename, "w+" );

    if( NULL == reg_file_handle )
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_REG,
          "Error creating registry file" );
      return( SNS_ERR_FAILED );
    }
    else if( SNS_SUCCESS != sns_reg_setperm() )
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_REG, "sns_reg_setperm failed" );
      return( SNS_ERR_FAILED );
    }
  }
  else
  { /* If file already exists */
    reg_file_handle = sns_fsa_open( sns_reg_filename, "r+" );

    if( NULL == reg_file_handle )
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_REG,
          "Error opening registry file" );
      return( SNS_ERR_FAILED );
    }
  }

  /* Add groups that do not exist in current file */
  for( i = 0; i < group_count; i++ )
  {
    if( 0 == stat.size || sns_reg_group_info[i].offset > stat.size )
    {
      SNS_PRINTF_STRING_LOW_3( SNS_MODULE_APPS_REG,
                               "sns_reg_storage_init, grp %d (%d of %d)",
                               sns_reg_group_info[i].group_id,
                               i + 1, group_count );

      group_ptr = SNS_OS_MALLOC(SNS_DBG_MOD_APPS_REG, sns_reg_group_info[i].size );
      if( NULL == group_ptr )
      {
        return( SNS_ERR_NOMEM );
      }
      err = sns_group_data_init( sns_reg_group_info[i].group_id, group_ptr );
      if( SNS_SUCCESS != err )
      {
        /* If defaults are not specified, initialize to zero */
        SNS_OS_MEMSET( group_ptr, 0, sns_reg_group_info[i].size );
      }
      err = sns_reg_write( group_ptr,
                           sns_reg_group_info[i].offset,
                           sns_reg_group_info[i].size,
                           false );
      SNS_OS_FREE( group_ptr );
      if( SNS_SUCCESS != err )
      {
        SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_REG, "sns_reg_write failed, %d", i );
        return( SNS_ERR_FAILED );
      }
    }
  }

  if( true != sns_reg_update( stat.size ) )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_REG,
        "sns_reg_update failed" );
    return( SNS_ERR_FAILED );
  }
  return( SNS_SUCCESS );
}


/*===========================================================================

  FUNCTION:   sns_reg_storage_deinit

  ===========================================================================*/
/*!
  @brief Deinitializes registry.

*/
/*=========================================================================*/
sns_err_code_e
sns_reg_storage_deinit( void )
{
    return( SNS_SUCCESS );
}

