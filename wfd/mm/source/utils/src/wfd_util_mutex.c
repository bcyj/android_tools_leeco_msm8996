/* =======================================================================
                              wfd_util_mutex.c
DESCRIPTION
  This header defines the wfd source item class.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
   $Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/src/wfd_util_mutex.c

========================================================================== */

/*========================================================================
  Include Files
 ==========================================================================*/
#include "wfd_util_mutex.h"
#include "wfd_util_debug.h"
#include "MMDebugMsg.h"
#include <pthread.h>
#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int venc_mutex_create(void** handle)
{
  int result = 0;
  pthread_mutex_t* mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

  if (handle)
  {
    if(mutex != NULL)
    {
        if (pthread_mutex_init(mutex, NULL) == 0)
        {
            *handle = mutex;
        }
        else
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"init mutex failed");
            free(mutex);
            result = 1;
        }
    }
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"handle is null");
    result = 1;
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int venc_mutex_destroy(void* handle)
{
  int result = 0;
  pthread_mutex_t* mutex = (pthread_mutex_t*) handle;

  if (mutex)
  {
    if (pthread_mutex_destroy(mutex) == 0)
    {
      free(handle);
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"destroy mutex failed");
      result = 1;
    }
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"handle is null");
    result = 1;
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int venc_mutex_lock(void* handle)
{
  int result = 0;
  pthread_mutex_t* mutex = (pthread_mutex_t*) handle;

  if (mutex)
  {
    if (pthread_mutex_lock(mutex) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"lock mutex failed");
    }
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"handle is null");
    result = 1;
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int venc_mutex_unlock(void* handle)
{
  int result = 0;
  pthread_mutex_t* mutex = (pthread_mutex_t*) handle;

  if (mutex)
  {
    if (pthread_mutex_unlock(mutex) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"lock mutex failed");
    }
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"handle is null");
    result = 1;
  }

  return result;
}
