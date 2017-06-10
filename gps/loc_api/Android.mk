ifneq ($(BUILD_TINY_ANDROID),true)
ifneq ($(TARGET_NO_RPC),true)

include $(call all-subdir-makefiles)

endif # not TARGET_NO_RPC := true
endif
