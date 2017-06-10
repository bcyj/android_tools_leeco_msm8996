LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#################################################
$(shell mkdir -p $(TARGET_OUT)/vendor/Smartfren/system/etc)
$(shell cp -r $(LOCAL_PATH)/*.xml $(TARGET_OUT)/vendor/Smartfren/system/etc)
