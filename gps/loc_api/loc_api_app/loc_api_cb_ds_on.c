/******************************************************************************
  @file:  loc_api_cb_ds_on.c
  @brief:  module for loc_api callback on data services (assuming always ON)

  DESCRIPTION
     LOC_API callback data services handlers, assuming that the data connection
     is already ON (for Android).

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
04/10/09   dx       Data always-on version
03/17/09   dx       Android version
01/09/09   dx       Initial version

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_cb_log.c#10 $
======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stringl.h>
#ifdef USE_GLIB
#include <glib.h>
#endif /* USE_GLIB */
#include "loc_api_rpc_glue.h"
#include "loc_api_test.h"
#include "loc_api_data.h"
#include "loc_api_cb_ds_on.h"
#include "platform_lib_includes.h"
/*=============================================================================
 *
 *                             DATA DECLARATION
 *
 *============================================================================*/

pthread_t loc_dcm_thread;  /* data call manager thread */

/* Server session handle */
rpc_loc_server_connection_handle loc_dcm_session_handle;
pthread_mutex_t state_info_mutex = PTHREAD_MUTEX_INITIALIZER;

loc_dcm_event_s_type curr_evt;
pthread_mutex_t atl_req_mutex          = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  atl_req_cond           = PTHREAD_COND_INITIALIZER;

/*===========================================================================

FUNCTION loc_dcm_open_req_handler

DESCRIPTION
   Store a data open request and call processor

RETURN VALUE
   error code (0 for success)
===========================================================================*/
static int loc_dcm_open_req_handler(const rpc_loc_server_request_s_type *req)
{
   int rc = 0;

   loc_write_log("Received open Request\n");

   pthread_mutex_lock(&atl_req_mutex);

   /* pass req to dcm thread */
   curr_evt.event = DCM_LOC_SERVER_REQUEST_OPEN;
   memcpy(&curr_evt.data.server_req_payload, &req->payload, sizeof req->payload);
   pthread_cond_signal(&atl_req_cond);

   pthread_mutex_unlock(&atl_req_mutex);

   return rc;
}

/*===========================================================================

FUNCTION loc_dcm_close_req_handler

DESCRIPTION
   Store a data close request and call processor

RETURN VALUE
   error code (0 for success)
===========================================================================*/
static int loc_dcm_close_req_handler(const rpc_loc_server_request_s_type *req)
{
   int rc = 0;

   loc_write_log("Received close Request\n");

   pthread_mutex_lock(&atl_req_mutex);

   /* pass req to dcm thread */
   curr_evt.event = DCM_LOC_SERVER_REQUEST_CLOSE;
   memcpy(&curr_evt.data.server_req_payload, &req->payload, sizeof req->payload);
   pthread_cond_signal(&atl_req_cond);

   pthread_mutex_unlock(&atl_req_mutex);

   return rc;
}

/*===========================================================================

FUNCTION loc_test_callback_dcm

DESCRIPTION
   Loc API callback handler

RETURN VALUE
   error code (0 for success)

===========================================================================*/
int loc_test_callback_dcm_always_on (
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
)
{
   int rc = 0;
   const rpc_loc_server_request_s_type *req = &loc_event_payload->rpc_loc_event_payload_u_type_u.loc_server_request;
   if (loc_event == RPC_LOC_EVENT_LOCATION_SERVER_REQUEST)
   {
      switch (req->event)
      {
      case RPC_LOC_SERVER_REQUEST_OPEN:
         loc_dcm_open_req_handler(req);
         break;
      case RPC_LOC_SERVER_REQUEST_CLOSE:
         loc_dcm_close_req_handler(req);
         break;
      default:
         break;
      }
   }
   return rc;
}

/*===========================================================================

FUNCTION gps_dcm_set_session_handle

===========================================================================*/
void gps_dcm_set_session_handle(rpc_loc_server_connection_handle sess_handle)
{
   pthread_mutex_lock(&state_info_mutex);
   loc_dcm_session_handle = sess_handle;
   pthread_mutex_unlock(&state_info_mutex);
}

/*===========================================================================

FUNCTION gps_dcm_get_session_handle

===========================================================================*/
rpc_loc_server_connection_handle gps_dcm_get_session_handle(void)
{
   return loc_dcm_session_handle;
}

