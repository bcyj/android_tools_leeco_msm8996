/******************************************************************************
  @file:  loc_api_cb_tests.c
  @brief:  module for loc_api callback tests

  DESCRIPTION
     LOC_API callback tests
     
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

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_cb_tests.c#1 $
======================================================================*/

#include <stdio.h>
#include <stdlib.h>

#include "loc_api_rpc_glue.h"
#include "loc_api_test.h"
#include "loc_api_data.h"
#include "loc_api_cb_tests.h"

/* Sudden SV drop test */ 
void loc_cb_sv_drop_test(
      rpc_loc_event_mask_type               loc_event,              /* event mask */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* event payload */
      )
{
   static int last_sv_count = -1;
   static int num_drops = 0;
   int max_allowed_drop = 2;   /* report abnormality if satellite drop exceeds this number */
   if (loc_event == RPC_LOC_EVENT_SATELLITE_REPORT)
   {
      const rpc_loc_gnss_info_s_type *gnss = &loc_event_payload->rpc_loc_event_payload_u_type_u.gnss_report;
      if (gnss->valid_mask & RPC_LOC_GNSS_INFO_VALID_SV_COUNT)
      {
         /* decreasing, add drops */
         if (gnss->sv_count < last_sv_count)
         {
            num_drops += (last_sv_count - gnss->sv_count);
         }
         
         /* increasing, reset drops */
         if (gnss->sv_count > last_sv_count)
         {
            num_drops = 0;
         }
         
         if (num_drops > max_allowed_drop)
         {
            failure("Sudden satellite drop detected.");
         }
         last_sv_count = gnss->sv_count;
      }
   }
}
