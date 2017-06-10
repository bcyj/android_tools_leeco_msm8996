ifeq ($(strip $(TARGET_USES_QTIC_CM)),true)
include $(call all-subdir-makefiles)
endif
