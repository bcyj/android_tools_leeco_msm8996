/*============================================================================
  @file sns_reg.c

  @brief
    This is the implementation of the Sensors Registry. It allows clients to
    read to and read from the registry stored in the file system on the
    applications processor.

  <br><br>

  DEPENDENCIES: This uses file system abstraction defined in sns_fsa.h.

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================
  INCLUDE FILES
  ============================================================================*/
#include "sensor1.h"
#include "sns_common.h"
#include "fixed_point.h"
#include "sns_debug_str.h"
#include "sns_osa.h"
#include "sns_fsa.h"
#include "sns_smr_util.h"
#include "sns_memmgr.h"
#include "sns_init.h"
#include "sns_reg.h"
#include "sns_pwr.h"
#include "sns_reg_common.h"
#include "sns_reg_api_v02.h"
#include "sns_reg_priv.h"
#include "sns_reg_platform.h"
#include <stdbool.h>

/*============================================================================
  Static Variable Definitions
  ============================================================================*/

/**
 * Table allowing quick conversion from data type to size
 */
const uint8_t sns_reg_item_size[ SNS_REG_TYPE_COUNT ] =
{
  sizeof( uint8_t ),  /* SNS_REG_TYPE_UINT8  */
  sizeof( uint16_t ), /* SNS_REG_TYPE_UINT16 */
  sizeof( uint32_t ), /* SNS_REG_TYPE_UINT32 */
  sizeof( uint64_t ), /* SNS_REG_TYPE_UINT64 */
  sizeof( int8_t ),   /* SNS_REG_TYPE_INT8   */
  sizeof( int16_t ),  /* SNS_REG_TYPE_INT16  */
  sizeof( int32_t ),  /* SNS_REG_TYPE_INT32  */
  sizeof( int64_t ),  /* SNS_REG_TYPE_INT64  */
  sizeof( q16_t ),    /* SNS_REG_TYPE_Q16    */
  sizeof( float ),    /* SNS_REG_TYPE_FLOAT  */
  sizeof( double )    /* SNS_REG_TYPE_COUNT  */
};

/*============================================================================
  Static Function Definitions and Documentation
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sns_reg_get_reg_settings

  ===========================================================================*/
/*!
  @brief Reads settings info from the registry

  @param settings[o] - Settings struct
  @param reg_size[i] - Size of the registry

  @return
   - true if read correctly
   - false indicates an error has occurred.
*/
/*=========================================================================*/
static bool
sns_reg_get_reg_settings( sns_reg_settings_group_s *settings, int32_t reg_size )
{
  uint16_t       offset;
  uint16_t       data_len;
  sns_err_code_e err;
  bool           rv    = true;
  int            index = sns_reg_lookup_group_index( SNS_REG_GROUP_SETTINGS_V02 );

  if( -1 == index )
  {
    rv = false;
  } else {
    offset = sns_reg_group_info[index].offset;
    if( offset > reg_size )
    {
      rv = false;
    } else {
      data_len = sns_reg_group_info[index].size;
      err = sns_reg_read( (uint8_t*)settings, offset, data_len );

      if( SNS_SUCCESS != err )
      {
        SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_REG,
                                   "sns_reg_read failed; err %d", err );
        rv = false;
      }
      else if( SNS_REG_SETTINGS_MG_WORD != settings->magic_word )
      {
        SNS_PRINTF_STRING_ERROR_2( SNS_MODULE_APPS_TIME, "invalid magic word: %X%X",
                                   (uint32_t)(settings->magic_word >> 32),
                                   (uint32_t)(settings->magic_word & 0xFFFFFFFF) );
        rv = false;
      }
    }
  }
  return rv;
}

/*===========================================================================

  FUNCTION:   sns_reg_update_internal_defaults

  ===========================================================================*/
