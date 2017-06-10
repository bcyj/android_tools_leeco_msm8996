LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO := qmi-framework/inc
LOCAL_COPY_HEADERS    := ../inc/qmi_common.h

common_libqmi_common_cflags := -g
common_libqmi_common_cflags += -O2
common_libqmi_common_cflags += -fno-inline
common_libqmi_common_cflags += -fno-short-enums
common_libqmi_common_cflags += -fpic
common_libqmi_common_cflags += -Wall
common_libqmi_common_cflags += -Werror
common_libqmi_common_cflags += -Wno-missing-field-initializers
common_libqmi_common_cflags += -Wno-unused-parameter


LOCAL_CFLAGS := $(common_libqmi_common_cflags)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc

LOCAL_SRC_FILES  := common_v01.c

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libutils

LOCAL_MODULE := libqmi_common

LOCAL_PRELINK_MODULE := false

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

common_libqmi_common_cflags += -O2
common_libqmi_common_cflags += -fno-inline
common_libqmi_common_cflags += -fno-short-enums
common_libqmi_common_cflags += -fpic

LOCAL_CFLAGS := $(common_libqmi_common_cflags)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc

LOCAL_SRC_FILES  := common_v01.c

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libutils

LOCAL_MODULE := libqmi_common_so
LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
