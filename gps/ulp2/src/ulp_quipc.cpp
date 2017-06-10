/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       ULP brain logic module

GENERAL DESCRIPTION
  This file contains ULP brain logic to select position provider and its
  configuration.

Copyright (c) 2011-2014 by Qualcomm Technologies, Inc. All Rights Reserved.
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
#include <dlfcn.h>

// Internal include files
#include <ulp_engine.h>
#include "loc_cfg.h"

#include "ulp_quipc.h"

#include "ulp_data.h"
#include "ulp_internal.h"


#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_libulp"

const ulpQuipcInterface* ulp_quipc_inf = NULL;

/*===========================================================================
FUNCTION    ulp_quipc_engine_running

DESCRIPTION
   This function returns true if QUIPC engine is running.

DEPENDENCIES
   None

RETURN VALUE
   true: engine is running
   false: engine is not running

SIDE EFFECTS
   N/A
===========================================================================*/
bool ulp_quipc_engine_running ()
{
   bool quipc_running = true;

   ENTRY_LOG_CALLFLOW();
   LOC_LOGD ("%s, quipc state = %d\n", __func__, ulp_data.quipc_provider_info.state);
   if (ulp_data.quipc_provider_info.state == QUIPC_STATE_IDLE)
   {
      quipc_running = false;
   }
   else
   {
      quipc_running = true;
   }

   EXIT_LOG(%d, quipc_running);
   return quipc_running;
}

/*===========================================================================
FUNCTION    ulp_quipc_start_engine

DESCRIPTION
   This function is called to start QUIPC. libulp module posts messages to
   QUIPC module via named pipe.

DEPENDENCIES
   None

RETURN VALUE
   0: QUIPC state remains ON
   1: QUIPC state changes from OFF to ON
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_quipc_start_engine ()
{
   int ret_val = -1;
   ENTRY_LOG_CALLFLOW();

#ifndef DEBUG_X86
   if (ulp_quipc_inf != NULL)
   {
      if (ulp_quipc_engine_running () == false)
      {
         LOC_LOGD ("%s: QUIPC start engine\n", __func__);

         // Use 1 second TBF
         ret_val = ulp_quipc_inf->start_fix (ULP_QUIPC_DEFAULT_TRACKING_INTERVAL_MSEC/1000,
                                             0, NULL);
         if (ret_val == 0)
         {
            ulp_quipc_set_state (QUIPC_STATE_INITIALIZING );
            ret_val = 1;
         }
      }
      else
      {
         ret_val = 0;
      }
   }
   else
   {
      LOC_LOGD ("%s: quipc not available, ignore the request\n", __func__);
   }
#else

   if (ulp_quipc_engine_running() == false)
   {
      LOC_LOGD ("%s: QUIPC start engine at state %d\n", __func__, ulp_data.quipc_provider_info.state);
      ulp_quipc_set_state (QUIPC_STATE_INITIALIZING );
      ret_val = 1;
   }
   else
   {
      ret_val = 0;
   }
#endif // DEBUG_X86

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_quipc_start_engine

DESCRIPTION
   This function is called to send lci selection command to IMC module.

Parameters:
   user_lci_specified : true if LCI is specified, false if LCI is not specified
   lci-id: formatted UUID string if user specifies the LCI, and NULL otherwise

DEPENDENCIES
   None

RETURN VALUE
   0: successful
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_quipc_set_lci_selection_mode (bool user_lci_specified,
                                      unsigned char* lci_id_str)
{
   int ret_val = 0;

#ifndef DEBUG_X86
   if(user_lci_specified == false)
   {
      // Use 1 second TBF
      ret_val = ulp_quipc_inf->start_fix (ULP_QUIPC_DEFAULT_TRACKING_INTERVAL_MSEC/1000,
                                          0, NULL);
   }
   else
   {
      // convert LCI id from string format of xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx
      // to 16 byte UUID
      unsigned char lci_id[16];
      char temp [3];
      int i = 0, j = 0;

      for (i = 0 ; i < LCI_ID_STRING_LENGTH; )
      {
         errno = 0;
         if (*(lci_id_str+i) == '-')
         {
            i++;
         }
         else
         {
            temp[0] = *(lci_id_str+i);
            i++;
            temp[1] = *(lci_id_str+i);
            i++;
            temp[2] = 0;
            lci_id[j] = strtol (temp, NULL, 16);
            j++;

            if (errno != 0)
            {
               LOC_LOGE ("%s: wrong user specified LCI ID, user selection ignored\n", __func__);
               break;
            }
         }
      }

      if (errno == 0)
      {
         // Use 1 second TBF
         ret_val = ulp_quipc_inf->start_fix (ULP_QUIPC_DEFAULT_TRACKING_INTERVAL_MSEC/1000,
                                             1, lci_id);
      }
   }
#endif

   return ret_val;
}

/*=====================================================================================
 * Function ulp_quipc_process_msg
 *  thread processing routine
 *
 * Description
 *  This is the processing routine of ulp quipc thread that waits for messages sent
 *  by QUIPC modules via named pipe.
 *
 * Parameters:
 *   ulp_msg_ptr, pointer to the message receievd by ULP
 *
 * Return value:
 *   NULL: on exit
 =============================================================================================*/
