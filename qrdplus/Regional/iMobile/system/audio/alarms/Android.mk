LOCAL_PATH := $(call my-dir)

$(shell mkdir -p $(TARGET_OUT)/vendor/iMobile/system/media/audio/alarms)
$(shell cp -r $(LOCAL_PATH)/*.ogg $(TARGET_OUT)/vendor/iMobile/system/media/audio/alarms)
