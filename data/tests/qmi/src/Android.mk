
################################################################################
# $Id: //linux/pkgs/proprietary/data/main/source/test/dss/src/Makefile#1 $
# 
# @file tests/unit/libs/dss/src/Makefile
# @brief Makefile for building dss api tests.
################################################################################

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_call_bringup_test_interactive.c \

LOCAL_SHARED_LIBRARIES := \
	libqmi \

LOCAL_CFLAGS := \
	-DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	system/kernel_headers/ \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_call_bringup_test_interactive
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_sdio_call_bringup_test_interactive.c \

LOCAL_SHARED_LIBRARIES := \
	libqmi \

LOCAL_CFLAGS := \
	-DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(TARGET_OUT_HEADERS)/common/inc/ \
	

LOCAL_MODULE:= qmi_sdio_call_bringup_test_interactive
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_sdio_call_bringup_test.c \

LOCAL_SHARED_LIBRARIES := \
	libqmi \

LOCAL_CFLAGS := \
	-DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_sdio_call_bringup_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

############## qmi_all_call_bringup_test #########

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_all_call_bringup_test.c \

LOCAL_SHARED_LIBRARIES := \
	libqmi \

LOCAL_CFLAGS := \
	-DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_all_call_bringup_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

############## qmi_umts_profile_config_test #########

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_umts_profile_config_test.c \

LOCAL_SHARED_LIBRARIES := \
	libqmi \

LOCAL_CFLAGS := \
	-DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(LOCAL_PATH)/../../../dsutils/inc \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_umts_profile_config_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

############## qmi_umts_mult_pdp_test #########

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_umts_mult_pdp_test.c\

LOCAL_SHARED_LIBRARIES := \
	libqmi \

LOCAL_CFLAGS := \
	-DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_umts_mult_pdp_test 
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

############## qmi_umts_same_apn_test#########

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_umts_same_apn_test.c \

LOCAL_SHARED_LIBRARIES := \
	libqmi \

LOCAL_CFLAGS := \
	-DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_umts_same_apn_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

############## qmi_umts_diff_apn_test#########

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_umts_diff_apn_test.c \

LOCAL_SHARED_LIBRARIES := \
	libqmi \

LOCAL_CFLAGS := \
	-DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_umts_diff_apn_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)


############## qmi_client_release_test #########

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_client_release_test.c \

LOCAL_SHARED_LIBRARIES := \
	libqmi \


LOCAL_CFLAGS := \
	-DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_client_release_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)


############## qmi_uim_test #########

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_uim_test.c \

LOCAL_SHARED_LIBRARIES := \
        libqmi \


LOCAL_CFLAGS := \
        -DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_uim_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)


############## qmi_client_reg_dereg_test #########

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_client_reg_dereg_test.c

LOCAL_SHARED_LIBRARIES := \
	libqmi \
	libcutils \

LOCAL_CFLAGS := \
	-DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(TARGET_OUT_HEADERS)/common/inc/ \
	system/core/include

LOCAL_MODULE:= qmi_client_reg_dereg_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)


############## qmi_akav1_test #########

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        qmi_akav1_test.c 

LOCAL_SHARED_LIBRARIES := \
        libqmi \


LOCAL_CFLAGS := \
        -DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_akav1_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)


############## qmi_akav2_test #########

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        qmi_akav2_test.c 

LOCAL_SHARED_LIBRARIES := \
        libqmi \


LOCAL_CFLAGS := \
        -DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_akav2_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)


############## qmi_omh_profiles_test #########

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_omh_profiles_test.c

LOCAL_SHARED_LIBRARIES := \
	libqmi \

LOCAL_CFLAGS := \
	-DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../../qmi/src \
	$(TARGET_OUT_HEADERS)/data/inc/ \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_omh_profiles_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

############## qmi_cdma_profile_config_test #########

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qmi_cdma_profile_config_test.c \

LOCAL_SHARED_LIBRARIES := \
	libqmi \

LOCAL_CFLAGS := \
	-DFEATURE_DSS_LINUX_ANDROID \

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_HEADERS)/qmi/inc \
	$(LOCAL_PATH)/../../../dss/inc \
	$(LOCAL_PATH)/../../../dss/src \
	$(LOCAL_PATH)/../../../dsutils/inc \
	$(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_MODULE:= qmi_cdma_profile_config_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

