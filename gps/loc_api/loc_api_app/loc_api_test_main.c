/******************************************************************************
  @file:  loc_api_test_main.c
  @brief: loc_api_test main() function

  DESCRIPTION
      Entry point of the loc_api_test application.

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
01/09/09   dx       Initial version

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_test_main.c#3 $
======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "loc_api_rpc_glue.h"
#include "loc_api_cmd.h"
#include "loc_api_test.h"

/*===========================================================================

FUNCTION    MAIN

DESCRIPTION
   The main function should register the test appplication's command line
   interpreter and functions that depends on whether CINT is included. Finally,
   the function should call loc_test_general_main() to continue the starting up
   process.

DEPENDENCIES

RETURN VALUE
   Application exit code. 0 if successful.

SIDE EFFECTS

===========================================================================*/
int main(int argc, char *argv[])
{
   loc_test_reg_cmd_loop(loc_test_default_cmd_loop);
   loc_test_reg_cmd_proc(loc_test_default_cmdline_proc);
   return loc_test_general_main(argc, argv);
}
