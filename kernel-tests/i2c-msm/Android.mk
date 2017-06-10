ifneq (,$(filter arm aarch64 arm64, $(TARGET_ARCH)))

LOCAL_PATH := $(call my-dir)
commonSources :=

include $(CLEAR_VARS)
LOCAL_MODULE := i2c-msm-test
LOCAL_SRC_FILES += $(commonSources) i2c-msm-test.c
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests


LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils

LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := i2c-msm-test.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := i2c-msm-test.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

endif
