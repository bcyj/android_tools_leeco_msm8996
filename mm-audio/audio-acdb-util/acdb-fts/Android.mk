ifeq ($(call is-board-platform-in-list, msm8960 msm8974 msm8610 msm8226 copper apq8084 msm8916 msm8994 msm8909),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libacdb-mcs-def := -g -O3
libacdb-mcs-def += -D_ANDROID_
libacdb-mcs-def += -D_ENABLE_QC_MSG_LOG_
# ---------------------------------------------------------------------------------
#             Make the Shared library (libaudcalctrl)
# ---------------------------------------------------------------------------------

libacdb-mcs-inc     := $(LOCAL_PATH)/inc
libacdb-mcs-inc     += $(LOCAL_PATH)/src

LOCAL_MODULE            := libacdb-fts
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libacdb-fts-def)
LOCAL_C_INCLUDES        := $(libacdb-fts-inc)
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/mm-audio/audcal
LOCAL_C_INCLUDES        += external/tinyalsa/include
LOCAL_COPY_HEADERS      := inc/acdb-fts.h

LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libcutils libutils liblog libaudcal
LOCAL_COPY_HEADERS_TO   := mm-audio/audio-acdb-util


LOCAL_SRC_FILES         := src/acdb-fts.c


LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)


endif
endif # is-board-platform-in-list

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