/*!
  @brief Updates the registry to hard-coded default values found internal
  to the sensor daemon.

  @param reg_size[i] - Size of registry data storage

  @return
   - true if update is successful.
   - false indicates an error has occurred.
*/
/*=========================================================================*/
static bool
sns_reg_update_internal_defaults( uint32_t file_version_major,
                                       uint32_t file_version_minor )
{
  sns_err_code_e err;
  bool rv = true;

  if( SNS_REG2_SVC_V02_IDL_MAJOR_VERS != file_version_major ||
      SNS_REG2_SVC_V02_IDL_MINOR_VERS != file_version_minor )
  {
    void *item_ptr = NULL;
    int i, item_count = sns_reg_get_item_count();
    for( i = 0; i < item_count; i++ )
    {
      if( sns_reg_item_info[i].version_major > file_version_major ||
          (sns_reg_item_info[i].version_major == file_version_major &&
           sns_reg_item_info[i].version_minor > file_version_minor) )
      {
        SNS_PRINTF_STRING_LOW_3( SNS_MODULE_APPS_REG,
                                 "sns_reg_update, item %d (v. %d,%d)",
                                 sns_reg_item_info[i].item_id,
                                 sns_reg_item_info[i].version_major,
                                 sns_reg_item_info[i].version_minor);

        item_ptr = SNS_OS_MALLOC( SNS_DBG_MOD_APPS_REG,
                                  sns_reg_item_size[ sns_reg_item_info[i].type ] );
        if( NULL == item_ptr )
        {
          rv = false;
          break;
        }
        err = sns_reg_get_default( sns_reg_item_info[i].item_id, item_ptr );
        if( SNS_SUCCESS != err )
        {
          /* If defaults are not specified, initialize to zero */
          SNS_OS_MEMSET( item_ptr, 0, sns_reg_item_size[ sns_reg_item_info[i].type ] );
        }
        err = sns_reg_write( item_ptr,
                             sns_reg_item_info[i].offset,
                             sns_reg_item_size[ sns_reg_item_info[i].type ],
                             false );
        SNS_OS_FREE( item_ptr );
        if( SNS_SUCCESS != err )
        {
          SNS_PRINTF_STRING_ERROR_2( SNS_MODULE_APPS_REG,
                                     "sns_reg_write failed, item %d, err %d",
                                     i, err );
          rv = false;
          break;
        }
      }
    }
  }
  return rv;
}

/*============================================================================
  Public Function Definitions
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sns_reg_lookup_item_index

  ===========================================================================*/
int
sns_reg_lookup_item_index( uint16_t item_id )
{
  int i = sns_reg_get_item_count() - 1;
  while( (i >= 0) && (item_id != sns_reg_item_info[i].item_id) )
  {
    i--;
  }
  return i;
}

/*===========================================================================

  FUNCTION:   sns_reg_lookup_group_index

  ===========================================================================*/
int
sns_reg_lookup_group_index( uint16_t group_id )
{
  int i = sns_reg_get_group_count() - 1;
  while( (i >= 0) && (group_id != sns_reg_group_info[i].group_id) )
  {
    i--;
  }
  return i;
}

/*===========================================================================

  FUNCTION:   sns_reg_handle_req

  ===========================================================================*/
sns_err_code_e
sns_reg_handle_req( int entry_id, bool is_read, bool is_item, uint16_t *req_data_len, void *data )
{
  int index;
  sns_err_code_e err = SNS_SUCCESS,
                 rv = SNS_SUCCESS;
  uint16_t offset = 0,
           data_len = 0;

  SNS_PRINTF_STRING_LOW_3( SNS_MODULE_APPS_REG,
                           "entry id: %d, is_read: %d, is_item: %d",
                           entry_id, is_read, is_item);

  if( is_item )
  {
    index = sns_reg_lookup_item_index( entry_id );
  }
  else
  {
    index = sns_reg_lookup_group_index( entry_id );
  }

  if( index < 0 )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_REG, "Invalid ID: %i", entry_id );
    rv = SNS_ERR_BAD_PARM;
  }
  else
  {
    if( is_item )
    {
      offset = sns_reg_item_info[index].offset;
      data_len = sns_reg_item_size[ sns_reg_item_info[index].type ];
    }
    else
    {
      offset = sns_reg_group_info[index].offset;
      data_len = sns_reg_group_info[index].size;
    }

    if( is_read )
    {
      err = sns_reg_read( data, offset, data_len );
    }
    else if( data_len != *req_data_len )
    {
      SNS_PRINTF_STRING_ERROR_2( SNS_MODULE_APPS_REG,
                                 "invalid data len: %d (%d)",
                                 req_data_len, data_len );
      rv = SNS_ERR_BAD_PARM;
    }
    else
    {
      err = sns_reg_write( data, offset, data_len, true );
    }

    if( SNS_SUCCESS != err )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_REG,
                                 "Received error from sns_reg_read/write %i",
                                 err );
      rv = SNS_ERR_FAILED;
    }
    else
    {
      *req_data_len = data_len;
    }
  }

  return rv;
}

