/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

    B L U E T O O T H   B T S   N V M   E F S C O N F I G  M O D E  

GENERAL DESCRIPTION
  This file implements Bluetooth Platform specific initialization code

Copyright (c) 2009-2013 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: 
  $DateTime: 
  $Author: 

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2011-03-01   rr  Deprecating unused formal paramters to parser routines.
  2010-03-26   rr  Runtime NVM mode selection for initialising NVM configuration
  2009-07-28  dgh  Read file into memory on heap.  Removed unused functions.
  2009-07-01   sa  Redesign NVM System.
  2009-05-07   sa  Support for EFS NVM Mode.
  2008-03-04   sa  Initial version.


===============================================================================*/

#include "btqsocnvmplatform.h"
#include "btqsocnvm.h"
#include "btqsocnvmprivate.h"


/*=========================================================================*/
/*                               TYPEDEFS                                  */
/*=========================================================================*/


/*===========================================================================
                                 Globals
===========================================================================*/


/*==============================================================
FUNCTION:  bt_qsoc_nvm_init_fs_mode
==============================================================*/
/**
  Initialize the parser for reading a NVM file.
  
  @see bt_qsoc_nvm_open
  
  @return  boolean: True: If initialized successfully.
                    False: otherwise.
*/
boolean bt_qsoc_nvm_init_fs_mode( void )
{

  boolean r_val = FALSE;
  char *nvm_string_ptr = NULL;
  int file_size = 0;
  int num_bytes_read = 0;

  if( btqsocnvmplatform_open_file() != FALSE )
  {
    r_val = btqsocnvmplatform_get_file_size(&file_size);

    if( FALSE != r_val )
    {
      nvm_string_ptr = btqsocnvmplatform_malloc(file_size + 1);
    }

    if( NULL != nvm_string_ptr && FALSE != r_val )
    {
      num_bytes_read = btqsocnvmplatform_read_file(
        (void *) nvm_string_ptr,file_size);
      nvm_string_ptr[file_size] = '\0';
      if( num_bytes_read <= 0 )
      {
        r_val = FALSE;
      }
    }

    if( NULL != nvm_string_ptr && FALSE != r_val )
    { /* If there is NVM file data, initialize and parse it */
      r_val = bt_qsoc_nvm_init_parser();
      if( FALSE != r_val )
      {
        r_val = bt_qsoc_nvm_parse_nvm_file(nvm_string_ptr);
      }
    }
  }

  if( NULL != nvm_string_ptr )
  {
    btqsocnvmplatform_free(nvm_string_ptr);
  }

  return r_val;
} /* bt_qsoc_nvm_init_fs_mode */

