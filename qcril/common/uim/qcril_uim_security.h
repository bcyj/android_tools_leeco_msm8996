#ifndef QCRIL_UIM_SECURITY_H
#define QCRIL_UIM_SECURITY_H
/*===========================================================================

  Copyright (c) 2010, 2015 Qualcomm Technologies, Inc. All Rights Reserved

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

$Header: //depot/asic/sandbox/users/micheleb/ril/qcril_uim_security.h#4 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/23/15   vr      SIMLOCK Temperory unlock status indication
03/18/15   at      Perso reactivation support
10/31/12   at      Explicit query for card status during PIN responses
06/29/10   at      Changes to support pin verification APIs
05/13/10   at      Clean up for merging with mainline
05/12/10   at      Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "qcril_uim.h"

/* Android property the temperory unlock status */
#define QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS         "persist.radio.uim_temp_unlock"
#define QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_TRUE    "1"
#define QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_FALSE   "0"

/*===========================================================================

  FUNCTION:  qcril_uim_pin_resp

===========================================================================*/
/*!
    @brief
    Handle PIN operation confirmations

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_pin_resp
(
  qcril_uim_callback_params_type             * params_ptr,
  qcril_request_return_type                  * const ret_ptr
);


/*===========================================================================

  FUNCTION:  qcril_uim_deperso_resp

===========================================================================*/
/*!
    @brief
    Handle deperso operation confirmation

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_deperso_resp
(
  qcril_uim_callback_params_type  * params_ptr
);


/*===========================================================================

  FUNCTION:  qcril_uim_perso_resp

===========================================================================*/
/*!
    @brief
    Handle perso operation confirmation

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_perso_resp
(
  qcril_uim_callback_params_type  * params_ptr
);


/*===========================================================================

  FUNCTION:  qcril_uim_request_set_pin_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS request from
    the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_set_pin_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
);


/*===========================================================================

  FUNCTION:  qcril_uim_request_get_pin_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_pin_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
);

/*===========================================================================

  FUNCTION:  qcril_uim_process_simlock_temp_unlock_ind

===========================================================================*/
/*!
    @brief
    Main function for processing QMI SIMLOCK temporary unlock indication.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_simlock_temp_unlock_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr,
  qcril_request_return_type               * const ret_ptr /*!< Output parameter */
);

/*===========================================================================

  FUNCTION:  qcril_uim_check_and_send_temp_unlock_expiry_ind

===========================================================================*/
/*!
    @brief
    Function to check the temperory unlock status and send expiry indication

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_check_and_send_temp_unlock_expiry_ind
(
  void
);

#endif /* QCRIL_UIM_SECURITY_H */