/*===========================================================================

  FUNCTION:   sns_reg_update

  ===========================================================================*/
bool
sns_reg_update( int32_t reg_size )
{
  bool rv = true;
  sns_reg_settings_group_s settings;
  sns_err_code_e err;
  uint32_t file_version_major = 0, file_version_minor = 0;

  SNS_OS_MEMSET( &settings, 0, sizeof(settings) );

  if( false == sns_reg_get_reg_settings( &settings, reg_size ) )
  {
    file_version_major = 2;
    file_version_minor = 1;
  }
  else
  {
    file_version_major = settings.version_major;
    file_version_minor = settings.version_minor;
  }
  SNS_PRINTF_STRING_LOW_2( SNS_MODULE_APPS_REG, "Found version %d, %d",
                           file_version_major, file_version_minor );
  rv = sns_reg_update_internal_defaults( file_version_major, file_version_minor );
  if( rv )
  {
    uint16_t       offset;
    int            index = sns_reg_lookup_group_index( SNS_REG_GROUP_SETTINGS_V02 );

    if( index != -1 ) {

      offset = sns_reg_group_info[index].offset;

      SNS_PRINTF_STRING_LOW_2( SNS_MODULE_APPS_REG, "Upgraded to version %d, %d",
                               SNS_REG2_SVC_V02_IDL_MAJOR_VERS, SNS_REG2_SVC_V02_IDL_MINOR_VERS );

      settings.version_major = SNS_REG2_SVC_V02_IDL_MAJOR_VERS;
      settings.version_minor = SNS_REG2_SVC_V02_IDL_MINOR_VERS;
      settings.magic_word    = SNS_REG_SETTINGS_MG_WORD;
      err = sns_reg_write( (const uint8_t*)&settings, offset, sizeof(settings), false );
      if( SNS_SUCCESS != err )
      {
        SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_REG,
                                   "sns_reg_write failed; err %d", err );
        rv = false;
      }
    } else {
        SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_REG,
                                   "can't find index of settings group" );
      rv = false;
    }
  }

  if( rv )
  {
    rv = sns_reg_update_external_defaults( &settings );
  }
  return rv;
}

/*============================================================================
  Externalized Function Definitions
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sns_reg_init

  ===========================================================================*/
sns_err_code_e
sns_reg_init( void )
{
  sns_err_code_e err;
  uint8_t        os_err;

  err = sns_reg_mr_init();
  if( SNS_SUCCESS != err )
  {
    SNS_PRINTF_STRING_FATAL_1( SNS_MODULE_APPS_REG, "mr init failed %i", err );
    return SNS_ERR_FAILED;
  }

  os_err = sns_os_task_create( sns_reg_mr_thread, NULL, NULL, SNS_MODULE_PRI_APPS_REG );
  if( OS_ERR_NONE != os_err )
  {
    SNS_PRINTF_STRING_FATAL_0( SNS_MODULE_APPS_REG, "init: task create failed" );
    return( SNS_ERR_FAILED );
  }

  return( SNS_SUCCESS );
}

/*===========================================================================

  FUNCTION:   sns_reg_deinit

  ===========================================================================*/
sns_err_code_e
sns_reg_deinit( void )
{
  uint8_t err = 0;

  err = sns_reg_mr_deinit();
  if( SNS_SUCCESS != err )
  {
    SNS_PRINTF_STRING_FATAL_1( SNS_MODULE_APPS_REG, "mr deinit failed %i", err );
    return SNS_ERR_FAILED;
  }

  sns_os_task_del_req( SNS_MODULE_PRI_APPS_REG );

  return SNS_SUCCESS;
}
