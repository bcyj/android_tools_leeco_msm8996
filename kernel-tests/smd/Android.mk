ifneq (,$(filter arm aarch64 arm64, $(TARGET_ARCH)))

LOCAL_PATH := $(call my-dir)
commonSources :=

include $(CLEAR_VARS)
LOCAL_MODULE:= smd_tty_loopback_test
LOCAL_C_FLAGS := -lpthread
LOCAL_SRC_FILES += $(commonSources) smd_tty_loopback_test.c
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE:= smem_log_test
LOCAL_C_FLAGS := -lpthread
LOCAL_SRC_FILES += $(commonSources) smem_log_test.c
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE:= smd_pkt_loopback_test
LOCAL_C_FLAGS := -lpthread
LOCAL_SRC_FILES += $(commonSources) smd_pkt_loopback_test.c
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif
