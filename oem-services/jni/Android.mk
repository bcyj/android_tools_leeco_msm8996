LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	SubSystemShutdown.cpp

LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE) \
	$(TARGET_OUT_HEADERS)/common/inc \
	$(TARGET_OUT_HEADERS)/subsystem_control/inc \
	$(TARGET_OUT_HEADERS)/libmdmdetect/inc

LOCAL_SHARED_LIBRARIES := \
	libandroid_runtime \
	libnativehelper \
	libsubsystem_control \
	libcutils \
	libmdmdetect

LOCAL_MODULE := libSubSystemShutdown
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

