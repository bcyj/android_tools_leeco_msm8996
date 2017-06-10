LOCAL_PATH := $(call my-dir)

$(shell mkdir -p $(TARGET_OUT)/vendor/Fly/system/media/audio/alarms)
$(shell cp -r $(LOCAL_PATH)/*.ogg $(TARGET_OUT)/vendor/Fly/system/media/audio/alarms)
