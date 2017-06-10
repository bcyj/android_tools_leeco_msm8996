/**
  @file btqsocnvm.c

  This file contains the public code for the BT NVM system.
*/

/*--------------------------------------------------------------
     Copyright (c) 2008-2009 Qualcomm Technologies, Inc. 
     All rights reserved.
     Qualcomm Technologies Confidential and Proprietary
--------------------------------------------------------------*/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //source/qcom/qct/wconnect/bthost/soccfg/main/latest/src/btqsocnvm.c#53 $ 
  $DateTime: 2010/04/08 04:54:29 $ 
  $Author: roystonr $ 

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2010-03-26   rr  Runtime NVM mode selection for initialising NVM configuration
  2009-11-23  sbk  Changed aeestd.h to AEEstd.h to work with linux build
                   environment
  2009-11-06  dgh  Added the bt_qsoc_nvm_tot_hc_tx_len() function.
  2009-10-05  dgh  Fixed compiler warning.
  2009-09-02  dgh  Doxygenated file.  Cleaned up function documentation (see 
                   header file for full documentation).  Added hybrid mode.  
                   bt_qsoc_nvm_set_tag_sent() no longer takes a flag.
  2009-07-09   sa  Adding the Header, DateTime, Author information.
  2009-06-12   sa  NVM Redesign.
===============================================================================*/

#include <string.h>
#include "AEEstd.h"
#include "btqsocnvm.h"
#include "btqsocnvmprivate.h"

/*--------------------------------------------------------------
Type definitions
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Enumerated types
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Structure definitions
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Union definitions
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Macros
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Constant values
--------------------------------------------------------------*/

/*------------------------------------------------------------------------------
Globals
------------------------------------------------------------------------------*/

/** global variable containing the active NVM mode. 

  @note Default to auto-mode.
*/
static bt_qsoc_enum_nvm_mode bt_qsoc_nvm_mode = NVM_AUTO_MODE;

/** global variable to track when NVM system is opened or closed. */
static boolean bt_qsoc_nvm_opened = FALSE;

/** global varaible to store the active nvm configuration. */
static bt_qsoc_config_params_struct_type bt_qsoc_config_params;

/*--------------------------------------------------------------
Function declarations
--------------------------------------------------------------*/

/*==============================================================
FUNCTION:  copy_config_param
==============================================================*/
/**
  Make a copy of a configuration parameter structure.

  @see bt_qsoc_config_params_struct_type
  
  @return  BT_QSOC_NVM_STATUS_INVALID_PARAMETERS if param_ptr invalid.
  @return  BT_QSOC_NVM_STATUS_OK if copy suceeded.

  @sideeffects Undetermined.
*/
static bt_qsoc_nvm_status copy_config_param
(
  bt_qsoc_config_params_struct_type *param_ptr 
)
{
  bt_qsoc_nvm_status rval = BT_QSOC_NVM_STATUS_INVALID_PARAMETERS;
   
  if( NULL != param_ptr )
  {
    /* Since bt_qsoc_config_param is provided by external driver
     * we may have to copy the data locally */
    std_memmove(&bt_qsoc_config_params,param_ptr,sizeof(*param_ptr));

    rval = BT_QSOC_NVM_STATUS_OK;
  }

  return rval;
} /* copy_config_param */

/*==========================================================================

  FUNCTION       BT_QSOC_NVM_CLEAN_UP

  DESCRIPTION    This function will reset the NVM state. 

==========================================================================*/

/*==============================================================
FUNCTION:  nvm_cleanup
==============================================================*/
/**
  Cleanup any NVM system "state" information.

  @sideeffects Undetermined.
*/
static void nvm_cleanup
(
  void
)
{
  bt_qsoc_nvm_opened = FALSE;
} /* nvm_cleanup */

