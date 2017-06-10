/*!
  @file
  sendcmd.cpp

  @brief
  Places a Remote Procedure Call (RPC) to Android's AtCmdFwd Service

*/

/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.


when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/11/11   jaimel   First cut.


===========================================================================*/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#define LOG_NDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0
#define LOG_TAG "Atfwd_Sendcmd"
#include <utils/Log.h>
#include "common_log.h"

#include "IAtCmdFwdService.h"
#include <binder/BpBinder.h>
#include <binder/IServiceManager.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "sendcmd.h"
#define MAX_KEYS 57

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>

namespace android {

/*===========================================================================

                           Global Variables

===========================================================================*/

sp<IAtCmdFwdService> gAtCmdFwdService; //At Command forwarding sevice object
sp<DeathNotifier> mDeathNotifier;

/*===========================================================================

                          Extern functions invoked from CKPD daemon

===========================================================================*/

extern "C" int initializeAtFwdService();
extern "C" int pressit(char key, int keyPressTime, int timeBetweenKeyPresses);
extern "C" void millisecondSleep(int milliseconds);

/*===========================================================================
  FUNCTION  initializeAtFwdService
===========================================================================*/
/*!
@brief
     Initializes the connection with the Window Manager service
@return
  Returns 0 if service intialization was successful; -1 otherwise

@note
  None.
*/
/*=========================================================================*/

extern "C" int initializeAtFwdService()
{
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder;
    int retryCnt = 1;
    if(sm == 0) {
        LOGE("Could not obtain IServiceManager \n");
        return -1;
    }

    do {
        binder = sm->getService(String16("AtCmdFwd"));
        if (binder == 0) {
            LOGW("AtCmdFwd service not published, waiting... retryCnt : %d", retryCnt);
            /*
             * Retry after (retryCnt * 5)s and yield in the cases when AtCmdFwd service is
             * is about to be published
             */
            sleep(retryCnt * ATFWD_RETRY_DELAY);
            ++retryCnt;
            continue;
        }

        break;
    } while(retryCnt <= ATFWD_MAX_RETRY_ATTEMPTS);

    if (binder == 0) {
        LOGI("AtCmdFwd service not ready - Exhausted retry attempts - :%d",retryCnt);
        //property_set("ctl.stop", "atfwd");
        return -1;
    }
    if (mDeathNotifier == NULL) {
        mDeathNotifier = new DeathNotifier();
    }
    binder->linkToDeath(mDeathNotifier);

    gAtCmdFwdService = interface_cast<IAtCmdFwdService>(binder);
    if (gAtCmdFwdService == 0)
    {
        LOGE("Could not obtain AtCmdFwd service\n");
        return -1;
    }

    // Start a Binder thread pool to receive Death notification callbacks
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();
    return 0;
}

/*===========================================================================
  FUNCTION  sendit
===========================================================================*/
/*!
@brief
     Invokes a Remote Procedure Call (RPC) to Android's Window Manager Service
     Window Manager service returns 0 if the call is successful
@return
  Returns 1 if the key press operation was successful; 0 otherwise

@note
  None.
*/
/*=========================================================================*/

extern "C" AtCmdResponse *sendit(const AtCmd *cmd)
{
    AtCmdResponse *result;

    if (!cmd) return NULL;

    result = gAtCmdFwdService->processCommand(*cmd);

    return result;
}

void DeathNotifier::binderDied(const wp<IBinder>& who) {
    LOGI("AtCmdFwd : binderDied");
    initializeAtFwdService();
}

};  /* namespace android */
