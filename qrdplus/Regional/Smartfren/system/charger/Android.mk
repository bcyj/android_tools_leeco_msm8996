LOCAL_PATH := $(call my-dir)

$(shell mkdir -p $(TARGET_OUT)/vendor/Smartfren/system/charger)
$(shell cp -r $(LOCAL_PATH)/*.png $(TARGET_OUT)/vendor/Smartfren/system/charger)
