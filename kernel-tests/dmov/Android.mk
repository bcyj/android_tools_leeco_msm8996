ifneq (,$(filter arm aarch64 arm64, $(TARGET_ARCH)))

LOCAL_PATH := $(call my-dir)
KERNEL_PATH := $(TOP)/kernel
commonSources :=

#
#dmov
#=======================================================
include $(CLEAR_VARS)
LOCAL_MODULE:= msm_dma

LOCAL_C_FLAGS := -lpthread

LOCAL_SRC_FILES := $(commonSources)
LOCAL_SRC_FILES += msm_dma.c

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := msm_dma.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := msm_dma.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

endif
