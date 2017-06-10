ifneq ($(call is-android-codename-in-list,JELLY_BEAN),true)
ifeq ($(call is-board-platform,msm8960),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE      := msm_sps_test
LOCAL_C_INCLUDES  := $(TARGET_OUT_HEADERS)/kernel-tests/sps
LOCAL_SRC_FILES   := msm_sps_test.c
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif
endif # JELLY BEAN
