/******************************************************************************
  @file:  loc_api_cb_ds.c
  @brief:  module for loc_api callback on data services

  DESCRIPTION
     LOC_API callback data services handlers

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

#include "loc_api_rpc_glue.h"
#include "loc_api_test.h"
#include "loc_api_data.h"
#include "loc_api_cb_ds.h"

#ifdef FEATURE_GPS_DCM_ENABLED
#include "dssocket.h"
#endif /* FEATURE_GPS_DCM_ENABLED */

/*=============================================================================
 *
 *                             DATA DECLARATION
 *
 *============================================================================*/

pthread_t loc_dcm_thread;  /* data call manager thread */
sint15 gps_dcm_nethandle;  /* dsnet handle */

gps_dcm_state_info_s_type  gps_dcm_state_info; /* DCM module state */
pthread_mutex_t state_info_mutex        = PTHREAD_MUTEX_INITIALIZER;

loc_dcm_event_s_type curr_evt;
pthread_mutex_t atl_req_mutex          = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  atl_req_cond           = PTHREAD_COND_INITIALIZER;

extern int dsc_main(int argc, char ** argv);

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

   if (gps_dcm_state_info.data_call_state == GPS_DCM_STATE_OPENED)
   {
      loc_write_error("PPP already opened\n");
      // Count as success
   }
   else
   {
      pthread_mutex_lock(&atl_req_mutex);

      /* pass req to dcm thread */
      curr_evt.event = DCM_LOC_SERVER_REQUEST_OPEN;
      memcpy(&curr_evt.data.server_req_payload, &req->payload, sizeof req->payload);
      pthread_cond_signal(&atl_req_cond);

      pthread_mutex_unlock(&atl_req_mutex);
   }

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

   if(gps_dcm_state_info.data_call_state == GPS_DCM_STATE_CLOSE)
   {
      loc_write_error("PPP already closed\n");
   }
   else
   {
      pthread_mutex_lock(&atl_req_mutex);

      /* pass req to dcm thread */
      curr_evt.event = DCM_LOC_SERVER_REQUEST_CLOSE;
      memcpy(&curr_evt.data.server_req_payload, &req->payload, sizeof req->payload);
      pthread_cond_signal(&atl_req_cond);

      pthread_mutex_unlock(&atl_req_mutex);
   }

   return rc;
}

/*===========================================================================

FUNCTION loc_test_callback_dcm

DESCRIPTION
   Loc API callback handler

RETURN VALUE
   error code (0 for success)

===========================================================================*/
int loc_test_callback_dcm (
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
)
{
   int rc = 0;
   const rpc_loc_server_request_s_type *req = &loc_event_payload->loc_server_request;
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

FUNCTION gps_dcm_net_cb

DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/
void gps_dcm_net_cb
(
      sint15            dss_nethandle,               /* Application id */
      dss_iface_id_type iface_id,                    /* Interfcae id structure */
      sint15            dss_errno,                   /* type of network error, ENETISCONN, ENETNONET.*/
      void            * net_cb_user_data             /* Call back User data  */
)
{
   if(dss_nethandle == gps_dcm_nethandle)
   {
      loc_write_log("Recv'd network callback dss_errno = %d\n", dss_errno);

      pthread_mutex_lock(&atl_req_mutex);

      curr_evt.event = DCM_DS_NET_CALLBACK;

      pthread_cond_signal(&atl_req_cond);

      pthread_mutex_unlock(&atl_req_mutex);
   }
   return;
}

/*===========================================================================

FUNCTION gps_dcm_socket_cb

DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/
void gps_dcm_socket_cb
(
      sint15 dss_nethandle,          /* Application id */
      sint15 sockfd,                 /* socket descriptor */
      uint32 event_mask,             /* Event occurred */
      void * sock_cb_user_data       /* User data specfied during registration */
)
{
   return;
}


/*===========================================================================

FUNCTION gps_dcm_set_session_handle

===========================================================================*/
void gps_dcm_set_session_handle(rpc_loc_server_connection_handle sess_handle)
{
   pthread_mutex_lock(&state_info_mutex);
   gps_dcm_state_info.session_handle = sess_handle;
   pthread_mutex_unlock(&state_info_mutex);
}

/*===========================================================================

FUNCTION gps_dcm_get_session_handle

===========================================================================*/
rpc_loc_server_connection_handle gps_dcm_get_session_handle(void)
{
   return gps_dcm_state_info.session_handle;
}

