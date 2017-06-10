/*!
  @file
  qcrilhook_oem.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2008 Qualcomm Technologies, Inc. All Rights Reserved

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

when         who     what, where, why
--------   ---     ----------------------------------------------------------
06/15/09   nrn      Initial supporint code for NAM programming.

===========================================================================*/

#ifndef QCRILHOOK_OEM_H
#define QCRILHOOK_OEM_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "ril.h"
#define QCRILHOOK_NOTUSED( var ) ((var) == (var))
/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

void qcrilhook_oem( int instance_id, int request, char *data, size_t data_len, RIL_Token t );

#endif /* QCRILHOOK_OEM_H */
