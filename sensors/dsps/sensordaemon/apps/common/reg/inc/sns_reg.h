#ifndef _SNS_REG_H_
#define _SNS_REG_H_
/*============================================================================
  @file sns_reg.h

  @brief
  Header file for the Sensors Registry.

  <br><br>

  DEPENDENCIES: None.

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
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
  2012-03-29 gju Added version info to reg_item struct.
  2011-3-31  dc  updates to group and item structures per revised design
  2010-11-30 jh  Initial revision

  ============================================================================*/

/*============================================================================

  INCLUDE FILES

  ============================================================================*/
#include "sns_common.h"

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

typedef enum
{
  SNS_REG_TYPE_UINT8,
  SNS_REG_TYPE_UINT16,
  SNS_REG_TYPE_UINT32,
  SNS_REG_TYPE_UINT64,
  SNS_REG_TYPE_INT8,
  SNS_REG_TYPE_INT16,
  SNS_REG_TYPE_INT32,
  SNS_REG_TYPE_INT64,
  SNS_REG_TYPE_Q16,
  SNS_REG_TYPE_FLOAT,
  SNS_REG_TYPE_DOUBLE,
  SNS_REG_TYPE_COUNT   /* number of data types supported */
} sns_reg_data_type_e;

typedef struct
{
  sns_reg_data_type_e type;
  uint16_t            offset;
  uint32_t            version_major;
  uint32_t            version_minor;
  uint16_t            item_id;
} sns_reg_item_info_s;

typedef struct
{
  uint16_t size;
  uint16_t offset;
  uint16_t group_id;
} sns_reg_group_info_s;

extern const sns_reg_item_info_s  sns_reg_item_info[];
extern const sns_reg_group_info_s sns_reg_group_info[];
extern const uint8_t              sns_reg_item_size[];

/*===========================================================================

  FUNCTION:   sns_reg_get_default

  ===========================================================================*/
/*!
  @brief Retrieve the default value of data items.

  @param[i] index: registry data item index.
  @param[i] data_ptr: pointer to the memory location where default value is to
                      be written.

  @return
  SNS_SUCCESS if item is found, SNS_ERR_FAILED otherwise.
*/
/*=========================================================================*/
sns_err_code_e sns_reg_get_default( uint16_t item, void* data_ptr);

/*===========================================================================

  FUNCTION:   sns_group_data_init

  ===========================================================================*/
/*!
  @brief Retrieve the default values of all data items in data groups.

  @param[i] group_id: registry data group id.
  @param[i] data_ptr: pointer to the data group structure where default values
                      are to be written.

  @return
  SNS_SUCCESS if group is found, SNS_ERR_FAILED otherwise.
*/
/*=========================================================================*/
sns_err_code_e sns_group_data_init( uint16_t group_id, void *data_ptr);

/*===========================================================================

  FUNCTION:   sns_reg_get_item_count

  ===========================================================================*/
/*!
  @brief Retrieves the number of data items defined

  @param None

  @return
  Number of entries in sns_reg_item_info[]
*/
/*=========================================================================*/
uint16_t sns_reg_get_item_count(void);

/*===========================================================================

  FUNCTION:   sns_reg_get_group_count

  ===========================================================================*/
/*!
  @brief Retrieves the number of group items defined

  @param None

  @return
  Number of entries in sns_reg_group_info[]
*/
/*=========================================================================*/
uint16_t sns_reg_get_group_count(void);

#endif /* _SNS_REG_H_ */
