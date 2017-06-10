#ifndef QCRIL_GSTK_QMI_H
#define QCRIL_GSTK_QMI_H
/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.


when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/28/14   at      Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "qcril_uim_srvc.h"
#include "qcril_qmi_client.h"


/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_init
===========================================================================*/
/*!
  @brief
    Initialize the RIL GSTK subsystem

    - Obtain Handle from QMI

  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_qmi_init
(
  void
);


/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_srvc_release_client
===========================================================================*/
/*!
  @brief
    Releases a previously opened QMI CAT client service handle via the
    qcril_gstk_qmi_init() API.

  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_qmi_srvc_release_client
(
  void
);

#endif /* QCRIL_GSTK_QMI_H */