/*===========================================================================

FUNCTION gps_dcm_qmi_update_data_call_state

DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/
void gps_dcm_qmi_update_data_call_state(gps_dcm_state_e_type new_state)
{
   pthread_mutex_lock(&state_info_mutex);
   gps_dcm_state_info.data_call_state = new_state;

   if(new_state != GPS_DCM_STATE_OPEN_INPROGRESS)
   {
      gps_dcm_state_info.recover_timer_count = 0;
   }

   loc_write_log("data call state = %d\n", new_state);
   pthread_mutex_unlock(&state_info_mutex);
}

/*===========================================================================

FUNCTION gps_dcm_report_result

DESCRIPTION
   Report server request result to Loc API

===========================================================================*/
void gps_dcm_report_result(int is_open, int success)
{
   rpc_loc_ioctl_data_u_type data;
   rpc_loc_ioctl_e_type ioctl_type;

   if (is_open)
   {
      ioctl_type = RPC_LOC_IOCTL_INFORM_SERVER_OPEN_STATUS;
      data.conn_open_status.conn_handle = gps_dcm_get_session_handle();
      data.conn_open_status.apn_name[0] = '\0'; /* FIXME alloc memory for rpc version, fill in real APN if specified */
      data.conn_open_status.open_status = success ? RPC_LOC_SERVER_OPEN_SUCCESS : RPC_LOC_SERVER_OPEN_FAIL;
   }
   else {
      /* is closing */
      ioctl_type = RPC_LOC_IOCTL_INFORM_SERVER_CLOSE_STATUS;
      data.conn_close_status.conn_handle = gps_dcm_get_session_handle();
      data.conn_close_status.close_status = (success ? RPC_LOC_SERVER_CLOSE_SUCCESS : RPC_LOC_SERVER_CLOSE_FAIL);
   }

   loc_ioctl(sys.loc_handle, ioctl_type, &data);
}

/*===========================================================================

FUNCTION gps_dcm_init_state

DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/
void gps_dcm_init_state(void)
{
   gps_dcm_state_info.data_call_state = GPS_DCM_STATE_CLOSE;
   gps_dcm_nethandle = DSS_ERROR;
}

/*===========================================================================

FUNCTION gps_dcm_open

DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/
static void gps_dcm_open
(
      rpc_loc_server_connection_handle  session_handle,
      rpc_loc_server_protocol_e_type protocol
)
{
   dss_net_policy_info_type net_policy;
   sint15 dss_errno;
   int dss_status;

   dss_init_net_policy_info(&net_policy);

   net_policy.iface.kind = DSS_IFACE_NAME;
   net_policy.iface.info.name = DSS_IFACE_UMTS;
   net_policy.umts.pdp_profile_num = GPS_DCM_DEFAULT_PDP_PROF;

   gps_dcm_nethandle = dsnet_get_handle(&gps_dcm_net_cb,    (void *)0,
         &gps_dcm_socket_cb, (void *)0,
         &net_policy, &dss_errno);

   if(gps_dcm_nethandle == DSS_ERROR)
   {
      loc_write_error("dsnet_get_handle failed, error_no = %d\n", dss_errno);

      /* Inform Loc API open failure */
      gps_dcm_report_result(1 /* open */, 0 /* failed */);

      gps_dcm_qmi_update_data_call_state(GPS_DCM_STATE_CLOSE);

      return;
   }

   dss_status = dsnet_start(gps_dcm_nethandle, &dss_errno);

   if( (DS_ENETEXIST == dss_errno)
         ||(DS_ENETISCONN == dss_errno))
   {
      /* Inform Loc API open success */
      gps_dcm_report_result(1 /* open */, 1 /* success */);

      gps_dcm_qmi_update_data_call_state(GPS_DCM_STATE_OPENED);

      return;
   }
   else if(DS_EWOULDBLOCK == dss_errno)
   {
      /* Inform PDComm open result later */
      loc_write_log("dsnet_start returned DS_EWOULDBLOCK\n");
      gps_dcm_qmi_update_data_call_state(GPS_DCM_STATE_OPEN_INPROGRESS);
      return;
   }
   else
   {
      loc_write_error("dsnet_start failed, error_no = %d", dss_errno);

      dsnet_release_handle(gps_dcm_nethandle, &dss_errno);

      gps_dcm_nethandle = DSS_ERROR;

      gps_dcm_qmi_update_data_call_state(GPS_DCM_STATE_CLOSE);

      /* Inform Loc API open failure */
      gps_dcm_report_result(1 /* open */, 0 /* failed */);

      return;
   }
}


/*===========================================================================

FUNCTION gps_dcm_close

DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/
static void gps_dcm_close
(
      rpc_loc_server_connection_handle  session_handle
)
{
   int dss_status;
   sint15 dss_errno;

   dss_status = dsnet_stop(gps_dcm_nethandle, &dss_errno);

   if(dss_status == DSS_SUCCESS)
   {
      /* Inform Loc API success event now */
      gps_dcm_report_result(0, 1); /* close, success */

      dsnet_release_handle(gps_dcm_nethandle, &dss_errno);
      gps_dcm_nethandle = DSS_ERROR;
      gps_dcm_qmi_update_data_call_state(GPS_DCM_STATE_CLOSE);

      return;
   }
   else
   {
      if(dss_errno != DS_EWOULDBLOCK)
      {
         loc_write_error("dsnet_stop failed, error_no = %d\n", dss_errno);

         /* Inform Loc API success event now */
         gps_dcm_report_result(0, 0); /* close, failed */

         dsnet_release_handle(gps_dcm_nethandle, &dss_errno);
         gps_dcm_nethandle = DSS_ERROR;
         gps_dcm_qmi_update_data_call_state(GPS_DCM_STATE_CLOSE);

         return;
      }
      else
      {
         loc_write_log("Waiting for net callback\n");
         gps_dcm_qmi_update_data_call_state(GPS_DCM_STATE_CLOSE_INPROGRESS);

         return;
      }
   }
}

