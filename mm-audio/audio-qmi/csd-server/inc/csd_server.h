/* Copyright (c) 2012, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef QMI_AUDIO_SERVICE_H
#define QMI_AUDIO_SERVICE_H

#include "qmi_idl_lib.h"
#include "qmi_csi.h"
#include "qmi_csi_common.h"
#include "qmi_cci_target_ext.h"
#include "qmi_sap.h"
#include "core_sound_driver_v01.h"

#define QMI_CLNT_WAIT_SIG  0x00010000
#define QMI_CLNT_TIMER_SIG  0x00000001


enum _qmi_csd_init_status_t
{
    QMI_CSD_INIT_DONE  = 1,
    QMI_CSD_DINIT_DONE,
    QMI_CSD_INIT_FAILED,
    QMI_CSD_DINIT_FAILED
};

static uint32_t  qmi_csd_init   = 0xFFFFFFFF;  /* default boot up value */
                         /* 1  - CSD INIT is done */
                         /* 2  - CSD DE INIT is done */
                         /* 3  - QMI CSD INIT is called and csd_init failed */
                         /* 4  - QMI CSD De Init called and csd_deinit failed */
                         /* 0xFFFFFFFF  - QMI CSD INIT Never called */

/* This struct is used to hold context of the server.  Cleanup is used
 * to signal the reader thread to clean up and die, num_requests just tracks
 * how many requests the service has handled, and service_handle is passed
 * into the qmi_csi_handle_event function
 * An instance of this structure is passed to the qmi_csi_register function as
 * the service_cookie, and this structure can be tailored to a specific services
 * needs.
 */
typedef struct {
    qmi_csi_service_handle service_handle;
    qmi_sap_client_handle qsap_handle;
    int num_requests;
}service_context_type;


static service_context_type service_cookie;

typedef struct _qmi_csd_dev_cmd_resp_msg_v01
{
    union
    {
        qmi_csd_ioctl_dev_cmd_enable_resp_msg_v01 qmi_csd_enable_dev_resp;
        qmi_csd_ioctl_dev_cmd_disable_resp_msg_v01 qmi_csd_disable_dev_resp;
        qmi_csd_ioctl_dev_cmd_afe_loopback_resp_msg_v01 qmi_csd_dev_afe_loopback_resp;
        qmi_csd_ioctl_dev_cmd_anc_control_resp_msg_v01 qmi_csd_dev_anc_control_resp;
        qmi_csd_ioctl_dev_cmd_companding_control_resp_msg_v01 qmi_csd_dev_companding_resp;
        qmi_csd_ioctl_dev_cmd_get_max_device_nums_resp_msg_v01 qmi_csd_max_dev_resp;
        qmi_csd_ioctl_dev_cmd_get_dev_caps_resp_msg_v01 qmi_csd_dev_caps_resp;
        qmi_csd_ioctl_dev_cmd_dtmf_control_resp_msg_v01 qmi_csd_dtmf_control_dev_resp;
        qmi_csd_ioctl_dev_cmd_sidetone_control_resp_msg_v01 qmi_csd_dev_sidetone_control_resp;
        qmi_csd_ioctl_dev_cmd_configure_resp_msg_v01 qmi_csd_dev_configure_resp;
        qmi_csd_ioctl_dev_cmd_connect_device_resp_msg_v01 qmi_csd_dev_connect_dev_resp;
    }u;
}qmi_csd_dev_cmd_resp_msg;

typedef struct _qmi_csd_dev_cmd_req_msg_v01
{
    union
    {
        qmi_csd_ioctl_dev_cmd_enable_req_msg_v01 qmi_csd_enable_dev_req;
        qmi_csd_ioctl_dev_cmd_disable_req_msg_v01 qmi_csd_disable_dev_req;
        qmi_csd_ioctl_dev_cmd_afe_loopback_req_msg_v01 qmi_csd_dev_afe_loopback_req;
        qmi_csd_ioctl_dev_cmd_anc_control_req_msg_v01 qmi_csd_dev_anc_control_req;
        qmi_csd_ioctl_dev_cmd_companding_control_req_msg_v01 qmi_csd_dev_companding_req;
        qmi_csd_ioctl_dev_cmd_get_max_device_nums_req_msg_v01 qmi_csd_max_dev_req;
        qmi_csd_ioctl_dev_cmd_get_dev_caps_req_msg_v01 qmi_csd_dev_caps_req;
        qmi_csd_ioctl_dev_cmd_dtmf_control_req_msg_v01 qmi_csd_dtmf_control_dev_req;
        qmi_csd_ioctl_dev_cmd_sidetone_control_req_msg_v01 qmi_csd_dev_sidetone_control_req;
        qmi_csd_ioctl_dev_cmd_configure_req_msg_v01 qmi_csd_dev_configure_req;
        qmi_csd_ioctl_dev_cmd_connect_device_req_msg_v01 qmi_csd_dev_connect_dev_req;
    }u;
}qmi_csd_dev_cmd_req_msg;

