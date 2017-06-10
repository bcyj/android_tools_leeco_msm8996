/* ========================================================================= * 
   Purpose:  Shared object Helper library used for fuzzing APIs

           -------------------------------------------------------
      Copyright © 2012 Qualcomm Technologies, Inc. All Rights Reserved.
              Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */
#include <stdio.h>
#include <string.h>
#include "cFuzzerLog.h"
#include "FuzzerLibs.h"
#include "QCamera2HAL.h"
extern cFuzzerLog cFL ;
#include "QCamera2HALFuzzLibHelper.h"

/*==========================================================================*
	Name: int init_module_helper()

	Input:
	       None

	Return:
           SUCCESS : RET_SUCCESS ( 0)
           FAILURE : RET_FAILURE (-1)

	Purpose:
	  This is the init function. Make all initializations here
*==========================================================================*/
extern "C" int init_module_helper()
{     
    //
    //    Add your initialization code here
    //
    //
    printf("Calling Fuzz HAL init\n");
    Fuzzer_HAL_init();   
    return RET_SUCCESS;
}

/*==========================================================================*
	Name: int deinit_module_helper()

	Input:
	       None

	Return:
           SUCCESS : RET_SUCCESS ( 0)
           FAILURE : RET_FAILURE (-1)

	Purpose:
	  This is the de-init function. Free all resources here
*==========================================================================*/
extern "C" int deinit_module_helper()
{ 
    //
    //    Free resources here
    //
    //
    Fuzzer_HAL_deinit();
    return RET_SUCCESS;
}

