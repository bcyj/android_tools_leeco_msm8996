#ifndef QCRIL_UIM_RESTART_H
#define QCRIL_UIM_RESTART_H
/*===========================================================================

  Copyright (c) 2011, 2013, 2105 Qualcomm Technologies, Inc. All Rights Reserved

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
06/15/15   vdc     Fix silent pin verification during MSSR
09/10/13   yt      Clear encrypted PIN data after card reset
04/10/13   yt      Silent PIN verification support for multi SIM
03/01/13   yt      Report SIM_STATUS only after finishing silent PIN verify
04/11/11   yt      Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "ril.h"
#include "qcril_uim.h"


/*=========================================================================

  FUNCTION:  qcril_uim_store_encrypted_pin

===========================================================================*/
/*!
    @brief
    Stores encrypted PIN1 along with ICCID and application ID.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_store_encrypted_pin
(
  qmi_uim_rsp_data_type                        *rsp_data_ptr,
  qmi_uim_session_type                         session_type
);


/*=========================================================================

  FUNCTION:  qcril_uim_try_pin1_verification

===========================================================================*/
/*!
    @brief
    Checks ICCID, AID and PIN1 status before sending PIN1 for verification
    after modem restart.

    @return
    Status of PIN1 verification.
*/
/*=========================================================================*/
RIL_Errno qcril_uim_try_pin1_verification
(
   uint8                                       slot
);


/*=========================================================================

  FUNCTION:  qcril_uim_clear_encrypted_pin

===========================================================================*/
/*!
    @brief
    Clears cached value of encrypted PIN1.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_clear_encrypted_pin
(
  qmi_uim_session_type                         session_type
);


/*=========================================================================

  FUNCTION:  qcril_uim_check_silent_pin_verify_in_progress

===========================================================================*/
/*!
    @brief
    Function to check if silent pin1 verification is in progress. Resets
    the flag if PIN1 state is ENABLED_VERIFIED.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_check_silent_pin_verify_in_progress
(
  uint8                                       slot
);


/*=========================================================================

  FUNCTION:  qcril_uim_clear_encrypted_pin_after_card_reset

===========================================================================*/
/*!
    @brief
    Clears encrypted PIN1 data after card reset.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_clear_encrypted_pin_after_card_reset
(
  uint8                                        slot
);


/*=========================================================================

  FUNCTION:  qcril_uim_process_modem_restart_start

===========================================================================*/
/*!
    @brief
    Resets state of QCRIL_UIM, sends card error events to QCRIL, and sends
    unsolicited response to Android framework for change in card status.
    Called as a result of QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_modem_restart_start
(
  const qcril_request_params_type            * const params_ptr,
  qcril_request_return_type                  * const ret_ptr
);


/*=========================================================================

  FUNCTION:  qcril_uim_process_modem_restart_complete

===========================================================================*/
/*!
    @brief
    Initializes state of QCRIL_UIM and sets global to indicate that the
    modem has restarted. Called as a result of
    QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_COMPLETE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_modem_restart_complete
(
  const qcril_request_params_type            * const params_ptr,
  qcril_request_return_type                  * const ret_ptr
);


/*===========================================================================

  FUNCTION:  qcril_uim_is_silent_pin_verification_needed

===========================================================================*/
/*!
    @brief
    Checks if pin needs to be verified silently for any application on the given slot.

    @return
    TRUE if silent pin verification is needed, else FALSE
*/
/*=========================================================================*/
boolean qcril_uim_is_silent_pin_verification_needed
(
  uint8                  slot
);

#endif /* QCRIL_UIM_RESTART_H */
