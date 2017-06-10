/***********************************************************************
 * tftp_malloc.h
 *
 * The TFTP malloc module.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * The TFTP malloc module.
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-06-04   rp    Switch to IPCRouter sockets.
2013-12-16   nr    Create

===========================================================================*/

#ifndef __TFTP_MALLOC_H__
#define __TFTP_MALLOC_H__

#include "tftp_config_i.h"
#include "tftp_comdef.h"

void tftp_malloc_init (void);

void* tftp_malloc (uint32 size);

void tftp_free (void *ptr);


#endif /* not __TFTP_MALLOC_H__ */
