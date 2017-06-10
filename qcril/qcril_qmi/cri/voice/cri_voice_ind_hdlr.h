/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE_IND_HDLR
#define CRI_VOICE_IND_HDLR

#include "voice_service_v02.h"

void cri_voice_ind_hdlr_all_call_status_ind(
    int qmi_service_client_id,
    voice_all_call_status_ind_msg_v02* call_status_ind_ptr
);

void cri_voice_ind_hdlr_info_rec_ind(
    int qmi_service_client_id,
    const voice_info_rec_ind_msg_v02* info_rec_ind_ptr
);

void cri_voice_ind_hdlr_conference_info_ind(
    int qmi_service_client_id,
    const voice_conference_info_ind_msg_v02* conf_info_ind_ptr
);

void cri_voice_ind_hdlr_otasp_status_ind(
    int qmi_service_client_id,
    const voice_otasp_status_ind_msg_v02* conf_info_ind_ptr
);

void cri_voice_ind_hdlr_privacy_ind(
    int qmi_service_client_id,
    const voice_privacy_ind_msg_v02* privacy_ind_ptr
);

void cri_voice_ind_hdlr_ext_brst_intl_ind(
    int qmi_service_client_id,
    const voice_ext_brst_intl_ind_msg_v02* ext_brst_intl_ind_ptr
);

void cri_voice_ind_hdlr_sups_notification_ind(
    int qmi_service_client_id,
    const voice_sups_notification_ind_msg_v02* sups_notification_ind_ptr
);

#endif
