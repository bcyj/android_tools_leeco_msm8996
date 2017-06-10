LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

commonIncludes := $(TARGET_OUT_HEADERS)/common/inc
commonIncludes += $(TARGET_OUT_HEADERS)/sensors/inc
commonIncludes += $(TARGET_OUT_HEADERS)/qmi/inc
commonIncludes += $(TARGET_OUT_HEADERS)/diag/include

commonCflags   := $(remote_api_defines)
commonCflags   += $(remote_api_enables)

LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sensortest/jni/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sensordaemon/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sensordaemon/common/util/mathtools/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sensordaemon/ucos-ii_posix/inc
LOCAL_C_INCLUDES += hardware/libhardware/include
LOCAL_C_INCLUDES += hardware/libhardware/include/hardware

# Figure out if this build system is a QCOM build. If so, include
# the path to the sanitized headers in the target "out" dir.
ifeq ($(call is-vendor-board-platform,QCOM),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE := activity_recognition.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SHARED_LIBRARIES += \
    libc \
    libutils \
    libcutils \
    liblog \
    libhardware \
    libsensor1 \
    libsensor_reg \
    libdl

LOCAL_SHARED_LIBRARIES += libdiag

LOCAL_SRC_FILES := src/ar_hal.c
LOCAL_SRC_FILES += src/ar_hal_time.cpp

LOCAL_CFLAGS := -DSNS_LA
LOCAL_CFLAGS += -Werror

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
