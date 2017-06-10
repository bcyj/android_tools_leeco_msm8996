LOCAL_PATH := $(call my-dir)

$(shell mkdir -p $(TARGET_OUT)/vendor/CherryMyanmar/system/media)
$(shell cp -r $(LOCAL_PATH)/*.zip $(TARGET_OUT)/vendor/CherryMyanmar/system/media)
