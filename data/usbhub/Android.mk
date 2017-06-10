
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
        system/kernel_headers/ \
        $(LOCAL_PATH) \
        $(TARGET_OUT_HEADERS)/common/inc/ \

LOCAL_SRC_FILES := \
        usbhub.c \


LOCAL_SHARED_LIBRARIES := libhardware_legacy
LOCAL_SHARED_LIBRARIES += libcutils libutils libc liblog


LOCAL_MODULE:= usbhub
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)

