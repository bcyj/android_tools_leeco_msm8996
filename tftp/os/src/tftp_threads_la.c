/***********************************************************************
 * tftp_threads_la.c
 *
 * Short description.
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
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
2014-07-28   rp    Add api to get thread-id.
2014-06-04   rp    Create

===========================================================================*/

#include "tftp_config_i.h"
#include "tftp_comdef.h"
#include "tftp_threads.h"
#include "tftp_threads_la.h"
#include "tftp_assert.h"

#if !defined (TFTP_LA_BUILD)
  #error "This file should only be compiled for LA build"
#endif

int
tftp_thread_mutex_init (tftp_mutex_handle *hdl_ptr)
{
  int result = 0;
  result = pthread_mutex_init (hdl_ptr, NULL);
  return result;
}

int
tftp_thread_lock (tftp_mutex_handle *hdl_ptr)
{
  int result = 0;
  result = pthread_mutex_lock (hdl_ptr);
  return result;
}

int
tftp_thread_unlock (tftp_mutex_handle *hdl_ptr)
{
  int result = 0;
  result = pthread_mutex_unlock (hdl_ptr);
  return result;
}

int
tftp_thread_create(tftp_thread_handle *thread,
                   tftp_thread_return_type (*thread_start) (void *),
                   void *args)
{
  int result;
  pthread_attr_t pthread_attr;

  result = pthread_attr_init(&pthread_attr);
  if (result != 0)
  {
    return result;
  }

  result = pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);
  if (result != 0)
  {
    return result;
  }

  result = pthread_create (thread, &pthread_attr, thread_start, args);

  (void) pthread_attr_destroy (&pthread_attr);
  return result;
}

tftp_thread_handle
tftp_thread_self (void)
{
  tftp_thread_handle thread_handle = (tftp_thread_handle)NULL;

  thread_handle = pthread_self ();

  return (tftp_thread_handle) thread_handle;
}

