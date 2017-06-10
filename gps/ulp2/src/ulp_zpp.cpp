/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       ULP ZPP module

GENERAL DESCRIPTION
  This file contains functions to start/stop ZPP provider.
=============================================================================*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/stat.h>
#include <fcntl.h>
#include <linux/types.h>
#include <errno.h>
#include <new>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

// Internal include files
#include <ulp_engine.h>

#include "loc_cfg.h"
#include "msg_q.h"

#include "ulp_quipc.h"

#include "ulp_data.h"
#include "ulp_internal.h"

#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_libulp"

static void* ulp_zpp_thread_proc(void *args);
zpp_provider_info_s_type* zpp_provider_info_p = &ulp_data.zpp_provider_info;

/*===========================================================================
FUNCTION    ulp_zpp_engine_running

DESCRIPTION
   This function returns true if ZPP engine is running.

DEPENDENCIES
   None

RETURN VALUE
   true: engine is running
   false: engine is not running

SIDE EFFECTS
   N/A
===========================================================================*/
bool ulp_zpp_engine_running ()
{
   bool zpp_running = true;

   ENTRY_LOG_CALLFLOW();

   if (ulp_data.zpp_provider_info.state == ZPP_STATE_IDLE)
   {
      zpp_running = false;
   }
   else
   {
      zpp_running = true;
   }

   EXIT_LOG(%d, zpp_running);
   return zpp_running;
}

/*===========================================================================
FUNCTION    ulp_zpp_start_engine

DESCRIPTION
   This function is called to start ZPP provider.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_zpp_start_engine ()
{
   int                   ret_val = -1;
   LocAdapterBase*       adapter = ulp_data.loc_proxy->getAdapter();

   ENTRY_LOG_CALLFLOW();

   LOC_LOGD ("%s, zpp state = %d, ulp_zpp_engine_running () = %d\n",
             __func__,
             ulp_data.zpp_provider_info.state,
             ulp_zpp_engine_running());
   do
   {
      // Wait for first fix to complete
      if (ulp_data.zpp_provider_info.first_fix_pending == true)
      {
         break;
      }

      if (ulp_zpp_engine_running () == false)
      {
         if ((ULP_LOC_SCREEN_ON == ulp_data.system_event)||
             (ULP_LOC_TIMEZONE_CHANGE == ulp_data.system_event)||
             (ULP_LOC_PHONE_CONTEXT_UPDATE == ulp_data.system_event))
         {
            //these events require single shot ZPP updates
            ulp_data.zpp_provider_info.recurrence_type =
                ULP_LOC_RECURRENCE_SINGLE;
         }

         else if ( ULP_LOC_POWER_CONNECTED == ulp_data.system_event)
         {
            ulp_data.zpp_provider_info.recurrence_type =
                ULP_LOC_RECURRENCE_PERIODIC;

            pthread_mutex_lock(&zpp_provider_info_p->tLock);
            ulp_data.zpp_provider_info.periodic_session_active = true;
            pthread_mutex_unlock(&zpp_provider_info_p->tLock);
            if (NULL == ulp_data.zpp_provider_info.thread)
            {
               //Launch a thread to do a periodic ZPP update
               int rc = 0;
               rc = pthread_create(&ulp_data.zpp_provider_info.thread,
                                   NULL, ulp_zpp_thread_proc, &ulp_data);
               if (rc) LOC_LOGE("ZPP thread could not created. rc = %d\n", rc);
            }
         }
         // Send ZPP location request to engine
         adapter->getZppInt();
         ulp_data.zpp_provider_info.state              = ZPP_STATE_ACTIVE;
         ulp_data.zpp_provider_info.first_fix_pending  = true;
         ret_val = 0;
      }
   } while (0);

   EXIT_LOG(%d, ret_val);

   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_zpp_stop_engine

DESCRIPTION
   This function is called to stop ZPP provider.Does not do anything significant
   until tracking support is added

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_zpp_stop_engine ()
{
   int ret_val = -1;

   ENTRY_LOG_CALLFLOW();

   LOC_LOGD ("%s, zpp state = %d, ulp_zpp_engine_running () = %d\n",
             __func__,
             ulp_data.zpp_provider_info.state,
             ulp_zpp_engine_running());

   do
   {
      if (ulp_zpp_engine_running () == true)
      {
         // Set zpp provider state to INACTIVE
         ulp_data.zpp_provider_info.state                  = ZPP_STATE_IDLE;
         ulp_data.zpp_provider_info.first_fix_pending      = false;
         ret_val = 0;
      }
      //check if periodic session active. if yes then shut it down
      if ((ULP_LOC_POWER_DISCONNECTED == ulp_data.system_event)&&
         (NULL != ulp_data.zpp_provider_info.thread))
      {
         pthread_mutex_lock(&zpp_provider_info_p->tLock);
         zpp_provider_info_p->periodic_session_active = false;
         pthread_cond_signal(&zpp_provider_info_p->tCond);
         pthread_mutex_unlock(&zpp_provider_info_p->tLock);
         void *ignored;
         //wait for the thread to termintate
         pthread_join(ulp_data.zpp_provider_info.thread, &ignored);
         ulp_data.zpp_provider_info.thread = NULL;
      }
      ret_val = 0;
   } while (0);
   EXIT_LOG(%d, ret_val);
   return ret_val;
}

static void* ulp_zpp_thread_proc(void *args)
{
   ENTRY_LOG();

   ulp_data_s_type* ulp_data_p = (ulp_data_s_type*)args;
   zpp_provider_info_s_type* zpp_provider_info_p = &ulp_data_p->zpp_provider_info;
   int rc = 0;          /* return code from pthread calls */

   struct timeval present_time;
   struct timespec expire_time;

   LOC_LOGD("Starting ulp_zpp thread...\n");
   pthread_mutex_lock(&zpp_provider_info_p->tLock);
   while (zpp_provider_info_p->periodic_session_active)
   {
      /* Calculate absolute expire time */
      gettimeofday(&present_time, NULL);
      expire_time.tv_sec  = present_time.tv_sec + (zpp_provider_info_p->zpp_trigger_threshold/1000);
      expire_time.tv_nsec = present_time.tv_usec * 1000;
      LOC_LOGD("%s,ulp_zpp_thread_proc-Present time %ld, Time out set for abs time %ld with delay %d msec\n",
               __func__,(long) present_time.tv_sec,
               (long) expire_time.tv_sec, zpp_provider_info_p->zpp_trigger_threshold );

      rc = pthread_cond_timedwait(&zpp_provider_info_p->tCond,
                                  &zpp_provider_info_p->tLock,
                                  &expire_time);
      if (rc == ETIMEDOUT)
      {
         LOC_LOGD("%s,ulp_zpp_thread_proc-Thread waking up. Ret Val %d\n",__func__, rc );
         ulp_msg_system_update *msg(new ulp_msg_system_update(&ulp_data, ULP_LOC_ZPP_PERIODIC_WAKEUP));
         msg_q_snd(ulp_data.loc_proxy->mQ, msg,ulp_msg_free);
      }
   }
   pthread_mutex_unlock(&zpp_provider_info_p->tLock);
   if (ETIMEDOUT != rc)
   {
      LOC_LOGD("ulp_zpp thread was signalled to exit. pthread_cond_timedwait = %d\n",rc );
   }

   EXIT_LOG(%s, VOID_RET);
   return NULL;
}

