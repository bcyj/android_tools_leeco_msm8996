LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
 
#################################################
PRE_LOAD_SPEC := .preloadspec
$(shell mkdir -p $(TARGET_OUT)/vendor/MTN)
$(shell cp -r $(LOCAL_PATH)/$(PRE_LOAD_SPEC) $(TARGET_OUT)/vendor/MTN/$(PRE_LOAD_SPEC))

#################################################
SPEC_PROP := vendor.prop
$(shell mkdir -p $(TARGET_OUT)/vendor/MTN/system/vendor/)
$(shell cp -r $(LOCAL_PATH)/$(SPEC_PROP) \
    $(TARGET_OUT)/vendor/MTN/system/vendor/$(SPEC_PROP))

#################################################
EXCLUDE_LIST := exclude.list
$(shell mkdir -p $(TARGET_OUT)/vendor/MTN)
$(shell cp -r $(LOCAL_PATH)/$(EXCLUDE_LIST) \
    $(TARGET_OUT)/vendor/MTN/$(EXCLUDE_LIST))
