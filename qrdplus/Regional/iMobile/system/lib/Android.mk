LOCAL_PATH := $(call my-dir)

$(shell mkdir -p $(TARGET_OUT)/vendor/iMobile/system/lib)
$(shell cp -r $(LOCAL_PATH)/*.so $(TARGET_OUT)/vendor/iMobile/system/lib)
