ifneq (,$(filter arm aarch64 arm64, $(TARGET_ARCH)))

LOCAL_PATH := $(call my-dir)
commonSources :=

include $(CLEAR_VARS)
LOCAL_MODULE       := yv12_qcif.yuv
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

#
# rotator
#=======================================================
include $(CLEAR_VARS)
LOCAL_MODULE:= rotator
CFLAGS += -DFEATURE_MEMORY_VIRTUAL
LOCAL_SRC_FILES := $(commonSources) \
                   rotator.c

LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif
