LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO := subsystem_control/inc
LOCAL_COPY_HEADERS := subsystem_control.h

LOCAL_SRC_FILES:= \
    subsystem_control_v01.c \
    subsystem_control_client.c

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(TARGET_OUT_HEADERS)/qmi-framework/inc \
    $(TARGET_OUT_HEADERS)/qmi/inc \
    $(TARGET_OUT_HEADERS)/qmi-framework/qcci/inc \
    $(TARGET_OUT_HEADERS)/common/inc \
    $(TARGET_OUT_HEADERS)/subsystem_control/inc \
    $(TARGET_OUT_HEADERS)/libmdmdetect/inc \
    $(TARGET_OUT_HEADERS)/libperipheralclient/inc

LOCAL_SHARED_LIBRARIES := \
    libc \
    libcutils \
    libqmi_cci \
    libqmi_common_so \
    libmdmdetect \
    libperipheral_client

LOCAL_MODULE := libsubsystem_control
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