int ulp_quipc_stop_engine ()
{
   int ret_val = -1;
   ENTRY_LOG_CALLFLOW();

#ifndef DEBUG_X86
   if(ulp_quipc_inf != NULL)
   {
      if (ulp_quipc_engine_running () == true)
      {
         LOC_LOGD ("%s: QUIPC stop engine\n", __func__);
         ulp_quipc_inf->stop_fix ();
      }
      else
      {
        LOC_LOGD ("%s: quipc already stopped\n", __func__);
      }

      ulp_quipc_set_state (QUIPC_STATE_IDLE);
      // If we are still waiting for coarse position, clear the flag as well
      ulp_data.gnp_provider_info.coarse_pos_req_pending = false;
   }
   else
   {
      LOC_LOGD ("%s: ulp_quipc_inf is NULL \n", __func__);
   }
#else
   if (ulp_quipc_engine_running() == true)
   {
      LOC_LOGD ("%s: QUIPC stop engine at state %d\n", __func__, ulp_data.quipc_provider_info.state);
      ulp_quipc_set_state (QUIPC_STATE_IDLE);
      // If we are still waiting for coarse position, clear the flag as well
      ulp_data.gnp_provider_info.coarse_pos_req_pending = false;
   }
#endif // DEBUG_X86

   ret_val = 0;
   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*=============================================================================================
 * Function: ulp_quipc_init
 *
 * Description
 *   This function initializes ULP-QUIPC module. In particular, ULP and QUIPC communicates
 *   with each other via named pipe. This function will creates the named pipe, and
 *   start the thread to listen on the named pipe for incoming QUIPC status and
 *   position report.
 *
 * Parameters:
 *   arg: None
 *
 * Return value:
 *   Error code
 =============================================================================================*/
int ulp_quipc_init()
{
   void *handle = NULL;
   const char *error = NULL;
   get_ulp_quipc_interface* get_ulp_quipc_inf = NULL;

   int ret_val = -1;

   ENTRY_LOG_CALLFLOW();

   do
   {
      if (ulp_data.quipc_enabled == 0)
      {
         ret_val = 0;
         break;
      }

      handle = dlopen ("libquipc_ulp_adapter.so", RTLD_NOW);

      if (handle == NULL)
      {
         if ((error = dlerror()) != NULL)
         {
             LOC_LOGE ("%s, dlopen for libquipc_ulp_adapter.so failed, error = %s\n", __func__, error);
         }
         break;
      }

      // Clear any existing error
      dlerror();
      get_ulp_quipc_inf = (get_ulp_quipc_interface*) dlsym(handle, "ulp_quipc_get_interface");
      if (get_ulp_quipc_inf == NULL)
      {
         if((error = dlerror()) != NULL)
         {
            LOC_LOGE ("%s, dlsym for ulp_quipc_get_interface failed, error = %s\n", __func__, error);
         }
         break;
      }

      // Initialize the ULP interface
      ulp_quipc_inf = get_ulp_quipc_inf ();
      if(ulp_quipc_inf != NULL)
      {
         // Initialize the ULP interface
         ret_val = ulp_quipc_inf->init(ulp_msg_forward_quipc_position_report,
                                       ulp_msg_forward_quipc_coarse_position_request);
      }

   } while (0);

   if (ret_val != 0)
   {
      ulp_quipc_inf = NULL;
      ulp_data.quipc_enabled = 0;
   }

   EXIT_LOG(%d, ret_val);

   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_quipc_set_state

DESCRIPTION
   This function is called to update the current QUIPC engine state.
   In the meantime, it will save the old state.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_quipc_set_state (quipc_state_e_type new_state)
{
   LOC_LOGD ("%s, current quipc state = %d, new quipc state = %d\n",
             __func__,
             ulp_data.quipc_provider_info.state,
             new_state);

   if (ulp_data.quipc_provider_info.state != new_state)
   {
      if (ulp_data.quipc_provider_info.state == QUIPC_STATE_IDLE)
      {
         ulp_data.quipc_provider_info.last_started_time_ms = ulp_util_get_time_ms ();
      }
      else if (new_state == QUIPC_STATE_IDLE)
      {
         ulp_data.quipc_provider_info.last_stopped_time_ms = ulp_util_get_time_ms ();
      }

      if (new_state == QUIPC_STATE_LCI_TRANSITION)
      {
         ulp_data.quipc_provider_info.last_lci_transition_time_ms = ulp_util_get_time_ms();
      }

      ulp_data.quipc_provider_info.last_state = ulp_data.quipc_provider_info.state;
      ulp_data.quipc_provider_info.state = new_state;
   }

   return 0;
}
/*=====================================================================================
 * Function ulp_quipc_request_debug_info
 *
 * Description
 *  This is the function to request the debug info from QuIPS.
 *
 * Parameters:
 *   debug_type : ULP_QUIPC_DEBUG_INFO_START / ULP_QUIPC_DEBUG_INFO_STOP
 *
 * Return value:
 *   0: SUCCESS
 *   -1: FAILURE
 =============================================================================================*/
int ulp_quipc_request_debug_info  (int debug_type)
{
   int ret_val = -1;
   ENTRY_LOG_CALLFLOW();

#ifndef DEBUG_X86
   if(ulp_quipc_inf != NULL)
   {
      LOC_LOGD ("%s: QUIPC enable debug info\n", __func__);
      ulp_quipc_inf->request_debug_info(debug_type);
      ret_val = 0;
   }
   else
   {
      LOC_LOGD ("%s: quipc interface not available\n", __func__);
   }
#endif // DEBUG_X86

   EXIT_LOG(%d, ret_val);
   return ret_val;
}
