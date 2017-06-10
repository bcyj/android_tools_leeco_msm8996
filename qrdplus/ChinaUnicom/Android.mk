ifeq ($(strip $(TARGET_USES_QTIC_CU)),true)
include $(call all-subdir-makefiles)
endif
