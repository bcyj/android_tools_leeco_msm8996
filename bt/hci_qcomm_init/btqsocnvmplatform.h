#ifndef _BTQSOCNVMPLATFORM_H
#define _BTQSOCNVMPLATFORM_H

/**
  @file btqsocnvmplatform.h
  
  Platform-specific declarations for the NVM system.  
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

$Header: //source/qcom/qct/wconnect/bthost/soccfg/main/latest/src/btqsocnvmplatform.h#14 $
$DateTime: 2009/10/06 10:38:12 $
$Author: dheil $

when        who  what, where, why
----------  ---  -----------------------------------------------------------
2009-10-05  dgh  Removed reference to comdef.h.
2009-09-02  dgh  Doxygenated.  Moved platform-independent constants to private header.
2009-07-28  dgh  Added functions for malloc, free, and determining file size.
2009-07-01   sa  Redesign NVM Subsystem.
2009-05-07   sa  Support for EFS NVM Mode.
2009-04-08  jtl  Change TX buffers to 5 buffers of 1021 bytes.
2009-02-23  jtl  Change TX buffers to 5 buffers of 1023 bytes.
2009-02-10   jn  Increase number of Rx buffers in BTS from 9 to 11.
*
*   #5          06 Feb 2009           SA  
*   Cleanup & Added Support for FTM_BT feature for WM targets 
*
*   #4          16 Sep 2008           SA
*   Added support for dynamic parsing of released nvm   
*
*   #3          22 May 2008           RH
*   Changed WM flag to UNDER_CE to avoid including "customer.h" (for WM7 compile)  
*
*   #2          20 Mar 2008           RH
*   Added Windows Mobile support using FEATURE_WINCE.  
*
*   #1         31 Jan  2008          BH
*   Created new file for AMSS BTS 4020 (QSoC) support.
===========================================================================*/

#include "btqsocnvmprivate.h"

#ifdef __cplusplus
extern "C"
{
#endif 

/**
  @defgroup btqsocvmplatform Bluetooth NVM System Platform-Specific Header File
  @{
*/

/*--------------------------------------------------------------
Classes
--------------------------------------------------------------*/

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

/*--------------------------------------------------------------
Global declarations
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Function declarations
--------------------------------------------------------------*/

/*==============================================================
FUNCTION:  btqsocnvmplatform_log_error
==============================================================*/
/**
  This function logs an error message based on an error code.

  @return  none
*/
void btqsocnvmplatform_log_error
(
  /** [in] An error code */
  btqsocnvm_parser_err_type parser_err
);

/*==============================================================
FUNCTION:  btqsocnvmplatform_find_file
==============================================================*/
/**
  This function returns a path to the NVM file on this platform.

  @see btqsocnvmplatform_open_file
  
  @return  boolean: TRUE: If string copy was sucessful.
                    FALSE: Otherwise.
*/
boolean btqsocnvmplatform_find_file
(
  /** [out] pointer to a buffer that will receive the filename. */
  char * filename 
);

/*==============================================================
FUNCTION:  btqsocnvmplatform_open_file
==============================================================*/
/**
  This function opens a NVM file for processing.

  @see btqsocnvmplatform_find_file
  
  @return  boolean: TRUE: If file was opened.
                    FALSE: Otherwise.
*/
boolean btqsocnvmplatform_open_file(void);

/*==============================================================
FUNCTION:  btqsocnvmplatform_get_file_size
==============================================================*/
/**
  Return the size of the NVM file.
  
  @see btqsocnvmplatform_open_file
  
  @return  boolean: True: If successful.
                    False: otherwise.
*/
boolean btqsocnvmplatform_get_file_size
(
  /** pointer to integer for returning file size */
  int *file_size_ptr
);

/*==============================================================
FUNCTION:  btqsocnvmplatform_read_file
==============================================================*/
/**
  This function reads from the NVM file.

  @see btqsocnvmplatform_open_file
  
  @return  The number of bytes read from the file.
*/
extern uint16 btqsocnvmplatform_read_file
(
  /** [out] Pointer to a buffer that receives the file data. */
  void *buf, 
  
  /** [in] Size of the data buffer. */
  uint16 buf_size
);

/*==============================================================
FUNCTION:  btqsocnvmplatform_close_file
==============================================================*/
/**
  This function closes the NVM file.

  @see btqsocnvmplatform_open_file
  
  @return  boolean: TRUE: If successful.
                    FALSE: otherwise.
*/
void btqsocnvmplatform_close_file(void);

/*==============================================================
FUNCTION:  btqsocnvmplatform_malloc
==============================================================*/
/**
  Platform independent memory allocation function.

  @see btqsocnvmplatform_free
  
  @return  pointer to allocated memory.
*/
void *btqsocnvmplatform_malloc 
(
  /** [in] number of bytes to allocate */
  int num_bytes
);

/*==============================================================
FUNCTION:  btqsocnvmplatform_free
==============================================================*/
/**
  Platform independent memory deallocation function.

  @see btqsocnvmplatform_malloc
  
  @return  none
*/
void btqsocnvmplatform_free 
(
  /** [in] pointer to memory to be deallocated */
  void *mem_ptr
);

/*------------------------------------------------------------------------------
Notes
------------------------------------------------------------------------------*/
 
/** @} */ /* end of btqsocvmpplatform group */

#ifdef __cplusplus
};
#endif 

#endif /* #ifndef _BTQSOCNVMPLATFORM_H */
