#-------------------------------------------------------------------------------
#                      make the als library (libmm-als)
#-------------------------------------------------------------------------------

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE             := libmm-als
LOCAL_MODULE_TAGS        := optional
LOCAL_MODULE_OWNER       := qti
LOCAL_SRC_FILES          := NativeLightSensor.cpp
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES   := libdl libandroid libcutils
LOCAL_C_FLAGS            := -DLOG_TAG=\"NativeLightSensor\"
LOCAL_C_INCLUDES         := $(TARGET_OUT_HEADERS)/pp/inc
LOCAL_C_INCLUDES         += $(TARGET_OUT_HEADERS)/common/inc

include $(BUILD_SHARED_LIBRARY)
