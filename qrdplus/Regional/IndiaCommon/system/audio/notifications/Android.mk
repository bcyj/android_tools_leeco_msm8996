LOCAL_PATH := $(call my-dir)

$(shell mkdir -p $(TARGET_OUT)/vendor/IndiaCommon/system/media/audio/notifications)
$(shell cp -r $(LOCAL_PATH)/*.ogg $(TARGET_OUT)/vendor/IndiaCommon/system/media/audio/notifications)
