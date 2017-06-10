/******************************************************************************
  @file    qcril_pbm.h
  @brief   qcril qmi - PB

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2008-2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#ifndef QCRIL_PBM_H
#define QCRIL_PBM_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "qcrili.h"
#include "qmi_client.h"


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

typedef enum
{
  QCRIL_PBM_DEVICE_TYPE_ECC,
  QCRIL_PBM_DEVICE_TYPE_FDN
} qcril_pbm_device_type;

#define NON_STD_OTASP_NUMBER "*228"


/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/
void qcril_qmi_pbm_init
(
  void
);

void qcril_qmi_pbm_unsol_ind_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                          *ind_buf,
  unsigned int                   ind_buf_len,
  void                          *ind_cb_data
);

int qmi_ril_phone_number_is_emergency(char * number_to_check);

int qmi_ril_phone_number_is_non_std_otasp(const char * number_to_check);

RIL_Errno qcril_qmi_pbm_enable_emergency_number_indications(int enable);

int qmi_ril_set_ecc_property(char* ecc_numbers);

void qcril_qmi_pbm_unsolicited_indication_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

#endif /* QCRIL_PBM_H */
