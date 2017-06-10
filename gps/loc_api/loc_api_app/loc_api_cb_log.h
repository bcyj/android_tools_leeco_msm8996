/******************************************************************************
  @file:  loc_api_cb_log.h
  @brief:  module for loc_api callback logging

  DESCRIPTION
     LOC_API callback logger header
     
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

#ifndef LOC_API_CB_LOG_H
#define LOC_API_CB_LOG_H

extern int loc_test_callback_log_header(
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
);

extern int loc_test_callback_log(
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
);

extern char* loc_get_event_name(rpc_loc_event_mask_type loc_event_mask);
extern char* loc_get_ioctl_status_name(uint32 status);
extern char* loc_get_sess_status_name(rpc_loc_session_status_e_type status);

#endif /* LOC_API_CB_LOG_H */
