#ifndef QCRIL_UIM_REMOTE_H
#define QCRIL_UIM_REMOTE_H
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

#include "user_identity_module_remote_v01.h"
#include "qcril_uim.h"

/*===========================================================================

                           TYPES

===========================================================================*/

/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_REMOTE_IND_PARAMS_TYPE

   DESCRIPTION:
     Structure used to copy indications received from the modem
-------------------------------------------------------------------------------*/
typedef struct
{
  qcril_instance_id_e_type         instance_id;
  qcril_modem_id_e_type            modem_id;
  qmi_client_type                  handle;
  unsigned int                     msg_id;
  void                           * msg_ptr;
} qcril_uim_remote_ind_params_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_REMOTE_CB_PARAMS_TYPE

   DESCRIPTION:
     Structure used to copy response received from the modem
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned int                         msg_id;
  void                               * msg_ptr;
  qmi_client_error_type                transp_err;
  qcril_uim_original_request_type    * orig_req_ptr;
} qcril_uim_remote_cb_params_type;


/*===========================================================================

  FUNCTION:  qcril_uim_remote_init

===========================================================================*/
void qcril_uim_remote_init
(
 void
);


/*===========================================================================

  FUNCTION:  qcril_uim_release

===========================================================================*/
void qcril_uim_release
(
 void
);


/*===========================================================================

  FUNCTION:  qcril_uim_remote_client_request_apdu

===========================================================================*/
void qcril_uim_remote_client_request_apdu
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
);


/*===========================================================================

  FUNCTION:  qcril_uim_remote_client_request_event

===========================================================================*/
void qcril_uim_remote_client_request_event
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
);

#endif /* QCRIL_UIM_REMOTE_H */

