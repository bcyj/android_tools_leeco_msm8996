/******************************************************************************
  @file:  loc_api_cb_hub.c
  @brief:  module for loc_api callback handling

  DESCRIPTION

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

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_cb.c#9 $
======================================================================*/

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

#include "loc_api_rpc_glue.h"
#include "loc_api_cb_hub.h"
#include "loc_api_data.h"
#include "loc_api_cb_log.h"
#include "loc_api_cb_tests.h"
#include "loc_api_cb_ni.h"

#if defined(FEATURE_GPS_DCM_ENABLED) && !defined(FEATURE_GPS_DCM_ALWAYS_ON)
#  include "loc_api_cb_ds.h"
#endif /* FEATURE_GPS_DCM_ENABLED */

#if defined(FEATURE_GPS_DCM_ALWAYS_ON)
#  include "loc_api_cb_ds_on.h"
#endif /* FEATURE_GPS_DCM_ALWAYS_ON */


/***************************************************************************
 *                        CALLBACK PROCESSSING THREAD
 **************************************************************************/

/***************************************************************************
 *                         DATA FOR CALLBACK WAITING
 **************************************************************************/

/* Callback waiting conditional variable */
pthread_cond_t             loc_cb_arrived_cond = PTHREAD_COND_INITIALIZER;

/* Callback waiting data block, protected by loc_cb_data_mutex */
pthread_mutex_t            loc_cb_data_mutex   = PTHREAD_MUTEX_INITIALIZER;
boolean                    loc_cb_is_selected = FALSE;       /* is cb selected? */
boolean                    loc_cb_is_waiting  = FALSE;       /* is waiting?     */
boolean                    loc_cb_has_arrived = FALSE;       /* callback has arrived */
rpc_loc_event_mask_type        loc_cb_wait_event_mask   = 0;     /* event to wait   */
rpc_loc_event_payload_u_type  *loc_cb_payload_buffer_ptr;        /* user payload buffer */
rpc_loc_event_payload_u_type   loc_cb_received_payload;          /* received payload */
rpc_loc_event_mask_type        loc_cb_received_event_mask = 0;   /* received event   */


/*===========================================================================

FUNCTION    loc_test_callback_process_blocked_call

DESCRIPTION
   Wakes up blocked API calls to check if the needed callback has arrived

DEPENDENCIES
   N/A

RETURN VALUE
   none

SIDE EFFECTS
   N/A

===========================================================================*/
static void loc_test_callback_process_blocked_call(rpc_loc_event_mask_type loc_event,
      const rpc_loc_event_payload_u_type* loc_event_payload)
{
   pthread_mutex_lock(&loc_cb_data_mutex);

   /* Callback arrives before wait, remember it */
   if (loc_cb_is_waiting && !loc_cb_has_arrived)
   {
      loc_cb_received_event_mask = loc_event;
      memcpy(&loc_cb_received_payload, loc_event_payload, sizeof (rpc_loc_event_payload_u_type));
      pthread_mutex_unlock(&loc_cb_data_mutex);
      pthread_cond_signal(&loc_cb_arrived_cond);
   }
   else if (loc_cb_is_selected && !(loc_cb_has_arrived) &&
         (loc_cb_wait_event_mask & loc_event) != 0)
   {
      memcpy(&loc_cb_received_payload, loc_event_payload, sizeof (rpc_loc_event_payload_u_type));
      loc_cb_has_arrived = TRUE;
   }

   pthread_mutex_unlock(&loc_cb_data_mutex);
}

/*===========================================================================

FUNCTION    loc_select_callback

DESCRIPTION
   Selects which callback is going to be waited for

DEPENDENCIES
   N/A

RETURN VALUE
   none

SIDE EFFECTS
   N/A

===========================================================================*/
void loc_select_callback(
      rpc_loc_event_mask_type event_mask,                /* Event mask to wait for             */
      rpc_loc_event_payload_u_type *callback_payload     /* Pointer to callback payload buffer */
)
{
   pthread_mutex_lock(&loc_cb_data_mutex);

   loc_cb_is_selected = TRUE;
   loc_cb_is_waiting = FALSE;
   loc_cb_has_arrived = FALSE;

   loc_cb_wait_event_mask = event_mask;
   loc_cb_payload_buffer_ptr = callback_payload;

   pthread_mutex_unlock(&loc_cb_data_mutex);
}