/*==============================================================
FUNCTION:  bt_qsoc_nvm_open
==============================================================*/
bt_qsoc_nvm_status bt_qsoc_nvm_open
(
  /** [in] QSOC type. */
  bt_qsoc_enum_type     qsoc_type,
  
  /** [in] NVM mode. */
  bt_qsoc_enum_nvm_mode mode,
  
  /** [in] Runtime configuration parameters. */
  bt_qsoc_config_params_struct_type *param_ptr  
)
{
   bt_qsoc_nvm_status  result;
   boolean ret_val;

   if(bt_qsoc_nvm_opened != FALSE)
   {
     return BT_QSOC_NVM_STATUS_ALREADY_INITIALIZED;
   }
   
   bt_qsoc_nvm_mode = mode;

   switch(mode)
   {
     case NVM_AUTO_MODE:

       result = copy_config_param(param_ptr);

       if( !BT_QSOC_NVM_IS_STATUS_OK (result) )
       {
          return result;
       }

       ret_val = bt_qsoc_nvm_init_auto_mode(qsoc_type, 
                                     &bt_qsoc_config_params);
       
       if( ret_val )
       {
         bt_qsoc_nvm_opened = TRUE;
         result = BT_QSOC_NVM_STATUS_SUCCESS;
       }
       else
       {
         result = BT_QSOC_NVM_STATUS_INIT_FAIILED;
       } 

     break;
     case NVM_EFS_MODE:
       ret_val = bt_qsoc_nvm_init_fs_mode();

       if( ret_val )
       {
         result = BT_QSOC_NVM_STATUS_SUCCESS;
       }
       else
       {
         result = BT_QSOC_NVM_STATUS_INIT_FAIILED;
       }
     break;

     case NVM_HYBRID_MODE:
      /*  Hybrid mode operation
          =====================
          
          Hybrid mode combines the operation of auto-mode with that of EFS-mode.
          In auto-mode there are three tables of NVM strings which are created:
          fixed, runtime, and dynamic.  In EFS-mode there is only a dynamic table
          which contains the strings from the NVM file.  In hybrid-mode the auto-mode 
          tables are created then the NVM file data is merged into the dynamic
          table.  If there is a conflict between an auto-mode tag and a tag from
          the NVM file, the NVM file will take precedence.
      */
            
       result = copy_config_param(param_ptr);

       if( BT_QSOC_NVM_IS_STATUS_OK(result) )
       {
         result = BT_QSOC_NVM_STATUS_INIT_FAIILED;
         ret_val = bt_qsoc_nvm_init_hybrid_mode(qsoc_type, 
                                       &bt_qsoc_config_params);
         
         if( FALSE != ret_val )
         {
           bt_qsoc_nvm_opened = TRUE;
           result = BT_QSOC_NVM_STATUS_SUCCESS;
         }
       }
     break;
     default:
       result = BT_QSOC_NVM_STATUS_NOT_IMPLEMENTED;
     break;
   }

   return result;

} /* bt_qsoc_nvm_open */

/*==============================================================
FUNCTION:  bt_qsoc_nvm_get_next_cmd
==============================================================*/
bt_qsoc_nvm_status bt_qsoc_nvm_get_next_cmd
(
  /** Pointer to a pointer to a NVM command string. */
  uint8 **soc_nvm_ptr
)
{
  bt_qsoc_nvm_status result = BT_QSOC_NVM_STATUS_INVALID_PARAMETERS;
  uint8 *nvm_ptr = NULL;

  if( NULL != soc_nvm_ptr )
  {
    *soc_nvm_ptr = NULL;

    if( bt_qsoc_nvm_get_next_nvm_tag( &nvm_ptr ) )
    {
      *soc_nvm_ptr = nvm_ptr;
      result = BT_QSOC_NVM_STATUS_SUCCESS;
    }
    else
    { /* IMPORTANT:  Need to return "complete" when we are out of NVM data. */
      result = BT_QSOC_NVM_STATUS_COMPLETE;
    }
  }

  return result;
} /* bt_qsoc_nvm_get_next_cmd */

/*==============================================================
FUNCTION:  bt_qsoc_nvm_close
==============================================================*/
bt_qsoc_nvm_status bt_qsoc_nvm_close()
{
  (void) bt_qsoc_nvm_close_nvm( bt_qsoc_nvm_mode );
  nvm_cleanup();
  return BT_QSOC_NVM_STATUS_SUCCESS;
} /* bt_qsoc_nvm_close */

/*==============================================================
FUNCTION:  bt_qsoc_nvm_tot_hc_tx_len
==============================================================*/
bt_qsoc_nvm_status bt_qsoc_nvm_tot_hc_tx_len(uint16 *tx_len_ptr)
{
  bt_qsoc_nvm_status result = BT_QSOC_NVM_STATUS_INVALID_PARAMETERS;
  
  if( NULL != tx_len_ptr )
  {
    if( bt_qsoc_config_params.num_tx_buf > 0 )
    { 
      *tx_len_ptr = bt_qsoc_config_params.num_tx_buf * bt_qsoc_config_params.max_hc_tx_len;
    }
    else
    {
      *tx_len_ptr = BT_QSOC_MAX_HC_NUM_TX_BUF * BT_QSOC_MAX_HC_TX_LEN;
    }
    result = BT_QSOC_NVM_STATUS_SUCCESS;
  }
  
  return result;
} /* bt_qsoc_nvm_tot_hc_tx_len */

/*------------------------------------------------------------------------------
Notes
------------------------------------------------------------------------------*/
