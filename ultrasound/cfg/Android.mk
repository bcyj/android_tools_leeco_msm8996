LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Put readme.txt
include $(CLEAR_VARS)
LOCAL_MODULE:= readme.txt
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# Put version.txt
include $(CLEAR_VARS)
LOCAL_MODULE:= version.txt
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# Build all of the sub-targets
include $(call all-makefiles-under, $(LOCAL_PATH))


