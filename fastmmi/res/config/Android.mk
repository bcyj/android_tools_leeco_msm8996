LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(call is-board-platform,msm8916),true)
LOCAL_SRC_FILES := mmi-8916.cfg
endif

ifeq ($(call is-board-platform-in-list,msm8909 msm8909_512),true)
LOCAL_SRC_FILES := mmi-8909.cfg
endif

LOCAL_CFLAGS := -Wall
LOCAL_MODULE := mmi.cfg
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/mmi
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := mmi-pcba.cfg
LOCAL_CFLAGS := -Wall
LOCAL_MODULE := mmi-pcba.cfg
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/mmi
include $(BUILD_PREBUILT)