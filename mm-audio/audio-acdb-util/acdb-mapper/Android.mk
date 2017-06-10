ifeq ($(call is-board-platform-in-list,msm8660 msm8974 msm8226 msm8610 copper apq8084 msm8916 msm8994 msm8909),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libacdbmapper-def := -g -O3
libacdbmapper-def += -D_ANDROID_
libacdbmapper-def += -D_ENABLE_QC_MSG_LOG_
# ---------------------------------------------------------------------------------
#             Make the Shared library (libaudcalctrl)
# ---------------------------------------------------------------------------------

libacdbmapper-inc     := $(LOCAL_PATH)/inc
libacdbmapper-inc     += $(LOCAL_PATH)/src

LOCAL_MODULE            := libacdbmapper
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libacdbmapper-def)
LOCAL_C_INCLUDES	:= $(libacdbmapper-inc)
LOCAL_C_INCLUDES	+= $(TARGET_OUT_HEADERS)/mm-audio/audio-alsa

LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libcutils libutils liblog libaudioalsa
LOCAL_COPY_HEADERS_TO   := mm-audio/audio-acdb-util
LOCAL_COPY_HEADERS      := inc/acdb-id-mapper.h

LOCAL_SRC_FILES         := src/acdb-id-mapper.c

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif
endif # is-board-platform

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------

