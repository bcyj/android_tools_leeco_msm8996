LOCAL_PATH := $(call my-dir)

$(shell mkdir -p $(TARGET_OUT)/vendor/CherryCommon/system/media/audio/alarms)
$(shell cp -r $(LOCAL_PATH)/*.ogg $(TARGET_OUT)/vendor/CherryCommon/system/media/audio/alarms)
$(shell cp -r $(LOCAL_PATH)/*.mp3 $(TARGET_OUT)/vendor/CherryCommon/system/media/audio/alarms)
