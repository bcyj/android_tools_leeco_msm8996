#ifndef QCRIL_UIM_REFRESH_H
#define QCRIL_UIM_REFRESH_H
/*===========================================================================

  Copyright (c) 2010-2011 Qualcomm Technologies, Inc. All Rights Reserved

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_uim_refresh.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/11   yt      Added support for ISIM Refresh
01/11/11   at      Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "qcril_uim.h"

/*=========================================================================

  FUNCTION:  qcril_uim_refresh_register_resp

===========================================================================*/
/*!
    @brief
    Process the response for refresh register.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_refresh_register_resp
(
  const qcril_uim_callback_params_type * const params_ptr
);


/*===========================================================================

  FUNCTION:  qcril_uim_init_refresh_info

===========================================================================*/
/*!
    @brief
    Initializes the global refresh info structure.
    
    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_init_refresh_info
(
  void
);


/*===========================================================================

  FUNCTION:  qcril_uim_cleanup_refresh_info

===========================================================================*/
/*!
    @brief
    Frees the memory in global refresh info structure.
    
    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_cleanup_refresh_info
(
  void
);


/*===========================================================================

  FUNCTION:  qcril_uim_refresh_register

===========================================================================*/
/*!
    @brief
    Checks if GW and Primary subscription Apps are in ready state. If yes,
    it Prepares the data needed for refresh register QMI command and sends it.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_refresh_register
(
  qmi_uim_session_type                qmi_session_type
);


#ifdef FEATURE_QCRIL_UIM_ISIM_REFRESH    
/*===========================================================================

  FUNCTION:  qcril_uim_refresh_register_isim

===========================================================================*/
/*!
    @brief
    Registers for refresh for an ISIM App.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_refresh_register_isim
(
  qmi_uim_slot_type                   qmi_slot
);
#endif /* FEATURE_QCRIL_UIM_ISIM_REFRESH */


/*===========================================================================

  FUNCTION:  qcril_uim_process_refresh_ind

===========================================================================*/
/*!
    @brief
    Main function for processing QMI refresh indications. Based 
    on the indication received, if needed, it updates the global card status,  
    ret_ptr and sends card events internal to QCRIL (CM & PBM).

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_refresh_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr,
  qcril_request_return_type               * const ret_ptr   /*!< Output parameter */
);


#endif /* QCRIL_UIM_REFRESH_H */

