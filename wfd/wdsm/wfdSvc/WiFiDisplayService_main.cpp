/*==============================================================================
*       WiFiDisplayService_main.cpp
*
*  DESCRIPTION:
*       Main entry point for native wfdservice
*
*
*  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
11/06/2014                    InitialDraft
================================================================================
*/

/*==============================================================================
**               Includes and Public Data Declarations
**==============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "WiFiDisplayService"
#endif

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>

#include "WiFiDisplayService.h"

int main(int argc, char** argv)
{
    argc,argv;
    ALOGE("Entered main routine of WiFiDisplayService");
    WiFiDisplayService::publishService();
    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();
    ALOGE("Exiting main routine of WiFiDisplayService");
}
