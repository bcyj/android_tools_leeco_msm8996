/***********************************************************************
 * tftp_threads_la.h
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
2014-06-04   rp   Create

===========================================================================*/

#ifndef __TFTP_THREADS_LA_H__
#define __TFTP_THREADS_LA_H__

#include "tftp_config_i.h"
#include "tftp_comdef.h"

#if !defined (TFTP_LA_BUILD)
  #error "This file should only be compiled for LA build"
#endif

#include <pthread.h>

typedef pthread_t tftp_thread_handle;
typedef pthread_mutex_t tftp_mutex_handle;
typedef void* tftp_thread_return_type;

#define tftp_thread_return()               \
          do {                             \
            pthread_exit((void *)NULL);    \
            return NULL;                   \
          } while (0)

#endif /* not __TFTP_THREADS_LA_H__ */
