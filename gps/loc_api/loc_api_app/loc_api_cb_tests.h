/******************************************************************************
  @file:  loc_api_cb_tests.h
  @brief:  module for loc_api callback testing

  DESCRIPTION
     LOC_API callback tests header
     
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

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_cb_tests.h#1 $
======================================================================*/

#ifndef LOC_API_CB_TESTS_H
#define LOC_API_CB_TESTS_H

typedef void (loc_cb_f_type) (
      rpc_loc_event_mask_type               loc_event,              /* event mask */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* event payload */
      );

/********************************************************
 * EXPORTED CALLBACK TEST FUNCTIONS 
 *******************************************************/ 
void loc_cb_sv_drop_test(
      rpc_loc_event_mask_type               loc_event,              /* event mask */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* event payload */
      );

#endif /* LOC_API_CB_TESTS_H */
