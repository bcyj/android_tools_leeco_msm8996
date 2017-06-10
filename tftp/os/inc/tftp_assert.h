/***********************************************************************
 * tftp_assert.h
 *
 * Short description.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Verbose Description
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-01-05   rp    In logs do not print full-paths just file-name portion.
2014-12-30   dks   Fixes to config and log module
2014-09-19   dks   Add hooks to extract performance numbers.
2014-10-14   rp    Use asserts for control-logic, debug-asserts for data-logic
2014-08-26   rp    Bring in changes from target-unit-testing.
2014-07-18   rp    tftp and ipc-router integration changes from target.
2014-06-11   rp    Renamed DEBUG_ASSERT as TFTP_DEBUG_ASSERT
2014-06-04   rp    Switch to IPCRouter sockets.
2013-11-14   rp    Create

===========================================================================*/

#ifndef __TFTP_ASSERT_H__
#define __TFTP_ASSERT_H__

#include "tftp_config_i.h"
#include "assert.h"

/*---------------------------------------------------------------------------
 * TFTP_ASSERT :
 *    Enabled on CRM-builds and can be disabled on commercial.

 * TFTP_DEBUG_ASSERT :
 *    Disabled on both CRM and commercial builds. Enabled only on demand
 *    when we debug TFTP
---------------------------------------------------------------------------*/

#if defined (TFTP_NHLOS_BUILD)

  #include "msg.h"

  extern int tftp_assert_run_infinite_loop;

  #define TFTP_ASSERT(cond)                                       \
    do {                                                          \
      if (!(cond)) {                                              \
        do {                                                      \
          MSG_SPRINTF_1(MSG_SSID_RFS_ACCESS, MSG_LEGACY_ERROR,    \
                        "TFTP_ASSERT : %s", #cond);               \
        } while (tftp_assert_run_infinite_loop);                  \
        ASSERT (cond);                                            \
      }                                                           \
    } while (0)

#elif defined (TFTP_LA_BUILD)

  #define TFTP_ASSERT(cond)                                       \
    do {                                                          \
      if (!(cond))                                                \
      {                                                           \
        fprintf (stdout, "\n\nASSERT : Line=%d : Cond=%s\n\n",    \
                 __LINE__, #cond);                                \
        assert(cond);                                             \
      }                                                           \
    } while (0)

#else
  #error "Unknown build-type. Configure as HLOS or NHLOS build"
#endif

#if defined (TFTP_ENABLE_DEBUG_MODE)
  #define TFTP_DEBUG_ASSERT(cond) TFTP_ASSERT(cond)
#else
  #define TFTP_DEBUG_ASSERT(cond) ((void)0)
#endif

#endif /* __TFTP_TFTP_DEBUG_ASSERT_H__ */
