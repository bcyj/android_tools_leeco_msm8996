CURRENT_PATH := $(call my-dir)

PRE_LOAD_SPEC := .preloadspec
$(shell mkdir -p $(TARGET_OUT)/vendor/ChinaMobile)
$(shell cp -r $(CURRENT_PATH)/$(PRE_LOAD_SPEC) $(TARGET_OUT)/vendor/ChinaMobile/$(PRE_LOAD_SPEC))

#################################################
EXCLUDE_LIST := exclude.list
$(shell mkdir -p $(TARGET_OUT)/vendor/ChinaMobile)
$(shell cp -r $(CURRENT_PATH)/$(EXCLUDE_LIST) $(TARGET_OUT)/vendor/ChinaMobile/$(EXCLUDE_LIST))

include $(call all-subdir-makefiles)
