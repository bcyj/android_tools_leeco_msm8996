/******************************************************************************
  @file:  loc_api_cb_ds_on.h
  @brief:  module for loc_api callback for always-on data services

  DESCRIPTION
     LOC_API callback data services header assuming the data connection is
     always ON (for Android)

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

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_cb_log.h#4 $
======================================================================*/

#ifndef LOC_API_CB_DS_ON_H
#define LOC_API_CB_DS_ON_H

#define LOC_DS_REPLY_WAIT 1  /* seconds */

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

extern int loc_test_callback_dcm_always_on (
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
);

extern int gps_dcm_thread_start(void);

#endif /* LOC_API_CB_DS_ON_H */
