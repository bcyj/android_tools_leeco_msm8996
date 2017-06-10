/*!
  @file
  qcrilhook_oem.c

  @brief
  Handles RIL requests which are OEM specfic .

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

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/15/09   nrn      Initial supporting code for NAM programming.

===========================================================================*/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "comdef.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "qcrilhook_oem.h"


/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/


/*===========================================================================

                                LOCAL VARIABLES

===========================================================================*/

/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcrilhook_oem

===========================================================================*/
/*!
    @brief
    Handles qcrilhook.

    @return
    Success
*/
/*=========================================================================*/
void qcrilhook_oem
(
  int           instance_id,
  int           request,
  char          *data,
  size_t        data_len,
  RIL_Token     t
)
{
  /* nothing to do here, has to done by oem */
    QCRILHOOK_NOTUSED(instance_id);
    QCRILHOOK_NOTUSED(request);
    QCRILHOOK_NOTUSED(data);
    QCRILHOOK_NOTUSED(data_len);
    QCRILHOOK_NOTUSED(t);
} /* end of qcrilhook_oem */


