/******************************************************************************

                 C O M M A N D A P I . H

******************************************************************************/

/******************************************************************************

  @file    CommandApi.h
  @brief   Interface of extern'd declarations. This file needs to be included
           by clients who wants to exercise CommandSvc proxy functionality

===========================================================================*/

/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved

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

#ifndef __COMMANDAPI_H
#define __COMMANDAPI_H

#include "comdef.h"

#ifdef __cplusplus
extern "C" {
#endif

boolean enableDataConnectivity(boolean enable);

#ifdef __cplusplus
}
#endif

#endif // __COMMANDAPI_
