################################################################################
# @file tests/dsi_netctrl/src/Android.mk
# @brief Makefile for building dsi_netctrl test script(s)
################################################################################

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := dsi_netctrl_test.c

LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libdsi_netctrl

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsi_netctrl/inc

LOCAL_MODULE := dsi_netctrl_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)


LOCAL_SRC_FILES := dsi_netctrl_test2.c

LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libdsi_netctrl
LOCAL_SHARED_LIBRARIES += libqmi
LOCAL_SHARED_LIBRARIES += libqcci_legacy
LOCAL_SHARED_LIBRARIES += libqmiservices
LOCAL_SHARED_LIBRARIES += libcutils

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsi_netctrl/inc
LOCAL_C_INCLUDES += system/core/include/cutils/

LOCAL_MODULE := dsi_netctrl_test2
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := dsi_netctrl_daemon.c
LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libqdp
LOCAL_SHARED_LIBRARIES += libqmi
LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libdsi_netctrl

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsi_netctrl/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc

LOCAL_MODULE := dsi_netctrl_daemon
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := dsi_netctrl_client.c

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsi_netctrl/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc

LOCAL_MODULE := dsi_netctrl_client
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test

LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)
