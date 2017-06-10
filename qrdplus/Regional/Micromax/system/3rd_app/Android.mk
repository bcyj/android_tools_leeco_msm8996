LOCAL_PATH := $(call my-dir)

$(shell mkdir -p $(TARGET_OUT)/vendor/Micromax/system/app)
$(shell cp -r $(LOCAL_PATH)/*.apk $(TARGET_OUT)/vendor/Micromax/system/app)