/*===========================================================================

FUNCTION gps_dcm_report_result

DESCRIPTION
   Report server request result to Loc API

TODO
   Use QCRIL function qcril_data_get_call_info to get current APN name

===========================================================================*/
void gps_dcm_report_result(int is_open, int success)
{
   rpc_loc_ioctl_e_type ioctl_type;
   rpc_loc_ioctl_data_u_type data;
   int rc;

   if (is_open)
   {
      ioctl_type = RPC_LOC_IOCTL_INFORM_SERVER_OPEN_STATUS;
      data.rpc_loc_ioctl_data_u_type_u.conn_open_status.conn_handle = gps_dcm_get_session_handle();

#ifndef FEATURE_RPC_CHAR_ARRAY
      data.rpc_loc_ioctl_data_u_type_u.conn_open_status.apn_name = param.apn_name;
#else
      strlcpy(data.rpc_loc_ioctl_data_u_type_u.conn_open_status.apn_name, param.apn_name,
            sizeof data.rpc_loc_ioctl_data_u_type_u.conn_open_status.apn_name);
#endif /* FEATURE_RPC_CHAR_ARRAY */

      data.rpc_loc_ioctl_data_u_type_u.conn_open_status.open_status = success ? RPC_LOC_SERVER_OPEN_SUCCESS : RPC_LOC_SERVER_OPEN_FAIL;
   }
   else {
      /* is closing */
      ioctl_type = RPC_LOC_IOCTL_INFORM_SERVER_CLOSE_STATUS;
      data.rpc_loc_ioctl_data_u_type_u.conn_close_status.conn_handle = gps_dcm_get_session_handle();
      data.rpc_loc_ioctl_data_u_type_u.conn_close_status.close_status = (success ? RPC_LOC_SERVER_CLOSE_SUCCESS : RPC_LOC_SERVER_CLOSE_FAIL);
   }

   loc_write_log("Replying to server request open=%d succ=%d apn=\"%s\".\n", is_open, success, param.apn_name);
   rc = (int) loc_ioctl(sys.loc_handle, ioctl_type, &data);

   if (rc != RPC_LOC_API_SUCCESS)
   {
      loc_write_log("IOCTL RPC call error. Return code=%d\n", rc);
   }
}

/*===========================================================================

FUNCTION gps_dcm_thread_proc

DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/
void* gps_dcm_thread_proc(void *threadid)
{
   loc_dcm_event_s_type     evt;

   loc_write_msg("Starting DCM thread (assuming data always ON)...\n");

   while (1)
   {
      /* Fetch request */
      pthread_mutex_lock(&atl_req_mutex);
      pthread_cond_wait(&atl_req_cond, &atl_req_mutex);
      memcpy(&evt, &curr_evt, sizeof evt);
      pthread_mutex_unlock(&atl_req_mutex);

      switch(evt.event)
      {
      case DCM_LOC_SERVER_REQUEST_OPEN:
         gps_dcm_set_session_handle(evt.data.server_req_payload.rpc_loc_server_request_u_type_u.open_req.conn_handle); /* save handle */

         /* Wait a little bit */
         sleep(LOC_DS_REPLY_WAIT);

         /* Directly return success and APN name (if specified in INI) */
         gps_dcm_report_result(1, 1); /* open, succ */
         break;

      case DCM_LOC_SERVER_REQUEST_CLOSE:
         gps_dcm_set_session_handle(evt.data.server_req_payload.rpc_loc_server_request_u_type_u.close_req.conn_handle); /* save handle */

         /* Wait a little bit */
         sleep(LOC_DS_REPLY_WAIT);

         /* Directly return success */
         gps_dcm_report_result(0, 1); /* close, succ */
         break;
      default:
         break;
      }
   } /* while (1) */

   pthread_exit(NULL);
   return NULL;
}

/*===========================================================================

FUNCTION gps_dcm_thread_start

DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/
int gps_dcm_thread_start(void)
{
   int rc;

   rc = pthread_create(&loc_dcm_thread, NULL, gps_dcm_thread_proc, NULL);

   if(rc)
   {
      loc_write_error("Loc DCM thread is not created.\n");
      return -1;
   }

   return 0;
}

