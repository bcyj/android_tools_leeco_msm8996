ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) <=19 ))" )))

include $(call all-subdir-makefiles)

endif
