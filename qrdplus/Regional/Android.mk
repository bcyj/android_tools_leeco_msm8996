ifeq ($(strip $(TARGET_USES_QTIC_REGIONAL)),true)
include $(call all-subdir-makefiles)
endif
