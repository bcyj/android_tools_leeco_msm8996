ifneq ($(call is-android-codename,HONEYCOMB),true)
include $(call all-subdir-makefiles)
endif
