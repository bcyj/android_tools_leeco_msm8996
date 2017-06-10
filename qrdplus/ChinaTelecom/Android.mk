ifeq ($(strip $(TARGET_USES_QTIC_CT)),true)
include $(call all-subdir-makefiles)
endif
