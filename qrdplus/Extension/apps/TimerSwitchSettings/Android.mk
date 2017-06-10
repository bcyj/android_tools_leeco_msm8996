LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
#include res directory from timepicker
datetimepicker_dir := ../../../../../../../frameworks/opt/datetimepicker/res
res_dirs := $(datetimepicker_dir) res
LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dirs))

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_JAVA_LIBRARIES := android-support-v13
LOCAL_STATIC_JAVA_LIBRARIES += android-opt-datetimepicker
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v4

LOCAL_SRC_FILES := \
        $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := TimerSwitch
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true
LOCAL_AAPT_FLAGS := --auto-add-overlay
LOCAL_AAPT_FLAGS += --extra-packages com.android.datetimepicker

include $(BUILD_PACKAGE)
