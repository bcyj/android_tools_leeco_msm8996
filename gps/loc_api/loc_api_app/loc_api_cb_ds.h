/******************************************************************************
  @file:  loc_api_cb_ds.h
  @brief:  module for loc_api callback about data services

  DESCRIPTION
     LOC_API callback data services header

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

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_cb_log.h#4 $
======================================================================*/

#ifndef LOC_API_CB_DS_H
#define LOC_API_CB_DS_H

#define   GPS_DCM_DEFAULT_PDP_PROF  1
#define   GPS_DCM_PPP_OPEN_TIMEOUT_VALUE    15

typedef enum
{
   GPS_DCM_STATE_CLOSE = 0,
   GPS_DCM_STATE_OPEN_INPROGRESS,
   GPS_DCM_STATE_OPENED,
   GPS_DCM_STATE_CLOSE_INPROGRESS
} gps_dcm_state_e_type;

typedef struct
{
   gps_dcm_state_e_type          data_call_state;
   rpc_loc_server_connection_handle  session_handle;
   int                           qmi_wds_clnt_id;
   int                           recover_timer_count;
} gps_dcm_state_info_s_type;

/* DCM event enum type */
typedef enum
{
   DCM_LOC_SERVER_REQUEST_OPEN  = 1, /* Connection to be opened */
   DCM_LOC_SERVER_REQUEST_CLOSE = 2, /* Connection to be closed */
   DCM_DS_NET_CALLBACK          = 3, /* Data service net call back */
} loc_dcm_event_e_type;

/* DCM event data */
typedef struct
{
   loc_dcm_event_e_type            event;
   union {
      rpc_loc_server_request_u_type    server_req_payload;
   } data;
} loc_dcm_event_s_type;

extern int loc_test_callback_dcm (
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
);

extern int gps_dcm_thread_start(void);
extern void gps_dcm_timer_kick(void);

#endif /* LOC_API_CB_DS_H */
