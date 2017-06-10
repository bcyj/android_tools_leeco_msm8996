/******************************************************************************
  @file:  loc_api_data.c
  @brief:

  DESCRIPTION
    This file contains LOC_API test application data that are directly
  accessible through the test command line. It will be parsed by CINT to
  enable run-time access. CINT is only 95% compatible with ANSI C. If any
  data in this file fails CINT scan during compilation, it can be moved to
  loc_api_app.c to solve the problem.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2007 Qualcomm Technologies, Inc.
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
01/01/09   dx       Initial version

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_data.c#15 $
======================================================================*/

/*=====================================================================

                     INCLUDE FILES FOR MODULE

======================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stringl.h>
#ifdef USE_GLIB
#include <glib.h>
#endif /*USE_GLIB */
#include "loc_api_test.h"
#include "loc_api_data.h"
#include "loc_api_ini.h"
#include "platform_lib_includes.h"
/*=============================================================================
 *
 *                        GLOBAL DATA DECLARATION
 *
 *============================================================================*/

/* Application variables */
loc_sys_data_s_type sys;

/* Application parameters */
loc_param_data_s_type param;

/* Last callback */
rpc_loc_event_payload_u_type lastcb;

/* Last data struct */
loc_last_data_s_type last;

/* Selected callback test */
loc_cb_f_type *cbf = NULL;

/* Test mode flags */
loc_test_mode_s_type tmode;

/*=============================================================================
 *
 *                        GLOBAL DATA INITIALIZATION
 *
 *============================================================================*/

/*===========================================================================
FUNCTION loc_api_data_reset

DESCRIPTION
   This function resets the global data before each test

DEPENDENCIES
   N/A

RETURN VALUE
   none

SIDE EFFECTS
   N/A
===========================================================================*/
void loc_api_data_reset(void)
{
   /* sys */
   sys.app_code = 0;
   sys.api_code = 0;
   sys.cbw_code = 0;
   sys.ioctl_code = 0;
   sys.loc_handle = -1;
   sys.fix_started = 0;
   sys.lcd_started = 0;
   sys.unit_test_ticks = 0;
   sys.unit_test_line = 0;
   sys.unit_test_quit = 0;
   
   /* param */
   memset(param.xtra_server_addr, 0, sizeof param.xtra_server_addr);
   memset(param.apn_name, 0, sizeof param.apn_name);

   /* tmode */
   tmode.xtra_inject_incomplete = 0;
   tmode.xtra_inject_out_of_order = 0;
}

/*===========================================================================
FUNCTION loc_api_data_init

DESCRIPTION
   This function initializes the global data in sys struct

DEPENDENCIES
   N/A

RETURN VALUE
   none

SIDE EFFECTS
   N/A
===========================================================================*/
void loc_api_data_init(void)
{
   /* sys */
   sys.rpc_started = 0;
   sys.dcm_started = 0;   
   sys.disable_dcm = 0;
   sys.unit_test_mode = 0;
   sys.unit_test_timeout = 300;   
      
   memset(sys.unit_test_script, 0, sizeof sys.unit_test_script);
   memset(sys.unit_test_filelist, 0, sizeof sys.unit_test_filelist);
   strlcpy(sys.log_path, LOC_MAIN_LOG_FILE, sizeof sys.log_path);
   
   loc_api_data_reset();
}
