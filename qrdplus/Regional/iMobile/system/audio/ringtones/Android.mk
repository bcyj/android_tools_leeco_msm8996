LOCAL_PATH := $(call my-dir)

$(shell mkdir -p $(TARGET_OUT)/vendor/iMobile/system/media/audio/ringtones)
$(shell cp -r $(LOCAL_PATH)/*.ogg $(TARGET_OUT)/vendor/iMobile/system/media/audio/ringtones)
$(shell cp -r $(LOCAL_PATH)/*.mid $(TARGET_OUT)/vendor/iMobile/system/media/audio/ringtones)
$(shell cp -r $(LOCAL_PATH)/*.wav $(TARGET_OUT)/vendor/iMobile/system/media/audio/ringtones)
$(shell cp -r $(LOCAL_PATH)/*.mp3 $(TARGET_OUT)/vendor/iMobile/system/media/audio/ringtones)