typedef struct _qmi_csd_vc_cmd_resp_msg_v01
{
    union
    {
        qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01 qmi_csd_get_ui_property_resp;
        qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01 qmi_csd_set_ui_property_resp;
        qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01 qmi_csd_tx_dtmf_detection_resp;
        qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01 qmi_csd_set_mute_resp;
        qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01 qmi_csd_set_rx_volume_resp;
        qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01 qmi_csd_disable_resp;
        qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01 qmi_csd_enable_resp;
        qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01 qmi_csd_set_device_config_resp;
        qmi_csd_open_voice_context_resp_msg_v01 qmi_csd_open_context_resp;
        qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01 qmi_csd_set_volume_steps_resp;
        qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01 qmi_csd_set_rx_volume_step_resp;
    }u;
}qmi_csd_vc_cmd_resp_msg;

typedef struct _qmi_csd_vc_cmd_req_msg_v01
{
    union
    {
        qmi_csd_ioctl_vc_cmd_get_ui_property_req_msg_v01 qmi_csd_get_ui_property_req;
        qmi_csd_ioctl_vc_cmd_set_ui_property_req_msg_v01 qmi_csd_set_ui_property_req;
        qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_req_msg_v01 qmi_csd_tx_dtmf_detection_req;
        qmi_csd_ioctl_vc_cmd_set_mute_req_msg_v01 qmi_csd_set_mute_req;
        qmi_csd_ioctl_vc_cmd_set_rx_volume_index_req_msg_v01 qmi_csd_set_rx_volume_req;
        qmi_csd_ioctl_vc_cmd_disable_req_msg_v01 qmi_csd_disable_req;
        qmi_csd_ioctl_vc_cmd_enable_req_msg_v01 qmi_csd_enable_req;
        qmi_csd_ioctl_vc_cmd_set_device_config_req_msg_v01 qmi_csd_set_device_config_req;
        qmi_csd_open_voice_context_req_msg_v01 qmi_csd_open_context_req;
        qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_req_msg_v01 qmi_csd_set_volume_steps_req;
        qmi_csd_ioctl_vc_cmd_set_rx_volume_step_req_msg_v01 qmi_csd_set_rx_volume_step_req;
    }u;
}qmi_csd_vc_cmd_req_msg;

typedef struct _qmi_csd_vm_cmd_resp_msg_v01
{
    union
    {
        qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01 qmi_csd_attach_stream_resp;
        qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01 qmi_csd_detach_stream_resp;
        qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01 qmi_csd_attach_context_resp;
        qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01 qmi_csd_detach_context_resp;
        qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01 qmi_csd_start_voice_resp;
        qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01 qmi_csd_standby_voice_resp;
        qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01 qmi_csd_stop_voice_resp;
        qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01 qmi_csd_set_network_resp;
        qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01 qmi_csd_set_voice_timing_resp;
        qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01 qmi_csd_set_tty_mode_resp;
        qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01 qmi_csd_set_widevoice_resp;
        qmi_csd_ioctl_vm_cmd_set_hdvoice_mode_resp_msg_v01 qmi_csd_set_hdvoice_mode_resp;
    }u;
}qmi_csd_vm_cmd_resp_msg;

