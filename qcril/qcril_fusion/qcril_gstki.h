/*!
  @file
  qcril_gstki.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc. All Rights Reserved

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

$Header: //depot/restricted/linux/android/ril/qcril_gstki.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
04/05/09   fc      Cleanup log macros.
01/26/08   fc      Logged assertion info.
07/09/08   adg     initial version

===========================================================================*/

#ifndef QCRIL_GSTKI_H
#define QCRIL_GSTKI_H

/*===========================================================================
                           INCLUDE FILES
===========================================================================*/

#include "qcrili.h"
#include "qcril_log.h"

#ifdef QCRIL_MMGSDI_UNIT_TEST
#include "assert.h"
#endif /* QCRIl_MMGSDI_UNIT_TEST */

#ifndef FEATURE_CDMA_NON_RUIM
#define FEATURE_GSTK_ENH_MULTI_CLIENT_SUPPORT
#include "gstk_exp.h"
#endif /* FEATURE_CDMA_NON_RUIM */

/*===========================================================================
                        DEFINITIONS AND TYPES
===========================================================================*/

#endif /* QCRIL_GSTKI_H */
