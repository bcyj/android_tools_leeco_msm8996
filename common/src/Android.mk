ifeq ($(TARGET_ARCH),arm)

LOCAL_PATH := $(call my-dir)
commonSources :=

include $(CLEAR_VARS)
LOCAL_MODULE:= n_smux
LOCAL_SRC_FILES += $(commonSources) n_smux_test.c
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif
