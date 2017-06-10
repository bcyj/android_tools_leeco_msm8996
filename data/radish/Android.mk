LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/common/inc/
LOCAL_SRC_FILES:= radish.c config.c icmpv6.c
LOCAL_CFLAGS := -Wall -Werror -Wno-error=unused-but-set-variable
LOCAL_CFLAGS += -DFEATURE_DATA_LINUX_ANDROID
LOCAL_MODULE := radish
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := libcutils liblog libc
include $(BUILD_EXECUTABLE)
