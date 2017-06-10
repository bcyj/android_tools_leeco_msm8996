ifneq ($(TARGET_USES_AOSP), true)
include $(call all-subdir-makefiles)
endif