typedef struct _qmi_csd_vm_cmd_req_msg_v01
{
    union
    {
        qmi_csd_ioctl_vm_cmd_attach_stream_req_msg_v01 qmi_csd_attach_stream_req;
        qmi_csd_ioctl_vm_cmd_detach_stream_req_msg_v01 qmi_csd_detach_stream_req;
        qmi_csd_ioctl_vm_cmd_attach_context_req_msg_v01 qmi_csd_attach_context_req;
        qmi_csd_ioctl_vm_cmd_detach_context_req_msg_v01 qmi_csd_detach_context_req;
        qmi_csd_ioctl_vm_cmd_start_voice_req_msg_v01 qmi_csd_start_voice_req;
        qmi_csd_ioctl_vm_cmd_standby_voice_req_msg_v01 qmi_csd_standby_voice_req;
        qmi_csd_ioctl_vm_cmd_stop_voice_req_msg_v01 qmi_csd_stop_voice_req;
        qmi_csd_ioctl_vm_cmd_set_network_req_msg_v01 qmi_csd_set_network_req;
        qmi_csd_ioctl_vm_cmd_set_voice_timing_req_msg_v01 qmi_csd_set_voice_timing_req;
        qmi_csd_ioctl_vm_cmd_set_tty_mode_req_msg_v01 qmi_csd_set_tty_mode_req;
        qmi_csd_ioctl_vm_cmd_set_widevoice_req_msg_v01 qmi_csd_set_widevoice_req;
        qmi_csd_ioctl_vm_cmd_set_hdvoice_mode_req_msg_v01 qmi_csd_set_hdvoice_mode_req;
    }u;
}qmi_csd_vm_cmd_req_msg;


typedef struct _qmi_csd_vs_cmd_resp_msg_v01
{
    union
    {
        qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01 qmi_csd_set_media_type_resp;
        qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01 qmi_csd_set_mute_resp;
        qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01 qmi_csd_enc_dtx_mode_resp;
        qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01 qmi_csd_dec_timewarp_resp;
        qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01 qmi_csd_enc_minmax_rate_resp;
        qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01 qmi_csd_enc_rate_modulation_resp;
        qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01 qmi_csd_qcelp_set_rate_resp;
        qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01 qmi_csd_4gvnb_set_rate_resp;
        qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01 qmi_csd_4gvwb_set_rate_resp;
        qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01 qmi_csd_amr_set_enc_rate_resp;
        qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01 qmi_csd_amrwb_set_enc_rate_resp;
        qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01 qmi_csd_dtmf_gen_resp;
        qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01 qmi_csd_rx_dtmf_dect_resp;
        qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01 qmi_csd_set_ui_prop_resp;
        qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01 qmi_csd_get_ui_prop_resp;
        qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01 qmi_csd_start_record_resp;
        qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01 qmi_csd_stop_record_resp;
        qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01 qmi_csd_start_playback_resp;
        qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01 qmi_csd_stop_playback_resp;
    }u;
}qmi_csd_vs_cmd_resp_msg;

typedef struct _qmi_csd_vs_cmd_req_msg_v01
{
    union
    {
        qmi_csd_ioctl_vs_cmd_set_media_type_req_msg_v01 qmi_csd_set_media_type_req;
        qmi_csd_ioctl_vs_cmd_set_mute_req_msg_v01 qmi_csd_set_mute_req;
        qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_req_msg_v01 qmi_csd_enc_dtx_mode_req;
        qmi_csd_ioctl_vs_cmd_set_dec_timewarp_req_msg_v01 qmi_csd_dec_timewarp_req;
        qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_req_msg_v01 qmi_csd_enc_minmax_rate_req;
        qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_req_msg_v01 qmi_csd_enc_rate_modulation_req;
        qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_req_msg_v01 qmi_csd_qcelp_set_rate_req;
        qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_req_msg_v01 qmi_csd_4gvnb_set_rate_req;
        qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_req_msg_v01 qmi_csd_4gvwb_set_rate_req;
        qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_req_msg_v01 qmi_csd_amr_set_enc_rate_req;
        qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_req_msg_v01 qmi_csd_amrwb_set_enc_rate_req;
        qmi_csd_ioctl_vs_cmd_set_dtmf_generation_req_msg_v01 qmi_csd_dtmf_gen_req;
        qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_req_msg_v01 qmi_csd_rx_dtmf_dect_req;
        qmi_csd_ioctl_vs_cmd_set_ui_property_req_msg_v01 qmi_csd_set_ui_prop_req;
        qmi_csd_ioctl_vs_cmd_get_ui_property_req_msg_v01 qmi_csd_get_ui_prop_req;
        qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01 qmi_csd_start_record_req;
        qmi_csd_ioctl_vs_cmd_stop_record_req_msg_v01 qmi_csd_stop_record_req;
        qmi_csd_ioctl_vs_cmd_start_playback_req_msg_v01 qmi_csd_start_playback_req;
        qmi_csd_ioctl_vs_cmd_stop_playback_req_msg_v01 qmi_csd_stop_playback_req;
    }u;
}qmi_csd_vs_cmd_req_msg;

#endif
