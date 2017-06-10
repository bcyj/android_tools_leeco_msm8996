/******************************************************************************

  @file    qmi_ril_peripheral_mng.h
  @brief   Provides interface to communicate with peripheral manager

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef QCRIL_PM_H
#define QCRIL_PM_H

#include "qcril_log.h"

/*===========================================================================

  FUNCTION  qmi_ril_peripheral_mng_init

===========================================================================*/
/*!
    @brief
        Initialize peripheral manager client

    @return
        0 if function is successful.
*/
/*=========================================================================*/
int qmi_ril_peripheral_mng_init
(
    char *device_name
);

/*===========================================================================

  FUNCTION  qmi_ril_peripheral_mng_deinit

===========================================================================*/
/*!
    @brief
        de init peripheral manager client

    @return
        None
*/
/*=========================================================================*/
void qmi_ril_peripheral_mng_deinit
(
    void
);

/*===========================================================================

  FUNCTION  qmi_ril_peripheral_mng_vote

===========================================================================*/
/*!
    @brief
        Vote for registered device with peripheral manager

    @return
        0 if function is successful.
*/
/*=========================================================================*/
int qmi_ril_peripheral_mng_vote
(
    void
);

/*===========================================================================

  FUNCTION  qmi_ril_peripheral_mng_release_vote

===========================================================================*/
/*!
    @brief
        Release vote from peripheral manager

    @return
        0 if function is successful.
*/
/*=========================================================================*/
void qmi_ril_peripheral_mng_release_vote
(
    void
);

#endif /* QCRIL_PM_H */
