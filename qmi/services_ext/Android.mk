LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../core/lib/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_SHARED_LIBRARIES := libidl

LOCAL_SRC_FILES := common_v01.c
LOCAL_SRC_FILES += voice_service_common_v02.c
LOCAL_SRC_FILES += voice_service_v02.c
LOCAL_SRC_FILES += wireless_data_service_v01.c
LOCAL_SRC_FILES += wireless_data_administrative_service_v01.c
LOCAL_SRC_FILES += wireless_messaging_service_v01.c
LOCAL_SRC_FILES += device_management_service_v01.c
LOCAL_SRC_FILES += network_access_service_v01.c
LOCAL_SRC_FILES += user_identity_module_v01.c
LOCAL_SRC_FILES += card_application_toolkit_v02.c
LOCAL_SRC_FILES += phonebook_manager_service_v01.c
LOCAL_SRC_FILES += sar_vs_service_v01.c
LOCAL_SRC_FILES += specific_absorption_rate_v01.c
LOCAL_SRC_FILES += control_service_v01.c
LOCAL_SRC_FILES += qmi_ims_vt_v01.c
LOCAL_SRC_FILES += qualcomm_mobile_access_point_v01.c
LOCAL_SRC_FILES += circuit_switched_video_telephony_v01.c
LOCAL_SRC_FILES += ip_multimedia_subsystem_video_telephony_v01.c
LOCAL_SRC_FILES += ip_multimedia_subsystem_presence_v01.c
LOCAL_SRC_FILES += coexistence_manager_v01.c
LOCAL_SRC_FILES += ip_multimedia_subsystem_settings_v01.c
LOCAL_SRC_FILES += radio_frequency_radiated_performance_enhancement_v01.c
LOCAL_SRC_FILES += ip_multimedia_subsystem_application_v01.c
LOCAL_SRC_FILES += ip_multimedia_subsystem_rtp_v01.c
LOCAL_SRC_FILES += ip_multimedia_subsystem_dcm_v01.c
LOCAL_SRC_FILES += data_system_determination_v01.c
LOCAL_SRC_FILES += data_port_mapper_v01.c
LOCAL_SRC_FILES += data_common_v01.c
LOCAL_SRC_FILES += persistent_device_configuration_v01.c

LOCAL_MODULE:= libqmiservices_ext

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