/*===========================================================================

FUNCTION    loc_wait_callback

DESCRIPTION
   Waits for a selected callback. The wait expires in timeout_seconds seconds.

   If the function is called before an existing wait has finished, it will
   immediately return EBUSY.

DEPENDENCIES
   N/A

RETURN VALUE
   0                    if successful
   ETIMEDOUT            if timed out
   EBUSY                if already in a wait
   EINVAL               if callback is not yet selected

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_wait_callback(
      int timeout_seconds  /* Timeout in this number of seconds  */
)
{
   int ret_val = 0; /* the return value of this function: 0 = no error */
   int rc;          /* return code from pthread calls */

   struct timeval present_time;
   struct timespec expire_time;

   pthread_mutex_lock(&loc_cb_data_mutex);

   if (!loc_cb_is_selected)
   {
      pthread_mutex_unlock(&loc_cb_data_mutex);
      ret_val = EINVAL;  /* callback not selected */
      return ret_val;    /* exit */
   }

   if (loc_cb_is_selected && loc_cb_has_arrived)
   {
      memcpy(loc_cb_payload_buffer_ptr, &loc_cb_received_payload,
            sizeof (rpc_loc_event_payload_u_type));
      pthread_mutex_unlock(&loc_cb_data_mutex);
      ret_val = 0;
      return ret_val;    /* success */
   }

   if (loc_cb_is_waiting)
   {
      pthread_mutex_unlock(&loc_cb_data_mutex);
      ret_val = EBUSY; /* busy, rejected */
      return ret_val;  /* exit */
   }

   /* Calculate absolute expire time */
   gettimeofday(&present_time, NULL);
   expire_time.tv_sec  = present_time.tv_sec;
   expire_time.tv_nsec = present_time.tv_usec * 1000;
   expire_time.tv_sec += timeout_seconds;

   /* Take new wait request */
   loc_cb_is_waiting = TRUE;

   /* Waiting */
   while (1) {
      rc = pthread_cond_timedwait(&loc_cb_arrived_cond, &loc_cb_data_mutex, &expire_time);
      if (rc == ETIMEDOUT)
      {
         loc_cb_is_waiting = FALSE;
         ret_val = ETIMEDOUT; /* Timed out */
         break;
      }

      /* Obtained the first awaited callback */
      if ( (loc_cb_received_event_mask & loc_cb_wait_event_mask) != 0)
      {
         loc_cb_is_selected = FALSE;    /* clear selection */
         loc_cb_is_waiting = FALSE;     /* stop waiting */
         ret_val = 0;                   /* Successful */
         memcpy(loc_cb_payload_buffer_ptr, &loc_cb_received_payload,
               sizeof (rpc_loc_event_payload_u_type));
         break;
      }
   }

   pthread_mutex_unlock(&loc_cb_data_mutex);
   return ret_val;
}

/*===========================================================================

FUNCTION    loc_test_callback_lasts

DESCRIPTION
   Save last event

DEPENDENCIES
   N/A

RETURN VALUE
   0                    if successful

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_test_callback_lasts(
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
)
{
   switch (loc_event)
   {
   case RPC_LOC_EVENT_PARSED_POSITION_REPORT:
      /* If latitude + longitude are valid, save the pos */
      if (loc_event_payload->rpc_loc_event_payload_u_type_u.parsed_location_report.valid_mask &
            (RPC_LOC_POS_VALID_LATITUDE | RPC_LOC_POS_VALID_LONGITUDE))
      {
         memcpy(&last.pos, &loc_event_payload->rpc_loc_event_payload_u_type_u.parsed_location_report,
               sizeof last.pos);
      }
      break;
   case RPC_LOC_EVENT_STATUS_REPORT:
      if (loc_event_payload->rpc_loc_event_payload_u_type_u.status_report.event == RPC_LOC_STATUS_EVENT_ENGINE_STATE)
      {
         last.engine_state = loc_event_payload->rpc_loc_event_payload_u_type_u.status_report.payload.
            rpc_loc_status_event_payload_u_type_u.engine_state;
      }
      break;
   default:
      break;
   }
   return 0;
}

/***************************************************************************
 *                         STANDARD CALLBACK HANDLER
 **************************************************************************/

/*===========================================================================

FUNCTION    loc_test_callback_handler

DESCRIPTION
   Handles a callback event

DEPENDENCIES
   N/A

RETURN VALUE
   0                    if successful

SIDE EFFECTS
   N/A

===========================================================================*/
int32 loc_test_callback_handler(
      void*                                 userData,
      rpc_loc_client_handle_type            loc_handle,             /* handle of the client */
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
)
{
   /* Log the callback event header */
   loc_test_callback_log_header(loc_event, loc_event_payload);

   /* Unblocks user thread if the event is being awaited */
   loc_test_callback_process_blocked_call(loc_event, loc_event_payload);

   if (param.data_request_reply)
   {
#if defined(FEATURE_GPS_DCM_ENABLED) && !defined(FEATURE_GPS_DCM_ALWAYS_ON)
   /* Responds to data service requests */
   loc_test_callback_dcm(loc_event, loc_event_payload);
#endif /* FEATURE_GPS_DCM_ENABLED */

#if defined(FEATURE_GPS_DCM_ALWAYS_ON)
   /* Responds to data service requests */
   loc_test_callback_dcm_always_on(loc_event, loc_event_payload);
#endif /* FEATURE_GPS_DCM_ALWAYS_ON */
   }

   /* Log the callback event */
   loc_test_callback_log(loc_event, loc_event_payload);

   /* Processes NI requests */
   loc_test_callback_ni(loc_event, loc_event_payload);

   /* Perform selected callback test */
   if (cbf != NULL)
   {
      cbf(loc_event, loc_event_payload);
   }

   /* Save last callback payload as global data for debugging */
   memcpy(&lastcb, loc_event_payload, sizeof(rpc_loc_event_payload_u_type));
   loc_test_callback_lasts(loc_event, loc_event_payload);

   return 0;
}
