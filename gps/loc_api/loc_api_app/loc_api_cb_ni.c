/******************************************************************************
  @file:  loc_api_cb_ni.c
  @brief:  module for loc_api callback for network initiated interactions

  DESCRIPTION
     LOC_API callback for network initiated interactions

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
******************************************************************************/

/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      -------------------------------------------------------
03/17/09   dx       Android version
01/09/09   dx       Initial version

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_cb_log.c#10 $
======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "loc_api_rpc_glue.h"
#include "loc_api_test.h"
#include "loc_api_data.h"
#include "loc_api_cmd.h"
#include "loc_api_cb_ni.h"

/*=============================================================================
 *
 *                             DATA DECLARATION
 *
 *============================================================================*/

pthread_t loc_ni_thread;             /* NI thread */

pthread_mutex_t         loc_ni_lock                     = PTHREAD_MUTEX_INITIALIZER;
int                     loc_ni_response_time_left       = 0;       /* examine time for NI response */
boolean                 loc_ni_need_input               = FALSE;   /* next input will be processed by NI */
rpc_loc_ni_event_s_type loc_ni_request;

/*=============================================================================
 *
 *                             FUNCTION DECLARATIONS
 *
 *============================================================================*/


/*===========================================================================

FUNCTION respond_from_enum

DESCRIPTION
   Returns the name of the response

RETURN VALUE
   response name string

===========================================================================*/
static const char* respond_from_enum(rpc_loc_ni_user_resp_e_type resp)
{
   switch (resp)
   {
   case RPC_LOC_NI_LCS_NOTIFY_VERIFY_ACCEPT:
      return "accept";
   case RPC_LOC_NI_LCS_NOTIFY_VERIFY_DENY:
      return "deny";
   case RPC_LOC_NI_LCS_NOTIFY_VERIFY_NORESP:
      return "no response";
   default:
      return NULL;
   }
}

/*===========================================================================

FUNCTION loc_ni_request_handler

DESCRIPTION
   Displays the NI request and awaits user input. If a previous request is
   in session, the new one is handled using sys.ni_default_response (if exists);
   otherwise, it is denied.

DEPENDENCY
   Do not lock the data by mutex loc_ni_lock

RETURN VALUE
   none

===========================================================================*/

static void loc_ni_respond
(
      rpc_loc_ni_user_resp_e_type resp,
      const rpc_loc_ni_event_s_type *request_pass_back
)
{
   loc_write_msg("Send NI response: %s\n", respond_from_enum(resp));

   rpc_loc_ioctl_data_u_type data;
   rpc_loc_event_payload_u_type callback_payload;

   memcpy(&data.rpc_loc_ioctl_data_u_type_u.user_verify_resp.ni_event_pass_back, 
         request_pass_back, sizeof (rpc_loc_ni_event_s_type));
   data.rpc_loc_ioctl_data_u_type_u.user_verify_resp.user_resp = resp;

   loc_test_checked_ioctl(RPC_LOC_IOCTL_INFORM_NI_USER_RESPONSE, 
         &data, &callback_payload, "loc_ni_respond");
}

/*===========================================================================

FUNCTION loc_ni_request_handler

DESCRIPTION
   Displays the NI request and awaits user input. If a previous request is
   in session, the new one is handled using sys.ni_default_response (if exists);
   otherwise, it is denied.

RETURN VALUE
   none

===========================================================================*/
static void loc_ni_request_handler(char *msg, const rpc_loc_ni_event_s_type *ni_req)
{
   /* If busy, use default or deny */
   if (loc_ni_need_input)
   {
#if 0
      /* Cannot be here because the current thread is in RPC client */
      /* XXX Consider adding an event queue to process overlapped NI requests */
      loc_ni_user_resp_e_type response =
         sys.ni_default_resp == 1 /* accept */ ?
               LOC_NI_LCS_NOTIFY_VERIFY_ACCEPT :
               LOC_NI_LCS_NOTIFY_VERIFY_DENY;

      loc_ni_respond(response, ni_req); */
#endif
   }
   else {
      /* Print notification */
      loc_write_msg("\n>>> NI Notification: %s\n", msg);
      loc_write_msg("Automatically sends 'no response' in %d seconds\n", LOC_NI_NO_RESPONSE_TIME);
      loc_write_msg("Accept (y/n)? \n");

      pthread_mutex_lock(&loc_ni_lock);

      /* Save request */
      memcpy(&loc_ni_request, ni_req, sizeof loc_ni_request);

      /* Set up NI response waiting */
      loc_ni_need_input = TRUE;
      loc_ni_response_time_left = LOC_NI_NO_RESPONSE_TIME;

      pthread_mutex_unlock(&loc_ni_lock);
   }

}

