ifeq ($(call is-vendor-board-platform,QCOM),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include


LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES:= \
        fmfactorytestclient.c \

LOCAL_MODULE:= fmfactorytest
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES  += libcutils
LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)
endif # filter
