/******************************************************************************

                 C O M M A N D A P I . CPP

******************************************************************************/

/******************************************************************************

  @file    CommandApi.cpp
  @brief   Implementation of extern'd interfaces.

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
#include "IPhoneService.h"

namespace android {

#ifdef __cplusplus
extern "C" {
#endif

bool enableDataConnectivity(bool enable) {
    return IPhoneService_enableDataConnectivity(enable);
}

#ifdef __cplusplus
}
#endif
};
