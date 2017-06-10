#ifndef SNS_REG_PRIV_H
#define SNS_REG_PRIV_H

/*============================================================================
  @file sns_reg_priv.h

  @brief Private include file for the registry service.

  <br><br>

  DEPENDENCIES:

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

#include <stdbool.h>
#include <stdint.h>
#include "sns_reg_common.h"

/*============================================================================
  Type Declarations
  ============================================================================*/

typedef struct
{
  uint32_t  version_major;
  uint32_t  version_minor;
  uint16_t  settings_group_id;
} sns_reg_mr_info_s;

/*===========================================================================

  FUNCTION:   sns_reg_mr_info

  ===========================================================================*/
/*!
  @brief Accessor function for general message routing information.

  @return IDL info
*/
/*=========================================================================*/
sns_reg_mr_info_s sns_reg_mr_info();

/*===========================================================================

  FUNCTION:   sns_reg_mr_init

  ===========================================================================*/
/*!
  @brief Initialize the message routing system for the Sensors Registry.

  @return SNS_ERR_FAILED if an internal or QMI error occurred; SNS_SUCCESS otherwise.
*/
/*=========================================================================*/
sns_err_code_e sns_reg_mr_init();

/*===========================================================================

  FUNCTION:   sns_reg_mr_deinit

  ===========================================================================*/
/*!
  @brief Deinitialize the message routing system for the Sensors Registry.

  @return SNS_ERR_FAILED if an internal or QMI error occurred; SNS_SUCCESS otherwise.
*/
/*=========================================================================*/
sns_err_code_e sns_reg_mr_deinit();

/*===========================================================================

  FUNCTION:   sns_reg_mr_thread

  ===========================================================================*/
/*!
  @brief Thread loop to process incoming requests to the sensors registry.

  @param[i] p_arg Not used
*/
/*=========================================================================*/
void sns_reg_mr_thread( void* );

/*===========================================================================

  FUNCTION:   sns_reg_handle_req

  ===========================================================================*/
/*!
  @brief Handle a registry read or write request.

  @param[i] entry_id: Entry ID (item or group) to read/write
  @param[i] is_read: True if read, false if write
  @param[i] is_item: True if item, false if group
  @param[io] req_data_len: Output read length; or input write length
  @param[io] data: Read data output; write data input

  @return SNS_SUCCESS
          SNS_ERR_BAD_PARM - Item/group ID not found; incorrect write length
          SNS_ERR_FAILED - Read/write operation failed
*/
/*=========================================================================*/
sns_err_code_e sns_reg_handle_req( int entry_id, bool is_read, bool is_item,
                                   uint16_t *req_data_len, void *data );

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
sns_err_code_e sns_reg_read( uint8_t* buffer_ptr, uint32_t offset,
                             uint16_t num_bytes );

/*============================================================================

  FUNCTION:   sns_reg_setperm

  ============================================================================*/
/*!
  @brief Set the permissions of the registry

  @return
  SNS_ERR_FAILED if operation failed; SNS_SUCCESS otherwise

*/
/*============================================================================*/
sns_err_code_e sns_reg_setperm( void );


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
                              uint16_t num_bytes, bool do_flush );

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
sns_err_code_e sns_reg_storage_init( void );

/*===========================================================================

  FUNCTION:   sns_reg_storage_deinit

  ===========================================================================*/
/*!
  @brief De-initializes registry storage, but retains data in storage.

  @return
   - SNS_SUCCESS if file initialization is successful.
   - All other values indicate an error has occurred.

*/
/*=========================================================================*/
sns_err_code_e sns_reg_storage_deinit( void );

/*===========================================================================

  FUNCTION:   sns_reg_orientation_init

  ===========================================================================*/
/*!
  @brief Sets sensor orientations based on platform type.

  @return
   - TRUE if orientation initialization is successful.
   - FALSE indicates an error has occurred.
*/
/*=========================================================================*/
bool sns_reg_orientation_init();

/*===========================================================================

  FUNCTION:   sns_reg_update

  ===========================================================================*/
/*!
  @brief Checks for current registry file version and updates to current
         version if necessary.

  @param reg_size[i] - Size of registry data storage

  @return
   - true if update is successful.
   - false indicates an error has occurred.
*/
/*=========================================================================*/
bool sns_reg_update( int32_t reg_size );

/*===========================================================================

  FUNCTION:   sns_reg_update_external_defaults

  ===========================================================================*/
/*!
  @brief Updates default values in the sensors registry via external library.

  @side effects
  In addition to writing the new defaults to the registry, it may write to
  the settings to update the library names and versions.

  @return
   - true if update is successful.
   - false indicates an error has occurred.
*/
/*=========================================================================*/
bool sns_reg_update_external_defaults( sns_reg_settings_group_s *settings );

/*===========================================================================

  FUNCTION:   sns_reg_lookup_group_index

  ===========================================================================*/
/*!
  @brief Lookup the info associated with a registry group.

  @param[i] group_id: group number as specified in sns_reg_api

  @return
  Index into sns_reg_group_info if group id found, otherwise -1.
*/
/*=========================================================================*/
int sns_reg_lookup_group_index( uint16_t group_id );

/*===========================================================================

  FUNCTION:   sns_reg_lookup_item_index

  ===========================================================================*/
/*!
  @brief Lookup the info associated with a registry item.

  @param[i] item_id: Item number as specified in sns_reg_api

  @return
  Index into sns_reg_item_info if item id found, otherwise -1.
*/
/*=========================================================================*/
int
sns_reg_lookup_item_index( uint16_t item_id );
#endif /* SNS_REG_PRIV_H */
