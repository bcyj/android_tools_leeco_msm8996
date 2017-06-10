#ifndef QCRIL_UIM_SAP_H
#define QCRIL_UIM_SAP_H
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
09/29/14   at      Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "qcril_uim.h"
#include "user_identity_module_v01.h"


/*===========================================================================

  FUNCTION:  qcril_qmi_sap_ind_hdlr

===========================================================================*/
/*!
    @brief
    This is the SAP indication callback implementation for the QMI interface.

    @return
    None.

*/
/*=========================================================================*/
void qcril_qmi_sap_ind_hdlr
(
  uim_sap_connection_ind_msg_v01   * ind_data_ptr
);


/*=========================================================================

  FUNCTION:  qcril_uim_sap_qmi_handle_sap_connection_resp

===========================================================================*/
/*!
    @brief
    Handles the SAP connection request callback. Based on the response
    received from the modem, respective packed response types are constructed
    and the onRequestComplete is called. This completes the original request
    called on the RIL SAP socket.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_sap_qmi_handle_sap_connection_resp
(
  const qcril_uim_callback_params_type * const params_ptr
);


/*=========================================================================

  FUNCTION:  qcril_uim_sap_qmi_handle_sap_request_resp

===========================================================================*/
/*!
    @brief
    Handles the SAP request callback. Based on the response
    received from the modem, respective packed response types are constructed
    and the onRequestComplete is called. This completes the original request
    called on the RIL SAP socket.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_sap_qmi_handle_sap_request_resp
(
  const qcril_uim_callback_params_type * const params_ptr
);


/*=========================================================================

  FUNCTION:  RIL_SAP_Init

===========================================================================*/
/*!
    @brief
    Initializes QMI_UIM service for SAP interface. It is called whenever
    RILD starts or modem restarts.

    @return
    Pointer to RIL_RadioFunctions.
*/
/*=========================================================================*/
const RIL_RadioFunctions     * RIL_SAP_Init
(
  const struct RIL_Env  *env,
  int                    argc,
  char                 **argv
);

#endif /* QCRIL_UIM_SAP_H */

