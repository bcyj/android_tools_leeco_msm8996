/***********************************************************************
 * tftp_comdef_la.h
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
2014-07-18   rp    tftp and ipc-router integration changes from target.
2014-06-04   rp    Create

===========================================================================*/

#ifndef __TFTP_COMDEF_LA_H__
#define __TFTP_COMDEF_LA_H__

#include "tftp_config_i.h"

#if !defined (TFTP_LA_BUILD)
  #error "This file should only be included for LA build"
#endif

#include <stdint.h>
#include <stdio.h>

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

#define PACKED
#define PACKED_POST __attribute__((__packed__))


#endif /* not __TFTP_COMDEF_LA_H__ */