/*===========================================================================

FUNCTION loc_ni_cmd_handler

DESCRIPTION
   Handles user input from the command line

RETURN VALUE
   error code (0 for successful)

===========================================================================*/
int loc_ni_cmd_handler(int argc, char* argv[])
{
   loc_write_msg("\n>>> NI response: %s\n", argv[0]);

   char resp_char = toupper(argv[0][0]);
   rpc_loc_ni_user_resp_e_type resp = resp_char == 'Y' ?
         RPC_LOC_NI_LCS_NOTIFY_VERIFY_ACCEPT : RPC_LOC_NI_LCS_NOTIFY_VERIFY_DENY;

   /* Make the NI respond */
   loc_ni_respond(
         resp,
         &loc_ni_request);

   pthread_mutex_lock(&loc_ni_lock);
   loc_ni_need_input = FALSE;
   loc_ni_response_time_left = 0;
   pthread_mutex_unlock(&loc_ni_lock);

   return 0;
}

/*===========================================================================

FUNCTION loc_test_callback_dcm

DESCRIPTION
   Loc API callback handler

RETURN VALUE
   error code (0 for success)

===========================================================================*/
int loc_test_callback_ni (
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
)
{
   int rc = 0;
   const rpc_loc_ni_event_s_type *ni_req = &loc_event_payload->rpc_loc_event_payload_u_type_u.ni_request;
   if (loc_event == RPC_LOC_EVENT_NI_NOTIFY_VERIFY_REQUEST)
   {
      switch (ni_req->event)
      {
      case RPC_LOC_NI_EVENT_VX_NOTIFY_VERIFY_REQ:
         loc_write_log("VX Notify\n");
         loc_ni_request_handler("VX Notify", ni_req);
         break;

      case RPC_LOC_NI_EVENT_UMTS_CP_NOTIFY_VERIFY_REQ:
         loc_write_log("UMTS Notify\n");
         loc_ni_request_handler("UMTS Notify", ni_req);
         break;

      case RPC_LOC_NI_EVENT_SUPL_NOTIFY_VERIFY_REQ:
         loc_write_log("SUPL Notify\n");
         loc_ni_request_handler("SUPL Notify", ni_req);
         break;

      default:
         loc_write_error("Unknown NI event: %x\n", (int) ni_req->event);
         break;
      }
   }
   return rc;
}

/*===========================================================================

FUNCTION loc_ni_thread_proc

===========================================================================*/
void* loc_ni_thread_proc(void *threadid)
{
   loc_write_log("Starting Loc NI thread...\n");

   while (1)
   {
      /* wakes up every second to check timed out requests */
      sleep(1);

      pthread_mutex_lock(&loc_ni_lock);

      if (loc_ni_need_input && loc_ni_response_time_left > 0)
      {
         loc_ni_response_time_left--;
         if (loc_ni_response_time_left <= 0)
         {
            loc_ni_respond(RPC_LOC_NI_LCS_NOTIFY_VERIFY_NORESP, &loc_ni_request);
            loc_ni_need_input = FALSE;
         }
      }

      pthread_mutex_unlock(&loc_ni_lock);
   } /* while (1) */

   pthread_exit(NULL);
   return NULL;
}

/*===========================================================================

FUNCTION gps_dcm_thread_start

===========================================================================*/
int loc_ni_thread_start(void)
{
   int rc = 0;

   rc = pthread_create(&loc_ni_thread, NULL, loc_ni_thread_proc, NULL);

   if (rc)
   {
      loc_write_error("Loc NI thread is not created.\n");
      return -1;
   }

   return 0;
}
