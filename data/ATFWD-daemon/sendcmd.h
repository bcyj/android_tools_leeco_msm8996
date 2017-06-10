/*!
  @file
  sendcmd.h

  @brief
	Contains functions required to send commands to android through the
	AtCmdFwd Service
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


when       who      what, where, why
--------   ---      ---------------------------------------------------------
04/11/11   jaimel    First cut.


===========================================================================*/

#ifndef __ATFWDDAEMON_H
#define __ATFWDDAEMON_H

#include "AtCmdFwd.h"

#define ATFWD_RETRY_DELAY                 5  /* Retry delay in sec */
#define ATFWD_MAX_RETRY_ATTEMPTS          5

#ifdef __cplusplus
extern "C" {
#endif

int initializeAtFwdService();
AtCmdResponse *sendit(const AtCmd *cmd);
void millisecondSleep(int milliseconds);

#ifdef __cplusplus
}
#endif

#endif /* __ATFWDDAEMON_H */