/*===========================================================================

FUNCTION gps_dcm_network_callback

DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/
void gps_dcm_network_callback(rpc_loc_server_connection_handle  session_handle)
{
   sint15 dss_errno;

   enum { SUCCESS, FAIL, IN_PROGRESS, NR_NULL } net_result = NR_NULL;
   enum { OPEN, CLOSE, S_NULL } state = S_NULL;

   if (gps_dcm_state_info.data_call_state == GPS_DCM_STATE_OPEN_INPROGRESS)
   {
      /***********************************
       *  OPENING
       **********************************/
      state = OPEN;
      dss_netstatus(gps_dcm_nethandle, &dss_errno);

      loc_write_log("dss_errno = %d", dss_errno);

      if( (dss_errno == DS_ENETISCONN)
            ||(dss_errno == DS_ENETEXIST))
      {
         loc_write_log("PPP Opened\n");
         gps_dcm_qmi_update_data_call_state(GPS_DCM_STATE_OPENED);
         net_result = SUCCESS;
      }
      else if(dss_errno == DS_ENETINPROGRESS)
      {
         loc_write_log("PPP Open in progress\n");
         net_result = IN_PROGRESS;
      }
      else
      {
         loc_write_error("PPP Open failed\n");
         gps_dcm_qmi_update_data_call_state(GPS_DCM_STATE_CLOSE);
         net_result = FAIL;
      }
   }
   else if (gps_dcm_state_info.data_call_state == GPS_DCM_STATE_CLOSE_INPROGRESS)
   {
      /***********************************
       *  CLOSING
       **********************************/
      state = CLOSE;
      dss_netstatus(gps_dcm_nethandle, &dss_errno);

      loc_write_log("dss_errno = %d", dss_errno);

      if(dss_errno == DS_ENETNONET)
      {
         dsnet_release_handle(gps_dcm_nethandle, &dss_errno);

         loc_write_log("PPP closed\n");
         gps_dcm_qmi_update_data_call_state(GPS_DCM_STATE_CLOSE);
         net_result = SUCCESS;
      }
      else if(dss_errno == DS_ENETCLOSEINPROGRESS)
      {
         loc_write_log("PPP close in progress\n");
         net_result = IN_PROGRESS;
      }
      else
      {
         dsnet_release_handle(gps_dcm_nethandle, &dss_errno);

         loc_write_error("PPP close failed\n");
         gps_dcm_qmi_update_data_call_state(GPS_DCM_STATE_CLOSE);
         net_result = FAIL;
      }
   }
   else
   {
      /***********************************
       *  NOTHING
       **********************************/
      loc_write_log("Ignore network event\n");
   }

   /***********************************
    *  REPORT TO LOC API
    **********************************/
   if (
         (state == OPEN || state == CLOSE) &&
         (net_result == SUCCESS || net_result == FAIL)
   )
   {
      gps_dcm_report_result(state == OPEN, net_result == SUCCESS);
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
   int                      dsc_input_len = 0;
   char *dscmain_input[] = {
         "testprog",
         "-f",
         "-s",
         "-k",
         "-l",
         "0",
         "-i",
         "rmnet",
         "-u",
         "/opt/qcom/bin/udhcpc.sh",
         "-m",
         "/opt/qcom/bin/qcomdsc-kif.sh",
         "-t",
         "umts",
         NULL  /* end mark */
   };

   loc_write_msg("Starting Loc DCM thread...\n");

   /* Init this module */
   gps_dcm_init_state();

   /* Count DSC parameters */
   dsc_input_len = 0;
   while (dscmain_input[dsc_input_len])
   {
      dsc_input_len++;
   }
   /* Call DSC */
   dsc_main(dsc_input_len, dscmain_input);

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
         /* save session handle */
         gps_dcm_set_session_handle(evt.data.server_req_payload.open_req.conn_handle);
         gps_dcm_open(
               evt.data.server_req_payload.open_req.conn_handle,
               evt.data.server_req_payload.open_req.protocol);
         break;

      case DCM_LOC_SERVER_REQUEST_CLOSE:
         /* save session handle */
         gps_dcm_set_session_handle(evt.data.server_req_payload.open_req.conn_handle);
         gps_dcm_close(
               evt.data.server_req_payload.close_req.conn_handle
         );
         break;

      case DCM_DS_NET_CALLBACK:
         gps_dcm_network_callback(
               gps_dcm_get_session_handle() /* get last session handle */
         );
         break;

      default:
         break;
      }
   } /* while (1) */

   pthread_exit(NULL);
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

