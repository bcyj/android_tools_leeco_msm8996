LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#################################################
SPEC_PROP := vendor.prop
$(shell mkdir -p $(TARGET_OUT)/vendor/ChinaMobile/system/vendor/)
$(shell cp -r $(LOCAL_PATH)/$(SPEC_PROP) $(TARGET_OUT)/vendor/ChinaMobile/system/vendor/$(SPEC_PROP))
