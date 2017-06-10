/***********************************************************************
 * tftp_os_la.h
 *
 * Short description
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Verbose description.
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-06-04   rp    Create

===========================================================================*/

#ifndef __TFTP_OS_LA_H__
#define __TFTP_OS_LA_H__


#include "tftp_config_i.h"
#include "tftp_comdef.h"

#include "errno.h"
#include <fcntl.h>

#ifndef TFTP_LE_BUILD_ONLY
  #include <private/android_filesystem_config.h>
#endif

#if !defined (TFTP_LA_BUILD)
  #error "This file should only be compiled for LA build"
#endif

#define MAIN_TYPE

#ifndef TFTP_LE_BUILD_ONLY
  #define TFTP_SHARED_GID AID_RFS_SHARED
#else
  #define TFTP_SHARED_GID 0
#endif

#endif /* not __TFTP_OS_LA_H__ */
