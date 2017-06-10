LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#################################################
PRE_LOAD_SPEC := .preloadspec
$(shell mkdir -p $(TARGET_OUT)/vendor/CherryCommon)
$(shell cp -r $(LOCAL_PATH)/$(PRE_LOAD_SPEC) $(TARGET_OUT)/vendor/CherryCommon/$(PRE_LOAD_SPEC))

#################################################
SPEC_PROP := local.prop
$(shell mkdir -p $(TARGET_OUT)/vendor/CherryCommon/data)
$(shell cp -r $(LOCAL_PATH)/$(SPEC_PROP) $(TARGET_OUT)/vendor/CherryCommon/data/$(SPEC_PROP))


