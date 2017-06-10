ifeq ($(call is-board-platform-in-list,msm8974 msm8610 msm8226 msm8610 apq8084 msm8916 msm8994 msm8909),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libacdbrtac-def := -g -O3
libacdbrtac-def += -D_ANDROID_
libacdbrtac-def += -D_ENABLE_QC_MSG_LOG_
# ---------------------------------------------------------------------------------
#             Make the Shared library (libacdbrtac)
# ---------------------------------------------------------------------------------

libacdbrtac-inc     := $(LOCAL_PATH)/inc
libacdbrtac-inc     += $(LOCAL_PATH)/src

LOCAL_MODULE            := libacdbrtac
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libacdbrtac-def)
LOCAL_C_INCLUDES        := $(libacdbrtac-inc)
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/mm-audio/audcal
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/mm-audio/audio-acdb-util
LOCAL_C_INCLUDES        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libcutils libutils liblog libaudcal
LOCAL_COPY_HEADERS_TO   := mm-audio/audio-acdb-util

LOCAL_COPY_HEADERS      := inc/acdb-rtac.h

LOCAL_SRC_FILES         := src/acdb-rtac.c




LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif
endif # is-board-platform-in-list

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
