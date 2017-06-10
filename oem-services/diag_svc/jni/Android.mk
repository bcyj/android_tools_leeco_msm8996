LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	DiagJNIInterface.cpp

LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE) \
	$(TARGET_OUT_HEADERS)/common/inc \
	$(TARGET_OUT_HEADERS)/diag/include

LOCAL_SHARED_LIBRARIES := \
	libandroid_runtime \
	libnativehelper \
	libutils \
	libcutils \
	libdiag

LOCAL_MODULE:= libDiagService
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
