/******************************************************************************
  @file:  loc_api_cb_hub.h
  @brief:  header for loc_api_cb_hub.c

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

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_cb.h#5 $
======================================================================*/

#ifndef LOC_API_CB_HUB_H
#define LOC_API_CB_HUB_H

extern void loc_select_callback(
      rpc_loc_event_mask_type event_mask,                /* Event mask to wait for             */ 
      rpc_loc_event_payload_u_type *callback_payload     /* Pointer to callback payload buffer */
);

extern int loc_wait_callback(
      int timeout_seconds                            /* Timeout in this number of seconds  */
);

extern int32 loc_test_callback_handler(
      void*                                 userData,
      rpc_loc_client_handle_type            loc_handle,             /* handle of the client */
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
);

#endif /* LOC_API_CB_HUB_H */
