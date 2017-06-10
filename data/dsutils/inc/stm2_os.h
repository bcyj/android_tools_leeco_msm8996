/*!
  @file
  stm2_os.h

  @brief
  This file contains variant source particular to a given operating system
  or target whose services are needed by the STM framework.

  @detail
  The STM framework needs several operating system specific constructs that
  are to be provided in this file.  Namely, they are as follows:

  - A generic error reporting mechanism.  Normally, STM relies upon the
    error function pointer in the state machine instance for error reporting,
    but needs a generic fallback mechanism to report errors in the case that
    the state machine instance pointer is NULL.
  - A generic critical section mechanism.  STM needs a critical section entry
    and exit construct such that it can perform thread-safe synchronization
    and locking on the STM data structures when in-use.

*/

/*===========================================================================

  Copyright (c) 2007 - 2008 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: /local/mnt/workspace/randrew/7x30_1/vendor/qcom-proprietary/data/common/inc/RCS/stm2_os.h,v 1.1 2010/02/17 23:56:22 randrew Exp $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/12/08   trc     Rename to stm2_os.h to match stm2.h
04/23/07   trc     Add STM_ASSERT() macro
04/16/07   trc     Update for UMB [get rid of direct printf() for MSG_*]
04/02/07   trc     Added STM_INIT_CRITICAL_SECTION() macro
02/28/07   trc     Added STM_NULL_CHECK() macro
02/27/07   trc     Lint fixes
02/26/07   trc     Initial version.  GCC variant.
===========================================================================*/

#ifndef STM2_OS_H
#define STM2_OS_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

/* Include file/s to obtain common types and definitions, such as 'int32',
   'TRUE', 'NULL', and so forth. */
#include "comdef.h"

/* Include file/s to support the definitions below */
#include "err.h"
#include <stdio.h>
#include <ds_util.h>

/*===========================================================================

                   EXTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/*! @brief STM macro used to log or report severe internal errors using
           whatever operating system specific 'printf' style error messaging
           mechanism is available.  Season to taste for your favorite OS...
*/
#define STM_ERROR_MSG(errormsg)   ds_log_err( "%s", errormsg )

/*! @brief STM macro used to check for NULL pointers.  This macro should
           be 'fatal' in the sense that normal code execution does not
           continue after it executes.  Season to taste for your favorite OS...
*/
#define STM_NULL_CHECK(pointer)                                         \
  if( pointer == NULL )                                                 \
  {                                                                     \
    fprintf(stderr, "%s, %d - Fatal Error: '" #pointer " == NULL'",     \
                    __FILE__,                                           \
                    __LINE__);                                          \
    ds_abort();                                                         \
  }


/*! @brief STM macro used to ASSERT() that a given condition is true.
           This macro should be 'fatal' in the sense that normal code
           execution does not continue after it evaluates to be false.
           Season to taste for your favorite OS...
*/
#define STM_ASSERT(condition)                                           \
  if( !(condition) )                                                    \
  {                                                                     \
    fprintf(stderr, "%s, %d - STM Assertion Failed: '" #condition,      \
                    __FILE__,                                           \
                    __LINE__);                                          \
    ds_abort();                                                         \
  }


/*! @brief STM macro used to init a critical section.  This is used prior to
           the entry/exit pair of critical section macros.  Season to taste
           for your favorite OS...
    @see   STM_ENTER_CRITICAL_SECTION
    @see   STM_EXIT_CRITICAL_SECTION
*/
#define STM_INIT_CRITICAL_SECTION()


/*! @brief STM macro used to enter a critical section.  This is used internally
           to ensure that only one thread may access the internal members of
           a state machine at once.  Season to taste for your favorite OS...
    @see   STM_EXIT_CRITICAL_SECTION
*/
#define STM_ENTER_CRITICAL_SECTION()


/*! @brief STM macro used to exit a critical section.  This is called by the
           STM framework once it is done manipulating internal members of a
           state machine.  Season to taste for your favorite OS...
    @see   STM_ENTER_CRITICAL_SECTION
*/
#define STM_EXIT_CRITICAL_SECTION()


#endif /* STM2_OS_H */
