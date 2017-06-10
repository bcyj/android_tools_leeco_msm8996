/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE_QMI_CLIENT
#define CRI_VOICE_QMI_CLIENT

#include "cri_core.h"

typedef struct
{
    int qmi_voice_client_id;
    int qmi_csvt_client_id;
} cri_voice_qmi_client_info_type;

int cri_voice_qmi_client_get_voice_client(const cri_voice_qmi_client_info_type* qmi_client_info_ptr);
int cri_voice_qmi_client_get_csvt_client(const cri_voice_qmi_client_info_type* qmi_client_info_ptr);

qmi_error_type_v01 cri_voice_qmi_client_init(cri_voice_qmi_client_info_type* qmi_client_info_ptr, hlos_ind_cb_type hlos_ind_cb_func_ptr);
void cri_voice_qmi_client_deinit(cri_voice_qmi_client_info_type* qmi_client_info_ptr);

hlos_ind_cb_type cri_voice_qmi_client_get_hlos_ind_cb(const cri_voice_qmi_client_info_type* qmi_client_info_ptr);

#endif
