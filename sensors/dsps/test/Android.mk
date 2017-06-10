LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
commonSources  :=
#commonIncludes := $(TARGET_OUT_HEADERS)common/inc
#commonIncludes += $(TARGET_OUT_HEADERS)/qmi/inc
#commonIncludes += $(TARGET_OUT_HEADERS)/sensors/inc
commonIncludes += $(LOCAL_PATH)/../api
commonIncludes += $(LOCAL_PATH)/../../../qmi/core/lib/inc/

LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc

# Figure out if this build system is a QCOM build. If so, include
# the path to the sanitized headers in the target "out" dir.
ifeq ($(call is-vendor-board-platform,QCOM),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE:=sns_cm_test
#LOCAL_MODULE_OWNER := qcom

LOCAL_SHARED_LIBRARIES += \
    libc \
    liblog \
    libutils \
    libsensor1

LOCAL_SRC_FILES += \
  src/sns_cm_test.cpp

LOCAL_CFLAGS += -D_GNU_SOURCE -DSNS_LA

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)


