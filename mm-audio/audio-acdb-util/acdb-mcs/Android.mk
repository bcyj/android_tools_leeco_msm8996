ifeq ($(call is-board-platform-in-list, msm8960 msm8974 msm8610 msm8226 copper apq8084 msm8916 msm8994 msm8909),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
libacdb-mcs-inc     += $(LOCAL_PATH)/inc
libacdb-mcs-inc     += $(LOCAL_PATH)/src

LOCAL_C_INCLUDES        := $(libacdb-mcs-inc)
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/mm-audio/audcal
LOCAL_SRC_FILES:= src/acdb-mcs-test.c
LOCAL_MODULE := acdb-mcs-test

LOCAL_SHARED_LIBRARIES:= libcutils libutils libacdbloader
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

endif
endif # is-board-platform-in-list

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
