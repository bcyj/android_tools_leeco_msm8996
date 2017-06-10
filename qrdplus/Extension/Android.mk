ifeq ($(strip $(TARGET_USES_QTIC_EXTENSION)),true)
include $(call all-subdir-makefiles)
endif
