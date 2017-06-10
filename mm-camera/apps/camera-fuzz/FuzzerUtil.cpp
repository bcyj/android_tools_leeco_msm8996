/* ========================================================================= *
   Purpose:  Module contains utility functions that will be used
             by cFuzzer

   Copyright (c) 2005-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */
#include <stdio.h>
#include <string.h>
#include "cFuzzerLog.h"
#include "FuzzerLibs.h"

extern cFuzzerLog cFL ;


/*==========================================================================*
	Name: char* getLibVersion()

	Input:
	       None

	Return:
          Returns the version information of this library

	Purpose:
	  This function will be called by CFuzzer to determine the fuzz library's
     version information
*==========================================================================*/
extern "C" char* getLibVersion()
{
    //DBGPRT(LEVEL_INFO, (char *)"getLibVersion Called");
    //DBGPRT(LEVEL_INFO, (char *)"Library Version Info : %s", VERSION_INFO);
    return VERSION_INFO;
}

