LOCAL_PATH := $(call my-dir)

$(shell mkdir -p $(TARGET_OUT)/vendor/LatamOpenAMX/system/media/audio/cleartones)
$(shell cp -r $(LOCAL_PATH)/*.wav $(TARGET_OUT)/vendor/LatamOpenAMX/system/media/audio/cleartones)
$(shell cp -r $(LOCAL_PATH)/*.mp3 $(TARGET_OUT)/vendor/LatamOpenAMX/system/media/audio/cleartones)
