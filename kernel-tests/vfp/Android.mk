ifneq (,$(filter arm aarch64 arm64, $(TARGET_ARCH)))

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE       := vfp.sh
LOCAL_SRC_FILES    := vfp.sh
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH  := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE      := vfptest
LOCAL_SRC_FILES   := vfptest.c
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif
