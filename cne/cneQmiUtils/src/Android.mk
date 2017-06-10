LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO := cne/inc
LOCAL_COPY_HEADERS += ../inc/CneQmiUtils.h

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= CneQmiUtils.cpp

LOCAL_MODULE:= libcneqmiutils
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -Wconversion

LOCAL_SHARED_LIBRARIES := libc libutils

LOCAL_C_INCLUDES := \
        $(LOCAL_PATH)/../inc \
        $(TARGET_OUT_HEADERS)/common/inc \
        $(TARGET_OUT_HEADERS)/qmi/inc

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true


include $(BUILD_SHARED_LIBRARY)
