/******************************************************************************
  @file:  loc_api_cb_ni.h
  @brief:  module for network initiated interactions

  DESCRIPTION
     LOC_API callback handling for network initiated interactions

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

#ifndef LOC_API_CB_NI_H
#define LOC_API_CB_NI_H

#define LOC_NI_NO_RESPONSE_TIME            15                      /* secs */

extern boolean loc_ni_need_input;   /* next input will be processed by NI */

extern int loc_test_callback_ni (
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
);

extern int loc_ni_cmd_handler(int argc, char* argv[]);
extern int loc_ni_thread_start(void);

#endif /* LOC_API_CB_NI_H */
