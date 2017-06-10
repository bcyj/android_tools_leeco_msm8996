ifeq ($(strip $(TARGET_USES_QTIC_VIDEO_CALL)),true)
include $(call all-subdir-makefiles)
endif
