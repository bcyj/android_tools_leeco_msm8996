# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
LOCAL_PATH := $(call my-dir)

#MAKE_DB_ZIP
include $(CLEAR_VARS)
LOCAL_MODULE := db.zip
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/mimerdbfiles
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)


