LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := qmi_ping_svc

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc

common_ping_svc_cflags := -g
common_ping_svc_cflags += -O2
common_ping_svc_cflags += -fno-inline
common_ping_svc_cflags += -fno-short-enums
common_ping_svc_cflags += -fpic
common_ping_svc_cflags += -Wall
common_ping_svc_cflags += -Werror

LOCAL_SRC_FILES  := qmi_ping_api_v01.c qmi_ping_svc.c qmi_ping_svc_ipc_router_main.c
LOCAL_C_FLAGS := $(common_ping_svc_cflags)
LOCAL_MODULE_TAGS := debug
LOCAL_SHARED_LIBRARIES := libqmi_csi libqmi_common_so
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)/qmi-framework-tests
include $(BUILD_EXECUTABLE)
