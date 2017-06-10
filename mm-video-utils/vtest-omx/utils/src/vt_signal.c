/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential

Copyright (c) 2010 The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of The Linux Foundation nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--------------------------------------------------------------------------*/

/*========================================================================
Include Files
==========================================================================*/
#include "vt_signal.h"
#include "vt_debug.h"
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include "vt_def.h"

typedef struct vt_signal_type
{
   pthread_cond_t cond;
   pthread_mutex_t mutex;
   boolean m_bSignalSet ;
} vt_signal_type;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_signal_create(void** handle)
{
   int result = 0;

   if (handle)
   {
      vt_signal_type* sig;

      *handle = malloc(sizeof(vt_signal_type));
      if (*handle)
      {
         sig = (vt_signal_type*) *handle;
         sig->m_bSignalSet = FALSE ;
         if (pthread_cond_init(&sig->cond,NULL) == 0)
         {
            if (pthread_mutex_init(&sig->mutex, NULL) != 0)
            {
               VTEST_MSG_ERROR("failed to create mutex");
               result = 1;
               pthread_cond_destroy(&sig->cond);
               free(*handle);
            }
         }
         else
         {
            VTEST_MSG_ERROR("failed to create cond var");
            result = 1;
            free(*handle);
         }
      }
      else
      {
         VTEST_MSG_ERROR("failed to alloc handle");
         result = 1;
      }
   }
   else
   {
      VTEST_MSG_ERROR("handle is null");
      result = 1;
   }

   return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_signal_destroy(void* handle)
{
   int result = 0;

   if (handle)
   {
      vt_signal_type* sig = (vt_signal_type*) handle;
	  sig->m_bSignalSet = FALSE ;
      pthread_cond_destroy(&sig->cond);
      pthread_mutex_destroy(&sig->mutex);
      free(handle);
   }
   else
   {
      VTEST_MSG_ERROR("handle is null");
      result = 1;
   }

   return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_signal_set(void* handle)
{
   int result = 0;

   if (handle)
   {
      vt_signal_type* sig = (vt_signal_type*) handle;

      if (pthread_mutex_lock(&sig->mutex) == 0)
      {
         sig->m_bSignalSet = TRUE;
         if (pthread_cond_signal(&sig->cond) != 0)
         {
            VTEST_MSG_ERROR("error setting signal");
            result = 1;
         }

         if (pthread_mutex_unlock(&sig->mutex) != 0)
         {
            VTEST_MSG_ERROR("error unlocking mutex");
            result = 1;
         }
      }
      else
      {
         VTEST_MSG_ERROR("error locking mutex");
         result = 1;
      }
   }
   else
   {
      VTEST_MSG_ERROR("handle is null");
      result = 1;
   }

   return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_signal_wait(void* handle, int timeout)
{
   int result = 0;

   if (handle)
   {
      vt_signal_type* sig = (vt_signal_type*) handle;

      if (pthread_mutex_lock(&sig->mutex) == 0)
      {

         if (timeout > 0)
         {
            int wait_result;
            struct timespec time;
            if(sig->m_bSignalSet == TRUE)
            {
               timeout = 1 ;
               sig->m_bSignalSet = FALSE ;
            }
            clock_gettime(CLOCK_REALTIME, &time);
            time.tv_sec += timeout / 1000;
            time.tv_nsec += (timeout % 1000) * 1000000;

            wait_result = pthread_cond_timedwait(&sig->cond, &sig->mutex, &time);
            if (wait_result == ETIMEDOUT)
            {
               result = 2;
            }
            else if (wait_result != 0)
            {
               result = 1;
            }
            if((timeout == 1) || (sig->m_bSignalSet == TRUE))
            {
               result = 0;
            }
         }
         else
         {
            if(sig->m_bSignalSet == TRUE)
			 {
				 struct timespec time;
				 timeout = 1 ;
				 clock_gettime(CLOCK_REALTIME, &time);
				 time.tv_sec += timeout / 1000;
				 time.tv_nsec += (timeout % 1000) * 1000000;
				 pthread_cond_timedwait(&sig->cond, &sig->mutex, &time) ;
				 sig->m_bSignalSet = FALSE ;
			 }
			 else
			 {
				 if (pthread_cond_wait(&sig->cond, &sig->mutex) != 0)
				 {
					 VTEST_MSG_ERROR("error waiting for signal");
					 result = 1;
				 }
				 sig->m_bSignalSet = FALSE ;
			 }
		 }

         if (pthread_mutex_unlock(&sig->mutex) != 0)
         {
            VTEST_MSG_ERROR("error unlocking mutex");
            result = 1;
         }

      }
      else
      {
         VTEST_MSG_ERROR("error locking mutex");
         result = 1;
      }
   }
   else
   {
      VTEST_MSG_ERROR("handle is null");
      result = 1;
   }

   return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int vt_signal_broadcast(void* handle)
{
   int result = 0;

   if (handle) {

      vt_signal_type* sig = (vt_signal_type*) handle;
      if (pthread_mutex_lock(&sig->mutex) == 0) {

         sig->m_bSignalSet = TRUE;
         if (pthread_cond_broadcast(&sig->cond) != 0) {
            VTEST_MSG_ERROR("error setting signal");
            result = 1;
         }
         if (pthread_mutex_unlock(&sig->mutex) != 0) {
            VTEST_MSG_ERROR("error unlocking mutex");
            result = 1;
         }
      } else {
         VTEST_MSG_ERROR("error locking mutex");
         result = 1;
      }
   } else {
      VTEST_MSG_ERROR("handle is null");
      result = 1;
   }

   return result;
}
