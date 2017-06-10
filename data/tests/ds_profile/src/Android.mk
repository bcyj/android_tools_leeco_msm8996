################################################################################
# @file tests/ds_profile/src/Android.mk
# @brief Makefile for building ds_profile test script(s)
################################################################################

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := dsprofile_get_3gpp_profiles.c

LOCAL_CFLAGS := -DFEATURE_DATA_LOG_STDERR

LOCAL_SHARED_LIBRARIES += libdsutils
LOCAL_SHARED_LIBRARIES += libdsprofile

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../ds_profile/inc

LOCAL_MODULE := dsprofile_get_3gpp_profiles
LOCAL_MODULE_TAGS := optional debug
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)

