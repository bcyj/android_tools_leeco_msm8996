/***********************************************************************
 * tftp_threads.h
 *
 * TFTP Thread abstraction layer.
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * TFTP Thread abstraction layer.
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-07-28   rp    Add api to get the thread-id.
2014-06-04   rp    Switch to IPCRouter sockets.
2014-01-13   dks   Add wrappers for rex critical section APIs.
2013-11-14   nr    Create

===========================================================================*/

#ifndef __TFTP_THREADS_H__
#define __TFTP_THREADS_H__

#include "tftp_config_i.h"
#include "tftp_comdef.h"

#if defined (TFTP_USE_POSIX_THREADS)
  #include "tftp_threads_la.h"
#elif  defined (TFTP_USE_WINDOWS_THREADS)/* WINDOWS */
  #include "tftp_threads_windows.h"
#elif defined (TFTP_USE_REX_THREADS)/* REX */
  #include "tftp_threads_modem.h"
#endif

int tftp_thread_mutex_init (tftp_mutex_handle *hdl_ptr);

int tftp_thread_lock (tftp_mutex_handle *hdl_ptr);

int tftp_thread_unlock (tftp_mutex_handle *hdl_ptr);

int tftp_thread_create(tftp_thread_handle *thread,
                       tftp_thread_return_type (*thread_start) (void *),
                       void *args);

tftp_thread_handle tftp_thread_self (void);


#endif /* not __TFTP_THREADS_H__ */
