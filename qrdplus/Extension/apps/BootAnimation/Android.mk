LOCAL_PATH := $(call my-dir)

$(shell mkdir -p $(TARGET_OUT)/vendor/Default/system/media)

ifeq ($(call is-board-platform-in-list,msm8226 msm8916),true)
$(shell cp -r $(LOCAL_PATH)/bootanimation.zip $(TARGET_OUT)/vendor/Default/system/media)
$(shell cp -r $(LOCAL_PATH)/shutdownanimation.zip $(TARGET_OUT)/vendor/Default/system/media)
else
$(shell cp -r $(LOCAL_PATH)/bootanimation_WVGA.zip $(TARGET_OUT)/vendor/Default/system/media/bootanimation.zip)
$(shell cp -r $(LOCAL_PATH)/shutdownanimation_WVGA.zip $(TARGET_OUT)/vendor/Default/system/media/shutdownanimation.zip)
endif
$(shell cp -r $(LOCAL_PATH)/*.wav $(TARGET_OUT)/vendor/Default/system/media)
