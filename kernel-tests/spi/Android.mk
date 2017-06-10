ifneq (,$(filter arm aarch64 arm64, $(TARGET_ARCH)))

LOCAL_PATH := $(call my-dir)
commonSources :=
commonSharedLibraries := libc libcutils libutils



include $(CLEAR_VARS)
LOCAL_MODULE := spidevtest
LOCAL_SRC_FILES += $(commonSources) spidevtest.c
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_SHARED_LIBRARIES := $(commonSharedLibraries)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := spidevtest.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := spidevtest.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := spiethernettest.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := spiethernettest.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

endif
