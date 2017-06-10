/******************************************************************************

                        N E T M G R _ S T U B S . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_stubs.C
  @brief   Network Manager test stubs

  DESCRIPTION
  NetMgr test stub functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2012 Qualcomm Technologies, Inc. All Rights Reserved

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

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/10/10   ar         Initial version

******************************************************************************/

#ifdef NETMGR_OFFTARGET

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ds_util.h"
#include "netmgr_util.h"
#include "netmgr_exec.h"
#include "linux/msm_rmnet.h"

int atexit(void (*function)(void))
{
  return 0;
}

/*-----------------------------------------------------------------------*/

#define MAX_CMD_LEN 256

/* Present in Android builds */
#define SIOCKILLADDR    0x8939
#define SIOCGIFFLAGS    0x8913
#define SIOCSIFFLAGS    0x8914
#define SIOCSIFNETMASK  0x891c


#endif /* NETMGR_OFFTARGET */

